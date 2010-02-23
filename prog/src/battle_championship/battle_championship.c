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

#include "arc_def.h"
#include "battle_championship.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_battle_championship.h"
#include "script_message.naix"
#include "msg/script/msg_common_scr.h"

#include "title/title.h"
#include "net_app/irc_battle.h"
#include "net_app/wifibattlematch.h"
#include "net_app/digitalcardcheck.h"

#include "battle_championship/battle_championship.h"
#include "test/ariizumi/ari_debug.h"

//WIFI世界対戦のグラフィックを流用
#include "net_app/wifi_match/wifibattlematch_graphic.h"
#include "net_app/wifi_match/wifibattlematch_view.h"
#include "net_app/wifi_match/wifibattlematch_util.h"

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

//切り替え先Proc種類
typedef enum
{
  BCNP_TITLE,
  BCNP_WIFI_BATTLE,
  BCNP_EVENT_BATTLE,
  
}BATTLE_CHAMPIONSHIP_NEXT_PROC;

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
  BATTLE_CHAMPIONSHIP_NEXT_PROC nextProcType;
  
  //メッセージ用
  WBM_TEXT_WORK    *text;
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  GFL_MSGDATA     *scr_msgHandle;
  PRINT_QUE       *taskMenuQue;

  //シーケンス管理
  WBM_SEQ_WORK     *p_seq;
  
  //メニュー
  WBM_LIST_WORK    *list;

  //描画
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_view;
  WIFIBATTLEMATCH_GRAPHIC_WORK  *p_graphic;

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
static void BATTLE_CHAMPIONSHIP_ShowMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk  , const u16 msgNo );
static void BATTLE_CHAMPIONSHIP_ShowErrMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk  , const u16 msgNo );
static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk );

//-------------------------------------
///	メニュー
//=====================================
typedef enum
{ 
  BC_MENU_TYPE_FIRST,
  BC_MENU_TYPE_WIFI,
  BC_MENU_TYPE_LIVE,
  BC_MENU_TYPE_YESNO,
}BC_MENU_TYPE;
static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk, BC_MENU_TYPE type );
static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk );
static const u32 BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk );

//-------------------------------------
///	シーケンス関数
//=====================================
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeIn( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeOut( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainMenu( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Info( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_WifiMenu( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_WifiInfo( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DigitalCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_RestrictUGC( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_EnableWireless( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//--------------------------------------------------------------
//  外部公開プロセス
//--------------------------------------------------------------
GFL_PROC_DATA BATTLE_CHAMPIONSHIP_ProcData =
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
  p_wk->nextProcType  = BCNP_TITLE;
  p_wk->heapId = HEAPID_BATTLE_CHAMPIONSHIP;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(wifibattlematch_view) );

  BATTLE_CHAMPIONSHIP_InitGraphic( p_wk );
  BATTLE_CHAMPIONSHIP_LoadResource( p_wk );
  BATTLE_CHAMPIONSHIP_InitMessage( p_wk );

  p_wk->p_seq = WBM_SEQ_Init( p_wk, SEQFUNC_FadeIn, HEAPID_BATTLE_CHAMPIONSHIP );
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *p_wk = mywk;

  WBM_SEQ_Exit( p_wk->p_seq);

  BATTLE_CHAMPIONSHIP_TermMessage( p_wk );
  BATTLE_CHAMPIONSHIP_ReleaseResource( p_wk );
  BATTLE_CHAMPIONSHIP_TermGraphic( p_wk );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifibattlematch_view) );

  switch( p_wk->nextProcType )
  { 
  default:
    GF_ASSERT(0);
  case BCNP_TITLE:
    GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
    break;
  case BCNP_WIFI_BATTLE:
    { 
      WIFIBATTLEMATCH_PARAM *p_param;
      p_param = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(WIFIBATTLEMATCH_PARAM) );
      GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_PARAM) );
      p_param->mode             = WIFIBATTLEMATCH_MODE_WIFI;
      p_param->is_auto_release  = TRUE;
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(wifibattlematch_sys), &WifiBattleMatch_ProcData, p_param );
    }
    break;

  case BCNP_EVENT_BATTLE:
    GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &IRC_BATTLE_ProcData, NULL);
    break;
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CHAMPIONSHIP );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *p_wk = mywk;
  
  WBM_SEQ_Main( p_wk->p_seq );
  BATTLE_CHAMPIONSHIP_MainGraphic( p_wk );

  if( WBM_SEQ_IsEnd( p_wk->p_seq ) )
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
  p_wk->scr_msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_common_scr_dat, p_wk->heapId );
  
  p_wk->taskMenuQue = PRINTSYS_QUE_Create( p_wk->heapId );
  
  //メッセージ
  p_wk->text    = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->taskMenuQue, p_wk->fontHandle, p_wk->heapId );
}

static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WBM_TEXT_Exit( p_wk->text );

  PRINTSYS_QUE_Delete( p_wk->taskMenuQue );
  
  GFL_MSG_Delete( p_wk->scr_msgHandle );
  GFL_MSG_Delete( p_wk->msgHandle );
  GFL_FONT_Delete( p_wk->fontHandle );
}

static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WBM_TEXT_Main( p_wk->text );
  PRINTSYS_QUE_Main( p_wk->taskMenuQue );
}
static void BATTLE_CHAMPIONSHIP_ShowMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk  , const u16 msgNo )
{
  WBM_TEXT_Print( p_wk->text, p_wk->msgHandle, msgNo, WBM_TEXT_TYPE_STREAM );
}
static void BATTLE_CHAMPIONSHIP_ShowErrMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk  , const u16 msgNo )
{
  WBM_TEXT_Print( p_wk->text, p_wk->scr_msgHandle, msgNo, WBM_TEXT_TYPE_STREAM );
}

static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  return WBM_TEXT_IsEnd( p_wk->text );
}

#pragma mark [>menu func
static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk, BC_MENU_TYPE type )
{
  enum
  { 
    POS_CENTER,
    POS_RIGHT_DOWN,
  } pos;

  u8 x,y,w,h;


  WBM_LIST_SETUP  setup;
  GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
  setup.p_msg   = p_wk->msgHandle;
  setup.p_font  = p_wk->fontHandle;
  setup.p_que   = p_wk->taskMenuQue;


  switch( type )
  { 
  case BC_MENU_TYPE_FIRST:
    setup.strID[0]= BC_SELECT_01;
    setup.strID[1]= BC_SELECT_02;
    setup.strID[2]= BC_SELECT_04;
    setup.strID[3]= BC_SELECT_05;
    setup.list_max= 4;
    pos = POS_CENTER;
    break;
  case BC_MENU_TYPE_WIFI:
    setup.strID[0]= BC_SELECT_06;
    setup.strID[1]= BC_SELECT_04;
    setup.strID[2]= BC_SELECT_03;
    setup.strID[3]= BC_SELECT_05;
    setup.list_max= 4;
    pos = POS_CENTER;
    break;
  case BC_MENU_TYPE_LIVE:
    setup.strID[0]= BC_SELECT_05;
    setup.list_max= 1;
    pos = POS_CENTER;
    break;
  case BC_MENU_TYPE_YESNO:
    setup.strID[0]= BC_SELECT_07;
    setup.strID[1]= BC_SELECT_08;
    setup.list_max= 2;
    pos = POS_RIGHT_DOWN;
    break;
  }
  setup.frm     = BG_FRAME_M_TEXT;
  setup.font_plt= PLT_FONT_M;
  setup.frm_plt = PLT_LIST_M;
  setup.frm_chr = CGR_OFS_M_LIST;


  switch( pos )
  { 
  case POS_CENTER:
    w  = 28;
    h  = setup.list_max * 2;
    x  = 32 / 2  - w /2; 
    y  = (24-6) /2  - h /2;
    break;
  case POS_RIGHT_DOWN:
    w  = 12;
    h  = setup.list_max * 2;
    x  = 32 - w - 1; //1はフレーム分
    y  = 24 - h - 1 - 6; //１は自分のフレーム分と6はテキスト分
    break;
  }

  p_wk->list  = WBM_LIST_InitEx( &setup, x, y, w, h, p_wk->heapId );
}

static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  WBM_LIST_Exit( p_wk->list );
}

static const u32 BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *p_wk )
{
  return WBM_LIST_Main( p_wk->list );
}
//=============================================================================
/**
 *  シーケンス関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_seqwk_adrs )
{ 
  WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
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
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_seqwk_adrs )
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
static void SEQFUNC_FadeIn( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_seqwk_adrs )
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
		WBM_SEQ_SetNext( p_seqwk, SEQFUNC_MainMenu );
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
 *	@brief  メインメニュー
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
    SEQ_START_FIRSTMSG,
    SEQ_WAIT_FIRSTMSG,
    SEQ_START_MENU,
    SEQ_WAIT_MENU,
  };
  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_FIRSTMSG:
    BATTLE_CHAMPIONSHIP_ShowMessage( p_wk, BC_STR_01 );
    *p_seq = SEQ_WAIT_FIRSTMSG;
    break;

  case SEQ_WAIT_FIRSTMSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      *p_seq = SEQ_START_MENU;
    }
    break;

  case SEQ_START_MENU:
    BATTLE_CHAMPIONSHIP_OpenFirstMenu( p_wk, BC_MENU_TYPE_FIRST );
    *p_seq = SEQ_WAIT_MENU;
    break;

  case SEQ_WAIT_MENU:
    {
      const u32 ret = BATTLE_CHAMPIONSHIP_UpdateFirstMenu( p_wk );
      if( ret != WBM_LIST_SELECT_NULL )
      {
        switch( ret )
        {
        case 0: //WiFi大会
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiMenu );
          break;
        case 1: //イベント大会
          //WBM_SEQ_SetNext( p_seqwk, SEQFUNC_LiveMenu );
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
          break;
        case 2: //説明をきく
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_Info );
          break;
        case 3: //やめる
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
          break;
        }
        BATTLE_CHAMPIONSHIP_CloseFirstMenu( p_wk );
      }
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	説明画面
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Info( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_INFOMSG,
    SEQ_WAIT_INFOMSG,
  };

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_INFOMSG:
    BATTLE_CHAMPIONSHIP_ShowMessage( p_wk, BC_STR_02 );
    *p_seq  = SEQ_WAIT_INFOMSG;
    break;
  
    break;
  case SEQ_WAIT_INFOMSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      WBM_SEQ_SetNext( p_seqwk, SEQFUNC_MainMenu );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	Wifiメニュー
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_WifiMenu( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_WIFIMSG,
    SEQ_WAIT_WIFIMSG,
    SEQ_START_WIFIMENU,
    SEQ_WAIT_WIFIMENU,

    SEQ_START_CANCELMSG,
    SEQ_WAIT_CANCELMSG,
    SEQ_START_CANCELMENU,
    SEQ_WAIT_CANCELMENU,
  };
  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_WIFIMSG:
    BATTLE_CHAMPIONSHIP_ShowMessage( p_wk, BC_STR_04 );
    *p_seq = SEQ_WAIT_WIFIMSG;
    break;

  case SEQ_WAIT_WIFIMSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      *p_seq = SEQ_START_WIFIMENU;
    }
    break;

  case SEQ_START_WIFIMENU:
    BATTLE_CHAMPIONSHIP_OpenFirstMenu( p_wk, BC_MENU_TYPE_WIFI );
    *p_seq = SEQ_WAIT_WIFIMENU;
    break;

  case SEQ_WAIT_WIFIMENU:
    {
      const u32 ret = BATTLE_CHAMPIONSHIP_UpdateFirstMenu( p_wk );
      if( ret != WBM_LIST_SELECT_NULL )
      {
        switch( ret )
        {
        case 0: //さんかする
          if( !DS_SYSTEM_IsAvailableWireless() )
          { 
            //DSの無線設定で通信不可のとき
            WBM_SEQ_SetNext( p_seqwk, SEQFUNC_EnableWireless );
          }
          else if( DS_SYSTEM_IsRestrictUGC() )
          { 
            //ペアレンタルコントロールで送受信拒否しているとき
            WBM_SEQ_SetNext( p_seqwk, SEQFUNC_RestrictUGC );
          }
          else
          { 
            //接続できるとき
            WBM_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
            p_wk->nextProcType = BCNP_WIFI_BATTLE;
          }
          break;
        case 1: //説明を聞く
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiInfo );
          break;
        case 2: //デジタル選手証をみる
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_DigitalCard );
          break;
        case 3: //やめる
          *p_seq  = SEQ_START_CANCELMSG;
          break;
        }
        BATTLE_CHAMPIONSHIP_CloseFirstMenu( p_wk );
      }
    }
    break;

  case SEQ_START_CANCELMSG:
    BATTLE_CHAMPIONSHIP_ShowMessage( p_wk, BC_STR_05 );
    *p_seq = SEQ_WAIT_CANCELMSG;
    break;

  case SEQ_WAIT_CANCELMSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      *p_seq = SEQ_START_CANCELMENU;
    }
    break;

  case SEQ_START_CANCELMENU:
    BATTLE_CHAMPIONSHIP_OpenFirstMenu( p_wk, BC_MENU_TYPE_YESNO );
    *p_seq = SEQ_WAIT_CANCELMENU;
    break;

  case SEQ_WAIT_CANCELMENU:
    {
      const u32 ret = BATTLE_CHAMPIONSHIP_UpdateFirstMenu( p_wk );
      if( ret != WBM_LIST_SELECT_NULL )
      {
        switch( ret )
        {
        case 0: //はい
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_MainMenu );
          break;
        case 1: //いいえ
          *p_seq  =SEQ_START_WIFIMSG;
          break;
        }
        BATTLE_CHAMPIONSHIP_CloseFirstMenu( p_wk );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	Wifi説明画面
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_WifiInfo( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_INFOMSG,
    SEQ_WAIT_INFOMSG,
  };

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_INFOMSG:
    BATTLE_CHAMPIONSHIP_ShowMessage( p_wk, BC_STR_03 );
    *p_seq  = SEQ_WAIT_INFOMSG;
    break;
  
    break;
  case SEQ_WAIT_INFOMSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiMenu );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	デジタル選手証
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DigitalCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
  };

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    (*p_seq)++;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			(*p_seq)++;
		}
		break;

  case SEQ_INIT:
    BATTLE_CHAMPIONSHIP_TermMessage( p_wk );
    BATTLE_CHAMPIONSHIP_ReleaseResource( p_wk );
    BATTLE_CHAMPIONSHIP_TermGraphic( p_wk );

    GFL_PROC_SysCallProc( NO_OVERLAY_ID, &DigitalCard_ProcData, NULL );
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    (*p_seq)++;
    break;

  case SEQ_END:

    BATTLE_CHAMPIONSHIP_InitGraphic( p_wk );
    BATTLE_CHAMPIONSHIP_LoadResource( p_wk );
    BATTLE_CHAMPIONSHIP_InitMessage( p_wk );

    (*p_seq)++;
    break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
      WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiMenu );
		}
		break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	ペアレンタルコントロールでユーザー作成コンテンツの送受信を拒否していた場合
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_RestrictUGC( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG,
    SEQ_WAIT_MSG,
  };

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_MSG:
    //@TODO
    BATTLE_CHAMPIONSHIP_ShowErrMessage( p_wk, msg_common_wireless_off_keywait );
    *p_seq  = SEQ_WAIT_MSG;
    break;
  
    break;
  case SEQ_WAIT_MSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiMenu );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	DSの無線通信設定で通信負荷にしていた場合
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_EnableWireless( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG,
    SEQ_WAIT_MSG,
  };

  BATTLE_CHAMPIONSHIP_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_MSG:
    BATTLE_CHAMPIONSHIP_ShowErrMessage( p_wk, msg_common_wireless_off_keywait );
    *p_seq  = SEQ_WAIT_MSG;
    break;
  
  case SEQ_WAIT_MSG:
    BATTLE_CHAMPIONSHIP_UpdateMessage( p_wk );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( p_wk ) == TRUE )
    {
      WBM_SEQ_SetNext( p_seqwk, SEQFUNC_WifiMenu );
    }
    break;
  }
}
