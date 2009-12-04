/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_cleanup.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitroWiFi/socl.h>
#include "wcm_cpsif.h"

/*---------------------------------------------------------------------------*
  Name:         SOCL_Clearup

  Description:  SOC/CPS ソケットライブラリをシャットダウンします。

  Arguments:    なし

  Returns:       0 正常終了
                -1 全てのソケットサービスが停止していません
 *---------------------------------------------------------------------------*/
int SOCL_Cleanup(void)
{
    int result;

    SDK_ASSERT(SOCLiConfigPtr);

    // IP アドレスが保存されていないなら保存しておく
    if (SOCLiRequestedIP == 0)
    {
        SOCLiRequestedIP = CPSMyIp;
    }

    // 全てのソケットをクローズしていく．
    // SOCLiUDPSendSocket もこの処理でクローズされる
    while (SOCL_EINPROGRESS == SOCL_CalmDown())
    {
        OS_Sleep(100);
    }

    result = SOCLi_CleanupCommandPacketQueue();

    if (result >= 0)
    {
#ifdef SDK_MY_DEBUG
        OS_TPrintf("CPS_Cleanup\n");
#endif
        SOCLi_CleanupCPS();

        if (!SOCLiConfigPtr->lan_buffer)    // 自力確保した領域の開放
        {
            SOCLi_Free(SOCLiCPSConfig.lan_buf);
        }

        SOCLiConfigPtr = NULL;
        
        SOCLi_InitResource();
    }

    return result;
}

void SOCLi_CleanupCPS(void)
{
    // DHCP Release を行なうためにブロックする
    // これを避けたい場合は SOCL_CalmDown() で処理を進めておくこと
    CPS_Cleanup();

    // 非同期 Cleanup 対策
    CPS_SetScavengerCallback(NULL);
    CPS_SetCheckNConfigCallback(NULL);

    WCM_SetRecvDCFCallback(NULL);
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_CloseAll

  Description:  ユーザソケットを全て非同期でクローズする
                (特殊ソケットは残す)
  
  Arguments:    なし
  
  Returns:      SOCL_EINPROGRESS=-26 ならクローズ処理中
                SOCL_ESUCCESS   = 0  なら完了
 *---------------------------------------------------------------------------*/
int SOCL_CloseAll(void)
{
    SOCLSocket*     socket;
    OSIntrMode      enable;

    for (;;)
    {
        // クローズ呼び出し中に SocketList の状態が変化するかもしれないので
        // 毎回先頭から検索する
        enable = OS_DisableInterrupts();
        for (socket = SOCLiSocketList; socket; socket = socket->next)
        {
            // まだクローズ処理が始まっていない一般のソケットであるか？
            if ((int)socket != SOCLiUDPSendSocket && !SOCL_SocketIsWaitingClose(socket))
            {
                break;
            }
        }
        (void)OS_RestoreInterrupts(enable);

        if (!socket)
        {
            break;
        }

        (void)SOCL_Close((int)socket);
    }

    // ソケットリストが空、あるいは UDPSend ソケットのみであり廃棄リストが空なら終了
    if (SOCLiSocketList == NULL || ((int)SOCLiSocketList == SOCLiUDPSendSocket && SOCLiSocketList->next == NULL))
    {
        if (SOCLiSocketListTrash == NULL)
        {
            return SOCL_ESUCCESS;
        }
    }

    return SOCL_EINPROGRESS;
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_CalmDown

  Description:  全てのソケットおよび補助ソケットを非同期でクローズし、cleanup の
                準備を行なう．CPS ライブラリの停止処理も開始する．
  
  Arguments:    なし
  
  Returns:      SOCL_EINPROGRESS=-26 ならクローズ処理中
                SOCL_ESUCCESS   = 0  なら完了
 *---------------------------------------------------------------------------*/
int SOCL_CalmDown(void)
{
    int result;

    if (SOCLiUDPSendSocket)
    {
        result = SOCL_CloseAll();

        if (result == SOCL_ESUCCESS)
        {
            (void)SOCL_Close(SOCLiUDPSendSocket);

            if (SOCL_IsClosed(SOCLiUDPSendSocket))
            {
                SOCLiUDPSendSocket = NULL;
            }

            result = SOCL_EINPROGRESS;
        }

        SOCLi_TrashSocket();
    }
    else
    {   // 非同期 Cleanup 対策
        if (CPS_CalmDown())
        {
            WCM_SetRecvDCFCallback(NULL);
            result = SOCL_ESUCCESS;
        }
        else
        {
            result = SOCL_EINPROGRESS;
        }
    }

    return result;
}
