/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_backup.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_spi.h"


#ifndef SDK_ARM9
SDK_ERROR("this code is only for ARM9"!
#endif // SDK_ARM9


/*---------------------------------------------------------------------------*/
/* constants */

#include <nitro/version_begin.h>
SDK_DEFINE_MIDDLEWARE(cardi_backup_assert, "NINTENDO", "BACKUP");
#include <nitro/version_end.h>
#define SDK_USING_BACKUP() SDK_USING_MIDDLEWARE(cardi_backup_assert)


/*---------------------------------------------------------------------------*/
/* variables */

/* 最後に転送したバックアップページのキャッシュ */
static u8 CARDi_backup_cache_page_buf[256] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_OnFifoRecv

  Description:  PXI FIFO ワード受信コールバック

  Arguments:    tag        PXI タグ (常に PXI_FIFO_TAG_FS)
                data       受信データ
                err        エラービット (旧仕様によるもの)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_OnFifoRecv(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(data)
    if ((tag == PXI_FIFO_TAG_FS) && err)
    {
        CARDiCommon *const p = &cardi_common;
        /* ARM7 からの応答を受信して完了を通知する */
        SDK_ASSERT(data == CARD_REQ_ACK);
        p->flag &= ~CARD_STAT_WAITFOR7ACK;
        OS_WakeupThreadDirect(p->current_thread_9);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_Request

  Description:  ARM9 から ARM7 へリクエスト送信して完了をブロッキング。
                結果がCARD_RESULT_SUCCESSでないなら指定回数までは再試行する。
                (所定のバスのロックおよびタスクスレッドの排他制御は
                 この関数の呼び出し元で保証する)
                他のコマンド内で繰り返し呼び出されるサブ処理。

  Arguments:    req_type     コマンドリクエストの種類
                retry_max    リトライ最大回数

  Returns:      結果がCARD_RESULT_SUCCESSであればTRUE
 *---------------------------------------------------------------------------*/
static BOOL CARDi_Request(CARDiCommon *p, int req_type, int retry_count)
{
    // PXI 未初期化ならここで実行
    if ((p->flag & CARD_STAT_INIT_CMD) == 0)
    {
        p->flag |= CARD_STAT_INIT_CMD;
        while (!PXI_IsCallbackReady(PXI_FIFO_TAG_FS, PXI_PROC_ARM7))
        {
            OS_SpinWait(100);
        }
        // 最初のコマンド "INIT" を送信 (再帰)
        (void)CARDi_Request(p, CARD_REQ_INIT, 1);
    }
    // 設定した共有メモリをフラッシュ
    DC_FlushRange(p->cmd, sizeof(*p->cmd));
    DC_WaitWriteBufferEmpty();

    // コマンド応答を受けるためにスレッドポインタを登録。
    p->current_thread_9 = OS_GetCurrentThread();

    do
    {
        // コマンドリクエスト送信。
        p->flag |= CARD_STAT_WAITFOR7ACK;
        CARDi_SendPxi((u32)req_type);
        // 後続引数があれば追加送信
        switch (req_type)
        {
        case CARD_REQ_INIT:
            CARDi_SendPxi((u32)p->cmd);
            break;
        }
        {
            // ARM7からリクエスト完了を待機。
            OSIntrMode bak_psr = OS_DisableInterrupts();
            while ((p->flag & CARD_STAT_WAITFOR7ACK) != 0)
            {
                OS_SleepThread(NULL);
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        DC_InvalidateRange(p->cmd, sizeof(*p->cmd));
    }
    while ((p->cmd->result == CARD_RESULT_TIMEOUT) && (--retry_count > 0));

    return (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommandCore

  Description:  データを転送するコマンドリクエストの処理本体。
                同期的あるいは非同期的に呼び出される。

  Arguments:    p          ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_RequestStreamCommandCore(CARDiCommon * p)
{
    const int req_type = p->req_type;
    const int req_mode = p->req_mode;
    const int retry_count = p->req_retry;
    u32     size = sizeof(CARDi_backup_cache_page_buf);

    SDK_USING_BACKUP();

    /* ページまたはセクタ単位でリクエスト */
    if (req_type == CARD_REQ_ERASE_SECTOR_BACKUP)
    {
        size = CARD_GetBackupSectorSize();
    }
    else if (req_type == CARD_REQ_ERASE_SUBSECTOR_BACKUP)
    {
        size = cardi_common.cmd->spec.subsect_size;
    }
    do
    {
        const u32 len = (size < p->len) ? size : p->len;
        p->cmd->len = len;

        /* キャンセル要求があればここで中止 */
        if ((p->flag & CARD_STAT_CANCEL) != 0)
        {
            p->flag &= ~CARD_STAT_CANCEL;
            p->cmd->result = CARD_RESULT_CANCELED;
            break;
        }
        switch (req_mode)
        {
        case CARD_REQUEST_MODE_RECV:
            /* 受信系コマンドならバッファを無効化 */
            DC_InvalidateRange(CARDi_backup_cache_page_buf, len);
            p->cmd->src = (u32)p->src;
            p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
            break;
        case CARD_REQUEST_MODE_SEND:
        case CARD_REQUEST_MODE_SEND_VERIFY:
            /* 送信系コマンドならデータをテンポラリバッファへコピー */
            MI_CpuCopy8((const void *)p->src, CARDi_backup_cache_page_buf, len);
            DC_FlushRange(CARDi_backup_cache_page_buf, len);
            DC_WaitWriteBufferEmpty();
            p->cmd->src = (u32)CARDi_backup_cache_page_buf;
            p->cmd->dst = (u32)p->dst;
            break;
        case CARD_REQUEST_MODE_SPECIAL:
            /* バッファ操作は不要 */
            p->cmd->src = (u32)p->src;
            p->cmd->dst = (u32)p->dst;
            break;
        }
        /* リクエスト送信 */
        if (!CARDi_Request(p, req_type, retry_count))
        {
            break;
        }
        /* 指定があればさらに同じ設定でベリファイ要求 */
        if (req_mode == CARD_REQUEST_MODE_SEND_VERIFY)
        {
            if (!CARDi_Request(p, CARD_REQ_VERIFY_BACKUP, 1))
            {
                break;
            }
        }
        else if (req_mode == CARD_REQUEST_MODE_RECV)
        {
            /* キャッシュからコピー */
            MI_CpuCopy8(CARDi_backup_cache_page_buf, (void *)p->dst, len);
        }
        p->src += len;
        p->dst += len;
        p->len -= len;
    }
    while (p->len > 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommandCore

  Description:  セクタ消去 + プログラムのリクエストの処理本体。
                同期的あるいは非同期的に呼び出される。

  Arguments:    p          ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_RequestWriteSectorCommandCore(CARDiCommon * p)
{
    const u32 sector_size = CARD_GetBackupSectorSize();
    SDK_USING_BACKUP();

    /* 処理範囲がセクタ単位の整数倍でなければ失敗を返す */
    if ((((u32)p->dst | p->len) & (sector_size - 1)) != 0)
    {
        p->flag &= ~CARD_STAT_CANCEL;
        p->cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else
    {
        /* セクタ単位処理 */
        for (; p->len > 0; p->len -= sector_size)
        {
            u32     len = sector_size;
            /* キャンセル要求があればここで中止 */
            if ((p->flag & CARD_STAT_CANCEL) != 0)
            {
                p->flag &= ~CARD_STAT_CANCEL;
                p->cmd->result = CARD_RESULT_CANCELED;
                break;
            }
            /* セクタ消去 */
            p->cmd->dst = (u32)p->dst;
            p->cmd->len = len;
            if (!CARDi_Request(p, CARD_REQ_ERASE_SECTOR_BACKUP, 1))
            {
                break;
            }
            while (len > 0)
            {
                const u32 page = sizeof(CARDi_backup_cache_page_buf);
                /* キャンセル要求があればここで中止 */
                if ((p->flag & CARD_STAT_CANCEL) != 0)
                {
                    p->flag &= ~CARD_STAT_CANCEL;
                    p->cmd->result = CARD_RESULT_CANCELED;
                    break;
                }
                /* プログラム */
                MI_CpuCopy8((const void *)p->src, CARDi_backup_cache_page_buf, page);
                DC_FlushRange(CARDi_backup_cache_page_buf, page);
                DC_WaitWriteBufferEmpty();
                p->cmd->src = (u32)CARDi_backup_cache_page_buf;
                p->cmd->dst = (u32)p->dst;
                p->cmd->len = page;
                if (!CARDi_Request(p, CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX))
                {
                    break;
                }
                /* 必要ならベリファイ */
                if (p->req_mode == CARD_REQUEST_MODE_SEND_VERIFY)
                {
                    if (!CARDi_Request(p, CARD_REQ_VERIFY_BACKUP, 1))
                    {
                        break;
                    }
                }
                p->src += page;
                p->dst += page;
                len -= page;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatusCore

  Description:  ステータスレジスタへのアクセス処理本体。
                同期的あるいは非同期的に呼び出される。

  Arguments:    p          ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_AccessStatusCore(CARDiCommon *p)
{
    CARDRequest command = (CARDRequest)CARDi_backup_cache_page_buf[1];
    DC_FlushRange(CARDi_backup_cache_page_buf, 1);
    p->cmd->src = (u32)CARDi_backup_cache_page_buf;
    p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
    (void)CARDi_Request(p, command, 1);
    DC_InvalidateRange(CARDi_backup_cache_page_buf, 1);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IdentifyBackupCore2

  Description:  デバイスタイプ特定.

  Arguments:    type              特定するデバイスタイプ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_IdentifyBackupCore2(CARDBackupType type)
{
    /*
     * 取得したパラメータを CARDiCommandArg へ保存.
     * これは最終的にテーブルを廃止することで完成する.
     */
    {
        CARDiCommandArg *const p = cardi_common.cmd;

        /* まず全パラメータをクリアして NOT_USE 状態にする */
        MI_CpuFill8(&p->spec, 0, sizeof(p->spec));
        p->type = type;
        p->spec.caps = (CARD_BACKUP_CAPS_AVAILABLE | CARD_BACKUP_CAPS_READ_STATUS);
        if (type != CARD_BACKUP_TYPE_NOT_USE)
        {
            /*
             * デバイスタイプ, 総容量, ベンダは type から取得可能.
             * ベンダ番号は, 同一タイプで複数メーカーが採用され
             * かつ不具合などの理由で区別する必要が生じた場合以外 0.
             */
            const u32 size = (u32)(1 << ((type >> CARD_BACKUP_TYPE_SIZEBIT_SHIFT) &
                                         CARD_BACKUP_TYPE_SIZEBIT_MASK));
            const int device =
                ((type >> CARD_BACKUP_TYPE_DEVICE_SHIFT) & CARD_BACKUP_TYPE_DEVICE_MASK);
            const int vender =
                ((type >> CARD_BACKUP_TYPE_VENDER_SHIFT) & CARD_BACKUP_TYPE_VENDER_MASK);

            p->spec.total_size = size;
            /* ステータスレジスタの補正が不要なら 0xFF. (これが通常) */
            p->spec.initial_status = 0xFF;
            if (device == CARD_BACKUP_TYPE_DEVICE_EEPROM)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x000200:        // CARD_BACKUP_TYPE_EEPROM_4KBITS
                    p->spec.page_size = 0x10;
                    p->spec.addr_width = 1;
                    p->spec.program_page = 5;
                    p->spec.initial_status = 0xF0;
                    break;
                case 0x002000:        // CARD_BACKUP_TYPE_EEPROM_64KBITS
                    p->spec.page_size = 0x0020;
                    p->spec.addr_width = 2;
                    p->spec.program_page = 5;
                    p->spec.initial_status = 0x00;
                    break;
                case 0x010000:        // CARD_BACKUP_TYPE_EEPROM_512KBITS
                    p->spec.page_size = 0x0080;
                    p->spec.addr_width = 2;
                    p->spec.program_page = 10;
                    p->spec.initial_status = 0x00;
                    break;
				case 0x020000:	      // CARD_BACKUP_TYPE_EEPROM_1MBITS
					p->spec.page_size = 0x0100;
					p->spec.addr_width = 3;
					p->spec.program_page = 5;
                    p->spec.initial_status = 0x00;
					break;
                }
                p->spec.sect_size = p->spec.page_size;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
            }
            else if (device == CARD_BACKUP_TYPE_DEVICE_FLASH)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x040000:        // CARD_BACKUP_TYPE_FLASH_2MBITS
                case 0x080000:        // CARD_BACKUP_TYPE_FLASH_4MBITS
                case 0x100000:        // CARD_BACKUP_TYPE_FLASH_8MBITS
                    p->spec.write_page = 25;
                    p->spec.write_page_total = 300;
                    p->spec.erase_page = 300;
                    p->spec.erase_sector = 5000;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_PAGE;
                    break;
                case 0x200000:        // CARD_BACKUP_TYPE_FLASH_16MBITS
                    p->spec.write_page = 23;
                    p->spec.write_page_total = 300;
                    p->spec.erase_sector = 500;
                    p->spec.erase_sector_total = 5000;
                    p->spec.erase_chip = 10000;
                    p->spec.erase_chip_total = 60000;
                    p->spec.initial_status = 0x00;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_PAGE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    break;
                case 0x400000:        // CARD_BACKUP_TYPE_FLASH_32MBITS
                    p->spec.erase_sector = 600;
                    p->spec.erase_sector_total = 3000;
                    p->spec.erase_subsector = 70;
                    p->spec.erase_subsector_total = 150;
                    p->spec.erase_chip = 23000;
                    p->spec.erase_chip_total = 800000;
                    p->spec.initial_status = 0x00;
                    p->spec.subsect_size = 0x1000;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_SUBSECTOR;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    break;
                case 0x800000:
                    if (vender == 0)  // CARD_BACKUP_TYPE_FLASH_64MBITS
                    {
                        p->spec.erase_sector = 1000;
                        p->spec.erase_sector_total = 3000;
                        p->spec.erase_chip = 68000;
                        p->spec.erase_chip_total = 160000;
                        p->spec.initial_status = 0x00;
                        p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                        p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    }
                    else if (vender == 1)   // CARD_BACKUP_TYPE_FLASH_64MBITS_EX
                    {
                        p->spec.erase_sector = 1000;
                        p->spec.erase_sector_total = 3000;
                        p->spec.erase_chip = 68000;
                        p->spec.erase_chip_total = 160000;
                        p->spec.initial_status = 0x84;
                        p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                        p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    }
                    break;
                }
                p->spec.sect_size = 0x010000;
                p->spec.page_size = 0x0100;
                p->spec.addr_width = 3;
                p->spec.program_page = 5;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_ERASE_SECTOR;
            }
            else if (device == CARD_BACKUP_TYPE_DEVICE_FRAM)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x002000:        // #CARD_BACKUP_TYPE_FRAM_64KBITS
                case 0x008000:        // #CARD_BACKUP_TYPE_FRAM_256KBITS
                    break;
                }
                p->spec.page_size = size;
                p->spec.sect_size = size;
                p->spec.addr_width = 2;
                p->spec.initial_status = 0x00;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
            }
            else
            {
              invalid_type:
                p->type = CARD_BACKUP_TYPE_NOT_USE;
                p->spec.total_size = 0;
                cardi_common.cmd->result = CARD_RESULT_UNSUPPORTED;
                return;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IdentifyBackupCore

  Description:  デバイスタイプ特定処理本体。
                同期的あるいは非同期的に呼び出される。

  Arguments:    p          ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_IdentifyBackupCore(CARDiCommon * p)
{
    (void)CARDi_Request(p, CARD_REQ_IDENTIFY, 1);
    /*
     * 先頭 1 BYTE のリードコマンドを発行して結果値を取得する.
     * 接触不良, 破損, 寿命のいずれかであれば値に関わらず TIMEOUT が返る.
     * (TIMEOUT は, デバイス種別に関わらず Read-Status コマンドで判断可能)
     */
    p->cmd->src = 0;
    p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
    p->cmd->len = 1;
    (void)CARDi_Request(p, CARD_REQ_READ_BACKUP, 1);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_BeginBackupCommand

  Description:  バックアップ操作コマンドの開始処理

  Arguments:    callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_BeginBackupCommand(MIDmaCallback callback, void *arg)
{
    SDK_USING_BACKUP();
    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_BACKUP,
                  "[CARD] current locking target is not backup.");
    CARD_CheckEnabled();
    if ((CARDi_GetAccessLevel() & CARD_ACCESS_LEVEL_BACKUP) == 0)
    {
        OS_TPanic("this program cannot access CARD-backup!");
    }
    (void)CARDi_WaitForTask(&cardi_common, TRUE, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommand

  Description:  データを転送するコマンドのリクエストを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE
                req_type   コマンドリクエストタイプ
                req_retry  コマンドリクエストの失敗時リトライ最大回数
                req_mode   コマンドリクエストの動作モード

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL CARDi_RequestStreamCommand(u32 src, u32 dst, u32 len,
                                MIDmaCallback callback, void *arg, BOOL is_async,
                                CARDRequest req_type, int req_retry, CARDRequestMode req_mode)
{
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(callback, arg);

    {
        CARDiCommon *p = &cardi_common;
        p->src = src;
        p->dst = dst;
        p->len = len;
        p->req_type = req_type;
        p->req_retry = req_retry;
        p->req_mode = req_mode;
    }

    return CARDi_ExecuteOldTypeTask(CARDi_RequestStreamCommandCore, is_async);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatus

  Description:  ステータスリードまたはライト (テスト用)

  Arguments:    command    CARD_REQ_READ_STATUSまたはCARD_REQ_WRITE_STATUS
                value      CARD_REQ_WRITE_STATUSであれば書き込む値

  Returns:      成功すれば0以上の値、失敗すれば負の値
 *---------------------------------------------------------------------------*/
int CARDi_AccessStatus(CARDRequest command, u8 value)
{
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(NULL, NULL);

    // タスク引数がわりにテンポラリバッファを使用。
    CARDi_backup_cache_page_buf[0] = value;
    CARDi_backup_cache_page_buf[1] = (u8)command;

    return CARDi_ExecuteOldTypeTask(CARDi_AccessStatusCore, FALSE) ?
           CARDi_backup_cache_page_buf[0] : -1;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommand

  Description:  セクタ消去 + プログラムのリクエストを発行

  Arguments:    src        転送元メモリアドレス
                dst        転送先オフセット
                len        転送サイズ
                verify     ベリファイを行う場合は TRUE
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL CARDi_RequestWriteSectorCommand(u32 src, u32 dst, u32 len, BOOL verify,
                                     MIDmaCallback callback, void *arg, BOOL is_async)
{
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(callback, arg);

    {
        CARDiCommon *p = &cardi_common;
        p->src = src;
        p->dst = dst;
        p->len = len;
        p->req_mode = verify ? CARD_REQUEST_MODE_SEND_VERIFY : CARD_REQUEST_MODE_SEND;
    }

    return CARDi_ExecuteOldTypeTask(CARDi_RequestWriteSectorCommandCore, is_async);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IdentifyBackup

  Description:  カードのバックアップデバイス種別を指定

  Arguments:    type        CARDBackupType型のデバイス種別

  Returns:      読み出しテストに成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL CARD_IdentifyBackup(CARDBackupType type)
{
    if (type == CARD_BACKUP_TYPE_NOT_USE)
    {
        OS_TPanic("cannot specify CARD_BACKUP_TYPE_NOT_USE.");
    }

    CARDi_BeginBackupCommand(NULL, NULL);

    CARDi_IdentifyBackupCore2(type);

    return CARDi_ExecuteOldTypeTask(CARDi_IdentifyBackupCore, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCurrentBackupType

  Description:  現在指定しているバックアップデバイス種別を取得

  Arguments:    None.

  Returns:      現在指定しているバックアップデバイス種別
 *---------------------------------------------------------------------------*/
CARDBackupType CARD_GetCurrentBackupType(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->type;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupTotalSize

  Description:  現在指定しているバックアップデバイスの全体サイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスの全体サイズ
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupTotalSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.total_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupSectorSize

  Description:  現在指定しているバックアップデバイスのセクタサイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスのセクタサイズ
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupSectorSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.sect_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupPageSize

  Description:  現在指定しているバックアップデバイスのページサイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスのページサイズ
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupPageSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.page_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitBackupAsync

  Description:  バックアップデバイスアクセス関数が完了するまで待機

  Arguments:    None.

  Returns:      最後に呼び出したバックアップデバイスアクセス関数が
                CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
BOOL CARD_WaitBackupAsync(void)
{
    return CARDi_WaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitBackupAsync

  Description:  バックアップデバイスアクセス関数が完了したか判定

  Arguments:    None.

  Returns:      バックアップデバイスアクセス関数が完了していればTRUE
 *---------------------------------------------------------------------------*/
BOOL CARD_TryWaitBackupAsync(void)
{
    return CARDi_TryWaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_CancelBackupAsync

  Description:  処理中のバックアップデバイスアクセス関数に中止を要求する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_CancelBackupAsync(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    cardi_common.flag |= CARD_STAT_CANCEL;
    (void)OS_RestoreInterrupts(bak_cpsr);
}
