//==============================================================================
/**
 * @file	gf_overlay.c
 * @brief	オーバーレイ制御
 * @author	GAME FREAK inc.
 * @date	2006.03.06
 */
//==============================================================================
#include <nitro.h>
#include <nitro/fs.h>
#include "gflib.h"
#include "gf_overlay.h"
#include "assert.h"


#if 0
NitroStaticInitを使用するには
/*
 * NitroStaticInit() を static initializer に指定するには
 * このヘッダをインクルードします.
 */
#include <nitro/sinit.h>

#endif


//==============================================================================
//	定数定義
//==============================================================================

//--------------------------------------------------------------
/**
 * @brief	オーバーレイの展開メモリエリア
 */
//--------------------------------------------------------------
typedef enum{
	GF_OVERLAY_AREA_MAIN = 0,			///<メインメモリ
	GF_OVERLAY_AREA_ITCM,				///<命令TCM
	GF_OVERLAY_AREA_DTCM,				///<データTCM
}GF_OVERLAY_AREA;

//--------------------------------------------------------------
//	デバッグ用定義
//--------------------------------------------------------------
#ifdef PM_DEBUG
///定義が有効の場合はオーバーレイのアンロード実行時にアンロードした領域をクリアする
#define DEBUG_OVELAY_UNLOAD_CLEAR
#endif


//==============================================================================
//	構造体定義
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief	オーバーレイ用ワーク定義
 */
//--------------------------------------------------------------
typedef struct{
	FSOverlayID loaded_id;
	BOOL loaded;
}OVERLAY_WORK;

//--------------------------------------------------------------
/**
 * @brief	オーバーレイ制御システム用ワーク定義
 */
//--------------------------------------------------------------
typedef struct{
	u8 main_num;
	u8 itcm_num;
	u8 dtcm_num;
	OVERLAY_WORK * main;
	OVERLAY_WORK * itcm;
	OVERLAY_WORK * dtcm;
}OVERLAY_SYSTEM;

//==============================================================================
//	グローバル変数宣言
//==============================================================================
static OVERLAY_SYSTEM * OverlaySys;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Overlay_Unload(OVERLAY_WORK *ovwork);
static BOOL CheckCrossArea(const FSOverlayID id);
static OVERLAY_WORK * GetWorkPointer(GF_OVERLAY_AREA memory_area);
static int GetWorkSize(GF_OVERLAY_AREA memory_area);
static BOOL GetOverlayAddress(const FSOverlayID id, u32 *start, u32 *end);
static GF_OVERLAY_AREA GetMemoryAreaID(const FSOverlayID id);
static void UnloadEachWork(OVERLAY_WORK * ov_wk, int count);


//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
void GFL_OVERLAY_boot(HEAPID heapID, int main_num, int itcm_num, int dtcm_num)
{
	int size;
	size = sizeof(OVERLAY_SYSTEM) + sizeof(OVERLAY_WORK) * (main_num + itcm_num + dtcm_num);

	GF_ASSERT(main_num >= 0);
	GF_ASSERT(itcm_num >= 0);
	GF_ASSERT(dtcm_num >= 0);
	OverlaySys = GFL_HEAP_AllocMemory(heapID, size);
	GFL_STD_MemClear32(OverlaySys, size);

	OverlaySys->main_num = main_num;
	OverlaySys->itcm_num = itcm_num;
	OverlaySys->dtcm_num = dtcm_num;
	OverlaySys->main = (OVERLAY_WORK *)((u8*)OverlaySys + sizeof(OVERLAY_SYSTEM));
	OverlaySys->itcm = OverlaySys->main + main_num;
	OverlaySys->dtcm = OverlaySys->itcm + itcm_num;
}

//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static void UnloadEachWork(OVERLAY_WORK * ov_wk, int count)
{
	int i;
	
	for(i = 0; i < count; i++){
		if(ov_wk[i].loaded == TRUE){
			GF_ASSERT(0);
			Overlay_Unload(&ov_wk[i]);
		}
	}
}
//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
void GFL_OVERLAY_Exit(void)
{
	UnloadEachWork(OverlaySys->main, OverlaySys->main_num);

	UnloadEachWork(OverlaySys->itcm, OverlaySys->itcm_num);

	UnloadEachWork(OverlaySys->dtcm, OverlaySys->dtcm_num);

	GFL_HEAP_FreeMemory(OverlaySys);
}

//--------------------------------------------------------------
/**
 * @brief   オーバーレイをアンロードします
 * @param   ovwork		オーバーレイワークへのポインタ
 */
//--------------------------------------------------------------
static void Overlay_Unload(OVERLAY_WORK *ovwork)
{
	//現在読み込まれているオーバーレイモジュールをアンロードします
	BOOL ret;

	GF_ASSERT(ovwork->loaded == TRUE);

	ret = FS_UnloadOverlay(MI_PROCESSOR_ARM9, ovwork->loaded_id);
	OS_Printf("overlay unload id = 0x%08X\n", ovwork->loaded_id);
	GF_ASSERT(ret == TRUE);
	ovwork->loaded = FALSE;

#ifdef DEBUG_OVELAY_UNLOAD_CLEAR
	{
		u32 start, end;
		//アンロードしたオーバーレイ領域を0クリアするために解放する前にアドレス取得
		GetOverlayAddress(ovwork->loaded_id, &start, &end);
		//アンロードしたオーバーレイ領域をクリアする
		OS_TPrintf("overlay unload start address = 0x%08X\n", start);
		OS_TPrintf("overlay unload end address = 0x%08X\n", end);
		MI_CpuFill8((void*)start, 0xff, end - start);
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   指定したオーバーレイIDのオーバーレイをアンロードします
 *
 * @param   id		オーバーレイID
 *
 */
//--------------------------------------------------------------
void GFL_OVERLAY_Unload(const FSOverlayID id)
{
	OVERLAY_WORK *ovwork;
	int memory_area;
	int size;
	int i;
	
	GF_ASSERT_MSG(OverlaySys != NULL, "Overlay Systemが初期化されていない\n");

	if (id == GFL_OVERLAY_BLANK_ID) {
		OS_TPrintf("不要なoverlay unload 呼び出し\n");
		return;
	}

	memory_area = GetMemoryAreaID(id);
	ovwork = GetWorkPointer(memory_area);
	size = GetWorkSize(memory_area);
	
	for(i = 0; i < size; i++){
		if(ovwork[i].loaded == TRUE && ovwork[i].loaded_id == id){
			//現在読み込まれているオーバーレイモジュールをアンロードします
			Overlay_Unload(&ovwork[i]);
			return;
		}
	}
	OS_TPrintf("エラー：読み込んでいないOverlayIDへのUnload要求(%d)\n",ovwork[i].loaded_id);
	GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief   指定したオーバーレイをロードする
 *
 * @param   id				ロードするオーバーレイID
 *
 * @retval  TRUE:成功
 * @retval  FALSE:失敗
 *
 * 複数のオーバーレイを実行している時、領域が被った場合はオーバーレイは失敗します。
 * (デバッグ中はGF_ASSERTで停止します)
 *
 * ロードはFS_SetDefaultDMA(or FS_Init)で設定されている方法で行います。
 * 使用するDMA,CPUを変更したい場合は、この関数実行前にFS_SetDefaultDMAで設定を行ってください。
 * ITCM,DTCMへのロードは内部で一時的にCPUに変更してロードを行います。(ロード後は元の設定に戻します)
 */
//--------------------------------------------------------------
BOOL GFL_OVERLAY_Load(const FSOverlayID id)
{
	BOOL ret;
	int memory_area;
	u32 dma_bak = FS_DMA_NOT_USE;
	OVERLAY_WORK *ovwork;
	int size;
	int i;
	
	GF_ASSERT_MSG(OverlaySys != NULL, "Overlay Systemが初期化されていない\n");

	if (id == GFL_OVERLAY_BLANK_ID) {
		OS_TPrintf("不要なoverlay load 呼び出し\n");
		return TRUE;
	}
	if(CheckCrossArea(id) == FALSE){
		return FALSE;
	}

	//空きワークサーチ
	memory_area = GetMemoryAreaID(id);
	ovwork = GetWorkPointer(memory_area);
	size = GetWorkSize(memory_area);
	for(i = 0; i < size; i++){
		if(ovwork[i].loaded == FALSE){
			ovwork = &ovwork[i];
			ovwork->loaded = TRUE;
			ovwork->loaded_id = id;
			break;
		}
	}
	if(i >= size){
		GF_ASSERT(0);	//同メモリエリア内にオーバーレイ出来る数を超えている
		return FALSE;
	}
	
	if(memory_area == GF_OVERLAY_AREA_ITCM || memory_area == GF_OVERLAY_AREA_DTCM){
		//TCMへの書き込みはDMAでは出来ないので一時的にCPUへ変更
		dma_bak = FS_SetDefaultDMA(FS_DMA_NOT_USE);
	}
	
	ret = FS_LoadOverlay(MI_PROCESSOR_ARM9, id);

	if(memory_area == GF_OVERLAY_AREA_ITCM || memory_area == GF_OVERLAY_AREA_DTCM){
		//一時的に変更したのを元に戻す
		FS_SetDefaultDMA(dma_bak);
	}

	if(ret == FALSE){	//ロード失敗
		OS_Printf("overlay 0x%08X is not available!\n", id);
		GF_ASSERT(0);
		return FALSE;
	}

	OS_Printf("overlay load id = 0x%08X\n", id);
	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   指定したオーバーレイIDの領域が現在展開されているオーバーレイ領域とアドレスが
 *          被っていないかチェックする
 *
 * @param   id		オーバーレイID
 *
 * @retval  TRUE:被っていない。　FALSE:被っている(エラーの場合も有)
 */
//--------------------------------------------------------------
static BOOL CheckCrossArea(const FSOverlayID id)
{
	GF_OVERLAY_AREA memory_area;
	u32 start, end, c_start, c_end;
	BOOL ret;
	OVERLAY_WORK *ovwork;
	int size;
	int i;
	
	ret = GetOverlayAddress(id, &start, &end);
	if(ret == FALSE){
		return FALSE;
	}
	
	memory_area = GetMemoryAreaID(id);
	ovwork = GetWorkPointer(memory_area);
	size = GetWorkSize(memory_area);
	for(i = 0; i < size; i++){
		if(ovwork[i].loaded == TRUE){
			ret = GetOverlayAddress(ovwork[i].loaded_id, &c_start, &c_end);
			if(ret == TRUE){
				if((start >= c_start && start < c_end) || (end > c_start && end <= c_end)
						|| (start <= c_start && end >= c_end)){
					OS_Printf("オーバーレイ領域に重複が発生しました！\n");
					OS_Printf("展開済みID=%d、新規に展開しようとしたID=%d\n", 
						ovwork[i].loaded_id, id);
					GF_ASSERT(0);
					return FALSE;
				}
			}
		}
	}
	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   指定メモリエリアが参照するオーバーレイワークの先頭アドレスを取得する
 * @param   memory_area		メモリエリア(GF_OVERLAY_AREA_???)
 * @retval  指定メモリに対応したオーバーレイワークの先頭アドレス
 */
//--------------------------------------------------------------
static OVERLAY_WORK * GetWorkPointer(GF_OVERLAY_AREA memory_area)
{
	OVERLAY_WORK *ovwork;
	
	switch(memory_area){
	default:
		GF_ASSERT_MSG(0, "error:overlay:未定義のメモリエリア指定(%d)\n", memory_area);
		/* FALL THROUGH */
	case GF_OVERLAY_AREA_MAIN:
		ovwork = OverlaySys->main;
		break;
	case GF_OVERLAY_AREA_ITCM:
		ovwork = OverlaySys->itcm;
		break;
	case GF_OVERLAY_AREA_DTCM:
		ovwork = OverlaySys->dtcm;
		break;
	}
	return ovwork;
}

//--------------------------------------------------------------
/**
 * @brief	指定メモリエリアが参照するオーバーレイワークの数を取得する
 * @param   memory_area		メモリエリア(GF_OVERLAY_AREA_???)
 * retval	int	保持しているワークの数
 */
//--------------------------------------------------------------
static int GetWorkSize(GF_OVERLAY_AREA memory_area)
{
	switch (memory_area){
	case GF_OVERLAY_AREA_MAIN:
		return OverlaySys->main_num;
	case GF_OVERLAY_AREA_ITCM:
		return OverlaySys->itcm_num;
	case GF_OVERLAY_AREA_DTCM:
		return OverlaySys->dtcm_num;
	}
	GF_ASSERT_MSG(0, "error:overlay:未定義のメモリエリア指定(%d)\n", memory_area);
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief   指定オーバーレイIDの展開先メモリエリアを取得する
 * @param   id		オーバーレイID
 * @retval  展開先メモリエリア(GF_OVERLAY_AREA_???)
 */
//--------------------------------------------------------------
static GF_OVERLAY_AREA GetMemoryAreaID(const FSOverlayID id)
{
	FSOverlayInfo info;
	BOOL ret;
	u32 ram_address;
	
	ret = FS_LoadOverlayInfo(&info, MI_PROCESSOR_ARM9, id);
	GF_ASSERT(ret == TRUE);
	
	ram_address = (u32)FS_GetOverlayAddress(&info);
	if(ram_address <= HW_ITCM_END && ram_address >= HW_ITCM_IMAGE){
		return GF_OVERLAY_AREA_ITCM;
	}
	else if(ram_address <= HW_DTCM_END && ram_address >= HW_DTCM){
		return GF_OVERLAY_AREA_DTCM;
	}
	return GF_OVERLAY_AREA_MAIN;
}

//--------------------------------------------------------------
/**
 * @brief   指定オーバーレイIDのロード開始アドレスとロード終了アドレスを取得する
 *
 * @param   id			オーバーレイID
 * @param   start		取得したロード開始アドレス代入先
 * @param   end			取得したロード終了アドレス代入先
 *
 * @retval  TRUE:正常終了。　FALSE:失敗
 */
//--------------------------------------------------------------
static BOOL GetOverlayAddress(const FSOverlayID id, u32 *start, u32 *end)
{
	FSOverlayInfo info;
	BOOL ret;
	
	ret = FS_LoadOverlayInfo(&info, MI_PROCESSOR_ARM9, id);
	if(ret == FALSE){
		GF_ASSERT(0);
		return FALSE;
	}
	
	*start = (u32)FS_GetOverlayAddress(&info);
	*end = *start + FS_GetOverlayTotalSize(&info);
	
	return TRUE;
}

