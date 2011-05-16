//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_common_net.c
 *	@brief  COMMON_DOWNLOAD デバッグ用
 *	@author	Toru=Nagihashi
 *	@data		2011.05.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "debug_util.h"

//ネット
#include "net/network_define.h"
#include "net/net_common_download.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "system/net_err.h"

//外部
#include "debug_common_net.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
FS_EXTERN_OVERLAY(common_download);
FS_EXTERN_OVERLAY(dpw_common);

//-------------------------------------
///	戻り先
//=====================================
typedef enum
{
  DEBUG_COMMON_NET_RETURN_NORMAL,
  DEBUG_COMMON_NET_RETURN_ERROR,
} DEBUG_COMMON_NET_RETURN;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	SYSワーク
//=====================================
typedef struct
{
  GAMEDATA          *p_gamedata;
  DEBUG_SEQ_WORK    *p_seq;
  void              *p_sub_proc_param;
  COMMON_DOWNLOAD_NET_TYPE  net_type;
  DEBUG_COMMON_NET_RETURN   ret;
} DEBUG_COMMON_NET_SYS_WORK;

//-------------------------------------
///	COREワーク
//=====================================
typedef struct
{
  DEBUG_COMMON_NET_SYS_WORK *p_param;
  DEBUG_SEQ_WORK    *p_seq;
  COMMON_DOWNLOAD   *p_common_download;
} DEBUG_COMMON_NET_CORE_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// SYS プロセス
//=====================================
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// CORE プロセス
//=====================================
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// SYS SEQFUNC
//=====================================
static void Debug_Common_Net_Sys_Seq_Wifi( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Debug_Common_Net_Sys_Seq_Beacon( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// CORE SEQFUNC
//=====================================
static void Debug_Common_Net_Core_Seq_Wifi( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Debug_Common_Net_Core_Seq_Beacon( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// ETC
//=====================================
static u32 Debug_Common_Net_GetDownloadIndexCallback( const DWCNdFileInfo *cp_fileInfo_array, u32 array_len );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	外部に出すSYSプロセスデータ
//=====================================
const GFL_PROC_DATA DEBUG_COMMON_NET_ProcData =
{
  Debug_Common_Net_SysProc_Init,
  Debug_Common_Net_SysProc_Main,
  Debug_Common_Net_SysProc_Exit,
};
//-------------------------------------
///	COREプロセスデータ
//=====================================
static const GFL_PROC_DATA Debug_Common_Net_CoreProcData =
{
  Debug_Common_Net_CoreProc_Init,
  Debug_Common_Net_CoreProc_Main,
  Debug_Common_Net_CoreProc_Exit,
};

//=============================================================================
/**
 *					SYS PROC
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
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_SYS_WORK *p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(common_download) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x40000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_COMMON_NET_SYS_WORK), HEAPID_NAGI_DEBUG_SUB );
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_COMMON_NET_SYS_WORK) );

  //ゲームデータ作成（デバッグのためゲームデータがない）
  p_wk->p_gamedata = GAMEDATA_Create( HEAPID_NAGI_DEBUG_SUB );

  //モードによってシーケンスと通信方式変更
  {
    DEBUG_SEQ_FUNCTION        init_seq;

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
    {
      /*  
       *  ビーコン受信モード
       *  送信側はタイトル→SELECT→おおのかつみ→ふしぎビーコン配信からできます
       *
       */
      p_wk->net_type = COMMON_DOWNLOAD_NET_TYPE_BEACON;
      init_seq = Debug_Common_Net_Sys_Seq_Beacon;
      OS_TPrintf( "共通ダウンロードデバッグ：ビーコンモード開始\n" );
    }
    else
    {
      p_wk->net_type = COMMON_DOWNLOAD_NET_TYPE_WIFI_ND;
      init_seq = Debug_Common_Net_Sys_Seq_Wifi;
      OS_TPrintf( "共通ダウンロードデバッグ：Wi-Fiダウンロードモード開始\n" );
    }

    //シーケンス
    p_wk->p_seq = DEBUG_SEQ_Init( p_wk, init_seq, HEAPID_NAGI_DEBUG_SUB );

  }
  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_SYS_WORK *p_wk = p_wk_adrs;

  //シーケンス
  DEBUG_SEQ_Exit( p_wk->p_seq );

  //ゲームデータ破棄
  GAMEDATA_Delete( p_wk->p_gamedata );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(common_download) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセスメイン
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Common_Net_SysProc_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_SYS_WORK *p_wk = p_wk_adrs;

  //シーケンス
  DEBUG_SEQ_Main( p_wk->p_seq );

  if( DEBUG_SEQ_IsEnd( p_wk->p_seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}
//=============================================================================
/**
 *					CORE PROC
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_CORE_WORK *p_wk;

  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_COMMON_NET_CORE_WORK), HEAPID_NAGI_DEBUG_SUB );
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_COMMON_NET_CORE_WORK) );

  p_wk->p_param  = p_param_adrs;

  //モードによってシーケンスと通信方式変更
  {
    DEBUG_SEQ_FUNCTION        init_seq;

    if( p_wk->p_param->net_type == COMMON_DOWNLOAD_NET_TYPE_BEACON )
    {
      /*  
       *  ビーコン受信モード
       *  送信側はタイトル→SELECT→おおのかつみ→ふしぎビーコン配信からできます
       */

      init_seq = Debug_Common_Net_Core_Seq_Beacon;
      OS_TPrintf( "共通ダウンロードデバッグ：ビーコンモード開始\n" );
    }
    else
    {
      init_seq = Debug_Common_Net_Core_Seq_Wifi;
      OS_TPrintf( "共通ダウンロードデバッグ：Wi-Fiダウンロードモード開始\n" );
    }

    //通信
    //Wi-Fiの場合、本来Wi-Fiログイン後に初期化する流れだが、
    //Init内ではメモリアロケートしかしていないので問題なし
    {
      COMMON_DOWNLOAD_INIT_PARAM  init_param;
      GFL_STD_MemClear( &init_param, sizeof(COMMON_DOWNLOAD_INIT_PARAM) );
      init_param.net_type     = p_wk->p_param->net_type;
      init_param.p_gamedata   = p_wk->p_param->p_gamedata;
      init_param.buffer_size  = 1024; //仮で1kb
      init_param.heapID       = HEAPID_NAGI_DEBUG_SUB;
      p_wk->p_common_download = COMMON_DOWNLOAD_Init( &init_param );
    }

    //シーケンス
    p_wk->p_seq = DEBUG_SEQ_Init( p_wk, init_seq, HEAPID_NAGI_DEBUG_SUB );

  }
  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_CORE_WORK *p_wk = p_wk_adrs;

  //シーケンス
  DEBUG_SEQ_Exit( p_wk->p_seq );

  //通信破棄
  COMMON_DOWNLOAD_Exit( p_wk->p_common_download );


  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コアプロセスメイン
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Debug_Common_Net_CoreProc_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_COMMON_NET_CORE_WORK *p_wk = p_wk_adrs;

  //エラー処理
  switch( COMMON_DOWNLOAD_HandleError(p_wk->p_common_download) )
  {
  case COMMON_DOWNLOAD_ERROR_TYPE_NONE:        //エラーが起こっていません。
    /*  なにもしない  */
    break;
  case COMMON_DOWNLOAD_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
    /*   エラー時のシーケンス遷移など　 */
    COMMON_DOWNLOAD_ClearError( p_wk->p_common_download );
    return GFL_PROC_RES_CONTINUE;
  case COMMON_DOWNLOAD_ERROR_TYPE_DISCONNECT:  //アプリ終了
    p_wk->p_param->ret = DEBUG_COMMON_NET_RETURN_ERROR;
    return GFL_PROC_RES_FINISH;
  default:
    GF_ASSERT(0);
  }

  //通信前処理
  COMMON_DOWNLOAD_MainTop( p_wk->p_common_download );

  //シーケンス
  DEBUG_SEQ_Main( p_wk->p_seq );

  //通信後処理
  COMMON_DOWNLOAD_MainTail( p_wk->p_common_download );


  if( DEBUG_SEQ_IsEnd( p_wk->p_seq ) )
  {
    p_wk->p_param->ret = DEBUG_COMMON_NET_RETURN_NORMAL;
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}

//=============================================================================
/**
 *					SYS SEQ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  Sys Wi-Fiダウンロードシーケンス
 *
 *	@param	DEBUG_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Debug_Common_Net_Sys_Seq_Wifi( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_WIFI_LOGIN,
    SEQ_RET_WIFI_LOGIN,

    SEQ_CALL_PROC,
    SEQ_RET_PROC,

    SEQ_CALL_WIFI_LOGOUT,
    SEQ_RET_WIFI_LOGOUT,
    SEQ_END,

  };

  DEBUG_COMMON_NET_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_CALL_WIFI_LOGIN:
    {
      WIFILOGIN_PARAM *p_param = GFL_HEAP_AllocMemory( HEAPID_NAGI_DEBUG_SUB, sizeof(WIFILOGIN_PARAM) );
      GFL_STD_MemClear( p_param, sizeof(WIFILOGIN_PARAM) );
      p_param->gamedata     = p_wk->p_gamedata;
      p_param->bg           = WIFILOGIN_BG_NORMAL;
      p_param->display      = WIFILOGIN_DISPLAY_UP;
      p_param->pSvl         = NULL;
      p_param->bgm          = WIFILOGIN_BGM_NORMAL;
      p_param->nsid         = WB_NET_WBT_DOWNLOAD;

      if( p_wk->ret == DEBUG_COMMON_NET_RETURN_ERROR )
      {
        p_param->mode       = WIFILOGIN_MODE_ERROR;
      }
      else
      {
        p_param->mode       = WIFILOGIN_MODE_NOTSAVE;
      }
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, p_param );

      p_wk->p_sub_proc_param = p_param;
    }
    (*p_seq)++;
    break;
  case SEQ_RET_WIFI_LOGIN:
    {
      WIFILOGIN_PARAM *p_param = p_wk->p_sub_proc_param;
      OS_TPrintf( "Wi-Fi接続完了\n" );
      if( p_param->result == WIFILOGIN_RESULT_CANCEL )
      {
        *p_seq = SEQ_END;
      }
      else
      {
        (*p_seq)++;
      }
      GFL_HEAP_FreeMemory( p_wk->p_sub_proc_param );
    }
    break;
  case SEQ_CALL_PROC:
    GFL_PROC_SysCallProc( GFL_OVERLAY_BLANK_ID, &Debug_Common_Net_CoreProcData, p_wk );
    (*p_seq)++;
    break;
  case SEQ_RET_PROC:
    NetErr_DispCall(FALSE);
    //アプリ内でエラーが起こったら、接続画面に戻る
    if( p_wk->ret == DEBUG_COMMON_NET_RETURN_ERROR )
    {
      (*p_seq) = SEQ_CALL_WIFI_LOGIN;
    }
    else
    {
      (*p_seq)++;
    }
    break;
  case SEQ_CALL_WIFI_LOGOUT:
    {
      WIFILOGOUT_PARAM *p_param = GFL_HEAP_AllocMemory( HEAPID_NAGI_DEBUG_SUB, sizeof(WIFILOGOUT_PARAM) );
      p_param->gamedata = p_wk->p_gamedata;
      p_param->bg       = WIFILOGIN_BG_NORMAL;
      p_param->display  = WIFILOGIN_DISPLAY_UP;
      p_param->bgm      = WIFILOGIN_BGM_NORMAL;
      p_param->fade     = WIFILOGIN_FADE_DEFAULT;
      GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogout_ProcData, p_wk->p_sub_proc_param );

      p_wk->p_sub_proc_param = p_param;
    }
    (*p_seq)++;
    break;
  case SEQ_RET_WIFI_LOGOUT:
    OS_TPrintf( "Wi-Fi切断完了\n" );
    GFL_HEAP_FreeMemory( p_wk->p_sub_proc_param );
    (*p_seq)++;
    break;
  case SEQ_END:
    DEBUG_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Sys ビーコンシーケンス
 *
 *	@param	DEBUG_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Debug_Common_Net_Sys_Seq_Beacon( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_RET_PROC,
    SEQ_END,
  };

  DEBUG_COMMON_NET_SYS_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_CALL_PROC:
    GFL_PROC_SysCallProc( GFL_OVERLAY_BLANK_ID, &Debug_Common_Net_CoreProcData, p_wk );
    (*p_seq)++;
    break;
  case SEQ_RET_PROC:
    (*p_seq)++;
    break;
  case SEQ_END:
    DEBUG_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }

}

//=============================================================================
/**
 *					CORE SEQ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  Wi-Fiダウンロードシーケンス
 *
 *	@param	DEBUG_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Debug_Common_Net_Core_Seq_Wifi( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
#if 1
    SEQ_START_SEARCH_DOWNLOAD,
    SEQ_WAIT_SEARCH_DOWNLOAD,
    SEQ_END,

    SEQ_START_DIRECT_DOWNLOAD,
    SEQ_WAIT_DIRECT_DOWNLOAD,
#else
    SEQ_START_DIRECT_DOWNLOAD,
    SEQ_WAIT_DIRECT_DOWNLOAD,
    SEQ_END,

    SEQ_START_SEARCH_DOWNLOAD,
    SEQ_WAIT_SEARCH_DOWNLOAD,
#endif
  };

  DEBUG_COMMON_NET_CORE_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_START_SEARCH_DOWNLOAD:
    {
      COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM param;
      GFL_STD_MemClear( &param, sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM) );
      param.download_search_num   = 5;
      param.cp_attribute_01   = "TEST";
      param.cp_attribute_02   = "";
      param.cp_attribute_03   = "";
      param.GetDownloadIndexCallback = Debug_Common_Net_GetDownloadIndexCallback;
      param.file_size = 0x10;

      OS_TPrintf( "Wi-Fiサーチリクエスト\n" );
      if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_START_SEARCH_DOWNLOAD, &param ) )
      {
        OS_TPrintf( "Wi-Fiサーチリクエストが通りました\n" );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_WAIT_SEARCH_DOWNLOAD:
    if( COMMON_DOWNLOAD_GetState(p_wk->p_common_download ) == COMMON_DOWNLOAD_STATE_WAIT )
    {
      const u8 *cp_data_adrs = COMMON_DOWNLOAD_GetDownloadData( p_wk->p_common_download );
      COMMON_DOWNLOAD_RESULT result = COMMON_DOWNLOAD_GetDownloadResult( p_wk->p_common_download );

      OS_TPrintf( "Wi-Fiサーチ終了\n" );
      OS_TPrintf( "download result%d\n", result );

      if( result == COMMON_DOWNLOAD_RESULT_SUCCESS )
      {
        int i;
        for( i = 0; i < 0x10; i++ )
        {
          OS_TPrintf( "%x ", cp_data_adrs[i] );
        }
        OS_TPrintf( "\n" );
      }

      (*p_seq)++;
    }
    else
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      {
        if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_CANCEL_DOWNLOAD, NULL ) )
        {
          OS_TPrintf( "Wi-Fiキャンセルリクエスト\n" );
        }
      }
    }

    break;
  case SEQ_START_DIRECT_DOWNLOAD:
    {
      COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM param;
      GFL_STD_MemClear( &param, sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM) );
      param.cp_file_name  = "test2.bin";
      param.file_size     = 0x10;

      OS_TPrintf( "Wi-Fiダイレクトリクエスト\n" );
      if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_START_DIRECT_DOWNLOAD, &param ) )
      {
        OS_TPrintf( "Wi-Fiダイレクトリクエストが通りました\n" );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_WAIT_DIRECT_DOWNLOAD:

    if( COMMON_DOWNLOAD_GetState(p_wk->p_common_download ) == COMMON_DOWNLOAD_STATE_WAIT )
    {
      const u8 *cp_data_adrs = COMMON_DOWNLOAD_GetDownloadData( p_wk->p_common_download );
      COMMON_DOWNLOAD_RESULT result = COMMON_DOWNLOAD_GetDownloadResult( p_wk->p_common_download );

      OS_TPrintf( "Wi-Fiダイレクト終了\n" );
      OS_TPrintf( "download result%d\n", result );

      if( result == COMMON_DOWNLOAD_RESULT_SUCCESS )
      {
        int i;
        for( i = 0; i < 0x10; i++ )
        {
          OS_TPrintf( "%x ", cp_data_adrs[i] );
        }
        OS_TPrintf( "\n" );
      }

      (*p_seq)++;
    }
    else
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      {
        if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_WIFI_ND_CANCEL_DOWNLOAD, NULL ) )
        {
          OS_TPrintf( "Wi-Fiキャンセルリクエスト\n" );
        }
      }
    }
    break;
  case SEQ_END:
    DEBUG_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンシーケンス
 *
 *	@param	DEBUG_SEQ_WORK *p_wk　シーケンスワーク
 *	@param	*p_seq                シーケンス
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Debug_Common_Net_Core_Seq_Beacon( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_START_BEACON,
    SEQ_WAIT_BEACON,
    SEQ_END,
  };

  DEBUG_COMMON_NET_CORE_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_START_BEACON:
    {
      COMMON_DOWNLOAD_REQ_BEACON_PARAM param;
      GFL_STD_MemClear( &param, sizeof(COMMON_DOWNLOAD_REQ_BEACON_PARAM) );
      param.net_dev_id = WB_NET_WBT_DOWNLOAD;
      param.language = PM_LANG;
      param.version_bit = 1<<GET_VERSION();
      param.file_size = 512;

      OS_TPrintf( "ビーコンリクエスト\n" );
      if( COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_BEACON_START_DOWNLOAD, &param ) )
      {
        OS_TPrintf( "ビーコンリクエストが通りました\n" );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_WAIT_BEACON:
    if( COMMON_DOWNLOAD_GetState(p_wk->p_common_download ) == COMMON_DOWNLOAD_STATE_WAIT )
    {
      const u8 *cp_data_adrs = COMMON_DOWNLOAD_GetDownloadData( p_wk->p_common_download );
      COMMON_DOWNLOAD_RESULT result = COMMON_DOWNLOAD_GetDownloadResult( p_wk->p_common_download );

      OS_TPrintf( "ビーコン終了\n" );
      OS_TPrintf( "download result%d\n", result );

      if( result == COMMON_DOWNLOAD_RESULT_SUCCESS )
      {
        int i;
        for( i = 0; i < 512; i++ )
        {
          OS_TPrintf( "%x ", cp_data_adrs[i] );
        }
        OS_TPrintf( "\n" );
      }

      (*p_seq)++;
    }
    else
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      {
        COMMON_DOWNLOAD_RequestState( p_wk->p_common_download, COMMON_DOWNLOAD_REQ_BEACON_CANCEL_DOWNLOAD, NULL );
        OS_TPrintf( "ビーコンキャンセルリクエスト\n" );
      }
    }
    break;
  case SEQ_END:
    DEBUG_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ファイルテーブルからダウンロードするインデックスを返す
 *
 *	@param	const DWCNdFileInfo *cp_fileInfo_array  ファイルデータテーブル
 *	@param	array_len テーブル数
 *
 *	@return ダウンロードするインデックス
 */
//-----------------------------------------------------------------------------
static u32 Debug_Common_Net_GetDownloadIndexCallback( const DWCNdFileInfo *cp_fileInfo_array, u32 array_len )
{
  DWCNdFileInfo   empty = {0};

  int i;
  for( i = 0; i < array_len; i++ )
  {
    //中身が空じゃないものの先頭を返す
    if( GFL_STD_MemComp( &empty, &cp_fileInfo_array[i], sizeof(DWCNdFileInfo) ) != 0 )
    {
      return i;
    }
  }

  GF_ASSERT(0);
  return 0;
}
