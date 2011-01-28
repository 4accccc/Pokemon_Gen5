//======================================================================
/**
 * @file	local_tvt_sys.c
 * @brief	非通信テレビトランシーバー メイン
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "test/ariizumi/ari_debug.h"
#include "local_tvt_util.h"
#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"
#include "local_tvt_scriptmode.h"
#include "local_tvt_callmode.h"
#include "local_tvt_graphic.h"
#include "local_tvt_message.h"
#include "local_tvt_debug.h"

#include "app/local_tvt_sys.h"

//======================================================================
//	define
//======================================================================
#ifdef PM_DEBUG
#define LTVT_DEBUG_USE_DEBUGWIN //デバッグウィンドウ使用
//#define LTVT_DEBUG_MODE_CHANGE  //ボタン長押しでモード切替
#endif //PM_DEBUG

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _LOCAL_TVT_WORK
{
  HEAPID heapId;
  LOCAL_TVT_SCRIPT        *script_wk;
  LOCAL_TVT_CALL          *call_wk;
  LOCAL_TVT_INIT_WORK     *initWork;
  LOCAL_TVT_GRAPHIC_WORK  *graphic;
  LOCAL_TVT_CHARA_SYS     *chara_sys;
  LOCAL_TVT_MESSAGE       *message;
  LOCAL_TVT_BG            *bg;

  void (*InitMode)( struct _LOCAL_TVT_WORK * );
  BOOL (*MainMode)( struct _LOCAL_TVT_WORK * );
  void (*ExitMode)( struct _LOCAL_TVT_WORK * );
}LOCAL_TVT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [>proc func
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

#pragma mark [> proto
static void Local_Tvt_SetMode( LOCAL_TVT_WORK *work, LOCAL_TVT_MODE mode );

static void Local_Tvt_ScriptMode_Init( LOCAL_TVT_WORK *work );
static BOOL Local_Tvt_ScriptMode_Main( LOCAL_TVT_WORK *work );
static void Local_Tvt_ScriptMode_Exit( LOCAL_TVT_WORK *work );

static void Local_Tvt_CallMode_InitSend( LOCAL_TVT_WORK *work );
static void Local_Tvt_CallMode_InitReceive( LOCAL_TVT_WORK *work );
static BOOL Local_Tvt_CallMode_Main( LOCAL_TVT_WORK *work );
static void Local_Tvt_CallMode_Exit( LOCAL_TVT_WORK *work );


//======================================================================
//	data
//======================================================================
const GFL_PROC_DATA LocalTvt_ProcData =
{
  LOCAL_TVT_ProcInit,
  LOCAL_TVT_ProcMain,
  LOCAL_TVT_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_INIT_WORK *initWork;
  LOCAL_TVT_WORK *work;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LOCAL_TVT, 0x40000 );
  work = GFL_PROC_AllocWork( proc, sizeof(LOCAL_TVT_WORK), HEAPID_LOCAL_TVT );
  if( pwk == NULL )
  {
    initWork = GFL_HEAP_AllocMemory( HEAPID_LOCAL_TVT , sizeof( LOCAL_TVT_INIT_WORK ));
    initWork->gameData = GAMEDATA_Create( GFL_HEAPID_APP );
    
    initWork->scriptId = 1;
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
    {
      initWork->scriptId = 5;
    }
  }
  else
  {
    initWork = pwk;
  }

  work->heapId = HEAPID_LOCAL_TVT;
  work->initWork = initWork;

#ifdef LTVT_DEBUG_MODE_CHANGE
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    work->initWork->mode     = LOCAL_TVT_MODE_SCRIPT;
    work->initWork->scriptId = 1;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    work->initWork->mode = LOCAL_TVT_MODE_APP;
  }
#endif

  //モード設定
  Local_Tvt_SetMode( work, work->initWork->mode );

  //グラフィック設定
  work->graphic = LOCAL_TVT_GRAPHIC_Init(GX_DISP_SELECT_MAIN_SUB, work->heapId );
  
  //メッセージ管理
  work->message = LOCAL_TVT_MESSAGE_Init( work->heapId );
  LOCAL_TVT_MESSAGE_RegisterPlayerName( work->message, work->initWork->gameData );

  //キャラ画像管理
  {
    LOCAL_TVT_CHARA_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_CHARA_SYS_PARAM) );

    param.cellUnit  = LOCAL_TVT_GRAPHIC_GetClunit( work->graphic );
    param.mystatus  = GAMEDATA_GetMyStatus( initWork->gameData );
    param.message   = work->message;
    param.timeZone  = LOCAL_TVT_UTIL_GetTimeZone( initWork->gameData );
    work->chara_sys = LOCAL_TVT_CHARA_SYS_Init( &param, work->heapId );
    /* ここは初期化のみで実際の人物設定はそれぞれのモード内から行う */
  }

  //共通BG読込み
  work->bg  = LOCAL_TVT_BG_Load( initWork->gameData, work->heapId );

  //モード初期化
  work->InitMode( work );


  GFL_NET_ReloadIconTopOrBottom(FALSE , work->heapId );
  

  //デバッグ
#ifdef LTVT_DEBUG_USE_DEBUGWIN
  {
    LOCAL_TVT_DEBUG_PARAM param;
    GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_DEBUG_PARAM) );
    param.p_font      = LOCAL_TVT_MESSAGE_GetFont( work->message );
    param.p_chara_sys = work->chara_sys;
    param.p_gamedata  = initWork->gameData;
    LOCAL_TVT_DEBUG_Init( &param, work->heapId );
  }
#endif

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;

  //デバッグ
#ifdef LTVT_DEBUG_USE_DEBUGWIN
  LOCAL_TVT_DEBUG_Exit();
#endif
  
  //モード破棄
  work->ExitMode( work );

  LOCAL_TVT_BG_UnLoad( work->bg );

  LOCAL_TVT_CHARA_SYS_Term( work->chara_sys );
  LOCAL_TVT_MESSAGE_Exit( work->message );
  LOCAL_TVT_GRAPHIC_Exit( work->graphic );

  if( pwk == NULL )
  {
    GAMEDATA_Delete( work->initWork->gameData );
    GFL_HEAP_FreeMemory( work->initWork );
  }
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_LOCAL_TVT );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  ループ
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  const BOOL ret = work->MainMode( work );

  LOCAL_TVT_MESSAGE_Main( work->message );
  LOCAL_TVT_CHARA_SYS_Main( work->chara_sys );
  LOCAL_TVT_GRAPHIC_2D_Draw( work->graphic );

#ifdef LTVT_DEBUG_USE_DEBUGWIN
  LOCAL_TVT_DEBUG_Main();
#endif
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *  private
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  モード設定
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 *	@param  mode                  モード
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_SetMode( LOCAL_TVT_WORK *work, LOCAL_TVT_MODE mode )
{
  switch( mode )
  {
  case LOCAL_TVT_MODE_SCRIPT:  //スクリプト制御モード
    work->InitMode  = Local_Tvt_ScriptMode_Init;
    work->MainMode  = Local_Tvt_ScriptMode_Main;
    work->ExitMode  = Local_Tvt_ScriptMode_Exit;
    break;

  case LOCAL_TVT_MODE_APP:     //アプリケーションモード 電話をかける
    work->InitMode  = Local_Tvt_CallMode_InitSend;
    work->MainMode  = Local_Tvt_CallMode_Main;
    work->ExitMode  = Local_Tvt_CallMode_Exit;
    break;

  case LOCAL_TVT_MODE_APP_SECRET_CALL:  //アプリケーションモード、電話がかかってくる
    work->InitMode  = Local_Tvt_CallMode_InitReceive;
    work->MainMode  = Local_Tvt_CallMode_Main;
    work->ExitMode  = Local_Tvt_CallMode_Exit;
    break;

  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトモード  初期化
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_ScriptMode_Init( LOCAL_TVT_WORK *work )
{
  work->script_wk = LOCAL_TVT_SCRIPT_Init( work->heapId, work->initWork->gameData, work->initWork->scriptId, work->chara_sys );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトモード  メイン処理
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static BOOL Local_Tvt_ScriptMode_Main( LOCAL_TVT_WORK *work )
{
  return LOCAL_TVT_SCRIPT_Main( work->script_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトモード  破棄
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_ScriptMode_Exit( LOCAL_TVT_WORK *work )
{
  LOCAL_TVT_SCRIPT_Term( work->script_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード電話をかける  初期化
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_CallMode_InitSend( LOCAL_TVT_WORK *work )
{
  LOCAL_TVT_CALL_PARAM  param;
  GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_CALL_PARAM) );
  param.p_gamedata  = work->initWork->gameData;
  param.p_chara_sys = work->chara_sys;
  param.p_message   = work->message;
  param.p_clunit    = LOCAL_TVT_GRAPHIC_GetClunit( work->graphic );
  param.mode        = LOCAL_TVT_CALL_MODE_SEND;

  work->call_wk = LOCAL_TVT_CALL_Init( &param, work->heapId );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード電話をうける  初期化
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_CallMode_InitReceive( LOCAL_TVT_WORK *work )
{
  LOCAL_TVT_CALL_PARAM  param;
  GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_CALL_PARAM) );
  param.p_gamedata  = work->initWork->gameData;
  param.p_chara_sys = work->chara_sys;
  param.p_message   = work->message;
  param.p_clunit    = LOCAL_TVT_GRAPHIC_GetClunit( work->graphic );
  param.mode        = LOCAL_TVT_CALL_MODE_RECEIVE;

  work->call_wk = LOCAL_TVT_CALL_Init( &param, work->heapId );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード  メイン処理
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static BOOL Local_Tvt_CallMode_Main( LOCAL_TVT_WORK *work )
{
  return LOCAL_TVT_CALL_Main( work->call_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード  破棄
 *
 *	@param	LOCAL_TVT_WORK *work ワーク
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_CallMode_Exit( LOCAL_TVT_WORK *work )
{
  LOCAL_TVT_CALL_Exit( work->call_wk );
}
