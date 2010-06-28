//============================================================================================
/**
 * @file		zknsearch_seq.c
 * @brief		図鑑検索画面 シーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHSEQ
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "ui/touchbar.h"

#include "../zukan_common.h"
#include "zknsearch_main.h"
#include "zknsearch_seq.h"
#include "zknsearch_obj.h"
#include "zknsearch_bmp.h"
#include "zknsearch_ui.h"
#include "zknsearch_list.h"
#include "zknsearch_snd_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,

	MAINSEQ_INIT_MENU,
	MAINSEQ_MAIN_MENU,
	MAINSEQ_EXIT_MENU,

	MAINSEQ_RESET_SORT,
	MAINSEQ_START_SORT,
	MAINSEQ_RESULT_SORT,

	MAINSEQ_INIT_ROW,
	MAINSEQ_MAIN_ROW,
	MAINSEQ_EXIT_ROW,

	MAINSEQ_INIT_NAME,
	MAINSEQ_MAIN_NAME,
	MAINSEQ_EXIT_NAME,

	MAINSEQ_INIT_TYPE,
	MAINSEQ_MAIN_TYPE,
	MAINSEQ_EXIT_TYPE,

	MAINSEQ_INIT_COLOR,
	MAINSEQ_MAIN_COLOR,
	MAINSEQ_EXIT_COLOR,

	MAINSEQ_INIT_FORM,
	MAINSEQ_MAIN_FORM,
	MAINSEQ_EXIT_FORM,

	MAINSEQ_PAGE_MOVE,
	MAINSEQ_BUTTON_ANM,

	MAINSEQ_END_SET,
	MAINSEQ_END,
};

// ボタンアニメの種類
enum {
	BUTTON_ANM_BG = 0,
	BUTTON_ANM_OBJ
};

#define	SORT_COUNT_PUT_WINDOW		( 0 )					// 検索中カウント：ウィンドウ表示
#define	SORT_COUNT_MAKE_ENGINE	( 1 )					// 検索中カウント：分割検索エンジン作成
#define	SORT_COUNT_SEARCH				( 2 )					// 検索中カウント：検索開始
#define	SORT_COUNT_STOP_SE			( 100 )				// 検索中カウント：検索中ＳＥ停止
#define	SORT_COUNT_SEARCH_END		( 60*2+1 )		// 検索中カウント：検索終了


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Wipe( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitMenu( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainMenu( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitMenu( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ResetSort( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_StartSort( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ResultSort( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitRow( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainRow( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitRow( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitName( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainName( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitName( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitType( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainType( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitType( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitColor( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainColor( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitColor( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitForm( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainForm( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitForm( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_PageMove( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ButtonAnm( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_EndSet( ZKNSEARCHMAIN_WORK * wk );

static void ChangeSortRow( ZKNSEARCHMAIN_WORK * wk, u32 pos );
static void ResetSortRow( ZKNSEARCHMAIN_WORK * wk );
static void ChangeSortName( ZKNSEARCHMAIN_WORK * wk, u32 pos );
static void ResetSortName( ZKNSEARCHMAIN_WORK * wk );
static void ChangeSortType( ZKNSEARCHMAIN_WORK * wk, u32 pos );
static void ResetSortType( ZKNSEARCHMAIN_WORK * wk );
static void ChangeSortColor( ZKNSEARCHMAIN_WORK * wk, u32 pos );
static void ResetSortColor( ZKNSEARCHMAIN_WORK * wk );
static void ChangeSortForm( ZKNSEARCHMAIN_WORK * wk, u32 pos );
static void ResetSortForm( ZKNSEARCHMAIN_WORK * wk );

static int SetWipeIn( ZKNSEARCHMAIN_WORK * wk, int next );
static int SetWipeOut( ZKNSEARCHMAIN_WORK * wk, int next );

static int SetButtonAnm( ZKNSEARCHMAIN_WORK * wk, u8 mode, u8 id, int next );
static int PageChange( ZKNSEARCHMAIN_WORK * wk, int next );

static void SetShortCut( ZKNSEARCHMAIN_WORK * wk );


FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pZKNSEARCH_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,

	MainSeq_InitMenu,
	MainSeq_MainMenu,
	MainSeq_ExitMenu,

	MainSeq_ResetSort,
	MainSeq_StartSort,
	MainSeq_ResultSort,

	MainSeq_InitRow,
	MainSeq_MainRow,
	MainSeq_ExitRow,

	MainSeq_InitName,
	MainSeq_MainName,
	MainSeq_ExitName,

	MainSeq_InitType,
	MainSeq_MainType,
	MainSeq_ExitType,

	MainSeq_InitColor,
	MainSeq_MainColor,
	MainSeq_ExitColor,

	MainSeq_InitForm,
	MainSeq_MainForm,
	MainSeq_ExitForm,

	MainSeq_PageMove,
	MainSeq_ButtonAnm,
	MainSeq_EndSet,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNSEARCHSEQ_MainSeq( ZKNSEARCHMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	ZKNSEARCHOBJ_AnmMain( wk );
	ZKNSEARCHBMP_PrintUtilTrans( wk );
	ZKNCOMM_ScrollBaseBG( GFL_BG_FRAME2_M, GFL_BG_FRAME2_S, &wk->BaseScroll );

	return TRUE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( ZKNSEARCHMAIN_WORK * wk )
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
	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );

	// 全部リセットするのではなく、モードだけ変更する 
	if( ZUKANSAVE_GetZukanMode( wk->dat->savedata ) == TRUE ){ 
		wk->dat->sort->mode = ZKNCOMM_LIST_SORT_MODE_ZENKOKU;
	}else{
		wk->dat->sort->mode = ZKNCOMM_LIST_SORT_MODE_LOCAL;
	}

	ZKNSEARCHMAIN_InitVram();
	ZKNSEARCHMAIN_InitBg();
	ZKNSEARCHMAIN_LoadBgGraphic();
	ZKNSEARCHMAIN_InitMsg( wk );

	ZKNSEARCHBMP_Init( wk );
	ZKNSEARCHOBJ_Init( wk );

	ZKNSEARCHMAIN_InitBlinkAnm( wk );

	GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_ZUKAN_SEARCH );

	ZKNSEARCHMAIN_InitBgWinFrame( wk );
	ZKNSEARCHMAIN_InitFrameScroll( wk );

	ZKNSEARCHMAIN_InitPaletteFade( wk );
	ZKNSEARCHMAIN_SetPalFadeSeq( wk, 16, 16 );

	ZKNSEARCHMAIN_LoadLoadingWindow( wk );

	ZKNSEARCHMAIN_SetWindow();

	ZKNSEARCHMAIN_InitVBlank( wk );
	ZKNSEARCHMAIN_InitHBlank( wk );

	wk->page = 0xff;
  
	ZKNSEARCHMAIN_ConvFormDataToList( wk );		// データからリストへ

	return MAINSEQ_INIT_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk )
{
	if( ZKNSEARCHMAIN_MainSrameScroll(wk) == TRUE || PaletteFadeCheck(wk->pfd) != 0 ){
		return MAINSEQ_RELEASE;
	}

	ZKNSEARCHMAIN_ExitHBlank( wk );
	ZKNSEARCHMAIN_ExitVBlank( wk );

	ZKNSEARCHMAIN_ResetWindow();

	ZKNSEARCHMAIN_UnloadLoadingWindow( wk );

	ZKNSEARCHMAIN_ExitBgWinFrame( wk );

	ZKNSEARCHMAIN_ExitPaletteFade( wk );

	ZKNSEARCHMAIN_ExitBlinkAnm( wk );

	ZKNSEARCHOBJ_Exit( wk );
	ZKNSEARCHBMP_Exit( wk );

	ZKNSEARCHMAIN_ExitMsg( wk );
	ZKNSEARCHMAIN_ExitBg();

	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return MAINSEQ_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フェード待ち
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( ZKNSEARCHMAIN_WORK * wk )
{
	if( PaletteFadeCheck(wk->pfd) == 0 ){
		return wk->funcSeq;
	}
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン画面初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitMenu( ZKNSEARCHMAIN_WORK * wk )
{
	if( PaletteFadeCheck(wk->pfd) != 0 ){
		return MAINSEQ_INIT_MENU;
	}

	ZKNSEARCHMAIN_SetBlendAlpha( TRUE );

	ZKNSEARCHMAIN_LoadMenuPageScreen( wk );

	ZKNSEARCHBMP_PutMainPage( wk );
	ZKNSEARCHOBJ_PutMainPage( wk );

	ZKNSEARCHMAIN_ChangeBgPriorityMenu();
	ZKNSEARCHMAIN_ListBGOn( wk );

	if( wk->page == 0xff ){
		ZKNSEARCHUI_MainCursorMoveInit( wk, 0 );
		GX_SetMasterBrightness( 0 );
		GXS_SetMasterBrightness( 0 );
		ZKNSEARCHMAIN_SetFrameScrollParam( wk, -ZKNCOMM_BAR_SPEED );
		ZKNSEARCHMAIN_SetPalFadeSeq( wk, 16, 0 );
		return MAINSEQ_MAIN_MENU;
	}
	ZKNSEARCHUI_MainCursorMoveInit( wk, wk->page );
	return MAINSEQ_MAIN_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン画面メイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainMenu( ZKNSEARCHMAIN_WORK * wk )
{
	if( ZKNSEARCHMAIN_MainSrameScroll(wk) == TRUE || PaletteFadeCheck(wk->pfd) != 0 ){
		return MAINSEQ_MAIN_MENU;
	}

	switch( ZKNSEARCHUI_MenuMain(wk) ){
	case ZKNSEARCHUI_ROW:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 0, PageChange(wk,MAINSEQ_INIT_ROW) );

	case ZKNSEARCHUI_NAME:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 1, PageChange(wk,MAINSEQ_INIT_NAME) );

	case ZKNSEARCHUI_TYPE:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 2, PageChange(wk,MAINSEQ_INIT_TYPE) );

	case ZKNSEARCHUI_COLOR:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 3, PageChange(wk,MAINSEQ_INIT_COLOR) );

	case ZKNSEARCHUI_FORM:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 4, PageChange(wk,MAINSEQ_INIT_FORM) );

	case ZKNSEARCHUI_START:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 5, MAINSEQ_START_SORT );

	case ZKNSEARCHUI_RESET:
		PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
		return SetButtonAnm( wk, BUTTON_ANM_BG, 6, MAINSEQ_RESET_SORT );

	case ZKNSEARCHUI_Y:
		PMSND_PlaySE( ZKNSEARCH_SE_Y );
		SetShortCut( wk );
		break;

	case ZKNSEARCHUI_X:
		PMSND_PlaySE( ZKNSEARCH_SE_CLOASE );
		wk->dat->retMode = ZKNSEARCH_RET_EXIT_X;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, APP_COMMON_BARICON_EXIT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_EXIT, MAINSEQ_END_SET );

	case ZKNSEARCHUI_RET:
		PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
		ZKNSEARCHMAIN_ConvFormListToData( wk );		// リストからデータへ
		wk->dat->retMode = ZKNSEARCH_RET_CANCEL;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_END_SET );

	case CURSORMOVE_CURSOR_ON:		// カーソル表示
		PMSND_PlaySE( ZKNSEARCH_SE_MOVE );
		break;

	case CURSORMOVE_CURSOR_MOVE:	// 移動
		PMSND_PlaySE( ZKNSEARCH_SE_MOVE );
		break;

	case CURSORMOVE_CANCEL:					// キャンセル
		PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
		ZKNSEARCHMAIN_ConvFormListToData( wk );		// リストからデータへ
		wk->dat->retMode = ZKNSEARCH_RET_CANCEL;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_END_SET );
	}

	BLINKPALANM_Main( wk->blink );

	return MAINSEQ_MAIN_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン画面終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitMenu( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, FALSE );
	ZKNSEARCHUI_CursorMoveExit( wk );

	return wk->nextSeq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：検索データリセット
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ResetSort( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNCOMM_ResetSortData( wk->dat->savedata, wk->dat->sort );
	ZKNSEARCHBMP_PutMainPage( wk );
	ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_LABEL, FALSE );
	return MAINSEQ_MAIN_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：検索開始
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StartSort( ZKNSEARCHMAIN_WORK * wk )
{
	// ウィンドウ表示
	if( wk->loadingCnt == SORT_COUNT_PUT_WINDOW ){
		PMSND_PlaySE( ZKNSEARCH_SE_LOADING );
		ZKNSEARCHMAIN_LoadingWindowOn( wk );
		ZKNSEARCHBMP_SearchStart( wk );
		ZKNSEARCHBMP_ClearMainPageLabel( wk );
	}

	// 検索中ＳＥ停止
	if( wk->loadingCnt == SORT_COUNT_STOP_SE ){
		PMSND_StopSE();
	}

  // 分割検索エンジン作成
	if( wk->loadingCnt == SORT_COUNT_MAKE_ENGINE ){
		ZKNSEARCHMAIN_ConvFormListToData( wk );		// リストからデータへ
		wk->egn_wk = ZUKAN_SEARCH_ENGINE_DivInit(
									wk->dat->savedata, wk->dat->sort, HEAPID_ZUKAN_SEARCH_L );
		wk->egn_st = ZKN_SCH_EGN_DIV_STATE_CONTINUE;
	}

	// 検索処理
	if( wk->loadingCnt >= SORT_COUNT_SEARCH && wk->egn_st == ZKN_SCH_EGN_DIV_STATE_CONTINUE ){
		u16  num;
		u16 * list;
		// 検索
		wk->egn_st = ZUKAN_SEARCH_ENGINE_DivSearch(
									wk->egn_wk, HEAPID_ZUKAN_SYS, &num, &list );
		if( wk->egn_st == ZKN_SCH_EGN_DIV_STATE_FINISH ){
			wk->dat->listMax = num;
			wk->dat->list = list;
			// 破棄
			ZUKAN_SEARCH_ENGINE_DivExit( wk->egn_wk );
		}
	}

	// 検索終了
	if( wk->loadingCnt == SORT_COUNT_SEARCH_END ){
		if( wk->egn_st == ZKN_SCH_EGN_DIV_STATE_FINISH ){
			if( wk->dat->listMax != 0 ){
				PMSND_PlaySE( ZKNSEARCH_SE_HIT );
				ZKNSEARCHBMP_SearchComp( wk );
			}else{
				PMSND_PlaySE( ZKNSEARCH_SE_ERROR );
				ZKNSEARCHBMP_SearchError( wk );
			}
			wk->loadingCnt = 0;
			return MAINSEQ_RESULT_SORT;
		}
	// 検索中
	}else{
		ZKNSEARCHOBJ_MoveLoadingBar( wk, wk->loadingCnt );
		wk->loadingCnt++;
	}

	return MAINSEQ_START_SORT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：検索結果
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ResultSort( ZKNSEARCHMAIN_WORK * wk )
{
	if( ZKNSEARCHUI_Result( wk ) == TRUE ){
		if( wk->dat->listMax != 0 ){
			wk->dat->retMode = ZKNSEARCH_RET_START;
			return MAINSEQ_END_SET;
		}
		ZKNSEARCHMAIN_LoadingWindowOff( wk );
		ZKNSEARCHBMP_PutMainPageLabel( wk );
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
			CURSORMOVE_CursorOnOffSet( wk->cmwk, FALSE );
		}else{
			CURSORMOVE_CursorOnOffSet( wk->cmwk, TRUE );
		}
		ZKNSEARCHMAIN_ConvFormDataToList( wk );		// データからリストへ戻す
		return MAINSEQ_MAIN_MENU;
	}
	return MAINSEQ_RESULT_SORT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：並びリスト初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitRow( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_ROW;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
		ZKNSEARCHBMP_PutRowPage( wk );
		ZKNSEARCHLIST_MakeRowList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutRowPage( wk );
			ZKNSEARCHMAIN_LoadRowListPageScreen( wk );
			ZKNSEARCHMAIN_ChangeBgPriorityList();
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_ROW;
		}
	}
	return MAINSEQ_INIT_ROW;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：並びリストメイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainRow( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortRow( wk, ret );
		}
		break;

	case FRAMELIST_RET_NONE:
		ret = ZKNSEARCHUI_ListMain( wk );

		switch( ret ){
		case ZKNSEARCHUI_LIST_RET:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_ROW );

		case ZKNSEARCHUI_LIST_CANCEL:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_ROW );

		case ZKNSEARCHUI_LIST_RESET:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ResetSortRow( wk );
			break;

		case ZKNSEARCHUI_LIST_ENTER1:
		case ZKNSEARCHUI_LIST_ENTER2:
		case ZKNSEARCHUI_LIST_ENTER3:
		case ZKNSEARCHUI_LIST_ENTER4:
		case ZKNSEARCHUI_LIST_ENTER5:
		case ZKNSEARCHUI_LIST_ENTER6:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortRow( wk, ret-ZKNSEARCHUI_LIST_ENTER1 );
			break;
		}
	}

	return MAINSEQ_MAIN_ROW;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：並びリスト終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitRow( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：名前リスト初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitName( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_NAME;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
		ZKNSEARCHBMP_PutNamePage( wk );
		ZKNSEARCHLIST_MakeNameList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ZKNSEARCHOBJ_PutNamePage( wk );
			ZKNSEARCHMAIN_LoadNameListPageScreen( wk );
			ZKNSEARCHMAIN_ChangeBgPriorityList();
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_NAME;
		}
	}
	return MAINSEQ_INIT_NAME;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：名前リストメイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainName( ZKNSEARCHMAIN_WORK * wk )
{
	int	seq;
	u32	ret;

	seq = MAINSEQ_MAIN_NAME;
	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortName( wk, ret );
		}
		break;

	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_LEFT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_NAME;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_RIGHT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_NAME;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_UP_NONE:		// ページ数が足りなくて１ページ上にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_LEFT, MAINSEQ_MAIN_NAME );

	case FRAMELIST_RET_PAGE_DOWN_NONE:	// ページ数が足りなくて１ページ下にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RIGHT, MAINSEQ_MAIN_NAME );

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
		ZKNSEARCHOBJ_VanishMark( wk );
		ZKNSEARCHOBJ_VanishJumpMark( wk, 0, wk->dat->sort->name );
		wk->btnID = ZKNSEARCHOBJ_IDX_MAX;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_NAME;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_NONE:
		ret = ZKNSEARCHUI_ListMain( wk );

		switch( ret ){
		case ZKNSEARCHUI_LIST_RET:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_NAME );

		case ZKNSEARCHUI_LIST_CANCEL:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_NAME );

		case ZKNSEARCHUI_LIST_RESET:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ResetSortName( wk );
			break;

		case ZKNSEARCHUI_LIST_ENTER1:
		case ZKNSEARCHUI_LIST_ENTER2:
		case ZKNSEARCHUI_LIST_ENTER3:
		case ZKNSEARCHUI_LIST_ENTER4:
		case ZKNSEARCHUI_LIST_ENTER5:
		case ZKNSEARCHUI_LIST_ENTER6:
		case ZKNSEARCHUI_LIST_ENTER7:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ChangeSortName( wk, ret-ZKNSEARCHUI_LIST_ENTER1 );
			break;
		}
	}

	return seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：名前リスト終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitName( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：タイプリスト初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitType( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_TYPE;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
		ZKNSEARCHBMP_PutTypePage( wk );
		ZKNSEARCHLIST_MakeTypeList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ZKNSEARCHOBJ_PutTypePage( wk );
			ZKNSEARCHMAIN_LoadTypeListPageScreen( wk );
			ZKNSEARCHMAIN_ChangeBgPriorityList();
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_TYPE;
		}
	}
	return MAINSEQ_INIT_TYPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：タイプリストメイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainType( ZKNSEARCHMAIN_WORK * wk )
{
	int	seq;
	u32	ret;

	seq = MAINSEQ_MAIN_TYPE;
	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortType( wk, ret );
		}
		break;

	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_LEFT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_TYPE;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_RIGHT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_TYPE;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_UP_NONE:		// ページ数が足りなくて１ページ上にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_LEFT, MAINSEQ_MAIN_TYPE );

	case FRAMELIST_RET_PAGE_DOWN_NONE:	// ページ数が足りなくて１ページ下にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RIGHT, MAINSEQ_MAIN_TYPE );

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
		ZKNSEARCHOBJ_VanishMark( wk );
		ZKNSEARCHOBJ_VanishJumpMark( wk, 0, ZKNSEARCHMAIN_GetSortTypeIndex(wk,wk->dat->sort->type1) );
		ZKNSEARCHOBJ_VanishJumpMark( wk, 1, ZKNSEARCHMAIN_GetSortTypeIndex(wk,wk->dat->sort->type2) );
		wk->btnID = ZKNSEARCHOBJ_IDX_MAX;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_TYPE;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_NONE:
		ret = ZKNSEARCHUI_ListMain( wk );

		switch( ret ){
		case ZKNSEARCHUI_LIST_RET:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_TYPE );

		case ZKNSEARCHUI_LIST_CANCEL:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_TYPE );

		case ZKNSEARCHUI_LIST_RESET:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ResetSortType( wk );
			break;

		case ZKNSEARCHUI_LIST_ENTER1:
		case ZKNSEARCHUI_LIST_ENTER2:
		case ZKNSEARCHUI_LIST_ENTER3:
		case ZKNSEARCHUI_LIST_ENTER4:
		case ZKNSEARCHUI_LIST_ENTER5:
		case ZKNSEARCHUI_LIST_ENTER6:
		case ZKNSEARCHUI_LIST_ENTER7:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ChangeSortType( wk, ret-ZKNSEARCHUI_LIST_ENTER1 );
			break;
		}
	}

	return seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：タイプリスト終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitType( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：色リスト初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitColor( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_COLOR;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
		ZKNSEARCHBMP_PutColorPage( wk );
		ZKNSEARCHLIST_MakeColorList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ZKNSEARCHOBJ_PutColorPage( wk );
			ZKNSEARCHMAIN_LoadColorListPageScreen( wk );
			ZKNSEARCHMAIN_ChangeBgPriorityList();
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_COLOR;
		}
	}
	return MAINSEQ_INIT_COLOR;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：色リストメイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainColor( ZKNSEARCHMAIN_WORK * wk )
{
	int	seq;
	u32	ret;

	seq = MAINSEQ_MAIN_COLOR;
	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortColor( wk, ret );
		}
		break;

	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_LEFT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_COLOR;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_RIGHT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_COLOR;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_UP_NONE:		// ページ数が足りなくて１ページ上にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_LEFT, MAINSEQ_MAIN_COLOR );

	case FRAMELIST_RET_PAGE_DOWN_NONE:	// ページ数が足りなくて１ページ下にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RIGHT, MAINSEQ_MAIN_COLOR );

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
		ZKNSEARCHOBJ_VanishMark( wk );
		ZKNSEARCHOBJ_VanishJumpMark( wk, 0, wk->dat->sort->color );
		wk->btnID = ZKNSEARCHOBJ_IDX_MAX;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_COLOR;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_NONE:
		ret = ZKNSEARCHUI_ListMain( wk );

		switch( ret ){
		case ZKNSEARCHUI_LIST_RET:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_COLOR );

		case ZKNSEARCHUI_LIST_CANCEL:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_COLOR );

		case ZKNSEARCHUI_LIST_RESET:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ResetSortColor( wk );
			break;

		case ZKNSEARCHUI_LIST_ENTER1:
		case ZKNSEARCHUI_LIST_ENTER2:
		case ZKNSEARCHUI_LIST_ENTER3:
		case ZKNSEARCHUI_LIST_ENTER4:
		case ZKNSEARCHUI_LIST_ENTER5:
		case ZKNSEARCHUI_LIST_ENTER6:
		case ZKNSEARCHUI_LIST_ENTER7:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ChangeSortColor( wk, ret-ZKNSEARCHUI_LIST_ENTER1 );
			break;
		}
	}

	return seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：色リスト終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitColor( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フォルムリスト初期化
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitForm( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_FORM;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
		ZKNSEARCHBMP_PutFormPage( wk );
		ZKNSEARCHLIST_MakeFormList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ZKNSEARCHOBJ_PutFormPage( wk );
			ZKNSEARCHMAIN_LoadFormListPageScreen( wk );
			ZKNSEARCHMAIN_ChangeBgPriorityList();
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_FORM;
		}
	}
	return MAINSEQ_INIT_FORM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フォルムリストメイン
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MainForm( ZKNSEARCHMAIN_WORK * wk )
{
	int	seq;
	u32	ret;

	seq = MAINSEQ_MAIN_FORM;
	ret = FRAMELIST_Main( wk->lwk );

	switch( ret ){
	case 0:
	case 1:
	case 2:
	case 3:
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ChangeSortForm( wk, ret );
		}
		break;

	case FRAMELIST_RET_CURSOR_ON:		// カーソル表示
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_LEFT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_FORM;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		wk->btnID = ZKNSEARCHOBJ_IDX_TB_RIGHT;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_FORM;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_PAGE_UP_NONE:		// ページ数が足りなくて１ページ上にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_LEFT, APP_COMMON_BARICON_CURSOR_LEFT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_LEFT, MAINSEQ_MAIN_FORM );

	case FRAMELIST_RET_PAGE_DOWN_NONE:	// ページ数が足りなくて１ページ下にスクロールできなかった
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RIGHT, APP_COMMON_BARICON_CURSOR_RIGHT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RIGHT, MAINSEQ_MAIN_FORM );

	case FRAMELIST_RET_JUMP_TOP:		// リスト最上部へジャンプ
	case FRAMELIST_RET_JUMP_BOTTOM:	// リスト最下部へジャンプ
		ZKNSEARCHOBJ_VanishJumpFormIcon( wk );
		ZKNSEARCHOBJ_VanishMark( wk );
		ZKNSEARCHOBJ_VanishJumpMarkForm( wk );
		wk->btnID = ZKNSEARCHOBJ_IDX_MAX;
		wk->subSeq = 0;
		wk->nextSeq = MAINSEQ_MAIN_FORM;
		seq = MAINSEQ_PAGE_MOVE;
		break;

	case FRAMELIST_RET_NONE:
		ret = ZKNSEARCHUI_FormListMain( wk );

		switch( ret ){
		case ZKNSEARCHUI_LIST_RET:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_FORM );

		case ZKNSEARCHUI_LIST_CANCEL:
			PMSND_PlaySE( ZKNSEARCH_SE_CANCEL );
			ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
			return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_EXIT_FORM );

		case ZKNSEARCHUI_LIST_RESET:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ResetSortForm( wk );
			break;

		case ZKNSEARCHUI_LIST_ENTER1:
		case ZKNSEARCHUI_LIST_ENTER2:
		case ZKNSEARCHUI_LIST_ENTER3:
		case ZKNSEARCHUI_LIST_ENTER4:
			PMSND_PlaySE( ZKNSEARCH_SE_DECIDE );
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			ChangeSortForm( wk, ret-ZKNSEARCHUI_LIST_ENTER1 );
			break;
		}
	}

	return seq;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フォルムリスト終了
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ExitForm( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：リストページ送り
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PageMove( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		if( FRAMELIST_Main( wk->lwk ) == FRAMELIST_RET_NONE ){
			wk->subSeq++;
		}
		break;

	case 1:
		if( wk->btnID == ZKNSEARCHOBJ_IDX_MAX || ZKNSEARCHOBJ_CheckAnm( wk, wk->btnID ) == FALSE ){
			wk->subSeq = 0;
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			return wk->nextSeq;
		}
		break;
	}

	return MAINSEQ_PAGE_MOVE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボタンアニメ待ち
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ButtonAnm( ZKNSEARCHMAIN_WORK * wk )
{
	if( wk->btnMode == BUTTON_ANM_BG ){
		switch( wk->btnSeq ){
		case 0:
		case 2:
			if( wk->btnCnt == 0 ){
				ZKNSEARCHUI_ChangeCursorPalette( wk, wk->btnID, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
				wk->btnCnt = 4;
				wk->btnSeq++;
			}else{
				wk->btnCnt--;
			}
			break;

		case 1:
		case 3:
			if( wk->btnCnt == 0 ){
				ZKNSEARCHUI_ChangeCursorPalette( wk, wk->btnID, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR );
				wk->btnCnt = 4;
				wk->btnSeq++;
			}else{
				wk->btnCnt--;
			}
			break;

		case 4:
			if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
				ZKNSEARCHUI_ChangeCursorPalette( wk, wk->btnID, ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF );
			}else{
				ZKNSEARCHUI_ChangeCursorPalette( wk, wk->btnID, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2 );
			}
			return wk->funcSeq;
		}
	}else{
		if( ZKNSEARCHOBJ_CheckAnm( wk, wk->btnID ) == FALSE ){
			ZKNSEARCHOBJ_SetListPageArrowAnime( wk, FALSE );
			return wk->funcSeq;
		}
	}

	return MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：終了設定
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndSet( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHUI_CursorMoveExit( wk );
	ZKNSEARCHMAIN_SetFrameScrollParam( wk, ZKNCOMM_BAR_SPEED );
	ZKNSEARCHMAIN_SetPalFadeSeq( wk, 0, 16 );
	return MAINSEQ_RELEASE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		データ切り替え：並び
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		pos			カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeSortRow( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	wk->dat->sort->row = pos;
	FRAMELIST_SetCursorPos( wk->lwk, pos );
	ZKNSEARCHBMP_PutRowItem( wk );
	ZKNSEARCHOBJ_ChangeMark( wk, pos, TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データリセット：並び
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ResetSortRow( ZKNSEARCHMAIN_WORK * wk )
{
	wk->dat->sort->row = ZKNCOMM_LIST_SORT_ROW_NUMBER;
	ZKNSEARCHBMP_PutRowItem( wk );
	ZKNSEARCHOBJ_ChangeMark( wk, 0, TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データ切り替え：名前
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		pos			カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeSortName( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	u32	list_pos = FRAMELIST_GetScrollCount( wk->lwk ) + pos;

	if( wk->dat->sort->name == list_pos ){
		wk->dat->sort->name = ZKNCOMM_LIST_SORT_NONE;
		ZKNSEARCHOBJ_ChangeMark( wk, pos, FALSE );
	}else{
		wk->dat->sort->name = list_pos;
		ZKNSEARCHOBJ_ChangeMark( wk, pos, TRUE );
	}
	FRAMELIST_SetCursorPos( wk->lwk, pos );
	ZKNSEARCHBMP_PutNameItem( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データリセット：名前
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ResetSortName( ZKNSEARCHMAIN_WORK * wk )
{
	wk->dat->sort->name = ZKNCOMM_LIST_SORT_NONE;
	ZKNSEARCHBMP_PutNameItem( wk );
	ZKNSEARCHOBJ_VanishMark( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データ切り替え：タイプ
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		pos			カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeSortType( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	u32	list_pos = ZKNSEARCHMAIN_GetSortType( wk, FRAMELIST_GetScrollCount(wk->lwk)+pos );

	if( wk->dat->sort->type1 == ZKNCOMM_LIST_SORT_NONE ){
		wk->dat->sort->type1 = list_pos;
	}else if( wk->dat->sort->type2 == ZKNCOMM_LIST_SORT_NONE ){
		if( wk->dat->sort->type1 == list_pos ){
			wk->dat->sort->type1 = ZKNCOMM_LIST_SORT_NONE;
		}else{
			wk->dat->sort->type2 = list_pos;
		}
	}else{
		if( wk->dat->sort->type1 == list_pos ){
			wk->dat->sort->type1 = wk->dat->sort->type2;
			wk->dat->sort->type2 = ZKNCOMM_LIST_SORT_NONE;
		}else if( wk->dat->sort->type2 == list_pos ){
			wk->dat->sort->type2 = ZKNCOMM_LIST_SORT_NONE;
		}else{
			wk->dat->sort->type1 = wk->dat->sort->type2;
			wk->dat->sort->type2 = list_pos;
		}
	}
	FRAMELIST_SetCursorPos( wk->lwk, pos );
	ZKNSEARCHBMP_PutTypeItem( wk );
	ZKNSEARCHOBJ_ChangeTypeMark( wk, wk->dat->sort->type1, wk->dat->sort->type2 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データリセット：タイプ
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ResetSortType( ZKNSEARCHMAIN_WORK * wk )
{
	wk->dat->sort->type1 = ZKNCOMM_LIST_SORT_NONE;
	wk->dat->sort->type2 = ZKNCOMM_LIST_SORT_NONE;
	ZKNSEARCHBMP_PutTypeItem( wk );
	ZKNSEARCHOBJ_VanishMark( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データ切り替え：色
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		pos			カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeSortColor( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	u32	list_pos = FRAMELIST_GetScrollCount( wk->lwk ) + pos;

	if( wk->dat->sort->color == list_pos ){
		wk->dat->sort->color = ZKNCOMM_LIST_SORT_NONE;
		ZKNSEARCHOBJ_ChangeMark( wk, pos, FALSE );
	}else{
		wk->dat->sort->color = list_pos;
		ZKNSEARCHOBJ_ChangeMark( wk, pos, TRUE );
	}
	FRAMELIST_SetCursorPos( wk->lwk, pos );
	ZKNSEARCHBMP_PutColorItem( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データリセット：色
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ResetSortColor( ZKNSEARCHMAIN_WORK * wk )
{
	wk->dat->sort->color = ZKNCOMM_LIST_SORT_NONE;
	ZKNSEARCHBMP_PutColorItem( wk );
	ZKNSEARCHOBJ_VanishMark( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データ切り替え：フォルム
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		pos			カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeSortForm( ZKNSEARCHMAIN_WORK * wk, u32 pos )
{
	u32	list_pos = FRAMELIST_GetScrollCount( wk->lwk ) + pos;

	if( wk->dat->sort->form == list_pos ){
		wk->dat->sort->form = ZKNCOMM_LIST_SORT_NONE;
		ZKNSEARCHOBJ_ChangeFormMark( wk, pos, FALSE );
	}else{
		wk->dat->sort->form = list_pos;
		ZKNSEARCHOBJ_ChangeFormMark( wk, pos, TRUE );
	}
	FRAMELIST_SetCursorPos( wk->lwk, pos );
	ZKNSEARCHOBJ_PutFormListNow( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		データリセット：フォルム
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ResetSortForm( ZKNSEARCHMAIN_WORK * wk )
{
	wk->dat->sort->form = ZKNCOMM_LIST_SORT_NONE;
	ZKNSEARCHOBJ_PutFormListNow( wk );
	ZKNSEARCHOBJ_VanishMark( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		next		フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetWipeIn( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNSEARCHMAIN_SetPalFadeSeq( wk, 16, 0 );
	wk->funcSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		next		フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetWipeOut( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNSEARCHMAIN_SetPalFadeSeq( wk, 0, 16 );
	wk->funcSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンアニメ設定
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		mode		モード
 * @param		id			ボタンＩＤ
 * @param		next		アニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetButtonAnm( ZKNSEARCHMAIN_WORK * wk, u8 mode, u8 id, int next )
{
	wk->btnMode = mode;
	wk->btnID   = id;
	wk->btnSeq  = 0;
	wk->btnCnt  = 0;
	wk->funcSeq = next;
	return MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		next		切り替え後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int PageChange( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->pageSeq = 0;
	wk->nextSeq = next;
	return MAINSEQ_EXIT_MENU;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		Ｙボタンショートカット設定
 *
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static void SetShortCut( ZKNSEARCHMAIN_WORK * wk )
{
	if( GAMEDATA_GetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_SEARCH ) == TRUE ){
		GAMEDATA_SetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_SEARCH, FALSE );
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, APP_COMMON_BARICON_CHECK_OFF );
	}else{
		GAMEDATA_SetShortCut( wk->dat->gamedata, SHORTCUT_ID_ZUKAN_SEARCH, TRUE );
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_Y_BUTTON, APP_COMMON_BARICON_CHECK_ON );
	}
}
