//============================================================================================
/**
 * @file		box2_bgwfrm.c
 * @brief		ボックス画面 BGウィンドウフレーム関連
 * @author	Hiroyuki Nakamura
 * @date		09.10.07
 *
 *	モジュール名：BOX2BGWFRM
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_bgwfrm.h"
#include "box_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================

// マーキング
#define	MARKING_FRM_SX	( 11 )						// マーキングフレームＸサイズ
#define	MARKING_FRM_SY	( 15 )						// マーキングフレームＹサイズ
//#define	MARKING_FRM_PX	( 32 - MARKING_FRM_SX )		// マーキングフレームＸ表示座標
//#define	MARKING_FRM_PY	( 26 )						// マーキングフレームＸ表示座標

#define	MARKING_ON_CHR	( 0x2b )					// マーキングキャラ開始位置 ( ON )
#define	MARKING_OFF_CHR	( 0x0b )					// マーキングキャラ開始位置 ( OFF )

// 手持ちポケモンフレーム
#define	PARTYPOKE_FRM_SX	( BOX2BGWFRM_PARTYPOKE_SX )		// 手持ちポケモンフレームＸサイズ
#define	PARTYPOKE_FRM_SY	( 18 )		// 手持ちポケモンフレームＹサイズ
#define	PARTYPOKE_FRM_PX	( 2 )		// 手持ちポケモンフレームＸ表示座標
#define	PARTYPOKE_FRM_PY	( 21 )		// 手持ちポケモンフレームＹ表示座標
#define	WINFRM_PARTYPOKE_LX	( BOX2BGWFRM_PARTYPOKE_LX )
#define	WINFRM_PARTYPOKE_RX	( 21 )
#define	WINFRM_PARTYPOKE_PY	( BOX2BGWFRM_PARTYPOKE_PY )
#define	WINFRM_PARTYPOKE_INIT_PY	( 24 )
#define	WINFRM_PARTYPOKE_RET_PX		( 24 )
#define	WINFRM_PARTYPOKE_RET_PY		( 15 )

// ボックス移動フレーム
#define BOXMOVE_FRM_SX			( 11 )		// ボックス移動フレームＸサイズ
#define BOXMOVE_FRM_SY			( 21 )		// ボックス移動フレームＹサイズ
#define BOXMV_PTOUT_FRM_SX	( 32 )		// ボックス移動フレームＸサイズ
#define BOXMV_PTOUT_FRM_SY	( 7 )		// ボックス移動フレームＹサイズ

// 壁紙変更フレーム
#define WPCHG_FRM_SX		( 32 )		// ボックス移動フレームＸサイズ
#define WPCHG_FRM_SY		( 7 )		// ボックス移動フレームＹサイズ

// BGWINフレームデータ
#define	SUBDISP_ITEM_FRM_SX			( 32 )
#define	SUBDISP_ITEM_FRM_SY			( 9 )
#define	SUBDISP_ITEM_FRM_IN_PX	( 0 )
#define	SUBDISP_ITEM_FRM_IN_PY	( 24 )
#define	SUBDISP_WAZA_FRM_SX			( 12 )
#define	SUBDISP_WAZA_FRM_SY			( 10 )
#define	SUBDISP_WAZA_FRM_IN_PX	( 32 )
#define	SUBDISP_WAZA_FRM_IN_PY	( 12 )

// ポケモン選択メニューフレームデータ
#define	WINFRM_MENU_INIT_PX	( 32 )		// 表示Ｘ座標
#define	WINFRM_MENU_PX		( 21 )		// 表示Ｘ座標
#define	WINFRM_MENU_PY		( 2 )		// 表示Ｙ座標
#define	WINFRM_MENU_SY		( 3 )		// 表示Ｙサイズ

//「ボックスをきりかえる」ボタンフレームデータ
#define	WINFRM_BOXCHG_BTN_PX			( 0 )
#define	WINFRM_BOXCHG_BTN_PY			( 21 )
#define	WINFRM_BOXCHG_BTN_INIT_PY	( 24 )

//「Ｙようすをみる」ボタンフレームデータ
#define	WINFRM_Y_ST_BTN_PX			( 0 )
#define	WINFRM_Y_ST_BTN_PY			( 21 )
#define	WINFRM_Y_ST_BTN_INIT_PY	( 24 )

//「てもちポケモン」フレームデータ
#define	WINFRM_TEMOCHI_PX		( 0 )
#define	WINFRM_TEMOCHI_PY		( 21 )
#define	WINFRM_TEMOCHI_OUT_PY	( 24 )
//「ポケモンいどう」フレームデータ
#define	WINFRM_IDOU_PX			( 12 )
#define	WINFRM_IDOU_PY			( 21 )
#define	WINFRM_IDOU_OUT_PY		( 24 )
//「もどる」フレームデータ
#define	WINFRM_MODORU_PX		( 24 )
#define	WINFRM_MODORU_PY		( 21 )
#define	WINFRM_MODORU_OUT_PY	( 24 )

// マーキング配置データ
#define	MARK_DEF_PX		( 3 )
#define	MARK_DEF_PY		( 1 )
#define	MARK_DEF_SX		( 4 )
#define	MARK_DEF_SY		( 3 )

// マーキングフレームデータ
#define	WINFRM_MARK_PX			( 21 )
#define	WINFRM_MARK_PY			( 5 )
#define	WINFRM_MARK_IN_START_PY	( 24 )
#define	WINFRM_MARK_MV_CNT		( WINFRM_MARK_IN_START_PY - WINFRM_MARK_PY )

// ボックス移動メニューフレームデータ
#define	WINFRM_BOXMV_MENU_PX		( 21 )
#define	WINFRM_BOXMV_MENU_PY		( 14 )
#define	WINFRM_BOXMV_MENU_INIT_PX	( 32 )
#define	WINFRM_BOXMV_MENU_SX		( 11 )

// トレイ選択フレームデータ
#define	WINFRM_TRAYMOVE_IN_PX		( 32 )
#define	WINFRM_TRAYMOVE_IN_PY		( 0 )
#define	WINFRM_TRAYMOVE_OUT_PX	( 21 )
#define	WINFRM_TRAYMOVE_OUT_PY	( 0 )

// ボックステーマ変更メニューフレームデータ
#define	WINFRM_BOXTHEMA_MENU_SX		( 11 )
//#define	WINFRM_BOXTHEMA_MENU_SY		( 14 )
#define	WINFRM_BOXTHEMA_MENU_PX		( WINFRM_BOXMV_MENU_PX )
#define	WINFRM_BOXTHEMA_MENU1_PY	( 8 )
#define	WINFRM_BOXTHEMA_MENU2_PY	( WINFRM_BOXTHEMA_MENU1_PY + 6 )
#define	WINFRM_BOXTHEMA_MENU3_PY	( WINFRM_BOXTHEMA_MENU2_PY + 3 )
#define	WINFRM_BOXTHEMA_MENU_OUT_PX	( 32 )

// 壁紙変更フレームデータ
#define	WINFRM_WALLPAPER_PX			( 0 )
#define	WINFRM_WALLPAPER_IN_PY		( -WPCHG_FRM_SY )
#define	WINFRM_WALLPAPER_OUT_PY		( 0 )

//#define	BOXPARTY_BTN_SPD	( 8 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロール速度
#define	BOXPARTY_BTN_CNT	( 3 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロールカウント


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void FrameArcLoad( BGWINFRM_WORK * wk, u32 index, u32 dataIdx );
static void PokeMenuInitPosSet( BGWINFRM_WORK * wk );
static void BoxMoveButtonInitPut( BGWINFRM_WORK * wk );
static void YStatusButtonInitPut( BGWINFRM_WORK * wk );
static void InitTouchBar( BOX2_SYS_WORK * syswk );



//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームワーク作成（全体）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_Init( BOX2_SYS_WORK * syswk )
{
	BOX2_APP_WORK * appwk = syswk->app;

	appwk->wfrm = BGWINFRM_Create( BGWINFRM_TRANS_VBLANK, BOX2MAIN_WINFRM_MAX, HEAPID_BOX_APP );

	BOX2BMP_PokeMenuBgFrmWkMake( appwk );				// メニュー
	InitTouchBar( syswk );											// タッチバー





	// 壁紙変更フレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_WPCHG, GFL_BG_FRAME1_M, WPCHG_FRM_SX, WPCHG_FRM_SY );
	// 手持ちポケモンフレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, GFL_BG_FRAME1_M, PARTYPOKE_FRM_SX, PARTYPOKE_FRM_SY );
	// マーキングフレーム
//	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MARK, GFL_BG_FRAME0_M, MARKING_FRM_SX, MARKING_FRM_SY );
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MARK, GFL_BG_FRAME1_M, MARKING_FRM_SX, MARKING_FRM_SY );
	// ボックス移動フレーム
//	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_BOXMV_MENU, GFL_BG_FRAME0_M, BOX2BMP_BOXMVMENU_SX, BOX2BMP_BOXMVMENU_SY );
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_BOXMV_MENU, GFL_BG_FRAME1_M, BOX2BMP_BOXMVMENU_SX, BOX2BMP_BOXMVMENU_SY );


	BOX2BMP_TemochiButtonBgFrmWkMake( appwk );	//「てもちポケモン」
	BOX2BMP_IdouButtonBgFrmWkMake( appwk );			//「ポケモンいどう」
	BOX2BMP_ModoruButtonBgFrmWkMake( appwk );		//「もどる」「やめる」
	BOX2BMP_ToziruButtonBgFrmWkMake( syswk );		//「とじる」

	if( syswk->dat->callMode == BOX_MODE_AZUKERU ){
		BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, GFL_BG_FRAME0_M, BOXMV_PTOUT_FRM_SX, BOXMV_PTOUT_FRM_SY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, NARC_box_gra_box_partyout_bg_lz_NSCR );
	}else{
		BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, GFL_BG_FRAME1_M, BOXMOVE_FRM_SX, BOXMOVE_FRM_SY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, NARC_box_gra_box_move_bg_lz_NSCR );
	}

	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_WPCHG, NARC_box_gra_box_wpchg_bg_lz_NSCR );

	if( syswk->dat->callMode == BOX_MODE_AZUKERU || syswk->dat->callMode == BOX_MODE_TURETEIKU ){
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, NARC_box_gra_box_poke_bg2_lz_NSCR );
	}else{
		BOX2BGWFRM_PartyPokeFrameLoadArrange( appwk->wfrm, BOX2MAIN_WINFRM_PARTY );
		BOX2BMP_PartyCngButtonFrmPut( appwk );
	}

/*	上画面　技・アイテムフレーム
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BGWINFRM_Add(
			appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP,
			GFL_BG_FRAME2_S, SUBDISP_ITEM_FRM_SX, SUBDISP_ITEM_FRM_SY );
		BGWINFRM_FramePut(
			appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP,
			SUBDISP_ITEM_FRM_IN_PX, SUBDISP_ITEM_FRM_IN_PY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP, NARC_box_gra_sub_item_frm_lz_NSCR );
	}else{
		BGWINFRM_Add(
			appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP,
			GFL_BG_FRAME2_S, SUBDISP_WAZA_FRM_SX, SUBDISP_WAZA_FRM_SY );
		BGWINFRM_FramePut(
			appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP,
			SUBDISP_WAZA_FRM_IN_PX, SUBDISP_WAZA_FRM_IN_PY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP, NARC_box_gra_sub_waza_frm_lz_NSCR );
	}
*/

	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MARK, NARC_box_gra_box_mark_bg_lz_NSCR );
	BOX2BMP_MarkingButtonFrmPut( syswk->app );

	PokeMenuInitPosSet( appwk->wfrm );
	BoxMoveButtonInitPut( appwk->wfrm );
	YStatusButtonInitPut( appwk->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームワーク解放
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_Exit( BOX2_APP_WORK * appwk )
{
	BGWINFRM_Exit( appwk->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * ＢＧフレームにアーカイブのグラフィックを設定
 *
 * @param	wk				ＢＧウィンドウフレームワーク
 * @param	ah				アーカイブハンドル
 * @param	dataIdx		データインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void FrameArcLoad( BGWINFRM_WORK * wk, u32 index, u32 dataIdx )
{
	BGWINFRM_FrameSetArc( wk, index, ARCID_BOX2_GRA, dataIdx, TRUE );
}


//============================================================================================
//	手持ちポケモンフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFramePut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_LX, WINFRM_PARTYPOKE_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム表示（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFramePutRight( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_RX, WINFRM_PARTYPOKE_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム非表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameOff( BOX2_SYS_WORK * syswk )
{
	GFL_BG_FillScreen(
		GFL_BG_FRAME1_M, 0,
		PARTYPOKE_FRM_PX, PARTYPOKE_FRM_PY,
		PARTYPOKE_FRM_SX, PARTYPOKE_FRM_SY, GFL_BG_SCRWRT_PALNL );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（左）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameInitPutLeft( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_LX, WINFRM_PARTYPOKE_INIT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameInitPutRight( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_RX, WINFRM_PARTYPOKE_INIT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, -1, py-WINFRM_PARTYPOKE_PY );
/*
		if( px == WINFRM_PARTYPOKE_LX ){
			BGWINFRM_FramePut(
				wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_PARTYPOKE_RET_PX, py+WINFRM_PARTYPOKE_RET_PY );
			BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, -1, py-WINFRM_PARTYPOKE_PY );
		}
*/
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_INIT_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
/*
		if( px == WINFRM_PARTYPOKE_LX ){
			BGWINFRM_FramePut(
				wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_PARTYPOKE_RET_PX, py+WINFRM_PARTYPOKE_RET_PY );
			BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
		}
*/
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンのみフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	通常は「やめる」もセットされる
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameOnlyOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_INIT_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンのみフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	通常は「やめる」もセットされる
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameOnlyInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, -1, py-WINFRM_PARTYPOKE_PY );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_AzukeruPartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
/*
	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_RET_BTN,
		WINFRM_PARTYPOKE_RET_PX+BOX2MAIN_PARTYPOKE_FRM_H_CNT,
		WINFRM_PARTYPOKE_PY+WINFRM_PARTYPOKE_RET_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_AzukeruPartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_PartyPokeFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	party_mv, ret_mv;
	s8	x1, y1, x2, y2;
	BOOL	ret;

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x1, &y1 );

	party_mv = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );
//	ret_mv   = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
	ret_mv   = 0;
	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN );

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x2, &y2 );

	if( x1 != x2 || y1 != y2 ){
		BOX2OBJ_PartyPokeIconScroll2( syswk );
	}

	if( party_mv == 0 && ret_mv == 0 ){
		ret = FALSE;
	}else{
		ret = TRUE;
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム移動（「ポケモンをあずける」用）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_AzukeruPartyPokeFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	party_mv;
	s8	x1, y1, x2, y2;
	BOOL	ret;

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x1, &y1 );

	party_mv = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x2, &y2 );

	if( x1 != x2 || y1 != y2 ){
		BOX2OBJ_PartyPokeIconScroll2( syswk );
	}

	if( party_mv == 0 ){
		ret = FALSE;
	}else{
		ret = TRUE;
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレームに手持ちポケモンフレームグラフィックをセット
 *
 * @param		wk			ＢＧウィンドウフレームワーク
 * @param		index		ＢＧウィンドウフレームインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameLoadArrange( BGWINFRM_WORK * wk, u32 index )
{
	FrameArcLoad( wk, index, NARC_box_gra_box_poke_bg_lz_NSCR );
}


//============================================================================================
//	ポケモンメニューフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニューボタンを画面外に配置
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeMenuInitPosSet( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<6; i++ ){
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_INIT_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
//	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタンを画面内に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PokeMenuOpenPosSet( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<6; i++ ){
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
//	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン非表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PokeMenuOff( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<6; i++ ){
		BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_MENU1+i );
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_INIT_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
//	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_CLOSE_BTN );
//	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PokeMenuInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_PX ){ return; }

	for( i=0; i<6; i++ ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU1+i, -1, 0, px-WINFRM_MENU_PX );
	}

//	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_CLOSE_BTN, &px, &py );
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_CLOSE_BTN, 0, -1, py-WINFRM_MODORU_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PokeMenuOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_INIT_PX ){ return; }

	for( i=0; i<6; i++ ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU1+i, 1, 0, WINFRM_MENU_INIT_PX-px );
	}

//	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_CLOSE_BTN, &px, &py );
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_CLOSE_BTN, 0, 1, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT-py );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニュー移動チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FLASE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_PokeMenuMoveMain( BGWINFRM_WORK * wk )
{
	u32		i;
	BOOL	ret;
	
	ret = FALSE;

	for( i=0; i<6; i++ ){
		if( BGWINFRM_MoveOne( wk, BOX2MAIN_WINFRM_MENU1+i ) == 1 ){
			ret = TRUE;
		}
	}
/*
	if( BGWINFRM_MoveOne( wk, BOX2MAIN_WINFRM_CLOSE_BTN ) == 1 ){
		ret = TRUE;
	}
*/

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニュー配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FLASE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_PokeMenuPutCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_INIT_PX ){
		return FALSE;
	}
	return TRUE;
}


//============================================================================================
//	「ボックスをきりかえる」ボタン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタンを画面外へ配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxMoveButtonInitPut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_BOXMV_BTN, WINFRM_BOXCHG_BTN_PX, WINFRM_BOXCHG_BTN_INIT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveButtonInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_BTN, 0, -1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveButtonOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_BTN, 0, 1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_BoxMoveButtonCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_BOXMV_BTN, &px, &py );
	if( py == WINFRM_BOXCHG_BTN_PY ){
		return TRUE;
	}
	return FALSE;
}


//============================================================================================
//	Ｙステータスボタン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンを画面外へ配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void YStatusButtonInitPut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_Y_ST_BTN, WINFRM_Y_ST_BTN_PX, WINFRM_Y_ST_BTN_INIT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンを画面内へ配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_YStatusButtonPut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_Y_ST_BTN, WINFRM_Y_ST_BTN_PX, WINFRM_Y_ST_BTN_INIT_PY-BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_YStatusButtonInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_Y_ST_BTN, 0, -1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_YStatusButtonOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_Y_ST_BTN, 0, 1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_YStatusButtonCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_Y_ST_BTN, &px, &py );
	if( py == WINFRM_Y_ST_BTN_PY ){
		return TRUE;
	}
	return FALSE;
}


//============================================================================================
//	タッチバー
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチバー初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitTouchBar( BOX2_SYS_WORK * syswk )
{
	u16 * scr = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME0_M );

	scr = &scr[BOX2MAIN_TOUCH_BAR_PY*BOX2MAIN_TOUCH_BAR_SX+BOX2MAIN_TOUCH_BAR_PX];
	BGWINFRM_Add(
		syswk->app->wfrm, BOX2MAIN_WINFRM_TOUCH_BAR,
		GFL_BG_FRAME0_M, BOX2MAIN_TOUCH_BAR_SX, BOX2MAIN_TOUCH_BAR_SY );
	BGWINFRM_FrameSet( syswk->app->wfrm, BOX2MAIN_WINFRM_TOUCH_BAR, scr );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチバー配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PutTouchBar( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_TOUCH_BAR, BOX2MAIN_TOUCH_BAR_PX, BOX2MAIN_TOUCH_BAR_PY );
}


//============================================================================================
//	その他
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「てもちポケモンボタン」ボタン配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonPutTemochi( BOX2_SYS_WORK * syswk )
{
/*
	BOX2BMP_ButtonPutTemochi( syswk );
	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN, WINFRM_TEMOCHI_PX, WINFRM_TEMOCHI_PY );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonOutPutTemochi( BOX2_SYS_WORK * syswk )
{
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN, WINFRM_TEMOCHI_PX, WINFRM_TEMOCHI_OUT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_TemochiButtonOutSet( BGWINFRM_WORK * wk )
{
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_POKE_BTN, 0, 1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_TemochiButtonInSet( BGWINFRM_WORK * wk )
{
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_POKE_BTN, 0, -1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンいどう」ボタン配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonPutIdou( BOX2_SYS_WORK * syswk )
{
/*
	BOX2BMP_ButtonPutIdou( syswk );
	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN, WINFRM_IDOU_PX, WINFRM_IDOU_PY );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonOutPutIdou( BOX2_SYS_WORK * syswk )
{
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN, WINFRM_IDOU_PX, WINFRM_IDOU_OUT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_IdouButtonOutSet( BGWINFRM_WORK * wk )
{
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MV_BTN, 0, 1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_IdouButtonInSet( BGWINFRM_WORK * wk )
{
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MV_BTN, 0, -1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もどる」ボタン配置
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonPutModoru( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ButtonPutModoru( syswk );
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * 「やめる」ボタン配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ButtonPutYameru( BOX2_SYS_WORK * syswk )
{
	BOX2BMP_ButtonPutYameru( syswk );
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_RetButtonOutPut( BGWINFRM_WORK * wk )
{
//	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_RET_BTN );
//	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_OUT_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_RetButtonOutSet( BGWINFRM_WORK * wk )
{
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, 1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_RetButtonInSet( BGWINFRM_WORK * wk )
{
	// 手持ちフレーム動作で位置が変更されている可能性があるので、再設定
//	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_OUT_PY );
//	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, -1, BOXPARTY_BTN_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * 「とじる」ボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_CloseButtonPutCheck( BGWINFRM_WORK * wk )
{
/*
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_CLOSE_BTN, &px, &py );
	if( py == WINFRM_MODORU_PY ){
		return TRUE;
	}
	return FALSE;
*/
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモン」「ポケモンいどう」ボタン非表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxPartyButtonVanish( BGWINFRM_WORK * wk )
{
/*
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_POKE_BTN );
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_MV_BTN );

	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_POKE_BTN,
		WINFRM_TEMOCHI_PX, WINFRM_TEMOCHI_PY+BOXPARTY_BTN_CNT );
	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_MV_BTN,
		WINFRM_IDOU_PX, WINFRM_IDOU_PY+BOXPARTY_BTN_CNT );
*/
}


//============================================================================================
//	上画面：技フレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレームインセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispWazaFrmInSet( BOX2_APP_WORK * appwk )
{
/*
	if( appwk->subdisp_waza_put == 0 ){ return; }
	BGWINFRM_MoveInit( appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP, -1, 0, SUBDISP_WAZA_FRM_SX );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレームアウトセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispWazaFrmOutSet( BOX2_APP_WORK * appwk )
{
/*
	if( appwk->subdisp_waza_put == 0 ){ return; }
	BGWINFRM_MoveInit( appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP, 1, 0, SUBDISP_WAZA_FRM_SX );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレーム画面内に配置
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispWazaFrmInPosSet( BOX2_APP_WORK * appwk )
{
/*
	if( appwk->subdisp_waza_put == 0 ){ return; }
	BGWINFRM_FrameOff( appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP );
	BGWINFRM_FramePut(
		appwk->wfrm, BOX2MAIN_WINFRM_SUBDISP,
		SUBDISP_WAZA_FRM_IN_PX-SUBDISP_WAZA_FRM_SX, SUBDISP_WAZA_FRM_IN_PY );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレーム画面外に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispWazaFrmOutPosSet( BGWINFRM_WORK * wk )
{
/*
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_SUBDISP );
	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_SUBDISP, SUBDISP_WAZA_FRM_IN_PX, SUBDISP_WAZA_FRM_IN_PY );
*/
}


//============================================================================================
//	上画面：持ち物フレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispItemFrmInSet( BGWINFRM_WORK * wk )
{
/*
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_SUBDISP, &px, &py );
	if( py == SUBDISP_ITEM_FRM_IN_PY-SUBDISP_ITEM_FRM_SY ){
		return;
	}
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_SUBDISP, 0, -1, SUBDISP_ITEM_FRM_SY-(SUBDISP_ITEM_FRM_IN_PY-py) );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispItemFrmOutSet( BGWINFRM_WORK * wk )
{
/*
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_SUBDISP, &px, &py );
	if( py == SUBDISP_ITEM_FRM_IN_PY ){
		return;
	}
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_SUBDISP, 0, 1, SUBDISP_ITEM_FRM_IN_PY-py );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームを画面内に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispItemFrmInPosSet( BGWINFRM_WORK * wk )
{
/*
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_SUBDISP );
	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_SUBDISP, SUBDISP_ITEM_FRM_IN_PX, SUBDISP_ITEM_FRM_IN_PY-SUBDISP_ITEM_FRM_SY );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームを画面外に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispItemFrmOutPosSet( BGWINFRM_WORK * wk )
{
/*
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_SUBDISP );
	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_SUBDISP, SUBDISP_ITEM_FRM_IN_PX, SUBDISP_ITEM_FRM_IN_PY );
*/
}


//============================================================================================
//	マーキングフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * マーク配置
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		マーク位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MarkingScrPut( BOX2_SYS_WORK * syswk, u32 pos )
{
	u16 * scr;
	u16	chr;
	u8	x, y;

	x = MARK_DEF_PX + MARK_DEF_SX * ( pos & 1 );
	y = MARK_DEF_PY + MARK_DEF_SY * ( pos / 2 );

	scr = BGWINFRM_FrameBufGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK ) + MARKING_FRM_SX * y + x;

	if( ( syswk->app->pokeMark & (1<<pos) ) == 0 ){
		chr = MARKING_OFF_CHR + pos;
	}else{
		chr = MARKING_ON_CHR + pos;
	}

	*scr = ( *scr & 0xf000 ) + chr;
}

//--------------------------------------------------------------------------------------------
/**
 * マークキングフレーム表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_MarkingFramePut( BOX2_SYS_WORK * syswk )
{
	u32	i;

	for( i=0; i<POKEPARA_MARKING_ELEMS_MAX; i++ ){
		MarkingScrPut( syswk, i );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * マーク表示切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		マーク位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos )
{
	syswk->app->pokeMark ^= (1<<pos);
	MarkingScrPut( syswk, pos );
	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK, WINFRM_MARK_PX, WINFRM_MARK_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_MarkingFrameInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MARK, WINFRM_MARK_PX, WINFRM_MARK_IN_START_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MARK, 0, -1, WINFRM_MARK_MV_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_MarkingFrameOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MARK, 0, 1, WINFRM_MARK_MV_CNT );
}


//============================================================================================
//	壁紙変更フレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更フレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_WallPaperFrameInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_WPCHG, WINFRM_WALLPAPER_PX, WINFRM_WALLPAPER_IN_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_WPCHG, 0, 1, WPCHG_FRM_SY );
}

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更フレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_WallPaperFrameOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_WPCHG, WINFRM_WALLPAPER_PX, WINFRM_WALLPAPER_OUT_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_WPCHG, 0, -1, WPCHG_FRM_SY );
}


//============================================================================================
//	ボックス切り替えメニュー
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ボックス切り替えメニューボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	壁紙変更、預けるボックス選択で使用してます
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveMenuInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_BOXMV_MENU, WINFRM_BOXMV_MENU_INIT_PX, WINFRM_BOXMV_MENU_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_MENU, -1, 0, WINFRM_BOXMV_MENU_SX );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス切り替えメニューボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	壁紙変更、預けるボックス選択で使用してます
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveMenuOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_BOXMV_MENU, WINFRM_BOXMV_MENU_PX, WINFRM_BOXMV_MENU_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_MENU, 1, 0, WINFRM_BOXMV_MENU_SX );
}


//============================================================================================
//	トレイ選択フレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレームを画面内へ配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 *
 *	他画面から復帰時にボックス移動フレームを表示
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveFrmPut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_ButtonPutYameru( syswk );

//	BGWINFRM_FrameOff( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	BGWINFRM_FrameOff( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN, WINFRM_TEMOCHI_PX, WINFRM_TEMOCHI_OUT_PY );
//	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN, WINFRM_IDOU_PX, WINFRM_IDOU_OUT_PY );

	BGWINFRM_FramePut( syswk->app->wfrm, BOX2MAIN_WINFRM_MOVE, 0, 0 );

//	BOX2BMP_BoxMoveNameWrite( syswk );
	BOX2OBJ_BoxMoveObjInit( syswk );
	BOX2OBJ_BoxMoveCursorSet( syswk );
//	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2OBJ_BoxMoveFrmScroll( syswk, 8 * BOXMOVE_FRM_SY );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレーム画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveFrmInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MOVE, WINFRM_TRAYMOVE_IN_PX, WINFRM_TRAYMOVE_IN_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MOVE, -1, 0, BOXMOVE_FRM_SX );
}

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレーム画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveFrmOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MOVE, 1, 0, BOXMOVE_FRM_SX );
}

// トレイ選択フレームが表示されているか
BOOL BOX2BGWFRM_CheckBoxMoveFrm( BGWINFRM_WORK * wk )
{
	s8	x, y;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MOVE, &x, &y );
	if( x == WINFRM_TRAYMOVE_OUT_PX ){
		return TRUE;
	}
	return FALSE;
}



//============================================================================================
// ボックステーマ変更フレーム表示
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更メニュー表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxThemaMenuOn( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_BOXMV_MENU, WINFRM_BOXTHEMA_MENU_PX, WINFRM_BOXTHEMA_MENU1_PY );
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU4, WINFRM_BOXTHEMA_MENU_PX, WINFRM_BOXTHEMA_MENU2_PY );
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU5, WINFRM_BOXTHEMA_MENU_PX, WINFRM_BOXTHEMA_MENU3_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレームを画面内へ配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 *
 *	他画面から復帰時にボックステーマ変更関連フレームを表示
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxThemaFrmPut( BOX2_SYS_WORK * syswk )
{
	BOX2BGWFRM_BoxMoveFrmPut( syswk );

	BOX2BMP_BoxThemaMenuPrint( syswk );
	BoxThemaMenuOn( syswk->app->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレーム画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxThemaMenuInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_BOXMV_MENU, WINFRM_BOXTHEMA_MENU_OUT_PX, WINFRM_BOXTHEMA_MENU1_PY );
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU4, WINFRM_BOXTHEMA_MENU_OUT_PX, WINFRM_BOXTHEMA_MENU2_PY );
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU5, WINFRM_BOXTHEMA_MENU_OUT_PX, WINFRM_BOXTHEMA_MENU3_PY );

	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_MENU, -1, 0, WINFRM_BOXTHEMA_MENU_SX );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU4, -1, 0, WINFRM_BOXTHEMA_MENU_SX );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU5, -1, 0, WINFRM_BOXTHEMA_MENU_SX );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレーム画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxThemaMenuOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_MENU, 1, 0, WINFRM_BOXTHEMA_MENU_SX );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU4, 1, 0, WINFRM_BOXTHEMA_MENU_SX );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU5, 1, 0, WINFRM_BOXTHEMA_MENU_SX );
}

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモン」「ポケモンいどう」を非表示にする
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_ArrangeUnderButtonDel( BOX2_SYS_WORK * syswk )
{
//	BGWINFRM_FrameOff( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	BGWINFRM_FrameOff( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );

//	BOX2BGWFRM_ButtonOutPutTemochi( syswk );
//	BOX2BGWFRM_ButtonOutPutIdou( syswk );
}


//--------------------------------------------------------------------------------------------
/**
 * サブ画面ウィンドウ動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_SubDispWinFrmMove( BOX2_SYS_WORK * syswk )
{
/*
	if( syswk->dat->callMode != BOX_MODE_ITEM ){
		BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP );
	}else{
		BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP );
		BOX2OBJ_ItemIconPutSub( syswk->app );
	}
*/
}
