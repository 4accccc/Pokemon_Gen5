//============================================================================================
/**
 * @file		dpc_seq.c
 * @brief		殿堂入り確認画面 シーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCSEQ
 */
//============================================================================================
#include <gflib.h>

#include "system/wipe.h"

#include "dpc_main.h"
#include "dpc_seq.h"
#include "dpc_bmp.h"
#include "dpc_obj.h"
#include "dpc_ui.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,

	MAINSEQ_MAIN,
	MAINSEQ_BUTTON_ANM,

	MAINSEQ_END,
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( DPCMAIN_WORK * wk );
static int MainSeq_Release( DPCMAIN_WORK * wk );
static int MainSeq_Wipe( DPCMAIN_WORK * wk );
static int MainSeq_Main( DPCMAIN_WORK * wk );
static int MainSeq_ButtonAnm( DPCMAIN_WORK * wk );

static int SetFadeIn( DPCMAIN_WORK * wk, int next );
static int SetFadeOut( DPCMAIN_WORK * wk, int next );

static void ChangePage( DPCMAIN_WORK * wk );

FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

static const pDENDOUPC_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,

	MainSeq_Main,
	MainSeq_ButtonAnm,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
int DPCSEQ_MainSeq( DPCMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	DPCOBJ_AnmMain( wk );
	DPCBMP_PrintUtilTrans( wk );

	return TRUE;
}

static int MainSeq_Init( DPCMAIN_WORK * wk )
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

	DPCMAIN_CreatePokeData( wk );

	DPCMAIN_InitVram();
	DPCMAIN_InitBg();
	DPCMAIN_LoadBgGraphic();
	DPCMAIN_InitMsg( wk );

	DPCBMP_Init( wk );
	DPCOBJ_Init( wk );

	DPCUI_Init( wk );

	ChangePage( wk );

	DPCMAIN_SetBlendAlpha();

	GFL_NET_WirelessIconEasy_HoldLCD( TRUE, HEAPID_DENDOU_PC );
	GFL_NET_ReloadIcon();

	DPCMAIN_InitVBlank( wk );

	return SetFadeIn( wk, MAINSEQ_MAIN );
}

static int MainSeq_Release( DPCMAIN_WORK * wk )
{
	DPCMAIN_ExitVBlank( wk );

	DPCUI_Exit( wk );

	DPCOBJ_Exit( wk );
	DPCBMP_Exit( wk );

	DPCMAIN_ExitMsg( wk );
	DPCMAIN_ExitBg();

	DPCMAIN_ExitPokeData( wk );

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

static int MainSeq_Wipe( DPCMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->nextSeq;
	}
	return MAINSEQ_WIPE;
}

static int MainSeq_Main( DPCMAIN_WORK * wk )
{
	u32	ret = DPCUI_Main( wk );

	switch( ret ){
	case DPCUI_ID_POKE1:
	case DPCUI_ID_POKE2:
	case DPCUI_ID_POKE3:
	case DPCUI_ID_POKE4:
	case DPCUI_ID_POKE5:
	case DPCUI_ID_POKE6:
	case DPCUI_ID_PAGE_LEFT:
	case DPCUI_ID_PAGE_RIGHT:
		break;

	case DPCUI_ID_EXIT:
		wk->dat->retMode = DENDOUPC_RET_CLOSE;
		return SetFadeOut( wk, MAINSEQ_RELEASE );

	case DPCUI_ID_RETURN:
	case CURSORMOVE_CANCEL:					// キャンセル
		wk->dat->retMode = DENDOUPC_RET_NORMAL;
		return SetFadeOut( wk, MAINSEQ_RELEASE );

	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
		break;

	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_CURSOR_MOVE:		// 移動
	case CURSORMOVE_NONE:						// 動作なし
	default:
		break;
	}

	return MAINSEQ_MAIN;
}

static int MainSeq_ButtonAnm( DPCMAIN_WORK * wk )
{
	return MAINSEQ_BUTTON_ANM;
}





static int SetFadeIn( DPCMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_PC );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

static int SetFadeOut( DPCMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_PC );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}


static void ChangePage( DPCMAIN_WORK * wk )
{
	DPCOBJ_AddPoke( wk );

	DPCBMP_PutTitle( wk );
	DPCBMP_PutPage( wk );
	DPCBMP_PutInfo( wk );
}

