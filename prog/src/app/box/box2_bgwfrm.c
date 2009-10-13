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
#include "box2_bgwfrm.h"
#include "box_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================

// マーキング
#define	MARKING_FRM_SX	( 11 )						// マーキングフレームＸサイズ
#define	MARKING_FRM_SY	( 18 )						// マーキングフレームＹサイズ
//#define	MARKING_FRM_PX	( 32 - MARKING_FRM_SX )		// マーキングフレームＸ表示座標
//#define	MARKING_FRM_PY	( 26 )						// マーキングフレームＸ表示座標
/*
#define	MARKING_ON_CHR	( 0x2b )					// マーキングキャラ開始位置 ( ON )
#define	MARKING_OFF_CHR	( 0x0b )					// マーキングキャラ開始位置 ( OFF )
*/
// 手持ちポケモンフレーム
#define	PARTYPOKE_FRM_SX	( 11 )		// 手持ちポケモンフレームＸサイズ
#define	PARTYPOKE_FRM_SY	( 18 )		// 手持ちポケモンフレームＹサイズ
//#define	PARTYPOKE_FRM_PX	( 2 )		// 手持ちポケモンフレームＸ表示座標
//#define	PARTYPOKE_FRM_PY	( 21 )		// 手持ちポケモンフレームＹ表示座標

// ボックス移動フレーム
#define BOXMOVE_FRM_SX		( 32 )		// ボックス移動フレームＸサイズ
#define BOXMOVE_FRM_SY		( 6 )		// ボックス移動フレームＹサイズ
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
//#define	WINFRM_MENU_PX		( 21 )		// 表示Ｘ座標
#define	WINFRM_MENU_PY		( 5 )		// 表示Ｙ座標
#define	WINFRM_MENU_SY		( 3 )		// 表示Ｙサイズ

//「もどる」フレームデータ
#define	WINFRM_MODORU_PX		( 24 )
#define	WINFRM_MODORU_PY		( 21 )
//#define	WINFRM_MODORU_OUT_PY	( 24 )

//「ボックスをきりかえる」ボタンフレームデータ
#define	WINFRM_BOXCHG_BTN_PX			( 0 )
//#define	WINFRM_BOXCHG_BTN_PY			( 21 )
#define	WINFRM_BOXCHG_BTN_INIT_PY	( 24 )

//「Ｙようすをみる」ボタンフレームデータ
#define	WINFRM_Y_ST_BTN_PX			( 0 )
//#define	WINFRM_Y_ST_BTN_PY			( 21 )
#define	WINFRM_Y_ST_BTN_INIT_PY	( 24 )


//#define	BOXPARTY_BTN_SPD	( 8 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロール速度
#define	BOXPARTY_BTN_CNT	( 3 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロールカウント


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void FrameArcLoad( BGWINFRM_WORK * wk, u32 index, u32 dataIdx );
static void PokeMenuInitPosSet( BGWINFRM_WORK * wk );
static void BoxMoveButtonInitPut( BGWINFRM_WORK * wk );
static void YStatusButtonInitPut( BGWINFRM_WORK * wk );



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

// 壁紙変更フレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_WPCHG, GFL_BG_FRAME1_M, WPCHG_FRM_SX, WPCHG_FRM_SY );
	// 手持ちポケモンフレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, GFL_BG_FRAME1_M, PARTYPOKE_FRM_SX, PARTYPOKE_FRM_SY );
	// マーキングフレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MARK, GFL_BG_FRAME0_M, MARKING_FRM_SX, MARKING_FRM_SY );
// ボックス移動フレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_BOXMV_MENU, GFL_BG_FRAME0_M, BOX2BMP_BOXMVMENU_SX, BOX2BMP_BOXMVMENU_SY );

	BOX2BMP_PokeMenuBgFrmWkMake( appwk );				// メニュー

	BOX2BMP_TemochiButtonBgFrmWkMake( appwk );	//「」
	BOX2BMP_IdouButtonBgFrmWkMake( appwk );			//「」
	BOX2BMP_ModoruButtonBgFrmWkMake( appwk );		//「」
	BOX2BMP_ToziruButtonBgFrmWkMake( syswk );		//「」

	if( syswk->dat->mode == BOX_MODE_AZUKERU ){
		BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, GFL_BG_FRAME0_M, BOXMV_PTOUT_FRM_SX, BOXMV_PTOUT_FRM_SY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, NARC_box_gra_box_partyout_bg_lz_NSCR );
	}else{
		BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, GFL_BG_FRAME1_M, BOXMOVE_FRM_SX, BOXMOVE_FRM_SY );
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, NARC_box_gra_box_move_bg_lz_NSCR );
	}

	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_WPCHG, NARC_box_gra_box_wpchg_bg_lz_NSCR );

	if( syswk->dat->mode == BOX_MODE_AZUKERU || syswk->dat->mode == BOX_MODE_TURETEIKU ){
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, NARC_box_gra_box_poke_bg2_lz_NSCR );
	}else{
		BOX2BGWFRM_PartyPokeFrameLoadArrange( appwk->wfrm, BOX2MAIN_WINFRM_PARTY );
//		BOX2BMP_PartyCngButtonFrmPut( appwk );
	}

	if( syswk->dat->mode == BOX_MODE_ITEM ){
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

	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MARK, NARC_box_gra_box_mark_bg_lz_NSCR );
//	BOX2BMP_MarkingButtonFrmPut( syswk->app );

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
/*
void BOX2MAIN_PartyPokeFramePut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_LX, WINFRM_PARTYPOKE_PY );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム表示（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFramePutRight( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_RX, WINFRM_PARTYPOKE_PY );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム非表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameOff( BOX2_SYS_WORK * syswk )
{
	GFL_BG_FillScreen(
//↑[GS_CONVERT_TAG]
		syswk->app->bgl, BOX2MAIN_BGF_BTN_M, 0,
		PARTYPOKE_FRM_PX, PARTYPOKE_FRM_PY,
		PARTYPOKE_FRM_SX, PARTYPOKE_FRM_SY, GFL_BG_SCRWRT_PALNL );
//↑[GS_CONVERT_TAG]
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（左）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameInitPutLeft( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_LX, WINFRM_PARTYPOKE_INIT_PY );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameInitPutRight( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_PARTY, WINFRM_PARTYPOKE_RX, WINFRM_PARTYPOKE_INIT_PY );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, -1, py-WINFRM_PARTYPOKE_PY );
		if( px == WINFRM_PARTYPOKE_LX ){
			BGWINFRM_FramePut(
				wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_PARTYPOKE_RET_PX, py+WINFRM_PARTYPOKE_RET_PY );
			BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, -1, py-WINFRM_PARTYPOKE_PY );
		}
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_INIT_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
		if( px == WINFRM_PARTYPOKE_LX ){
			BGWINFRM_FramePut(
				wk, BOX2MAIN_WINFRM_RET_BTN, WINFRM_PARTYPOKE_RET_PX, py+WINFRM_PARTYPOKE_RET_PY );
			BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
		}
	}
}
*/

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
/*
static void PartyPokeFrameOnlyOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_INIT_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, 1, WINFRM_PARTYPOKE_INIT_PY-py );
	}
}
*/

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
/*
static void PartyPokeFrameOnlyInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &px, &py );
	if( py != WINFRM_PARTYPOKE_PY ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 0, -1, py-WINFRM_PARTYPOKE_PY );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_AzukeruPartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );

	BGWINFRM_FramePut(
		wk, BOX2MAIN_WINFRM_RET_BTN,
		WINFRM_PARTYPOKE_RET_PX+BOX2MAIN_PARTYPOKE_FRM_H_CNT,
		WINFRM_PARTYPOKE_PY+WINFRM_PARTYPOKE_RET_PY );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_RET_BTN, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_AzukeruPartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}
*/

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
/*
static BOOL PartyPokeFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	party_mv, ret_mv;
	s8	x1, y1, x2, y2;
	BOOL	ret;

	BGWINFRM_PosGet( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY, &x1, &y1 );

	party_mv = BGWINFRM_MoveOne( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY );
	ret_mv   = BGWINFRM_MoveOne( syswk->app->wfrmwk, BOX2MAIN_WINFRM_RET_BTN );
	BGWINFRM_MoveOne( syswk->app->wfrmwk, BOX2MAIN_WINFRM_Y_ST_BTN );

	BGWINFRM_PosGet( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY, &x2, &y2 );

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
*/

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
/*
static BOOL AzukeruPartyPokeFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	party_mv;
	s8	x1, y1, x2, y2;
	BOOL	ret;

	BGWINFRM_PosGet( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY, &x1, &y1 );

	party_mv = BGWINFRM_MoveOne( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY );

	BGWINFRM_PosGet( syswk->app->wfrmwk, BOX2MAIN_WINFRM_PARTY, &x2, &y2 );

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
*/

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

	for( i=0; i<5; i++ ){
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_INIT_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT );
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
/*
void BOX2MAIN_PokeMenuOpenPosSet( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<5; i++ ){
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン非表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PokeMenuOff( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<5; i++ ){
		BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_MENU1+i );
		BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_MENU1+i, WINFRM_MENU_INIT_PX, WINFRM_MENU_PY+i*WINFRM_MENU_SY );
	}
	BGWINFRM_FrameOff( wk, BOX2MAIN_WINFRM_CLOSE_BTN );
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_CLOSE_BTN, WINFRM_MODORU_PX, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PokeMenuInSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_PX ){ return; }

	for( i=0; i<5; i++ ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU1+i, -1, 0, px-WINFRM_MENU_PX );
	}

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_CLOSE_BTN, &px, &py );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_CLOSE_BTN, 0, -1, py-WINFRM_MODORU_PY );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_PokeMenuOutSet( BGWINFRM_WORK * wk )
{
	u16	i;
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_INIT_PX ){ return; }

	for( i=0; i<5; i++ ){
		BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MENU1+i, 1, 0, WINFRM_MENU_INIT_PX-px );
	}

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_CLOSE_BTN, &px, &py );
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_CLOSE_BTN, 0, 1, WINFRM_MODORU_PY+BOXPARTY_BTN_CNT-py );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニュー移動
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FLASE = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
static BOOL PokeMenuMoveMain( BGWINFRM_WORK * wk )
{
	u32		i;
	BOOL	ret;
	
	ret = FALSE;

	for( i=0; i<5; i++ ){
		if( BGWINFRM_MoveOne( wk, BOX2MAIN_WINFRM_MENU1+i ) == 1 ){
			ret = TRUE;
		}
	}
	if( BGWINFRM_MoveOne( wk, BOX2MAIN_WINFRM_CLOSE_BTN ) == 1 ){
		ret = TRUE;
	}

	return ret;
}
*/
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
/*
BOOL BOX2MAIN_PokeMenuPutCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_INIT_PX ){
		return FALSE;
	}
	return TRUE;
}
*/


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
/*
void BOX2MAIN_BoxMoveButtonInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_BTN, 0, -1, BOXPARTY_BTN_CNT );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_BoxMoveButtonOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_BOXMV_BTN, 0, 1, BOXPARTY_BTN_CNT );
}
*/
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
/*
BOOL BOX2MAIN_BoxMoveButtonCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_BOXMV_BTN, &px, &py );
	if( py == WINFRM_BOXCHG_BTN_PY ){
		return TRUE;
	}
	return FALSE;
}
*/

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
/*
void BOX2MAIN_YStatusButtonPut( BGWINFRM_WORK * wk )
{
	BGWINFRM_FramePut( wk, BOX2MAIN_WINFRM_Y_ST_BTN, WINFRM_Y_ST_BTN_PX, WINFRM_Y_ST_BTN_INIT_PY-BOXPARTY_BTN_CNT );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_YStatusButtonInSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_Y_ST_BTN, 0, -1, BOXPARTY_BTN_CNT );
}
*/
//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BOX2MAIN_YStatusButtonOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_Y_ST_BTN, 0, 1, BOXPARTY_BTN_CNT );
}
*/
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
/*
BOOL BOX2MAIN_YStatusButtonCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_Y_ST_BTN, &px, &py );
	if( py == WINFRM_Y_ST_BTN_PY ){
		return TRUE;
	}
	return FALSE;
}
*/
