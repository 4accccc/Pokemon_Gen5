/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_common.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc/common.h>


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXC_DEBUG_LOG

  Description:  WXC ライブラリ内部のデバッグ出力関数.
                この関数は SDK_WEAK_SYMBOL で修飾されており,
                標準では OS_TPrintf と等価な処理になります.
                アプリケーション固有のデバッグシステムなどで
                この同名関数をオーバーライドすることができます.

  Arguments:    format        デバッグ出力内容を示す書式文字列
                ...           可変引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if !defined(WXC_DEBUG_LOG)
SDK_WEAK_SYMBOL void WXC_DEBUG_LOG(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    OS_TVPrintf(format, va);
    va_end(va);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmApiName

  Description:  WM 関数の名前文字列を取得

  Arguments:    id          WM 関数の種類を示す WMApiid 列挙値

  Returns:      WM 関数の名前文字列.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmApiName(WMApiid id)
{
    const char *ret = "";
    static const char *(api_name_table[]) =
    {
    "WM_Initialize",
            "WM_Reset",
            "WM_End",
            "WM_Enable",
            "WM_Disable",
            "WM_PowerOn",
            "WM_PowerOff",
            "WM_SetParentParameter",
            "WM_StartParent",
            "WM_EndParent",
            "WM_StartScan",
            "WM_EndScan",
            "WM_StartConnect",
            "WM_Disconnect",
            "WM_StartMP",
            "WM_SetMPData",
            "WM_EndMP",
            "WM_StartDCF",
            "WM_SetDCFData",
            "WM_EndDCF",
            "WM_SetWEPKey",
            "WM_StartKeySharing",
            "WM_EndKeySharing",
            "WM_GetKeySet",
            "WM_SetGameInfo",
            "WM_SetBeaconIndication",
            "(WM_StartTestMode)",
            "(WM_StopTestMode)",
            "(W-Alarm ind. in ARM7)",
            "WM_SetLifeTime",
            "WM_MeasureChannel",
            "WM_InitWirelessCounter",
            "WM_GetWirelessCounter",
            "WM_SetEntry",
            "(auto-deauthentication in ARM7)",
            "WM_SetMPFrequency",
            "WMi_SetBeaconPeriod",
            "(auto-disconnection in ARM7)",
            "WM_StartScanEx",
            "WM_SetWEPKeyEx",
            "WM_SetPowerSaveMode",
            "WM_StartTestRxMode",
            "WM_StopTestRxMode",
            "(MP-timing controller in ARM7)",
            "(MP-timing controller in ARM7)", "(MP-timing controller in ARM7)",}
    ;
    /* ここでコンパイルエラーとなる場合, wm.h の定義をもとに更新すべき */
    SDK_STATIC_ASSERT(WM_APIID_ASYNC_KIND_MAX <= sizeof(api_name_table) / sizeof(*api_name_table));
    if (id < WM_APIID_ASYNC_KIND_MAX)
    {
        ret = api_name_table[id];
    }
    else if (id == WM_APIID_INDICATION)
    {
        ret = "WM_SetIndCallback";
    }
    else if (id == WM_APIID_PORT_SEND)
    {
        ret = "PortSendCallback";
    }
    else if (id == WM_APIID_PORT_RECV)
    {
        ret = "PortRecvCallback";
    }
    else if (id == WM_APIID_READ_STATUS)
    {
        ret = "WM_ReadStatus";
    }
    else
    {
        ret = "(unknown)";
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmErrorName

  Description:  WM エラーコードの名前文字列を取得

  Arguments:    err         WM エラーコードを示す WMErrCode 列挙値

  Returns:      WM エラーコードの名前文字列.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmErrorName(WMErrCode err)
{
    const char *ret = "";
    static const char *(error_name_table[]) =
    {
    "WM_ERRCODE_SUCCESS",
            "WM_ERRCODE_FAILED",
            "WM_ERRCODE_OPERATING",
            "WM_ERRCODE_ILLEGAL_STATE",
            "WM_ERRCODE_WM_DISABLE",
            "WM_ERRCODE_NO_DATASET",
            "WM_ERRCODE_INVALID_PARAM",
            "WM_ERRCODE_NO_CHILD",
            "WM_ERRCODE_FIFO_ERROR",
            "WM_ERRCODE_TIMEOUT",
            "WM_ERRCODE_SEND_QUEUE_FULL",
            "WM_ERRCODE_NO_ENTRY",
            "WM_ERRCODE_OVER_MAX_ENTRY",
            "WM_ERRCODE_INVALID_POLLBITMAP",
            "WM_ERRCODE_NO_DATA",
            "WM_ERRCODE_SEND_FAILED",
            "WM_ERRCODE_DCF_TEST",
            "WM_ERRCODE_WL_INVALID_PARAM", "WM_ERRCODE_WL_LENGTH_ERR", "WM_ERRCODE_FLASH_ERROR",}
    ;
    /* ここでコンパイルエラーとなる場合, wm.h の定義をもとに更新すべき */
    SDK_STATIC_ASSERT(WM_ERRCODE_MAX == sizeof(error_name_table) / sizeof(*error_name_table));
    if (err < WM_ERRCODE_MAX)
    {
        ret = error_name_table[err];
    }
    else
    {
        ret = "(unknown WMErrCode)";
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmApiResult

  Description:  WM 関数の呼び出し返り値を判定し,
                エラーであれば詳細をデバッグ出力.

  Arguments:    id          WM 関数の種類
                err         関数から返されたエラーコード

  Returns:      正当な返り値であれば TRUE, そうでなければ FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_CheckWmApiResult(WMApiid id, WMErrCode err)
{
#pragma unused(id)
    BOOL    ret = TRUE;
    if (err == WM_ERRCODE_SUCCESS)
    {
        WXC_DEBUG_LOG(">< %s succeeded.\n", WXC_GetWmApiName(id));
    }
    else if (err == WM_ERRCODE_OPERATING)
    {
        WXC_DEBUG_LOG(">  %s started.\n", WXC_GetWmApiName(id));
    }
    else
    {
        WXC_DEBUG_LOG(">< %s failed. %s\n", WXC_GetWmApiName(id), WXC_GetWmErrorName(err));
        ret = FALSE;
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmCallbackResult

  Description:  WM コールバックの返り値を判定し,
                エラーであれば詳細をデバッグ出力.

  Arguments:    arg         WM 関数のコールバックへ返された引数

  Returns:      正当な結果値であれば TRUE, そうでなければ FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_CheckWmCallbackResult(void *arg)
{
    const WMCallback *cb = (const WMCallback *)arg;
    const WMApiid id = (WMApiid)cb->apiid;
    const WMErrCode err = (WMErrCode)cb->errcode;
    BOOL    ret = (cb->errcode == WM_ERRCODE_SUCCESS);

    /* 成功の場合は, confirm のみ結果表示 */
    if (ret)
    {
        if ((id != WM_APIID_INDICATION) &&
            ((id != WM_APIID_START_MP)
             || (((WMStartMPCallback *)arg)->state == WM_STATECODE_MP_START))
            && ((id != WM_APIID_START_CONNECT)
                || (((WMStartConnectCallback *)arg)->state == WM_STATECODE_CONNECT_START)))
        {
            WXC_DRIVER_LOG(" < %s succeeded.\n", WXC_GetWmApiName(id));
        }
    }
    /* エラーのうち, しばしば起こりうる警告的エラーは無視する */
    else
    {
        switch (id)
        {
        case WM_APIID_START_MP:
            switch (err)
            {
            case WM_ERRCODE_SEND_FAILED:
            case WM_ERRCODE_TIMEOUT:
            case WM_ERRCODE_INVALID_POLLBITMAP:
                ret = TRUE;
                break;
            }
            break;
        }
        /* 本質的なエラーの場合にのみ結果表示 */
        if (!ret)
        {
            WXC_DRIVER_LOG(" < %s failed. %s(cmd=%02X,res=%02X)\n",
                           WXC_GetWmApiName(id), WXC_GetWmErrorName(err),
                           cb->wlCmdID, cb->wlResult);
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetNextAllowedChannel

  Description:  指定されたチャンネルの次に使用可能なチャンネルを取得.
                最上位チャンネルの次は最下位チャンネルへループする.
                0 を指定すれば最下位のチャンネルを取得.

  Arguments:    ch            今回のチャンネル位置 (1から14, あるいは0)

  Returns:      次に使用可能なチャンネル. (1から14)
 *---------------------------------------------------------------------------*/
u16 WXC_GetNextAllowedChannel(int ch)
{
    int     allowed = WM_GetAllowedChannel();
    if (!allowed)
    {
        OS_TPanic("no allowed channels to scan!");
    }
    else if (allowed == 0x8000)
    {
        OS_TPanic("WM_GetAllowedChannel() failed! (WM might have not been initialized)");
    }
    else
    {
        ++ch;
        while (((1 << (ch - 1)) & allowed) == 0)
        {
            if (++ch > 16)
            {
                ch = 1;
            }
        }
    }
    return (u16)ch;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
