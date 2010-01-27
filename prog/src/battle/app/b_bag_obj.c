//============================================================================================
/**
 * @file	b_bag_obj.c
 * @brief	戦闘用バッグ画面 OBJ関連
 * @author	Hiroyuki Nakamura
 * @date	09.08.26
 */
//============================================================================================
#include <gflib.h>
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/msgdata.h"
#include "system/numfont.h"
#include "system/wordset.h"
*/
//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "battle/battle_common.h"
#include "battle/fight_tool.h"
#include "itemtool/item.h"
*/
#include "arc_def.h"
#include "battle/battgra_wb.naix"

#include "b_app_tool.h"

#include "b_bag.h"
#include "b_bag_main.h"
#include "b_bag_obj.h"
#include "b_bag_item.h"
#include "b_bag_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
/*
#define	CLACT_ID_COMMON		( 46263 )	// この画面で使用するセルアクターのID

// キャラリソースID
enum {
	CHR_ID_ITEM1 = CLACT_ID_COMMON,		// アイテムアイコン：１
	CHR_ID_ITEM2,						// アイテムアイコン：２
	CHR_ID_ITEM3,						// アイテムアイコン：３
	CHR_ID_ITEM4,						// アイテムアイコン：４
	CHR_ID_ITEM5,						// アイテムアイコン：５
	CHR_ID_ITEM6,						// アイテムアイコン：６
	CHR_ID_GETDEMO,						// 捕獲デモカーソル
	CHR_ID_CURSOR,						// カーソル

	CHR_ID_MAX = CHR_ID_CURSOR - CLACT_ID_COMMON + 1
};

// パレットリソースID
enum {
	PAL_ID_ITEM1 = CLACT_ID_COMMON,		// アイテムアイコン：１
	PAL_ID_ITEM2,						// アイテムアイコン：２
	PAL_ID_ITEM3,						// アイテムアイコン：３
	PAL_ID_ITEM4,						// アイテムアイコン：４
	PAL_ID_ITEM5,						// アイテムアイコン：５
	PAL_ID_ITEM6,						// アイテムアイコン：６
	PAL_ID_GETDEMO,						// 捕獲デモカーソル
	PAL_ID_CURSOR,						// カーソル

	PAL_ID_MAX = PAL_ID_CURSOR - CLACT_ID_COMMON + 1
};

// セルリソースID
enum {
	CEL_ID_ITEM = CLACT_ID_COMMON,		// アイテムアイコン
	CEL_ID_GETDEMO,						// 捕獲デモカーソル
	CEL_ID_CURSOR,						// カーソル

	CEL_ID_MAX = CEL_ID_CURSOR - CLACT_ID_COMMON + 1
};

// セルアニメリソースID
enum {
	ANM_ID_ITEM = CLACT_ID_COMMON,		// アイテムアイコン
	ANM_ID_GETDEMO,						// 捕獲デモカーソル
	ANM_ID_CURSOR,						// カーソル

	ANM_ID_MAX = ANM_ID_CURSOR - CLACT_ID_COMMON + 1
};
*/

#define	CURSOR_CLA_MAX		( 5 )		// カーソルのOBJ数
#define	FINGER_CLA_MAX		( 1 )		// 指カーソルのOBJ数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void BBAG_ClactResManInit( BBAG_WORK * wk );
static void BBAG_ClactItemLoad( BBAG_WORK * wk );
static void BBAG_ItemIconCharChg( BBAG_WORK * wk, u16 item, u32 chrResID, u32 palResID );
static void BBAG_ItemIconPlttChg( BBAG_WORK * wk, u16 item, u32 palResID );
static void BBAG_ClactGetDemoLoad( BBAG_WORK * wk );
static void BBAG_ClactAddAll( BBAG_WORK * wk );
static void BBAG_Page1ObjSet( BBAG_WORK * wk );
static void BBAG_Page2ObjSet( BBAG_WORK * wk );
static void BBAG_Page3ObjSet( BBAG_WORK * wk );
static void BBAG_ClactCursorAdd( BBAG_WORK * wk );
static void BBAG_CursorDel( BBAG_WORK * wk );
//static void BBAG_ClactGetDemoCursorAdd( BBAG_WORK * wk );
//static void BBAG_GetDemoCursorDel( BBAG_WORK * wk );
static void CostResLoad( BBAG_WORK * wk );
static void CursorResLoad( BBAG_WORK * wk );

static void SetGetDemoCursorCallBack( BBAG_WORK * wk, u32 anm );

static void CostObjPut( BBAG_WORK * wk, u16 idx, u8 ene, u8 res_ene, const GFL_CLACTPOS * pos );


//============================================================================================
//	グローバル変数
//============================================================================================
// ページ１のポケモンアイコンの座標
//static const int P1_ItemIconPos[2] = { 24, 178 };
static const GFL_CLACTPOS P1_ItemIconPos = { 36, 180 };

// ページ２のアイテムアイコンの座標
//static const int P2_ItemIconPos[][2] =
static const GFL_CLACTPOS P2_ItemIconPos[] =
{
	// アイテムアイコン
	{ 36,  45 }, { 164,  45 },
	{ 36,  93 }, { 164,  93 },
	{ 36, 141 }, { 164, 141 },
	// コスト
	{ 48+4,  41 }, { 176+4,  41 },
	{ 48+4,  89 }, { 176+4,  89 },
	{ 48+4, 137 }, { 176+4, 137 },
	// エネルギー
	{ 88+4, 172 },
};

// ページ３のポケモンアイコンの座標
static const GFL_CLACTPOS P3_ItemIconPos[] =
{
	{  40, 44 },
	{ 168, 40 },
};

// セルアクターのデータ
static const u32 ClactDat[][3] =
{	// char, pal, cell
	{ BBAG_CHRRES_ITEM1, BBAG_PALRES_ITEM1, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM2, BBAG_PALRES_ITEM2, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM3, BBAG_PALRES_ITEM3, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM4, BBAG_PALRES_ITEM4, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM5, BBAG_PALRES_ITEM5, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM6, BBAG_PALRES_ITEM6, BBAG_CELRES_ITEM },
	{ BBAG_CHRRES_ITEM7, BBAG_PALRES_ITEM7, BBAG_CELRES_ITEM },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },
	{ BBAG_CHRRES_COST, BBAG_PALRES_COST, BBAG_CELRES_COST },

	{ BBAG_CHRRES_DEMO, BBAG_PALRES_DEMO, BBAG_CELRES_DEMO },
};



//--------------------------------------------------------------------------------------------
/**
 * 戦闘用バッグOBJ初期化
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_ObjInit( BBAG_WORK * wk )
{
	BBAG_ClactResManInit( wk );
	BBAG_ClactItemLoad( wk );
	BBAG_ClactGetDemoLoad( wk );
	CostResLoad( wk );
	CursorResLoad( wk );

	BBAG_ClactAddAll( wk );
	BBAG_ClactCursorAdd( wk );
//	BBAG_ClactGetDemoCursorAdd( wk );

	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
/*↑[GS_CONVERT_TAG]*/
}

//--------------------------------------------------------------------------------------------
/**
 * リソースマネージャー初期化
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactResManInit( BBAG_WORK * wk )
{
	u32	i;

	for( i=0; i<BBAG_CHRRES_MAX; i++ ){
		wk->chrRes[i] = 0xffffffff;
	}
	for( i=0; i<BBAG_PALRES_MAX; i++ ){
		wk->palRes[i] = 0xffffffff;
	}
	for( i=0; i<BBAG_CELRES_MAX; i++ ){
		wk->celRes[i] = 0xffffffff;
	}

/*
	TCATS_RESOURCE_NUM_LIST	crnl = { CHR_ID_MAX, PAL_ID_MAX, CEL_ID_MAX, ANM_ID_MAX, 0, 0 };
	CATS_SYS_PTR	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	wk->crp = CATS_ResourceCreate( csp );
	CATS_ClactSetInit( csp, wk->crp, BBAG_CA_MAX+CURSOR_CLA_MAX+FINGER_CLA_MAX );
	CATS_ResourceManagerInit( csp, wk->crp, &crnl );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンのグラフィックロード
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactItemLoad( BBAG_WORK * wk )
{
  ARCHANDLE * ah;
	u32	i;
	
	ah = GFL_ARC_OpenDataHandle( ITEM_GetIconArcID(), wk->dat->heap );

	for( i=0; i<7; i++ ){
	  wk->chrRes[BBAG_CHRRES_ITEM1+i] = GFL_CLGRP_CGR_Register(
																				ah, ITEM_GetIndex(1,ITEM_GET_ICON_CGX),
																				FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	  wk->palRes[BBAG_PALRES_ITEM1+i] = GFL_CLGRP_PLTT_Register(
																				ah, ITEM_GetIndex(1,ITEM_GET_ICON_PAL),
																				CLSYS_DRAW_SUB, i*32, wk->dat->heap );
	}
  wk->celRes[BBAG_CELRES_ITEM] = GFL_CLGRP_CELLANIM_Register(
																	ah, ITEM_GetIconCell(),  ITEM_GetIconCellAnm(), wk->dat->heap );

  GFL_ARC_CloseDataHandle( ah );

/*
	CATS_SYS_PTR	csp;
	u32	i;
	ARCHANDLE* hdl;
	
	hdl = GFL_ARC_OpenDataHandle( ARC_ITEMICON,  wk->dat->heap ); 
	
	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	for( i=0; i<6; i++ ){
		// キャラ
		CATS_LoadResourceCharArcH(
			csp, wk->crp, hdl,
			GetItemIndex(1,ITEM_GET_ICON_CGX), 0, NNS_G2D_VRAM_TYPE_2DSUB, CHR_ID_ITEM1+i );
		// パレット
		CATS_LoadResourcePlttWorkArcH(
			wk->pfd, FADE_SUB_OBJ, csp, wk->crp, hdl,
			GetItemIndex(1,ITEM_GET_ICON_PAL), 0, 1, NNS_G2D_VRAM_TYPE_2DSUB, PAL_ID_ITEM1+i );
	}
	// セル
	CATS_LoadResourceCellArcH( csp, wk->crp, hdl, ItemIconCellGet(), 0, CEL_ID_ITEM );
	// セルアニメ
	CATS_LoadResourceCellAnmArcH( csp, wk->crp, hdl, ItemIconCAnmGet(), 0, ANM_ID_ITEM );
	
	GFL_ARC_CloseDataHandle( hdl );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンキャラ切り替え
 *
 * @param	wk		ワーク
 * @param	item	アイテム番号
 * @param	res_id	キャラリソースID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ItemIconCharChg( BBAG_WORK * wk, u16 item, u32 chrResID, u32 palResID )
{
	NNSG2dCharacterData * dat;
	void * buf;
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ITEM_GetIconArcID(), wk->dat->heap );

	// キャラ
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter(
					ah, ITEM_GetIndex(item,ITEM_GET_ICON_CGX), FALSE, &dat, wk->dat->heap );
	GFL_CLGRP_CGR_Replace( wk->chrRes[chrResID], dat );
	GFL_HEAP_FreeMemory( buf );

	// パレット
	PaletteWorkSet_ArcHandle(
					wk->pfd, ah, ITEM_GetIndex(item,ITEM_GET_ICON_PAL),
					wk->dat->heap, FADE_SUB_OBJ, 0x20, palResID * 16 );

  GFL_ARC_CloseDataHandle( ah );

/*
	CATS_SYS_PTR	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	CATS_ChangeResourceCharArc(
		csp, wk->crp, ARC_ITEMICON,
		GetItemIndex(item,ITEM_GET_ICON_CGX), 0, res_id );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコンパレット切り替え
 *
 * @param	wk		ワーク
 * @param	item	アイテム番号
 * @param	pos		位置
 * @param	res_id	パレットリソースID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ItemIconPlttChg( BBAG_WORK * wk, u16 item, u32 palResID )
{
	GFL_ARC_UTIL_TransVramPalette(
		ITEM_GetIconArcID(),
		ITEM_GetIndex(item,ITEM_GET_ICON_PAL),
		PALTYPE_SUB_OBJ, palResID * 0x20, 0x20, wk->dat->heap );
/*
	PaletteWorkSet_Arc(
		wk->pfd, ARC_ITEMICON,
		GetItemIndex(item,ITEM_GET_ICON_PAL), wk->dat->heap,FADE_SUB_OBJ, 0x20, pos*16 );
*/
}

static void CostResLoad( BBAG_WORK * wk )
{
  ARCHANDLE * ah;
	u32	i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_B_BAG_GRA, wk->dat->heap );

  wk->chrRes[BBAG_CHRRES_COST] = GFL_CLGRP_CGR_Register(
																		ah, NARC_b_bag_gra_test_cost_lz_NCGR,
																		TRUE, CLSYS_DRAW_SUB, wk->dat->heap );
  wk->palRes[BBAG_PALRES_COST] = GFL_CLGRP_PLTT_Register(
																		ah, NARC_b_bag_gra_test_cost_NCLR,
																		CLSYS_DRAW_SUB, BBAG_PALRES_COST*32, wk->dat->heap );
  wk->celRes[BBAG_CELRES_COST] = GFL_CLGRP_CELLANIM_Register(
																	ah, NARC_b_bag_gra_test_cost_NCER,
																	NARC_b_bag_gra_test_cost_NANR, wk->dat->heap );
	// パレット
	PaletteWorkSet_ArcHandle(
					wk->pfd, ah, NARC_b_bag_gra_test_cost_NCLR,
					wk->dat->heap, FADE_SUB_OBJ, 0x20, BBAG_PALRES_COST * 16 );

  GFL_ARC_CloseDataHandle( ah );
}

static void CursorResLoad( BBAG_WORK * wk )
{
  ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, wk->dat->heap );

	// キャラ
	wk->chrRes[BBAG_CHRRES_CURSOR] = GFL_CLGRP_CGR_Register(
																		ah, NARC_battgra_wb_battle_w_cursor_NCGR,
																		FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	// パレット
  wk->palRes[BBAG_PALRES_CURSOR] = GFL_CLGRP_PLTT_RegisterEx(
																		ah, NARC_battgra_wb_battle_w_obj_NCLR,
																		CLSYS_DRAW_SUB, BBAG_PALRES_CURSOR*0x20, 1, 1, wk->dat->heap );
	// セル・アニメ
  wk->celRes[BBAG_CELRES_CURSOR] = GFL_CLGRP_CELLANIM_Register(
																		ah,
																		NARC_battgra_wb_battle_w_cursor_NCER,
																		NARC_battgra_wb_battle_w_cursor_NANR,
																		wk->dat->heap );

	// パレットフェードに設定
	{
		NNSG2dPaletteData * dat;
		void * buf;

		buf = GFL_ARCHDL_UTIL_LoadPalette( ah, NARC_battgra_wb_battle_w_obj_NCLR, &dat, wk->dat->heap );
		PaletteWorkSet(
			wk->pfd, &(((u16*)(dat->pRawData))[16]), FADE_SUB_OBJ, BBAG_PALRES_CURSOR*16, 0x20 );
	
		GFL_HEAP_FreeMemory( buf );
	}

  GFL_ARC_CloseDataHandle( ah );
}


//--------------------------------------------------------------------------------------------
/**
 * 捕獲デモカーソルのグラフィックロード
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactGetDemoLoad( BBAG_WORK * wk )
{
  ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, wk->dat->heap );

	// キャラ
	wk->chrRes[BBAG_CHRRES_DEMO] = GFL_CLGRP_CGR_Register(
																	ah, NARC_battgra_wb_finger_cursor_NCGR,
																	FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
/*
	// パレット
  wk->palRes[BBAG_PALRES_DEMO] = GFL_CLGRP_PLTT_RegisterEx(
																		ah, NARC_battgra_wb_finger_cursor_NCLR,
																		CLSYS_DRAW_SUB, BBAG_PALRES_DEMO*0x20, 0, 1, wk->dat->heap );
*/
  wk->palRes[BBAG_PALRES_DEMO] = GFL_CLGRP_PLTT_Register(
																		ah, NARC_battgra_wb_finger_cursor_NCLR,
																		CLSYS_DRAW_SUB, BBAG_PALRES_DEMO*0x20, wk->dat->heap );
	// セル・アニメ
  wk->celRes[BBAG_CELRES_DEMO] = GFL_CLGRP_CELLANIM_Register(
																		ah,
																		NARC_battgra_wb_finger_cursor_NCER,
																		NARC_battgra_wb_finger_cursor_NANR,
																		wk->dat->heap );

	// パレットフェードに設定
	{
		NNSG2dPaletteData * dat;
		void * buf;

		buf = GFL_ARCHDL_UTIL_LoadPalette( ah, NARC_battgra_wb_finger_cursor_NCLR, &dat, wk->dat->heap );
		PaletteWorkSet(
			wk->pfd, &(((u16*)(dat->pRawData))[0]), FADE_SUB_OBJ, BBAG_PALRES_DEMO*16, 0x20 );
	
		GFL_HEAP_FreeMemory( buf );
	}

  GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクター追加（個別）
 *
 * @param	wk		ワーク
 * @param	id		追加するセルアクターのID
 *
 * @return	セルアクターデータ
 */
//--------------------------------------------------------------------------------------------
static GFL_CLWK * BBAG_ClactAdd( BBAG_WORK * wk, const u32 * res )
{
	GFL_CLWK * clwk;
	GFL_CLWK_DATA	dat;

	dat.pos_x = 0;
	dat.pos_y = 0;
	dat.anmseq = 0;
	dat.softpri = 1;
	dat.bgpri = 1;

	clwk = GFL_CLACT_WK_Create(
						wk->clunit,
						wk->chrRes[res[0]],
						wk->palRes[res[1]],
						wk->celRes[res[2]],
						&dat, CLSYS_DRAW_SUB, wk->dat->heap );

//	GFL_CLACT_WK_SetPlttOffs( clwk , prm->palNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
//	GFL_CLACT_WK_SetAutoAnmFlag( plateWork->pokeIcon , TRUE );

	return clwk;

/*
    plateWork->pokeIconNcgRes = GFL_CLGRP_CGR_Register( arcHandle , 
        POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    
    GFL_ARC_CloseDataHandle(arcHandle);

    pltNum = POKEICON_GetPalNumGetByPPP( ppp );
    cellInitData.pos_x = PLIST_PLATE_POKE_POS_X;
    cellInitData.pos_y = PLIST_PLATE_POKE_POS_Y;
    cellInitData.anmseq = POKEICON_ANM_HPMAX;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    plateWork->pokeIcon = GFL_CLACT_WK_Create( plateWork->cellUnit ,
              plateWork->pokeIconNcgRes,
              work->cellRes[PCR_PLT_POKEICON],
              work->cellRes[PCR_ANM_POKEICON],
              &cellInitData ,PLIST_RENDER_MAIN , work->heapId );
    GFL_CLACT_WK_SetPlttOffs( plateWork->pokeIcon , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
    GFL_CLACT_WK_SetAutoAnmFlag( plateWork->pokeIcon , TRUE );
*/

/*
	TCATS_OBJECT_ADD_PARAM_S	prm;
	CATS_SYS_PTR	csp;

	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	prm.x = 0;
	prm.y = 0;
	prm.z = 0;

	prm.anm = 0;
	prm.pri = ClactDat[id][4];
	prm.pal = 0;
	prm.d_area = NNS_G2D_VRAM_TYPE_2DSUB;

	prm.id[0] = ClactDat[id][0];
	prm.id[1] = ClactDat[id][1];
	prm.id[2] = ClactDat[id][2];
	prm.id[3] = ClactDat[id][3];
	
	prm.bg_pri = 1;
	prm.vram_trans = 0;

	return CATS_ObjectAdd_S( csp, wk->crp, &prm );
*/
}

static GFL_CLWK * BBAG_ClactAddDemoCursor( BBAG_WORK * wk, const u32 * res )
{
/*
typedef struct {
	GFL_CLWK_DATA clwkdata;		// 基本データ
	
	s16	affinepos_x;		// アフィンｘ座標
	s16 affinepos_y;		// アフィンｙ座標
	fx32 scale_x;			// 拡大ｘ値
	fx32 scale_y;			// 拡大ｙ値
	u16	rotation;			// 回転角度(0～0xffff 0xffffが360度)
	u16	affine_type;		// 上書きアフィンタイプ（CLSYS_AFFINETYPE）
} GFL_CLWK_AFFINEDATA;
*/
	GFL_CLWK * clwk;
	GFL_CLWK_AFFINEDATA	dat;

	dat.clwkdata.pos_x   = 0;
	dat.clwkdata.pos_y   = 0;
	dat.clwkdata.anmseq  = 0;
	dat.clwkdata.softpri = 1;
	dat.clwkdata.bgpri   = 1;

	dat.affinepos_x = 0;
	dat.affinepos_y = 0;
	dat.scale_x = FX32_ONE;
	dat.scale_y = FX32_ONE;
	dat.rotation = 0;
	dat.affine_type = CLSYS_AFFINETYPE_DOUBLE;

	clwk = GFL_CLACT_WK_CreateAffine(
						wk->clunit,
						wk->chrRes[res[0]],
						wk->palRes[res[1]],
						wk->celRes[res[2]],
						&dat, CLSYS_DRAW_SUB, wk->dat->heap );


//	GFL_CLACT_WK_SetPlttOffs( clwk , prm->palNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
//	GFL_CLACT_WK_SetAutoAnmFlag( plateWork->pokeIcon , TRUE );

	return clwk;
}



//--------------------------------------------------------------------------------------------
/**
 * セルアクター追加（全て）
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactAddAll( BBAG_WORK * wk )
{
	u32	i;

	wk->clunit = GFL_CLACT_UNIT_Create( BBAG_CA_MAX+BAPPTOOL_CURSOR_MAX, 0, wk->dat->heap );
//	wk->clunit = GFL_CLACT_UNIT_Create( BBAG_CA_MAX, 0, wk->dat->heap );

	for( i=0; i<BBAG_CA_GETDEMO; i++ ){
		wk->clwk[i] = BBAG_ClactAdd( wk, ClactDat[i] );
	}

	// 捕獲デモカーソル
	wk->clwk[BBAG_CA_GETDEMO] = BBAG_ClactAddDemoCursor( wk, ClactDat[BBAG_CA_GETDEMO] );
}

//--------------------------------------------------------------------------------------------
/**
 * 戦闘用バッグOBJ削除
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_ObjFree( BBAG_WORK * wk )
{
	u32	i;

	for( i=0; i<BBAG_CA_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
	BBAG_CursorDel( wk );

	for( i=0; i<BBAG_CHRRES_MAX; i++ ){
		if( wk->chrRes[i] != 0xffffffff ){
			GFL_CLGRP_CGR_Release( wk->chrRes[i] );
		}
	}
	for( i=0; i<BBAG_PALRES_MAX; i++ ){
		if( wk->palRes[i] != 0xffffffff ){
	    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
		}
	}
	for( i=0; i<BBAG_CELRES_MAX; i++ ){
		if( wk->celRes[i] != 0xffffffff ){
	    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
		}
	}

	GFL_CLACT_UNIT_Delete( wk->clunit );

/*
	CATS_SYS_PTR	csp;
	u32	i;
	
	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	for( i=0; i<BBAG_CA_MAX; i++ ){
		CATS_ActorPointerDelete_S( wk->cap[i] );
	}

	BBAG_CursorDel( wk );
	BBAG_GetDemoCursorDel( wk );

	CATS_ResourceDestructor_S( csp, wk->crp );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * セルアクターを表示して座標変更
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactOn( GFL_CLWK * clwk, const GFL_CLACTPOS * pos )
{
	GFL_CLACT_WK_SetDrawEnable( clwk, TRUE );
	GFL_CLACT_WK_SetPos( clwk, pos, CLSYS_DRAW_SUB );
}

//--------------------------------------------------------------------------------------------
/**
 * ページごとにOBJをセット
 *
 * @param	wk		ワーク
 * @param	page	ページ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_PageObjSet( BBAG_WORK * wk, u32 page )
{
	u32	i;

	for( i=0; i<BBAG_CA_MAX; i++ ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
	}

	switch( page ){
	case BBAG_PAGE_POCKET:		// ポケット選択ページ
		BBAG_Page1ObjSet( wk );
		break;
	case BBAG_PAGE_MAIN:		// アイテム選択ページ
		BBAG_Page2ObjSet( wk );
		break;
	case BBAG_PAGE_ITEM:		// アイテム使用ページ
		BBAG_Page3ObjSet( wk );
		break;
	}
}

// 捕獲デモのカーソル表示座標
#define	P1_GETDEMO_CURSOR_X		( 192 )
#define	P1_GETDEMO_CURSOR_Y		( 24 )
#define	P2_GETDEMO_CURSOR_X		( 64 )
#define	P2_GETDEMO_CURSOR_Y		( 16 )
#define	P3_GETDEMO_CURSOR_X		( 104 )
#define	P3_GETDEMO_CURSOR_Y		( 152 )

static const GFL_CLACTPOS P1_DemoCursorPos = { P1_GETDEMO_CURSOR_X, P1_GETDEMO_CURSOR_Y };
static const GFL_CLACTPOS P2_DemoCursorPos = { P2_GETDEMO_CURSOR_X, P2_GETDEMO_CURSOR_Y };
static const GFL_CLACTPOS P3_DemoCursorPos = { P3_GETDEMO_CURSOR_X, P3_GETDEMO_CURSOR_Y };

//--------------------------------------------------------------------------------------------
/**
 * ポケット選択ページのOBJをセット
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_Page1ObjSet( BBAG_WORK * wk )
{
	u16	item;

	if( wk->used_item != ITEM_DUMMY_DATA ){
		BBAG_ItemIconCharChg( wk, wk->used_item, BBAG_CHRRES_ITEM7, BBAG_PALRES_ITEM7 );
//		BBAG_ItemIconPlttChg( wk, wk->used_item, 0, PAL_ID_ITEM1 );
		BBAG_ClactOn( wk->clwk[BBAG_CA_ITEM7], &P1_ItemIconPos );
	}

	if( wk->dat->mode == BBAG_MODE_GETDEMO ){
		BBAG_ClactOn( wk->clwk[BBAG_CA_GETDEMO], &P1_DemoCursorPos );
		BBAG_ChangeGetDemoCursorAnm( wk, 0 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム選択ページのOBJをセット
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_Page2ObjSet( BBAG_WORK * wk )
{
	u32	i;
	u16	item;

	for( i=0; i<6; i++ ){
		item = BattleBag_PosItemCheck( wk, i );
		if( item == 0 ){ continue; }
		BBAG_ItemIconCharChg( wk, item, BBAG_CHRRES_ITEM1+i, BBAG_PALRES_ITEM1+i );
		BBAG_ItemIconPlttChg( wk, item, BBAG_PALRES_ITEM1+i );
		BBAG_ClactOn( wk->clwk[BBAG_CA_ITEM1+i], &P2_ItemIconPos[i] );

		if( wk->dat->mode == BBAG_MODE_SHOOTER ){
			CostObjPut( wk, BBAG_CA_COST1_NUM+i*8, BBAGITEM_GetCost(item), 0, &P2_ItemIconPos[6+i] );
//			BBAG_ClactOn( wk->clwk[BBAG_CA_COST1+i], &P2_ItemIconPos[6+i] );
//			GFL_CLACT_WK_SetAnmSeq( wk->clwk[BBAG_CA_COST1+i], BBAGITEM_GetCost(item)-1 );
		}
	}

	if( wk->dat->mode == BBAG_MODE_SHOOTER ){
		CostObjPut( wk, BBAG_CA_ENERGIE_NUM, wk->dat->energy, wk->dat->reserved_energy, &P2_ItemIconPos[6+i] );
//		BBAG_ClactOn( wk->clwk[BBAG_CA_ENERGIE], &P2_ItemIconPos[6+i] );
//		GFL_CLACT_WK_SetAnmSeq( wk->clwk[BBAG_CA_ENERGIE], 13 );
	}

	if( wk->dat->mode == BBAG_MODE_GETDEMO ){
		BBAG_ClactOn( wk->clwk[BBAG_CA_GETDEMO], &P2_DemoCursorPos );
		BBAG_ChangeGetDemoCursorAnm( wk, 0 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用ページのOBJをセット
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_Page3ObjSet( BBAG_WORK * wk )
{
	u16	item;

	item = BattleBag_PosItemCheck( wk, wk->dat->item_pos[wk->poke_id] );
	BBAG_ItemIconCharChg( wk, item, BBAG_CHRRES_ITEM7, BBAG_PALRES_ITEM7 );
//	BBAG_ItemIconPlttChg( wk, item, 0, PAL_ID_ITEM1 );
	BBAG_ClactOn( wk->clwk[BBAG_CA_ITEM7], &P3_ItemIconPos[0] );

	if( wk->dat->mode == BBAG_MODE_SHOOTER ){
			CostObjPut( wk, BBAG_CA_COST1_NUM, BBAGITEM_GetCost(item), 0, &P3_ItemIconPos[1] );
//		BBAG_ClactOn( wk->clwk[BBAG_CA_COST1], &P3_ItemIconPos[1] );
//		GFL_CLACT_WK_SetAnmSeq( wk->clwk[BBAG_CA_COST1], BBAGITEM_GetCost(item)-1 );
	}

	if( wk->dat->mode == BBAG_MODE_GETDEMO ){
		BBAG_ClactOn( wk->clwk[BBAG_CA_GETDEMO], &P3_DemoCursorPos );
		BBAG_ChangeGetDemoCursorAnm( wk, 0 );
	}
}


//============================================================================================
//	カーソル
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル追加
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_ClactCursorAdd( BBAG_WORK * wk )
{
	BAPPTOOL_AddCursor(
		wk->cpwk, wk->clunit,
		wk->chrRes[BBAG_CHRRES_CURSOR],
		wk->palRes[BBAG_PALRES_CURSOR],
		wk->celRes[BBAG_CELRES_CURSOR] );

	BAPPTOOL_VanishCursor( wk->cpwk, FALSE );

/*
	CATS_SYS_PTR csp;
	BCURSOR_PTR	cursor;

	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	BCURSOR_ResourceLoad(
		csp, wk->crp, wk->pfd, wk->dat->heap, CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR );

	cursor = BCURSOR_ActorCreate(
				csp, wk->crp, wk->dat->heap,
				CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR, 0, 1 );

	BAPP_CursorMvWkSetBCURSOR_PTR( wk->cmv_wk, cursor );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル削除
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_CursorDel( BBAG_WORK * wk )
{
	BAPPTOOL_DelCursor( wk->cpwk );

/*
	BCURSOR_ActorDelete( BAPP_CursorMvWkGetBCURSOR_PTR( wk->cmv_wk ) );
	BCURSOR_ResourceFree(
		wk->crp, CHR_ID_CURSOR, PAL_ID_CURSOR, CEL_ID_CURSOR, ANM_ID_CURSOR );
*/
}


/*
// ポケット選択画面移動テーブル
static const POINTSEL_WORK P1_CursorPosTbl[] =
{
	{   8,  16, 120,  72, 0, 1, 0, 2 },					// 0 : HP回復ポケット
	{   8,  88, 120, 144, 0, 4, 1, 3 },					// 1 : 状態回復ポケット
	{ 136,  16, 248,  72, 2, 3, 0, 2 },					// 2 : ボールポケット
	{ 136,  88, 248, 144, 2, 5, 1, 3 },					// 3 : 戦闘用ポケット
	{   8, 160, 200, 184, 1, 4, 4, 5 },					// 4 : 最後に使用した道具
	{ 224, 160, 248, 184, BAPP_CMV_RETBIT|3, 5, 4, 5 },	// 5 : 戻る
};

// アイテム選択画面移動テーブル
static const POINTSEL_WORK P2_CursorPosTbl[] =
{
	{   8,  16, 120,  48, 0, 2, 0, 1 },					// 0 : アイテム１
	{ 136,  16, 248,  48, 1, 3, 0, 1 },					// 1 : アイテム２
	{   8,  64, 120,  96, 0, 4, 2, 3 },					// 2 : アイテム３
	{ 136,  64, 248,  96, 1, 5, 2, 3 },					// 3 : アイテム４
	{   8, 112, 120, 144, 2, 6, 4, 5 },					// 4 : アイテム５
	{ 136, 112, 248, 144, 3, 6, 4, 5 },					// 5 : アイテム６
	{ 224, 160, 248, 184, BAPP_CMV_RETBIT|5, 6, 6, 6 },	// 6 : 戻る
};

// アイテム選択画面移動テーブル
static const POINTSEL_WORK P3_CursorPosTbl[] =
{
	{   8, 160, 200, 184, 0, 0, 0, 1 },	// 0 : 使う
	{ 224, 160, 248, 184, 1, 1, 0, 1 },	// 1 : 戻る
};

static const POINTSEL_WORK * const CursorPosTable[] = {
	P1_CursorPosTbl,
	P2_CursorPosTbl,
	P3_CursorPosTbl,
};


#define	P1_DEF_CMV_TBL	( 0x3f )	// ページ１のデフォルトカーソル移動テーブル
#define	P1_LST_CMV_TBL	( 0x2f )	// ページ１の最後に使った道具有のカーソル移動テーブル
#define	P2_DEF_CMV_TBL	( 0x100 )	// ページ２のデフォルトカーソル移動テーブル
#define	P2_PAGE_TBL		( 0xc0 )	// ページ２のページ切り替えカーソル移動テーブル

/*
void BBAG_P1CursorMvTblMake( BBAG_WORK * wk )
{
	if( wk->used_item != 0 ){
		BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, P1_DEF_CMV_TBL );
	}else{
		BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, P1_LST_CMV_TBL );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動テーブル作成
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BBAG_P2CursorMvTblMake( BBAG_WORK * wk )
{
	u32	tbl;
	u32	i;

	tbl = P2_DEF_CMV_TBL;
	for( i=0; i<6; i++ ){
		if( wk->pocket[wk->poke_id][wk->dat->item_scr[wk->poke_id]*6+i].id == 0 ){
			continue;
		}
		tbl |= (1<<i);
	}
	if( wk->scr_max[wk->poke_id] != 0 ){
		tbl |= P2_PAGE_TBL;
	}
	BAPP_CursorMvWkSetMvTbl( wk->cmv_wk, tbl );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソルセット
 *
 * @param	wk		ワーク
 * @param	page	ページID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BattleBag_CursorMoveSet( BBAG_WORK * wk, u8 page )
{
	BAPP_CursorMvWkSetPoint( wk->cmv_wk, CursorPosTable[page] );

	switch( page ){
	case BBAG_PAGE_POCKET:		// ポケット選択ページ
//		if( BAPP_CursorMvWkGetFlag( wk->cmv_wk ) == 1 ){
			BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->poke_id );
//		}
//		BBAG_P1CursorMvTblMake( wk );
		break;

	case BBAG_PAGE_MAIN:		// アイテム選択ページ
//		if( BAPP_CursorMvWkGetFlag( wk->cmv_wk ) == 1 ){
			BAPP_CursorMvWkSetPos( wk->cmv_wk, wk->dat->item_pos[wk->poke_id] );
//		}
//		BBAG_P2CursorMvTblMake( wk );
		break;

	case BBAG_PAGE_ITEM:		// アイテム使用ページ
		break;
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル非表示
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BattleBag_CursorOff( BBAG_WORK * wk )
{
	BAPP_CursorMvWkSetFlag( wk->cmv_wk, 0 );
	BAPP_CursorMvWkPosInit( wk->cmv_wk );
	BCURSOR_OFF( BAPP_CursorMvWkGetBCURSOR_PTR( wk->cmv_wk ) );
}
*/


//============================================================================================
//	捕獲デモカーソル
//============================================================================================
/*
void BattleBag_GetDemoCursorAnm( BBAG_WORK * wk )
{
	GFL_CLACT_WK_AddAnmFrameNumCap( wk->cap[BBAG_CA_GETDEMO], FX32_ONE );
//	BattleBag_GetDemoCursorPush( wk );
}
*/
/*
void BattleBag_GetDemoCursorPush( BBAG_WORK * wk )
{
//	if( wk->get_anm == 1 ){
		s16	x, y;
		GFL_CLACT_WK_GetPosCap( wk->cap[BBAG_CA_GETDEMO], &x, &y );
		GFL_CLACT_WK_SetWldPosCap( wk->cap[BBAG_CA_GETDEMO], x, y+8 );
//	}
}
*/


static void GetDemoCursorCallBack_Normal( u32 param, fx32 currentFrame )
{
	BBAG_WORK * wk = (BBAG_WORK *)param;
	wk->get_cnt++;
}

static void GetDemoCursorCallBack_Push( u32 param, fx32 currentFrame )
{
	BBAG_WORK * wk = (BBAG_WORK *)param;
	wk->get_cnt++;
}

static void SetGetDemoCursorCallBack( BBAG_WORK * wk, u32 anm )
{
	GFL_CLWK_ANM_CALLBACK	dat;

	dat.param = (u32)wk;

	if( anm == 0 ){
		dat.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM;
		dat.frame_idx     = 0;
		dat.p_func        = GetDemoCursorCallBack_Normal;
	}else{
		dat.callback_type = CLWK_ANM_CALLBACK_TYPE_SPEC_FRM;
		dat.frame_idx     = BBAGOBJ_GETDEMO_CURSOR_PUSH_BUTTON_ANIME_FRAME;
		dat.p_func        = GetDemoCursorCallBack_Push;
	}

	wk->get_cnt = 0;

	GFL_CLACT_WK_StartAnmCallBack( wk->clwk[BBAG_CA_GETDEMO], &dat );
}

void BBAG_ChangeGetDemoCursorAnm( BBAG_WORK * wk, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[BBAG_CA_GETDEMO], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[BBAG_CA_GETDEMO], anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[BBAG_CA_GETDEMO], TRUE );

	SetGetDemoCursorCallBack( wk, anm );
}


//--------------------------------------------------------------------------------------------
/**
 * 捕獲デモカーソル追加
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BBAG_ClactGetDemoCursorAdd( BBAG_WORK * wk )
{
	CATS_SYS_PTR csp;
	FINGER_PTR	finger;

	csp = BattleWorkCATS_SYS_PTRGet( wk->dat->bw );

	FINGER_ResourceLoad(
		csp, wk->crp, wk->dat->heap, wk->pfd, CHR_ID_GETDEMO, PAL_ID_GETDEMO, CEL_ID_GETDEMO, ANM_ID_GETDEMO );

	wk->finger = FINGER_ActorCreate(
					csp, wk->crp, wk->dat->heap,
					CHR_ID_GETDEMO, PAL_ID_GETDEMO, CEL_ID_GETDEMO, ANM_ID_GETDEMO, 0, 0 );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 捕獲デモカーソル削除
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BBAG_GetDemoCursorDel( BBAG_WORK * wk )
{
	FINGER_ActorDelete( wk->finger );
	FINGER_ResourceFree(
		wk->crp, CHR_ID_GETDEMO, PAL_ID_GETDEMO, CEL_ID_GETDEMO, ANM_ID_GETDEMO );
}
*/

/*
#define	GET_DEMO_FINGER_WAIT	( 60 )		// アニメウェイト

// カーソル表示座標テーブル
static const int GetDemoCursorPos[3][2] =
{
	{ P1_GETDEMO_CURSOR_X, P1_GETDEMO_CURSOR_Y },
	{ P2_GETDEMO_CURSOR_X, P2_GETDEMO_CURSOR_Y },
	{ P3_GETDEMO_CURSOR_X, P3_GETDEMO_CURSOR_Y }
};
*/

//--------------------------------------------------------------------------------------------
/**
 * 捕獲デモカーソルセット
 *
 * @param	wk		ワーク
 * @param	page	ページID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BBAG_GetDemoCursorSet( BBAG_WORK * wk, u8 page )
{
	if( wk->dat->mode == BBAG_MODE_GETDEMO ){
		FINGER_PosSetON( wk->finger, GetDemoCursorPos[page][0], GetDemoCursorPos[page][1] );
		FINGER_TouchReq( wk->finger, GET_DEMO_FINGER_WAIT );
	}else{
		FINGER_OFF( wk->finger );
	}
}
*/

#define	COST_ANM_ENE_NONE			( 15 )
#define	COST_ANM_ENE_ONE			( 16 )
#define	COST_ANM_ENE_TWO			( 17 )
#define	COST_ANM_ENE_RES_ONE	( 18 )
#define	COST_ANM_RES_ONE			( 19 )
#define	COST_ANM_RES_TWO			( 20 )

static void CostObjPut( BBAG_WORK * wk, u16 idx, u8 ene, u8 res_ene, const GFL_CLACTPOS * pos )
{
	GFL_CLACTPOS	mvp;
	u16	i;
	s8	te, re;

	// 数字
	BBAG_ClactOn( wk->clwk[idx], pos );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], ene );

	// ゲージ
	idx++;
	te = ene;
	re = res_ene;
	mvp = *pos;
	mvp.x += 12;
	for( i=0; i<7; i++ ){
		if( te <= 0 ){
			if( re <= 0 ){
				GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_ENE_NONE );
			}else if( re == 1 ){
				GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_RES_ONE );
				re -= 1;
			}else{
				GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_RES_TWO );
				re -= 2;
			}
		}else if( te == 1 ){
			if( re == 0 ){
				GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_ENE_ONE );
			}else if( re > 0 ){
				GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_ENE_RES_ONE );
				re -= 1;
			}
			te -= 1;
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->clwk[idx], COST_ANM_ENE_TWO );
			te -= 2;
		}
		BBAG_ClactOn( wk->clwk[idx], &mvp );
		mvp.x += 8;
		idx++;
	}
}
