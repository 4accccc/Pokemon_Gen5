//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_download_sys.c
 *	@brief  WBTダウンロード画面のための通信接続管理プロセス
 *	@author	Toru=Nagihashi
 *	@data		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"

//ネット
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "system/net_err.h"

//外部参照
#include "wbt_download_sys.h"

//オーバーレイ
FS_EXTERN_OVERLAY(common_download);
FS_EXTERN_OVERLAY(dpw_common);


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//TODO HAEPIDは仮
#define HEAPID_WBT_DOWNLOAD_SYS HEAPID_NAGI_DEBUG_SUB
#define WBT_DOWNLOAD_SYS_HEAPSIZE (0x10000)
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
/// サブプロック移動管理
//=====================================
typedef struct
{
  FSOverlayID             ov_id;
  const GFL_PROC_DATA     *cp_procdata;
  void *(*alloc_func)( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
  void (*free_func)( void *p_param, void *p_wk_adrs );
} SUBPROC_DATA;
typedef struct {
  GFL_PROCSYS     *p_procsys;
  u32             seq;
  void            *p_proc_param;
  SUBPROC_DATA    *p_data;

  HEAPID          heapID;
  void            *p_wk_adrs;
  const SUBPROC_DATA      *cp_procdata_tbl;

  u32             pre_procID;
  u32             next_procID;
  u32             now_procID;
} SUBPROC_WORK;


//-------------------------------------
///	WBT_DOWNLOAD_SYSのメインワーク
//=====================================
typedef struct
{
  SUBPROC_WORK  subproc;            //サブプロセス管理システム

  WBT_DOWNLOAD_SYS_PARAM *p_param;  //プロセス引数

} WBT_DOWNLOAD_SYS_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// SYS プロセス
//=====================================
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
/// サブプロセス
//=====================================
static void SUBPROC_Init( SUBPROC_WORK *p_wk, const SUBPROC_DATA *cp_procdata_tbl, void *p_wk_adrs, HEAPID heapID );
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk );
static void SUBPROC_Exit( SUBPROC_WORK *p_wk );
static void SUBPROC_CallProc( SUBPROC_WORK *p_wk, u32 procID );

//-------------------------------------
/// サブプロセス用引数の解放、破棄関数
//=====================================
//Wi-FiLogin
static void *WiFiLogin_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
static void WiFiLogin_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//Wi-FiLogout
static void *WiFiLogout_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
static void WiFiLogout_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//App
static void *App_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
static void App_FreeParam( void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
const GFL_PROC_DATA WBT_DOWNLOAD_SYS_ProcData = 
{
  Wbt_Download_Sys_Proc_Init,
  Wbt_Download_Sys_Proc_Main,
  Wbt_Download_Sys_Proc_Exit,
};

//-------------------------------------
/// サブプロセス移動データ
//=====================================
typedef enum
{
  SUBPROCID_WIFI_LOGIN,
  SUBPROCID_WIFI_LOGOUT,
  SUBPROCID_APP,

  SUBPROCID_MAX
} SUBPROC_ID;
static const SUBPROC_DATA sc_subproc_data[SUBPROCID_MAX]  =
{
  //SUBPROCID_WIFI_LOGIN
  {
    FS_OVERLAY_ID(wifi_login),
    &WiFiLogin_ProcData,
    WiFiLogin_AllocParam,
    WiFiLogin_FreeParam,
  },
  //SUBPROCID_WIFI_LOGOUT
  {
    FS_OVERLAY_ID(wifi_login),
    &WiFiLogout_ProcData,
    WiFiLogout_AllocParam,
    WiFiLogout_FreeParam,
  },
  //SUBPROCID_APP
  {
    //TODO 仮にWiFiLoout画面を呼び出すようになっている
    FS_OVERLAY_ID(wifi_login),
    &WiFiLogout_ProcData,
    App_AllocParam,
    App_FreeParam,
  },
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK *p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(common_download) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WBT_DOWNLOAD_SYS, WBT_DOWNLOAD_SYS_HEAPSIZE );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(WBT_DOWNLOAD_SYS_WORK), HEAPID_WBT_DOWNLOAD_SYS );
  GFL_STD_MemClear( p_wk, sizeof(WBT_DOWNLOAD_SYS_WORK) );

  //引数登録
  p_wk->p_param         = p_param_adrs;

   //モジュール作成
  SUBPROC_Init( &p_wk->subproc, sc_subproc_data, p_wk, HEAPID_WBT_DOWNLOAD_SYS );
  SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGIN );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK *p_wk = p_wk_adrs;

  //モジュール破棄
  SUBPROC_Exit( &p_wk->subproc );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_WBT_DOWNLOAD_SYS );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(common_download) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Wbt_Download_Sys_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  WBT_DOWNLOAD_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    {
      if( SUBPROC_Main( &p_wk->subproc ) )
      {
        (*p_seq)++;
      }
    }
    break;

  case SEQ_EXIT:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *      SUBPROCシステム
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SUBPROCシステム 初期化
 *
 *  @param  SUBPROC_WORK *p_wk  ワーク
 *  @param  cp_procdata_tbl     プロセス接続テーブル
 *  @param  void *p_wk_adrs     アロック関数と解放関数に渡すワーク
 *  @param  heapID              システム構築用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_Init( SUBPROC_WORK *p_wk, const SUBPROC_DATA *cp_procdata_tbl, void *p_wk_adrs, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
  p_wk->p_procsys       = GFL_PROC_LOCAL_boot( heapID );
  p_wk->p_wk_adrs       = p_wk_adrs;
  p_wk->cp_procdata_tbl = cp_procdata_tbl;
  p_wk->heapID          = heapID;
}

//----------------------------------------------------------------------------
/**
 *  @brief  SUBPROCシステム メイン処理
 *
 *  @param  SUBPROC_WORK *p_wk  ワーク
 *
 *  @retval TRUEならば終了  FALSEならばPROCが存在する
 */
//-----------------------------------------------------------------------------
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk )
{
  enum
  {
    SEQ_INIT,
    SEQ_ALLOC_PARAM,
    SEQ_MAIN,
    SEQ_FREE_PARAM,
    SEQ_END,
  };

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    p_wk->pre_procID  = p_wk->now_procID;
    p_wk->now_procID  = p_wk->next_procID;
    p_wk->seq = SEQ_ALLOC_PARAM;
    break;

  case SEQ_ALLOC_PARAM:
    //プロセス引数作成関数があれば呼び出し
    if( p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func )
    {
      p_wk->p_proc_param  = p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func(
          p_wk->heapID, p_wk->p_wk_adrs, p_wk->pre_procID );
    }
    else
    {
      p_wk->p_proc_param  = NULL;
    }

    //プロック呼び出し
    GFL_PROC_LOCAL_CallProc( p_wk->p_procsys, p_wk->cp_procdata_tbl[  p_wk->now_procID ].ov_id,
          p_wk->cp_procdata_tbl[  p_wk->now_procID ].cp_procdata, p_wk->p_proc_param );

    p_wk->seq = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    {
      GFL_PROC_MAIN_STATUS result;
      result  = GFL_PROC_LOCAL_Main( p_wk->p_procsys );

      if( result == GFL_PROC_MAIN_NULL || result == GFL_PROC_MAIN_CHANGE )
      {
        NetErr_DispCall(FALSE);
      }

      if( GFL_PROC_MAIN_NULL == result )
      {
        p_wk->seq = SEQ_FREE_PARAM;
      }
    }
    break;

  case SEQ_FREE_PARAM:
    //プロセス引数破棄関数呼び出し
    if( p_wk->cp_procdata_tbl[  p_wk->now_procID ].free_func )
    {
      p_wk->cp_procdata_tbl[  p_wk->now_procID ].free_func( p_wk->p_proc_param, p_wk->p_wk_adrs );
      p_wk->p_proc_param  = NULL;
    }

    //もし次のプロセスがあれば呼び出し
    //なければ終了
    if( p_wk->now_procID  != p_wk->next_procID )
    {
      p_wk->seq = SEQ_INIT;
    }
    else
    {
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SUBPROCシステム 破棄
 *
 *  @param  SUBPROC_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_Exit( SUBPROC_WORK *p_wk )
{
  GF_ASSERT_HEAVY( p_wk->p_proc_param == NULL );

  GFL_PROC_LOCAL_Exit( p_wk->p_procsys );
  GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  SUBPROCシステム プロセスリクエスト
 *
 *  @param  SUBPROC_WORK *p_wk  ワーク
 *  @param  proc_id             呼ぶプロセスID
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_CallProc( SUBPROC_WORK *p_wk, u32 procID )
{
  p_wk->next_procID = procID;
}

//=============================================================================
/**
 *    サブプロセス用作成破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  WiFiログインの引数  作成
 *
 *  @param  HEAPID heapID     ヒープID
 *  @param  *p_wk_adrs        ワーク
 *  @param  pre_procID
 *
 *  @return 引数
 */
//-----------------------------------------------------------------------------
static void *WiFiLogin_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;

  WIFILOGIN_PARAM *p_param = GFL_HEAP_AllocMemory( heapID, sizeof(WIFILOGIN_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(WIFILOGIN_PARAM) );
  p_param->gamedata     = p_wk->p_param->p_gamedata;
  p_param->bg           = WIFILOGIN_BG_NORMAL;
  p_param->display      = WIFILOGIN_DISPLAY_UP;
  p_param->pSvl         = NULL;
  p_param->bgm          = WIFILOGIN_BGM_NORMAL;
  p_param->nsid         = WB_NET_WBT_DOWNLOAD;

  //もしアプリから着ていたらエラー後
  if( pre_procID == SUBPROCID_APP )
  {
    p_param->mode       = WIFILOGIN_MODE_ERROR;
  }
  else
  {
    p_param->mode       = WIFILOGIN_MODE_NORMAL;
  }

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  WiFiログインの引数  破棄
 *
 *  @param  void *p_param_adrs        引数
 *  @param  *p_wk_adrs                ワーク
 */
//-----------------------------------------------------------------------------
static void WiFiLogin_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;
  WIFILOGIN_PARAM         *p_param = p_param_adrs; 

  switch( p_param->result )
  {
  case WIFILOGIN_RESULT_LOGIN:
    //  次のプロセスへ
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_APP );
    break;

  case WIFILOGIN_RESULT_CANCEL:
    // 何も呼ばなければPROC終了
    break;
  }

  GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *  @brief  WiFiログアウトの引数  作成
 *
 *  @param  HEAPID heapID     ヒープID
 *  @param  *p_wk_adrs        ワーク
 *  @param  pre_procID
 *
 *  @return 引数
 */
//-----------------------------------------------------------------------------
static void *WiFiLogout_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;

  WIFILOGOUT_PARAM *p_param = GFL_HEAP_AllocMemory( heapID, sizeof(WIFILOGOUT_PARAM) );
  p_param->gamedata = p_wk->p_param->p_gamedata;
  p_param->bg       = WIFILOGIN_BG_NORMAL;
  p_param->display  = WIFILOGIN_DISPLAY_UP;
  p_param->bgm      = WIFILOGIN_BGM_NORMAL;
  p_param->fade     = WIFILOGIN_FADE_DEFAULT;

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  WiFiログアウトの引数  破棄
 *
 *  @param  void *p_param_adrs        引数
 *  @param  *p_wk_adrs                ワーク
 */
//-----------------------------------------------------------------------------
static void WiFiLogout_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;
  WIFILOGOUT_PARAM         *p_param = p_param_adrs; 

  /* 何も呼ばなければPROC終了 */

  GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *  @brief  アプリ本体の引数  作成
 *
 *  @param  HEAPID heapID     ヒープID
 *  @param  *p_wk_adrs        ワーク
 *  @param  pre_procID
 *
 *  @return 引数
 */
//-----------------------------------------------------------------------------
static void *App_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;

  WIFILOGOUT_PARAM *p_param = GFL_HEAP_AllocMemory( heapID, sizeof(WIFILOGOUT_PARAM) );
  p_param->gamedata = p_wk->p_param->p_gamedata;
  p_param->bg       = WIFILOGIN_BG_NORMAL;
  p_param->display  = WIFILOGIN_DISPLAY_UP;
  p_param->bgm      = WIFILOGIN_BGM_NORMAL;
  p_param->fade     = WIFILOGIN_FADE_DEFAULT;

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  アプリ本体の引数  破棄
 *
 *  @param  void *p_param_adrs        引数
 *  @param  *p_wk_adrs                ワーク
 */
//-----------------------------------------------------------------------------
static void App_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;
  WIFILOGOUT_PARAM         *p_param = p_param_adrs; 

  //TODO
  switch( 0 )
  {
  case 0:
    //  終了なので次のプロセスへ
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGOUT );
    break;

  case 1:
    //  エラーなのでWiFiログイン画面へ戻る
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGIN );
    break;
  }
  

  GFL_HEAP_FreeMemory( p_param );
}
