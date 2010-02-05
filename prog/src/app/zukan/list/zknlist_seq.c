//============================================================================================
/**
 * @file		zknlist_seq.c
 * @brief		図鑑リスト画面 シーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTSEQ
 */
//============================================================================================
#include <gflib.h>

#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "app/app_menu_common.h"

#include "zknlist_main.h"
#include "zknlist_seq.h"
#include "zknlist_obj.h"
#include "zknlist_bmp.h"
#include "zknlist_ui.h"
#include "zknlist_bgwfrm.h"
#include "zknlist_snd_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_INIT_LIST_WAIT,

//	MAINSEQ_INIT,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,

	MAINSEQ_MAIN,
	MAINSEQ_PAGE_MOVE,

/*
	MAINSEQ_LIST_WAIT,
	MAINSEQ_LIST_SCROLL,
	MAINSEQ_LIST_MOVE_LEFT,
	MAINSEQ_LIST_MOVE_RIGHT,

	MAINSEQ_LIST_MOVE_TOUCH,
	MAINSEQ_LIST_SCROLL_TOUCH,
	MAINSEQ_LIST_AUTO_SCROLL,

	MAINSEQ_LIST_MOVE_BAR,
*/

	MAINSEQ_BUTTON_ANM,
	MAINSEQ_ITEM_ANM,

	MAINSEQ_END_SET,
	MAINSEQ_END,
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( ZKNLISTMAIN_WORK * wk );
static int MainSeq_InitListWait( ZKNLISTMAIN_WORK * wk );
static int MainSeq_Main( ZKNLISTMAIN_WORK * wk );
static int MainSeq_PageMove( ZKNLISTMAIN_WORK * wk );

//static int MainSeq_Init( ZKNLISTMAIN_WORK * wk );
static int MainSeq_Release( ZKNLISTMAIN_WORK * wk );
static int MainSeq_Wipe( ZKNLISTMAIN_WORK * wk );
//static int MainSeq_Main( ZKNLISTMAIN_WORK * wk );
/*
static int MainSeq_ListWait( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListScroll( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListMoveLeft( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListMoveRight( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListMoveTouch( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListScrollTouch( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListAutoScroll( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ListMoveBar( ZKNLISTMAIN_WORK * wk );
*/
static int MainSeq_ButtonAnm( ZKNLISTMAIN_WORK * wk );
static int MainSeq_ItemAnm( ZKNLISTMAIN_WORK * wk );
static int MainSeq_EndSet( ZKNLISTMAIN_WORK * wk );

static int SetFadeIn( ZKNLISTMAIN_WORK * wk, int next );
static int SetFadeOut( ZKNLISTMAIN_WORK * wk, int next );
static int SetButtonAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm, int next );
static BOOL CheckInfoData( ZKNLISTMAIN_WORK * wk, int pos );
static int SetInfoData( ZKNLISTMAIN_WORK * wk, int pos );
static void SetShortCut( ZKNLISTMAIN_WORK * wk );
/*
static int SetListMoveBar( ZKNLISTMAIN_WORK * wk );
static int SetListMoveTouch( ZKNLISTMAIN_WORK * wk );
static u32 GetTouchPos( u32 tpy );
*/

static void ScrollBaseBg( ZKNLISTMAIN_WORK * wk );

FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

static const pZKNLIST_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_InitListWait,

//	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,

	MainSeq_Main,
	MainSeq_PageMove,

/*
	MainSeq_ListWait,
	MainSeq_ListScroll,
	MainSeq_ListMoveLeft,
	MainSeq_ListMoveRight,

	MainSeq_ListMoveTouch,
	MainSeq_ListScrollTouch,
	MainSeq_ListAutoScroll,

	MainSeq_ListMoveBar,
*/
	MainSeq_ButtonAnm,
	MainSeq_ItemAnm,

	MainSeq_EndSet,
};




BOOL ZKNLISTSEQ_MainSeq( ZKNLISTMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	ZKNLISTOBJ_AnmMain( wk );
	ZKNLISTBMP_PrintUtilTrans( wk );
//	BGWINFRM_MoveMain( wk->wfrm );
	ScrollBaseBg( wk );

	return TRUE;
}

static int MainSeq_Init( ZKNLISTMAIN_WORK * wk )
{
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	ZKNLISTMAIN_InitVram();
	ZKNLISTMAIN_InitBg();
	ZKNLISTMAIN_LoadBgGraphic();
	ZKNLISTMAIN_InitMsg( wk );

	ZKNLISTBMP_Init( wk );
	ZKNLISTOBJ_Init( wk );
//	ZKNLISTBGWFRM_Init( wk );

	ZKNLISTMAIN_MakeList( wk );

	ZKNLISTBMP_PutPokeEntryStr( wk );

	ZKNLISTMAIN_SetBlendAlpha();

	ZKNLISTMAIN_InitVBlank( wk );
	ZKNLISTMAIN_InitHBlank( wk );

	return MAINSEQ_INIT_LIST_WAIT;
}

static int MainSeq_InitListWait( ZKNLISTMAIN_WORK * wk )
{
	if( FRAMELIST_Init( wk->lwk ) == FALSE ){
		ZKNLISTOBJ_SetPutPokeIconFlag( wk );
		return SetFadeIn( wk, MAINSEQ_MAIN );
	}
	return MAINSEQ_INIT_LIST_WAIT;
}

static int MainSeq_Main( ZKNLISTMAIN_WORK * wk )
{
	int	seq;
	u32	ret;

	seq = MAINSEQ_MAIN;
	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
//		PMSND_PlaySE( ZKNLIST_SE_DECIDE );
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
		ZKNLISTOBJ_VanishJumpPokeIcon( wk );
		break;

	case FRAMELIST_RET_NONE:				// 動作なし

		ret = ZKNLISUI_ListMain( wk );

		switch( ret ){
		case ZKNLISTUI_ID_POKE:				// 02: ポケモン正面絵
			{
				int	pos = FRAMELIST_GetListPos( wk->lwk );
				if( CheckInfoData( wk, pos ) == TRUE ){
					PMSND_PlaySE( ZKNLIST_SE_DECIDE );
					return SetInfoData( wk, pos );
				}
			}
			break;

		case ZKNLISTUI_ID_ICON1:			// 03: ポケモンアイコン
		case ZKNLISTUI_ID_ICON2:			// 04: ポケモンアイコン
		case ZKNLISTUI_ID_ICON3:			// 05: ポケモンアイコン
		case ZKNLISTUI_ID_ICON4:			// 06: ポケモンアイコン
		case ZKNLISTUI_ID_ICON5:			// 07: ポケモンアイコン
		case ZKNLISTUI_ID_ICON6:			// 08: ポケモンアイコン
		case ZKNLISTUI_ID_ICON7:			// 09: ポケモンアイコン
			{
				int	pos = FRAMELIST_GetScrollCount( wk->lwk ) + ret - ZKNLISTUI_ID_ICON1;
				if( CheckInfoData( wk, pos ) == TRUE ){
					PMSND_PlaySE( ZKNLIST_SE_DECIDE );
					FRAMELIST_SetCursorPos( wk->lwk, ret-ZKNLISTUI_ID_ICON1 );
					return SetInfoData( wk, pos );
				}
			}
			break;

		case ZKNLISTUI_ID_START:			// 10: スタート
			PMSND_PlaySE( ZKNLIST_SE_DECIDE );
			wk->dat->retMode = ZKNLIST_RET_SEARCH;
			return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_START, ZKNLISTOBJ_ANM_START_ANM, MAINSEQ_END_SET );

		case ZKNLISTUI_ID_SELECT:			// 11: セレクト
			PMSND_PlaySE( ZKNLIST_SE_DECIDE );
			return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_SELECT, ZKNLISTOBJ_ANM_SELECT_ANM, MAINSEQ_MAIN );

		case ZKNLISTUI_ID_Y:					// 14: Ｙ
			PMSND_PlaySE( ZKNLIST_SE_Y );
			SetShortCut( wk );
			break;

		case ZKNLISTUI_ID_X:					// 15: Ｘ
			PMSND_PlaySE( ZKNLIST_SE_CLOASE );
			wk->dat->retMode = ZKNLIST_RET_EXIT_X;
			return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_EXIT, APP_COMMON_BARICON_EXIT_ON, MAINSEQ_END_SET );

		case ZKNLISTUI_ID_RETURN:			// 16: 戻る
			PMSND_PlaySE( ZKNLIST_SE_CANCEL );
			wk->dat->retMode = ZKNLIST_RET_EXIT;
			return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON, MAINSEQ_END_SET );

		case ZKNLISTUI_ID_CANCEL:			// キャンセルボタン
			PMSND_PlaySE( ZKNLIST_SE_CANCEL );
			wk->dat->retMode = ZKNLIST_RET_EXIT;
			return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON, MAINSEQ_END_SET );
		}

		break;

	// 項目選択
	default:
		{
			int	pos = FRAMELIST_GetListPos( wk->lwk );
			if( CheckInfoData( wk, pos ) == TRUE ){
				PMSND_PlaySE( ZKNLIST_SE_DECIDE );
				return SetInfoData( wk, pos );
			}
		}
		break;
	}

	ZKNLISTOBJ_SetPutPokeIconFlag( wk );

	return seq;
}

static int MainSeq_PageMove( ZKNLISTMAIN_WORK * wk )
{
	if( FRAMELIST_Main( wk->lwk ) == FRAMELIST_RET_NONE ){
		ZKNLISTOBJ_SetListPageArrowAnime( wk, FALSE );
		return MAINSEQ_MAIN;
	}
	return MAINSEQ_PAGE_MOVE;
}

















/*
static int MainSeq_Init( ZKNLISTMAIN_WORK * wk )
{
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	ZKNLISTMAIN_InitVram();
	ZKNLISTMAIN_InitBg();
	ZKNLISTMAIN_LoadBgGraphic();
	ZKNLISTMAIN_InitMsg( wk );

	ZKNLISTBMP_Init( wk );
	ZKNLISTOBJ_Init( wk );
	ZKNLISTBGWFRM_Init( wk );

	ZKNLISTMAIN_MakeList( wk );

	ZKNLISTBMP_PutPokeEntryStr( wk );

	ZKNLISTMAIN_SetBlendAlpha();

	ZKNLISTMAIN_InitVBlank( wk );
	ZKNLISTMAIN_InitHBlank( wk );

	return SetFadeIn( wk, MAINSEQ_MAIN );
}
*/

static int MainSeq_Release( ZKNLISTMAIN_WORK * wk )
{
	ZKNLISTMAIN_ExitHBlank( wk );
	ZKNLISTMAIN_ExitVBlank( wk );

	ZKNLISTMAIN_FreeList( wk );

//	ZKNLISTBGWFRM_Exit( wk );
	ZKNLISTOBJ_Exit( wk );
	ZKNLISTBMP_Exit( wk );

	ZKNLISTMAIN_ExitMsg( wk );
	ZKNLISTMAIN_ExitBg();

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return MAINSEQ_END;
}

static int MainSeq_Wipe( ZKNLISTMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->wipeSeq;
	}
	return MAINSEQ_WIPE;
}

/*
static void SetListScrollSpeed( ZKNLISTMAIN_WORK * wk )
{
	if( wk->listRepeat < 40 ){
		wk->listRepeat++;
	}
	if( wk->listRepeat == 40 ){
		wk->listSpeed = 24;
	}else if( wk->listRepeat >= 32 ){
		wk->listSpeed = 12;
	}else if( wk->listRepeat >= 24 ){
		wk->listSpeed = 8;
	}else if( wk->listRepeat >= 16 ){
		wk->listSpeed = 6;
	}else if( wk->listRepeat >= 8 ){
		wk->listSpeed = 4;
	}else{
		wk->listSpeed = 3;
	}
}
*/
/*
static int MainSeq_Main( ZKNLISTMAIN_WORK * wk )
{
	u32	ret;

#if 0
  if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
		return MAINSEQ_MAIN;
	}
#endif

	// リストのキー操作
	switch( ZKNLISTMAIN_Main( wk->list ) ){
	case ZKNLISTMAIN_LIST_MOVE_UP:
	case ZKNLISTMAIN_LIST_MOVE_DOWN:
		wk->listConut = 8;
		return MAINSEQ_LIST_WAIT;

	case ZKNLISTMAIN_LIST_MOVE_LEFT:
	case ZKNLISTMAIN_LIST_MOVE_RIGHT:
		wk->listConut = 4;
		return MAINSEQ_LIST_WAIT;

	case ZKNLISTMAIN_LIST_MOVE_SCROLL_UP:
		SetListScrollSpeed( wk );
		wk->listConut = 0;
		wk->listScroll = GFL_BG_SCROLL_Y_DEC;
		return MAINSEQ_LIST_SCROLL;

	case ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN:
		SetListScrollSpeed( wk );
		wk->listConut = 0;
		wk->listScroll = GFL_BG_SCROLL_Y_INC;
		return MAINSEQ_LIST_SCROLL;
	}

	wk->listRepeat = 0;
	
	ret = ZKNLISUI_ListMain( wk );

	switch( ret ){
	case ZKNLISTUI_ID_LIST:				// 00: リスト
		return SetListMoveTouch( wk );

	case ZKNLISTUI_ID_RAIL:				// 01: レール
		return SetListMoveBar( wk );

	case ZKNLISTUI_ID_POKE:				// 02: ポケモン正面絵
		return SetInfoData( wk, 0 );

	case ZKNLISTUI_ID_ICON1:			// 03: ポケモンアイコン
	case ZKNLISTUI_ID_ICON2:			// 04: ポケモンアイコン
	case ZKNLISTUI_ID_ICON3:			// 05: ポケモンアイコン
	case ZKNLISTUI_ID_ICON4:			// 06: ポケモンアイコン
	case ZKNLISTUI_ID_ICON5:			// 07: ポケモンアイコン
	case ZKNLISTUI_ID_ICON6:			// 08: ポケモンアイコン
	case ZKNLISTUI_ID_ICON7:			// 09: ポケモンアイコン
		return SetInfoData( wk, ret );

	case ZKNLISTUI_ID_START:			// 10: スタート
		wk->dat->retMode = ZKNLIST_RET_SEARCH;
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_START, ZKNLISTOBJ_ANM_START_ANM, MAINSEQ_END_SET );

	case ZKNLISTUI_ID_SELECT:			// 11: セレクト
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_SELECT, ZKNLISTOBJ_ANM_SELECT_ANM, MAINSEQ_MAIN );

	case ZKNLISTUI_ID_LEFT:				// 12: 左
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON, MAINSEQ_LIST_MOVE_LEFT );

	case ZKNLISTUI_ID_RIGHT:			// 13: 右
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON, MAINSEQ_LIST_MOVE_RIGHT );

	case ZKNLISTUI_ID_Y:					// 14: Ｙ
		break;

	case ZKNLISTUI_ID_X:					// 15: Ｘ
		wk->dat->retMode = ZKNLIST_RET_EXIT_X;
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_EXIT, APP_COMMON_BARICON_EXIT_ON, MAINSEQ_END_SET );

	case ZKNLISTUI_ID_RETURN:			// 16: 戻る
		wk->dat->retMode = ZKNLIST_RET_EXIT;
		return SetButtonAnm( wk, ZKNLISTOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON, MAINSEQ_END_SET );

	case ZKNLISTUI_ID_LIST_UP:		// 上キー
	case ZKNLISTUI_ID_LIST_DOWN:	// 下キー
		break;
	}

	return MAINSEQ_MAIN;
}
*/
/*
static int MainSeq_ListWait( ZKNLISTMAIN_WORK * wk )
{
	wk->listConut--;
	if( wk->listConut == 0 ){
		return MAINSEQ_MAIN;
	}
	return MAINSEQ_LIST_WAIT;
}

static int MainSeq_ListScroll( ZKNLISTMAIN_WORK * wk )
{
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, wk->listScroll, wk->listSpeed );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, wk->listScroll, wk->listSpeed );
	if( wk->listScroll == GFL_BG_SCROLL_Y_DEC ){
		ZKNLISTOBJ_ScrollPokeIcon( wk, wk->listSpeed );
		wk->listBgScroll -= wk->listSpeed;
	}else{
		ZKNLISTOBJ_ScrollPokeIcon( wk, -wk->listSpeed );
		wk->listBgScroll += wk->listSpeed;
	}

	wk->listConut += wk->listSpeed;
	if( wk->listConut == 24 ){
		ZKNLISTMAIN_PutListCursor( wk, 2, ZKNLISTMAIN_GetListPos(wk->list) );
		ZKNLISTOBJ_ChgListPosAnm( wk, ZKNLISTMAIN_GetListPos(wk->list), TRUE );
		ZKNLISTOBJ_PutListPosPokeGra( wk, ZKNLISTMAIN_GetListCursorPos(wk->list) );
		ZKNLISTOBJ_SetListScrollBarPos( wk );
		return MAINSEQ_MAIN;
	}
	return MAINSEQ_LIST_SCROLL;
}

static int MainSeq_ListMoveLeft( ZKNLISTMAIN_WORK * wk )
{
	ZKNLISTMAIN_MoveLeft( wk->list );
	return MAINSEQ_MAIN;
}

static int MainSeq_ListMoveRight( ZKNLISTMAIN_WORK * wk )
{
	ZKNLISTMAIN_MoveRight( wk->list );
	return MAINSEQ_MAIN;
}
*/

/*
static BOOL CheckListMoveTouch( ZKNLISTMAIN_WORK * wk, u32 tpy )
{
	u32	mvPos;
	u32	nowPos;
	s16	nowScroll;

	mvPos  = GetTouchPos( tpy );
	nowPos = ZKNLISTMAIN_GetListPos( wk->list );

	nowScroll = ZKNLISTMAIN_GetListScroll( wk->list );

	if( mvPos < nowPos ){
		if( nowScroll == ZKNLISTMAIN_GetListScrollMax(wk->list) ){
			return FALSE;
		}
//		wk->targetPos = mvPos;
//		wk->tsCount = GFL_STD_Abs( mvPos - nowPos );
//		SetListScrollSpeedTouch( wk, wk->tsCount );
		wk->listSpeed = 12;
		wk->listConut = 0;
		wk->listScroll = GFL_BG_SCROLL_Y_INC;
		ZKNLISTMAIN_SetListDirect( wk->list, 0, 1, FALSE );
		return TRUE;
	}else if( mvPos > nowPos ){
		if( nowScroll == 0 ){
			return FALSE;
		}
//		wk->tsCount = GFL_STD_Abs( mvPos - nowPos );
//		SetListScrollSpeedTouch( wk, wk->tsCount );
		wk->listSpeed = 12;
		wk->listConut = 0;
		wk->listScroll = GFL_BG_SCROLL_Y_DEC;
		ZKNLISTMAIN_SetListDirect( wk->list, 0, -1, FALSE );
		return TRUE;
	}

	return FALSE;
}

static int MainSeq_ListMoveTouch( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;

	if( ZKNLISTUI_CheckListHit( &x, &y ) == FALSE ){
		return MAINSEQ_MAIN;
	}

	if( CheckListMoveTouch( wk, y ) == TRUE ){
		wk->initTouchFlag = TRUE;
		wk->initTouchPY = y;
		wk->frameTouchPY = y;
		wk->autoScroll = 0;
		return MAINSEQ_LIST_SCROLL_TOUCH;
	}

	return MAINSEQ_LIST_MOVE_TOUCH;
}
*/

/*
static const u32 ListScrollSpeed[] = {
	24, 12, 8, 6, 4, 3
};

static int SetAutoListScroll( ZKNLISTMAIN_WORK * wk, u32 scroll, u32 cnt, u32 speed )
{
	if( wk->listConut != 24 ){
		while(1){
			if( MainSeq_ListScroll( wk ) == MAINSEQ_MAIN ){
				break;
			}
		}
	}

	if( wk->listScroll == GFL_BG_SCROLL_Y_INC ){
		ZKNLISTMAIN_SetListDirect( wk->list, 0, 1, TRUE );
	}else{
		ZKNLISTMAIN_SetListDirect( wk->list, 0, -1, TRUE );
	}

	wk->autoSpeed = speed;
	wk->autoScroll = scroll;
	wk->autoCount = cnt;
	wk->autoWait = 0;

	wk->listSpeed = ListScrollSpeed[ wk->autoSpeed ];
	wk->listConut = 0;

	return MAINSEQ_LIST_AUTO_SCROLL;
}

static int MainSeq_ListScrollTouch( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;

	if( ZKNLISTUI_CheckListHit( &x, &y ) == TRUE ){
		wk->frameTouchPY = y;
	}

	if( MainSeq_ListScroll( wk ) == MAINSEQ_MAIN ){
		if( ZKNLISTUI_CheckListHit( &x, &y ) == FALSE ){
			u32	abs = GFL_STD_Abs( wk->initTouchPY - wk->frameTouchPY );
			if( abs >= 64 ){
				return SetAutoListScroll( wk, MONSNO_END, 400, 0 );
			}else if( abs >= 48 ){
				return SetAutoListScroll( wk, 256, 51, 1 );
			}else if( abs >= 32 ){
				return SetAutoListScroll( wk, 128, 32, 2 );
			}
			return MAINSEQ_MAIN;
		}else{
			if( CheckListMoveTouch( wk, y ) == TRUE ){
				wk->initTouchFlag = TRUE;
				wk->initTouchPY = y;
				wk->frameTouchPY = y;
				wk->autoScroll = 0;
				return MAINSEQ_LIST_SCROLL_TOUCH;
			}
			return MAINSEQ_LIST_MOVE_TOUCH;
		}
	}

	return MAINSEQ_LIST_SCROLL_TOUCH;
}

static int MainSeq_ListAutoScroll( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;

	OS_Printf( "%d - %d\n", wk->initTouchPY, wk->frameTouchPY );

	if( MainSeq_ListScroll( wk ) == MAINSEQ_MAIN ){
		s16	nowScroll = ZKNLISTMAIN_GetListScroll( wk->list );

		if( ZKNLISTUI_CheckListHit( &x, &y ) == TRUE ){
			ZKNLISTMAIN_SetPosDirect( wk->list, GetTouchPos(y) );
			return MAINSEQ_MAIN;
		}

		if( nowScroll == 0 || nowScroll == ZKNLISTMAIN_GetListScrollMax(wk->list) ){
			return MAINSEQ_MAIN;
		}
		if( wk->autoScroll == 0 ){
			return MAINSEQ_MAIN;
		}

		wk->listConut = 0;
		wk->autoScroll--;
		wk->autoWait++;
		if( wk->autoWait == wk->autoCount ){
			if( ListScrollSpeed[wk->autoSpeed] != 3 ){
				wk->autoSpeed++;
				wk->listSpeed = ListScrollSpeed[ wk->autoSpeed ];
			}
			wk->autoWait = 0;
		}
		if( wk->listScroll == GFL_BG_SCROLL_Y_INC ){
			ZKNLISTMAIN_SetListDirect( wk->list, 0, 1, TRUE );
		}else{
			wk->listConut = 0;
			ZKNLISTMAIN_SetListDirect( wk->list, 0, -1, TRUE );
		}
	}

	return MAINSEQ_LIST_AUTO_SCROLL;
}



static int MainSeq_ListMoveBar( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;
	u32	pos;

	if( ZKNLISTUI_CheckRailHit( &x, &y ) == FALSE ){
		return MAINSEQ_MAIN;
	}

	ZKNLISTOBJ_SetScrollBar( wk, y );

	pos = ZKNLISTOBJ_GetListScrollBarPos( wk );
	ZKNLISTMAIN_SetScrollDirect( wk->list, pos );

	return MAINSEQ_LIST_MOVE_BAR;
}
*/

static int MainSeq_ButtonAnm( ZKNLISTMAIN_WORK * wk )
{
	if( ZKNLISTOBJ_CheckAnm( wk, wk->buttonID ) == FALSE ){
		return wk->nextSeq;
	}	
	return MAINSEQ_BUTTON_ANM;
}

static int MainSeq_ItemAnm( ZKNLISTMAIN_WORK * wk )
{
	switch( wk->buttonSeq ){
	case 0:
	case 2:
		if( wk->buttonCnt == 0 ){
			FRAMELIST_ChangePosPalette( wk->lwk, FRAMELIST_GetCursorPos(wk->lwk), 1 );
			wk->buttonCnt = 4;
			wk->buttonSeq++;
		}else{
			wk->buttonCnt--;
		}
		break;

	case 1:
	case 3:
		if( wk->buttonCnt == 0 ){
			FRAMELIST_ChangePosPalette( wk->lwk, FRAMELIST_GetCursorPos(wk->lwk), 2 );
			GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
			wk->buttonCnt = 4;
			wk->buttonSeq++;
		}else{
			wk->buttonCnt--;
		}
		break;

	case 4:
		if( wk->buttonCnt == 0 ){
			wk->buttonSeq = 0;
			return SetFadeOut( wk, MAINSEQ_RELEASE );
//			return MAINSEQ_MAIN;
		}else{
			wk->buttonCnt--;
		}
	}

	return MAINSEQ_ITEM_ANM;
}

static int MainSeq_EndSet( ZKNLISTMAIN_WORK * wk )
{
	return SetFadeOut( wk, MAINSEQ_RELEASE );
}






static int SetFadeIn( ZKNLISTMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_LIST );
	wk->wipeSeq = next;
	return MAINSEQ_WIPE;
}

static int SetFadeOut( ZKNLISTMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_LIST );
	wk->wipeSeq = next;
	return MAINSEQ_WIPE;
}

static int SetButtonAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm, int next )
{
	ZKNLISTOBJ_SetAutoAnm( wk, id, anm );
	wk->buttonID = id;
	wk->nextSeq  = next;
	return MAINSEQ_BUTTON_ANM;
}

static BOOL CheckInfoData( ZKNLISTMAIN_WORK * wk, int pos )
{
	if( GET_LIST_INFO( FRAMELIST_GetItemParam(wk->lwk,pos) ) == 0 ){
		return FALSE;
	}
	return TRUE;
}

static int SetInfoData( ZKNLISTMAIN_WORK * wk, int pos )
{
	wk->dat->retMons = GET_LIST_MONS( FRAMELIST_GetItemParam(wk->lwk,pos) );
	wk->dat->retMode = ZKNLIST_RET_INFO;
	return MAINSEQ_ITEM_ANM;
//	return SetFadeOut( wk, MAINSEQ_RELEASE );
}

static void SetShortCut( ZKNLISTMAIN_WORK * wk )
{
	if( GAMEDATA_GetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_MENU ) == TRUE ){
		GAMEDATA_SetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_MENU, FALSE );
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_Y_BUTTON, APP_COMMON_BARICON_CHECK_OFF );
	}else{
		GAMEDATA_SetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_MENU, TRUE );
		ZKNLISTOBJ_SetAutoAnm( wk, ZKNLISTOBJ_IDX_TB_Y_BUTTON, APP_COMMON_BARICON_CHECK_ON );
	}
}

/*
static int SetListMoveBar( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;

	GFL_UI_TP_GetPointCont( &x, &y );
	ZKNLISTOBJ_SetScrollBar( wk, y );

	return MAINSEQ_LIST_MOVE_BAR;
}
*/
/*
static int SetListMoveTouch( ZKNLISTMAIN_WORK * wk )
{
	u32	x, y;

	GFL_UI_TP_GetPointCont( &x, &y );

	wk->initTouchPY = y;

	y = GetTouchPos( y );
	if( y > ZKNLISTMAIN_GetListPosMax(wk->list) ){
		return MAINSEQ_MAIN;
	}

	ZKNLISTMAIN_SetPosDirect( wk->list, y );

	return MAINSEQ_LIST_MOVE_TOUCH;
}
*/
/*
static u32 GetTouchPos( u32 tpy )
{
	return ( tpy / 8 / ZKNLISTMAIN_LIST_SY );
}
*/


#define	BASEBG_SCROLL_VAL		( 1 )

static void ScrollBaseBg( ZKNLISTMAIN_WORK * wk )
{
	wk->BaseScroll++;
	if( wk->BaseScroll == ZKNCOMM_BG_SCROLL_WAIT ){
		GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		wk->BaseScroll = 0;
	}
}

