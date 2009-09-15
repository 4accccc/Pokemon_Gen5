/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_spi.h

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
#ifndef NITRO_LIBRARIES_CARD_SPI_H__
#define NITRO_LIBRARIES_CARD_SPI_H__


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* constants */

#define	CSPI_CONTINUOUS_ON	0x0040
#define	CSPI_CONTINUOUS_OFF	0x0000

/* コマンド */
#define COMM_WRITE_STATUS	0x01   /* ステータスライト. */
#define COMM_PROGRAM_PAGE	0x02   /* ページプログラム. */
#define COMM_READ_ARRAY		0x03   /* リード. */
#define COMM_WRITE_DISABLE	0x04   /* ライトディセーブル. (未使用) */
#define COMM_READ_STATUS	0x05   /* ステータスリード. */
#define COMM_WRITE_ENABLE	0x06   /* ライトイネーブル. */

/* FLASH */
#define COMM_PAGE_WRITE		0x0A
#define COMM_PAGE_ERASE		0xDB
#define COMM_SECTOR_ERASE	0xD8
#define COMM_SUBSECTOR_ERASE	0x20
#define COMM_CHIP_ERASE		0xC7
#define CARDFLASH_READ_BYTES_FAST	0x0B    /* 未使用 */
#define CARDFLASH_DEEP_POWDOWN		0xB9    /* 未使用 */
#define CARDFLASH_WAKEUP			0xAB    /* 未使用 */

/* ステータスレジスタのビット */
#define	COMM_STATUS_WIP_BIT		0x01    /* WriteInProgress (bufy) */
#define	COMM_STATUS_WEL_BIT		0x02    /* WriteEnableLatch */
#define	COMM_STATUS_BP0_BIT		0x04
#define	COMM_STATUS_BP1_BIT		0x08
#define	COMM_STATUS_BP2_BIT		0x10
#define COMM_STATUS_WPBEN_BIT	0x80


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitStatusRegister

  Description:  FRAM/EEPROM の場合は起動時にステータスレジスタの補正処理を行う。
                (FRAMは電源投入時にライトプロテクトの変化が起こりうるため)
                (EEPROMは納入時に初期値不正がありうるため)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_InitStatusRegister(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandReadStatus

  Description:  リードステータス

  Arguments:    None

  Returns:      ステータス値
 *---------------------------------------------------------------------------*/
u8 CARDi_CommandReadStatus(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackupCore

  Description:  デバイスへのリードコマンド全体

  Arguments:    src               読み出し元のデバイスオフセット
                dst               読み出し先のメモリアドレス
                len               読み出しサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadBackupCore(u32 src, void *dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackupCore

  Description:  デバイスへのプログラム(消去無し書き込み)コマンド全体

  Arguments:    dst               書き込み先のデバイスオフセット
                src               書き込み元のメモリアドレス
                len               書き込みサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackupCore

  Description:  デバイスへのライト(消去 + プログラム)コマンド全体

  Arguments:    dst               書き込み先のデバイスオフセット
                src               書き込み元のメモリアドレス
                len               書き込みサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_WriteBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackupCore

  Description:  デバイスへのベリファイ(実際はリード + 比較処理)コマンド全体

  Arguments:    dst               比較先のデバイスオフセット
                src               比較元のメモリアドレス
                len               比較サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSectorCore

  Description:  デバイスへのセクタ消去コマンド全体

  Arguments:    dst               消去先のデバイスオフセット
                len               消去サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseBackupSectorCore(u32 dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSectorCore

  Description:  デバイスへのサブセクタ消去コマンド全体.

  Arguments:    dst               消去先のデバイスオフセット.
                len               消去サイズ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseBackupSubSectorCore(u32 dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseChipCore

  Description:  デバイスへのチップ消去コマンド全体

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseChipCore(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetWriteProtectCore

  Description:  デバイスへのライトプロテクトコマンド全体

  Arguments:    stat              設定するプロテクトフラグ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_SetWriteProtectCore(u16 stat);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_SPI_H__
