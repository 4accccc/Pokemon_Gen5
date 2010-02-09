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

#include "../zukan_common.h"
#include "zknsearch_main.h"
#include "zknsearch_seq.h"
#include "zknsearch_obj.h"
#include "zknsearch_bmp.h"
#include "zknsearch_ui.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,
	MAINSEQ_FADE,

	MAINSEQ_MAIN,

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


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Wipe( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Fade( ZKNSEARCHMAIN_WORK * wk );
static int MainSeq_Main( ZKNSEARCHMAIN_WORK * wk );
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

FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

static const pZKNSEARCH_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_Fade,

	MainSeq_Main,

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

/*
	ZKNLISTOBJ_AnmMain( wk );
	ZKNLISTBMP_PrintUtilTrans( wk );
//	BGWINFRM_MoveMain( wk->wfrm );
	ScrollBaseBg( wk );
*/
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
/*
	ZKNLISTMAIN_InitMsg( wk );

	ZKNLISTBMP_Init( wk );
	ZKNLISTOBJ_Init( wk );
//	ZKNLISTBGWFRM_Init( wk );

	ZKNLISTMAIN_MakeList( wk );

	ZKNLISTBMP_PutPokeEntryStr( wk );

	ZKNLISTMAIN_SetBlendAlpha();
*/
	ZKNSEARCHMAIN_InitVBlank( wk );
	ZKNSEARCHMAIN_InitHBlank( wk );

	return SetWipeIn( wk, MAINSEQ_MAIN );
}

static int MainSeq_Release( ZKNSEARCHMAIN_WORK * wk )
{
	ZKNSEARCHMAIN_ExitHBlank( wk );
	ZKNSEARCHMAIN_ExitVBlank( wk );
/*
	ZKNLISTMAIN_FreeList( wk );

//	ZKNLISTBGWFRM_Exit( wk );
	ZKNLISTOBJ_Exit( wk );
	ZKNLISTBMP_Exit( wk );

	ZKNLISTMAIN_ExitMsg( wk );
*/
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
		return wk->fadeSeq;
	}
	return MAINSEQ_WIPE;
}

static int MainSeq_Fade( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_FADE;
}

static int MainSeq_Main( ZKNSEARCHMAIN_WORK * wk )
{
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		return SetWipeOut( wk, MAINSEQ_RELEASE );
	}
	return MAINSEQ_MAIN;
}


static int MainSeq_InitRow( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_INIT_ROW;
}
static int MainSeq_MainRow( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_ROW;
}
static int MainSeq_ExitRow( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_EXIT_ROW;
}


static int MainSeq_InitName( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_INIT_NAME;
}
static int MainSeq_MainName( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_NAME;
}
static int MainSeq_ExitName( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_EXIT_NAME;
}


static int MainSeq_InitType( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_INIT_TYPE;
}
static int MainSeq_MainType( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_TYPE;
}
static int MainSeq_ExitType( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_EXIT_TYPE;
}


static int MainSeq_InitColor( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_INIT_COLOR;
}
static int MainSeq_MainColor( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_COLOR;
}
static int MainSeq_ExitColor( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_COLOR;
}


static int MainSeq_InitForm( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_INIT_FORM;
}
static int MainSeq_MainForm( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_MAIN_FORM;
}
static int MainSeq_ExitForm( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_EXIT_FORM;
}


static int MainSeq_ButtonAnm( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_BUTTON_ANM;
}

static int MainSeq_EndSet( ZKNSEARCHMAIN_WORK * wk )
{
	return MAINSEQ_END_SET;
}







static int SetWipeIn( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_SEARCH );
	wk->fadeSeq = next;
	return MAINSEQ_WIPE;
}

static int SetWipeOut( ZKNSEARCHMAIN_WORK * wk, int next )
{
	ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_SEARCH );
	wk->fadeSeq = next;
	return MAINSEQ_WIPE;
}

static int SetFadeIn( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->fadeSeq = next;
	return MAINSEQ_FADE;
}

static int SetFadeOut( ZKNSEARCHMAIN_WORK * wk, int next )
{
	wk->fadeSeq = next;
	return MAINSEQ_FADE;
}
