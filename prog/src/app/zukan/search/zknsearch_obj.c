//============================================================================================
/**
 * @file		zknsearch_obj.c
 * @brief		図鑑検索画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHOBJ
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "app/app_menu_common.h"
#include "ui/touchbar.h"

#include "../zukan_common.h"
#include "zknsearch_main.h"
#include "zknsearch_obj.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	RES_NONE	( 0xffffffff )		// リソースが読み込まれていない

// パレット定義（メイン画面）
#define	PALNUM_ZKNOBJ			( 0 )															// パレット番号：図鑑用ＯＢＪ
#define	PALSIZ_ZKNOBJ			(	3 )															// パレットサイズ：図鑑用ＯＢＪ
#define	PALNUM_LOADING		( PALNUM_ZKNOBJ+PALSIZ_ZKNOBJ )		// パレット番号：読み込み中ＯＢＪ
#define	PALSIZ_LOADING		(	1 )															// パレットサイズ：読み込み中ＯＢＪ
#define	PALNUM_FORM				( PALNUM_LOADING+PALSIZ_LOADING )	// パレット番号：フォルムＯＢＪ
#define	PALSIZ_FORM				(	1 )															// パレットサイズ：フォルムＯＢＪ
#define	PALNUM_TOUCH_BAR	( PALNUM_FORM+PALSIZ_FORM )				// パレット番号：タッチバー
#define	PALSIZ_TOUCH_BAR	( 4 )															// パレットサイズ：タッチバー
// パレット定義（サブ画面）
#define	PALNUM_ZKNOBJ_S		( 0 )																// パレット番号：図鑑用ＯＢＪ
#define	PALSIZ_ZKNOBJ_S		(	3 )																// パレットサイズ：図鑑用ＯＢＪ
#define	PALNUM_FORM_S			( PALNUM_ZKNOBJ_S+PALSIZ_ZKNOBJ_S )	// パレット番号：フォルムＯＢＪ
#define	PLASIZ_FORM_S			( PALSIZ_FORM )											// パレットサイズ：フォルムＯＢＪ

#define	MARK_PX				( 140 )
#define	MARK_PY				( 12 )
#define	MARK_SY				( 24 )
#define	MARK_PX_FORM	( 140 )
#define	MARK_PY_FORM	( 20 )
#define	MARK_SY_FORM	( 40 )

#define	FORM_PX				( 204 )
#define	FORM_PY				( 20 )

#define	SCROLL_BAR_PX	( 252 )
#define	SCROLL_BAR_UY	( 12 )
#define	SCROLL_BAR_DY	( 156 )

#define	MAINPAGE_FORM_PX	( 192 )
#define	MAINPAGE_FORM_PY	( 116 )

#define	FORMPAGE_FORM_PX	( 64 )
#define	FORMPAGE_FORM_PY	( 104 )

#define	LOADING_BAR_PX		( 8 )
#define	LOADING_BAR_PY		( 176 )
#define	LOADING_BAR_MVX		( 2 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void InitClact(void);
static void InitResource( ZKNSEARCHMAIN_WORK * wk );
static void ExitResource( ZKNSEARCHMAIN_WORK * wk );
static void AddClact( ZKNSEARCHMAIN_WORK * wk );
static void DelClactAll( ZKNSEARCHMAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// セルアクターデータ
static const ZKNCOMM_CLWK_DATA ClactParamTbl[] =
{
	{	// スクロールバー
		{ SCROLL_BAR_PX, SCROLL_BAR_UY, ZKNSEARCHOBJ_ANM_BAR, 0, 1 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR, ZKNSEARCHOBJ_PALRES_SCROLL_BAR, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// レール
		{ SCROLL_BAR_PX, 84, ZKNSEARCHOBJ_ANM_RAIL, 10, 1 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR, ZKNSEARCHOBJ_PALRES_SCROLL_BAR, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// 戻るボタン
		{ 232, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_RETURN, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_TOUCH_BAR, ZKNSEARCHOBJ_PALRES_TOUCH_BAR, ZKNSEARCHOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// Ｘボタン
		{ 208, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_EXIT, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_TOUCH_BAR, ZKNSEARCHOBJ_PALRES_TOUCH_BAR, ZKNSEARCHOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// Ｙボタン
		{ 188, TOUCHBAR_ICON_Y_CHECK, APP_COMMON_BARICON_CHECK_OFF, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_TOUCH_BAR, ZKNSEARCHOBJ_PALRES_TOUCH_BAR, ZKNSEARCHOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// 右ボタン
		{ 208, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_RIGHT, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_TOUCH_BAR, ZKNSEARCHOBJ_PALRES_TOUCH_BAR, ZKNSEARCHOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// 左ボタン
		{ 188, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_LEFT, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_TOUCH_BAR, ZKNSEARCHOBJ_PALRES_TOUCH_BAR, ZKNSEARCHOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// フォルム
		{ 0, 0, 0, 0, 0 },
		ZKNSEARCHOBJ_CHRRES_FORM_M, ZKNSEARCHOBJ_PALRES_FORM_M, ZKNSEARCHOBJ_CELRES_FORM,
		0, CLSYS_DRAW_MAIN
	},
	{	// 読み込み中
		{ LOADING_BAR_PX, LOADING_BAR_PY, 0, 0, 1 },
		ZKNSEARCHOBJ_CHRRES_LOADING_BAR, ZKNSEARCHOBJ_PALRES_LOADING_BAR, ZKNSEARCHOBJ_CELRES_LOADING_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// マーク１
		{ 0, 0, 2, 0, 2 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR, ZKNSEARCHOBJ_PALRES_SCROLL_BAR, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// マーク２
		{ 0, 0, 2, 0, 2 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR, ZKNSEARCHOBJ_PALRES_SCROLL_BAR, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// マーク１
		{ 0, 0, 2, 0, 2 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR_S, ZKNSEARCHOBJ_PALRES_SCROLL_BAR_S, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_SUB
	},
	{	// マーク２
		{ 0, 0, 2, 0, 2 },
		ZKNSEARCHOBJ_CHRRES_SCROLL_BAR_S, ZKNSEARCHOBJ_PALRES_SCROLL_BAR_S, ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
		0, CLSYS_DRAW_SUB
	},
};

// フォルムセルアクターデータ（メイン画面）
static const ZKNCOMM_CLWK_DATA FormClactParamMain = {
	{ 0, 0, 0, 0, 2 },
	ZKNSEARCHOBJ_CHRRES_FORM_M, ZKNSEARCHOBJ_PALRES_FORM_M, ZKNSEARCHOBJ_CELRES_FORM,
	0, CLSYS_DRAW_MAIN
};
// フォルムセルアクターデータ（サブ画面）
static const ZKNCOMM_CLWK_DATA FormClactParamSub = {
	{ 0, 0, 0, 0, 2 },
	ZKNSEARCHOBJ_CHRRES_FORM_S, ZKNSEARCHOBJ_PALRES_FORM_S, ZKNSEARCHOBJ_CELRES_FORM,
	0, CLSYS_DRAW_SUB
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_Init( ZKNSEARCHMAIN_WORK * wk )
{
	InitClact();
	InitResource( wk );
	AddClact( wk );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_Exit( ZKNSEARCHMAIN_WORK * wk )
{
	DelClactAll( wk );
	ExitResource( wk );
	GFL_CLACT_SYS_Delete();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメメイン
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_AnmMain( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNSEARCHOBJ_IDX_MAX; i++ ){
		if( wk->clwk[i] == NULL ){ continue; }
		if( GFL_CLACT_WK_GetAutoAnmFlag( wk->clwk[i] ) == TRUE ){ continue; }
		GFL_CLACT_WK_AddAnmFrame( wk->clwk[i], FX32_ONE );
	}
	GFL_CLACT_SYS_Main();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ変更
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_SetAnm( ZKNSEARCHMAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターオートアニメ設定
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_SetAutoAnm( ZKNSEARCHMAIN_WORK * wk, u32 id, u32 anm )
{
	ZKNSEARCHOBJ_SetAnm( wk, id, anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ取得
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @return	アニメ番号
 */
//--------------------------------------------------------------------------------------------
u32 ZKNSEARCHOBJ_GetAnm( ZKNSEARCHMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_GetAnmSeq( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ状態取得
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNSEARCHOBJ_CheckAnm( ZKNSEARCHMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_CheckAnmActive( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示切り替え
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		flg		表示フラグ
 *
 * @return	none
 *
 * @li	flg = TRUE : 表示
 * @li	flg = FALSE : 非表示
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_SetVanish( ZKNSEARCHMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], flg );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示チェック
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNSEARCHOBJ_CheckVanish( ZKNSEARCHMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_GetDrawEnable( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター半透明設定
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		flg		ON/OFFフラグ
 *
 * @return	none
 *
 * @li	flg : TRUE = ON, FALSE = OFF
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_SetBlendMode( ZKNSEARCHMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( flg == TRUE ){
		GFL_CLACT_WK_SetObjMode( wk->clwk[id], GX_OAM_MODE_XLU );
	}else{
		GFL_CLACT_WK_SetObjMode( wk->clwk[id], GX_OAM_MODE_NORMAL );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標設定
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_SetPos( ZKNSEARCHMAIN_WORK * wk, u32 id, s16 x, s16 y, u16 setsf )
{
	GFL_CLACTPOS	pos;

	pos.x = x;
	pos.y = y;
	GFL_CLACT_WK_SetPos( wk->clwk[id], &pos, setsf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標取得
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHOBJ_GetPos( ZKNSEARCHMAIN_WORK * wk, u32 id, s16 * x, s16 * y, u16 setsf )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->clwk[id], &pos, setsf );
	*x = pos.x;
	*y = pos.y;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターパレット変更
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		id			OBJ ID
 * @param		pal			パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void ZKNSEARCHOBJ_ChgPltt( ZKNSEARCHMAIN_WORK * wk, u32 id, u32 pal )
{
	GFL_CLACT_WK_SetPlttOffs( wk->clwk[id], pal, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}
*/



static void InitClact(void)
{
	const GFL_CLSYS_INIT init = {
		0, 0,									// メイン　サーフェースの左上座標
		0, 512,								// サブ　サーフェースの左上座標
		4,										// メイン画面OAM管理開始位置	4の倍数
		124,									// メイン画面OAM管理数				4の倍数
		4,										// サブ画面OAM管理開始位置		4の倍数
		124,									// サブ画面OAM管理数					4の倍数
		0,										// セルVram転送管理数

		ZKNSEARCHOBJ_CHRRES_MAX,	// 登録できるキャラデータ数
		ZKNSEARCHOBJ_PALRES_MAX,	// 登録できるパレットデータ数
		ZKNSEARCHOBJ_CELRES_MAX,	// 登録できるセルアニメパターン数
		0,											// 登録できるマルチセルアニメパターン数（※現状未対応）

	  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	};
	GFL_CLACT_SYS_Create( &init, ZKNSEARCHMAIN_GetVramBankData(), HEAPID_ZUKAN_SEARCH );
}

static void InitResource( ZKNSEARCHMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	u32 * chr;
	u32 * pal;
	u32 * cel;
	u32	i;

	// 初期化
	for( i=0; i<ZKNSEARCHOBJ_CHRRES_MAX; i++ ){
		wk->chrRes[i] = RES_NONE;
	}
	for( i=0; i<ZKNSEARCHOBJ_PALRES_MAX; i++ ){
		wk->palRes[i] = RES_NONE;
	}
	for( i=0; i<ZKNSEARCHOBJ_CELRES_MAX; i++ ){
		wk->celRes[i] = RES_NONE;
	}

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_SEARCH_L );
	// スクロールバー
	// メイン画面
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_SCROLL_BAR ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_SCROLL_BAR ];
	cel = &wk->celRes[ ZKNSEARCHOBJ_CELRES_SCROLL_BAR ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_search_search_objd_NCGR,
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_search_search_objd_NCLR,
					CLSYS_DRAW_MAIN, PALNUM_ZKNOBJ*0x20, HEAPID_ZUKAN_SEARCH );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					NARC_zukan_gra_search_search_objd_NCER,
					NARC_zukan_gra_search_search_objd_NANR,
					HEAPID_ZUKAN_SEARCH );
	// サブ画面
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_SCROLL_BAR_S ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_SCROLL_BAR_S ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_search_search_objd_NCGR,
					FALSE, CLSYS_DRAW_SUB, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_search_search_objd_NCLR,
					CLSYS_DRAW_SUB, PALNUM_ZKNOBJ_S*0x20, HEAPID_ZUKAN_SEARCH );
	// ローディングバー
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_LOADING_BAR ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_LOADING_BAR ];
	cel = &wk->celRes[ ZKNSEARCHOBJ_CELRES_LOADING_BAR ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_search_loading_NCGR,
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_search_loading_NCLR,
					CLSYS_DRAW_MAIN, PALNUM_LOADING*0x20, HEAPID_ZUKAN_SEARCH );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					NARC_zukan_gra_search_loading_NCER,
					NARC_zukan_gra_search_loading_NANR,
					HEAPID_ZUKAN_SEARCH );
	// フォルムＯＢＪ
	// メイン画面
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_FORM_M ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_FORM_M ];
	cel = &wk->celRes[ ZKNSEARCHOBJ_CELRES_FORM ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_search_zkn_form_NCGR,
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_search_zkn_form_NCLR,
					CLSYS_DRAW_MAIN, PALNUM_FORM*0x20, HEAPID_ZUKAN_SEARCH );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					NARC_zukan_gra_search_zkn_form_NCER,
					NARC_zukan_gra_search_zkn_form_NANR,
					HEAPID_ZUKAN_SEARCH );
	// サブ画面
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_FORM_S ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_FORM_S ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_search_zkn_form_NCGR,
					FALSE, CLSYS_DRAW_SUB, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_search_zkn_form_NCLR,
					CLSYS_DRAW_SUB, PALNUM_FORM_S*0x20, HEAPID_ZUKAN_SEARCH );
	GFL_ARC_CloseDataHandle( ah );

	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_SEARCH_L );
	chr = &wk->chrRes[ ZKNSEARCHOBJ_CHRRES_TOUCH_BAR ];
	pal = &wk->palRes[ ZKNSEARCHOBJ_PALRES_TOUCH_BAR ];
	cel = &wk->celRes[ ZKNSEARCHOBJ_CELRES_TOUCH_BAR ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, APP_COMMON_GetBarIconCharArcIdx(),
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_SEARCH );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, APP_COMMON_GetBarIconPltArcIdx(),
					CLSYS_DRAW_MAIN, PALNUM_TOUCH_BAR*0x20,HEAPID_ZUKAN_SEARCH );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K),
					APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K),
					HEAPID_ZUKAN_SEARCH );
	GFL_ARC_CloseDataHandle( ah );
}

static void ExitResource( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNSEARCHOBJ_CHRRES_MAX; i++ ){
		if( wk->chrRes[i] != RES_NONE ){
			GFL_CLGRP_CGR_Release( wk->chrRes[i] );
		}
	}
	for( i=0; i<ZKNSEARCHOBJ_PALRES_MAX; i++ ){
		if( wk->palRes[i] != RES_NONE ){
	    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
		}
	}
	for( i=0; i<ZKNSEARCHOBJ_CELRES_MAX; i++ ){
		if( wk->celRes[i] != RES_NONE ){
	    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
		}
	}
}

static void AddClact( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	wk->clunit = GFL_CLACT_UNIT_Create( ZKNSEARCHOBJ_IDX_MAX, 0, HEAPID_ZUKAN_SEARCH );

	// 初期化
	for( i=0; i<ZKNSEARCHOBJ_IDX_MAX; i++ ){
		wk->clwk[i] = NULL;
	}

	for( i=0; i<=ZKNSEARCHOBJ_IDX_MARK2_S; i++ ){
		ZKNCOMM_CLWK_DATA	dat = ClactParamTbl[i];
		dat.chrRes = wk->chrRes[dat.chrRes];
		dat.palRes = wk->palRes[dat.palRes];
		dat.celRes = wk->celRes[dat.celRes];
		wk->clwk[i] = ZKNCOMM_CreateClact( wk->clunit, &dat, HEAPID_ZUKAN_SEARCH );
	}
	// Ｙ登録
	if( GAMEDATA_GetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_SEARCH ) == TRUE ){
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, APP_COMMON_BARICON_CHECK_ON );
	}

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_LOADING_BAR, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_S, FALSE );

	ZKNSEARCHOBJ_SetBlendMode( wk, ZKNSEARCHOBJ_IDX_MARK1_S, TRUE );
	ZKNSEARCHOBJ_SetBlendMode( wk, ZKNSEARCHOBJ_IDX_MARK2_S, TRUE );

	// フォルム
	for( i=0; i<ZKNSEARCHOBJ_FORM_MAX; i++ ){
		ZKNCOMM_CLWK_DATA	dat = FormClactParamMain;
		dat.chrRes = wk->chrRes[dat.chrRes];
		dat.palRes = wk->palRes[dat.palRes];
		dat.celRes = wk->celRes[dat.celRes];
		dat.dat.anmseq = i;
		wk->clwk[ZKNSEARCHOBJ_IDX_FORM_M+i] = ZKNCOMM_CreateClact( wk->clunit, &dat, HEAPID_ZUKAN_SEARCH );
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_M+i, FALSE );

		dat = FormClactParamSub;
		dat.chrRes = wk->chrRes[dat.chrRes];
		dat.palRes = wk->palRes[dat.palRes];
		dat.celRes = wk->celRes[dat.celRes];
		dat.dat.anmseq = i;
		wk->clwk[ZKNSEARCHOBJ_IDX_FORM_S+i] = ZKNCOMM_CreateClact( wk->clunit, &dat, HEAPID_ZUKAN_SEARCH );
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_S+i, FALSE );
		ZKNSEARCHOBJ_SetBlendMode( wk, ZKNSEARCHOBJ_IDX_FORM_S+i, TRUE );
	}
}

static void DelClact( ZKNSEARCHMAIN_WORK * wk, u32 idx )
{
	if( wk->clwk[idx] != NULL ){
		GFL_CLACT_WK_Remove( wk->clwk[idx] );
		wk->clwk[idx] = NULL;
	}
}

static void DelClactAll( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNSEARCHOBJ_IDX_MAX; i++ ){
		DelClact( wk, i );
	}
	GFL_CLACT_UNIT_Delete( wk->clunit );
}



/*
void ZKNSEARCHOBJ_VanishAll( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNSEARCHOBJ_IDX_MAX; i++ ){
		ZKNSEARCHOBJ_SetVanish( wk, i, FALSE );
	}
}
*/

void ZKNSEARCHOBJ_VanishList( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	// スクロールバー
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, FALSE );

	// マーク
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_S, FALSE );

	// フォルム
	for( i=0; i<ZKNSEARCHOBJ_FORM_MAX; i++ ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_M+i, FALSE );
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_S+i, FALSE );
	}
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, FALSE );
}


void ZKNSEARCHOBJ_PutMainPage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, TRUE );

	if( wk->dat->sort->form != ZKNCOMM_LIST_SORT_NONE ){
		ZKNSEARCHOBJ_SetAnm( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, wk->dat->sort->form );
		ZKNSEARCHOBJ_SetPos(
			wk, ZKNSEARCHOBJ_IDX_FORM_LABEL,
			MAINPAGE_FORM_PX, MAINPAGE_FORM_PY, CLSYS_DRAW_MAIN );
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, TRUE );
	}
}

void ZKNSEARCHOBJ_PutRowPage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
}

void ZKNSEARCHOBJ_PutNamePage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, TRUE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, TRUE );

	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, SCROLL_BAR_UY, CLSYS_DRAW_MAIN );
}

void ZKNSEARCHOBJ_PutTypePage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, TRUE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, TRUE );

	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, SCROLL_BAR_UY, CLSYS_DRAW_MAIN );
}

void ZKNSEARCHOBJ_PutColorPage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, TRUE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, TRUE );

	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, SCROLL_BAR_UY, CLSYS_DRAW_MAIN );
}

void ZKNSEARCHOBJ_PutFormPage( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, FALSE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, TRUE );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, TRUE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_SCROLL_RAIL, TRUE );

	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, SCROLL_BAR_UY, CLSYS_DRAW_MAIN );

	ZKNSEARCHOBJ_PutFormListNow( wk );

}

void ZKNSEARCHOBJ_PutFormListNow( ZKNSEARCHMAIN_WORK * wk )
{
	if( wk->dat->sort->form != ZKNCOMM_LIST_SORT_NONE ){
		ZKNSEARCHOBJ_SetAnm( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, wk->dat->sort->form );
		ZKNSEARCHOBJ_SetPos(
			wk, ZKNSEARCHOBJ_IDX_FORM_LABEL,
			FORMPAGE_FORM_PX, FORMPAGE_FORM_PY, CLSYS_DRAW_MAIN );
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, TRUE );
	}else{
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, FALSE );
	}
}



// スクロールバー
void ZKNSEARCHOBJ_SetScrollBar( ZKNSEARCHMAIN_WORK * wk, u32 py )
{
	if( py < SCROLL_BAR_UY ){
		py = SCROLL_BAR_UY;
	}else if( py > SCROLL_BAR_DY ){
		py = SCROLL_BAR_DY;
	}
	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, py, CLSYS_DRAW_MAIN );
}

void ZKNSEARCHOBJ_SetListPageArrowAnime( ZKNSEARCHMAIN_WORK * wk, BOOL anm )
{
	if( ZKNSEARCHOBJ_GetAnm(wk,ZKNSEARCHOBJ_IDX_TB_LEFT) != APP_COMMON_BARICON_CURSOR_LEFT_ON ||
			anm == FALSE ){
		if( FRAMELIST_GetScrollCount( wk->lwk ) == 0 ){
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_OFF );
		}else{
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT );
		}
	}
	if( ZKNSEARCHOBJ_GetAnm(wk,ZKNSEARCHOBJ_IDX_TB_RIGHT) != APP_COMMON_BARICON_CURSOR_RIGHT_ON ||
			anm == FALSE ){
		if( FRAMELIST_CheckScrollMax( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_OFF );
		}else{
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT );
		}
	}
}

// マーク表示
void ZKNSEARCHOBJ_PutMark( ZKNSEARCHMAIN_WORK * wk, u16 num, s16 py, BOOL disp )
{
	if( disp == TRUE ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M+num, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_M+num, MARK_PX, py+MARK_PY, CLSYS_DRAW_MAIN );
	}else{
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S+num, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_S+num, MARK_PX, py+MARK_PY, CLSYS_DRAW_SUB );
	}
}

void ZKNSEARCHOBJ_ChangeMark( ZKNSEARCHMAIN_WORK * wk, u16 pos, BOOL flg )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, flg );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	if( flg == TRUE ){
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_M, MARK_PX, MARK_PY+MARK_SY*pos, CLSYS_DRAW_MAIN );
	}
}

void ZKNSEARCHOBJ_ChangeTypeMark( ZKNSEARCHMAIN_WORK * wk, u8	pos1, u8 pos2 )
{
	u8	list_pos = FRAMELIST_GetScrollCount( wk->lwk );

	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_M, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_S, FALSE );

	if( pos1 != ZKNCOMM_LIST_SORT_NONE ){
		u8	abs = GFL_STD_Abs( pos1 - list_pos );
		if( pos1 < list_pos ){
			if( abs <= 8 ){
				ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, TRUE );
				ZKNSEARCHOBJ_SetPos(
					wk, ZKNSEARCHOBJ_IDX_MARK1_S, MARK_PX, MARK_PY+MARK_SY*(8-abs), CLSYS_DRAW_SUB );
			}
		}else{
			if( abs <= 7 ){
				ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, TRUE );
				ZKNSEARCHOBJ_SetPos(
					wk, ZKNSEARCHOBJ_IDX_MARK1_M, MARK_PX, MARK_PY+MARK_SY*abs, CLSYS_DRAW_MAIN );
			}
		}
	}

	if( pos2 != ZKNCOMM_LIST_SORT_NONE ){
		u8	abs = GFL_STD_Abs( pos2 - list_pos );
		if( pos2 < list_pos ){
			if( abs <= 8 ){
				ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_S, TRUE );
				ZKNSEARCHOBJ_SetPos(
					wk, ZKNSEARCHOBJ_IDX_MARK2_S, MARK_PX, MARK_PY+MARK_SY*(8-abs), CLSYS_DRAW_SUB );
			}
		}else{
			if( abs <= 7 ){
				ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK2_M, TRUE );
				ZKNSEARCHOBJ_SetPos(
					wk, ZKNSEARCHOBJ_IDX_MARK2_M, MARK_PX, MARK_PY+MARK_SY*abs, CLSYS_DRAW_MAIN );
			}
		}
	}
}

void ZKNSEARCHOBJ_PutFormMark( ZKNSEARCHMAIN_WORK * wk, s16 py, BOOL disp )
{
	if( disp == TRUE ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_M, MARK_PX, py+MARK_PY_FORM, CLSYS_DRAW_MAIN );
	}else{
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_S, MARK_PX, py+MARK_PY_FORM, CLSYS_DRAW_SUB );
	}
}

void ZKNSEARCHOBJ_ChangeFormMark( ZKNSEARCHMAIN_WORK * wk, u16 pos, BOOL flg )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, flg );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	if( flg == TRUE ){
		ZKNSEARCHOBJ_SetPos(
			wk, ZKNSEARCHOBJ_IDX_MARK1_M, MARK_PX_FORM, MARK_PY_FORM+MARK_SY_FORM*pos, CLSYS_DRAW_MAIN );
	}
}

void ZKNSEARCHOBJ_PutFormList( ZKNSEARCHMAIN_WORK * wk, u16 num, s16 py, BOOL disp )
{
	if( disp == TRUE ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_M+num, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_FORM_M+num, FORM_PX, py+FORM_PY, CLSYS_DRAW_MAIN );
	}else{
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_S+num, TRUE );
		ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_FORM_S+num, FORM_PX, py+FORM_PY, CLSYS_DRAW_SUB );
	}
}

void ZKNSEARCHOBJ_ScrollList( ZKNSEARCHMAIN_WORK * wk, s8 mv )
{
	u32	i;
	s16	x, y;

	for( i=ZKNSEARCHOBJ_IDX_MARK1_M; i<=ZKNSEARCHOBJ_IDX_MARK2_M; i++ ){
		if( ZKNSEARCHOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNSEARCHOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_MAIN );
		ZKNSEARCHOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_MAIN );

		if( (y+mv) <= -24 || (y+mv) >= 255 ){
			ZKNSEARCHOBJ_SetVanish( wk, i, FALSE );
		}
	}
	for( i=ZKNSEARCHOBJ_IDX_MARK1_S; i<=ZKNSEARCHOBJ_IDX_MARK2_S; i++ ){
		if( ZKNSEARCHOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNSEARCHOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_SUB );
		ZKNSEARCHOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_SUB );

		if( (y+mv) <= -24 || (y+mv) >= 255 ){
			ZKNSEARCHOBJ_SetVanish( wk, i, FALSE );
		}
	}
}

void ZKNSEARCHOBJ_ScrollFormList( ZKNSEARCHMAIN_WORK * wk, s8 mv )
{
	u32	i;
	s16	x, y;

	for( i=ZKNSEARCHOBJ_IDX_FORM_M; i<ZKNSEARCHOBJ_IDX_FORM_M+ZKNSEARCHOBJ_FORM_MAX; i++ ){
		if( ZKNSEARCHOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNSEARCHOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_MAIN );
		ZKNSEARCHOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_MAIN );

		if( (y+mv) <= -24 || (y+mv) >= 255 ){
			ZKNSEARCHOBJ_SetVanish( wk, i, FALSE );
		}
	}
	ZKNSEARCHOBJ_GetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_M, &x, &y, CLSYS_DRAW_MAIN );
	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_M, x, y+mv, CLSYS_DRAW_MAIN );
	if( (y+mv) <= -24 || (y+mv) >= 255 ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_M, FALSE );
	}

	for( i=ZKNSEARCHOBJ_IDX_FORM_S; i<ZKNSEARCHOBJ_IDX_FORM_S+ZKNSEARCHOBJ_FORM_MAX; i++ ){
		if( ZKNSEARCHOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNSEARCHOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_SUB );
		ZKNSEARCHOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_SUB );

		if( (y+mv) <= -32 || (y+mv) >= 255 ){
			ZKNSEARCHOBJ_SetVanish( wk, i, FALSE );
		}
	}
	ZKNSEARCHOBJ_GetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_S, &x, &y, CLSYS_DRAW_SUB );
	ZKNSEARCHOBJ_SetPos( wk, ZKNSEARCHOBJ_IDX_MARK1_S, x, y+mv, CLSYS_DRAW_SUB );
	if( (y+mv) <= -32 || (y+mv) >= 255 ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_MARK1_S, FALSE );
	}
}

void ZKNSEARCHOBJ_MoveLoadingBar( ZKNSEARCHMAIN_WORK * wk, u32 cnt )
{
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_LOADING_BAR, TRUE );
	ZKNSEARCHOBJ_SetPos(
		wk, ZKNSEARCHOBJ_IDX_LOADING_BAR,
		LOADING_BAR_PX+LOADING_BAR_MVX*cnt, LOADING_BAR_PY, CLSYS_DRAW_MAIN );
}
