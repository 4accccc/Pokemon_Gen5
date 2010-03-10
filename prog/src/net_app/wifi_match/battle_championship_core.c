//======================================================================
/**
 * @file	battle_championship.c
 * @brief	大会メニュー
 * @author	ariizumi
 * @author  Toru=Nagihashi 引継
 * @data	09/10/08 -> 100112
 *
 * モジュール名：BATTLE_CHAMPINONSHIP
 */
//======================================================================
#include <gflib.h>
#include "buflen.h"

#include "app/app_taskmenu.h"
#include "gamesystem/msgspeed.h"
#include "print/printsys.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_regulation.h"
#include "system/bmp_winframe.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/bmp_menulist.h"
#include "app/app_keycursor.h"
#include "system/ds_system.h"
#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "battle_championship.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_battle_championship.h"
#include "script_message.naix"
#include "msg/script/msg_common_scr.h"

#include "net_app/irc_battle.h"
#include "net/dreamworld_netdata.h"

#include "battle_championship_core.h"

//WIFI対戦のグラフィックを流用
#include "net_app/wifi_match/wifibattlematch_graphic.h"
#include "net_app/wifi_match/wifibattlematch_view.h"
#include "net_app/wifi_match/wifibattlematch_util.h"

#include "battle_championship_flow.h"
#include "livebattlematch_flow.h"

//-------------------------------------
///	OVERLAYエクスターン
//=====================================
FS_EXTERN_OVERLAY(wifibattlematch_view);

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
//-------------------------------------
///	メインワーク
//=====================================
typedef struct
{
  HEAPID heapId;
  
  //フロー
  WBM_SEQ_WORK    *p_seq;

  //メッセージ用
  WBM_TEXT_WORK   *text;
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  PRINT_QUE       *taskMenuQue;
  
  //メニュー
  WBM_LIST_WORK    *list;
  u32               list_type;

  //描画
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_view;
  WIFIBATTLEMATCH_GRAPHIC_WORK  *p_graphic;

  //引数
  BATTLE_CHAMPIONSHIP_CORE_PARAM  *p_param;

  //フロー
  void            *p_flow;

}BATTLE_CHAMPIONSHIP_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

//-------------------------------------
///	グラフィック
//=====================================
static void BATTLE_CHAMPIONSHIP_InitGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_TermGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_MainGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_LoadResource( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_ReleaseResource( BATTLE_CHAMPIONSHIP_WORK *p_wk );

//-------------------------------------
///	メッセージ
//=====================================
static void BATTLE_CHAMPIONSHIP_InitMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk );

//-------------------------------------
///	シーケンス
//=====================================
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeIn( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeOut( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainMenu( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_LiveCup( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//--------------------------------------------------------------
//  外部公開プロセス
//--------------------------------------------------------------
const GFL_PROC_DATA BATTLE_CHAMPIONSHIP_CORE_ProcData =
{
  BATTLE_CHAMPIONSHIP_ProcInit,
  BATTLE_CHAMPIONSHIP_ProcMain,
  BATTLE_CHAMPIONSHIP_ProcTerm
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
#pragma mark [>proc 
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *p_wk;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_CHAMPIONSHIP, 0x30000 );

  p_wk = GFL_PROC_AllocWork( proc, sizeof(BATTLE_CHAMPIONSHIP_WORK), HEAPID_BATTLE_CHAMPIONSHIP );
  GFL_STD_MemClear( p_wk, sizeof(BATTLE_CHAMPIONSHIP_WORK) );
  p_wk->p_param = pwk;
  p_wk->heapId = HEAPID_BATTLE_CHAMPIONSHIP;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(wifibattlematch_view) );

  BATTLE_CHAMPIONSHIP_InitGraphic( p_wk );
  BATTLE_CHAMPIONSHIP_LoadResource( p_wk );
  BATTLE_CHAMPIONSHIP_InitMessage( p_wk );

  p_wk->p_seq = WBM_SEQ_Init( p_wk, SEQFUNC_FadeIn, HEAPID_BATTLE_CHAMPIONSHIP );

  PMSND_PushBGM();
  PMSND_PlayBGM( SEQ_BGM_WCS );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *p_wk = mywk;

  PMSND_PopBGM();

  WBM_SEQ_Exit( p_wk->p_seq);

  BATTLE_CHAMPIONSHIP_TermMessage( p_wk );
  BATTLE_CHAMPIONSHIP_ReleaseResource( p_wk );
  BATTLE_CHAMPIONSHIP_TermGraphic( p_wk );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifibattlematch_view) );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CHAMPIONSHIP );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *p_wk = mywk;
  BATTLE_CHAMPIONSHIP_MainGraphic( p_wk );
  BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );

  WBM_SEQ_Main( p_wk->p_seq );

  if( WBM_SEQ_IsEnd( p_wk->p_seq )  ) 
  { 
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

#pragma mark [>graphic func
static void BATTLE_CHAMPIONSHIP_InitGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  p_wk->p_graphic = WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, p_wk->heapId );
}
static void BATTLE_CHAMPIONSHIP_TermGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );
  p_wk->p_graphic = NULL;
}
static void BATTLE_CHAMPIONSHIP_MainGraphic( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{ 
  if( p_wk->p_graphic )
  { 
    WIFIBATTLEMATCH_GRAPHIC_2D_Draw( p_wk->p_graphic );
    WIFIBATTLEMATCH_VIEW_Main( p_wk->p_view );
  }
}

static void BATTLE_CHAMPIONSHIP_LoadResource( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  p_wk->p_view  = WIFIBATTLEMATCH_VIEW_LoadResource( WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic ), WIFIBATTLEMATCH_VIEW_RES_MODE_MENU, p_wk->heapId );
}

static void BATTLE_CHAMPIONSHIP_ReleaseResource( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WIFIBATTLEMATCH_VIEW_UnLoadResource( p_wk->p_view );
}

#pragma mark [>message func
static void BATTLE_CHAMPIONSHIP_InitMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  p_wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , p_wk->heapId );
  
  p_wk->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_battle_championship_dat , p_wk->heapId );
  
  p_wk->taskMenuQue = PRINTSYS_QUE_Create( p_wk->heapId );
  
  //メッセージ
  p_wk->text    = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->taskMenuQue, p_wk->fontHandle, p_wk->heapId );
}

static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WBM_TEXT_Exit( p_wk->text );

  PRINTSYS_QUE_Delete( p_wk->taskMenuQue );
  
  GFL_MSG_Delete( p_wk->msgHandle );
  GFL_FONT_Delete( p_wk->fontHandle );
}

static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WBM_TEXT_Main( p_wk->text );
  PRINTSYS_QUE_Main( p_wk->taskMenuQue );
}
#pragma mark [>message func
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( p_wk->p_param->mode )
  { 
  case BATTLE_CHAMPIONSHIP_CORE_MODE_MAIN_MEMU: //メインメニューへいく
  case BATTLE_CHAMPIONSHIP_CORE_MODE_WIFI_MENU: //WIFIメニューへいく
  case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_MENU: //LIVEメニューへいく
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_MainMenu );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_START: //ライブ大会フロー開始へいく（いらない？）
  case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_BTLEND: //ライブ大会フローバトル終了後へいく
  case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_RECEND: //ライブ大会フロー録画終了後へいく
  case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_MENU:   //ライブ大会フローメインメニューへいく
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_LiveCup );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_seqwk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  //終了
  WBM_SEQ_End( p_seqwk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_seqwk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
		WBM_SEQ_SetNext( p_seqwk, SEQFUNC_Start );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
		WBM_SEQ_SetNext( p_seqwk, SEQFUNC_End );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	メインメニュー
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MainMenu( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
	{
		SEQ_INIT,
		SEQ_MAIN,
		SEQ_EXIT,
		SEQ_NEXT,
	};

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

	switch( *p_seq )
	{	
  case SEQ_INIT:
    { 
      BATTLE_CHAMPIONSHIP_FLOW_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(BATTLE_CHAMPIONSHIP_FLOW_PARAM ) );
      param.p_text      = p_wk->text;
      param.p_font      = p_wk->fontHandle;
      param.p_msg       = p_wk->msgHandle;
      param.p_que       = p_wk->taskMenuQue;
      param.p_view      = p_wk->p_view;
      param.p_graphic   = p_wk->p_graphic;
      param.p_gamedata  = p_wk->p_param->p_gamedata;

      switch( p_wk->p_param->mode )
      { 
      case BATTLE_CHAMPIONSHIP_CORE_MODE_MAIN_MEMU:
        param.mode  = BATTLE_CHAMPIONSHIP_FLOW_MODE_MAINMENU;
        break;
      case BATTLE_CHAMPIONSHIP_CORE_MODE_WIFI_MENU:
        param.mode  = BATTLE_CHAMPIONSHIP_FLOW_MODE_WIFIMENU;
        break;
      }

      p_wk->p_flow  = BATTLE_CHAMPIONSHIP_FLOW_Init( &param, HEAPID_BATTLE_CHAMPIONSHIP );
    }
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    BATTLE_CHAMPIONSHIP_FLOW_Main( p_wk->p_flow );
    switch( BATTLE_CHAMPIONSHIP_FLOW_IsEnd( p_wk->p_flow ) )
    { 
    case BATTLE_CHAMPIONSHIP_FLOW_RET_TITLE:     //終了してタイトルへ
      p_wk->p_param->ret  = BATTLE_CHAMPIONSHIP_CORE_RET_TITLE;
      (*p_seq)  = SEQ_EXIT;
      break;
    case BATTLE_CHAMPIONSHIP_FLOW_RET_WIFICUP:   //終了してWIFI大会へ
      p_wk->p_param->ret  = BATTLE_CHAMPIONSHIP_CORE_RET_WIFICUP;
      (*p_seq)  = SEQ_EXIT;
      break;
    case BATTLE_CHAMPIONSHIP_FLOW_RET_LIVECUP:   //ライブ大会へ 
      (*p_seq)  = SEQ_NEXT;
      break;
    }
    break;
	case SEQ_EXIT:
    BATTLE_CHAMPIONSHIP_FLOW_Exit( p_wk->p_flow );
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
    break;

  case SEQ_NEXT:
    BATTLE_CHAMPIONSHIP_FLOW_Exit( p_wk->p_flow );
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_LiveCup );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	ライブカップ
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_LiveCup( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
	{
		SEQ_INIT,
		SEQ_MAIN,
		SEQ_EXIT,
		SEQ_NEXT,
	};

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

	switch( *p_seq )
	{	
  case SEQ_INIT:
    { 
      LIVEBATTLEMATCH_FLOW_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LIVEBATTLEMATCH_FLOW_PARAM ) );
      param.p_text      = p_wk->text;
      param.p_font      = p_wk->fontHandle;
      param.p_que       = p_wk->taskMenuQue;
      param.p_view      = p_wk->p_view;
      param.p_graphic   = p_wk->p_graphic;
      param.p_gamedata  = p_wk->p_param->p_gamedata;
      param.p_player_data = p_wk->p_param->p_player_data;
      param.p_enemy_data  = p_wk->p_param->p_enemy_data;

      switch( p_wk->p_param->mode )
      { 
      default:
      case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_START:
        param.mode  = LIVEBATTLEMATCH_FLOW_MODE_START;
        break;
      case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_BTLEND:
        param.mode  = LIVEBATTLEMATCH_FLOW_MODE_BTL;
        break;
      case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_RECEND:
        param.mode  = LIVEBATTLEMATCH_FLOW_MODE_REC;
        break;
      case BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_MENU:
        param.mode  = LIVEBATTLEMATCH_FLOW_MODE_MENU;
        break;
      }

      p_wk->p_flow  = LIVEBATTLEMATCH_FLOW_Init( &param, HEAPID_BATTLE_CHAMPIONSHIP );
    }
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    LIVEBATTLEMATCH_FLOW_Main( p_wk->p_flow );
    switch( LIVEBATTLEMATCH_FLOW_IsEnd( p_wk->p_flow ) )
    { 
    case LIVEBATTLEMATCH_FLOW_RET_LIVEMENU:   //バトル大会ライブ大会メニューへ 
      (*p_seq)  = SEQ_NEXT;
      break;
    case LIVEBATTLEMATCH_FLOW_RET_BATTLE:    //バトルへ
      p_wk->p_param->ret  = BATTLE_CHAMPIONSHIP_CORE_RET_LIVEBTL;
      (*p_seq)  = SEQ_EXIT;
      break;
    case LIVEBATTLEMATCH_FLOW_RET_REC:       //録画へ
      p_wk->p_param->ret  = BATTLE_CHAMPIONSHIP_CORE_RET_LIVEREC;
      (*p_seq)  = SEQ_EXIT;
      break;
    case LIVEBATTLEMATCH_FLOW_RET_BTLREC:    //録画再生へ
      p_wk->p_param->ret  = BATTLE_CHAMPIONSHIP_CORE_RET_LIVERECPLAY;
      (*p_seq)  = SEQ_EXIT;
      break;
    }
    break;
	case SEQ_EXIT:
    LIVEBATTLEMATCH_FLOW_Exit( p_wk->p_flow );
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
    break;

  case SEQ_NEXT:
    LIVEBATTLEMATCH_FLOW_Exit( p_wk->p_flow );
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_MainMenu );
    break;
  }
}
