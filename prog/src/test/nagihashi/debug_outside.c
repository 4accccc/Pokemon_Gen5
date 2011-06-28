//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_outside.c
 *	@brief  管理外永続データデバッグ
 *	@author	Toru=Nagihashi
 *	@data		2011.06.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG
//	ライブラリ
#include <gflib.h>

//システム
#include "pm_version.h"
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "savedata/save_tbl.h"
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"

//アーカイブ
#include "arc_def.h"
#include "debug_message.naix"
#include "font/font.naix"
#include "msg/debug/msg_d_nagihashi.h"

//モジュール
#include "debug_graphic.h"
#include "debug_util.h"
#include "debug/debugwin_sys.h"
#include "debugwin_keysystem.h"

//外部公開
#include "debug_outside.h"

FS_EXTERN_OVERLAY( outside_save );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	パレット
//=====================================
enum
{ 
  PLT_BG_FONT_M = 0,

  PLT_BG_FONT_S = 0,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{
  BG_FRAME_TEXT_M = GFL_BG_FRAME0_M,

  BG_FRAME_BACK_S = GFL_BG_FRAME0_S,
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
typedef struct
{
  DEUBG_GRAPHIC_WORK        *p_graphic;
  DEBUG_SEQ_WORK            *p_seq;
  DEBUG_MSGWIN_WORK         *p_msgwin;
  GFL_FONT                  *p_font;
  PRINT_QUE                 *p_que;
  GFL_MSGDATA               *p_msg;
  OUTSIDE_SAVE_CONTROL      *p_outside_sv;
} DEBUG_OUTSIDE_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT Debug_Outside_Proc_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Outside_Proc_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Outside_Proc_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
/// シーケンス
//=====================================
static void Seqfunc_FadeIn( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Seqfunc_Main( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Seqfunc_FadeOut( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// その他
//=====================================
static void Util_LoadBg( HEAPID heapID );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
const GFL_PROC_DATA DEBUG_OUTSIDE_ProcData = 
{
  Debug_Outside_Proc_Init,
  Debug_Outside_Proc_Main,
  Debug_Outside_Proc_Exit,
};

//=============================================================================
/**
 *				PROC
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	管理外デバッグPROC初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Outside_Proc_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x20000 );

  {
	  DEBUG_OUTSIDE_WORK * p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_OUTSIDE_WORK), HEAPID_NAGI_DEBUG_SUB );
	  GFL_STD_MemClear( p_wk, sizeof(DEBUG_OUTSIDE_WORK) );	

    //共通モジュールの初期化
    p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NAGI_DEBUG_SUB );
    p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_NAGI_DEBUG_SUB );
    p_wk->p_msg     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_nagihashi_dat, HEAPID_NAGI_DEBUG_SUB );

    //管理外セーブ取得
    p_wk->p_outside_sv = OutsideSave_SystemLoad(HEAPID_NAGI_DEBUG_SUB);

    //グラフィック
    p_wk->p_graphic = DEUBG_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_NAGI_DEBUG_SUB );
    Util_LoadBg( HEAPID_NAGI_DEBUG_SUB );

    //シーケンス
    p_wk->p_seq = DEBUG_SEQ_Init( p_wk, Seqfunc_FadeIn, HEAPID_NAGI_DEBUG_SUB );

    //メッセージ
    p_wk->p_msgwin = DEBUG_MSGWIN_Init( DEBUG_MSGWIN_TRANS_MODE_AUTO, BG_FRAME_BACK_S, 1, 10, 30, 4, PLT_BG_FONT_S, p_wk->p_que, HEAPID_NAGI_DEBUG_SUB );
    DEBUG_MSGWIN_Print( p_wk->p_msgwin, p_wk->p_msg, OUTSIDE_STR_01, p_wk->p_font );

    //デバッグウィンドウ
    DEBUGWIN_InitProc( BG_FRAME_TEXT_M, p_wk->p_font );
    {
      OUTSIDE_PERMANENT *p_per = OutsideSave_GetPermanentPtr(p_wk->p_outside_sv);
      KEY_SYSTEM_SAVE *p_key_system = OUTSIDE_PERMANENT_GetKeySystem(p_per);
      DEBUGWIN_KEY_SYSTEM_Init( p_key_system, HEAPID_NAGI_DEBUG_SUB );
    }
  }


  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	管理外デバッグPROC破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Outside_Proc_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_OUTSIDE_WORK * p_wk = p_wk_adrs;

  //セーブしたらチェックしないといけない
  OS_ResetSystem(0);

  //デバッグウィンドウ
  DEBUGWIN_KEY_SYSTEM_Exit();
  DEBUGWIN_ExitProc();

  //メッセージ
  DEBUG_MSGWIN_Exit( p_wk->p_msgwin );

  //シーケンス
  DEBUG_SEQ_Exit( p_wk->p_seq );

  //グラフィック
  DEUBG_GRAPHIC_Exit( p_wk->p_graphic );

  //管理外セーブ破棄
  OutsideSave_SystemUnload(p_wk->p_outside_sv);

	//共通モジュールの破棄
  GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	管理外デバッグPROCメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Outside_Proc_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_OUTSIDE_WORK * p_wk = p_wk_adrs;

  //シーケンス
  DEBUG_SEQ_Main( p_wk->p_seq );
  if( DEBUG_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //描画
  DEUBG_GRAPHIC_2D_Draw( p_wk->p_graphic );

  PRINTSYS_QUE_Main( p_wk->p_que );
  DEBUG_MSGWIN_PrintMain( p_wk->p_msgwin );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *    シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Seqfunc_FadeIn( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		DEBUG_SEQ_SetNext( p_seqwk, Seqfunc_Main );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Seqfunc_Main( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  {
    SEQ_MAIN,
    SEQ_SAVE_START,
    SEQ_SAVE_WAIT,
  };

  DEBUG_OUTSIDE_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_MAIN:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
    {
      DEBUG_SEQ_SetNext( p_seqwk, Seqfunc_FadeOut );
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
    {
      DEBUG_MSGWIN_Print( p_wk->p_msgwin, p_wk->p_msg, OUTSIDE_STR_02, p_wk->p_font );
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_START:
    if( DEBUG_MSGWIN_IsPrintEnd( p_wk->p_msgwin ) )
    {
      OutsideSave_SaveAsyncInit( p_wk->p_outside_sv );
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_WAIT:
    if( OutsideSave_SaveAsyncMain( p_wk->p_outside_sv ) )
    {
      DEBUG_MSGWIN_Print( p_wk->p_msgwin, p_wk->p_msg, OUTSIDE_STR_01, p_wk->p_font );
      (*p_seq) = SEQ_MAIN;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void Seqfunc_FadeOut( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_EXIT:
		DEBUG_SEQ_End( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG読み込み
 */
//-----------------------------------------------------------------------------
static void Util_LoadBg( HEAPID heapID )
{ 
  //フォント
  {	
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上下画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, PLT_BG_FONT_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, PLT_BG_FONT_S*0x20, 0x20, heapID );

      GFL_ARC_CloseDataHandle( p_handle );
  }

  //他のBGは真っ白
  GFL_BG_SetBackGroundColor( BG_FRAME_TEXT_M, 0xFFFF );
  GFL_BG_SetBackGroundColor( BG_FRAME_BACK_S, 0xFFFF );
  GFL_BG_LoadScreenReq( BG_FRAME_TEXT_M );
  GFL_BG_LoadScreenReq( BG_FRAME_BACK_S );
}

#endif
