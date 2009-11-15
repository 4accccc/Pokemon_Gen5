#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	dwc_raputil.c
 * @bfief	WIFI設定画面呼び出し用
 * @author	k.ohnop
 * @date	09/01/15
 */
//=============================================================================

#pragma once

#include <gflib.h>

//==============================================================================
/**
 * WiFi接続ユーティリティを呼び出す
 * @param   none
 * @retval  none
 */
//==============================================================================


FS_EXTERN_OVERLAY(wifi_util);
extern const GFL_PROC_DATA WifiUtilProcData;
extern const GFL_PROC_DATA WifiUtilGSyncProcData;




#ifdef __cplusplus
} /* extern "C" */
#endif
