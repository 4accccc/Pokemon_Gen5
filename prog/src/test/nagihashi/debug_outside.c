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

//-------------------------------------
///	デバッグ番号
//=====================================
enum
{
  OUTSIDE_DEBUGWIN_GROUP    = 0,
  KEYSYSTEM_DEBUGWIN_GROUP  = 1,
  KEYSYSTEM_SELF_DEBUGWIN_GROUP  = 2,
  KEYSYSTEM_RECV_DEBUGWIN_GROUP  = 3,
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
/// デバッグウィンドウ関数
//=====================================
static void DebugWin_U_Key_Addr( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Addr( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

static void DebugWin_U_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

static void DebugWin_U_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

static void DebugWin_U_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_U_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DebugWin_D_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
//-------------------------------------
/// その他
//=====================================
static void Util_LoadBg( HEAPID heapID );
static void Util_KeySystem_SetDebugWin( DEBUG_OUTSIDE_WORK *p_wk );

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
    DEBUGWIN_AddGroupToTop( OUTSIDE_DEBUGWIN_GROUP , "かんりがいデータ", HEAPID_NAGI_DEBUG_SUB );
    Util_KeySystem_SetDebugWin( p_wk );
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
  DEBUGWIN_RemoveGroup( OUTSIDE_DEBUGWIN_GROUP );
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
//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムのデバッグウィンドウ
 *
 *	@param	DEBUG_OUTSIDE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_KeySystem_SetDebugWin( DEBUG_OUTSIDE_WORK *p_wk )
{
  OUTSIDE_PERMANENT *p_permanent = OutsideSave_GetPermanentPtr( p_wk->p_outside_sv );
  KEY_SYSTEM_SAVE *p_key_system = OUTSIDE_PERMANENT_GetKeySystem( p_permanent );

  DEBUGWIN_AddGroupToGroup( KEYSYSTEM_DEBUGWIN_GROUP , "キーシステム", OUTSIDE_DEBUGWIN_GROUP, HEAPID_NAGI_DEBUG_SUB );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Addr, 
                              DebugWin_D_Key_Addr, 
                              p_permanent, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Difficult, 
                              DebugWin_D_Key_Difficult, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Wfbc, 
                              DebugWin_D_Key_Wfbc, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Init, 
                              DebugWin_D_Key_Init, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddGroupToGroup( KEYSYSTEM_SELF_DEBUGWIN_GROUP, "しょじキー", KEYSYSTEM_DEBUGWIN_GROUP , HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_00, 
                              DebugWin_D_Key_Have_00, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_01, 
                              DebugWin_D_Key_Have_01, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_02, 
                              DebugWin_D_Key_Have_02, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );

  DEBUGWIN_AddGroupToGroup( KEYSYSTEM_RECV_DEBUGWIN_GROUP, "こうもくかいほう", KEYSYSTEM_DEBUGWIN_GROUP , HEAPID_NAGI_DEBUG_SUB );


  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_00, 
                              DebugWin_D_Key_Use_00, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_01, 
                              DebugWin_D_Key_Use_01, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_02, 
                              DebugWin_D_Key_Use_02, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              HEAPID_NAGI_DEBUG_SUB );
}
//=============================================================================
/**
 *          デバッグウィンドウ関数
 */
//=============================================================================
static void DebugWin_U_Key_Addr( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
}
static void DebugWin_D_Key_Addr( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  OUTSIDE_PERMANENT *p_permanent = p_wk_adrs;
  KEY_SYSTEM_SAVE *p_key_system = OUTSIDE_PERMANENT_GetKeySystem( p_permanent );
  DEBUGWIN_ITEM_SetNameV( p_item , "アドレス=0x%x", OUTSIDE_MM_PERMANENT + (u32)p_permanent - (u32)p_key_system );
}
static void DebugWin_U_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
 DEBUGWIN_ITEM_SetNameV( p_item , "かんたんキー=%d", is_exits );
}
static void DebugWin_U_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
  DEBUGWIN_ITEM_SetNameV( p_item , "むずいキー=%d", is_exits );
}
static void DebugWin_U_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
  DEBUGWIN_ITEM_SetNameV( p_item , "まちキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
  DEBUGWIN_ITEM_SetNameV( p_item , "かんたんキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
  DEBUGWIN_ITEM_SetNameV( p_item , "むずいキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC);
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
  DEBUGWIN_ITEM_SetNameV( p_item , "まちキー=%d", is_exits );
}
static void DebugWin_U_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_DIFFICULT dif = KEY_SYSTEM_SAVE_GetDifficult( p_key_system );
    dif++;
    dif %= KEY_SYSTEM_DIFFICULT_MAX;
    KEY_SYSTEM_SAVE_SetDifficult( p_key_system, dif );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char *sc_difficult[] =
  {
    "かんたん",
    "つうじょう",
    "むずかしい",
  };

  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  KEY_SYSTEM_DIFFICULT dif = KEY_SYSTEM_SAVE_GetDifficult( p_key_system );

  DEBUGWIN_ITEM_SetNameV( p_item , "なんいど=%s", sc_difficult[dif] );
}
static void DebugWin_U_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_SAVE_Init( p_key_system );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DEBUGWIN_ITEM_SetName( p_item , "しょきか" );
}
static void DebugWin_U_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_key_system );
    wfbc++;
    wfbc %= KEY_SYSTEM_WFBC_MAX;
    KEY_SYSTEM_SAVE_SetWfbc( p_key_system, wfbc );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char *sc_wfbc[] =
  {
    "つうじょうのまち",
    "はんたいのまち",
  };

  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_key_system );

  DEBUGWIN_ITEM_SetNameV( p_item , "まち=%s", sc_wfbc[wfbc] );
}
#endif
