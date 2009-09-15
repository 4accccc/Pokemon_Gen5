/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
  File:     main.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include <nitro/wxc.h>

#include "user.h"
#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"

void VBlankIntr(void);          // Vブランク割込みハンドラ

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // キーリピート開始までのフレーム数
#define     KEY_REPEAT_SPAN     10     // キーリピートの間隔フレーム数

#define     PICTURE_FRAME_PER_GAME_FRAME    2


/* テスト用の GGID */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x52)

/* このSharingで使用する GGID */
#define WH_GGID                 SDK_MAKEGGID_SYSTEM(0x21)

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// キー入力情報
typedef struct KeyInfo
{
    u16     cnt;                       // 未加工入力値
    u16     trg;                       // 押しトリガ入力
    u16     up;                        // 離しトリガ入力
    u16     rep;                       // 押し維持リピート入力

}
KeyInfo;


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void ModeSelect(void);          // 親機/子機 選択画面
static void ModeError(void);           // エラー表示画面


// 汎用サブルーチン
static void KeyRead(KeyInfo * pKey);
static void InitializeAllocateSystem(void);

static void GetMacAddress(u8 * macAddr, u16 * gScreen);


static void ModeConnect();
static void ModeWorking(void);

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static KeyInfo gKey;                   // キー入力
static vs32 gPictureFrame;             // ピクチャーフレーム カウンタ


/*---------------------------------------------------------------------------*
    外部変数定義
 *---------------------------------------------------------------------------*/
int passby_endflg;
WMBssDesc bssdesc;
WMParentParam parentparam;
u8 macAddress[6];


static void WaitPressButton(void);
static void GetChannelMain(void);
static BOOL ConnectMain(u16 tgid);
static void PrintChildState(void);
static BOOL JudgeConnectableChild(WMStartParentCallback *cb);


//============================================================================
//   変数定義
//============================================================================

static s32 gFrame;                     // フレームカウンタ

//============================================================================
//   関数定義
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  キートリガ取得

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    // ピクチャーフレームカウンタをインクリメント
    gPictureFrame++;

    // ゲームフレームに合わせて再描画
    if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
    {
        DispVBlankFunc();
    }

    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  メインルーチン

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    u16 tgid = 0;

    // 画面、OSの初期化
    CommonInit();
    // 画面初期化
    DispInit();
    // ヒープの初期化
    InitAllocateSystem();

    // 割り込み有効
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    OS_InitTick();
    FX_Init();

    DispOn();

    passby_endflg = 0;
    
    /* すれちがい通信の初期化, 登録, 起動 */
    User_Init();

    for (gFrame = 0; passby_endflg == 0; gFrame++)
    {
        /* キー入力情報取得 */
        KeyRead(&gKey);

        // スクリーンクリア
        BgClear();
        
        /* 通信状態により処理を振り分け */
        switch (WXC_GetStateCode())
        {
        case WXC_STATE_END:
            ModeSelect();
            break;
        case WXC_STATE_ENDING:
            break;
        case WXC_STATE_ACTIVE:
            if(WXC_IsParentMode() == TRUE)
            {
                BgPutString(9, 2, 0x2, "Now parent...");
            }
            else
            {
                BgPutString(9, 2, 0x2, "Now child...");
            }
            break;
        }
        if (gKey.trg & PAD_BUTTON_START)
        {
            (void)WXC_End();
        }

        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }

    (void)WXC_End();
    
    // WXC_Endが完了するまで待つ
    while( WXC_GetStateCode() != WXC_STATE_END )
    {
        ;
    }

    // 変数初期化
    gPictureFrame = 0;

    // データシェアリング通信用にバッファを設定
    GInitDataShare();

    // 無線初期化
    if( WH_Initialize() == FALSE )
    {
        OS_Printf("\n WH_Initialize() ERROR!!\n");
    }

    if(passby_endflg == 1)
    {
        WH_SetGgid(WH_GGID);
        
        // 接続子機の判定用関数を設定
        WH_SetJudgeAcceptFunc(JudgeConnectableChild);
    
        /* メインルーチン */
        for (gFrame = 0; TRUE; gFrame++)
        {
            BgClear();

            switch (WH_GetSystemState())
            {
            case WH_SYSSTATE_IDLE:
                (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, (u16)(parentparam.tgid+1), parentparam.channel);
                break;

            case WH_SYSSTATE_CONNECTED:
            case WH_SYSSTATE_KEYSHARING:
            case WH_SYSSTATE_DATASHARING:
                {
                    BgPutString(8, 1, 0x2, "Parent mode");
                    if(GStepDataShare(gPictureFrame) == FALSE)
                    {
                        gPictureFrame--;
                    }
                    GMain();
                }
                break;
            }

            // 複数回ピクチャーフレームの完了を待つ
            while (TRUE)
            {
                // Vブランク待ち
                OS_WaitVBlankIntr();
                if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
                {
                    break;
                }
            }
        }
    }
    else
    {
        OS_Printf("\nmacAddress = %02X:%02X:%02X:%02X:%02X:%02X\n", bssdesc.bssid[0],bssdesc.bssid[1],bssdesc.bssid[2],bssdesc.bssid[3],bssdesc.bssid[4],bssdesc.bssid[5]);
        
        bssdesc.gameInfo.tgid++;
    
        // メインループ
        for (gFrame = 0; TRUE; gFrame++)
        {
            // スクリーンクリア
            BgClear();

            // 通信状態により処理を振り分け
            switch (WH_GetSystemState())
            {
            case WH_SYSSTATE_CONNECT_FAIL:
                {
                    // WM_StartConnect()に失敗した場合にはWM内部のステートが不正になっている為
                    // 一度WM_ResetでIDLEステートにリセットする必要があります。
                    WH_Reset();
                }
                break;
            case WH_SYSSTATE_IDLE:
                {
                    static retry = 0;
                    enum
                    {
                        MAX_RETRY = 30
                    };

                    if (retry < MAX_RETRY)
                    {
                        ModeConnect(bssdesc.bssid);
                        retry++;
                        break;
                    }
                    // MAX_RETRYで親機に接続できなければERROR表示
                }
            case WH_SYSSTATE_ERROR:
                ModeError();
                break;
            case WH_SYSSTATE_BUSY:
            case WH_SYSSTATE_SCANNING:
                ModeWorking();
                break;

            case WH_SYSSTATE_CONNECTED:
            case WH_SYSSTATE_KEYSHARING:
            case WH_SYSSTATE_DATASHARING:
                {
                    BgPutString(8, 1, 0x2, "Child mode");
                    if(GStepDataShare(gPictureFrame) == FALSE)
                    {
                        gPictureFrame--;
                    }
                    GMain();
                }
                break;
            }

            // 電波受信強度の表示
            {
                int level;
                level = WH_GetLinkLevel();
                BgPrintStr(31, 23, 0xf, "%d", level);
            }

            // 複数回ピクチャーフレームの完了を待つ
            while (TRUE)
            {
                // Vブランク待ち
                OS_WaitVBlankIntr();
                if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
                {
                    break;
                }
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  親機/子機 選択画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    BgPutString(3, 1, 0x2, "Push A to start");

    if (gKey.trg & PAD_BUTTON_A)
    {
        //********************************
        User_Init();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  エラー表示画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    BgPutString(5, 10, 0x2, "======= ERROR! =======");
    BgPutString(5, 13, 0x2, " Fatal error occured.");
    BgPutString(5, 14, 0x2, "Please reboot program.");
}


/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  キー入力情報を編集する。
                押しトリガ、離しトリガ、押し継続リピートを検出。

  Arguments:    pKey  - 編集するキー入力情報構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInfo * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // 押しトリガ
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // 押し継続リピート
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // 離しトリガ
            }
        }
    }
    pKey->cnt = r;                     // 未加工キー入力
}

/*---------------------------------------------------------------------------*
  Name:         JudgeConnectableChild

  Description:  再接続時に接続可能な子機かどうかを判定する関数

  Arguments:    cb      接続してきた子機の情報.

  Returns:      接続を受け付ける場合は TRUE.
                受け付けない場合は FALSE.
 *---------------------------------------------------------------------------*/
static BOOL JudgeConnectableChild(WMStartParentCallback *cb)
{
    if (cb->macAddress[0] != macAddress[0] || cb->macAddress[1] != macAddress[1] || 
        cb->macAddress[2] != macAddress[2] || cb->macAddress[3] != macAddress[3] || 
        cb->macAddress[4] != macAddress[4] || cb->macAddress[5] != macAddress[5] )
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ModeConnect

  Description:  接続開始

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void ModeConnect()
{
    WH_SetGgid(WH_GGID);
    //********************************
    (void)WH_ChildConnectAuto(WH_CONNECTMODE_DS_CHILD, bssdesc.bssid,
                              bssdesc.channel);
    //********************************
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  処理中画面を表示

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void ModeWorking(void)
{
    BgPrintStr(9, 11, 0xf, "Now working...");
}
