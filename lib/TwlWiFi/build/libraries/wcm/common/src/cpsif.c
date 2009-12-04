/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     cpsif.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-12-19#$
  $Rev: 178 $
  $Author: adachi_hiroaki $
 *---------------------------------------------------------------------------*/
#include "wcm_private.h"
#include "wcm_message.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define WCMCPSIF_DUMMY_IRQ_THREAD   ((OSThread*)OS_IrqHandler)

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/* 
 * ThreadQueue や Mutex など、WCMライブラリが終了される際に解放されてしまうと
 * 困る変数群は static な変数とする。これらの変数は一度初期化されると２度と
 * 解放されたり再初期化されたりしない。
 */
static WCMCpsifWork wcmCpsifw;

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static void WcmCpsifWmCallbackForSendAsync(void* arg);
static u32 WCMi_MergeToBuffer(u8 deviceId, u16* sendBuf, 
                              const void* header, u32 headerLen,
                              const void* body, u32 bodyLen);


/*---------------------------------------------------------------------------*
  Name:         WCMi_InitCpsif

  Description:  CPS ライブラリとのインターフェースとなる部分について、ワーク
                変数群を初期化する。一度初期化したら再初期化はしない。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_InitCpsif(void)
{
    if (wcmCpsifw.initialized == 0)
    {
        wcmCpsifw.initialized = 1;
        wcmCpsifw.sendResult = WM_ERRCODE_SUCCESS;
        MI_CpuClear8(wcmCpsifw.macAddress, sizeof(wcmCpsifw.macAddress));

        /* wcmCpsifw.recvCallback は変更されている可能性があるので初期化しない */
        OS_InitThreadQueue(&(wcmCpsifw.sendQ));
        OS_InitMutex(&(wcmCpsifw.sendMutex));
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_CpsifRecvCallback

  Description:  無線ドライバが DCF フレームを受信した際に呼び出される関数。
                割り込みサービス内で呼び出され、CPS の受信コールバック関数を
                呼び出す。

  Arguments:    srcAdrs    -   受信した DCF フレーム構造体に含まれる、送信元アドレス
                destAdrs   -   受信した DCF フレーム構造体に含まれる、送信先アドレス
                data       -   受信したデータ
                length     -   受信した DCF フレーム構造体のサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_CpsifRecvCallback(u8 *srcAdrs, u8 *destAdrs, u8 *data, s32 length)
{
#ifndef SDK_FINALROM
    if (WCM_DebugFlags & WCM_DEBUGFLAG_DUMP_RECV)
    {
        OS_TPrintf("Received frame: src=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   srcAdrs[0], srcAdrs[1], srcAdrs[2], srcAdrs[3], srcAdrs[4], srcAdrs[5]);
        WCMi_DumpMemory(data, (u32)length);
    }
#endif
    if (wcmCpsifw.recvCallback != NULL)
    {
        wcmCpsifw.recvCallback(srcAdrs, destAdrs, data, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetCpsifWork
  
  Description:  
  
  Argument:     None.
  
  Returns:      WCMCpsifWork - 
 *---------------------------------------------------------------------------*/
WCMCpsifWork* WCMi_GetCpsifWork(void)
{
    SDK_ASSERT(wcmCpsifw.initialized != 0);
    return &wcmCpsifw;
}
/*---------------------------------------------------------------------------*
  Name:         WCMi_CpsifSendNullPacket

  Description:  キープアライブ NULL パケット を送信する。
                他の DCF パケットを送信中の場合は何も行わない。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_CpsifSendNullPacket(void)
{
    WCMWork*    w = WCMi_GetSystemWork();
    
    if (!w)
    {
        return;
    }
    
    switch(w->deviceId)
    {
    case WCM_DEVICEID_WM:
        WCMi_WmifSendNullPacket();
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            WCMi_NwmifSendNullPacket();
            break;
        }
        // no break;
#endif
    default:
        break;;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetApMacAddress

  Description:  無線ドライバが接続を確立している AP の MAC アドレスを取得する。
    NOTICE:     この関数によって取得したポインタが示すバッファの内容は、割り込み
                等による状態変化に伴って変更されたり、バッファ自身が解放されたり
                する可能性がある点に注意。割り込み禁止した状態で呼び出し、内容を
                別バッファに退避することを推奨する。

  Arguments:    None.

  Returns:      u8*     -   接続を確立している AP の MAC アドレスへのポインタを
                            返す。接続が確立されていない場合には NULL を返す。
 *---------------------------------------------------------------------------*/
u8* WCM_GetApMacAddress(void)
{
    u8*         mac = NULL;
    WCMWork*    w = WCMi_GetSystemWork();
    OSIntrMode  e = OS_DisableInterrupts();

    // 初期化確認
    if (w != NULL)
    {
        // WCM ライブラリの状態確認
        if ((w->phase == WCM_PHASE_DCF) && (w->resetting == WCM_RESETTING_OFF))
        {
            /* 無線ドライバにおいてコネクションが確立されている状態 */
            mac = w->bssDesc.bssid;
        }
    }

    (void)OS_RestoreInterrupts(e);
    return mac;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetApEssid

  Description:  無線ドライバが接続を確立している AP の ESS-ID を取得する。
    NOTICE:     この関数によって取得したポインタが示すバッファの内容は、割り込み
                等による状態変化に伴って変更されたり、バッファ自身が解放されたり
                する可能性がある点に注意。割り込み禁止した状態で呼び出し、内容を
                別バッファに退避することを推奨する。

  Arguments:    length  -   ESS-ID の長さを取得する u16 型変数へのポインタを指定
                            する。接続が確立されていない場合には 0 が格納される。
                            長さを取得する必要がない場合には、この引数に NULL を
                            指定してもよい。

  Returns:      u8*     -   接続を確立している AP の ESS-ID データ列へのポインタ
                            を返す。接続が確立されていない場合には NULL を返す。
 *---------------------------------------------------------------------------*/
u8* WCM_GetApEssid(u16* length)
{
    u8*         essid = NULL;
    u16         len = 0;
    WCMWork*    w = WCMi_GetSystemWork();
    OSIntrMode  e = OS_DisableInterrupts();

    // 初期化確認
    if (w != NULL)
    {
        // WCM ライブラリの状態確認
        if ((w->phase == WCM_PHASE_DCF) && (w->resetting == WCM_RESETTING_OFF))
        {
            /* 無線ドライバにおいてコネクションが確立されている状態 */
            essid = &(w->bssDesc.ssid[0]);
            len = w->bssDesc.ssidLength;
        }
    }

    (void)OS_RestoreInterrupts(e);
    if (length != NULL)
    {
        *length = len;
    }

    return essid;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_SetRecvDCFCallback

  Description:  CPS の受信コールバック関数を登録する。
                無線ドライバが DCF フレームを受信する度にここで登録した受信
                コールバック関数が呼び出される。
    NOTICE:     ここで登録したコールバック関数がオーバーレイ内にある場合には、
                飛び先がメモリに存在しなくなる前に登録しているコールバック関数を
                NULL で上書きクリアする必要がある点に注意。

  Arguments:    callback    -   DCF 受信コールバック関数を指定する。
                                NULL を指定すると受信通知されなくなる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCM_SetRecvDCFCallback(WCMRecvInd callback)
{
    OSIntrMode  e = OS_DisableInterrupts();

    wcmCpsifw.recvCallback = callback;

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         WCM_SendDCFData

  Description:  無線ドライバに DCF フレーム送信を指示する。DCF フレームの送信
                完了、もしくは失敗するまで内部でブロックして待つ。また、無線の
                接続状態が DCF フレームを送信できない状態の場合には失敗する。
    NOTICE:     複数のスレッドから非同期に呼び出されることを想定しているが、
                割り込みサービス内で呼び出されることは想定していない点に注意。

  Arguments:    dstAddr -   DCF フレームのあて先となる MAC アドレスを指定する。
                buf     -   送信するデータへのポインタを指定する。
                            当関数から戻るまでは指定したバッファの内容は保持
                            しておく必要がある点に注意。
                len     -   送信するデータの長さをバイト単位で指定する。

  Returns:      s32     -   送信に成功した場合に送信されたデータ長を返す。
                            失敗した場合には負値を返す。
 *---------------------------------------------------------------------------*/
s32 WCM_SendDCFData(const u8* dstAddr, const u8* buf, s32 len)
{
    return WCM_SendDCFDataEx(dstAddr, buf, len, NULL, 0);
}

/*---------------------------------------------------------------------------*
  Name:         WCM_SendDCFDataEx

  Description:  無線ドライバに DCF フレーム送信を指示する。DCF フレームの送信
                完了、もしくは失敗するまで内部でブロックして待つ。また、無線の
                接続状態が DCF フレームを送信できない状態の場合には失敗する。
    NOTICE:     複数のスレッドから非同期に呼び出されることを想定しているが、
                割り込みサービス内で呼び出されることは想定していない点に注意。

  Arguments:    dstAddr 	-   DCF フレームのあて先となる MAC アドレスを指定する。
  				header      -   送信するデータのヘッダ部分へのポインタを指定する。
  				headerLen   -   送信するデータのヘッダの長さをバイト単位で指定する。
  				body        -   送信するデータのボディ部分へのポインタを指定する。
  				bodyLen     -   送信するデータのボディ部分の長さをバイト単位で指定する。
                
                * header、body に関しては、当関数から戻るまでは指定したバッファの内容は
                  保持しておく必要がある点に注意。

  Returns:      s32     -   送信に成功した場合に送信されたデータ長を返す。
                            失敗した場合には負値を返す。
 *---------------------------------------------------------------------------*/
s32 WCM_SendDCFDataEx(const u8* dstAddr, const u8* header, s32 headerLen,
                      const u8* body, s32 bodyLen)
{
    OSIntrMode          e = OS_DisableInterrupts();
    volatile WCMWork*   w = WCMi_GetSystemWork();
    s32                 result;
    u32                 totalLen = (u32)(headerLen + bodyLen);

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_BEFORE_INIT;        // WCM ライブラリが初期化されていない
    }

#if WCM_DEBUG
    // パラメータ確認
    if ((dstAddr == NULL) || (header == NULL))
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "dstAddr or header");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // header または body のポインタが NULL
    }

    if (headerLen <= 0)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "headerLen");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // headerLen または bodyLen が 0 以下
    }

    if (headerLen + bodyLen > WCM_ROUNDUP32(WM_DCF_MAX_SIZE))
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "headerLen + bodyLen");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // (headerLen + bodyLen) が サイズ超過
    }

    if ((u32) header & 0x01 || (u32) body & 0x01)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "header or body");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // header または body のバッファが奇数アドレス
    }

    // 割り込みサービス内かどうかを確認
    if (OS_GetProcMode() == HW_PSR_IRQ_MODE)
    {
        WCMi_Warning(WCMi_WText_InIrqMode);
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_IN_IRQ_MODE;        // IRQ モードでの呼び出しは禁止
    }
#endif

    // 別スレッドが排他送信ブロック中の場合は解除されるのを待つ
    OS_LockMutex(&(wcmCpsifw.sendMutex));

    /*
     * ブロック状態から復帰した時点で、WCMライブラリの状態は不明。
     * AP と接続されている状態でないと送信はできないため、ここで WCM ライブラリの
     * 状態を確認する必要がある。
     */
    w = WCMi_GetSystemWork();
    if (w == NULL)
    {
        OS_UnlockMutex(&(wcmCpsifw.sendMutex));
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_BEFORE_INIT;        // 寝ている間に WCM ライブラリが終了してしまった場合
    }

    if ((w->phase != WCM_PHASE_DCF) || (w->resetting == WCM_RESETTING_ON))
    {
        OS_UnlockMutex(&(wcmCpsifw.sendMutex));
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_NO_CONNECTION;      // 送信しようとした時に AP との接続が確立していない場合
    }

    /*
     * 送信データをキャッシュセーフなバッファに一旦コピーする。
     * 複数のスレッドからの送信要求は排他的に処理されるので、コピー先のキャッシュセーフな
     * バッファは一回の送信分だけあれば良いはず。
     * 
     * ヘッダ部分とボディ部分に分かれていたデータを、WCMWork の SendBuf にまとめてコピーする
     */
    totalLen = WCMi_MergeToBuffer(w->deviceId, (u16*)w->sendBuf, header, (u32)headerLen, body, (u32)bodyLen);

#ifndef SDK_FINALROM
    if (WCM_DebugFlags & WCM_DEBUGFLAG_DUMP_SEND)
    {
        OS_TPrintf("Send frame: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   dstAddr[0], dstAddr[1], dstAddr[2], dstAddr[3], dstAddr[4], dstAddr[5]);
        WCMi_DumpMemory((u8*)(w->sendBuf), totalLen);
    }
#endif

    switch(w->deviceId)
    {
    case WCM_DEVICEID_WM:
        result = WCMi_WmifSetDCFData(dstAddr, (u16*)w->sendBuf, (u16) totalLen, NULL);
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            result = WCMi_NwmifSetDCFData(dstAddr, (u16*)w->sendBuf, (u16) totalLen, NULL);
            break;
        }
        // no break;
#endif
    default:
        result = WCM_CPSIF_RESULT_ILLEGAL_PARAM;
    }
    
    // 正常終了
    OS_UnlockMutex(&(wcmCpsifw.sendMutex));
    (void)OS_RestoreInterrupts(e);

#ifndef SDK_FINALROM
    if (WCM_DebugFlags & WCM_DEBUGFLAG_DUMP_SEND)
    {
        OS_TPrintf("Send frame complete\n");
    }
#endif
    return headerLen + bodyLen;
}
/*---------------------------------------------------------------------------*
  Name:         WCM_SendDCFDataAsync

  Description:  無線ドライバに DCF フレーム送信を指示する。DCF フレームの送信
                完了を待たずに直ちに制御を返す。また、他の DCF フレームを送信
                中であったの場合や、無線の接続状態が DCF フレームを送信できな
                い状態の場合には直ちに失敗する。
    NOTICE:     割り込みサービス内でも呼び出しは可能。

  Arguments:    dstAddr -   DCF フレームのあて先となる MAC アドレスを指定する。
                buf     -   送信するデータへのポインタを指定する。
                            当関数から戻るまでは指定したバッファの内容は保持
                            しておく必要がある点に注意。
                len     -   送信するデータの長さをバイト単位で指定する。
                callback -  次のデータが送信可能になった際に呼び出される関数
                            へのポインタを指定する。

  Returns:      s32     -   送信の予約に成功した場合には0を返す。
                            失敗した場合には負値を返す。
 *---------------------------------------------------------------------------*/
s32 WCM_SendDCFDataAsync(const u8* dstAddr, const u8* buf, s32 len, WCMSendInd callback)
{
    OSIntrMode          e = OS_DisableInterrupts();
    s32                 result;
    u32                 totalLen;
    volatile WCMWork*   w = WCMi_GetSystemWork();

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_BEFORE_INIT;        // WCM ライブラリが初期化されていない
    }

    if ((w->phase != WCM_PHASE_DCF) || (w->resetting == WCM_RESETTING_ON))
    {
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_NO_CONNECTION;      // AP との接続が確立していない場合
    }

#if WCM_DEBUG
    // パラメータ確認
    if ((dstAddr == NULL) || (buf == NULL))
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "dstAddr or buf");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // ポインタが NULL
    }

    if (len <= 0)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "len");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // len が 0 以下
    }

    if (len > WCM_ROUNDUP32(WM_DCF_MAX_SIZE))
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "len");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // len が サイズ超過
    }

    if ((u32) buf & 0x01)
    {
        WCMi_Warning(WCMi_WText_IllegalParam, "buf");
        (void)OS_RestoreInterrupts(e);
        return WCM_CPSIF_RESULT_ILLEGAL_PARAM;      // buf が奇数アドレス
    }
#endif

    // 排他ロック (IRQ モードにいるつもりで排他処理を行う)
    if (FALSE == WCMi_CpsifTryLockMutexInIRQ(&(wcmCpsifw.sendMutex)))
    {
        (void)OS_RestoreInterrupts(e);
        WCMi_Warning("WCMi_CpsifTryLockMutexInIRQ failed.");
        return WCM_CPSIF_RESULT_SEND_FAILURE;       // 他のパケットを送信中
    }
    /*
     * 送信データをキャッシュセーフなバッファに一旦コピーする。
     * 複数のスレッドからの送信要求は排他的に処理されるので、コピー先のキャッシュセーフな
     * バッファは一回の送信分だけあれば良いはず。
     */
     
    totalLen = WCMi_MergeToBuffer(w->deviceId, (u16*)w->sendBuf, buf, (u32)len, NULL, 0);
    
    // 送信要求発行
    switch(w->deviceId)
    {
    case WCM_DEVICEID_WM:
        result = WCMi_WmifSetDCFData(dstAddr, (u16*)w->sendBuf, (u16) totalLen, callback);
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        if (OS_IsRunOnTwl())
        {
            result = WCMi_NwmifSetDCFData(dstAddr, (u16*)w->sendBuf, (u16) totalLen, callback);
            break;
        }
        // no break;
#endif
    default:
        result = WCM_CPSIF_RESULT_ILLEGAL_PARAM;
    }
    
    if (result < 0)
    {
        WCMi_CpsifUnlockMutexInIRQ(&(wcmCpsifw.sendMutex));
    }
    // 成功した場合はコールバックで WCMi_CpsifUnlockMutexInIRQ が呼ばれる

    (void)OS_RestoreInterrupts(e);
    return WCM_RESULT_ACCEPT;
}


/*---------------------------------------------------------------------------*
  Name:         WCMi_CpsifTryLockMutexInIRQ

  Description:  IRQ 割り込みサービス内において排他制御用 Mutex のロックを試みる。
                この関数でロックした Mutex は WcmCpsifUnlockMutexInIRQ 関数で
                のみアンロック可能。

  Arguments:    mutex   -   排他制御用 OSMutex 変数へのポインタを指定。

  Returns:      BOOL    -   ロックに成功した場合に TRUE を返す。既になんらかの
                            スレッドからロックされていた場合には FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL WCMi_CpsifTryLockMutexInIRQ(OSMutex* mutex)
{
#if WCM_DEBUG
    if (mutex == NULL)
    {
        OS_TWarning("WCMi_CpsifTryLockMutexInIRQ: Invalid argument.");
        return FALSE;
    }
#endif
    if (mutex->thread == NULL)
    {
        mutex->thread = WCMCPSIF_DUMMY_IRQ_THREAD;
        mutex->count++;
        return TRUE;
    }
    else if (mutex->thread == WCMCPSIF_DUMMY_IRQ_THREAD)
    {
        mutex->count++;
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_CpsifUnlockMutexInIRQ

  Description:  IRQ 割り込みサービス内において排他制御用 Mutex のアンロックを
                試みる。WcmCpsifTryLockMutexInIRQ 関数でロックした Mutex に対
                してのみ有効。
                アンロックできなかった場合には、それを知る手段はない。

  Arguments:    mutex   -   排他制御用 OSMutex 変数へのポインタを指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_CpsifUnlockMutexInIRQ(OSMutex* mutex)
{
#if WCM_DEBUG
    if (mutex == NULL)
    {
        OS_TWarning("WCMi_CpsifUnlockMutexInIRQ: Invalid argument.");
        return;
    }
#endif
    if (mutex->thread == WCMCPSIF_DUMMY_IRQ_THREAD)
    {
        if (--(mutex->count) == 0)
        {
            mutex->thread = NULL;
            OS_WakeupThread(&(mutex->queue));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_CpsifSetMacAddress

  Description:  CPS に渡す MAC アドレスを設定する.

  Arguments:    macAddress  - MAC アドレスの格納先ポインタを指定.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_CpsifSetMacAddress(u8* macAddress)
{
    MI_CpuCopy(macAddress, wcmCpsifw.macAddress, 6);
}

/*---------------------------------------------------------------------------*
  Name:         WCM_GetMacAddress

  Description:  選択されている無線デバイスの MAC アドレスを取得する。

  Arguments:    macAddress  - MAC アドレスの格納先ポインタを指定.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCM_GetMacAddress(u8 *macAddress)
{
    MI_CpuCopy(wcmCpsifw.macAddress, macAddress, 6);
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_MergeToBuffer

  Description:  

  Arguments:    deviceId     - 
                sendBuf      - 
                header       - 
                headerLen    - 
                body         - 
                bodyLen      - 

  Returns:      u32          - 
 *---------------------------------------------------------------------------*/
static u32 WCMi_MergeToBuffer(u8 deviceId, u16* sendBuf, 
                              const void* header, u32 headerLen,
                              const void* body, u32 bodyLen)
{
    void*   copyDest = NULL;
    u32     totalLen = (u32)(headerLen + bodyLen);
    
    /*
     * WM と NWM では送信時に渡すべきフレームの形式が違うため、
     * どちらが使われているかに応じて形式を変換する。
     *
     *        | da | sa | len | llc/snap | type | data |
     * WM                      <-----data from cps---->
     *                        ^ ptr
     *
     * NWM    <---reserved---> <-----data from cps---->
     *        ^ ptr
     */
    
    switch(deviceId)
    {
    default:
    case WCM_DEVICEID_WM:
        copyDest = sendBuf;
        break;
#ifdef SDK_TWL
    case WCM_DEVICEID_NWM:
        copyDest = (void*)((u32)sendBuf + WCM_SIZE_ETHHDR);
        ((u16*)sendBuf)[ WCM_SIZE_MACADDR * 2 / 2 ] = MI_HToBE16((u16)totalLen);
        totalLen += WCM_SIZE_ETHHDR;
        break;
#endif
    }
    
    SDK_POINTER_ASSERT(copyDest);
    
    /*
     * 送信データをキャッシュセーフなバッファに一旦コピーする。
     * 複数のスレッドからの送信要求は排他的に処理されるので、コピー先のキャッシュセーフな
     * バッファは一回の送信分だけあれば良いはず。
     * 
     * ヘッダ部分とボディ部分に分かれていたデータを、WCMWork の SendBuf にまとめてコピーする
     */
    MI_CpuCopy(header, copyDest, (u32)headerLen );
    if (body && bodyLen > 0)
    {
        MI_CpuCopy(body, (u8*)copyDest + headerLen, (u32) bodyLen);
    }
    
    return totalLen;
}


/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
