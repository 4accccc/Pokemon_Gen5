//==============================================================================
/**
 * @file	actor_tool.c
 * @brief	アクター操作用ツール
 * @author	matsuda
 * @date	2008.12.18(木)
 */
//==============================================================================
#include <gflib.h>


///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
	GX_VRAM_BG_16_F,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_128_C,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_01_AB,				//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G,			//テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

static const GFL_CLSYS_INIT clactIni = {
	0,0,	// メイン　サーフェースの左上座標
	0,1000,	// サブ　サーフェースの左上座標
	0,128,	// メイン画面OAM管理開始位置、管理数
	0,128,	// サブ画面OAM管理開始位置、管理数
	64,		// セルVram転送管理数
};

static const GFL_OBJGRP_INIT_PARAM objgrpIni = {
	64,			///< 登録できるキャラデータ数
	64,			///< 登録できるパレットデータ数
	64,			///< 登録できるセルアニメパターン数
	16,			///< 登録できるマルチセルアニメパターン数（※現状未対応）
};


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	u32 data_id;			///<登録データID
	u32 entry_index;		///<登録INDEX
	

//==============================================================================
//
//	
//
//==============================================================================
void ACTORTOOL_Init(int heap_id, const GFL_CLSYS_INIT *clsysinit, const GFL_DISP_VRAM *dispvram, const GFL_OBJGRP_INIT_PARAM *objparam)
{
	GFL_CLACT_Init(clsysinit, heap_id);
	GFL_OBJGRP_sysStart(heap_id, dispvram, objparam);
}



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
typedef struct{
	u32 entry_index;	//GFL_OBJGRP_RegisterPlttで取得した登録index
	u8 pltt_num_max;	///<管理するパレット最大登録本数
	u8 slot[VRAM_TBL_MAX][16];
}PLTTSLOT_SYSTEM;


//==============================================================================
//	マクロ
//==============================================================================
///GFL_VRAM_???をVRAMTBLに変換するマクロ
#define VRAMTBL(vram_type)		((vram_type == GFL_VRAM_2D_MAIN) ? 0 : 1)


//--------------------------------------------------------------
/**
 * @brief   パレットスロット管理システム作成
 *
 * @param   heap_id				ヒープID
 * @param   pltt_num_max		パレット管理本数
 *
 * @retval  パレットスロットへのポインタ
 */
//--------------------------------------------------------------
PLTTSLOT_SYS_PTR PLTTSLOT_Init(int heap_id, int pltt_num_max)
{
	PLTTSLOT_SYS_PTR pssp;
	int i;
	
	GF_ASSERT(pltt_num_max <= PLTT_NUM_MAX);
	
	pssp = GFL_HEAP_AllocClearMemory(heap_id, sizeof(PLTTSLOT_SYSTEM));
	pssp->pltt_num_max = pltt_num_max;
	for(tbl = 0; tbl < VRAM_TBL_MAX; tbl++){
		for(i = 0; i < PLTT_NUM_MAX; i++){
			pssp->slot[tb][i] = PLTT_SLOT_FREE;
		}
	}
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
 * @param   vram_type		VRAMタイプ
 *
 * @retval  パレット番号
 */
//--------------------------------------------------------------
static u32 PLTTSLOT_Get(PLTTSLOT_SYS_PTR pssp, int pltt_num, GFL_VRAM_TYPE vram_type)
{
	int i, s, tbl;
	
	tbl = VRAMTBL(vram_type);
	for(i = 0; i < pssp->pltt_num_max; i++){
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
					pspp->slot[tbl][s] = i;
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
 * @param   vram_type		VRAMタイプ
 */
//--------------------------------------------------------------
static void PLTTSLOT_Free(PLTTSLOT_SYS_PTR pssp, u32 pal_no, GFL_VRAM_TYPE vram_type)
{
	int i, tbl;
	
	tbl = VRAMTBL(vram_type);
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
 * @param   vram_type		GFL_VRAM_2D_MAIN or GFL_VRAM_2D_SUB (※GFL_VRAM_2D_BOTHは非対応)
 * @param   heap_id			テンポラリで使用するヒープID
 *
 * @retval  登録INDEX(パレット番号ではありません。削除時に必要になります)
 */
//--------------------------------------------------------------
u32 PLTTSLOT_ResourceSet(PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, int pltt_num, GFL_VRAM_TYPE vram_type, int heap_id)
{
	int pal_no;
	
	pal_no = PLTTSLOT_Get(pssp, pltt_num);
	index = GFL_OBJGRP_RegisterPlttEx(
		handle, data_id, vram_type, pal_no * 0x20, 0, pltt_num, heap_id);
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
u32 PLTTSLOT_GetPalNo(PLTTSLOT_SYS_PTR pssp, u32 index, GFL_VRAM_TYPE vram_type)
{
	int offset, nitro_vramtype;
	NNSG2dImagePaletteProxy proxy;
	
	nitro_vramtype = ((vram_type == GFL_VRAM_2D_MAIN) ? NNS_G2D_VRAM_TYPE_2DMAIN : NNS_G2D_VRAM_TYPE_2DSUB);
	
	GFL_OBJGRP_GetPlttProxy(index, &proxy);
	offset =  NNS_G2dGetImagePaletteLocation(&proxy, nitro_vramtype);
	GF_ASSERT(offset != NNS_G2D_VRAM_ADDR_NOT_INITIALIZED);
	
	return offset / 0x20;
}

//--------------------------------------------------------------
/**
 * @brief   パレットリソースの解放(と領域解放)
 *
 * @param   pssp		パレットスロットへのポインタ
 * @param   index		登録INDEX (PLTTSLOT_ResourceSet関数の戻り値)
 */
//--------------------------------------------------------------
void PLTTSLOT_ResourceFree(PLTTSLOT_SYS_PTR pssp, u32 index, GFL_VRAM_TYPE vram_type)
{
	int pal_no;
	
	pal_no = PLTTSLOT_GetPalNo(pssp, index, vram_type);
	PLTTSLOT_Free(pssp, pal_no, vram_type);
}


