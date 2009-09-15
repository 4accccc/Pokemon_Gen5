/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wmDEMOlib
  File:     wm_lib.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

// 2005/05/26 update by terui@nintendo
#include <nitro/wm.h>
#include "wm_lib.h"
#include <stdlib.h>
#include <string.h>

void    mprintf(const char *fmt, ...);

#undef WMLIB_DEBUG                     // デバッグ出力

#define CAM_LIFE_TIME       40
#define FRAME_LIFE_TIME     5
#define TABLE_NO          0xffff

#undef  NO_LIFETIME

#define STARTCONNECT_EX_TEST	1      // StartConnectEx実験

static BOOL WM_lib_auto_flag = FALSE;
static BOOL WM_lib_mp_dcf_parallel = FALSE;


static WM_lib_param *param = NULL;     // パラメータ構造体へのポインタ

static void WM_lib_mp_parent_callback(void *arg);       // MP 親機モード コールバック
static void WM_lib_mp_child_callback(void *arg);        // MP 子機モード コールバック
static void WM_lib_dcf_child_callback(void *arg);       // DCF 子機モード コールバック
static void WM_lib_dummy(void *arg);   // ダミーコールバック
static void changeScanChannel(WMscanParam *param);      // Scanチャンネル変更関数

static void (*callback_ptr) (void *arg);

static const u8 test_ssid[WM_SIZE_CHILD_SSID] = "TEST SSID\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";      // 24Byte

#if 0
#define CHK_ERRCODE(x,y)    if ( x != WM_ERRCODE_OPERATING &&           \
                                    x != WM_ERRCODE_SUCCESS)            \
                            {                                           \
                                param->callback( WM_TYPE_ERROR, y); \
                            }
#else
static void check_WMAPI_errcode(u16 apiid, int errcode);
#endif

static inline int conv_errcode(int errcode)
{
    return (errcode == WM_ERRCODE_OPERATING) ? WM_ERRCODE_SUCCESS : errcode;
}

static WMscanParam scanParam ATTRIBUTE_ALIGN(32);
static WMscanExParam scanExParam ATTRIBUTE_ALIGN(32);
static u16 scanning_flag = 0;

static BOOL WM_lib_change_scan_channel_flag = TRUE;

static u16 maxChannelTime_org;

//--------------------------------------------------------
// WMの自動親機子機登録機能フラグ ＯＮ
void WM_lib_set_auto(BOOL flag)
{
    WM_lib_auto_flag = flag;
}
void WM_lib_set_mp_dcf_parallel(BOOL flag)
{
    WM_lib_mp_dcf_parallel = flag;
}

// MaxScanTime設定
void WM_lib_set_max_scan_time(u16 time)
{
    scanParam.maxChannelTime = time;
    maxChannelTime_org = time;
}

// MaxScanTime取得
u16 WM_lib_get_max_scan_time(void)
{
    return scanParam.maxChannelTime;
}

// ScanChannel設定
void WM_lib_set_channel(u16 channel)
{
    if (channel == 0)
    {
        WM_lib_set_scan_channel_seek(TRUE);
        scanParam.channel = 1;
    }
    else
    {
        WM_lib_set_scan_channel_seek(FALSE);
        scanParam.channel = channel;
    }
}

// Scan時のMacAddress(BSSID)フィルタリングに用いるMacAddress設定
void WM_lib_set_scanBssid(u8 *bssid)
{
    MI_CpuCopy8(bssid, scanParam.bssid, WM_SIZE_MACADDR);
}

//-------------------------------------------------------------
// StartScanEx用パラメータ設定関数

// ScanEx用 maxChannelTime設定
void WM_lib_set_scanEx_maxChannelTime(u16 time)
{
    scanExParam.maxChannelTime = time;
    maxChannelTime_org = time;
}

// ScanEx用 maxChannelTime取得
u16 WM_lib_get_scanEx_maxChannelTime(void)
{
    return scanExParam.maxChannelTime;
}

// ScanEx用 Channel設定
void WM_lib_set_scanEx_channelList(u16 channelList)
{
    // channel は、ビットマップであることに注意
    // 0x1041なら、1,7,13ch
    WM_lib_set_scan_channel_seek(FALSE);
    scanExParam.channelList = channelList;
}

// ScanEx用 Channel取得
u16 WM_lib_get_scanEx_channelList(void)
{
    return scanExParam.channelList;
}

// ScanEx用 ScanType(passive or active)設定
void WM_lib_set_scanEx_scanType(u16 scanType)
{
    scanExParam.scanType = scanType;
}

// ScanEx用 ScanType(passive or active)取得
// WM_SCANTYPE_ACTIVE(0), WM_SCANTYPE_PASSIVE(1)
u16 WM_lib_get_scanEx_scanType(void)
{
    return scanExParam.scanType;
}

// ScanEx用 SSIDフィルタリング(SSID長)設定
void WM_lib_set_scanEx_ssidLength(u16 ssidLength)
{
    scanExParam.ssidLength = ssidLength;
}

// ScanEx用 SSIDフィルタリングに用いるSSID設定
void WM_lib_set_scanEx_ssid(u8 *ssid)
{
    if (ssid == NULL)
    {
        MI_CpuClear8(scanExParam.ssid, WM_SIZE_SSID);
    }
    else
    {
        MI_CpuCopy8(ssid, scanExParam.ssid, WM_SIZE_SSID);
    }
}

// ScanEx用 MacAddress(BSSID)フィルタリングに用いるMacAddress設定
void WM_lib_set_scanEx_bssid(u8 *bssid)
{
    MI_CpuCopy8(bssid, scanExParam.bssid, WM_SIZE_MACADDR);
}


//--------------------------------------------------------
// WMの初期化処理
int WM_lib_Init(WM_lib_param * para)
{
    int     errcode;
    param = para;

    param->mpStarted = 0;
    param->mpBusy = 0;
    param->child_bitmap = 0;
    param->currentTgid = 0;

    param->endReq = 0;
    WM_lib_mp_dcf_parallel = FALSE;

    switch (param->mode)
    {
    case WM_MODE_MP_PARENT:
        callback_ptr = WM_lib_mp_parent_callback;
        break;
    case WM_MODE_MP_CHILD:
        callback_ptr = WM_lib_mp_child_callback;
        break;
    case WM_MODE_INFRASTRUCTURE:
        callback_ptr = WM_lib_dcf_child_callback;
        break;
    default:
        callback_ptr = WM_lib_dummy;
        return WM_ERRCODE_FAILED;
    }

    do
    {
        errcode = WM_Initialize(param->wmBuf, callback_ptr, WM_DMA_NO);
    }
    while (errcode == WM_ERRCODE_WM_DISABLE);
    if (errcode != WM_ERRCODE_OPERATING)
    {
        return WM_ERRCODE_FAILED;
    }
    else
    {
        return WM_SetIndCallback(callback_ptr);
    }
}

//=================================================================
// ダミーコールバック
static void WM_lib_dummy(void *arg)
{
    WMCallback *buf = (WMCallback *)arg;
}

//=================================================================
// MP親機のコールバック
void WM_lib_mp_parent_callback(void *arg)
{
    int     errcode;
    WMCallback *buf = (WMCallback *)arg;

    switch (buf->apiid)
    {
    case WM_APIID_START_PARENT:
        {
            WMstartParentCallback *callback = (WMstartParentCallback *)arg;

#ifdef	WMLIB_DEBUG
            OS_Printf("StartParent callback\n");
            OS_Printf("      state:%d\n", callback->state);
#endif

            if (WM_ERRCODE_SUCCESS == callback->errcode)
            {
                switch (callback->state)
                {
                case WM_STATECODE_PARENT_START:
                    param->callback(WM_TYPE_PARENT_STARTED, arg);
                    break;

                case WM_STATECODE_CONNECTED:
                    errcode = WM_ReadStatus(param->statusBuf);
                    check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);

                    // ステータス読み出し
                    param->child_bitmap = param->statusBuf->child_bitmap;

                    if (WM_lib_auto_flag == TRUE)
                    {
                        wm_lib_add_child_list(callback);
                    }

                    param->callback(WM_TYPE_CHILD_CONNECTED, arg);

                    if (param->statusBuf->mp_flag == 0) // MP開始前ならMP開始
                    {
                        errcode = WM_StartMPEx(WM_lib_mp_parent_callback, param->recvBuf, param->recvBufSize, param->sendBuf, param->sendBufSize, (u16)(param->contSend ? 0 : 1), 0, FALSE, FALSE, param->no_retry,     // デフォルトFALSE
                                               param->contSend ? TRUE : FALSE   // ignoreFatalError
                            );

                        check_WMAPI_errcode(WM_APIID_START_MP, errcode);
                    }
                    // MP送信許可コールバック
                    if (TRUE == WM_lib_CheckMPSend())
                    {
                        param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
                    }
                    break;

                case WM_STATECODE_DISCONNECTED:
                    errcode = WM_ReadStatus(param->statusBuf);  // ステータス読み出し
                    check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);

                    param->child_bitmap = param->statusBuf->child_bitmap;

                    if (WM_lib_auto_flag == TRUE)
                    {
                        wm_lib_delete_child_list(callback);
                    }
                    param->callback(WM_TYPE_CHILD_DISCONNECTED, arg);
                    break;

                case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
                    // 自ら切断した場合は処理を行いません
                    break;

                case WM_STATECODE_BEACON_SENT: // ビーコン送信完了通知
                    param->callback(WM_TYPE_BEACON_SENT, arg);
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

    case WM_APIID_SET_LIFETIME:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                param->callback(WM_TYPE_SET_LIFETIME, arg);

                // APIの発行
                errcode = WM_SetParentParameter(WM_lib_mp_parent_callback, param->parentParam);
                check_WMAPI_errcode(WM_APIID_SET_P_PARAM, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

    case WM_APIID_START_MP:
        {
            // ここではエラーコードは返ってこない
            WMstartMPCallback *callback = (WMstartMPCallback *)arg;

#ifdef	WMLIB_DEBUG
            OS_Printf("StartMP callback\n");
            OS_Printf("     state:%d\n", callback->state);
#endif
            switch (callback->state)
            {
            case WM_STATECODE_MP_START:
                param->mpStarted = 1;  // MP開始済みフラグをセット
                param->callback(WM_TYPE_MP_STARTED, arg);

                if (param->keySharing == 1)
                {
                    // KeySharing開始
                    errcode = WM_StartKeySharing(param->ksBuf, WM_PORT_KEYSHARING);
                    check_WMAPI_errcode(WM_APIID_START_KS, errcode);
                    if (TRUE == WM_lib_CheckMPSend())
                        param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
                }
                else
                {
                    // MP送信許可コールバック
                    if (TRUE == WM_lib_CheckMPSend())
                    {
                        param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
                        if (WM_lib_mp_dcf_parallel == TRUE)
                        {
                            errcode =
                                WM_StartDCF(WM_lib_mp_parent_callback, param->dcfBuf,
                                            param->dcfBufSize);
                            check_WMAPI_errcode(WM_APIID_START_DCF, errcode);
                        }
                    }
                }
                break;

            case WM_STATECODE_MPEND_IND:
                // MP受信コールバック
                param->callback(WM_TYPE_MP_PARENT_RECV, (void *)(callback->recvBuf));
                break;

            default:
                param->callback(WM_TYPE_ERROR, arg);
                break;
            }
            break;
        }
        break;

    case WM_APIID_START_DCF:
        if (WM_ERRCODE_SUCCESS == buf->errcode)
        {
#if 0
#define WM_TYPE_DCF_STARTED
#define WM_TYPE_DCF_SENT
#define WM_TYPE_DCF_SENT_ERR
#define WM_TYPE_DCF_RECV
#define WM_TYPE_DCF_END
#endif
            param->callback(WM_TYPE_DCF_STARTED, arg);
        }
        else
        {
            param->callback(WM_TYPE_ERROR, arg);
        }
        break;

    case WM_APIID_SET_MP_DATA:
        {
            param->mpBusy = 0;

            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_MP_PARENT_SENT, arg);
                param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);  // 次の送信を許可
            }
            else if (buf->errcode == WM_ERRCODE_SEND_QUEUE_FULL)
            {
                param->callback(WM_TYPE_SEND_QUEUE_FULL_ERR, arg);
            }
            else
            {
                param->callback(WM_TYPE_MP_PARENT_SENT_ERR, arg);
                param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);  // 次の送信を許可
            }
        }
        break;

    case WM_APIID_INITIALIZE:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                errcode = WM_ReadStatus(param->statusBuf);
                check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);
                // ステータス読み出し
                // APIの発行
                errcode = WM_SetIndCallback(WM_lib_mp_parent_callback);
                check_WMAPI_errcode(WM_APIID_INDICATION, errcode);
#ifdef NO_LIFETIME
                errcode =
                    WM_SetLifeTime(WM_lib_mp_parent_callback, TABLE_NO, 0xffff, 0xffff, 0xffff);
#else
                errcode =
                    WM_SetLifeTime(WM_lib_mp_parent_callback, TABLE_NO, CAM_LIFE_TIME,
                                   FRAME_LIFE_TIME, CAM_LIFE_TIME);
#endif
                check_WMAPI_errcode(WM_APIID_SET_LIFETIME, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

#if 1
    case WM_APIID_SET_P_PARAM:
        {
            param->callback(WM_TYPE_INIT_COMPLETE, arg);
            // ここではエラーコードは返ってこない
            errcode = WM_SetBeaconIndication(WM_lib_mp_parent_callback, 1 /* 1:ON, 0:OFF */ );
            check_WMAPI_errcode(WM_APIID_SET_BEACON_IND, errcode);
        }
        break;
        // Beacon Send/Recv Indicationを発生させるかどうか
    case WM_APIID_SET_BEACON_IND:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                errcode = WM_StartParent(WM_lib_mp_parent_callback);
                check_WMAPI_errcode(WM_APIID_START_PARENT, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;
#else
    case WM_APIID_SET_P_PARAM:
        {
            param->callback(WM_TYPE_INIT_COMPLETE, arg);
            // ここではエラーコードは返ってこない
            errcode = WM_StartParent(WM_lib_mp_parent_callback);
            check_WMAPI_errcode(WM_APIID_START_PARENT, errcode);
        }
        break;
#endif

    case WM_APIID_RESET:
        {
#ifdef	WMLIB_DEBUG
            OS_Printf("Reset callback\n");
            OS_Printf("      errcode:%d\n", buf->errcode);
#endif
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                errcode = WM_End(WM_lib_mp_parent_callback);
                check_WMAPI_errcode(WM_APIID_END, errcode);
            }
            else
            {
                param->endReq = 0;
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

    case WM_APIID_END:
        {
            param->endReq = 0;
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_END_COMPLETE, arg);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

    case WM_APIID_DISCONNECT:
        {
            WMDisconnectCallback *cb = (WMDisconnectCallback *)buf;
#ifdef WMLIB_DEBUG
            OS_Printf("Disconnect callback\n");
            OS_Printf("    cb->errcode            = %d\n", cb->errcode);
            OS_Printf("    cb->wlCmdID            = %d\n", cb->wlCmdID);
            OS_Printf("    cb->wlResult           = %d\n", cb->wlResult);
            OS_Printf("    cb->tryBitmap          = %d\n", cb->tryBitmap);
            OS_Printf("    cb->disconnectedBitmap = %d\n", cb->disconnectedBitmap);
#endif
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                // 子機情報を更新
                errcode = WM_ReadStatus(param->statusBuf);
                check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);
                param->child_bitmap = param->statusBuf->child_bitmap;

                if (WM_lib_auto_flag == TRUE)
                {
                    wm_lib_delete_child_list((WMstartParentCallback *)arg);
                }
                param->callback(WM_TYPE_DISCONNECT_COMPLETE, arg);
            }
            else
            {
                param->callback(WM_TYPE_DISCONNECT_FAILED, arg);
            }
        }
        break;

    case WM_APIID_SET_GAMEINFO:
        {
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_SET_GAMEINFO_COMPLETE, arg);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

    case WM_APIID_END_MP:
        {
            param->callback(WM_TYPE_END_MP_COMPLETE, arg);
        }
        break;

    case WM_APIID_INDICATION:
        {
            WMindCallback *cb = (WMindCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_BEACON_RECV:     // Beacon受信indicate
                param->callback(WM_TYPE_BEACON_RECV, arg);
                break;
            case WM_STATECODE_DISASSOCIATE:    // 接続切断indicate
                param->callback(WM_TYPE_DISASSOCIATE, arg);
                break;
            case WM_STATECODE_REASSOCIATE:     // 再接続indicate
                param->callback(WM_TYPE_REASSOCIATE, arg);
                break;
            case WM_STATECODE_AUTHENTICATE:    // 認証確認indicate
                param->callback(WM_TYPE_AUTHENTICATE, arg);
                break;
            }
        }
        break;

    default:
        param->callback(WM_TYPE_ERROR, arg);
        break;

    }
}

//=================================================================
// MP子機のコールバック
void WM_lib_mp_child_callback(void *arg)
{
    WMCallback *buf = (WMCallback *)arg;
    int     errcode;

    switch (buf->apiid)
    {
        //---------------------------------------------------------------
    case WM_APIID_INITIALIZE:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
#ifdef WMLIB_DEBUG
                OS_TPrintf("mp C: WM_Initialize callback\n");
#endif
                param->callback(WM_TYPE_INIT_COMPLETE, arg);
                {
                    errcode = WM_ReadStatus(param->statusBuf);  // ステータス読み出し
                    check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);

                    // APIの発行
                    errcode = WM_SetIndCallback(WM_lib_mp_child_callback);
                    check_WMAPI_errcode(WM_APIID_INDICATION, errcode);
#ifdef NO_LIFETIME
                    errcode =
                        WM_SetLifeTime(WM_lib_mp_child_callback, TABLE_NO, 0xffff, 0xffff, 0xffff);
#else
                    errcode =
                        WM_SetLifeTime(WM_lib_mp_child_callback, TABLE_NO, CAM_LIFE_TIME,
                                       FRAME_LIFE_TIME, CAM_LIFE_TIME);
#endif
                    check_WMAPI_errcode(WM_APIID_SET_LIFETIME, errcode);
                }
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_SET_LIFETIME:
        {
#ifdef WMLIB_DEBUG
            OS_TPrintf("mp C: WM_SetLifeTime callback\n");
#endif
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                param->callback(WM_TYPE_SET_LIFETIME, arg);

                // WM_StartScan()発行
                if (param->scanExFlag == TRUE)
                {
                    // WM_StartScanEx使用時
                    scanExParam.scanBuf = param->parentInfoBuf;
                    scanExParam.scanBufSize = param->parentInfoBufSize;

                    if (scanExParam.maxChannelTime == 0)
                        WM_lib_set_scanEx_maxChannelTime(200);

                    scanExParam.bssid[0] = 0xff;
                    scanExParam.bssid[1] = 0xff;
                    scanExParam.bssid[2] = 0xff;
                    scanExParam.bssid[3] = 0xff;
                    scanExParam.bssid[4] = 0xff;
                    scanExParam.bssid[5] = 0xff;

                    scanning_flag = 1;
                    errcode = WM_StartScanEx(WM_lib_mp_child_callback, &scanExParam);
                    check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                }
                else
                {
                    // WM_StartScan使用時
                    scanParam.scanBuf = param->parentInfoBuf;

                    if (scanParam.channel == 0)
                        scanParam.channel = 1;
                    if (scanParam.maxChannelTime == 0)
                        WM_lib_set_max_scan_time(200);

                    scanParam.bssid[0] = 0xff;
                    scanParam.bssid[1] = 0xff;
                    scanParam.bssid[2] = 0xff;
                    scanParam.bssid[3] = 0xff;
                    scanParam.bssid[4] = 0xff;
                    scanParam.bssid[5] = 0xff;

                    scanning_flag = 1;
                    errcode = WM_StartScan(WM_lib_mp_child_callback, &scanParam);
                    check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                }
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_SCAN:
        {
            WMstartScanCallback *callback = (WMstartScanCallback *)arg;

#ifdef WMLIB_DEBUG
            OS_TPrintf("mp C: WM_StartScan callback\n");
#endif
            if (WM_ERRCODE_SUCCESS == callback->errcode)
            {
                switch (callback->state)
                {
                case WM_STATECODE_SCAN_START:
                    break;

                case WM_STATECODE_PARENT_FOUND:
                    if (WM_lib_auto_flag)
                    {                  // wm_tool.cの自動化を使う場合
                        if (WM_IsValidGameInfo
                            (&((WMstartScanCallback *)arg)->gameInfo,
                             ((WMstartScanCallback *)arg)->gameInfoLength))
                        {
                            wm_lib_add_parent_list((WMstartScanCallback *)arg);
                        }
                    }

                    param->callback(WM_TYPE_PARENT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        if (WM_lib_change_scan_channel_flag == TRUE)
                            changeScanChannel(&scanParam);

                        errcode = WM_StartScan(WM_lib_mp_child_callback, &scanParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                    }
                    break;

                case WM_STATECODE_PARENT_NOT_FOUND:
                    if (WM_lib_auto_flag)
                    {                  // wm_tool.cの自動化を使う場合
                        wm_lib_delete_parent_list((WMstartScanCallback *)arg);
                    }
                    param->callback(WM_TYPE_PARENT_NOT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        if (WM_lib_change_scan_channel_flag == TRUE)
                            changeScanChannel(&scanParam);

                        errcode = WM_StartScan(WM_lib_mp_child_callback, &scanParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                    }
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_SCAN_EX:
        {
            WMstartScanExCallback *callback = (WMstartScanExCallback *)arg;

#ifdef WMLIB_DEBUG
            OS_TPrintf("mp C: WM_StartScanEx callback\n");
#endif
            if (WM_ERRCODE_SUCCESS == callback->errcode)
            {
                switch (callback->state)
                {
                case WM_STATECODE_SCAN_START:
                    break;

                case WM_STATECODE_PARENT_FOUND:
                    if (WM_lib_auto_flag)
                    {                  // wm_tool.cの自動化を使う場合
                        WMBssDesc *p_bssDesc;
                        u16     bssDescCount;
                        int     i;

                        bssDescCount = ((WMStartScanExCallback *)arg)->bssDescCount;
                        for (i = 0; i < bssDescCount; ++i)
                        {
                            p_bssDesc = ((WMStartScanExCallback *)arg)->bssDesc[i];

                            if (WM_IsValidGameInfo(&p_bssDesc->gameInfo, p_bssDesc->gameInfoLength))
                            {
                                wm_lib_add_parent_listEx((WMstartScanExCallback *)arg);
                            }
                        }
                    }

                    param->callback(WM_TYPE_PARENT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        errcode = WM_StartScanEx(WM_lib_mp_child_callback, &scanExParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                    }
                    break;

                case WM_STATECODE_PARENT_NOT_FOUND:
                    param->callback(WM_TYPE_PARENT_NOT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        errcode = WM_StartScanEx(WM_lib_mp_child_callback, &scanExParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                    }
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_END_SCAN:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                errcode = WM_StartConnect(WM_lib_mp_child_callback, param->pInfo, test_ssid);
                //errcode = WM_StartConnect( WM_lib_mp_child_callback, param->pInfo, NULL );
                check_WMAPI_errcode(WM_APIID_START_CONNECT, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_CONNECT:
        {
            WMstartConnectCallback *callback = (WMstartConnectCallback *)arg;

            if (callback->errcode == WM_ERRCODE_SUCCESS)
            {
                switch (callback->state)
                {
                case WM_STATECODE_CONNECT_START:
                    break;

                case WM_STATECODE_CONNECTED:
                    if (WM_lib_auto_flag == TRUE)
                    {
                        wm_lib_set_my_aid(callback->aid);
                    }
                    param->callback(WM_TYPE_CONNECTED_TO_PARENT, arg);
                    param->child_bitmap = 1;
#if 0
                    OS_Printf("WM_GetMPReceiveBufferSize:%d\n", WM_GetMPReceiveBufferSize());

                    OS_Printf("WM_GetMPSendBufferSize   :%d\n", WM_GetMPSendBufferSize());
#endif
                    errcode = WM_StartMPEx(WM_lib_mp_child_callback, param->recvBuf, param->recvBufSize, param->sendBuf, param->sendBufSize, (u16)(param->contSend ? 0 : 1), 0, FALSE, FALSE, param->no_retry,  // デフォルトFALSE
                                           param->contSend ? TRUE : FALSE       // ignoreFatalError
                        );


                    check_WMAPI_errcode(WM_APIID_START_MP, errcode);
                    break;

                case WM_STATECODE_DISCONNECTED:
                    param->callback(WM_TYPE_DISCONNECTED_FROM_PARENT, arg);
                    param->child_bitmap = 0;
                    break;

                case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
                    // 自ら切断した場合は処理を行いません
                    break;

                case WM_STATECODE_BEACON_LOST:
                    param->callback(WM_TYPE_BEACON_LOST, arg);
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
            else
            {
                // コールバックでエラー通知
                if (WM_lib_auto_flag == TRUE)
                {
                    wm_lib_parent_found_count_reset();
                }
                param->callback(WM_TYPE_CONNECT_FAILED, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_MP:
        {
            WMstartMPCallback *callback = (WMstartMPCallback *)arg;

            switch (callback->state)
            {
            case WM_STATECODE_MP_START:
                param->mpStarted = 1;  // MP開始済みフラグをセット
                param->callback(WM_TYPE_MP_STARTED, arg);

                if (param->keySharing == 1)
                {
                    // KeySharing開始
                    errcode = WM_StartKeySharing(param->ksBuf, WM_PORT_KEYSHARING);
                    check_WMAPI_errcode(WM_APIID_START_KS, errcode);
                    if (TRUE == WM_lib_CheckMPSend())
                        param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
                }
                else
                {
                    // MP送信許可コールバック
                    if (TRUE == WM_lib_CheckMPSend())
                    {
                        param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
                        if (WM_lib_mp_dcf_parallel == TRUE)
                        {
                            errcode =
                                WM_StartDCF(WM_lib_mp_child_callback, param->dcfBuf,
                                            param->dcfBufSize);
                            check_WMAPI_errcode(WM_APIID_START_DCF, errcode);
                        }
                    }
                }
                break;

            case WM_STATECODE_MP_IND:
                if (callback->errcode == WM_ERRCODE_INVALID_POLLBITMAP)
                {
                    // 自分はポールされていないが、MPフレームは受信しているのでそのまま通知
                    param->callback(WM_TYPE_MP_CHILD_RECV, (void *)(callback->recvBuf));
                }
                else
                {
                    param->callback(WM_TYPE_MP_CHILD_RECV, (void *)(callback->recvBuf));
                }
                break;

            case WM_STATECODE_MPACK_IND:
                break;

            default:
                param->callback(WM_TYPE_ERROR, arg);
                break;
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_DCF:
        if (WM_ERRCODE_SUCCESS == buf->errcode)
        {
            param->callback(WM_TYPE_DCF_STARTED, arg);
        }
        else
        {
            param->callback(WM_TYPE_ERROR, arg);
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_SET_MP_DATA:
        {
            param->mpBusy = 0;

            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_MP_CHILD_SENT, arg);
            }
            else if (buf->errcode == WM_ERRCODE_TIMEOUT)
            {
                param->callback(WM_TYPE_MP_CHILD_SENT_TIMEOUT, arg);
            }
            else
            {
                param->callback(WM_TYPE_MP_CHILD_SENT_ERR, arg);
            }

            // 次の送信を許可
            param->callback(WM_TYPE_MP_SEND_ENABLE, NULL);
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_RESET:
        {
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                errcode = WM_End(WM_lib_mp_child_callback);
                check_WMAPI_errcode(WM_APIID_END, errcode);
            }
            else
            {
                param->endReq = 0;
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_END:
        {
            param->endReq = 0;
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_END_COMPLETE, arg);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_INDICATION:
        {
            WMindCallback *cb = (WMindCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_BEACON_RECV:     // Beacon受信indicate
                {
                    static u16 flag = 0;        // 2回続けてここに来たとき、WM_lib_Endの二重発行を防ぐ
                    WMBeaconRecvIndCallback *callback = (WMBeaconRecvIndCallback *)arg;
                    if ((param->currentTgid != 0) && (callback->gameInfo.tgid != param->currentTgid)
                        && (callback->wmstate == WM_STATE_MP_CHILD))
                    {
                        if (flag == 0)
                        {
                            int     errcode;
                            flag = 1;
                            errcode = WM_lib_End();
                            check_WMAPI_errcode(WM_APIID_END, errcode);
                        }
                    }
                    else
                    {
                        flag = 0;
                    }
                    param->callback(WM_TYPE_BEACON_RECV, arg);
                }
                break;
            case WM_STATECODE_DISASSOCIATE:    // 接続切断indicate
                param->callback(WM_TYPE_DISASSOCIATE, arg);
                break;
            case WM_STATECODE_REASSOCIATE:     // 再接続indicate
                param->callback(WM_TYPE_REASSOCIATE, arg);
                break;
            case WM_STATECODE_AUTHENTICATE:    // 認証確認indicate
                param->callback(WM_TYPE_AUTHENTICATE, arg);
                break;
            }
        }
        break;

    default:
        param->callback(WM_TYPE_ERROR, arg);
        break;
    }
}

//========================================================================================

//=================================================================
// DCF子機のコールバック
void WM_lib_dcf_child_callback(void *arg)
{
    WMCallback *buf = (WMCallback *)arg;
    int     errcode;

    switch (buf->apiid)
    {
        //---------------------------------------------------------------
    case WM_APIID_INITIALIZE:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
#ifdef WMLIB_DEBUG
                OS_TPrintf("dcf: WM_Initialize callback\n");
#endif
                param->callback(WM_TYPE_INIT_COMPLETE, arg);

                errcode =
                    WM_SetLifeTime(WM_lib_dcf_child_callback, TABLE_NO, 0xffff, 0xffff, 0xffff);
                check_WMAPI_errcode(WM_APIID_SET_LIFETIME, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_SET_LIFETIME:
        {
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
#ifdef WMLIB_DEBUG
                OS_TPrintf("dcf: WM_SetLifeTime callback\n");
#endif

                errcode = WM_SetWEPKeyEx(WM_lib_dcf_child_callback, param->wepMode,
                                         param->wepKeyId, param->wepKey);
                check_WMAPI_errcode(WM_APIID_SET_WEPKEY_EX, errcode);
            }
            else
            {
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_SET_WEPKEY:
    case WM_APIID_SET_WEPKEY_EX:
        {
#ifdef WMLIB_DEBUG
            OS_TPrintf("dcf: WM_SetWEPKey(Ex) callback\n");
#endif
            if (WM_ERRCODE_SUCCESS == buf->errcode)
            {
                // ステータス読み出し
                errcode = WM_ReadStatus(param->statusBuf);
                check_WMAPI_errcode(WM_APIID_READ_STATUS, errcode);

                // WM_SetIncCallback発行
                errcode = WM_SetIndCallback(WM_lib_dcf_child_callback);
                check_WMAPI_errcode(WM_APIID_INDICATION, errcode);

                // Scan開始
                if (param->scanExFlag == TRUE)
                {
                    // WM_StartScanEx使用時
                    scanExParam.scanBuf = param->parentInfoBuf;
                    scanExParam.scanBufSize = param->parentInfoBufSize;

                    if (scanExParam.maxChannelTime == 0)
                        WM_lib_set_scanEx_maxChannelTime(200);

                    scanExParam.bssid[0] = 0xff;
                    scanExParam.bssid[1] = 0xff;
                    scanExParam.bssid[2] = 0xff;
                    scanExParam.bssid[3] = 0xff;
                    scanExParam.bssid[4] = 0xff;
                    scanExParam.bssid[5] = 0xff;

                    scanning_flag = 1;

#ifdef WMLIB_DEBUG
                    OS_TPrintf("dcf: WM_StartScanEx\n");
#endif
                    // WM_StartScanEx発行
                    errcode = WM_StartScanEx(WM_lib_dcf_child_callback, &scanExParam);
                    check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                }
                else
                {
                    // WM_StartScan使用時
                    scanParam.scanBuf = param->parentInfoBuf;

                    if (scanParam.channel == 0)
                        scanParam.channel = 1;
                    if (scanParam.maxChannelTime == 0)
                        WM_lib_set_max_scan_time(200);

                    scanExParam.bssid[0] = 0xff;
                    scanExParam.bssid[1] = 0xff;
                    scanExParam.bssid[2] = 0xff;
                    scanExParam.bssid[3] = 0xff;
                    scanExParam.bssid[4] = 0xff;
                    scanExParam.bssid[5] = 0xff;

                    scanning_flag = 1;

#ifdef WMLIB_DEBUG
                    OS_TPrintf("dcf: WMi_StartScan\n");
#endif
                    // WM_StartScan発行
                    errcode = WM_StartScan(WM_lib_dcf_child_callback, &scanParam);
                    check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                }
            }
        }
        break;


        //---------------------------------------------------------------
    case WM_APIID_START_SCAN:
        {
            WMstartScanCallback *callback = (WMstartScanCallback *)arg;

#ifdef WMLIB_DEBUG
            OS_TPrintf("dcf: WMi_StartScan callback ");
#endif
            if (WM_ERRCODE_SUCCESS == callback->errcode)
            {
                switch (callback->state)
                {
                case WM_STATECODE_SCAN_START:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("start\n");
#endif
                    break;

                case WM_STATECODE_PARENT_FOUND:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("found\n");
#endif
                    param->callback(WM_TYPE_PARENT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        if (WM_lib_change_scan_channel_flag == TRUE)
                            changeScanChannel(&scanParam);

                        errcode = WM_StartScan(WM_lib_dcf_child_callback, &scanParam);

                        check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                    }
                    break;

                case WM_STATECODE_PARENT_NOT_FOUND:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("not found\n");
#endif
                    param->callback(WM_TYPE_PARENT_NOT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        if (WM_lib_change_scan_channel_flag == TRUE)
                            changeScanChannel(&scanParam);

                        errcode = WM_StartScan(WM_lib_dcf_child_callback, &scanParam);

                        check_WMAPI_errcode(WM_APIID_START_SCAN, errcode);
                    }
                    break;

                default:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("error\n");
#endif
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_SCAN_EX:
        {
            WMstartScanExCallback *callback = (WMstartScanExCallback *)arg;

#ifdef WMLIB_DEBUG
            OS_TPrintf("dcf: WMi_StartScanEx callback ");
#endif
            if (WM_ERRCODE_SUCCESS == callback->errcode)
            {
                switch (callback->state)
                {
                case WM_STATECODE_SCAN_START:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("start\n");
#endif
                    break;

                case WM_STATECODE_PARENT_FOUND:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("found\n");
#endif
                    param->callback(WM_TYPE_PARENT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        errcode = WM_StartScanEx(WM_lib_dcf_child_callback, &scanExParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                    }
                    break;

                case WM_STATECODE_PARENT_NOT_FOUND:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("not found\n");
#endif
                    param->callback(WM_TYPE_PARENT_NOT_FOUND, arg);
                    if (scanning_flag == 1)
                    {
                        errcode = WM_StartScanEx(WM_lib_dcf_child_callback, &scanExParam);
                        check_WMAPI_errcode(WM_APIID_START_SCAN_EX, errcode);
                    }
                    break;

                default:
#ifdef WMLIB_DEBUG
                    OS_TPrintf("error\n");
#endif
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_END_SCAN:
        {
            switch (buf->errcode)
            {
            case WM_ERRCODE_SUCCESS:
#ifdef STARTCONNECT_EX_TEST
                errcode = WM_StartConnectEx(WM_lib_dcf_child_callback, param->pInfo, NULL,
                                            FALSE, param->authMode);
#else
                errcode = WM_StartConnect(WM_lib_dcf_child_callback, param->pInfo, NULL);
#endif
                check_WMAPI_errcode(WM_APIID_START_CONNECT, errcode);
                break;

            default:
                param->callback(WM_TYPE_ERROR, arg);
                break;
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_CONNECT:
        {
            WMstartConnectCallback *callback = (WMstartConnectCallback *)arg;

            if (callback->errcode == WM_ERRCODE_SUCCESS)
            {
                switch (callback->state)
                {
                case WM_STATECODE_CONNECT_START:
                    break;

                case WM_STATECODE_CONNECTED:
                    param->callback(WM_TYPE_CONNECTED_TO_PARENT, arg);
                    errcode =
                        WM_StartDCF(WM_lib_dcf_child_callback, param->dcfBuf, param->dcfBufSize);
                    check_WMAPI_errcode(WM_APIID_START_DCF, errcode);
                    break;

                case WM_STATECODE_DISCONNECTED:
                    param->callback(WM_TYPE_DISCONNECTED_FROM_PARENT, arg);
                    break;

                case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
                    // 自ら切断した場合は処理を行いません
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
            else
            {
                // コールバックでエラー通知
                param->callback(WM_TYPE_CONNECT_FAILED, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_DCF:
        {
            WMstartDCFCallback *callback = (WMstartDCFCallback *)arg;

            if (callback->errcode == WM_ERRCODE_SUCCESS)
            {
                switch (callback->state)
                {
                case WM_STATECODE_DCF_START:
                    break;

                case WM_STATECODE_DCF_IND:
                    param->callback(WM_TYPE_DCF_CHILD_RECV, callback->recvBuf);
                    break;

                default:
                    param->callback(WM_TYPE_ERROR, arg);
                    break;
                }
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_RESET:
        {
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                errcode = WM_End(WM_lib_dcf_child_callback);
                check_WMAPI_errcode(WM_APIID_END, errcode);
            }
            else
            {
                param->endReq = 0;
                param->callback(WM_TYPE_ERROR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_END:
        {
            param->endReq = 0;
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_END_COMPLETE, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_SET_DCF_DATA:
        {
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                param->callback(WM_TYPE_DCF_CHILD_SENT, arg);
            }
            else
            {
                param->callback(WM_TYPE_DCF_CHILD_SENT_ERR, arg);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_INDICATION:
        {
        }
        break;

    default:
        param->callback(WM_TYPE_ERROR, arg);
        break;
    }
}




//========================================================================================

//--------------------------------------------------------
// WM_lib 終了要求
int WM_lib_End(void)
{
    int     errcode;

    if (param == NULL)
        return WM_ERRCODE_FAILED;

    /* End要求が既に出されている場合は、何もしない． */
    if (param->endReq == 0)
    {
        scanning_flag = 0;
        param->endReq = 1;
        errcode = WM_Reset(callback_ptr);
        return conv_errcode(errcode);
    }

    return WM_ERRCODE_FAILED;

}

//--------------------------------------------------------
// WM_lib MPデータ送信要求
int WM_lib_SetMPDataEX(u16 *data, u16 size, u16 pollbitmap)
{
    int     errcode;

    if (param == NULL)
        return WM_ERRCODE_FAILED;

    // MP開始前及び、MP送信要求中及び、終了要求発行後はMPデータの送信はできない
    if ((param->mpStarted == 0) || (param->mpBusy == 1) || (param->endReq == 1))
        return WM_ERRCODE_FAILED;

    // MPデータ送信要求中フラグを立てる
    param->mpBusy = 1;

    switch (param->mode)
    {
    case WM_MODE_MP_PARENT:
        if (0 == param->contSend)
        {
            errcode = WM_SetMPData(WM_lib_mp_parent_callback, data, size, 1000, pollbitmap);
        }
        else
        {
            errcode = WM_SetMPData(WM_lib_mp_parent_callback, data, size, 0, pollbitmap);
        }
        if (errcode == WM_ERRCODE_OPERATING)
        {
        }

        if (errcode != WM_ERRCODE_SUCCESS)
            param->mpBusy = 0;
        return conv_errcode(errcode);

    case WM_MODE_MP_CHILD:
        errcode = WM_SetMPData(WM_lib_mp_child_callback, data, size, 0, pollbitmap);
        if (errcode == WM_ERRCODE_OPERATING)
        {
        }
        if (errcode != WM_ERRCODE_SUCCESS)
            param->mpBusy = 0;
        return conv_errcode(errcode);

    default:
        return WM_ERRCODE_FAILED;
    }
}

int WM_lib_SetMPData(u16 *data, u16 size)
{
    return WM_lib_SetMPDataEX(data, size, 0xffff);
}


//--------------------------------------------------------
// WM_lib DCFデータ送信要求
int WM_lib_SetDCFData(const u8 *destAdr, u16 *data, u16 size)
{
    int     errcode;
    errcode = WM_SetDCFData(WM_lib_dcf_child_callback, destAdr, data, size);
    return conv_errcode(errcode);
}



//--------------------------------------------------------
// WM_lib 接続要求
int WM_lib_ConnectToParent(WMbssDesc *pInfo)
{
    int     errcode;
    WMGameInfo *pGameInfo = (WMGameInfo *)&(pInfo->gameInfo);

    if (param == NULL)
        return WM_ERRCODE_FAILED;

    param->pInfo = pInfo;
    param->currentTgid = pGameInfo->tgid;

    scanning_flag = 0;

    errcode = WM_EndScan(callback_ptr);
    return conv_errcode(errcode);
}

//--------------------------------------------------------
// WM_lib 切断要求
int WM_lib_Disconnect(u16 aid)
{
    int     errcode;

    if (param == NULL)
        return WM_ERRCODE_FAILED;

    errcode = WM_Disconnect(callback_ptr, aid);
    if (errcode != WM_ERRCODE_OPERATING)
        param->callback(WM_TYPE_DISCONNECT_FAILED, 0);
    return conv_errcode(errcode);
}

//--------------------------------------------------------
// WM_lib GameInfo設定要求
int WM_lib_SetGameInfo(u16 *userGameInfo, u16 size, u32 ggid, u16 tgid)
{
    int     errcode;
    u8      attribute;
    if (param == NULL)
        return WM_ERRCODE_FAILED;

    param->parentParam->userGameInfo = userGameInfo;
    param->parentParam->userGameInfoLength = size;
    param->parentParam->ggid = ggid;
    param->parentParam->tgid = tgid;

    attribute = (u8)(((param->parentParam->entryFlag) ? WM_ATTR_FLAG_ENTRY : 0) |       // entryFlag 最下位ビット
                     ((param->parentParam->multiBootFlag) ? WM_ATTR_FLAG_MB : 0) |      // multiBootFlag 第2ビット
                     ((param->parentParam->KS_Flag) ? WM_ATTR_FLAG_KS : 0) |    // KS_Flag 第3ビット
                     ((param->parentParam->CS_Flag) ? WM_ATTR_FLAG_CS : 0)      // CS_Flag 第4ビット
        );


    // 実は、userGameInfoだけでなく、parentParam自体を更新している
//    errcode = WM_SetGameInfo( callback_ptr, param->parentParam );    
    errcode = WM_SetGameInfo(callback_ptr,
                             param->parentParam->userGameInfo,
                             param->parentParam->userGameInfoLength,
                             param->parentParam->ggid, param->parentParam->tgid, attribute);

    if (errcode != WM_ERRCODE_OPERATING)
        param->callback(WM_TYPE_SET_GAMEINFO_FAILED, 0);
    return conv_errcode(errcode);
}


//--------------------------------------------------------
// WM_lib MP送信許可のチェック
BOOL WM_lib_CheckMPSend(void)
{
    if (param == NULL)
        return WM_ERRCODE_FAILED;

    if ((param->mpStarted == 1) && (param->mpBusy == 0) &&
        (param->endReq == 0) && (param->child_bitmap != 0))
        return TRUE;
    else
        return FALSE;
}

//--------------------------------------------------------
// WM_lib キーセット取得関数
int WM_lib_GetKeySet(WMkeySet *keySet)
{
    if (TRUE == WM_lib_CheckMPSend())
        return WM_GetKeySet(param->ksBuf, keySet);
    else
        return 0xffff;
}

//--------------------------------------------------------
u16 WM_lib_CurrentScanChannel(void)
{
    return scanParam.channel;
}


//--------------------------------------------------------
// ScanChannel変更関数
static void changeScanChannel(WMscanParam *param)
{
    switch (param->channel)
    {
    case 1:
        param->channel = 7;
        break;

    case 7:
        param->channel = 13;
        break;

    case 13:
        param->channel = 1;
        break;

    default:
        param->channel = 1;
        break;
    }
}


void WM_lib_set_scan_channel_seek(BOOL flag)
{
    WM_lib_change_scan_channel_flag = flag;
}


void WM_lib_set_bssid(u16 bssid0, u16 bssid1, u16 bssid2)
{
    /* 順序の変更厳禁 */
    scanParam.bssid[0] = (u8)(bssid0 & 0x00ff);
    scanParam.bssid[1] = (u8)(bssid0 >> 8);
    scanParam.bssid[2] = (u8)(bssid1 & 0x00ff);
    scanParam.bssid[3] = (u8)(bssid1 >> 8);
    scanParam.bssid[4] = (u8)(bssid2 & 0x00ff);
    scanParam.bssid[5] = (u8)(bssid2 >> 8);
}

void   *WM_lib_get_mp_parent_callback_ptr(void)
{
    return &WM_lib_mp_parent_callback;
}

void   *WM_lib_get_mp_child_callback_ptr(void)
{
    return &WM_lib_mp_child_callback;
}

//-----------------------------------------------------------------------------------
// rate_rssiを4段階のRSSI値に変換する
u16 WM_lib_CalcRSSI(u16 rate_rssi, u16 aid)
{
    u16     rssi;
    u16     i;
    static u16 bufCounter[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static u16 rssiBuf[16][16] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    rssi = (u16)(rate_rssi >> 8);
//┌─────────────────┬──┬──┐
//│ RSSI(上位6bit)                   │LNA │rsv │
//└─────────────────┴──┴──┘
    rssi &= ~0x0001;                   // rsvを念のためクリア
    if (rssi & 0x0002)
    {
        rssi >>= 2;
    }
    else
    {
        rssi >>= 2;
        rssi += 25;
    }

    // 最近16回の平均値を計算
    rssiBuf[aid][bufCounter[aid]] = rssi;
    if (++bufCounter[aid] == 16)
        bufCounter[aid] = 0;

    rssi = 0;
    for (i = 0; i < 16; ++i)
        rssi += rssiBuf[aid][i];

    rssi >>= 4;

    // RSSI値を4段階にして返す
    if (rssi < 8)
        return 0;
    else if (rssi < 20)
        return 1;
    else if (rssi < 36)
        return 2;
    else
        return 3;
}


// WM_APIの戻り値チェック
static void check_WMAPI_errcode(u16 apiid, int errcode)
{
    u16     arg[2];

    if (errcode != WM_ERRCODE_OPERATING && errcode != WM_ERRCODE_SUCCESS)
    {
        arg[0] = apiid;
        arg[1] = (u16)errcode;
        param->callback(WM_TYPE_API_ERROR, arg);
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
