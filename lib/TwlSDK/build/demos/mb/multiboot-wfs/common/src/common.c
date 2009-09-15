/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - common
  File:     common.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-29#$
  $Rev: 8747 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    "common.h"
#include    "wfs.h"
#include    "wh.h"


/*****************************************************************************/
/* constant */

/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         StateCallbackForWFS

  Description:  WFS のコールバック関数.
                状態が WFS_STATE_READY になった時点で呼び出されます.
                このコールバックで通知を受けず, 単に任意の位置で
                WFS_GetStatus() を使用して判定することも可能です.

  Arguments:    arg       コールバックに指定したユーザ定義引数.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateCallbackForWFS(void *arg)
{
    (void)arg;
    switch (WFS_GetStatus())
    {
    case WFS_STATE_READY:
        OS_TPrintf("WFS ready.\n");
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         AllocatorForWFS

  Description:  WFS に指定するメモリの動的確保関数.

  Arguments:    arg       アロケータに指定したユーザ定義引数.
                size      メモリ確保要求ならば必要なサイズ.
                ptr       NULL の場合はメモリ確保, そうでなければ解放要求.

  Returns:      ptr が NULL なら size 分のメモリを確保してポインタを返すこと.
                そうでなければ ptr のメモリを解放すること.
                解放の場合, 返り値は単に無視される.
 *---------------------------------------------------------------------------*/
void   *AllocatorForWFS(void *arg, u32 size, void *ptr)
{
    (void)arg;
    if (!ptr)
        return OS_Alloc(size);
    else
    {
        OS_Free(ptr);
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCCallbackForWFS

  Description:  WC の無線自動駆動から状態通知を受けるコールバック.

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WHCallbackForWFS(void *arg)
{
//    WCStatus wcStatus = WcGetStatus();
    switch (((WMCallback *)arg)->apiid)
    {
    case WM_APIID_START_MP:
        {                              /* MP ステート開始 */
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                /*
                 * MP_PARENT または MP_CHILD ステートへの移行を通知するために
                 * WFS_Start() を呼び出してください.
                 * この通知を契機に, WFS はブロック転送のために
                 * 内部で WM_SetMPDataToPort() 関数を呼び出し始めます.
                 */
                WFS_Start();
                break;
            }
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitFrameLoop

  Description:  ゲームフレームループの初期化処理。

  Arguments:    p_key           初期化するキー情報構造体.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitFrameLoop(KeyInfo * p_key)
{
    ClearLine();
    ClearString();
    KeyRead(p_key);
}

/*---------------------------------------------------------------------------*
  Name:         WaitForNextFrame

  Description:  次の描画フレームまで待機。

  Arguments:    p_key           更新するキー情報構造体.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WaitForNextFrame(KeyInfo * p_key)
{
    FlushLine();
    OS_WaitVBlankIntr();
    ClearString();
    KeyRead(p_key);
}

/*---------------------------------------------------------------------------*
  Name:         ModeInitialize

  Description:  無線モジュールの初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeInitialize(void)
{
    // WH モジュールを使用
    // WH_GetSystemState() の値が WH_SYSSTATE_STOP から WH_SYSSTATE_IDLE へ移行する。
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
    
    WH_SetGgid(GGID_WBT_FS);
    WH_SetSessionUpdateCallback(WHCallbackForWFS);
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  エラー表示画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeError(void)
{
    KeyInfo key[1];

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_FATAL)
    {
        WaitForNextFrame(key);
        PrintString(5, 10, COLOR_RED, "======= ERROR! =======");
        PrintString(5, 13, COLOR_WHITE, " Fatal error occured.");
        PrintString(5, 14, COLOR_WHITE, "Please reboot program.");
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  ビジー画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeWorking(void)
{
    KeyInfo key[1];

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_SCANNING || WH_GetSystemState() == WH_SYSSTATE_BUSY)
    {
        WaitForNextFrame(key);
        PrintString(9, 11, COLOR_WHITE, "Now working...");
        if (key->trg & PAD_BUTTON_START)
        {
            WH_Finalize();
        }
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
