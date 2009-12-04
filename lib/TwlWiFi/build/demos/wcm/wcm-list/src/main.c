/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WCM - demos - wcm-list-2
  File:     main.c

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
 
/*---------------------------------------------------------------------------*
    WCM ライブラリを使用して 一般的な無線アクセスポイント ( AP ) を検索して
    リストアップし、接続するサンプルです。
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#include <twlWiFi.h>
#else
#include <nitro.h>
#include <nitroWiFi.h>
#endif

#include "screen.h"
#include "wcm_control.h"

#include "privacy_check.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define KEY_REPEAT_START    25  // キーリピート開始までのフレーム数
#define KEY_REPEAT_SPAN     10  // キーリピートの間隔フレーム数

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// キー入力情報
typedef struct KeyInfo
{
    u16 cnt;    // 未加工入力値
    u16 trg;    // 押しトリガ入力
    u16 up;     // 離しトリガ入力
    u16 rep;    // 押し維持リピート入力
} KeyInfo;

/*---------------------------------------------------------------------------*
    内部変数 定義
 *---------------------------------------------------------------------------*/

// キー入力
static KeyInfo  gKey;

// 無線デバイス
u8 gDeviceID = WCM_DEVICEID_WM;  /* デフォルトでは旧無線を使用する */

//デフォルトの接続先BSSID
static u8       gBssidDefault[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static int      gSelectedAP = 0;
static BOOL     gConnectAuto = FALSE;

/*---------------------------------------------------------------------------*
    内部関数 プロトタイプ
 *---------------------------------------------------------------------------*/
static void     ProcMain(void);
static void     SelectAP(void);
static const WCMBssDesc*     GetBssDesc(const u8* bssid);
static void VBlankIntr(void);
static void InitInterrupts(void);
static void InitHeap(void);
static void ReadKey(KeyInfo* pKey);

static void PutMainPrivateMode(s32 x, s32 y, u16 mode);
static void PutSubPrivateMode(s32 x, s32 y, u16 mode);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // 各種初期化
    OS_Init();
    OS_InitTick();  // OS_Sleep 関数を使用する為に必要
    OS_InitAlarm(); // OS_Sleep 関数を使用する為に必要
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // メモリ割り当て初期化
    InitHeap();

    // 表示設定初期化
    InitScreen();

    // 割り込み設定初期化
    InitInterrupts();

    // LCD 表示開始
    GX_DispOn();
    GXS_DispOn();

    // デバッグ文字列出力
    OS_TPrintf("\"wcm-list\" demo program started.\n");

    // キー入力情報取得の空呼び出し(IPL での A ボタン押下対策)
    ReadKey(&gKey);

    while (TRUE)
    {
        // キー入力情報取得
        ReadKey(&gKey);

        // メイン画面クリア
        ClearScreen();

        ProcMain();

        // Ｖブランク待ち ( スレッド対応 )
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ProcMain

  Description:  WCM ライブラリの接続管理状態の表示、及び入力による処理の
                切り替えを行う。メインループからピクチャーフレームごとに
                呼び出される。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcMain(void)
{
    switch (WCM_GetPhase())
    {
    case WCM_PHASE_NULL:
        // 初期状態 もしくは WCM コントロールスレッドが終了した状態
        ((u16*)HW_PLTT)[0] = 0x0000;
        PutMainScreen(11, 10, 0xff, "Now offline");
        PutMainScreen(4, 12, (u8) (((OS_GetVBlankCount() / 30) % 2) + 7), "Push A key to search AP");

#ifdef SDK_TWL /* TWL では新無線を選択可能 */

        if(OS_IsRunOnTwl())
        {
            PutMainScreen(4, 14, (u8) (((OS_GetVBlankCount() / 30) % 2) + 7), "Push R key to change I/F");
            PutMainScreen(4, 16, 0xff, "Interface:");
        
            if(gDeviceID == WCM_DEVICEID_WM)
            {
                PutMainScreen(16, 17, 0xff, "DS compatible");
            }
            else
            {
                PutMainScreen(16, 17, 0xff, "     New WiFi");
            }
    
            if (gKey.trg & PAD_BUTTON_R)
            {
                if(gDeviceID == WCM_DEVICEID_WM)
                {
                    gDeviceID = WCM_DEVICEID_NWM;
                }
                else
                {
                    gDeviceID = WCM_DEVICEID_WM;
                }
            }
        }
#endif
        if (( gKey.trg & PAD_BUTTON_A ) | gKey.rep)
        {
            SetWcmManualConnect(TRUE);
            //InitWcmControlByApInfo(NULL);
            InitWcmControlByApInfoEx(NULL, gDeviceID);
        }

        if (!WCM_CompareBssID(WCM_BSSID_ANY, gBssidDefault))
        {
            PutMainScreen(5, 23, 0x0, "Start button: auto connect");
            if (gKey.trg & PAD_BUTTON_START)
            {
                gConnectAuto = TRUE;
            }
            else
            {
                gConnectAuto = FALSE;
            }
        }
        break;

    case WCM_PHASE_SEARCH:
        // APを検索中の状態
        if (gConnectAuto)
        {
            // 自動接続モード
            PutMainScreen(11, 10, 0x0, "Now online");
            PutMainScreen(4, 11, 0x0, "connecting to default AP");
            PutMainScreen(3, 12, (u8) (((OS_GetVBlankCount() / 30) % 2) + 7), "Push any key to select AP");
            if (gKey.trg | gKey.rep)
            {
                // キーが押されていればAP選択モードへ
                gConnectAuto = FALSE;
            }
            else
            {
                //デフォルトのBssIDを持つAPを探す
                const WCMBssDesc*    pbd = GetBssDesc(gBssidDefault);
                if (pbd)
                {
                    //見つかれば接続する
                    ConnectAP(pbd);
                }
            }
        }
        else
        {
            // AP選択モード
            SelectAP();
        }
        break;

    case WCM_PHASE_DCF:
        // 接続された状態
        ((u16*)HW_PLTT)[0] = 0x7ff0;
        PutMainScreen(11, 10, 0x0, "Now online");
        PutMainScreen(3, 12, (u8) (((OS_GetVBlankCount() / 30) % 2) + 7), "Push any key to disconnect");
        if (gKey.trg | gKey.rep)
        {
            TerminateWcmControl();
        }
        break;

    case WCM_PHASE_FATAL_ERROR:
        // 復旧不可能な FATAL ERROR
        ((u16*)HW_PLTT)[0] = 0x001f;
        PutMainScreen(0, 7, 0xf, "================================");
        PutMainScreen(7, 10, 0xf, "WCM FATAL ERROR !");
        PutMainScreen(2, 12, 0xf, "Turn off power supply please.");
        PutMainScreen(0, 15, 0xf, "================================");
        OS_WaitVBlankIntr();
        OS_Terminate();
        break;

    default:
        // 処理中
        ((u16*)HW_PLTT)[0] = 0x6318;
        PutMainScreen(10, 10, 0x0, "Now working...");
        PutMainScreen(6, 12, (u8) (((OS_GetVBlankCount() / 30) % 2) + 7), "Push any key to stop");
        if (gKey.trg | gKey.rep)
        {
            TerminateWcmControl();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SelectAP

  Description:  利用できるアクセスポイント一覧を表示し選択する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SelectAP(void)
{
    int     nIndex = 0;
    int     nAPCount = 0;
    char    buffer[256];
    s32     nLevel;
    BOOL    bDefault = FALSE;

    ((u16*)HW_PLTT)[0] = 0x0000;
    PutMainScreen(0, 0, 0x4, "ch:bssid        ssid / link");
    PutMainScreen(0, 23, 0x4, "A:connect, Sel:set def, St:clear");

    (void)WCM_LockApList(WCM_APLIST_LOCK);
    nAPCount = WCM_CountApList();
    if (nAPCount == 0)
    {
        //APが一個も見つからなければ表示・選択しない
        (void)WCM_LockApList(WCM_APLIST_UNLOCK);
        return;
    }

    //選択カーソルの移動
    if (gKey.trg & PAD_KEY_DOWN)
    {
        ++gSelectedAP;
    }

    if (gKey.trg & PAD_KEY_UP)
    {
        --gSelectedAP;
    }

    gSelectedAP = MATH_CLAMP(gSelectedAP, 0, nAPCount - 1);
    {
        WCMBssDesc*  pbdSelected = WCM_PointApListEx(gSelectedAP);
        if (pbdSelected)
        {
            if (gKey.trg & PAD_BUTTON_A)
            {
                ConnectAP(pbdSelected);
            }
            if (gKey.trg & PAD_BUTTON_SELECT)
            {
                MI_CpuCopy8(pbdSelected->bssid, gBssidDefault, WM_SIZE_BSSID);
            }
        }
    }

    for (nIndex = 0; nIndex < nAPCount; ++nIndex)
    {
        WCMBssDesc* pbd = WCM_PointApListEx(nIndex);
        u16 privacy_mode = GetPrivacyMode(pbd);
        u8 tmp_ssid[33];    // ssid(32Byte を文字列として表示する際に、必ず終端文字が入るように33Byteへ一旦コピーする

        bDefault = WCM_CompareBssID(gBssidDefault, pbd->bssid);
        nLevel = WCM_PointApListLinkLevel(nIndex);

        MI_CpuClear8(tmp_ssid, sizeof(tmp_ssid));
        
        /* ステルス SSID は * STEALTH * と表示 */
        if( pbd->ssidLength != 0 )
        {
            MI_CpuCopy8(pbd->ssid, tmp_ssid, pbd->ssidLength);
        }
        else
        {
            MI_CpuCopy8("* STEALTH *", tmp_ssid, 12);
        }

        (void)OS_SPrintf(buffer, "%02d:%02X%02X%02X%02X%02X%02X%s%s/%d", pbd->channel, pbd->bssid[0], pbd->bssid[1], pbd->bssid[2],
                         pbd->bssid[3], pbd->bssid[4], pbd->bssid[5], bDefault ? "*" : " ", tmp_ssid, nLevel);                         

        if(nIndex < 10)
        {
            PutMainScreen(0, 1 + nIndex * 2, (u8) (nIndex == gSelectedAP ? 0x1 : 0xf), buffer);
            PutMainPrivateMode(0, 2 + nIndex * 2, privacy_mode);

        }
        else /* 表示しきれない AP はサブ画面に */
        {
            PutSubScreen(0, 1 + (nIndex-10) * 2, (u8) (nIndex == gSelectedAP ? 0x1 : 0xf), buffer);
            PutSubPrivateMode(0, 2 + (nIndex-10) * 2, privacy_mode);
        }
    
    }
    (void)WCM_LockApList(WCM_APLIST_UNLOCK);

    if (gKey.trg & PAD_BUTTON_START)
    {
        WCM_ClearApList();
    }

}

/*---------------------------------------------------------------------------*
  Name:         GetBssDesc

  Description:  指定されたBssIDを持ったAPが見つかっていればそのBssDescを得る。

  Arguments:    BssDescを得たいAPのBssID

  Returns:      見つかったAPのBssDescへのポインタ
 *---------------------------------------------------------------------------*/
static const WCMBssDesc* GetBssDesc(const u8* bssid)
{
    int nAPCount = 0;
    int nIndex = 0;

    (void)WCM_LockApList(WCM_APLIST_LOCK);
    nAPCount = WCM_CountApList();
    for (nIndex = 0; nIndex < nAPCount; ++nIndex)
    {
        WCMBssDesc*  pbd = WCM_PointApListEx(nIndex);
        if (WCM_CompareBssID((u8*)bssid, pbd->bssid))
        {
            (void)WCM_LockApList(WCM_APLIST_UNLOCK);
            return pbd;
        }
    }
    (void)WCM_LockApList(WCM_APLIST_UNLOCK);
    return NULL;
}

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みハンドラ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // テキスト表示を更新
    UpdateScreen();

    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitInterrupts

  Description:  割り込み設定を初期化する。
                V ブランク割り込みを許可し、割り込みハンドラを設定する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInterrupts(void)
{
    // V ブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // 割り込み許可
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitHeap

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitHeap(void)
{
    void*           tempLo;
    OSHeapHandle    hh;

    // メインメモリ上のアリーナにヒープをひとつ作成
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        // ヒープ作成に失敗した場合は異常終了
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  キー入力情報を取得し、入力情報構造体を編集する。
                押しトリガ、離しトリガ、押し継続リピートトリガ を検出する。

  Arguments:    pKey  - 編集するキー入力情報構造体を指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadKey(KeyInfo* pKey)
{
    static u16  repeat_count[12];
    int         i;
    u16         r;

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
                    repeat_count[i] = (u16) (KEY_REPEAT_START - KEY_REPEAT_SPAN);
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

    pKey->cnt = r;  // 未加工キー入力
}

static void PutMainPrivateMode(s32 x, s32 y, u16 mode)
{
    switch( mode )
    {
    case WCM_PRIVACYMODE_NONE:
        PutMainScreen(x, y, 0x2, "             NONE or UNKNOWN");
        break;
    case WCM_PRIVACYMODE_WEP40:
    case WCM_PRIVACYMODE_WEP104:
    case WCM_PRIVACYMODE_WEP128:
    case WCM_PRIVACYMODE_WEP:
        PutMainScreen(x, y, 0x8, "                         WEP");
        break;
#ifdef SDK_TWL
    case WCM_PRIVACYMODE_WPA_TKIP:
        PutMainScreen(x, y, 0x2, "             WPA-PSK( TKIP )");
        break;
    case WCM_PRIVACYMODE_WPA2_TKIP:
        PutMainScreen(x, y, 0x2, "            WPA2_PSK( TKIP )");
        break;
    case WCM_PRIVACYMODE_WPA_AES:
        PutMainScreen(x, y, 0x2, "              WPA_PSK( AES )");
        break;
    case WCM_PRIVACYMODE_WPA2_AES:
        PutMainScreen(x, y, 0x2, "             WPA2-PSK( AES )");
        break;
#else
    case WCM_PRIVACYMODE_WPA_TKIP:
    case WCM_PRIVACYMODE_WPA2_TKIP:
    case WCM_PRIVACYMODE_WPA_AES:
    case WCM_PRIVACYMODE_WPA2_AES:
        PutMainScreen(x, y, 0x2, "    NONSUPPORT SECURITY TYPE");
        break;
#endif
    }
}

static void PutSubPrivateMode(s32 x, s32 y, u16 mode)
{
    switch( mode )
    {
    case WCM_PRIVACYMODE_NONE:
        PutSubScreen(x, y, 0x2, "             NONE or UNKNOWN");
        break;
    case WCM_PRIVACYMODE_WEP40:
    case WCM_PRIVACYMODE_WEP104:
    case WCM_PRIVACYMODE_WEP128:
    case WCM_PRIVACYMODE_WEP:
        PutSubScreen(x, y, 0x8, "                         WEP");
        break;
#ifdef SDK_TWL
    case WCM_PRIVACYMODE_WPA_TKIP:
        PutSubScreen(x, y, 0x2, "             WPA-PSK( TKIP )");
        break;
    case WCM_PRIVACYMODE_WPA2_TKIP:
        PutSubScreen(x, y, 0x2, "            WPA2_PSK( TKIP )");
        break;
    case WCM_PRIVACYMODE_WPA_AES:
        PutSubScreen(x, y, 0x2, "              WPA_PSK( AES )");
        break;
    case WCM_PRIVACYMODE_WPA2_AES:
        PutSubScreen(x, y, 0x2, "             WPA2-PSK( AES )");
        break;
#else
    case WCM_PRIVACYMODE_WPA_TKIP:
    case WCM_PRIVACYMODE_WPA2_TKIP:
    case WCM_PRIVACYMODE_WPA_AES:
    case WCM_PRIVACYMODE_WPA2_AES:
        PutMainScreen(x, y, 0x2, "    NONSUPPORT SECURITY TYPE");
        break;
#endif
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
