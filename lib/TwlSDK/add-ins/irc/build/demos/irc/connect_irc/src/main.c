/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IRC - demos - connect_irc
  File:     main.c

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nitro/card.h>
#include <nitro/fs.h>

#include <nitro/card/backup_ex.h>
#include <nitro/irc.h>


/* 画面描画用のルーチン */
#include "draw.h"

#define IRC_DEBUG   (1)
#define MY_UNIT_NUMBER  (0x7F)

static void TestWriteAndVerify(void);
static void ShowResult(void);

/* CARD ライブラリ用変数 */

/*
 * CARD バックアップのロック用 ID.
 * CARD ハードウェアリソースを FS ライブラリなどと排他使用するため
 * CARD_LockBackup() / CARD_UnlockBackup() で使用します.
 */
static u16 card_lock_id;

/*
 * CARD アクセス関数のエラー時の結果値.
 * CARD_GetResultCode() の返り値は CARD_UnlockBackup() 等で変更されるので,
 * アクセス処理とエラー処理が離れている場合にはこの点に注意してください.
 */
static CARDResult last_result = CARD_RESULT_SUCCESS;

// 書き込みテストが実行されたかのフラグ.
static BOOL is_test_run;

#define MY_TRNS_SIZE        (128)   // (IRC_TRNS_MAX)
#define DATA_CONFIRM_SIZE   (0x80)
/**********************************************************************/
static u32 cnt_success;
u8 data_up[MY_TRNS_SIZE];
u8 data_down[MY_TRNS_SIZE];

static int cnt_userdata = 0;
/**********************************************************************/
static void init_data(void)
{
    int i;

    for (i = 0; i < MY_TRNS_SIZE; i++)
    {
        data_up[i] = (u8)i;
    }
    for (i = 0; i < MY_TRNS_SIZE; i++)
    {
        data_down[i] = (u8)(data_up[MY_TRNS_SIZE-1-i]);
    }
}

// 接続コールバック
static void connectCallback(u8 id, BOOL isSender)
{
    if (id != MY_UNIT_NUMBER)
    {
        return;
    }
    cnt_userdata = 0;
#ifdef IRC_DEBUG
    DrawText(0, 8, "CONNECT");
#endif
    if (!isSender) // レシーバからの最初の送信
    {
        data_down[0] = (u8)cnt_userdata;
        IRC_Send((u8*)data_down, MY_TRNS_SIZE, 0x02);
    }
}

// データ受信コールバック(センダーとレシーバーで共通)
static void recvCallback(u8 *data, u8 size, u8 command, u8/*id*/)
{
    u8 value;

    if (size != MY_TRNS_SIZE)
    {
        return;
    }

    value = data[0];
    switch (command)
    {
    case 0x02: // センダーは2を受信
        if (value >= DATA_CONFIRM_SIZE)
        {
            break;
        }
        cnt_success++;
      
        data_up[0] = (u8)value;
        IRC_Send((u8*)data_up, MY_TRNS_SIZE, 0x04);

        SetTextColor(RGB555_GREEN);
        FillRect(8, 48, 8*8, 8, RGB555_BLACK);
        DrawText(8, 48, "%02X TIMES", value);
        if (value == 0)
        {
            FillRect(0, (64-8), 8*32, 8*16, RGB555_BLACK);
        }
        DrawText((value&0x7)*16*2, (64-8)+(value/8)*8, "%02X", value);
        break;
    case 0x04: // レシーバーは4を受信
        if (value >= DATA_CONFIRM_SIZE)
        {
            break;
        }
        if (value != cnt_userdata)
        {
            break;
        }
        cnt_success++;

        FillRect(8, 48, 8*8, 8, RGB555_BLACK);
        SetTextColor(RGB555_GREEN);
        DrawText(8, 48, "%02X TIMES", value);
        if (value == 0)
        {
            FillRect(0, (64-8), 8*32, 8*16, RGB555_BLACK);
        }
        DrawText((value&0x7)*16*2, (64-8)+(value/8)*8, "%02X", value);

        cnt_userdata = (cnt_userdata+1) & (DATA_CONFIRM_SIZE-1);
        data_down[0] = (u8)cnt_userdata;
        IRC_Send(data_down, MY_TRNS_SIZE, 0x02);
        break;
    case 0xF4: // COMMAND_SHUTDOWN:
#ifdef IRC_DEBUG
        OS_TPrintf("コールバック内: SHUTDOWN COMMAND ACCEPT\n");
#endif
        cnt_userdata = 0;
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetPadDown

  Description:  押下タイミングのキー入力を取得.

  Arguments:    None.

  Returns:      前回呼び出しから今回呼び出しまでの間に押下されたキーのフラグ.
 *---------------------------------------------------------------------------*/
static u32 GetPadDown(void)
{
    static u32 btn = 0;
    static u32 btn_bak = 0;

    btn_bak = btn;
    btn = PAD_Read();
    return (u32)(~btn_bak & btn);
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V ブランクコールバック.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitApp

  Description:  OS 等, アプリケーション基本部分の初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitApp(void)
{
    void   *tmp;
    OSHeapHandle hh;

    /* OS の初期化 */
    OS_Init();
    OS_InitThread();
    OS_InitTick();
    /* アリーナの初期化 */
    tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tmp);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_TPanic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
    /* 割り込みを有効に */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         GetCardResultString

  Description:  CARD 関数の結果値を文字列で取得.

  Arguments:    result : CARDResult 結果値.

  Returns:      結果値を説明する文字列へのポインタ.
 *---------------------------------------------------------------------------*/
static const char *GetCardResultString(CARDResult result)
{
    switch (result)
    {
    case CARD_RESULT_SUCCESS:
        return "success";
    case CARD_RESULT_FAILURE:
        return "failure";
    case CARD_RESULT_INVALID_PARAM:
        return "invalid param";
    case CARD_RESULT_UNSUPPORTED:
        return "unsupported";
    case CARD_RESULT_TIMEOUT:
        return "timeout";
    case CARD_RESULT_CANCELED:
        return "canceled";
    case CARD_RESULT_NO_RESPONSE:
        return "no response";
    case CARD_RESULT_ERROR:
        return "error";
    default:
        return "unknown error";
    }
}

/*---------------------------------------------------------------------------*
  Name:         SelectDevice

  Description:  デバイス選択画面.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static u8 idxCursor;

static BOOL isSender; // 最初に赤外線を出す側
static void IRCTest(void)
{
    BOOL end = FALSE;

    cnt_success = 0;
    IRC_Init(MY_UNIT_NUMBER);                // 初期化とユニット番号のセット
    IRC_SetConnectCallback(connectCallback); // 接続コールバックをセット
    IRC_SetRecvCallback(recvCallback);       // データ受信コールバックをセット
    isSender = FALSE;
    while (!end)
    {
        u32     btn_down = GetPadDown();

        DrawBegin();
#ifdef IRC_DEBUG
        //エラー率の表示
        {
            u32 rate; 
            u32 cnt_retry;

            cnt_retry = IRC_GetRetryCount();
            rate = 10000*cnt_retry / (cnt_success+cnt_retry);
            FillRect(0, 8*3, 8*24, 8, RGB555_BLACK);
            DrawText(0, 8*3, "% 8d/% 8d % 3d.%02d%%", cnt_retry, cnt_success+cnt_retry, rate/100, rate%100);
        }

        FillRect(8*(31-5), 0, 8*2, 8, RGB555_BLACK);
        if (IRC_IsConnect())
        {
            DrawText(8*(31-5), 0, "C");
        }
        //if (flg_restore){ DrawText(8*(31-4), 0, "R"); }

        FillRect(8*(31-2), 0, 8*3, 8, RGB555_BLACK);
        {
            int i;
            for (i = 0; i < (idxCursor>>4); i++)
            {
                DrawText(8*(31-2+i), 0, "*");
            }
        }
        idxCursor = (u8)((idxCursor+1)&63);

        FillRect(0, 0, 8*3, 8, RGB555_BLACK);
        if (IRC_IsSender())
        {
            DrawText(0, 0, "SEND");
        }
        else
        {
            DrawText(0, 0, "RECV");
        }
        FillRect(0, 8*2, 8*12, 8, RGB555_BLACK);
        DrawText(0, 8*2, "UNIT ID %02X-%02X", MY_UNIT_NUMBER, IRC_GetUnitNumber()); // 相手のID
#endif
        IRC_Move();

        if ((btn_down & PAD_BUTTON_A) || (isSender == TRUE && IRC_IsConnect() == FALSE)) // ボタンを押したか、センダー側が切断した場合
        {
            cnt_userdata = 0;
            IRC_Connect();
            isSender = TRUE;
            ClearFrame(RGB555_BLACK);
            DrawText(32, 0, "REQ>");
        }
        if (btn_down & PAD_BUTTON_B)
        {
            isSender = FALSE;
            IRC_Shutdown(); 
            cnt_userdata = 0;
            end = TRUE; 
        }
        DrawEnd();
    }
}

static void SelectDevice(void)
{
    static const struct
    {
        CARDBackupType type;
        const char *comment;
    }
    types_table[] =
    {
        { CARD_BACKUP_TYPE_EEPROM_4KBITS_IRC,    "EEPROM    4 kb"},
        { CARD_BACKUP_TYPE_EEPROM_64KBITS_IRC,   "EEPROM   64 kb"},
        { CARD_BACKUP_TYPE_EEPROM_512KBITS_IRC,  "EEPROM  512 kb"},
        { CARD_BACKUP_TYPE_FLASH_2MBITS_IRC,     "FLASH     2 Mb"},
        { CARD_BACKUP_TYPE_FLASH_4MBITS_IRC,     "FLASH     4 Mb"},
        { CARD_BACKUP_TYPE_FLASH_8MBITS_IRC,     "FLASH     8 Mb"},
    };

    enum
    {
        types_table_max = sizeof(types_table) / sizeof(*types_table)
    };

    int     cur = 0;
    BOOL    error = FALSE;
    BOOL    end = FALSE;
    int     i;

    while (!end)
    {

        u32     btn_down = GetPadDown();

        DrawBegin();
        ClearFrame(RGB555_BLACK);
        
        /* 上下キーでカーソル移動 */
        if (btn_down & PAD_KEY_DOWN)
        {
            error = FALSE;
            if (++cur >= types_table_max)
            {
                cur -= types_table_max;
            }
        }
        if (btn_down & PAD_KEY_UP)
        {
            error = FALSE;
            if (--cur < 0)
            {
                cur += types_table_max;
            }
        }

        /*
         * A ボタンで現在の選択デバイスに特定.
         *
         * ライブラリは指定のデバイスが実際に搭載されているのかは
         * 判断できませんので, 正しく指定してください.
         */
        if (btn_down & PAD_BUTTON_A)
        {
            CARD_LockBackup((u16)card_lock_id);
            end = CARD_IdentifyBackup(types_table[cur].type);
            if (!end)
            {
                error = TRUE;
                last_result = CARD_GetResultCode();
            }
            CARD_UnlockBackup(card_lock_id);
        }

        /* 文字列表示 */
        SetTextColor(RGB555_GREEN);
        DrawText(10, 40, "select device!");
        for (i = 0; i < types_table_max; ++i)
        {
            DrawText(20, 60 + 10 * i, types_table[i].comment);
        }
        FillRect(10, 60, 10, i * 10, RGB555_BLACK);
        DrawText(10, 60 + 10 * cur, ">");
        if (error)
        {
            /* エラー表示 */
            SetTextColor(RGB555_RED);
            DrawText(10, 120, "error!");
            DrawText(10, 130, "result:\"%s\"", GetCardResultString(last_result));
        }
        DrawEnd();
    }
}

static void test(void)
{
    static int cur = 0;
    u32 btn_down = GetPadDown();

    DrawBegin();
    ClearFrame(RGB555_BLACK);

    /* 上下キーでカーソル移動 */
    if (btn_down & PAD_KEY_DOWN)
    {
        if (++cur >= 2)
        {
            cur -= 2;
        }
    }
    if (btn_down & PAD_KEY_UP)
    {
        if (--cur < 0)
        {
            cur += 2;
        }
    }
    if (btn_down & PAD_BUTTON_A)
    {
        if(cur == 0)
        {
            IRCTest();
        }
        else
        {
            /* デバイス選択画面 */
            SelectDevice();
            /* テスト開始 */
            is_test_run = FALSE;
            TestWriteAndVerify();
            /* 結果表示 */
            if (is_test_run)
            {
                ShowResult();
            }
        }
    }

    SetTextColor(RGB555_GREEN);
    DrawText(10, 40, "select device!");
    DrawText(20, 60 + 10 * 0, "IR");
    DrawText(20, 60 + 10 * 1, "BACKUP");

    FillRect(10, 60, 10, 10 * 2, RGB555_BLACK);
    DrawText(10, 60 + 10 * cur, ">");

    DrawEnd();
}
/*---------------------------------------------------------------------------*
  Name:         TestWriteAndVerify

  Description:  書き込みテスト画面.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TestWriteAndVerify(void)
{
    const u32 page_size  = CARD_GetBackupPageSize();
    const u32 total_size = CARD_GetBackupTotalSize();

    u32     pos = 0;
    BOOL    end = FALSE;

    /* 各種情報を表示 */
    DrawBegin();
    ClearFrame(RGB555_BLACK);
    SetTextColor(RGB555_WHITE);
    if (CARD_IsBackupEeprom())
    {
        DrawText(10, 10, "EEPROM");
    }
    else if (CARD_IsBackupFlash())
    {
        DrawText(10, 10, "FLASH");
    }
    else if (CARD_IsBackupFram())
    {
        DrawText(10, 10, "FRAM");
    }
    DrawText(10, 20, "page:%d  total:%d", page_size, total_size);
    FillRect(10, 40, 256, 100, RGB555_BLACK);
    SetTextColor(RGB555_BLUE);
    MoveTo(0 + 8, 35);
    LineTo(256 - 8, 35);
    LineTo(256 - 8, 182 - 8);
    LineTo(0 + 8, 182 - 8);
    LineTo(0 + 8, 35);
    DrawEnd();

    while (!end)
    {
        int mode = 0;
        u32 btn_down = GetPadDown();
        if (btn_down & PAD_KEY_LEFT)
        {
            mode = (mode+(3-1))%3;
        }
        if (btn_down & PAD_KEY_RIGHT)
        {
            mode = (mode+1)%3;
        }
        DrawBegin();

        /* 開始前なら A ボタンで開始 */
        if (!is_test_run)
        {
            if (btn_down & PAD_BUTTON_A)
            {
                is_test_run = TRUE;
            }
            SetTextColor(RGB555_WHITE);
            DrawText(10, 40, "press A-BUTTON to test");
        }

        /* B ボタンでテスト中止 */
        if (btn_down & PAD_BUTTON_B)
        {
            end = TRUE;
        }

        /* テスト中 */
        if (is_test_run)
        {

            u8      tmp_buf[256];

            FillRect(10, 40, 256 - 10 * 2, 100, RGB555_BLACK);

            SetTextColor(RGB555_GREEN);
            DrawText(10, 40, "now testing...");
            DrawText(10, 50, "address:%d-%d", pos, pos + sizeof(tmp_buf));

            /*
             * 以降のバックアップアクセスのために CARD リソースをロックする.
             *
             * これより CARD_UnlockBackup() を呼ぶまでの間
             * FS など他のモジュールからの CARD アクセスは
             * ブロッキングされることになります.
             * 処理のデッドロックに注意してください.
             */
            CARD_LockBackup((u16)card_lock_id);
            {
                OSTick  tick;

                int     i;

                for (i = 0; i < sizeof(tmp_buf); ++i)
                {
                    tmp_buf[i] = (u8)(pos * 3 + i);
                }
                tick = OS_GetTick();

                /* 非同期で書き込み、内容確認判定 */
                if (CARD_IsBackupEeprom())
                {
                    CARD_WriteAndVerifyEepromAsync(pos, tmp_buf, sizeof(tmp_buf), NULL, (void *)0);
                }
                else if (CARD_IsBackupFlash())
                {
                    CARD_WriteAndVerifyFlashAsync(pos, tmp_buf, sizeof(tmp_buf), NULL, (void *)0);
                }
                else if (CARD_IsBackupFram())
                {
                    CARD_WriteAndVerifyFramAsync(pos, tmp_buf, sizeof(tmp_buf), NULL, (void *)0);
                }

                /*
                 * 完了待ち.
                 *
                 * デバイス種類と指定サイズによっては
                 * 非常に長い時間ブロッキングされますので,
                 * 各ユーザアプリケーションのフレームワークに合う形で
                 * 待ち方を工夫してください.
                 * (1 フレームに 1 回 CARD_TryWaitBackupAsync() で判定する, 等)
                 */
                (void)CARD_WaitBackupAsync();
                last_result = CARD_GetResultCode();

                if (last_result != CARD_RESULT_SUCCESS)
                {
                    /* エラーなら終了 */
                    end = TRUE;
                }
                else
                {
                    /* 成功なら時間を表示 */
                    tick = OS_GetTick() - tick;
                    DrawText(10, 60, "%6d[ms]", OS_TicksToMilliSeconds(tick));
                    /* テストアドレスを次へ移動 */
                    pos += sizeof(tmp_buf);
                    if (pos + sizeof(tmp_buf) > total_size)
                    {
                        end = TRUE;
                    }
                }
            }
            CARD_UnlockBackup(card_lock_id);
        }
        DrawEnd();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ShowResult

  Description:  テスト結果表示画面.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowResult(void)
{

    BOOL    end = FALSE;

    while (!end)
    {
        u32     btn_down = GetPadDown();

        DrawBegin();

        /* A / B / START ボタンで戻る */
        if (btn_down & (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_START))
        {
            end = TRUE;
        }

        if (last_result == CARD_RESULT_SUCCESS)
        {
            /* 成功表示 */
            SetTextColor(RGB555_WHITE);
            DrawText(10, 70, "done! (success)");
        }
        else
        {
            /* エラー表示 */
            SetTextColor(RGB555_RED);
            DrawText(10, 70, "error!");
            DrawText(10, 80, "result:\"%s\"", GetCardResultString(last_result));
        }
        DrawEnd();
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  メイン エントリポイント.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{

    {
        /* サンプルアプリケーションの初期化 */
        InitApp();
        InitDraw();
        SetClearColor(RGB555_CLEAR);
    }

    {
        /* CARD ライブラリの初期化 */
        s32     ret;

        CARD_Init();
        ret = OS_GetLockID();
        if (ret == OS_LOCK_ID_ERROR)
        {
            OS_TPanic("demo fatal error! OS_GetLockID() failed");
        }
        card_lock_id = (u16)ret;
    }
    init_data();
    /* 画面遷移 */
    for (;;)
    {
        test();
    }

}
