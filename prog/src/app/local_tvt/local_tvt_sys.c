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
#include "local_tvt_script.h"
#include "local_tvt_graphic.h"
#include "local_tvt_message.h"

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
typedef struct
{
  u8 chara[LOCAL_TVT_MEMBER_MAX];
  u8 back[LOCAL_TVT_MEMBER_MAX];
}LOCAL_TVT_SCRIPT_HEADER;

struct _LOCAL_TVT_WORK
{
  HEAPID heapId;
  LOCAL_TVT_SCRIPT        *script_wk;
  LOCAL_TVT_INIT_WORK     *initWork;
  LOCAL_TVT_GRAPHIC_WORK  *graphic;
  LOCAL_TVT_CHARA_SYS     *chara_sys;
  LOCAL_TVT_MESSAGE       *message;
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
#pragma mark [>proc func
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA LocalTvt_ProcData =
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

  //グラフィック設定
  work->graphic = LOCAL_TVT_GRAPHIC_Init(GX_DISP_SELECT_MAIN_SUB, work->heapId );
  
  //メッセージ管理
  work->message = LOCAL_TVT_MESSAGE_Init( work->heapId );
  LOCAL_TVT_MESSAGE_RegisterPlayerName( work->message, initWork->gameData );

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

  //スクリプトモード
  {
    GFL_CLUNIT  *clunit = LOCAL_TVT_GRAPHIC_GetClunit( work->graphic );
    work->script_wk = LOCAL_TVT_SCRIPT_Init( work->heapId, initWork->gameData, initWork->scriptId, work->chara_sys );
  }


  GFL_NET_ReloadIconTopOrBottom(FALSE , work->heapId );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  
  LOCAL_TVT_SCRIPT_Term( work->script_wk );

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
  const BOOL ret = LOCAL_TVT_SCRIPT_Main( work->script_wk );
  

  LOCAL_TVT_MESSAGE_Main( work->message );
  LOCAL_TVT_CHARA_SYS_Main( work->chara_sys );
  LOCAL_TVT_GRAPHIC_2D_Draw( work->graphic );
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}
