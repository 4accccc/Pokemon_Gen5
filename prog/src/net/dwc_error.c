//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		dwc_error.c
 *	@brief  DWC汎用エラーチェック検知関数
 *	@author	Toru=Nagihashi
 *	@data		2010.02.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <dwc.h>

//ネットワーク
#include "system/net_err.h"
#include "net/dwc_rap.h"

//メッセージ
#include "msg\msg_wifi_system.h"

//外部公開
#include "net/dwc_error.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
#ifdef DEBUG_ONLY_FOR_toru_nagihashi
#define DEBUG_DWC_ERROR_PRINT
#endif //DEBUG_ONLY_FOR_
#endif //PM_DEBUG

#ifdef DEBUG_DWC_ERROR_PRINT
#define DEBUG_DWC_ERROR_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#else //DEBUG_DWC_ERROR_PRINT
#define DEBUG_DWC_ERROR_Printf(...) /*  */
#endif //DEBUG_DWC_ERROR_PRINT

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================


static void _ErrorClear(void)
{
  GFL_NET_StateClearWifiError();
  GFL_NET_StateResetError();
  DWC_ClearError(); //表示後クリアする決まり
}


//----------------------------------------------------------------------------
/**
 *	@brief  DWCでエラーをチェックしエラーの軽度によって表示を切り分ける
 *	        軽いエラーならば、その場で表示
 *	        重いエラーならば、プロセス移動時に表示
 *	        Fatalならば、その場で表示し無限ループになる
 *
 *	@param	is_heavy  TRUEならば軽度判別する　FALSEならば軽度も重度として返す
 *	@param  is_timeout  タイムアウトを検知するか
 *
 *	@return TRUEエラー発生し表示  FALSEならばエラーではない
 */
//-----------------------------------------------------------------------------
GFL_NET_DWC_ERROR_RESULT GFL_NET_DWC_ERROR_ReqErrorDisp( BOOL is_heavy, BOOL is_timeout )
{ 
  if( GFL_NET_IsInit() )
  { 
    const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
    { 
      DEBUG_DWC_ERROR_Printf( "GFL_NET_DWC_ERROR_ReqErrorDisp code=%d type=%d line=%d\n", cp_error->errorType, cp_error->errorUser, __LINE__ );
      switch( cp_error->errorType )
      { 
      case DWC_ETYPE_LIGHT:
        /* fallthru */
      case DWC_ETYPE_SHOW_ERROR:
        //エラーコードorメッセージを表示するだけ
        if( !is_heavy )
        {
          _ErrorClear();
          NetErr_DispCallPushPop();
          NetErr_ErrWorkInit();
          return GFL_NET_DWC_ERROR_RESULT_PRINT_MSG;
        }
        /* fallthru */
        DWC_ClearError(); //表示後クリアする決まりだが無理やりシャットダウンにしたい場合はその前にクリアする

      case DWC_ETYPE_SHUTDOWN_FM:
      case DWC_ETYPE_SHUTDOWN_GHTTP:
      case DWC_ETYPE_SHUTDOWN_ND:
        //シャットダウン
        NetErr_App_ReqErrorDisp();
        //下記はDWCのエラーにならないときに呼ぶ。
        //上記関数を受け付けていれば下記は呼ばれない。
        NetErr_App_ReqErrorDispForce( dwc_error_0014 );
        return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;

      case DWC_ETYPE_DISCONNECT:
        //切断
        NetErr_App_ReqErrorDisp();
        return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;

      case DWC_ETYPE_FATAL:
        //Fatal
        NetErr_App_FatalDispCallWifiMessage(dwc_error_0016);
        return GFL_NET_DWC_ERROR_RESULT_FATAL;
      }
    }
    //マッチング系
    if( (cp_error->errorUser == ERRORCODE_TIMEOUT 
        || cp_error->errorUser == ERRORCODE_DISCONNECT ) )
    {
      if( is_timeout )
      {
        //マッチングタイムアウトをエラーにする場合
        NetErr_ErrorSet();    //エラーをセット
        _ErrorClear();
        GFL_NET_StateWifiMatchEnd(TRUE);  //エラーをクリアしてもずっとタイムアウト中になってしまうので
        NetErr_DispCallPushPop();
        NetErr_ErrWorkInit();

//        NetErr_DispCallPushPop();       //エラーメッセージ表示
//        GFL_NET_StateClearWifiError();  //WIFIエラー詳細情報クリア
//        NetErr_ErrWorkInit();           //エラーシステム情報クリア
//        GFL_NET_StateResetError();      //NET内エラー情報クリア
        //解消する
        DEBUG_DWC_ERROR_Printf( "GFL_NET_DWC_ERROR_ReqErrorDisp user=%d line=%d\n", cp_error->errorUser, __LINE__ );
        return GFL_NET_DWC_ERROR_RESULT_TIMEOUT;
      }
      else
      {
        //マッチングタイムアウトをエラーにしない場合
        GFL_NET_StateClearWifiError();  //WIFIエラー詳細情報クリア
        NetErr_ErrWorkInit();           //エラーシステム情報クリア
        GFL_NET_StateResetError();      //NET内エラー情報クリア
        GFL_NET_StateWifiMatchEnd(TRUE);  //エラーをクリアしてもずっとタイムアウト中になってしまうので
        return GFL_NET_DWC_ERROR_RESULT_NONE;
      }
    }
    //システムエラー（ライト）系
    else if( cp_error->errorUser == ERRORCODE_CRC
        || cp_error->errorUser == ERRORCODE_SYSTEM 
        || cp_error->errorUser == ERRORCODE_SENDQUEUE
        || cp_error->errorUser == ERRORCODE_NHTTP )
    {
      if( !is_heavy )
      { 
        NetErr_DispCallPushPop();
        GFL_NET_StateClearWifiError();
        NetErr_ErrWorkInit();
        GFL_NET_StateResetError();
        DEBUG_DWC_ERROR_Printf( "GFL_NET_DWC_ERROR_ReqErrorDisp user=%d line=%d\n", cp_error->errorUser, __LINE__ );
        return GFL_NET_DWC_ERROR_RESULT_PRINT_MSG;
      }
      else
      {
        NetErr_App_ReqErrorDisp();
        NetErr_App_ReqErrorDispForce( dwc_error_0014 );

        DEBUG_DWC_ERROR_Printf( "GFL_NET_DWC_ERROR_ReqErrorDisp user=%d line=%d\n", cp_error->errorUser, __LINE__ );
        return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;
      }
    }
    //システムエラー（ヘビー）系  ヘビー系ができたらifの中に書いてください
    else  if( cp_error->errorUser == ERRORCODE_HEAP )
    {
      NetErr_App_ReqErrorDisp();
      NetErr_App_ReqErrorDispForce( dwc_error_0014 );
      DEBUG_DWC_ERROR_Printf( "GFL_NET_DWC_ERROR_ReqErrorDisp user=%d line=%d\n", cp_error->errorUser, __LINE__ );
      return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;
    }
  }

  return GFL_NET_DWC_ERROR_RESULT_NONE;
}
