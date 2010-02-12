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
#include "ui/touchbar.h"

#include "../zukan_common.h"
#include "zknsearch_main.h"
#include "zknsearch_seq.h"
#include "zknsearch_obj.h"
#include "zknsearch_bmp.h"
#include "zknsearch_ui.h"
#include "zknsearch_list.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,
	MAINSEQ_FADE,

	MAINSEQ_INIT_MENU,
	MAINSEQ_MAIN_MENU,
	MAINSEQ_EXIT_MENU,

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

	MAINSEQ_BUTTON_ANM,

	MAINSEQ_END_SET,
	MAINSEQ_END,
};

// ボタンアニメの種類
enum {
	BUTTON_ANM_BG = 0,
	BUTTON_ANM_OBJ
};

//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Wipe( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Fade( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_InitMenu( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_MainMenu( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_ExitMenu( ZKNSEARCHMAIN_WORK * wk );
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
static int MainSeq_ButtonAnm( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_EndSet( ZKNSEARCHMAIN_WORK * wk );

static int SetWipeIn( ZKNSEARCHMAIN_WORK * wk, int next );
static int SetWipeOut( ZKNSEARCHMAIN_WORK * wk, int next );
static int SetFadeIn( ZKNSEARCHMAIN_WORK * wk, int next );
static int SetFadeOut( ZKNSEARCHMAIN_WORK * wk, int next );

static int SetButtonAnm( ZKNSEARCHMAIN_WORK * wk, u8 mode, u8 id, int next );
static int PageChange( ZKNSEARCHMAIN_WORK * wk, int next );

FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

static const pZKNSEARCH_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_Fade,

	MainSeq_InitMenu,
	MainSeq_MainMenu,
	MainSeq_ExitMenu,

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

	MainSeq_ButtonAnm,
	MainSeq_EndSet,
};








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

	ZKNSEARCHMAIN_InitVram();
	ZKNSEARCHMAIN_InitBg();
	ZKNSEARCHMAIN_LoadBgGraphic();
	ZKNSEARCHMAIN_InitMsg( wk );

	ZKNSEARCHBMP_Init( wk );
	ZKNSEARCHOBJ_Init( wk );

	ZKNSEARCHMAIN_InitBlinkAnm( wk );

	ZKNSEARCHMAIN_InitVBlank( wk );
	ZKNSEARCHMAIN_InitHBlank( wk );

	wk->page = 0xff;

	return MAINSEQ_INIT_MENU;
}

static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ExitHBlank( wk );
	ZKNSEARCHMAIN_ExitVBlank( wk );

	ZKNSEARCHMAIN_ExitBlinkAnm( wk );

	ZKNSEARCHOBJ_Exit( wk );
	ZKNSEARCHBMP_Exit( wk );

	ZKNSEARCHMAIN_ExitMsg( wk );
	ZKNSEARCHMAIN_ExitBg();

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return MAINSEQ_END;
}

static int MainSeq_Wipe( ZKNSEARCHMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->funcSeq;
	}
	return MAINSEQ_WIPE;
}

static int MainSeq_Fade( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_FADE;
}


static int MainSeq_InitMenu( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_SetBlendAlpha( FALSE );

	ZKNSEARCHMAIN_LoadManuPageScreen( wk );

	ZKNSEARCHBMP_PutMainPage( wk );
//	ZKNSEARCHOBJ_VanishAll( wk );
	ZKNSEARCHOBJ_PutMainPage( wk );

	ZKNSEARCHMAIN_ListBGOn( wk );

	if( wk->page == 0xff ){
		ZKNSEARCHUI_MainCursorMoveInit( wk, 0 );
		return SetWipeIn( wk, MAINSEQ_MAIN_MENU );
	}
	ZKNSEARCHUI_MainCursorMoveInit( wk, wk->page );
	return MAINSEQ_MAIN_MENU;
}

static int MainSeq_MainMenu( ZKNSEARCHMAIN_WORK * wk )
{
	switch( CURSORMOVE_MainCont(wk->cmwk) ){
	case ZKNSEARCHUI_ROW:
		return SetButtonAnm( wk, BUTTON_ANM_BG, 0, PageChange(wk,MAINSEQ_INIT_ROW) );

	case ZKNSEARCHUI_NAME:
		return SetButtonAnm( wk, BUTTON_ANM_BG, 1, PageChange(wk,MAINSEQ_INIT_NAME) );

	case ZKNSEARCHUI_TYPE:
		return SetButtonAnm( wk, BUTTON_ANM_BG, 2, PageChange(wk,MAINSEQ_INIT_TYPE) );

	case ZKNSEARCHUI_COLOR:
		return SetButtonAnm( wk, BUTTON_ANM_BG, 3, PageChange(wk,MAINSEQ_INIT_COLOR) );

	case ZKNSEARCHUI_FORM:
		return SetButtonAnm( wk, BUTTON_ANM_BG, 4, PageChange(wk,MAINSEQ_INIT_FORM) );

	case ZKNSEARCHUI_START:
//		return MAINSEQ_END_SET;
		break;

	case ZKNSEARCHUI_RESET:
//		ZKNCOMM_ResetSortData( wk->dat->sort );
//		ZKNSEARCHBMP_PutMainPage( wk );
		break;

	case ZKNSEARCHUI_Y:
		break;

	case ZKNSEARCHUI_X:
		wk->dat->retMode = ZKNSEARCH_RET_EXIT_X;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_EXIT, APP_COMMON_BARICON_EXIT_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_EXIT, MAINSEQ_END_SET );

	case ZKNSEARCHUI_RET:
		wk->dat->retMode = ZKNSEARCH_RET_CANCEL;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_END_SET );

	case CURSORMOVE_CANCEL:					// キャンセル
		wk->dat->retMode = ZKNSEARCH_RET_CANCEL;
		ZKNSEARCHOBJ_SetAutoAnm( wk, ZKNSEARCHOBJ_IDX_TB_RETURN, APP_COMMON_BARICON_RETURN_ON );
		return SetButtonAnm( wk, BUTTON_ANM_OBJ, ZKNSEARCHOBJ_IDX_TB_RETURN, MAINSEQ_END_SET );
	}

	BLINKPALANM_Main( wk->blink );

	return MAINSEQ_MAIN_MENU;
}

static int MainSeq_ExitMenu( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	if( wk->dat->sort->form != ZKNCOMM_LIST_SORT_NONE ){
		ZKNSEARCHOBJ_SetVanish( wk, ZKNSEARCHOBJ_IDX_FORM_M+wk->dat->sort->form, FALSE );
	}

	ZKNSEARCHUI_CursorMoveExit( wk );

	return wk->nextSeq;
}






static int MainSeq_InitRow( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_ROW;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
//		ZKNSEARCHOBJ_VanishAll( wk );
		ZKNSEARCHBMP_PutRowPage( wk );
		ZKNSEARCHLIST_MakeRowList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutRowPage( wk );
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_ROW;
		}
	}
	return MAINSEQ_INIT_ROW;
}
static int MainSeq_MainRow( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );


	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return MAINSEQ_EXIT_ROW;
	}
	return MAINSEQ_MAIN_ROW;
}

static int MainSeq_ExitRow( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


static int MainSeq_InitName( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_NAME;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
//		ZKNSEARCHOBJ_VanishAll( wk );
		ZKNSEARCHBMP_PutNamePage( wk );
		ZKNSEARCHLIST_MakeNameList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutNamePage( wk );
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_NAME;
		}
	}
	return MAINSEQ_INIT_NAME;
}

static int MainSeq_MainName( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return MAINSEQ_EXIT_NAME;
	}
	return MAINSEQ_MAIN_NAME;
}
static int MainSeq_ExitName( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


static int MainSeq_InitType( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_TYPE;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
//		ZKNSEARCHOBJ_VanishAll( wk );
		ZKNSEARCHBMP_PutTypePage( wk );
		ZKNSEARCHLIST_MakeTypeList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutTypePage( wk );
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_TYPE;
		}
	}
	return MAINSEQ_INIT_TYPE;
}
static int MainSeq_MainType( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return MAINSEQ_EXIT_TYPE;
	}
	return MAINSEQ_MAIN_TYPE;
}
static int MainSeq_ExitType( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


static int MainSeq_InitColor( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_COLOR;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
//		ZKNSEARCHOBJ_VanishAll( wk );
		ZKNSEARCHBMP_PutColorPage( wk );
		ZKNSEARCHLIST_MakeColorList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutColorPage( wk );
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_COLOR;
		}
	}
	return MAINSEQ_INIT_COLOR;
}
static int MainSeq_MainColor( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return MAINSEQ_EXIT_COLOR;
	}
	return MAINSEQ_MAIN_COLOR;
}
static int MainSeq_ExitColor( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}


static int MainSeq_InitForm( ZKNSEARCHMAIN_WORK * wk )
{
	switch( wk->pageSeq ){
	case 0:
		wk->page = ZKNSEARCHMAIN_PAGE_FORM;
		ZKNSEARCHMAIN_SetBlendAlpha( TRUE );
//		ZKNSEARCHOBJ_VanishAll( wk );
		ZKNSEARCHBMP_PutFormPage( wk );
		ZKNSEARCHLIST_MakeFormList( wk );
		wk->pageSeq++;
	case 1:
		if( FRAMELIST_Init( wk->lwk ) == FALSE ){
			ZKNSEARCHOBJ_PutFormPage( wk );
			ZKNSEARCHMAIN_ListBGOn( wk );
			wk->pageSeq = 0;
			return MAINSEQ_MAIN_FORM;
//		}else{
//			ZKNSEARCHOBJ_VanishList( wk );
		}
	}
	return MAINSEQ_INIT_FORM;
}
static int MainSeq_MainForm( ZKNSEARCHMAIN_WORK * wk )
{
	u32	ret = FRAMELIST_Main( wk->lwk );

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return MAINSEQ_EXIT_FORM;
	}
	return MAINSEQ_MAIN_FORM;
}
static int MainSeq_ExitForm( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ListBGOff( wk );
	ZKNSEARCHOBJ_VanishList( wk );
	ZKNSEARCHLIST_FreeList( wk );

	return MAINSEQ_INIT_MENU;
}



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
			return wk->funcSeq;
		}
	}else{
		if( ZKNSEARCHOBJ_CheckAnm( wk, wk->btnID ) == FALSE ){
			return wk->funcSeq;
		}
	}

	return MAINSEQ_BUTTON_ANM;
}

static int MainSeq_EndSet( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHUI_CursorMoveExit( wk );

	return SetWipeOut( wk, MAINSEQ_RELEASE );
}







static int SetWipeIn( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_SEARCH );
	wk->funcSeq = next;
	return MAINSEQ_WIPE;
}

static int SetWipeOut( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_SEARCH );
	wk->funcSeq = next;
	return MAINSEQ_WIPE;
}

static int SetFadeIn( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->funcSeq = next;
	return MAINSEQ_FADE;
}

static int SetFadeOut( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->funcSeq = next;
	return MAINSEQ_FADE;
}

static int SetButtonAnm( ZKNSEARCHMAIN_WORK * wk, u8 mode, u8 id, int next )
{
	wk->btnMode = mode;
	wk->btnID   = id;
	wk->btnSeq  = 0;
	wk->btnCnt  = 0;
	wk->funcSeq = next;
	return MAINSEQ_BUTTON_ANM;
}


static int PageChange( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->pageSeq = 0;
	wk->nextSeq = next;
	return MAINSEQ_EXIT_MENU;
}





