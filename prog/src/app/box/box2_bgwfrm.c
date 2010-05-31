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

// 手持ちポケモンフレーム
#define	PARTYPOKE_FRM_SX					( BOX2BGWFRM_PARTYPOKE_SX )		// 手持ちポケモンフレームＸサイズ
#define	PARTYPOKE_FRM_SY					( 15 )												// 手持ちポケモンフレームＹサイズ
#define	PARTYPOKE_FRM_PX					( 2 )													// 手持ちポケモンフレームＸ表示座標
#define	PARTYPOKE_FRM_PY					( 21 )												// 手持ちポケモンフレームＹ表示座標
#define	WINFRM_PARTYPOKE_LX				( BOX2BGWFRM_PARTYPOKE_LX )
#define	WINFRM_PARTYPOKE_RX				( 21 )
#define	WINFRM_PARTYPOKE_PY				( BOX2BGWFRM_PARTYPOKE_PY )
#define	WINFRM_PARTYPOKE_INIT_PY	( 24 )
#define	WINFRM_PARTYPOKE_RET_PX		( 24 )
#define	WINFRM_PARTYPOKE_RET_PY		( 15 )

// ボックス移動フレーム
#define BOXMOVE_FRM_SX			( 11 )		// ボックス移動フレームＸサイズ
#define BOXMOVE_FRM_SY			( 21 )		// ボックス移動フレームＹサイズ
#define BOXMV_PTOUT_FRM_SX	( 32 )		// ボックス移動フレームＸサイズ
#define BOXMV_PTOUT_FRM_SY	( 7 )			// ボックス移動フレームＹサイズ

// ポケモン選択メニューフレームデータ
#define	WINFRM_MENU_INIT_PX	( 32 )		// 表示Ｘ座標
#define	WINFRM_MENU_PX			( 21 )		// 表示Ｘ座標
#define	WINFRM_MENU_PY			( 2 )		// 表示Ｙ座標
#define	WINFRM_MENU_SY			( 3 )		// 表示Ｙサイズ

//「てもちポケモン」フレームデータ
#define	WINFRM_TEMOCHI_PX		( 0 )
#define	WINFRM_TEMOCHI_PY		( 21 )
//「ボックスリスト」フレームデータ
#define	WINFRM_BOXLIST_PX		( 0 )
#define	WINFRM_BOXLIST_PY		( 21 )

// マーキングフレームデータ
#define	WINFRM_MARK_PX					( 21 )
#define	WINFRM_MARK_PY					( 5 )
#define	WINFRM_MARK_IN_START_PY	( 24 )
#define	WINFRM_MARK_MV_CNT			( WINFRM_MARK_IN_START_PY - WINFRM_MARK_PY )

// トレイ選択フレームデータ
#define	WINFRM_TRAYMOVE_IN_PX		( 32 )
#define	WINFRM_TRAYMOVE_IN_PY		( 0 )
#define	WINFRM_TRAYMOVE_OUT_PX	( 21 )
#define	WINFRM_TRAYMOVE_OUT_PY	( 0 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void FrameArcLoad( BGWINFRM_WORK * wk, u32 index, u32 dataIdx );
static void PokeMenuInitPosSet( BGWINFRM_WORK * wk );
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

	BOX2BMP_TemochiButtonBgFrmWkMake( syswk );	//「てもちポケモン」or「バトルボックス」
	BOX2BMP_BoxListButtonBgFrmWkMake( syswk );	//「ボックスリスト」

	// 手持ちポケモンフレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, GFL_BG_FRAME1_M, PARTYPOKE_FRM_SX, PARTYPOKE_FRM_SY );
	if( syswk->dat->callMode == BOX_MODE_BATTLE ){
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, NARC_box_gra_box_poke_bg_lz_NSCR );
	}else{
		FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_PARTY, NARC_box_gra_box_poke_bg2_lz_NSCR );
	}

	// マーキングフレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MARK, GFL_BG_FRAME1_M, MARKING_FRM_SX, MARKING_FRM_SY );
	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MARK, NARC_box_gra_box_mark_bg_lz_NSCR );
	BOX2BMP_MarkingButtonFrmPut( syswk );

	// ボックス移動フレーム
	BGWINFRM_Add( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, GFL_BG_FRAME1_M, BOXMOVE_FRM_SX, BOXMOVE_FRM_SY );
	FrameArcLoad( appwk->wfrm, BOX2MAIN_WINFRM_MOVE, NARC_box_gra_box_move_bg_lz_NSCR );

	PokeMenuInitPosSet( appwk->wfrm );
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
 * @brief		ＢＧフレームにアーカイブのグラフィックを設定
 *
 * @param		wk				ＢＧウィンドウフレームワーク
 * @param		ah				アーカイブハンドル
 * @param		dataIdx		データインデックス
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
 * @brief		手持ちポケモンフレーム表示
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		手持ちポケモンフレーム表示（右）
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		手持ちポケモンフレーム初期位置設定（左）
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		手持ちポケモンフレーム初期位置設定（右）
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		手持ちポケモンフレームインセット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームアウトセット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレーム右移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, 1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレーム左移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_PartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_PARTY, -1, 0, BOX2MAIN_PARTYPOKE_FRM_H_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレーム移動
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_PartyPokeFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	party_mv;
	s8	x1, y1, x2, y2;

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x1, &y1 );

	party_mv = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY );

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY, &x2, &y2 );

	if( x1 != x2 || y1 != y2 ){
		BOX2OBJ_PartyPokeIconScroll( syswk );
	}

	if( party_mv == 0 ){
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームが右にあるか
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = あり"
 * @retval	"FALSE = なし"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_CheckPartyPokeFrameRight( BGWINFRM_WORK * wk )
{
	s8	x, y;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &x, &y );
	if( x == WINFRM_PARTYPOKE_RX && y == WINFRM_PARTYPOKE_PY ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームが左にあるか
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = あり"
 * @retval	"FALSE = なし"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_CheckPartyPokeFrameLeft( BGWINFRM_WORK * wk )
{
	s8	x, y;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &x, &y );
	if( x == WINFRM_PARTYPOKE_LX && y == WINFRM_PARTYPOKE_PY ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームが表示中か
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = あり"
 * @retval	"FALSE = なし"
 *
 * @li	Ｘ座標は見ていない
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_CheckPartyPokeFrame( BGWINFRM_WORK * wk )
{
	s8	x, y;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_PARTY, &x, &y );
	if( y == WINFRM_PARTYPOKE_PY ){
		return TRUE;
	}
	return FALSE;
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
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちポケモンフレームが左にあるかポケモンメニューボタンを画面内に配置
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニューボタン非表示
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニューボタン画面内への移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニューボタン画面外への移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニュー移動チェック
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニュー配置チェック
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンメニュー配置チェック（完全に開いた状態か）
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FLASE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_PokeMenuOpenPutCheck( BGWINFRM_WORK * wk )
{
	s8	px, py;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MENU1, &px, &py );
	if( px == WINFRM_MENU_PX ){
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
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		「てもちポケモン」ボタン配置
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_TemochiButtonOn( BOX2_APP_WORK * appwk )
{
	BGWINFRM_FramePut( appwk->wfrm, BOX2MAIN_WINFRM_POKE_BTN, WINFRM_TEMOCHI_PX, WINFRM_TEMOCHI_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「てもちポケモン」ボタン非表示
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_TemochiButtonOff( BOX2_APP_WORK * appwk )
{
	BGWINFRM_FrameOff( appwk->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスリスト」ボタン配置
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxListButtonOn( BOX2_APP_WORK * appwk )
{
	BGWINFRM_FramePut( appwk->wfrm, BOX2MAIN_WINFRM_BOXLIST_BTN, WINFRM_BOXLIST_PX, WINFRM_BOXLIST_PY );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスリスト」ボタン非表示
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxListButtonOff( BOX2_APP_WORK * appwk )
{
	BGWINFRM_FrameOff( appwk->wfrm, BOX2MAIN_WINFRM_BOXLIST_BTN );
}


//============================================================================================
//	マーキングフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレームインセット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		マーキングフレームアウトセット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_MarkingFrameOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MARK, 0, 1, WINFRM_MARK_MV_CNT );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレーム移動
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2BGWFRM_MarkingFrameMove( BOX2_SYS_WORK * syswk )
{
	u32	mv;
	s8	x1, y1, x2, y2;

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK, &x1, &y1 );

	mv = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK );

	BGWINFRM_PosGet( syswk->app->wfrm, BOX2MAIN_WINFRM_MARK, &x2, &y2 );

	if( x1 != x2 || y1 != y2 ){
		BOX2OBJ_MarkingScroll( syswk );
	}

	return mv;
}


//============================================================================================
//	トレイ選択フレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ選択フレーム画面内への移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
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
 * @brief		トレイ選択フレーム画面外への移動セット
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2BGWFRM_BoxMoveFrmOutSet( BGWINFRM_WORK * wk )
{
	BGWINFRM_MoveInit( wk, BOX2MAIN_WINFRM_MOVE, 1, 0, BOXMOVE_FRM_SX );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ選択フレームが表示されているか
 *
 * @param		wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 表示されている"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2BGWFRM_CheckBoxMoveFrm( BGWINFRM_WORK * wk )
{
	s8	x, y;

	BGWINFRM_PosGet( wk, BOX2MAIN_WINFRM_MOVE, &x, &y );
	if( x == WINFRM_TRAYMOVE_OUT_PX ){
		return TRUE;
	}
	return FALSE;
}
