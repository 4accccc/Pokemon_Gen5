/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - CARD - backup
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-27#$
  $Rev: 9430 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"


/*---------------------------------------------------------------------------*/
/* variables */

// CARD バックアップのロック用 ID。
// CARD ハードウェアリソースを FS ライブラリなどと排他的に使用するため
// CARD_LockBackup() / CARD_UnlockBackup() で使用します。
static u16  card_lock_id;

// CARD アクセス関数のエラー時の結果値。
// CARD_GetResultCode() の返り値は CARD_UnlockBackup() 等で変更されるので
// アクセス処理とエラー処理が離れている場合にはこの点に注意してください。
static CARDResult last_result = CARD_RESULT_SUCCESS;

// 書き込みテストが実行されたかのフラグ。
static BOOL is_test_run;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         GetCardResultString

  Description:  CARD 関数の結果値を文字列で取得

  Arguments:    result           CARDResult 結果値

  Returns:      結果値を説明する文字列へのポインタ
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

  Description:  デバイス選択画面

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SelectDevice(void)
{
    /* *INDENT-OFF* */
    static const struct
    {
        CARDBackupType type;
        const char *comment;
    }
    types_table[] =
    {
        { CARD_BACKUP_TYPE_EEPROM_4KBITS,   "EEPROM    4 kb"},
        { CARD_BACKUP_TYPE_EEPROM_64KBITS,  "EEPROM   64 kb"},
        { CARD_BACKUP_TYPE_EEPROM_512KBITS, "EEPROM  512 kb"},
        { CARD_BACKUP_TYPE_EEPROM_1MBITS,   "EEPROM    1 Mb"},
        { CARD_BACKUP_TYPE_FLASH_2MBITS,    "FLASH     2 Mb"},
        { CARD_BACKUP_TYPE_FLASH_4MBITS,    "FLASH     4 Mb"},
        { CARD_BACKUP_TYPE_FLASH_8MBITS,    "FLASH     8 Mb"},
        { CARD_BACKUP_TYPE_FLASH_16MBITS,   "FLASH    16 Mb"},
        { CARD_BACKUP_TYPE_FLASH_64MBITS,   "FLASH    64 Mb"},
    };
    /* *INDENT-ON* */
    enum
    { types_table_max = sizeof(types_table) / sizeof(*types_table) };

    int     cur = 0;
    BOOL    error = FALSE;
    BOOL    end = FALSE;
    int     i;

    while (!end)
    {
        DEMOReadKey();
        // 上下キーでカーソル移動
        if (DEMO_IS_TRIG(PAD_KEY_DOWN))
        {
            error = FALSE;
            if (++cur >= types_table_max)
            {
                cur -= types_table_max;
            }
        }
        if (DEMO_IS_TRIG(PAD_KEY_UP))
        {
            error = FALSE;
            if (--cur < 0)
            {
                cur += types_table_max;
            }
        }

        // A ボタンで現在の選択デバイスに特定。
        // ライブラリは指定のデバイスが実際に搭載されているか
        // 判断できませんので, 正しく指定してください。
        if (DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            CARD_LockBackup(card_lock_id);
            end = CARD_IdentifyBackup(types_table[cur].type);
            if (!end)
            {
                error = TRUE;
                last_result = CARD_GetResultCode();
            }
            CARD_UnlockBackup(card_lock_id);
        }

        // 画面表示
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
        DEMODrawText(10, 40, "select device!");
        for (i = 0; i < types_table_max; ++i)
        {
            DEMODrawText(10, 60 + 10 * i, "%c%s",
                         (cur == i) ? '>' : ' ', types_table[i].comment);
        }
        if (error)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 0, 0, 1));
            DEMODrawText(10, 160, "error!");
            DEMODrawText(10, 170, "result:\"%s\"", GetCardResultString(last_result));
        }

        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         TestWriteAndVerify

  Description:  書き込みテスト画面

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TestWriteAndVerify(void)
{
    const u32   page_size = CARD_GetBackupPageSize();
    const u32   sector_size = CARD_GetBackupSectorSize();
    const u32   total_size = CARD_GetBackupTotalSize();

    OSTick      erase_time = 0;
    u32         pos = 0;

    BOOL        end = FALSE;

    // 画面の初期化
    {
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
        if (CARD_IsBackupEeprom())
        {
            DEMODrawText(10, 10, "EEPROM");
        }
        else if (CARD_IsBackupFlash())
        {
            DEMODrawText(10, 10, "FLASH");
        }
        DEMODrawText(10, 20, "page:%d  sector:%d", page_size, sector_size);
        DEMODrawText(10, 30, "total:%d", total_size);
        DEMOFillRect(10, 50, GX_LCD_SIZE_X - 10 * 2, 100, GX_RGBA(0, 0, 0, 1));
        DEMODrawFrame(8, 45, GX_LCD_SIZE_X - 8 * 2, 182 - 8 - 45, GX_RGBA(0, 0, 31, 1));
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }

    while (!end)
    {
        DEMOReadKey();
        // 開始前ならAボタンで開始
        if (!is_test_run)
        {
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                is_test_run = TRUE;
            }
        }
        //Bボタンでテスト中止
        else if (DEMO_IS_TRIG(PAD_BUTTON_B))
        {
            end = TRUE;
        }

        // 書き込みテストと画面表示
        if (!is_test_run)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
            DEMODrawText(10, 50, "press A-BUTTON to test");
        }
        else
        {
            static u8 tmp_buf[65536];
            SDK_ASSERT(sizeof(tmp_buf) >= sector_size);

            DEMOFillRect(10, 50, 256 - 10 * 2, 100, GX_RGBA(0, 0, 0, 1));
            DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
            DEMODrawText(10, 50, "now testing...");
            DEMODrawText(10, 60, "address:%d-%d", pos, pos + page_size);

            // 以降のバックアップアクセスのために CARD リソースをロックします。
            // これより CARD_UnlockBackup() を呼ぶまでの間は
            // FS など他のモジュールからの CARD アクセスはブロッキングされます。
            // 処理のデッドロックに注意してください。
            CARD_LockBackup(card_lock_id);
            {
                OSTick  tick;
                int     i;
                for (i = 0; i < page_size; ++i)
                {
                    tmp_buf[i] = (u8)(pos * 3 + i);
                }
                tick = OS_GetTick();
                // 非同期で書き込み + 内容確認判定
                if (CARD_IsBackupEeprom())
                {
                    CARD_WriteAndVerifyEepromAsync(pos, tmp_buf, page_size, NULL, NULL);
                }
                else if (CARD_IsBackupFlash())
                {
                    // ほとんどのFLASHデバイスはWrite操作を使用可能です。
                    if (CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_FLASH_64MBITS)
                    {
                        CARD_WriteAndVerifyFlashAsync(pos, tmp_buf, page_size, NULL, NULL);
                    }
                    // 一部の大容量FLASHはEraseSector操作とProgram操作のみ使用可能です。
                    // 消去操作はセクタサイズの整数倍単位のみという点に注意してください。
                    else
                    {
                        BOOL    programmable = FALSE;
                        if ((pos % sector_size) == 0)
                        {
                            // これから書き込む領域は事前にセクタごと消去する必要があります。
                            // このサンプルデモではセクタ全体の内容を書き換えていますが、
                            // セクタの一部分のみを書き換えたい場合には消去前の内容を退避して
                            // 消去後に再び同じ内容を書き込みなおす必要があります。
                            erase_time = tick;
                            programmable = CARD_EraseFlashSector(pos, sector_size);
                            tick = OS_GetTick();
                            erase_time = tick - erase_time;
                            last_result = CARD_GetResultCode();
                        }
                        else
                        {
                            // すでに消去済みの領域はそのまま書き込み可能です。
                            programmable = TRUE;
                        }
                        if (programmable)
                        {
                            CARD_ProgramAndVerifyFlashAsync(pos, tmp_buf, page_size, NULL, NULL);
                        }
                    }
                }

                // このサンプルでは、この場で処理完了を待って結果を確認します。
                // デバイス種類と指定サイズによっては非常に長い時間ブロッキングされますので
                // 各ユーザアプリケーションのフレームワークに合う形で待ち方を工夫してください。
                // (例えば、1フレームに1回だけCARD_TryWaitBackupAsync()で確認するなど)
                (void)CARD_WaitBackupAsync();
                last_result = CARD_GetResultCode();
                if (last_result != CARD_RESULT_SUCCESS)
                {
                    // エラーならひとまず終了
                    end = TRUE;
                }
                else
                {
                    // 成功なら時間を表示
                    tick = OS_GetTick() - tick;
                    DEMODrawText(10, 70, "write:%6d[ms]/%d[BYTE]", (int)OS_TicksToMilliSeconds(tick), page_size);
                    if (erase_time != 0)
                    {
                        DEMODrawText(10, 80, "erase:%6d[ms]/%d[BYTE]", (int)OS_TicksToMilliSeconds(erase_time), sector_size);
                    }
                    // テストアドレスを次へ移動
                    pos += page_size;
                    if (pos + page_size > total_size)
                    {
                        end = TRUE;
                    }
                }
            }
            CARD_UnlockBackup(card_lock_id);
        }

        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ShowResult

  Description:  テスト結果表示画面

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowResult(void)
{
    BOOL    end = FALSE;

    while (!end)
    {
        DEMOReadKey();
        // A / B / START ボタンで戻る
        if (DEMO_IS_TRIG(PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_START))
        {
            end = TRUE;
        }

        // 画面表示
        if (last_result == CARD_RESULT_SUCCESS)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
            DEMODrawText(10, 100, "done! (success)");
        }
        else
        {
            DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
            DEMODrawText(10, 100, "error!");
            DEMODrawText(10, 110, "result:\"%s\"", GetCardResultString(last_result));
        }
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  メイン エントリポイント

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // SDKの初期化
    OS_Init();
    OS_InitTick();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // デモ表示の初期化
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    // カードアプリ以外がROMやバックアップにアクセスする場合、
    // 挿入されているDSカードが自社タイトルであるか判定した上で
    // 明示的にCARD_Enable関数を呼び出して有効化する必要があります。
    // 詳細はプログラミングガイドラインをご参照ください。
    if (OS_GetBootType() != OS_BOOTTYPE_ROM)
    {
        const CARDRomHeader *own_header = (const CARDRomHeader *)HW_ROM_HEADER_BUF;
        const CARDRomHeader *rom_header = (const CARDRomHeader *)CARD_GetRomHeader();
        if (own_header->maker_code != rom_header->maker_code)
        {
            // このサンプルはバックアップデバイスにアクセスすることが目的なので、
            // DSカードが挿入されていないならここで停止します。
            static const char *message = "cannot detect own-maker title DS-CARD!";
            DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
            DEMOSetBitmapTextColor(GX_RGBA(31, 0, 0, 1));
            DEMODrawText(10, 40, message);
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
            OS_TPanic(message);
        }
        else
        {
            CARD_Enable(TRUE);
        }
    }

    {
        // CARDライブラリのデータバスをロックするためのIDを確保。
        s32     ret = OS_GetLockID();
        if (ret == OS_LOCK_ID_ERROR)
        {
            OS_TPanic("demo fatal error! OS_GetLockID() failed");
        }
        card_lock_id = (u16)ret;
    }

    // 画面遷移
    for (;;)
    {
        // デバイス選択画面
        SelectDevice();
        // テスト開始
        is_test_run = FALSE;
        TestWriteAndVerify();
        // 結果表示
        if (is_test_run)
        {
            ShowResult();
        }
    }

}
