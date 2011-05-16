//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\net\net_common_download.c
 *	@brief  SWANネット共通ダウンロードモジュール
 *	@author	Toru=Nagihashi
 *	@data		2011.05.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <dwc.h>

//ネット
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/wih_dwc.h"
#include "system/net_err.h"
#include "net/delivery_beacon.h"
#include "net/dwc_error.h"

//外部公開
#include "net/net_common_download.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	RequestStateの引数保存領域
//=====================================
#define COMMON_DOWNLOAD_REQ_STATE_PARAM_SIZE    (32)
SDK_COMPILER_ASSERT( sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM) < COMMON_DOWNLOAD_REQ_STATE_PARAM_SIZE );
SDK_COMPILER_ASSERT( sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM) < COMMON_DOWNLOAD_REQ_STATE_PARAM_SIZE );
SDK_COMPILER_ASSERT( sizeof(COMMON_DOWNLOAD_REQ_BEACON_PARAM) < COMMON_DOWNLOAD_REQ_STATE_PARAM_SIZE );

//-------------------------------------
///	NDエラーの種類
//=====================================
enum {
  ND_RESULT_EXECUTE,		// 実行中
  ND_RESULT_COMPLETE,		// 正常ダウンロード終了
  ND_RESULT_NOT_FOUND_FILES,	// ファイルが見つからなかった
  ND_RESULT_DOWNLOAD_CANCEL,	// ユーザーからキャンセルされた
  ND_RESULT_DOWNLOAD_ERROR,	// なんらかのエラーが起きた
  ND_RESULT_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	WIFIダウンロード
//=====================================
typedef struct 
{
  DWCNdFileInfo     *fileInfo;
  s32               next_seq;
  BOOL              cancel_req;
  BOOL              wifi_cancel;
  int               recv_filenum;
  u32               percent;
  u32               recived;
  u32               contentlen;
  u32               target;
  u32               async_timeout;
  u32               sub_seq;
  BOOL              is_nd_disconnect_enable;
} WIFI_DOWNLOAD_DATA;

//-------------------------------------
///	ハンドル
//=====================================
struct _COMMON_DOWNLOAD
{
  COMMON_DOWNLOAD_INIT_PARAM param; //初期化パラメータ
  BOOL                    is_enable;//コマンド使用許可
  COMMON_DOWNLOAD_RESULT  result;   //ダウンロードリクエスト
  COMMON_DOWNLOAD_ERROR_TYPE error; //エラー
  u32   seq;                        //状態のシーケンス
  WIFI_DOWNLOAD_DATA      nd_data;  //Wi-Fiダウンロードデータ
  DELIVERY_BEACON_WORK  *p_beacon;  //ビーコン配信システム
  BOOL is_beacon_cancel;            //ビーコンキャンセル
  BOOL  (*State)( COMMON_DOWNLOAD *p_wk, u32 *p_seq );  //状態
  u8    req_state_param[COMMON_DOWNLOAD_REQ_STATE_PARAM_SIZE];
  u8    buffer[0];                  //ダウンロードバッファ
};

//-------------------------------------
///	コールバックフラグ
//    コールバックに引数がないのでしかたなくstatic
//=====================================
static BOOL s_callback_flag = FALSE;
static int  s_callback_result = DWC_ND_ERROR_NONE;
static BOOL s_cleanup_callback_flag = FALSE;

//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//状態
static BOOL Common_Download_ErrDisconnectCallbackEx(void* p_wk_adrs, int code, int type, int ret );
//状態
static BOOL Common_Download_State_Wait( COMMON_DOWNLOAD *p_wk, u32 *p_seq );
static BOOL Common_Download_State_WifiNdSearchDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq );
static BOOL Common_Download_State_WifiNdDirectDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq );
static BOOL Common_Download_State_BeaconDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq );

//汎用
static void *Common_Download_GetReqStateParam( COMMON_DOWNLOAD *p_wk );
static BOOL Common_Download_IsEnableRequest( const COMMON_DOWNLOAD *cp_wk, COMMON_DOWNLOAD_REQ request );

//WIFI_DOWNLOADで使う関数
static void WIFI_DOWNLOAD_InitSearch( WIFI_DOWNLOAD_DATA *p_wk, const COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM *cp_param, HEAPID heapID );
static void WIFI_DOWNLOAD_InitDirect( WIFI_DOWNLOAD_DATA *p_wk, const COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM *cp_param, HEAPID heapID );
static void WIFI_DOWNLOAD_Exit( WIFI_DOWNLOAD_DATA *p_wk );
static BOOL WIFI_DOWNLOAD_Common_Sequence_Init( COMMON_DOWNLOAD *p_wk );
static BOOL WIFI_DOWNLOAD_Common_Sequence_Exit( COMMON_DOWNLOAD *p_wk );
static void WIFI_DOWNLOAD_WaitNdCallback( WIFI_DOWNLOAD_DATA *p_wk, u32 *p_seq, int next_seq );
static void WiFi_Download_WaitNdCleanCallback( WIFI_DOWNLOAD_DATA *p_wk, int result, u32 *p_seq, int next_seq, int false_seq);
static void WiFi_Download_Callback(DWCNdCallbackReason reason, DWCNdError error, int servererror);
static BOOL UTIL_IsClear( void *p_adrs, u32 size );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  共通ダウンロード  初期化
 *
 *	@param	const COMMON_DOWNLOAD_INIT_PARAM *cp_param  引数
 *
 *	@return ハンドル
 */
//-----------------------------------------------------------------------------
COMMON_DOWNLOAD * COMMON_DOWNLOAD_Init( const COMMON_DOWNLOAD_INIT_PARAM *cp_param )
{
  u32 size = sizeof(COMMON_DOWNLOAD) + cp_param->buffer_size;
  COMMON_DOWNLOAD *p_wk = GFL_HEAP_AllocClearMemory( cp_param->heapID, size );
  p_wk->param   = *cp_param;
  p_wk->State   = Common_Download_State_Wait;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief　共通ダウンロード  破棄
 *
 *	@param	COMMON_DOWNLOAD *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
void COMMON_DOWNLOAD_Exit( COMMON_DOWNLOAD *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  共通ダウンロード  メイン頭処理
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void COMMON_DOWNLOAD_MainTop( COMMON_DOWNLOAD *p_wk )
{
  if( 1 && p_wk->error == COMMON_DOWNLOAD_ERROR_TYPE_NONE )
  {
    p_wk->is_enable = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  共通ダウンロード  メイン後ろ処理
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void COMMON_DOWNLOAD_MainTail( COMMON_DOWNLOAD *p_wk )
{
  if( p_wk->is_enable )
  {
    if( p_wk->State( p_wk, &p_wk->seq ) )
    {
      p_wk->seq   = 0;
      p_wk->State = Common_Download_State_Wait;
    }
  }

  p_wk->is_enable = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  共通ダウンロード  状態をリクエスト
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 *	@param	request               リクエスト
 *	@param	void *cp_param_adrs   リクエストによる引数（詳しくはCOMMON_DOWNLOAD_REQにコメントとして記述）
 *
 *	@return TRUE  リクエストを受け付けた  FALSE 受付なかった
 */
//-----------------------------------------------------------------------------
BOOL COMMON_DOWNLOAD_RequestState( COMMON_DOWNLOAD *p_wk, COMMON_DOWNLOAD_REQ request, const void *cp_param_adrs )
{
  BOOL enable = Common_Download_IsEnableRequest( p_wk, request );
  GF_ASSERT( request < COMMON_DOWNLOAD_REQ_MAX );

  //リクエスト許可されていないといけない
  if( !p_wk->is_enable )
  {
    return FALSE;
  }

  if( enable )
  {
    switch( request )
    {
    case COMMON_DOWNLOAD_REQ_WIFI_ND_START_SEARCH_DOWNLOAD:
      GFL_STD_MemCopy( cp_param_adrs, p_wk->req_state_param, sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM) );
      p_wk->State = Common_Download_State_WifiNdSearchDownload;
      break;
#ifndef COMMON_DOWNLOAD_WIFI_ND_DIRECT_DESIABLE
    case COMMON_DOWNLOAD_REQ_WIFI_ND_START_DIRECT_DOWNLOAD:
      GFL_STD_MemCopy( cp_param_adrs, p_wk->req_state_param, sizeof(COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM) );
      p_wk->State = Common_Download_State_WifiNdDirectDownload;
      break;
#endif //COMMON_DOWNLOAD_WIFI_ND_DIRECT_DESIABLE
    case COMMON_DOWNLOAD_REQ_WIFI_ND_CANCEL_DOWNLOAD:
      p_wk->nd_data.wifi_cancel = TRUE;
      break;
    case COMMON_DOWNLOAD_REQ_BEACON_START_DOWNLOAD:
      GFL_STD_MemCopy( cp_param_adrs, p_wk->req_state_param, sizeof(COMMON_DOWNLOAD_REQ_BEACON_PARAM) );
      p_wk->State = Common_Download_State_BeaconDownload;
      break;
    case COMMON_DOWNLOAD_REQ_BEACON_CANCEL_DOWNLOAD:
      p_wk->is_beacon_cancel   = TRUE;
      break;
    }
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  現在の状態を取得
 *
 *	@param	const COMMON_DOWNLOAD *cp_wk ワーク
 *
 *	@return 状態
 */
//-----------------------------------------------------------------------------
COMMON_DOWNLOAD_STATE COMMON_DOWNLOAD_GetState( const COMMON_DOWNLOAD *cp_wk )
{
  if( cp_wk->State == Common_Download_State_Wait )
  {
    return COMMON_DOWNLOAD_STATE_WAIT; 
  }
  else if( cp_wk->State == Common_Download_State_WifiNdSearchDownload )
  {
    return COMMON_DOWNLOAD_STATE_WIFI_ND_SEARCH_DOWNLOAD; 
  }
  else if( cp_wk->State == Common_Download_State_WifiNdDirectDownload )
  {
    return COMMON_DOWNLOAD_STATE_WIFI_ND_DIRECT_DOWNLOAD;
  }
  else if( cp_wk->State == Common_Download_State_BeaconDownload )
  {
    return COMMON_DOWNLOAD_STATE_BEACON_DOWNLOAD; 
  }
  else
  {
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロード結果を取得
 *
 *	@param	const COMMON_DOWNLOAD *cp_wk  ワーク
 *
 *	@return   ダウンロード結果
 */
//-----------------------------------------------------------------------------
COMMON_DOWNLOAD_RESULT COMMON_DOWNLOAD_GetDownloadResult( const COMMON_DOWNLOAD *cp_wk )
{
  return cp_wk->result;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロードしたデータアドレスを取得
 *
 *	@param	const COMMON_DOWNLOAD *cp_wk  ワーク
 *
 *	@return ダウンロードしたデータアドレス
 */
//-----------------------------------------------------------------------------
const void * COMMON_DOWNLOAD_GetDownloadData( const COMMON_DOWNLOAD *cp_wk )
{
  return (const void *)cp_wk->buffer;
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラーの解決や取得など取り扱いを行う
 *
 *	@param	const COMMON_DOWNLOAD *cp_wk  ワーク
 *
 *	@return エラーの解決種類
 */
//-----------------------------------------------------------------------------
COMMON_DOWNLOAD_ERROR_TYPE COMMON_DOWNLOAD_HandleError( COMMON_DOWNLOAD *p_wk )
{
  //セーブ中であればエラーにしない
  if( GAMEDATA_GetIsSave( p_wk->param.p_gamedata ) )
  {
    return COMMON_DOWNLOAD_ERROR_TYPE_NONE;
  }

  //NDはCleanUpAsyncを行わないとエラーにしない
  switch( p_wk->param.net_type )
  {
  case COMMON_DOWNLOAD_NET_TYPE_WIFI_ND:
    if( p_wk->nd_data.is_nd_disconnect_enable )
    {
      return COMMON_DOWNLOAD_ERROR_TYPE_NONE;
    }

    //通信中のみエラーチェック＆画面を出す
    if(GFL_NET_IsInit())
    {
      switch( GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE,FALSE) ) 
      {
      case GFL_NET_DWC_ERROR_RESULT_NONE:        //エラーはおきていない
        p_wk->error = COMMON_DOWNLOAD_ERROR_TYPE_NONE;
        break;
      case GFL_NET_DWC_ERROR_RESULT_PRINT_MSG:   //メッセージを描画するだけ
        p_wk->error = COMMON_DOWNLOAD_ERROR_TYPE_RETURN;
        break;
      case GFL_NET_DWC_ERROR_RESULT_RETURN_PROC: //PROCから抜けなければならない
        p_wk->error = COMMON_DOWNLOAD_ERROR_TYPE_DISCONNECT;
        break;
      case GFL_NET_DWC_ERROR_RESULT_FATAL:       //電源切断のため無限ループになる
        GF_ASSERT(0);
      case GFL_NET_DWC_ERROR_RESULT_TIMEOUT:     //タイムアウト中
        GF_ASSERT(0);
      }
    }
    break;

  case COMMON_DOWNLOAD_NET_TYPE_BEACON:
    p_wk->error = COMMON_DOWNLOAD_ERROR_TYPE_NONE;
    break;
  }
  return p_wk->error;
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラーのクリア
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void COMMON_DOWNLOAD_ClearError( COMMON_DOWNLOAD *p_wk )
{
  p_wk->error = COMMON_DOWNLOAD_ERROR_TYPE_NONE;
}
//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  なにもしない
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 *	@param	*p_seq  シーケンス
 *
 *	@return TRUEで状態終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_State_Wait( COMMON_DOWNLOAD *p_wk, u32 *p_seq )
{
  /* なにもしない */
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  Wi-FiNDダウンロード
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 *	@param	*p_seq  シーケンス
 *
 *	@return TRUEで状態終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_State_WifiNdSearchDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq )
{
  enum
  { 
    SEQ_INIT,
    SEQ_ATTR,
    SEQ_CHECK_DATA,
    SEQ_FILELIST,
    SEQ_CHECKLIST,
    SEQ_GET_FILE,
    SEQ_GETTING_FILE,

    SEQ_CANCEL,
    SEQ_DOWNLOAD_COMPLETE,
    SEQ_END,

    SEQ_WAIT_CALLBACK         = 100,
    SEQ_WAIT_CLEANUP_CALLBACK = 200,
  };

  WIFI_DOWNLOAD_DATA  *p_nd_data  = &p_wk->nd_data;
  COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM *p_param = Common_Download_GetReqStateParam(p_wk);

  switch( *p_seq )
  { 
  case SEQ_INIT:
    GFL_NET_DWC_SetErrDisconnectCallbackEx(Common_Download_ErrDisconnectCallbackEx, p_nd_data );
    WIFI_DOWNLOAD_InitSearch( p_nd_data, p_param, p_wk->param.heapID );

    p_wk->result = COMMON_DOWNLOAD_RESULT_NONE;
    if( DWC_NdInitAsync( WiFi_Download_Callback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      OS_TPrintf( "DWC_NdInitAsync: Failed\n" );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      *p_seq  = SEQ_END;
    }
    else
    { 
      p_nd_data->is_nd_disconnect_enable = TRUE;
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_ATTR );
    }
    break;

  case SEQ_ATTR:
    //キャンセル
    if( p_nd_data->wifi_cancel )
    { 
      p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
    }
    else
    { 
      // ファイル属性の設定
      if( DWC_NdSetAttr(p_param->cp_attribute_01, p_param->cp_attribute_02, p_param->cp_attribute_03) == FALSE )
      {
        OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
        p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
        WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END );
      }
      else
      { 
        *p_seq = SEQ_FILELIST;
      }
    }
    break;

//-------------------------------------
///	サーバーのファイルチェック
//=====================================
  case SEQ_FILELIST:
    { 
      //一気に複数個見に行く
      if( DWC_NdGetFileListAsync( p_nd_data->fileInfo, 0, p_param->download_search_num ) == FALSE)
      {
        OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
        WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END );
      }
      else
      { 
        WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_CHECK_DATA );
      }
    }
    break;

  case SEQ_CHECK_DATA:
    { 
      int i;
      //カラじゃないものの数を取得
      p_nd_data->recv_filenum = 0;
      for( i = 0; i < p_param->download_search_num; i++ )
      { 
        if( !UTIL_IsClear( &p_nd_data->fileInfo[i], sizeof(DWCNdFileInfo) ) )
        {
          p_nd_data->recv_filenum++;
        }
      }

      OS_Printf( "%d個のファイルを取得可能\n",p_nd_data->recv_filenum );
      *p_seq  = SEQ_CHECKLIST;
    }
    break;

  case SEQ_CHECKLIST:
    if( p_nd_data->recv_filenum == 1 )
    { 
      //自分が取得できるファイルが１つならば、それを落とす
      int i;
      for( i = 0; i < p_param->download_search_num; i++ )
      { 
        if( !UTIL_IsClear( &p_nd_data->fileInfo[i], sizeof(DWCNdFileInfo) ) )
        {
          p_nd_data->target = i;
          break;
        }
      }
    }
    else if( p_nd_data->recv_filenum > 1 )
    {
      p_nd_data->target = p_param->GetDownloadIndexCallback( p_nd_data->fileInfo, p_param->download_search_num );
    }
    else
    { 
      //ファイルがないときは終了
      p_wk->result  = COMMON_DOWNLOAD_RESULT_EMPTY;
      WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_COMPLETE, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      break;
    }

    *p_seq  = SEQ_GET_FILE;
    break;
//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    // ファイル読み込み開始
    OS_TPrintf( "取得するもの target%d max%d size%d \n", p_nd_data->target, p_nd_data->recv_filenum, p_nd_data->fileInfo[ p_nd_data->target ].size );
    if(DWC_NdGetFileAsync( &p_nd_data->fileInfo[ p_nd_data->target ], (void*)p_wk->buffer, p_nd_data->fileInfo[ p_nd_data->target ].size ) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END ); 
    }
    else
    { 
      p_nd_data->percent = 0;
      s_callback_flag   = FALSE;
      s_callback_result = DWC_ND_ERROR_NONE;
      p_nd_data->async_timeout  = 0;
      *p_seq = SEQ_GETTING_FILE;
    }
    break;
    
  case SEQ_GETTING_FILE:
    // ファイル読み込み中
    DWC_NdProcess();
    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_TIMEOUT;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
    }

    if( s_callback_flag == FALSE )
    {
      // ファイル読み込み中
      if( p_nd_data->cancel_req )
      {
        // ダウンロードキャンセル
        p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
        WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL ); 
      }
      else
      {
        // 進行度を表示
        if(DWC_NdGetProgress( &p_nd_data->recived, &p_nd_data->contentlen ) == TRUE)
        {
          if(p_nd_data->percent != (p_nd_data->recived*100)/p_nd_data->contentlen)
          {
            p_nd_data->percent = (p_nd_data->recived*100)/p_nd_data->contentlen;
            OS_Printf( "Download %d/100\n", p_nd_data->percent );
          }
        }
      }
    }
    else if( s_callback_result != DWC_ND_ERROR_NONE)
    {
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END ); 
    }
    else
    { //callback1_result
      if(p_nd_data->wifi_cancel == FALSE)
      {
        // ファイル読み込み終了
        p_wk->result  = COMMON_DOWNLOAD_RESULT_SUCCESS;
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_COMPLETE, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      } else {
        // ダウンロードキャンセル
        p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      }
    }
    break;

//-------------------------------------
///	終了・キャンセル処理
//=====================================
  case SEQ_CANCEL:
    if( DWC_NdCancelAsync() == FALSE )
    {
      *p_seq = SEQ_DOWNLOAD_COMPLETE;
    }
    else 
    {
      *p_seq = SEQ_DOWNLOAD_COMPLETE;
    }
    break;

  case SEQ_DOWNLOAD_COMPLETE:
    *p_seq  = SEQ_END;
    break;
    
  case SEQ_END:
    WIFI_DOWNLOAD_Exit( p_nd_data );

    GFL_NET_DWC_SetErrDisconnectCallbackEx( NULL, NULL );
    p_nd_data->is_nd_disconnect_enable = FALSE;
    return TRUE;

//-------------------------------------
///	コールバック待ち処理  
//　　WIFI_DOWNLOAD_WaitNdCallbackやWiFi_Download_WaitNdCleanCallback等を使ってください
//=====================================
  case SEQ_WAIT_CALLBACK:
    //コールバック処理を待つ
    DWC_NdProcess();

    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_TIMEOUT;
      WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_END, SEQ_END ); 
    }

    if( s_callback_flag )
    { 
      s_callback_flag = FALSE;
      if( s_callback_result != DWC_ND_ERROR_NONE)
      {
        //エラーが起こったので終了
        p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_END, SEQ_END );  
      }
      else
      { 
        *p_seq  = p_nd_data->next_seq;
      }
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "キャンセルしました!\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;

  case SEQ_WAIT_CLEANUP_CALLBACK:
    DWC_NdProcess();

    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      *p_seq  = p_nd_data->next_seq;
    }

    if( s_cleanup_callback_flag )
    { 
      s_cleanup_callback_flag = FALSE;
      *p_seq  = p_nd_data->next_seq;
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "クリーンアップしました! 2\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  直接ダウンロード
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 *	@param	*p_seq  シーケンス
 *
 *	@return TRUEで状態終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_State_WifiNdDirectDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq )
{
  enum
  { 
    SEQ_INIT,
    SEQ_ATTR,
    SEQ_GET_FILE,
    SEQ_GETTING_FILE,

    SEQ_CANCEL,
    SEQ_DOWNLOAD_COMPLETE,
    SEQ_END,

    SEQ_WAIT_CALLBACK         = 100,
    SEQ_WAIT_CLEANUP_CALLBACK = 200,
  };

  WIFI_DOWNLOAD_DATA  *p_nd_data  = &p_wk->nd_data;
  COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM *p_param = Common_Download_GetReqStateParam( p_wk );

  switch( *p_seq )
  { 
  case SEQ_INIT:
    GFL_NET_DWC_SetErrDisconnectCallbackEx(Common_Download_ErrDisconnectCallbackEx, p_nd_data );
    WIFI_DOWNLOAD_InitDirect( p_nd_data, p_param, p_wk->param.heapID );

    p_wk->result = COMMON_DOWNLOAD_RESULT_NONE;
    if( DWC_NdInitAsync( WiFi_Download_Callback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      OS_TPrintf( "DWC_NdInitAsync: Failed\n" );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      *p_seq  = SEQ_END;
    }
    else
    { 
      p_nd_data->is_nd_disconnect_enable = TRUE;
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_ATTR );
    }
    break;

  case SEQ_ATTR:
    //キャンセル
    if( p_nd_data->wifi_cancel )
    { 
      p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
    }
    else
    { 
      // ファイル属性の設定
      if( DWC_NdSetAttr("", "", "") == FALSE )
      {
        OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
        p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
        WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END );
      }
      else
      { 
        *p_seq = SEQ_GET_FILE;
      }
    }
    break;

//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    // ファイル読み込み開始
    OS_TPrintf( "取得するもの target%d max%d size%d\n", p_nd_data->target, p_nd_data->recv_filenum, p_nd_data->fileInfo->size );
    if(DWC_NdGetFileAsync( p_nd_data->fileInfo, (void*)p_wk->buffer, p_nd_data->fileInfo->size ) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END ); 
    }
    else
    { 
      p_nd_data->percent = 0;
      s_callback_flag   = FALSE;
      s_callback_result = DWC_ND_ERROR_NONE;
      p_nd_data->async_timeout  = 0;
      *p_seq = SEQ_GETTING_FILE;
    }
    break;
    
  case SEQ_GETTING_FILE:
    // ファイル読み込み中
    DWC_NdProcess();
    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_TIMEOUT;
      WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
    }

    if( s_callback_flag == FALSE )
    {
      // ファイル読み込み中
      if( p_nd_data->cancel_req )
      {
        // ダウンロードキャンセル
        p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
        WiFi_Download_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL ); 
      }
      else
      {
        // 進行度を表示
        if(DWC_NdGetProgress( &p_nd_data->recived, &p_nd_data->contentlen ) == TRUE)
        {
          if(p_nd_data->percent != (p_nd_data->recived*100)/p_nd_data->contentlen)
          {
            p_nd_data->percent = (p_nd_data->recived*100)/p_nd_data->contentlen;
            OS_Printf( "Download %d/100\n", p_nd_data->percent );
          }
        }
      }
    }
    else if( s_callback_result != DWC_ND_ERROR_NONE)
    {
      p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
      OS_Printf( "エラー発生%d\n",s_callback_result );
      WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_ERROR, p_seq, SEQ_END, SEQ_END ); 
    }
    else
    { //callback1_result
      if(p_nd_data->wifi_cancel == FALSE)
      {
        // ファイル読み込み終了
        p_wk->result  = COMMON_DOWNLOAD_RESULT_SUCCESS;
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_COMPLETE, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      } else {
        // ダウンロードキャンセル
        p_wk->result  = COMMON_DOWNLOAD_RESULT_CANCEL;
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      }
    }
    break;

//-------------------------------------
///	終了・キャンセル処理
//=====================================
  case SEQ_CANCEL:
    if( DWC_NdCancelAsync() == FALSE )
    {
      *p_seq = SEQ_DOWNLOAD_COMPLETE;
    }
    else 
    {
      *p_seq = SEQ_DOWNLOAD_COMPLETE;
    }
    break;

  case SEQ_DOWNLOAD_COMPLETE:
    *p_seq  = SEQ_END;
    break;
    
  case SEQ_END:
    WIFI_DOWNLOAD_Exit( p_nd_data );

    GFL_NET_DWC_SetErrDisconnectCallbackEx( NULL, NULL );
    p_nd_data->is_nd_disconnect_enable = FALSE;
    return TRUE;

//-------------------------------------
///	コールバック待ち処理  
//　　WIFI_DOWNLOAD_WaitNdCallbackやWiFi_Download_WaitNdCleanCallback等を使ってください
//=====================================
  case SEQ_WAIT_CALLBACK:
    //コールバック処理を待つ
    DWC_NdProcess();

    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      p_wk->result  = COMMON_DOWNLOAD_RESULT_TIMEOUT;
      WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_END, SEQ_END ); 
    }

    if( s_callback_flag )
    { 
      s_callback_flag = FALSE;
      if( s_callback_result != DWC_ND_ERROR_NONE)
      {
        //エラーが起こったので終了
        p_wk->result  = COMMON_DOWNLOAD_RESULT_FAILED;
        OS_Printf( "エラー発生%d\n",s_callback_result );
        WiFi_Download_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_END, SEQ_END );  
      }
      else
      { 
        *p_seq  = p_nd_data->next_seq;
      }
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "キャンセルしました!\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;

  case SEQ_WAIT_CLEANUP_CALLBACK:
    DWC_NdProcess();

    if( p_nd_data->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      *p_seq  = p_nd_data->next_seq;
    }

    if( s_cleanup_callback_flag )
    { 
      s_cleanup_callback_flag = FALSE;
      *p_seq  = p_nd_data->next_seq;
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "クリーンアップしました! 2\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンダウンロード
 *
 *	@param	COMMON_DOWNLOAD *p_wk ワーク
 *	@param	*p_seq  シーケンス
 *
 *	@return TRUEで状態終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_State_BeaconDownload( COMMON_DOWNLOAD *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_INIT_WAIT,
    SEQ_MAIN,
    SEQ_EXIT,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };

  COMMON_DOWNLOAD_REQ_BEACON_PARAM *p_param = Common_Download_GetReqStateParam(p_wk);

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      DELIVERY_BEACON_INIT  belivery_beacon_init;
      GFL_STD_MemClear( &belivery_beacon_init, sizeof(DELIVERY_BEACON_INIT) );
      belivery_beacon_init.NetDevID     = p_param->net_dev_id;
      belivery_beacon_init.data[0].datasize = p_param->file_size;  //データ全体サイズ
      belivery_beacon_init.data[0].pData = (void*)p_wk->buffer;     // 受信バッファデータ
      belivery_beacon_init.data[0].LangCode  = p_param->language;     // 受け取る言語コード
      belivery_beacon_init.data[0].version   = p_param->version_bit;     // 受け取るバージョンのビット
      belivery_beacon_init.dataNum = 1;  //受け取り側は１
      belivery_beacon_init.ConfusionID  = 0;
      belivery_beacon_init.heapID       = p_wk->param.heapID;
      p_wk->p_beacon  = DELIVERY_BEACON_Init( &belivery_beacon_init );
      DELIVERY_BEACON_RecvStart( p_wk->p_beacon );    
      p_wk->is_beacon_cancel = FALSE;
    }
    p_wk->result = COMMON_DOWNLOAD_RESULT_NONE;
    (*p_seq)++;
    break;

  case SEQ_INIT_WAIT:
    if( GFL_NET_IsInit() )
		{
      //通信アイコン
      GFL_NET_ReloadIconTopOrBottom( TRUE, p_wk->param.heapID );
      (*p_seq)++;
    }
    break;

  case SEQ_MAIN:
    DELIVERY_BEACON_Main( p_wk->p_beacon );

    //受信成功
    if( DELIVERY_BEACON_RecvCheck(p_wk->p_beacon) )
    { 
      OS_TFPrintf( 3, "ビーコン受け取り\n" );
      p_wk->result = COMMON_DOWNLOAD_RESULT_SUCCESS;
      *p_seq = SEQ_EXIT;
    }
    else if( p_wk->is_beacon_cancel )
    {
      //受け取りキャンセル
      p_wk->result = COMMON_DOWNLOAD_RESULT_CANCEL;
      *p_seq = SEQ_EXIT;
    }
    break;
    
  case SEQ_EXIT:
    DELIVERY_BEACON_End( p_wk->p_beacon );
    (*p_seq)++;
    break;

  case SEQ_EXIT_WAIT:
    if( GFL_NET_IsResetEnable() )
		{
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  状態リクエストの際の引数を取得
 *
 *	@param	COMMON_DOWNLOAD *p_wk   ワーク
 *
 *	@return 引数バッファ
 */
//-----------------------------------------------------------------------------
static void *Common_Download_GetReqStateParam( COMMON_DOWNLOAD *p_wk )
{
  return (void*)p_wk->req_state_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief リクエストを受け付けるかどうかチェック
 *
 *	@param	const COMMON_DOWNLOAD *cp_wk  ワーク
 *	@param	request   検査するリクエスト
 *
 *	@return TRUEならば受け付ける  FALSEならば受け付けない
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_IsEnableRequest( const COMMON_DOWNLOAD *cp_wk, COMMON_DOWNLOAD_REQ request )
{
  static const u8 sc_state_enable_tbl[COMMON_DOWNLOAD_STATE_MAX][COMMON_DOWNLOAD_REQ_MAX] =
  { // ND_START_SEARCH, ND_START_DIRECT, ND_CANCEL, BEACON_START, BEACON_CANCEL
    { TRUE,             TRUE,             FALSE,    TRUE,          FALSE, },//COMMON_DOWNLOAD_STATE_WAIT
    { FALSE,            FALSE,            TRUE,     FALSE,         FALSE, },//COMMON_DOWNLOAD_STATE_WIFI_ND_SEARCH_DOWNLOAD
    { FALSE,            FALSE,            TRUE,     FALSE,         FALSE, },//COMMON_DOWNLOAD_STATE_WIFI_ND_DIRECT_DOWNLOAD
    { FALSE,            FALSE,            FALSE,    FALSE,         TRUE, },//COMMON_DOWNLOAD_STATE_BEACON_DOWNLOAD
  };

  static const u8 sc_type_enable_tbl[COMMON_DOWNLOAD_NET_TYPE_MAX][COMMON_DOWNLOAD_REQ_MAX] =
  { // ND_START_SEARCH, ND_START_DIRECT, ND_CANCEL, BEACON_START, BEACON_CANCEL
    { TRUE, TRUE, TRUE, FALSE, FALSE },//COMMON_DOWNLOAD_NET_TYPE_WIFI_ND
    { FALSE, FALSE, FALSE, TRUE, TRUE },//COMMON_DOWNLOAD_NET_TYPE_BEACON
  };

  COMMON_DOWNLOAD_STATE state = COMMON_DOWNLOAD_GetState( cp_wk );

  GF_ASSERT_MSG( sc_state_enable_tbl[state][request], "リクエスト可能な状態ではありません。state%d, request%d", state, request);
  GF_ASSERT_MSG( sc_type_enable_tbl[cp_wk->param.net_type][request], "現在の通信タイプではこのリクエストは使えません net_type%d request%d",cp_wk->param.net_type, request );

  return sc_state_enable_tbl[state][request] && sc_type_enable_tbl[cp_wk->param.net_type][request];
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラー時に切断の前に呼ばれるコールバック  拡張版
 *
 *	@param	p_wk_adrs ワークアドレス
 *	@param	code      エラーコード
 *	@param	type      エラータイプ
 *	@param	ret       エラーリターン
 *  @retval  TRUEならば切断処理をしないでこの関数を呼びつづける　
 *          FALSEならば切断処理をして終了する
 */
//-----------------------------------------------------------------------------
static BOOL Common_Download_ErrDisconnectCallbackEx(void* p_wk_adrs, int code, int type, int ret )
{
  WIFI_DOWNLOAD_DATA *p_wk  = p_wk_adrs;

  NAGI_Printf( "Call ErrDisconnectCallback[EX] code=%d type=%d ret=%d\n", code, type, ret );
  switch( type )
  {
    //切断エラー
  case DWC_ETYPE_SHUTDOWN_FM:
  case DWC_ETYPE_DISCONNECT:
  case DWC_ETYPE_SHUTDOWN_ND:
    return p_wk->is_nd_disconnect_enable;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIダウンロードサーバー用ワーク検索用初期化
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM *cp_param  引数
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_InitSearch( WIFI_DOWNLOAD_DATA *p_wk, const COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM *cp_param, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFI_DOWNLOAD_DATA) );

  p_wk->fileInfo  = GFL_HEAP_AllocClearMemory(heapID, cp_param->download_search_num*sizeof(DWCNdFileInfo) );
}
//----------------------------------------------------------------------------
/**
 *	@brief　WIFIダウンロードサーバー用ワーク直接指定用初期化
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM *cp_param  引数
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_InitDirect( WIFI_DOWNLOAD_DATA *p_wk, const COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM *cp_param, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFI_DOWNLOAD_DATA) );
  p_wk->fileInfo  = GFL_HEAP_AllocClearMemory(heapID, sizeof(DWCNdFileInfo) );

  //ファイル名をコピー
  {
    int len = GFL_STD_StrLen( cp_param->cp_file_name );
    GF_ASSERT( len < DWC_ND_FILENAME_LEN );
    GFL_STD_MemCopy( cp_param->cp_file_name, p_wk->fileInfo->name, len );
  }

  //サイズをコピー
  p_wk->fileInfo->size = cp_param->file_size;
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIダウンロードサーバー用ワーク破棄
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_Exit( WIFI_DOWNLOAD_DATA *p_wk )
{
  if( p_wk->fileInfo )
  {
    GFL_HEAP_FreeMemory( p_wk->fileInfo );
  }

  GFL_STD_MemClear( p_wk, sizeof(WIFI_DOWNLOAD_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック待ち
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	*p_seq                    シーケンス
 *	@param	next_seq                  コールバック成功後へいくシーケンス
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_WaitNdCallback( WIFI_DOWNLOAD_DATA *p_wk, u32 *p_seq, int next_seq )
{ 
  s_callback_flag   = FALSE;
  s_callback_result = DWC_ND_ERROR_NONE;
  p_wk->next_seq  = next_seq;
  p_wk->async_timeout  = 0;
  *p_seq          = 100;
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック待ち２
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	result                    結果
 *	@param	*seq                      シーケンス
 *	@param	next_seq                  コールバック成功後にいくシーケンス
 *	@param	false_seq                 コールバック失敗後にいくシーケンス
 */
//-----------------------------------------------------------------------------
static void WiFi_Download_WaitNdCleanCallback( WIFI_DOWNLOAD_DATA *p_wk, int result, u32 *p_seq, int next_seq, int false_seq)
{ 
  s_cleanup_callback_flag = FALSE;
  s_callback_result       = result;
  p_wk->next_seq          = next_seq;
  p_wk->async_timeout  = 0;
  *p_seq                  = 200;

  if( !DWC_NdCleanupAsync(  ) ){  //FALSEの場合コールバックが呼ばれない
    OS_Printf("DWC_NdCleanupAsyncに失敗\n");
    *p_seq = false_seq;
  }
}
/*-------------------------------------------------------------------------*
 * Name        : WiFi_Download_Callback
 * Description : ND用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void WiFi_Download_Callback(DWCNdCallbackReason reason, DWCNdError error, int servererror)
{
  OS_Printf("WiFi_Download_Callback: Called\n");
  switch(reason) {
  case DWC_ND_CBREASON_GETFILELISTNUM:
    OS_Printf("DWC_ND_CBREASON_GETFILELISTNUM\n");
    break;
  case DWC_ND_CBREASON_GETFILELIST:
    OS_Printf("DWC_ND_CBREASON_GETFILELIST\n");
    break;
  case DWC_ND_CBREASON_GETFILE:
    OS_Printf("DWC_ND_CBREASON_GETFILE\n");
    break;
  case DWC_ND_CBREASON_INITIALIZE:
    OS_Printf("DWC_ND_CBREASON_INITIALIZE\n");
    break;
  }
	
  switch(error) {
  case DWC_ND_ERROR_NONE:
    OS_Printf("DWC_ND_NOERR\n");
    break;
  case DWC_ND_ERROR_ALLOC:
    OS_Printf("DWC_ND_MEMERR\n");
    break;
  case DWC_ND_ERROR_BUSY:
    OS_Printf("DWC_ND_BUSYERR\n");
    break;
  case DWC_ND_ERROR_HTTP:
    OS_Printf("DWC_ND_HTTPERR\n");
    // ファイル数の取得でＨＴＴＰエラーが発生した場合はダウンロードサーバに繋がっていない可能性が高い
    if( reason == DWC_ND_CBREASON_GETFILELISTNUM )
      {
          OS_Printf( "It is not possible to connect download server.\n." );
      }
    break;
  case DWC_ND_ERROR_BUFFULL:
    OS_Printf("DWC_ND_BUFFULLERR\n");
    break;
  case DWC_ND_ERROR_DLSERVER:
    OS_Printf("DWC_ND_SERVERERR\n");
    break;
  case DWC_ND_ERROR_CANCELED:
    OS_Printf("DWC_ND_CANCELERR\n");
    break;
  }
  OS_Printf("errorcode = %d\n", servererror);
  s_callback_flag   = TRUE;
  s_cleanup_callback_flag = TRUE;
  s_callback_result = error;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ０クリアされているかどうか
 *
 *	@param	void *p_adrs  アドレス
 *	@param	size          バイトサイズ
 *
 *	@return TRUEで０クリアされている  FALSEでされていない
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_IsClear( void *p_adrs, u32 size )
{
  int i;
  u32 *p_ptr  = (u32*)p_adrs;

  for( i = 0; i < size/4; i++ )
  {
    if( *(p_ptr + i) != 0 )
    {
      return FALSE;
    }
  }

  return TRUE;
}
