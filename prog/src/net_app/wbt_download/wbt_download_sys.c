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
#include "sound/pm_sndsys.h"

//ネット
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "system/net_err.h"

#include "wbt_download_net.h"
#include "app/t_download.h"
#include "savedata/wbt_download.h"

//外部参照
#include "net_app/wbt_download_sys.h"


#ifdef PM_DEBUG
#define DEBUG_WBT_DOWNLOAD_WIFI_THROUGH
#define DEBUG_WBT_DOWNLOAD_CREATE_DATA
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	動作状態
//=====================================
typedef enum
{
  WBT_DOWNLOAD_SYS_STATE_LIST,
  WBT_DOWNLOAD_SYS_STATE_FINISH,
} WBT_DOWNLOAD_SYS_STATE;

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
  SUBPROC_WORK            subproc;    //サブプロセス管理システム
  WBT_DOWNLOAD_SYS_PARAM  *p_param;   //プロセス引数
  void                    *p_buffer;  //バッファ
  WBT_DOWNLOAD_SYS_STATE  state;
} WBT_DOWNLOAD_SYS_WORK;

SDK_COMPILER_ASSERT( sizeof(WBTDL_TOTAL_DATA) < WBT_DOWNLOAD_MAX_SIZE );

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
//WbtDownload
static void *WbtDlNet_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
static void WbtDlNet_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//TDownload
static void *TDownload_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID );
static void TDownload_FreeParam( void *p_param_adrs, void *p_wk_adrs );

#ifdef PM_DEBUG
static void Debug_CreateWbtDlData( WBTDL_TOTAL_DATA *p_data );
#endif

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
  SUBPROCID_WBT_DLNET,
  SUBPROCID_WBT_LIST,

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
  //SUBPROCID_WBT_DLNET
  {
    GFL_OVERLAY_BLANK_ID,
    &WBT_DOWNLOAD_NET_ProcData,
    WbtDlNet_AllocParam,
    WbtDlNet_FreeParam,
  },
  //SUBPROCID_WBT_LIST
  {
    FS_OVERLAY_ID(entry_checker),
    &TDownloadProcData,
    TDownload_AllocParam,
    TDownload_FreeParam,
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

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WBT_DOWNLOAD_SYS, 0x20000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(WBT_DOWNLOAD_SYS_WORK), HEAPID_WBT_DOWNLOAD_SYS );
  GFL_STD_MemClear( p_wk, sizeof(WBT_DOWNLOAD_SYS_WORK) );

  //バッファ作成
  p_wk->p_buffer  = GFL_HEAP_AllocClearMemory( HEAPID_WBT_DOWNLOAD_SYS, WBT_DOWNLOAD_MAX_SIZE );

  //引数登録
  p_wk->p_param         = p_param_adrs;

   //モジュール作成
  SUBPROC_Init( &p_wk->subproc, sc_subproc_data, p_wk, HEAPID_WBT_DOWNLOAD_SYS );

  //退避
  PMSND_PushBGM();

#ifdef DEBUG_WBT_DOWNLOAD_WIFI_THROUGH
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WBT_LIST );
  }
  else
  {
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGIN );
  }
#else

  SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGIN );

#endif

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

  //バッファクリア
  GFL_HEAP_FreeMemory( p_wk->p_buffer );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_WBT_DOWNLOAD_SYS );

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

  WIFILOGIN_PARAM *p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WIFILOGIN_PARAM) );
  p_param->gamedata     = p_wk->p_param->p_gamedata;
  p_param->bg           = WIFILOGIN_BG_NORMAL;
  p_param->display      = WIFILOGIN_DISPLAY_UP;
  p_param->pSvl         = NULL;
  p_param->bgm          = WIFILOGIN_BGM_NONE;
  p_param->nsid         = WB_NET_WBT_DOWNLOAD;

  //もしアプリから着ていたらエラー後
  if( pre_procID == SUBPROCID_WBT_DLNET )
  {
    p_param->mode       = WIFILOGIN_MODE_ERROR;
  }
  else
  {
    PMSND_PlayBGM( SEQ_BGM_WIFI_ACCESS );
    PMSND_FadeInBGM( PMSND_FADE_FAST );
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
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WBT_DLNET );
    break;

  case WIFILOGIN_RESULT_CANCEL:
    PMSND_PopBGM();
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

  WIFILOGOUT_PARAM *p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WIFILOGOUT_PARAM) );
  p_param->gamedata = p_wk->p_param->p_gamedata;
  p_param->bg       = WIFILOGIN_BG_NORMAL;
  p_param->display  = WIFILOGIN_DISPLAY_UP;
  p_param->bgm      = WIFILOGIN_BGM_NONE;
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

  switch( p_wk->state )
  {
  case WBT_DOWNLOAD_SYS_STATE_LIST:
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WBT_LIST );
    break;

  default:
    GF_ASSERT(0);
  case WBT_DOWNLOAD_SYS_STATE_FINISH:
    /* 何も呼ばなければPROC終了 */
    break;
  }

  PMSND_FadeOutBGM( PMSND_FADE_FAST );
  PMSND_PopBGM();

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
static void *WbtDlNet_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;

  WBT_DOWNLOAD_NET_PARAM *p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBT_DOWNLOAD_NET_PARAM) );

  p_param->p_gamedata = p_wk->p_param->p_gamedata;
  p_param->p_buffer   = p_wk->p_buffer;;

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
static void WbtDlNet_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;
  WBT_DOWNLOAD_NET_PARAM         *p_param = p_param_adrs; 

  switch( p_param->result )
  {
  default:
    GF_ASSERT(0);
  case WBT_DOWNLOAD_NET_RESULT_SUCCESS:
    //  ログアウトしたあとにLISTプロセスへ
    p_wk->state = WBT_DOWNLOAD_SYS_STATE_LIST;
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGOUT );
    break;

  case WBT_DOWNLOAD_NET_RESULT_DATA_ILLIGAL:
    //  ログアウトして終了
    p_wk->state = WBT_DOWNLOAD_SYS_STATE_FINISH;
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGOUT );
    break;

  case WBT_DOWNLOAD_NET_RESULT_ERROR:
    //  エラーなのでWiFiログイン画面へ戻る
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_WIFI_LOGIN );
    break;
  }

  GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *  @brief  WBTリストの引数  作成
 *
 *  @param  HEAPID heapID     ヒープID
 *  @param  *p_wk_adrs        ワーク
 *  @param  pre_procID
 *
 *  @return 引数
 */
//-----------------------------------------------------------------------------
static void *TDownload_AllocParam( HEAPID heapID, void *p_wk_adrs, u32 pre_procID )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;

  T_DOWNLOAD_PARAM *p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(T_DOWNLOAD_PARAM) );
  p_param->gamesys  = p_wk->p_param->p_gamesys;
  p_param->data     = p_wk->p_buffer;
  p_param->status   = T_DOWNLOAD_STATUS_SELECT;

  //TODO
#ifdef DEBUG_WBT_DOWNLOAD_CREATE_DATA
  Debug_CreateWbtDlData( p_wk->p_buffer );
#endif

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  WBTリストの引数  破棄
 *
 *  @param  void *p_param_adrs        引数
 *  @param  *p_wk_adrs                ワーク
 */
//-----------------------------------------------------------------------------
static void TDownload_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{
  WBT_DOWNLOAD_SYS_WORK   *p_wk = p_wk_adrs;
  T_DOWNLOAD_PARAM         *p_param = p_param_adrs; 

  /* 何も呼ばなければPROC終了 */

  GFL_HEAP_FreeMemory( p_param );
}
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグデータ作成
 *
 *	@param	WBTDL_TOTAL_DATA *p_data データ
 */
//-----------------------------------------------------------------------------
static void Debug_CreateWbtDlData( WBTDL_TOTAL_DATA *p_total_data )
{
  GFL_STD_MemClear( p_total_data, sizeof(WBTDL_TOTAL_DATA) );


  {
    int i;
    int j;
    int k;
    for( i = 0; i < WBTDL_TOTAL_NUM; i++ )
    {
      WBTDL_DATA *p_data  = &p_total_data->data[i];
      WBTDL_RULE *p_rule  = &p_data->regulation;
      p_data->cassette_version = PM_VERSION;
      p_data->year    = 11;
      p_data->month   = 8;
      p_data->day     = 3;
      p_data->lang_id = PM_LANG;
      p_data->tournament_id = i;
      GFL_STD_MemFill16( p_data->tournament_name, L'０', WBTDL_TOURNAMENT_NAME_LEN*sizeof(STRCODE) );
      p_data->tournament_name[0] = L'て';
      p_data->tournament_name[1] = L'す';
      p_data->tournament_name[2] = L'と';
      p_data->tournament_name[3] = L'　';
      p_data->tournament_name[4] = L'た';
      p_data->tournament_name[5] = L'い';
      p_data->tournament_name[6] = L'か';
      p_data->tournament_name[7] = L'い';
      p_data->tournament_name[WBTDL_TOURNAMENT_NAME_LEN-1] = GFL_STR_GetEOMCode();
      GFL_STD_MemFill16( p_data->contents_explain, L'０', WBTDL_TOURNAMENT_EXPLAIN_LEN*sizeof(STRCODE) );
      p_data->contents_explain[0] = L'な';
      p_data->contents_explain[1] = L'い';
      p_data->contents_explain[2] = L'よ';
      p_data->contents_explain[3] = L'う';
      p_data->contents_explain[WBTDL_TOURNAMENT_EXPLAIN_LEN-1] = GFL_STR_GetEOMCode();
      GFL_STD_MemFill16( p_data->regulation_explain, L'０', WBTDL_TOURNAMENT_REG_EXPLAIN_LEN*sizeof(STRCODE) );
      p_data->regulation_explain[0] = L'レ';
      p_data->regulation_explain[1] = L'ギ';
      p_data->regulation_explain[2] = L'ュ';
      p_data->regulation_explain[3] = L'レ';
      p_data->regulation_explain[4] = L'ー';
      p_data->regulation_explain[5] = L'シ';
      p_data->regulation_explain[6] = L'ョ';
      p_data->regulation_explain[7] = L'ン';
      p_data->regulation_explain[WBTDL_TOURNAMENT_REG_EXPLAIN_LEN-1] = GFL_STR_GetEOMCode();

      p_rule->num_lo  = 3;
      p_rule->num_hi  = 3;
      p_rule->level   = 50;
      p_rule->level_range   = 3;
      p_rule->level_total = 0;
      p_rule->both_pokemon  = FALSE;
      p_rule->both_item     = FALSE;
      p_rule->nick_name     = FALSE;
      p_rule->btl_rule      = BTL_RULE_SINGLE;

      for( j = 0; j < WBTDL_CHAR_MAX; j++ )
      {
        WBTDL_CHARACTER *p_chara = &p_data->character[j];
        for( k = 0; k < TEMOTI_POKEMAX; k++ )
        {
          WBTDL_POKEMON *p_poke = &p_chara->pokemon[k];
          p_poke->monsno  = k+1;
          GFL_STD_MemFill16( p_poke->nickname, L'０', (MONS_NAME_SIZE+EOM_SIZE)*sizeof(STRCODE) );
          p_poke->nickname[0]  = L'ニ';
          p_poke->nickname[1]  = L'ッ';
          p_poke->nickname[2]  = L'ク';
          p_poke->nickname[3]  = L'ネ';
          p_poke->nickname[4]  = L'ー';
          p_poke->nickname[5]  = L'ム';
          p_poke->nickname[MONS_NAME_SIZE]  = GFL_STR_GetEOMCode();

          p_poke->waza_id[0] = 1;
          p_poke->level     = 50;
        }
        GFL_STD_MemFill16( p_chara->name, L'０', SAVELEN_TRAINER_NAME*sizeof(STRCODE) );
        p_chara->name[0]  = L'な';
        p_chara->name[1]  = L'ま';
        p_chara->name[2]  = L'え';
        p_chara->name[3]  = L'０'+j;
        p_chara->name[SAVELEN_TRAINER_NAME-1]  = GFL_STR_GetEOMCode();
        GFL_STD_MemFill16( p_chara->title, L'０', WBTDL_TRAINER_TITLE_LEN*sizeof(STRCODE) );
        p_chara->title[0] = L'タ';
        p_chara->title[1] = L'イ';
        p_chara->title[2] = L'ト';
        p_chara->title[3] = L'ル';
        p_chara->title[WBTDL_TRAINER_TITLE_LEN-1] = GFL_STR_GetEOMCode();
        GFL_STD_MemFill16( p_chara->msg_before, L'０', WBTDL_MSG_LEN*sizeof(STRCODE) );
        p_chara->msg_before[0]  = L'た';
        p_chara->msg_before[1]  = L'い';
        p_chara->msg_before[2]  = L'せ';
        p_chara->msg_before[3]  = L'ん';
        p_chara->msg_before[4]  = L'ま';
        p_chara->msg_before[5]  = L'え';
        p_chara->msg_before[WBTDL_MSG_LEN-1]  = GFL_STR_GetEOMCode();
        GFL_STD_MemFill16( p_chara->msg_win, L'０', WBTDL_MSG_LEN*sizeof(STRCODE) );
        p_chara->msg_win[0]  = L'か';
        p_chara->msg_win[1]  = L'ち';
        p_chara->msg_win[2]  = L'せ';
        p_chara->msg_win[3]  = L'り';
        p_chara->msg_win[4]  = L'ふ';
        p_chara->msg_win[WBTDL_MSG_LEN-1]  = GFL_STR_GetEOMCode();
        GFL_STD_MemFill16( p_chara->msg_lose, L'０', WBTDL_MSG_LEN*sizeof(STRCODE) );
        p_chara->msg_lose[0]  = L'ま';
        p_chara->msg_lose[1]  = L'け';
        p_chara->msg_lose[2]  = L'せ';
        p_chara->msg_lose[3]  = L'り';
        p_chara->msg_lose[4]  = L'ふ';
        p_chara->msg_lose[WBTDL_MSG_LEN-1]  = GFL_STR_GetEOMCode();

        GFL_STD_MemFill16( p_chara->msg_announce, L'０', WBTDL_MSG_LEN*sizeof(STRCODE) );
        p_chara->msg_announce[0]  = L'け';
        p_chara->msg_announce[1]  = L'っ';
        p_chara->msg_announce[2]  = L'し';
        p_chara->msg_announce[3]  = L'ょ';
        p_chara->msg_announce[4]  = L'う';
        p_chara->msg_announce[5]  = L'ま';
        p_chara->msg_announce[6]  = L'え';
        p_chara->msg_announce[WBTDL_MSG_LEN-1]  = GFL_STR_GetEOMCode();

        p_chara->title_id           = 1;
        p_chara->msg_before_id      = 1;
        p_chara->msg_after_win_id   = 2;
        p_chara->msg_after_lose_id  = 3;
        p_chara->msg_win_id         = 4;
        p_chara->msg_lose_id        = 5;
        p_chara->msg_announce_id    = 6;
        p_chara->obj_id             = 0xe + j;
        p_chara->lang_id            = PM_LANG;
      }

      for( j = 0; j < WBT_TRAINER_NUM - 1; j++ )
      {
        WBTDL_MATCH *p_match = &p_data->match[j];
        p_match->type = WBTDL_MATCH_TRAINER_DATA;
        p_match->pri = j;
        p_match->id   = j;
      }

      p_data->crc = GFL_STD_CrcCalc( p_data, sizeof(WBTDL_DATA)-sizeof(p_data->crc) );
    }
    p_total_data->crc = GFL_STD_CrcCalc( p_total_data, sizeof(WBTDL_TOTAL_DATA)-sizeof(p_total_data->crc) );
  }
}
#endif
