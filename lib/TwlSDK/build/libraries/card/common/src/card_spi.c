/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_spi.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_spi.h"

/* CARD-SPI の制御 */


/*---------------------------------------------------------------------------*/
/* constants */

/* reg_MI_MCCNT0 各ビット*/

#define MCCNT0_SPI_CLK_MASK	0x0003 /* ボーレート設定マスク */
#define MCCNT0_SPI_CLK_4M	0x0000 /* ボーレート 4.19MHz */
#define MCCNT0_SPI_CLK_2M	0x0001 /* ボーレート 2.10MHz */
#define MCCNT0_SPI_CLK_1M	0x0002 /* ボーレート 1.05MHz */
#define MCCNT0_SPI_CLK_524K	0x0003 /* ボーレート 524kHz */
#define MCCNT0_SPI_BUSY		0x0080 /* SPI ビジーフラグ */
#define	MMCNT0_SEL_MASK		0x2000 /* CARD ROM / SPI 選択マスク */
#define	MMCNT0_SEL_CARD		0x0000 /* CARD ROM 選択 */
#define	MMCNT0_SEL_SPI		0x2000 /* CARD SPI 選択 */
#define MCCNT0_INT_MASK		0x4000 /* 転送完了割り込みマスク */
#define MCCNT0_INT_ON		0x4000 /* 転送完了割り込み有効 */
#define MCCNT0_INT_OFF		0x0000 /* 転送完了割り込み無効 */
#define MCCNT0_MASTER_MASK	0x8000 /* CARD マスターマスク */
#define MCCNT0_MASTER_ON	0x8000 /* CARD マスター ON */
#define MCCNT0_MASTER_OFF	0x0000 /* CARD マスター OFF */


/*---------------------------------------------------------------------------*/
/* variables */

typedef struct
{                                      /* SPI 内部管理パラメータ. */
    u32     rest_comm;                 /* 全送信バイト数の残り. */
    u32     src;                       /* 転送元 */
    u32     dst;                       /* 転送先 */
    BOOL    cmp;                       /* 比較結果 */
}
CARDiParam;

static CARDiParam cardi_param;


/*---------------------------------------------------------------------------*/
/* functions */

static BOOL CARDi_CommandCheckBusy(void);
static void CARDi_CommArray(const void *src, void *dst, u32 n, void (*func) (CARDiParam *));
static void CARDi_CommReadCore(CARDiParam * p);
static void CARDi_CommWriteCore(CARDiParam * p);
static void CARDi_CommVerifyCore(CARDiParam * p);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayRead

  Description:  リードコマンドの後続読み出し処理.

  Arguments:    dst               読み出し先メモリ.
                len               読み出しサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayRead(void *dst, u32 len)
{
    CARDi_CommArray(NULL, dst, len, CARDi_CommReadCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayWrite

  Description:  ライトコマンドの後続書き込み処理.

  Arguments:    dst               書き込み元メモリ.
                len               書き込みサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayWrite(const void *src, u32 len)
{
    CARDi_CommArray(src, NULL, len, CARDi_CommWriteCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayVerify

  Description:  (比較のための)リードコマンドの後続比較処理.

  Arguments:    src               比較元メモリ.
                len               比較サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayVerify(const void *src, u32 len)
{
    CARDi_CommArray(src, NULL, len, CARDi_CommVerifyCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EnableSpi

  Description:  CARD-SPI を有効にする.

  Arguments:    cont              連続転送フラグ. (CSPI_CONTINUOUS_ON or OFF)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_EnableSpi(u32 cont)
{
    /*
     * 将来クロックスピードが遅いデバイスが登場したら
     * MCCNT0_SPI_CLK_4M をプロパティ配列に追加して動的変更.
     */
    const u16 ctrl = (u16)(MCCNT0_MASTER_ON |
                           MCCNT0_INT_OFF | MMCNT0_SEL_SPI | MCCNT0_SPI_CLK_4M | cont);
    reg_MI_MCCNT0 = ctrl;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitBusy

  Description:  CARD-SPI の完了を待つ.

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_WaitBusy(void)
{
    while ((reg_MI_MCCNT0 & MCCNT0_SPI_BUSY) != 0)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandBegin

  Description:  コマンド発行開始の宣言.

  Arguments:    len               発行するコマンド長.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CommandBegin(int len)
{
    cardi_param.rest_comm = (u32)len;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandEnd

  Description:  コマンド送信完了処理.

  Arguments:    force_wait     強制ウェイト時間 [ms]
                timeout        タイムアウト時間 [ms]

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CommandEnd(u32 force_wait, u32 timeout)
{
    if (force_wait + timeout > 0)
    {
        /*
         * 強制ウェイト時間があればその分だけ待機.
         * 最初の話と違って FLASH もウェイトが強制された.
         */
        if (force_wait > 0)
        {
            OS_Sleep(force_wait);
        }
        if (timeout > 0)
        {
            /*
             * PageWrite コマンドのみ「前半だけ強制ウェイト」
             * なのでこのような特殊なループになる.
             */
            int     rest = (int)(timeout - force_wait);
            while (!CARDi_CommandCheckBusy() && (rest > 0))
            {
                int     interval = (rest < 5) ? rest : 5;
                OS_Sleep((u32)interval);
                rest -= interval;
            }
        }
        /*
         * その他のコマンドは指定時間ウェイト済みなので
         * ReadStatus は 1 回だけで良い.
         */
        /* ここまできてビジーならタイムアウト */
        if (!CARDi_CommandCheckBusy())
        {
            cardi_common.cmd->result = CARD_RESULT_TIMEOUT;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandReadStatus

  Description:  リードステータス

  Arguments:    None

  Returns:      ステータス値
 *---------------------------------------------------------------------------*/
u8 CARDi_CommandReadStatus(void)
{
    const u8    buf = COMM_READ_STATUS;
    u8          dst;
    CARDi_CommandBegin(1 + 1);
    CARDi_CommArrayWrite(&buf, 1);
    CARDi_CommArrayRead(&dst, 1);
    CARDi_CommandEnd(0, 0);
    return dst;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandCheckBusy

  Description:  デバイスのビジーが解消されたかリードステータスコマンドで判定.

  Arguments:    None

  Returns:      ビジーでなければ TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_CommandCheckBusy(void)
{
    return ((CARDi_CommandReadStatus() & COMM_STATUS_WIP_BIT) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitPrevCommand

  Description:  コマンド発行前のビジーチェック.
                前回のコマンドによってビジー状態であれば 50[ms] まで待つ.
                (ライブラリの実装とカードの接触が正常な限り通常ありえない)

  Arguments:    None.

  Returns:      コマンドが正しく発行可能であれば TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_WaitPrevCommand(void)
{
    CARDi_CommandEnd(0, 50);
    /* ここでタイムアウトの場合は無応答 */
    if (cardi_common.cmd->result == CARD_RESULT_TIMEOUT)
    {
        cardi_common.cmd->result = CARD_RESULT_NO_RESPONSE;
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArray

  Description:  コマンド発行の共通処理.

  Arguments:    src               処理元のメモリ. (不使用ならNULL)
                dst               処理先のメモリ. (不使用ならNULL)
                len               処理サイズ.
                func              処理内容.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommArray(const void *src, void *dst, u32 len, void (*func) (CARDiParam *))
{
    CARDiParam *const p = &cardi_param;
    p->src = (u32)src;
    p->dst = (u32)dst;
    CARDi_EnableSpi(CSPI_CONTINUOUS_ON);
    for (; len > 0; --len)
    {
        if (!--p->rest_comm)
        {
            CARDi_EnableSpi(CSPI_CONTINUOUS_OFF);
        }
        CARDi_WaitBusy();
        (*func) (p);
    }
    if (!p->rest_comm)
    {
        reg_MI_MCCNT0 = (u16)(MCCNT0_MASTER_OFF | MCCNT0_INT_OFF);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommReadCore

  Description:  1バイトリード処理.

  Arguments:    p                 コマンドパラメータ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommReadCore(CARDiParam * p)
{
    reg_MI_MCD0 = 0;
    CARDi_WaitBusy();
    MI_WriteByte((void *)p->dst, (u8)reg_MI_MCD0);
    ++p->dst;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommWriteCore

  Description:  1バイトライト処理.

  Arguments:    p                 コマンドパラメータ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommWriteCore(CARDiParam * p)
{
    vu16    tmp;
    reg_MI_MCD0 = (u16)MI_ReadByte((void *)p->src);
    ++p->src;
    CARDi_WaitBusy();
    tmp = reg_MI_MCD0;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommVerifyCore

  Description:  1バイト比較処理.

  Arguments:    p                 コマンドパラメータ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommVerifyCore(CARDiParam * p)
{
    reg_MI_MCD0 = 0;
    CARDi_WaitBusy();
    /*
     * リードして一致しなければ中断.
     * ただし連続転送を抜けなければいけないので
     * 少なくとも 1 回だけ余計に読む必要がある.
     */
    if ((u8)reg_MI_MCD0 != MI_ReadByte((void *)p->src))
    {
        p->cmp = FALSE;
        if (p->rest_comm > 1)
        {
            p->rest_comm = 1;
        }
    }
    ++p->src;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteEnable

  Description:  デバイスの書き込み操作有効化. (Write 系コマンドの前に毎回必要)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_WriteEnable(void)
{
    static const u8 arg[1] = { COMM_WRITE_ENABLE, };
    CARDi_CommandBegin(sizeof(arg));
    CARDi_CommArrayWrite(arg, sizeof(arg));
    CARDi_CommandEnd(0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SendSpiAddressingCommand

  Description:  アドレス指定コマンドの設定.

  Arguments:    addr              捜査対象となるデバイス上のアドレス.
                mode              発行するコマンド.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SendSpiAddressingCommand(u32 addr, u32 mode)
{
    const u32 width = cardi_common.cmd->spec.addr_width;
    u32     addr_cmd;
    switch (width)
    {
    case 1:
        /* 4kbit デバイスは [A:8] がコマンドの一部になる */
        addr_cmd = (u32)(mode | ((addr >> 5) & 0x8) | ((addr & 0xFF) << 8));
        break;
    case 2:
        addr_cmd = (u32)(mode | (addr & 0xFF00) | ((addr & 0xFF) << 16));
        break;
    case 3:
        addr_cmd = (u32)(mode |
                         ((addr >> 8) & 0xFF00) | ((addr & 0xFF00) << 8) | ((addr & 0xFF) << 24));
        break;
    default:
        SDK_ASSERT(FALSE);
        break;
    }
    CARDi_CommArrayWrite(&addr_cmd, (u32)(1 + width));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitStatusRegister

  Description:  FRAM/EEPROM の場合, 起動時にステータスレジスタの補正処理を行う.
                ・FRAM は電源投入時にライトプロテクトの変化が起こりうるため.
                ・EEPROM は納入時に初期値不正がありうるため.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitStatusRegister(void)
{
    /*
     * ステータスレジスタが異常な初期値を取りうるデバイスに対しては
     * 初回使用時に適宜補正する.
     */
    const u8 stat = cardi_common.cmd->spec.initial_status;
    if (stat != 0xFF)
    {
        static BOOL status_checked = FALSE;
        if (!status_checked)
        {
            if (CARDi_CommandReadStatus() != stat)
            {
                CARDi_SetWriteProtectCore(stat);
            }
            status_checked = TRUE;
        }
    }
}


/********************************************************************/
/*
 * 内部直接処理.
 * この段階ではすでに排他やリクエスト等が全て完了している.
 * サイズの制限についてもすでに考慮済みのものとする.
 */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackupCore

  Description:  デバイスへのリードコマンド全体.

  Arguments:    src               読み出し元のデバイスオフセット.
                dst               読み出し先のメモリアドレス.
                len               読み出しサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadBackupCore(u32 src, void *dst, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* リードに関してはページ境界の制限が無いので一括処理 */
        CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
        CARDi_SendSpiAddressingCommand(src, COMM_READ_ARRAY);
        CARDi_CommArrayRead(dst, len);
        CARDi_CommandEnd(0, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackupCore

  Description:  デバイスへのプログラム(消去無し書き込み)コマンド全体.

  Arguments:    dst               書き込み先のデバイスオフセット.
                src               書き込み元のメモリアドレス.
                len               書き込みサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* ライトはページ境界をまたがないように分割処理 */
        const u32 page = cmd->spec.page_size;
        while (len > 0)
        {
            const u32 mod = (u32)(dst & (page - 1));
            u32     size = page - mod;
            if (size > len)
            {
                size = len;
            }
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
            CARDi_SendSpiAddressingCommand(dst, COMM_PROGRAM_PAGE);
            CARDi_CommArrayWrite(src, size);
            CARDi_CommandEnd(cmd->spec.program_page, 0);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            src = (const void *)((u32)src + size);
            dst += size;
            len -= size;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackupCore

  Description:  デバイスへのライト(消去 + プログラム)コマンド全体.

  Arguments:    dst               書き込み先のデバイスオフセット.
                src               書き込み元のメモリアドレス.
                len               書き込みサイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_WriteBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* ライトはページ境界をまたがないように分割処理 */
        const u32 page = cmd->spec.page_size;
        while (len > 0)
        {
            const u32 mod = (u32)(dst & (page - 1));
            u32     size = page - mod;
            if (size > len)
            {
                size = len;
            }
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
            CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_WRITE);
            CARDi_CommArrayWrite(src, size);
            CARDi_CommandEnd(cmd->spec.write_page, cmd->spec.write_page_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            src = (const void *)((u32)src + size);
            dst += size;
            len -= size;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackupCore

  Description:  デバイスへのベリファイ(実際はリード + 比較処理)コマンド全体.

  Arguments:    dst               比較先のデバイスオフセット.
                src               比較元のメモリアドレス.
                len               比較サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* リードに関してはページ境界の制限が無いので一括処理 */
        cardi_param.cmp = TRUE;
        CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
        CARDi_SendSpiAddressingCommand(dst, COMM_READ_ARRAY);
        CARDi_CommArrayVerify(src, len);
        CARDi_CommandEnd(0, 0);
        if ((cmd->result == CARD_RESULT_SUCCESS) && !cardi_param.cmp)
        {
            cmd->result = CARD_RESULT_FAILURE;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSectorCore

  Description:  デバイスへのセクタ消去コマンド全体.

  Arguments:    dst               消去先のデバイスオフセット.
                len               消去サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseBackupSectorCore(u32 dst, u32 len)
{
    CARDiCommandArg *const cmd = cardi_common.cmd;
    const u32 sector = cmd->spec.sect_size;
    /* 処理範囲がセクタの整数倍に整合していない場合は処理をしない */
    if (((dst | len) & (sector - 1)) != 0)
    {
        cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else if (CARDi_WaitPrevCommand())
    {
        /* セクタ境界単位で処理 */
        while (len > 0)
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
            CARDi_SendSpiAddressingCommand(dst, COMM_SECTOR_ERASE);
            CARDi_CommandEnd(cmd->spec.erase_sector, cmd->spec.erase_sector_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            dst += sector;
            len -= sector;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSectorCore

  Description:  デバイスへのサブセクタ消去コマンド全体.

  Arguments:    dst               消去先のデバイスオフセット.
                len               消去サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseBackupSubSectorCore(u32 dst, u32 len)
{
    CARDiCommandArg *const cmd = cardi_common.cmd;
    const u32 sector = cmd->spec.subsect_size;
    /* 処理範囲がサブセクタの整数倍に整合していない場合は処理をしない */
    if (((dst | len) & (sector - 1)) != 0)
    {
        cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else if (CARDi_WaitPrevCommand())
    {
        /* セクタ境界単位で処理 */
        while (len > 0)
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
            CARDi_SendSpiAddressingCommand(dst, COMM_SUBSECTOR_ERASE);
            CARDi_CommandEnd(cmd->spec.erase_subsector, cmd->spec.erase_subsector_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            dst += sector;
            len -= sector;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseChipCore

  Description:  デバイスへのチップ消去コマンド全体.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseChipCore(void)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        static const u8 arg[1] = { COMM_CHIP_ERASE, };
        CARDi_WriteEnable();
        CARDi_CommandBegin(sizeof(arg));
        CARDi_CommArrayWrite(arg, sizeof(arg));
        CARDi_CommandEnd(cmd->spec.erase_chip, cmd->spec.erase_chip_total);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetWriteProtectCore

  Description:  デバイスへのライトプロテクトコマンド全体.

  Arguments:    stat              設定するプロテクトフラグ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetWriteProtectCore(u16 stat)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        int     retry_count = 10;
        u8      arg[2];
        arg[0] = COMM_WRITE_STATUS;
        arg[1] = (u8)stat;
        do
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin(1 + 1);
            CARDi_CommArrayWrite(&arg, sizeof(arg));
            CARDi_CommandEnd(5, 0);
        }
        while ((cmd->result == CARD_RESULT_TIMEOUT) && (--retry_count > 0));
    }
}


#if	0

/********************************************************************/
/********************************************************************/
/* 検証中デバイス固有コマンド ***************************************/
/********************************************************************/
/********************************************************************/


/* ID リード */
static void CARDi_ReadIdCore(void)
{
    /*
     * EEPROM, FRAM にはこのコマンドが無い.
     * ST 製 2Mbit FLASH にもこのコマンドが無い?
     * 対応外のコマンドバイト送出時の動作定義が見当たらない.
     * とすると, このコマンドは 2Mbit 時は常に使用不可とするか
     * CARD_BACKUP_TYPE_FLASH_2MBITS_SANYO などと細分化するか.
     * いずれにせよそこまで事前に区分しているのに
     * ID が必要になるシーンがあるのだろうか...?
     * 余力があれば内部で正当性判定に使用予定.
     */
    cardi_common.cmd->result = CARD_RESULT_UNSUPPORTED;
}

/* 2M FLASH のみ使用可能なコマンド **********************************/

/* ページ消去 (FLASH) */
static void CARDi_ErasePageCore(u32 dst)
{
    CARDi_WriteEnable();
    CARDi_CommandBegin(1 + cardi_common.cmd->spec.addr_width + 0);
    CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_ERASE);
    CARDi_CommandEnd(cardi_common.cmd->spec.erase_page, 0);
}


#endif
