//============================================================================================
/**
 * @file		zknlist_obj.c
 * @brief		図鑑リスト画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTOBJ
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/poke2dgra.h"
#include "pokeicon/pokeicon.h"
#include "app/app_menu_common.h"
#include "ui/touchbar.h"

#include "zknlist_main.h"
#include "zknlist_obj.h"
#include "zknlist_bgwfrm.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	RES_NONE	( 0xffffffff )		// リソースが読み込まれていない

// パレット定義（メイン画面）
#define	PALNUM_ZKNOBJ			( 0 )																	// パレット番号：　図鑑用ＯＢＪ
#define	PALSIZ_ZKNOBJ			( 3 )																	// パレットサイズ：図鑑用ＯＢＪ
#define	PALNUM_POKEICON		( PALNUM_ZKNOBJ+PALSIZ_ZKNOBJ )				// パレット番号：　ポケモンアイコン
#define	PALSIZ_POKEICON		( POKEICON_PAL_MAX )									// パレットサイズ：ポケモンアイコン
#define	PALNUM_TOUCH_BAR	( PALNUM_POKEICON+PALSIZ_POKEICON )		// パレット番号：　タッチバー
#define	PALSIZ_TOUCH_BAR	( 4 )																	// パレットサイズ：タッチバー
#define	PALNUM_POKEGRA		( PALNUM_TOUCH_BAR+PALSIZ_TOUCH_BAR )	// パレット番号：　ポケモン正面絵
#define	PALSIZ_POKEGRA		( 1 )																	// パレットサイズ：ポケモン正面絵
#define	PALNUM_POKEGRA2		( PALNUM_POKEGRA+PALSIZ_POKEGRA )			// パレット番号：　ポケモン正面絵２
#define	PALSIZ_POKEGRA2		( PALSIZ_POKEGRA )										// パレットサイズ：ポケモン正面絵２
// パレット定義（サブ画面）
#define	PALNUM_POKEICON_S	( 0 )									// パレット番号：　ポケモンアイコン
#define	PLASIZ_POKEICON_S	( POKEICON_PAL_MAX )	// パレットサイズ：ポケモンアイコン

#define	POKEICON_PX		( 112 )
#define	POKEICON_PY		( 8 )
#define	POKEICON_SY		( 24 )

#define	SCROLL_BAR_PX	( 252 )
#define	SCROLL_BAR_UY	( 12 )
#define	SCROLL_BAR_DY	( 156 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void InitClact(void);
static void InitResource( ZKNLISTMAIN_WORK * wk );
static void ExitResource( ZKNLISTMAIN_WORK * wk );
static void AddClact( ZKNLISTMAIN_WORK * wk );
static void DelClactAll( ZKNLISTMAIN_WORK * wk );

static void AddPokeIcon( ZKNLISTMAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// セルアクターデータ
static const ZKNCOMM_CLWK_DATA ClactParamTbl[] =
{
	{	// スクロールバー
		{ SCROLL_BAR_PX, SCROLL_BAR_UY, ZKNLISTOBJ_ANM_BAR, 0, 1 },
		ZKNLISTOBJ_CHRRES_ZKNOBJ, ZKNLISTOBJ_PALRES_ZKNOBJ, ZKNLISTOBJ_CELRES_ZKNOBJ,
		0, CLSYS_DRAW_MAIN
	},
	{	// レール
		{ SCROLL_BAR_PX, 84, ZKNLISTOBJ_ANM_RAIL, 10, 1 },
		ZKNLISTOBJ_CHRRES_ZKNOBJ, ZKNLISTOBJ_PALRES_ZKNOBJ, ZKNLISTOBJ_CELRES_ZKNOBJ,
		0, CLSYS_DRAW_MAIN
	},
	{	// 戻るボタン
		{ 232, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_RETURN, 0, 1 },
		ZKNLISTOBJ_CHRRES_TOUCH_BAR, ZKNLISTOBJ_PALRES_TOUCH_BAR, ZKNLISTOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// Ｘボタン
		{ 208, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_EXIT, 0, 1 },
		ZKNLISTOBJ_CHRRES_TOUCH_BAR, ZKNLISTOBJ_PALRES_TOUCH_BAR, ZKNLISTOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// Ｙボタン
		{ 188, TOUCHBAR_ICON_Y_CHECK, APP_COMMON_BARICON_CHECK_OFF, 0, 1 },
		ZKNLISTOBJ_CHRRES_TOUCH_BAR, ZKNLISTOBJ_PALRES_TOUCH_BAR, ZKNLISTOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// 右ボタン
		{ 160, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_RIGHT, 0, 1 },
		ZKNLISTOBJ_CHRRES_TOUCH_BAR, ZKNLISTOBJ_PALRES_TOUCH_BAR, ZKNLISTOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// 左ボタン
		{ 136, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_LEFT, 0, 1 },
		ZKNLISTOBJ_CHRRES_TOUCH_BAR, ZKNLISTOBJ_PALRES_TOUCH_BAR, ZKNLISTOBJ_CELRES_TOUCH_BAR,
		0, CLSYS_DRAW_MAIN
	},
	{	// セレクトボタン
		{ 104, 180, ZKNLISTOBJ_ANM_SELECT, 0, 1 },
		ZKNLISTOBJ_CHRRES_ZKNOBJ, ZKNLISTOBJ_PALRES_ZKNOBJ, ZKNLISTOBJ_CELRES_ZKNOBJ,
		0, CLSYS_DRAW_MAIN
	},
	{	// スタートボタン
		{ 36, 180, ZKNLISTOBJ_ANM_START, 0, 1 },
		ZKNLISTOBJ_CHRRES_ZKNOBJ, ZKNLISTOBJ_PALRES_ZKNOBJ, ZKNLISTOBJ_CELRES_ZKNOBJ,
		0, CLSYS_DRAW_MAIN
	},
};

// ポケモン正面絵セルアクターデータ
static const ZKNCOMM_CLWK_DATA PokeClactParam = {
	{ 48, 48, 0, 0, 1 },
	ZKNLISTOBJ_CHRRES_POKEGRA, ZKNLISTOBJ_PALRES_POKEGRA, ZKNLISTOBJ_CELRES_POKEGRA,
	0, CLSYS_DRAW_MAIN
};

// ポケモンアイコンセルアクターデータ
static const ZKNCOMM_CLWK_DATA PokeIconClactParam = {
	{ POKEICON_PX, POKEICON_PY, 0, 0, 2 },
	ZKNLISTOBJ_CHRRES_POKEICON, ZKNLISTOBJ_PALRES_POKEICON, ZKNLISTOBJ_CELRES_POKEICON,
	0, CLSYS_DRAW_MAIN
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター初期化
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_Init( ZKNLISTMAIN_WORK * wk )
{
	InitClact();
	InitResource( wk );
	AddClact( wk );
	AddPokeIcon( wk );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_Exit( ZKNLISTMAIN_WORK * wk )
{
	DelClactAll( wk );
	ExitResource( wk );
	GFL_CLACT_SYS_Delete();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメメイン
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_AnmMain( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNLISTOBJ_IDX_MAX; i++ ){
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
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_SetAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターオートアニメ設定
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_SetAutoAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm )
{
	ZKNLISTOBJ_SetAnm( wk, id, anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ取得
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 *
 * @return	アニメ番号
 */
//--------------------------------------------------------------------------------------------
u32 ZKNLISTOBJ_GetAnm( ZKNLISTMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_GetAnmSeq( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ状態取得
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNLISTOBJ_CheckAnm( ZKNLISTMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_CheckAnmActive( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示切り替え
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		flg		表示フラグ
 *
 * @return	none
 *
 * @li	flg = TRUE : 表示
 * @li	flg = FALSE : 非表示
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_SetVanish( ZKNLISTMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], flg );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示チェック
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNLISTOBJ_CheckVanish( ZKNLISTMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_GetDrawEnable( wk->clwk[id] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターBGプライオリティ変更
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		pri		プライオリティ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void ZKNLISTOBJ_ChgBgPriority( ZKNLISTMAIN_WORK * wk, u32 id, int pri )
{
	GFL_CLACT_WK_SetBgPri( wk->clwk[id], pri );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターのOBJ同士のプライオリティ変更
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		pri		プライオリティ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void ZKNLISTOBJ_ChgObjPriority( ZKNLISTMAIN_WORK * wk, u32 id, int pri )
{
	GFL_CLACT_WK_SetSoftPri( wk->clwk[id], pri );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター半透明設定
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		flg		ON/OFFフラグ
 *
 * @return	none
 *
 * @li	flg : TRUE = ON, FALSE = OFF
 */
//--------------------------------------------------------------------------------------------
void BOX2OBJ_SetBlendMode( ZKNLISTMAIN_WORK * wk, u32 id, BOOL flg )
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
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_SetPos( ZKNLISTMAIN_WORK * wk, u32 id, s16 x, s16 y, u16 setsf )
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
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_GetPos( ZKNLISTMAIN_WORK * wk, u32 id, s16 * x, s16 * y, u16 setsf )
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
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		pal			パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_ChgPltt( ZKNLISTMAIN_WORK * wk, u32 id, u32 pal )
{
	GFL_CLACT_WK_SetPlttOffs( wk->clwk[id], pal, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}








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

		ZKNLISTOBJ_CHRRES_MAX,	// 登録できるキャラデータ数
		ZKNLISTOBJ_PALRES_MAX,	// 登録できるパレットデータ数
		ZKNLISTOBJ_CELRES_MAX,	// 登録できるセルアニメパターン数
		0,											// 登録できるマルチセルアニメパターン数（※現状未対応）

	  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	};
	GFL_CLACT_SYS_Create( &init, ZKNLISTMAIN_GetVramBankData(), HEAPID_ZUKAN_LIST );
}


static void InitResource( ZKNLISTMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	u32 * chr;
	u32 * pal;
	u32 * cel;
	u32	i;

	// 初期化
	for( i=0; i<ZKNLISTOBJ_CHRRES_MAX; i++ ){
		wk->chrRes[i] = RES_NONE;
	}
	for( i=0; i<ZKNLISTOBJ_PALRES_MAX; i++ ){
		wk->palRes[i] = RES_NONE;
	}
	for( i=0; i<ZKNLISTOBJ_CELRES_MAX; i++ ){
		wk->celRes[i] = RES_NONE;
	}


	// 図鑑用ＯＢＪ
	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST_L );
	chr = &wk->chrRes[ ZKNLISTOBJ_CHRRES_ZKNOBJ ];
	pal = &wk->palRes[ ZKNLISTOBJ_PALRES_ZKNOBJ ];
	cel = &wk->celRes[ ZKNLISTOBJ_CELRES_ZKNOBJ ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, NARC_zukan_gra_list_list_objd_NCGR,
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, NARC_zukan_gra_list_list_objd_NCLR,
					CLSYS_DRAW_MAIN, PALNUM_ZKNOBJ*0x20, HEAPID_ZUKAN_LIST );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					NARC_zukan_gra_list_list_objd_NCER,
					NARC_zukan_gra_list_list_objd_NANR,
					HEAPID_ZUKAN_LIST );
	GFL_ARC_CloseDataHandle( ah );

	// ポケアイコン
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_ZUKAN_LIST_L );
	// メイン画面
	chr = &wk->chrRes[ ZKNLISTOBJ_CHRRES_POKEICON ];
	pal = &wk->palRes[ ZKNLISTOBJ_PALRES_POKEICON ];
	cel = &wk->celRes[ ZKNLISTOBJ_CELRES_POKEICON ];
	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		chr[i] = GFL_CLGRP_CGR_Register(
							ah, POKEICON_GetCgxArcIndexByMonsNumber(0,0,FALSE),
							FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
	}
  *pal = GFL_CLGRP_PLTT_RegisterComp(
					ah, POKEICON_GetPalArcIndex(),
					CLSYS_DRAW_MAIN, PALNUM_POKEICON*0x20, HEAPID_ZUKAN_LIST );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					POKEICON_GetCellArcIndex(),
					POKEICON_GetAnmArcIndex(),
					HEAPID_ZUKAN_LIST );
	// サブ画面
	chr = &wk->chrRes[ ZKNLISTOBJ_CHRRES_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX ];
	pal = &wk->palRes[ ZKNLISTOBJ_PALRES_POKEICON_S ];
	cel = &wk->celRes[ ZKNLISTOBJ_CELRES_POKEICON_S ];
	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		chr[i] = GFL_CLGRP_CGR_Register(
							ah, POKEICON_GetCgxArcIndexByMonsNumber(0,0,FALSE),
							FALSE, CLSYS_DRAW_SUB, HEAPID_ZUKAN_LIST );
	}
  *pal = GFL_CLGRP_PLTT_RegisterComp(
					ah, POKEICON_GetPalArcIndex(),
					CLSYS_DRAW_SUB, PALNUM_POKEICON_S*0x20, HEAPID_ZUKAN_LIST );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					POKEICON_GetCellSubArcIndex(),
					POKEICON_GetAnmSubArcIndex(),
					HEAPID_ZUKAN_LIST );
	GFL_ARC_CloseDataHandle( ah );

	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_LIST_L );
	chr = &wk->chrRes[ ZKNLISTOBJ_CHRRES_TOUCH_BAR ];
	pal = &wk->palRes[ ZKNLISTOBJ_PALRES_TOUCH_BAR ];
	cel = &wk->celRes[ ZKNLISTOBJ_CELRES_TOUCH_BAR ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, APP_COMMON_GetBarIconCharArcIdx(),
					FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, APP_COMMON_GetBarIconPltArcIdx(),
					CLSYS_DRAW_MAIN, PALNUM_TOUCH_BAR*0x20,HEAPID_ZUKAN_LIST );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K),
					APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K),
					HEAPID_ZUKAN_LIST );
	GFL_ARC_CloseDataHandle( ah );
}

static void ExitResource( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNLISTOBJ_CHRRES_MAX; i++ ){
		if( wk->chrRes[i] != RES_NONE ){
			GFL_CLGRP_CGR_Release( wk->chrRes[i] );
		}
	}
	for( i=0; i<ZKNLISTOBJ_PALRES_MAX; i++ ){
		if( wk->palRes[i] != RES_NONE ){
	    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
		}
	}
	for( i=0; i<ZKNLISTOBJ_CELRES_MAX; i++ ){
		if( wk->celRes[i] != RES_NONE ){
	    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
		}
	}
}

static void AddClact( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	wk->clunit = GFL_CLACT_UNIT_Create( ZKNLISTOBJ_IDX_MAX, 0, HEAPID_ZUKAN_LIST );

	// 初期化
	for( i=0; i<ZKNLISTOBJ_IDX_MAX; i++ ){
		wk->clwk[i] = NULL;
	}

	for( i=0; i<=ZKNLISTOBJ_IDX_TB_START; i++ ){
		ZKNCOMM_CLWK_DATA	dat = ClactParamTbl[i];
		dat.chrRes = wk->chrRes[dat.chrRes];
		dat.palRes = wk->palRes[dat.palRes];
		dat.celRes = wk->celRes[dat.celRes];
		wk->clwk[i] = ZKNCOMM_CreateClact( wk->clunit, &dat, HEAPID_ZUKAN_LIST );
	}
}

static void DelClact( ZKNLISTMAIN_WORK * wk, u32 idx )
{
	if( wk->clwk[idx] != NULL ){
		GFL_CLACT_WK_Remove( wk->clwk[idx] );
		wk->clwk[idx] = NULL;
	}
}

static void DelClactAll( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNLISTOBJ_IDX_MAX; i++ ){
		DelClact( wk, i );
	}
	GFL_CLACT_UNIT_Delete( wk->clunit );
}



//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン正面絵セルアクターセット
 *
 * @param		wk			図鑑リストワーク
 * @param		mons		ポケモン番号
 * @param		form		フォルム
 * @param		sex			性別
 * @param		rare		レアかどうか
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_SetPokeGra( ZKNLISTMAIN_WORK * wk, u16 mons, u8 form, u8 sex, u8 rare )
{
	ARCHANDLE * ah;
	ZKNCOMM_CLWK_DATA	prm;
	u32	nowID, newID;
	u32	pal;

	prm = PokeClactParam;

	if( wk->pokeGraFlag == 0 ){
		nowID = ZKNLISTOBJ_IDX_POKEGRA2;
		newID = ZKNLISTOBJ_IDX_POKEGRA;
		pal = PALNUM_POKEGRA * 0x20;
	}else{
		nowID = ZKNLISTOBJ_IDX_POKEGRA;
		newID = ZKNLISTOBJ_IDX_POKEGRA2;
		prm.chrRes = ZKNLISTOBJ_CHRRES_POKEGRA2;
		prm.palRes = ZKNLISTOBJ_PALRES_POKEGRA2;
		prm.celRes = ZKNLISTOBJ_CELRES_POKEGRA2;
		pal = PALNUM_POKEGRA2 * 0x20;
	}

	// 新ＯＢＪが登録されていたら削除
	if( wk->clwk[newID] != NULL ){
		DelClact( wk, newID );
		if( wk->chrRes[prm.chrRes] != RES_NONE ){
			GFL_CLGRP_CGR_Release( wk->chrRes[prm.chrRes] );
		}
		if( wk->palRes[prm.palRes] != RES_NONE ){
	    GFL_CLGRP_PLTT_Release( wk->palRes[prm.palRes] );
		}
		if( wk->celRes[prm.celRes] != RES_NONE ){
	    GFL_CLGRP_CELLANIM_Release( wk->celRes[prm.celRes] );
		}
	}

	// 新ＯＢＪを追加
	if( mons == 0 ){
		ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST_L );
		wk->chrRes[prm.chrRes] = GFL_CLGRP_CGR_Register(
															ah, NARC_zukan_gra_list_list_hate_NCGR,
															FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
		wk->palRes[prm.palRes] = GFL_CLGRP_PLTT_Register(
															ah, NARC_zukan_gra_list_list_hate_NCLR,
															CLSYS_DRAW_MAIN, pal, HEAPID_ZUKAN_LIST );
		wk->celRes[prm.celRes] = GFL_CLGRP_CELLANIM_Register(
															ah,
															NARC_zukan_gra_list_list_hate_NCER,
															NARC_zukan_gra_list_list_hate_NANR,
															HEAPID_ZUKAN_LIST );
		GFL_ARC_CloseDataHandle( ah );
	}else{
		ah = POKE2DGRA_OpenHandle( HEAPID_ZUKAN_LIST_L );
		wk->chrRes[prm.chrRes] = POKE2DGRA_OBJ_CGR_Register(
															ah, mons, form, sex, rare, POKEGRA_DIR_FRONT,
															FALSE, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
		wk->palRes[prm.palRes] = POKE2DGRA_OBJ_PLTT_Register(
															ah, mons, form, sex, rare, POKEGRA_DIR_FRONT,
															FALSE, CLSYS_DRAW_MAIN, pal, HEAPID_ZUKAN_LIST );
		wk->celRes[prm.celRes] = POKE2DGRA_OBJ_CELLANM_Register(
															mons, form, sex, rare, POKEGRA_DIR_FRONT,
															FALSE, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_ZUKAN_LIST );
	  GFL_ARC_CloseDataHandle( ah );
	}

	prm.chrRes = wk->chrRes[prm.chrRes];
	prm.palRes = wk->palRes[prm.palRes];
	prm.celRes = wk->celRes[prm.celRes];
	wk->clwk[newID] = ZKNCOMM_CreateClact( wk->clunit, &prm, HEAPID_ZUKAN_LIST );

	if( mons != 0 ){
		GFL_CLACT_WK_SetFlip( wk->clwk[newID], CLWK_FLIP_H, TRUE );
	}

	// 旧ＯＢＪを非表示
	ZKNLISTOBJ_SetVanish( wk, nowID, FALSE );

	wk->pokeGraFlag ^= 1;
}

/*
void ZKNLISTOBJ_PutListPosPokeGra( ZKNLISTMAIN_WORK * wk, s16 pos )
{
	u16	mons;

	if( ZKNLISTMAIN_GetListInfo( wk->list, pos ) == 0 ){
		mons = 0;
	}else{
		mons = ZKNLISTMAIN_GetListMons( wk->list, pos );
	}
	ZKNLISTOBJ_SetPokeGra( wk, mons, 0, 0, 0 );
}
*/


//============================================================================================
//	ポケモンアイコン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコン追加
 *
 * @param		wk			図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void AddPokeIcon( ZKNLISTMAIN_WORK * wk )
{
	ZKNCOMM_CLWK_DATA	prm;
	u32	i;

	prm = PokeIconClactParam;

	prm.palRes = wk->palRes[ZKNLISTOBJ_PALRES_POKEICON];
	prm.celRes = wk->celRes[ZKNLISTOBJ_CELRES_POKEICON];
	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		prm.dat.pos_y = POKEICON_PY + POKEICON_SY * i;
		prm.chrRes = wk->chrRes[ZKNLISTOBJ_CHRRES_POKEICON+i];
		wk->clwk[ZKNLISTOBJ_IDX_POKEICON+i] = ZKNCOMM_CreateClact( wk->clunit, &prm, HEAPID_ZUKAN_LIST );
	}

	prm = PokeIconClactParam;

	prm.palRes = wk->palRes[ZKNLISTOBJ_PALRES_POKEICON_S];
	prm.celRes = wk->celRes[ZKNLISTOBJ_CELRES_POKEICON_S];
	for( i=0; i<ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		prm.dat.pos_y = POKEICON_PY + POKEICON_SY * (i+1);
		prm.chrRes = wk->chrRes[ZKNLISTOBJ_CHRRES_POKEICON_S+i];
		prm.disp = CLSYS_DRAW_SUB;
		wk->clwk[ZKNLISTOBJ_IDX_POKEICON_S+i] = ZKNCOMM_CreateClact( wk->clunit, &prm, HEAPID_ZUKAN_LIST );
		BOX2OBJ_SetBlendMode( wk, ZKNLISTOBJ_IDX_POKEICON_S+i, TRUE );
	}


	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
//		ZKNLISTOBJ_ChgPokeIcon( wk, ZKNLISTOBJ_IDX_POKEICON+i, 1+i, 0, TRUE );
		ZKNLISTOBJ_SetVanish( wk, ZKNLISTOBJ_IDX_POKEICON+i, FALSE );
	}
	for( i=0; i<ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
//		ZKNLISTOBJ_ChgPokeIcon( wk, ZKNLISTOBJ_IDX_POKEICON_S+i, 8+i, 0, FALSE );
		ZKNLISTOBJ_SetVanish( wk, ZKNLISTOBJ_IDX_POKEICON_S+i, FALSE );
	}

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコンキャラ書き換え
 *
 * @param		wk			図鑑リストワーク
 * @param		idx			OBJ Index
 * @param		form		フォルム
 * @param		disp		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTOBJ_ChgPokeIcon( ZKNLISTMAIN_WORK * wk, u32 idx, u16 mons, u16 form, BOOL disp )
{
	ARCHANDLE * ah;
	NNSG2dCharacterData * chr;
	void * buf;

	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_ZUKAN_LIST_L );

	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter(
					ah, POKEICON_GetCgxArcIndexByMonsNumber(mons,form,FALSE), FALSE, &chr, HEAPID_ZUKAN_LIST_L );
	if( disp == TRUE ){
		GFL_CLGRP_CGR_ReplaceEx(
			GFL_CLACT_WK_GetCgrIndex(wk->clwk[idx]), chr->pRawData, POKEICON_SIZE_CGX, 0, CLSYS_DRAW_MAIN );
	}else{
		GFL_CLGRP_CGR_ReplaceEx(
			GFL_CLACT_WK_GetCgrIndex(wk->clwk[idx]), chr->pRawData, POKEICON_SIZE_CGX, 0, CLSYS_DRAW_SUB );
	}
	GFL_HEAP_FreeMemory( buf );

  GFL_ARC_CloseDataHandle( ah );

	GFL_CLACT_WK_SetPlttOffs(
		wk->clwk[idx], POKEICON_GetPalNum(mons,form,FALSE), CLWK_PLTTOFFS_MODE_PLTT_TOP );
}


void ZKNLISTOBJ_ScrollPokeIcon( ZKNLISTMAIN_WORK * wk, s16 mv )
{
	u32	i;
	s16	x, y;

	for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		ZKNLISTOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_MAIN );
		ZKNLISTOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_MAIN );
	}
	for( i=ZKNLISTOBJ_IDX_POKEICON_S; i<ZKNLISTOBJ_IDX_POKEICON_S+ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		ZKNLISTOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_SUB );
		ZKNLISTOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_SUB );
	}
}

/*
void ZKNLISTOBJ_PutPokeList( ZKNLISTMAIN_WORK * wk, u32 objIdx, s32 listPos, BOOL disp )
{
	if( listPos < 0 ){
		ZKNLISTOBJ_SetVanish( wk, objIdx, FALSE );
		return;
	}

	if( ZKNLISTMAIN_GetListInfo( wk->list, listPos ) == 0 ){
		ZKNLISTOBJ_SetVanish( wk, objIdx, FALSE );
	}else{
		ZKNLISTOBJ_ChgPokeIcon( wk, objIdx, ZKNLISTMAIN_GetListMons(wk->list,listPos), 0, disp );
		ZKNLISTOBJ_SetVanish( wk, objIdx, TRUE );
	}
	ZKNLISTOBJ_SetAutoAnm( wk, objIdx, POKEICON_ANM_DEATH );
}
*/
/*
void ZKNLISTOBJ_PutScrollList( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv )
{
	if( mv == ZKNLISTBGWFRM_LISTPUT_UP ){
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON+idx, POKEICON_PX, POKEICON_PY-POKEICON_SY, CLSYS_DRAW_MAIN );
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON_S+idx, POKEICON_PX, POKEICON_PY, CLSYS_DRAW_SUB );
	}else{
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON+idx, POKEICON_PX, POKEICON_PY+POKEICON_SY*7, CLSYS_DRAW_MAIN );
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON_S+idx, POKEICON_PX, POKEICON_PY+POKEICON_SY*8, CLSYS_DRAW_SUB );
	}
}
*/
/*
void ZKNLISTOBJ_InitScrollList( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON+i, POKEICON_PX, POKEICON_PY+POKEICON_SY*i, CLSYS_DRAW_MAIN );
	}
	for( i=0; i<ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		ZKNLISTOBJ_SetPos(
			wk, ZKNLISTOBJ_IDX_POKEICON_S+i, POKEICON_PX, POKEICON_PY+POKEICON_SY*(i+1), CLSYS_DRAW_SUB );
	}
}
*/

void ZKNLISTOBJ_ChgListPosAnm( ZKNLISTMAIN_WORK * wk, u32 pos, BOOL flg )
{
	u16	i;
	s16	py;
	s16	x, y;

	py = POKEICON_PY + POKEICON_SY * pos;

	for( i=0; i<ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		ZKNLISTOBJ_GetPos( wk, ZKNLISTOBJ_IDX_POKEICON+i, &x, &y, CLSYS_DRAW_MAIN );
		if( y == py ){
			break;
		}
	}

	if( flg == TRUE ){
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_POKEICON+i, POKEICON_ANM_HPMAX );
	}else{
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_POKEICON+i, POKEICON_ANM_DEATH );
	}
}




u32 ZKNLISTOBJ_GetChgPokeIconIndex( ZKNLISTMAIN_WORK * wk, BOOL disp )
{
	u32	i;

	if( disp == TRUE ){
		for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
			if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
				return i;
			}
		}
	}else{
		for( i=ZKNLISTOBJ_IDX_POKEICON_S; i<ZKNLISTOBJ_IDX_POKEICON_S+ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
			if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
				return i;
			}
		}
	}

	return 0;
}

void ZKNLISTOBJ_PutPokeList2( ZKNLISTMAIN_WORK * wk, u16 mons, s16 py, BOOL disp )
{
	u32	obj = ZKNLISTOBJ_GetChgPokeIconIndex( wk, disp );

	ZKNLISTOBJ_ChgPokeIcon( wk, obj, mons, 0, disp );
	ZKNLISTOBJ_SetVanish( wk, obj, TRUE );
	ZKNLISTOBJ_SetAutoAnm( wk, obj, POKEICON_ANM_DEATH );

	if( disp == TRUE ){
		ZKNLISTOBJ_SetPos( wk, obj, POKEICON_PX, py+POKEICON_PY, CLSYS_DRAW_MAIN );
	}else{
		ZKNLISTOBJ_SetPos( wk, obj, POKEICON_PX, py+POKEICON_PY, CLSYS_DRAW_SUB );
	}
}

void ZKNLISTOBJ_PutScrollList2( ZKNLISTMAIN_WORK * wk, s8 mv )
{
	u32	i;
	s16	x, y;

	for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNLISTOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_MAIN );
		ZKNLISTOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_MAIN );

		if( (y+mv) <= -16 || (y+mv) >= 200 ){
			ZKNLISTOBJ_SetVanish( wk, i, FALSE );
		}
	}
	for( i=ZKNLISTOBJ_IDX_POKEICON_S; i<ZKNLISTOBJ_IDX_POKEICON_S+ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		ZKNLISTOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_SUB );
		ZKNLISTOBJ_SetPos( wk, i, x, y+mv, CLSYS_DRAW_SUB );

		if( (y+mv) <= -16 || (y+mv) >= 200 ){
			ZKNLISTOBJ_SetVanish( wk, i, FALSE );
		}
	}
}

void ZKNLISTOBJ_SetPutPokeIconFlag( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	wk->iconPutMain = 0;
	wk->iconPutSub = 0;

	for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		wk->iconPutMain |= (1<<(i-ZKNLISTOBJ_IDX_POKEICON));
	}
	for( i=ZKNLISTOBJ_IDX_POKEICON_S; i<ZKNLISTOBJ_IDX_POKEICON_S+ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
			continue;
		}
		wk->iconPutSub |= (1<<(i-ZKNLISTOBJ_IDX_POKEICON_S));
	}
}

void ZKNLISTOBJ_VanishJumpPokeIcon( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		if( wk->iconPutMain & (1<<(i-ZKNLISTOBJ_IDX_POKEICON)) ){
			ZKNLISTOBJ_SetVanish( wk, i, FALSE );
		}
	}
	for( i=ZKNLISTOBJ_IDX_POKEICON_S; i<ZKNLISTOBJ_IDX_POKEICON_S+ZKNLISTOBJ_SUB_POKEICON_MAX; i++ ){
		if( wk->iconPutSub & (1<<(i-ZKNLISTOBJ_IDX_POKEICON_S)) ){
			ZKNLISTOBJ_SetVanish( wk, i, FALSE );
		}
	}

	wk->iconPutMain = 0;
	wk->iconPutSub = 0;
}

void ZKNLISTOBJ_ChangePokeIconAnime( ZKNLISTMAIN_WORK * wk, u32 pos )
{
	u32	i;
	s16	x, y;

	pos = POKEICON_PY + POKEICON_SY * pos;

	for( i=ZKNLISTOBJ_IDX_POKEICON; i<ZKNLISTOBJ_IDX_POKEICON+ZKNLISTOBJ_MAIN_POKEICON_MAX; i++ ){
		if( ZKNLISTOBJ_CheckVanish( wk, i ) == FALSE ){
			ZKNLISTOBJ_SetAutoAnm( wk, i, POKEICON_ANM_DEATH );
			continue;
		}
		ZKNLISTOBJ_GetPos( wk, i, &x, &y, CLSYS_DRAW_MAIN );
		if( pos == y ){
			if( ZKNLISTOBJ_GetAnm(wk,i) != POKEICON_ANM_HPMAX ){
				ZKNLISTOBJ_SetAutoAnm( wk, i, POKEICON_ANM_HPMAX );
			}
		}else{
			ZKNLISTOBJ_SetAutoAnm( wk, i, POKEICON_ANM_DEATH );
		}
	}
}







// スクロールバー
void ZKNLISTOBJ_SetScrollBar( ZKNLISTMAIN_WORK * wk, u32 py )
{
	if( py < SCROLL_BAR_UY ){
		py = SCROLL_BAR_UY;
	}else if( py > SCROLL_BAR_DY ){
		py = SCROLL_BAR_DY;
	}
	ZKNLISTOBJ_SetPos( wk, ZKNLISTOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, py, CLSYS_DRAW_MAIN );
}

/*
u32 ZKNLISTOBJ_GetListScrollBarPos( ZKNLISTMAIN_WORK * wk )
{
	u32	max;
	u32	cnt;
	s16	x, y;

	ZKNLISTOBJ_GetPos( wk, ZKNLISTOBJ_IDX_SCROLL_BAR, &x, &y, CLSYS_DRAW_MAIN );
	max = ZKNLISTMAIN_GetListScrollMax( wk->list );

	if( y == SCROLL_BAR_UY ){
		cnt = 0;
	}else if( y == SCROLL_BAR_DY ){
		cnt = max;
	}else{
//		cnt = ( ( (max+1) << 8 ) / ( SCROLL_BAR_DY - SCROLL_BAR_UY ) * ( y - SCROLL_BAR_UY ) ) >> 8;
		cnt = ( ( max << 8 ) / ( SCROLL_BAR_DY - SCROLL_BAR_UY ) * ( y - SCROLL_BAR_UY ) ) >> 8;
	}

	return cnt;
}
*/
/*
void ZKNLISTOBJ_SetListScrollBarPos( ZKNLISTMAIN_WORK * wk )
{
	u32	max;
	s16	scroll;
	s16	y;
	
	scroll = ZKNLISTMAIN_GetListScroll( wk->list );
	max    = ZKNLISTMAIN_GetListScrollMax( wk->list );

	if( scroll == 0 ){
		y = SCROLL_BAR_UY;
	}else if( scroll == max ){
		y = SCROLL_BAR_DY;
	}else{
		y = ( ( ( (SCROLL_BAR_DY-SCROLL_BAR_UY) << 8 ) / max * scroll ) >> 8 ) + SCROLL_BAR_UY;
	}

	ZKNLISTOBJ_SetPos( wk, ZKNLISTOBJ_IDX_SCROLL_BAR, SCROLL_BAR_PX, y, CLSYS_DRAW_MAIN );
}
*/

void ZKNLISTOBJ_SetListPageArrowAnime( ZKNLISTMAIN_WORK * wk, BOOL anm )
{
//	if( ZKNLISTOBJ_CheckAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT ) == FALSE || anm == FALSE ){
	if( ZKNLISTOBJ_GetAnm(wk,ZKNLISTOBJ_IDX_TB_LEFT) != APP_COMMON_BARICON_CURSOR_LEFT_ON ||
			anm == FALSE ){
		if( FRAMELIST_GetScrollCount( wk->lwk ) == 0 ){
			ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_OFF );
		}else{
			ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT );
		}
	}
//	if( ZKNLISTOBJ_CheckAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT ) == FALSE || anm == FALSE ){
	if( ZKNLISTOBJ_GetAnm(wk,ZKNLISTOBJ_IDX_TB_RIGHT) != APP_COMMON_BARICON_CURSOR_RIGHT_ON ||
			anm == FALSE ){
		if( FRAMELIST_CheckScrollMax( wk->lwk ) == FALSE ){
			ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_OFF );
		}else{
			ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT );
		}
	}
}
