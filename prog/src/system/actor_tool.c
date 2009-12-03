//==============================================================================
/**
 * @file	actor_tool.c
 * @brief	アクター操作用ツール
 * @author	matsuda
 * @date	2008.12.18(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/actor_tool.h"


//==============================================================================
//
//	
//
//==============================================================================
#if 0
void ACTORTOOL_Init(int heap_id, const GFL_CLSYS_INIT *clsysinit, const GFL_DISP_VRAM *dispvram, const GFL_OBJGRP_INIT_PARAM *objparam)
{
	GFL_CLACT_Init(clsysinit, heap_id);
	GFL_OBJGRP_sysStart(heap_id, dispvram, objparam);
}
#endif



#if 1
//==============================================================================
//
//	パレットスロット管理
//
//==============================================================================
//==============================================================================
//	定数定義
//==============================================================================
///パレット登録最大本数
#define PLTT_NUM_MAX		(16)
///スロットの空きを示す値
#define PLTT_SLOT_FREE		(0xff)
///VRAMTBL最大数
#define VRAM_TBL_MAX		(2)


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct _PLTTSLOT_SYSTEM{
	u32 entry_index;	//GFL_OBJGRP_RegisterPlttで取得した登録index
	u8 pltt_num_max[2];	///<管理するパレット最大登録本数
	u8 slot[VRAM_TBL_MAX][16];
}PLTTSLOT_SYSTEM;


//==============================================================================
//	マクロ
//==============================================================================
///GFL_VRAM_???をVRAMTBLに変換するマクロ
#define VRAMTBL(draw_type)		((draw_type == CLSYS_DRAW_MAIN) ? 0 : 1)


//--------------------------------------------------------------
/**
 * @brief   パレットスロット管理システム作成
 *
 * @param   heap_id				ヒープID
 * @param   main_num			パレット管理本数:メイン画面
 * @param   sub_num				パレット管理本数:メイン画面
 *
 * @retval  パレットスロットへのポインタ
 */
//--------------------------------------------------------------
PLTTSLOT_SYS_PTR PLTTSLOT_Init(HEAPID heap_id, int main_num, int sub_num)
{
	PLTTSLOT_SYS_PTR pssp;
	int tbl, i;
	
	GF_ASSERT(main_num <= PLTT_NUM_MAX);
	GF_ASSERT(sub_num <= PLTT_NUM_MAX);
	
	pssp = GFL_HEAP_AllocClearMemory(heap_id, sizeof(PLTTSLOT_SYSTEM));
	pssp->pltt_num_max[0] = main_num;
	pssp->pltt_num_max[1] = sub_num;
	for(tbl = 0; tbl < VRAM_TBL_MAX; tbl++){
		for(i = 0; i < PLTT_NUM_MAX; i++){
			pssp->slot[tbl][i] = PLTT_SLOT_FREE;
		}
	}
	
	return pssp;
}

//--------------------------------------------------------------
/**
 * @brief   パレットスロット管理システム削除
 *
 * @param   pssp		パレットスロットへのポインタ
 */
//--------------------------------------------------------------
void PLTTSLOT_Exit(PLTTSLOT_SYS_PTR pssp)
{
	GFL_HEAP_FreeMemory(pssp);
}

//--------------------------------------------------------------
/**
 * @brief   パレット登録エリアを取得
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   pltt_num		パレット本数
 * @param   draw_type		VRAMタイプ
 *
 * @retval  パレット番号
 */
//--------------------------------------------------------------
static u32 PLTTSLOT_Get(PLTTSLOT_SYS_PTR pssp, int pltt_num, CLSYS_DRAW_TYPE draw_type)
{
	int i, s, tbl;
	
	tbl = VRAMTBL(draw_type);
	for(i = 0; i < pssp->pltt_num_max[tbl]; i++){
		if(pssp->slot[tbl][i] == PLTT_SLOT_FREE){
			for(s = i; s < i+pltt_num; s++){
				if(pssp->slot[tbl][s] != PLTT_SLOT_FREE){
					i = s;
					break;	//途中に空きスロットではない領域があった
				}
			}
			if(s == i+pltt_num){
				//確保成功
				for(s = i; s < i+pltt_num; s++){
					pssp->slot[tbl][s] = i;
				}
				return i;
			}
		}
	}
	
	GF_ASSERT(0);	//パレットに空きが無い
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief   パレット登録エリアを解放
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   pal_no			パレット番号(PLTTSLOT_Get関数の戻り値)
 * @param   draw_type		VRAMタイプ
 */
//--------------------------------------------------------------
static void PLTTSLOT_Free(PLTTSLOT_SYS_PTR pssp, u32 pal_no, CLSYS_DRAW_TYPE draw_type)
{
	int i, tbl;
	
	tbl = VRAMTBL(draw_type);
	for(i = pal_no; pssp->slot[tbl][i] == pal_no; i++){
		pssp->slot[tbl][i] = PLTT_SLOT_FREE;
	}
}

//--------------------------------------------------------------
/**
 * @brief   パレットリソースの転送(と領域確保)
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   handle			アーカイブハンドル
 * @param   data_id			実データへのデータIndex
 * @param   pltt_num		転送するパレット本数
 * @param   draw_type		CLSYS_DRAW_MAIN or CLSYS_DRAW_SUB
 * @param   heap_id			テンポラリで使用するヒープID
 *
 * @retval  登録INDEX(パレット番号ではありません。削除時に必要になります)
 */
//--------------------------------------------------------------
u32 PLTTSLOT_ResourceSet(PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id)
{
	int pal_no, index;
	
	pal_no = PLTTSLOT_Get(pssp, pltt_num, draw_type);
	index = GFL_CLGRP_PLTT_RegisterEx(
		handle, data_id, draw_type, pal_no * 0x20, 0, pltt_num, heap_id);
	return index;
}

//--------------------------------------------------------------
/**
 * @brief   圧縮パレットリソースの転送(と領域確保) ※-pcmオプションがついているパレットが対象
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   handle			アーカイブハンドル
 * @param   data_id			実データへのデータIndex
 * @param   pltt_num		転送するパレット本数
 * @param   draw_type		CLSYS_DRAW_MAIN or CLSYS_DRAW_SUB
 * @param   heap_id			テンポラリで使用するヒープID
 *
 * @retval  登録INDEX(パレット番号ではありません。削除時に必要になります)
 */
//--------------------------------------------------------------
u32 PLTTSLOT_ResourceCompSet(PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id)
{
	int pal_no, index;

#ifdef PM_DEBUG
  {//圧縮パレット登録に本数指定が無い為、pltt_numと展開本数が一致しているか調べる
    BOOL cmpFlag;
    NNSG2dPaletteCompressInfo*  cmpData;
    NNSG2dPaletteData*  palData;
    void* loadPtr;
    
    loadPtr = GFL_ARC_LoadDataAllocByHandle(handle, data_id, GFL_HEAP_LOWID(heap_id));
    
    cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( loadPtr, &cmpData );
    GF_ASSERT(cmpFlag); //-pcmオプションがついたパレットではない
    
    if( NNS_G2dGetUnpackedPaletteData( loadPtr, &palData ) ){
      //展開サイズとpltt_numが不一致。
      //現状GFL_CLGRP_PLTT_RegisterCompには本数指定が無い為、このASSERTを入れておく
      OS_TPrintf("pcm palData->szByte = 0x%x, pltt_num = 0x%x\n", palData->szByte, pltt_num*0x20);
      GF_ASSERT(palData->szByte == pltt_num*0x20);
    }
    else{
      GF_ASSERT("unpacked faile");
    }
    
    GFL_HEAP_FreeMemory(loadPtr);
  }
#endif

	pal_no = PLTTSLOT_Get(pssp, pltt_num, draw_type);
	index = GFL_CLGRP_PLTT_RegisterComp( handle, data_id, draw_type, pal_no * 0x20, heap_id );
	return index;
}

//--------------------------------------------------------------
/**
 * @brief   パレットリソースの転送(と領域確保) & パレットフェードバッファへも転送
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   handle			アーカイブハンドル
 * @param   data_id			実データへのデータIndex
 * @param   pltt_num		転送するパレット本数
 * @param   draw_type		CLSYS_DRAW_MAIN or CLSYS_DRAW_SUB
 * @param   heap_id			テンポラリで使用するヒープID
 *
 * @retval  登録INDEX(パレット番号ではありません。削除時に必要になります)
 */
//--------------------------------------------------------------
u32 PLTTSLOT_ResourceSet_PalAnm(PALETTE_FADE_PTR pfd, PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id)
{
  u32 index, pal_no;
  FADEREQ fade_req;
  
  fade_req = (draw_type == CLSYS_DRAW_MAIN) ? FADE_MAIN_OBJ : FADE_SUB_OBJ;
  
  index = PLTTSLOT_ResourceSet(pssp, handle, data_id, draw_type, pltt_num, heap_id);
  pal_no = PLTTSLOT_GetPalNo(pssp, index, draw_type);
  PaletteWorkSet_VramCopy(pfd, fade_req, pal_no * 16, pltt_num * 0x20);
  return index;
}

//--------------------------------------------------------------
/**
 * @brief   圧縮パレットリソースの転送(と領域確保) ※-pcmオプションがついているパレットが対象
 *           & パレットフェードバッファへも転送
 *
 * @param   pssp			パレットスロットへのポインタ
 * @param   handle			アーカイブハンドル
 * @param   data_id			実データへのデータIndex
 * @param   pltt_num		転送するパレット本数
 * @param   draw_type		CLSYS_DRAW_MAIN or CLSYS_DRAW_SUB
 * @param   heap_id			テンポラリで使用するヒープID
 *
 * @retval  登録INDEX(パレット番号ではありません。削除時に必要になります)
 */
//--------------------------------------------------------------
u32 PLTTSLOT_ResourceCompSet_PalAnm(PALETTE_FADE_PTR pfd, PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id)
{
  u32 index, pal_no;
  FADEREQ fade_req;
  
  fade_req = (draw_type == CLSYS_DRAW_MAIN) ? FADE_MAIN_OBJ : FADE_SUB_OBJ;
  
  index = PLTTSLOT_ResourceCompSet(pssp, handle, data_id, draw_type, pltt_num, heap_id);
  pal_no = PLTTSLOT_GetPalNo(pssp, index, draw_type);
  PaletteWorkSet_VramCopy(pfd, fade_req, pal_no * 16, pltt_num * 0x20);
  return index;
}

//--------------------------------------------------------------
/**
 * @brief   エントリー番号からパレット番号を取得
 * @param   pssp		パレットスロットへのポインタ
 * @param   index		登録INDEX (PLTTSLOT_ResourceSet関数の戻り値)
 * @retval	パレット番号
 */
//--------------------------------------------------------------
u32 PLTTSLOT_GetPalNo(PLTTSLOT_SYS_PTR pssp, u32 index, CLSYS_DRAW_TYPE draw_type)
{
	return GFL_CLGRP_PLTT_GetAddr(index, draw_type) / 0x20;
}

//--------------------------------------------------------------
/**
 * @brief   パレットリソースの解放(と領域解放)
 *
 * @param   pssp		パレットスロットへのポインタ
 * @param   index		登録INDEX (PLTTSLOT_ResourceSet関数の戻り値)
 */
//--------------------------------------------------------------
void PLTTSLOT_ResourceFree(PLTTSLOT_SYS_PTR pssp, u32 index, CLSYS_DRAW_TYPE draw_type)
{
	int pal_no;
	
	pal_no = PLTTSLOT_GetPalNo(pssp, index, draw_type);
	PLTTSLOT_Free(pssp, pal_no, draw_type);
	GFL_CLGRP_PLTT_Release(index);
}

#endif

