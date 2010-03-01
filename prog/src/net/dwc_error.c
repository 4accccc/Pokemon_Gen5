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

//外部公開
#include "net/dwc_error.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

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

//----------------------------------------------------------------------------
/**
 *	@brief  DWCでエラーをチェックしエラーの軽度によって表示を切り分ける
 *	        軽いエラーならば、その場で表示
 *	        重いエラーならば、プロセス移動時に表示
 *	        Fatalならば、その場で表示し無限ループになる
 *
 *	@param	TRUEならば軽度判別する　FALSEならば軽度も重度として返す
 *
 *	@return TRUEエラー発生し表示  FALSEならばエラーではない
 */
//-----------------------------------------------------------------------------
GFL_NET_DWC_ERROR_RESULT GFL_NET_DWC_ERROR_ReqErrorDisp( BOOL is_heavy )
{ 
  if( GFL_NET_IsInit() )
  { 
    if( NetErr_App_CheckError() )
    { 
      const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

      switch( cp_error->errorType )
      { 
      case DWC_ETYPE_LIGHT:
        /* fallthru */
      case DWC_ETYPE_SHOW_ERROR:
        //エラーコードorメッセージを表示するだけ
        if( !is_heavy )
        { 
          NetErr_DispCallPushPop();
          GFL_NET_StateClearWifiError();
          return GFL_NET_DWC_ERROR_RESULT_PRINT_MSG;
        }
        /* fallthru */

      case DWC_ETYPE_SHUTDOWN_FM:
      case DWC_ETYPE_SHUTDOWN_GHTTP:
      case DWC_ETYPE_SHUTDOWN_ND:
        //シャットダウン
        NetErr_App_ReqErrorDisp();
        return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;

      case DWC_ETYPE_DISCONNECT:
        //切断
        NetErr_App_ReqErrorDisp();
        return GFL_NET_DWC_ERROR_RESULT_RETURN_PROC;

      case DWC_ETYPE_FATAL:
        //Fatal
        NetErr_DispCallFatal();
        return GFL_NET_DWC_ERROR_RESULT_FATAL;
      }
    }
  }

  return GFL_NET_DWC_ERROR_RESULT_NONE;
}
