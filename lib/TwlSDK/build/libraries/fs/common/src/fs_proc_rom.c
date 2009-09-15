/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_proc_rom.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9544 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/fs/romfat.h>
#include <nitro/fs/api.h>
#include <nitro/card.h>

#ifdef SDK_TWL
#include <twl/fatfs.h>
#endif

#include "../include/command.h"
#include "../include/util.h"
#include "../include/rom.h"


#if defined(FS_IMPLEMENT)


/*---------------------------------------------------------------------------*/
/* variables */

static s32          fsi_card_lock_id;
static u32          fsi_default_dma_no;
static FSArchive    fsi_arc_rom;

/*---------------------------------------------------------------------------*/
/* function */

/*---------------------------------------------------------------------------*
  Name:         FSi_IsUnreadableRomOffset

  Description:  指定のROMオフセットが現在の環境で読めない範囲にあるか判定。

  Arguments:    arc                 呼び出し元アーカイブ。
                offset              判定するROMオフセット。

  Returns:      指定のROMオフセットが現在の環境で読めなければTRUE。
 *---------------------------------------------------------------------------*/
BOOL FSi_IsUnreadableRomOffset(FSArchive *arc, u32 offset)
{
    BOOL    result = FALSE;
#if !defined(SDK_TWL)
    (void)arc;
    (void)offset;
#else // defined(SDK_TWL)
    if (STD_CompareString(FS_GetArchiveName(arc), "rom") == 0)
    {
        if (!OS_IsRunOnTwl())
        {
            const CARDRomHeader    *header = CARD_GetOwnRomHeader();
            if (offset >= header->twl_ltd_area_rom_offset * 0x80000)
            {
                result = TRUE;
            }
        }
    }
#endif // defined(SDK_TWL)
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_OnRomReadDone

  Description:  ROM非同期読み込み完了コールバック

  Arguments:    arg              ユーザ指定のコールバック引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_OnRomReadDone(void *arg)
{
    FS_NotifyArchiveAsyncEnd((FSArchive *)arg,
                             CARD_IsPulledOut() ? FS_RESULT_ERROR : FS_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadRomCallback

  Description:  ROM読み込みコールバック

  Arguments:    p_arc            FSArchive構造体
                dst              転送先
                src              転送元
                len              転送サイズ

  Returns:      読み込み処理の結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadRomCallback(FSArchive *p_arc, void *dst, u32 src, u32 len)
{
    CARD_ReadRomAsync(fsi_default_dma_no, (const void *)src, dst, len, FSi_OnRomReadDone, p_arc);
    return FS_RESULT_PROC_ASYNC;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_RomArchiveProc

  Description:  ROMアーカイブのプロシージャ

  Arguments:    p_file           コマンド情報を格納した FSFile 構造体
                cmd              コマンドタイプ

  Returns:      コマンド処理結果.
 *---------------------------------------------------------------------------*/
static FSResult FSi_RomArchiveProc(FSFile *p_file, FSCommandType cmd)
{
    (void)p_file;
    switch (cmd)
    {
    case FS_COMMAND_ACTIVATE:
        CARD_LockRom((u16)fsi_card_lock_id);
        return FS_RESULT_SUCCESS;
    case FS_COMMAND_IDLE:
        CARD_UnlockRom((u16)fsi_card_lock_id);
        return FS_RESULT_SUCCESS;
#if defined(SDK_TWL)
    case FS_COMMAND_OPENFILEDIRECT:
        // NTRモードで動作中ならTWL専用領域が読み出せないためエラーとする。
        if (FSi_IsUnreadableRomOffset(FS_GetAttachedArchive(p_file), p_file->arg.openfiledirect.top))
        {
            OS_TWarning("specified file can be read on TWL only!(fileID=%d)\n", p_file->arg.openfiledirect.index);
            return FS_RESULT_PERMISSION_DENIED;
        }
        return FS_RESULT_PROC_DEFAULT;
#endif
    case FS_COMMAND_WRITEFILE:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EmptyArchiveProc

  Description:  何もしない空のアーカイブのプロシージャ

  Arguments:    p_file           コマンド情報を格納した FSFile 構造体
                cmd              コマンドタイプ

  Returns:      コマンド処理結果.
 *---------------------------------------------------------------------------*/
static FSResult FSi_EmptyArchiveProc(FSFile *p_file, FSCommandType cmd)
{
    (void)p_file;
    switch (cmd)
    {
    case FS_COMMAND_WRITEFILE:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
}
static FSResult FSi_ReadDummyCallback(FSArchive *p_arc, void *dst, u32 src, u32 len)
{
    (void)p_arc;
    (void)dst;
    (void)src;
    (void)len;
    return FS_RESULT_UNSUPPORTED;
}
static FSResult FSi_WriteDummyCallback(FSArchive *p_arc, const void *src, u32 dst, u32 len)
{
    (void)p_arc;
    (void)src;
    (void)dst;
    (void)len;
    return FS_RESULT_UNSUPPORTED;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_OverrideRomArchive

  Description:  必要に応じてROMアーカイブの内容を置き換える。

  Arguments:    arc              マウントすべきROMアーカイブ構造体

  Returns:      カード上にある標準のROMアーカイブを使用せず
                別の実装に置き換えるべきであればTRUEを返す必要がある。
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL BOOL FSi_OverrideRomArchive(FSArchive *arc) __attribute__((never_inline))
{
    (void)arc;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_InitRomArchive

  Description:  Initialize "rom" archive

  Arguments:    default_dma_no   default DMA channel for ROM
                                 if out of range(0-3),
                                 use CpuCopy instead of DMA.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_InitRomArchive(u32 default_dma_no)
{
    CARD_Init();

    fsi_default_dma_no = default_dma_no;
    fsi_card_lock_id = OS_GetLockID();

    FS_InitArchive(&fsi_arc_rom);
    (void)FS_RegisterArchiveName(&fsi_arc_rom, "rom", 3);

    // 種別がカードブートならカード上に存在するROMファイルシステムをマウント。
    if (OS_GetBootType() == OS_BOOTTYPE_ROM)
    {
        const CARDRomRegion *const fnt = CARD_GetRomRegionFNT();
        const CARDRomRegion *const fat = CARD_GetRomRegionFAT();
        FS_SetArchiveProc(&fsi_arc_rom, FSi_RomArchiveProc,
                          FS_ARCHIVE_PROC_WRITEFILE |
                          FS_ARCHIVE_PROC_OPENFILEDIRECT |
                          FS_ARCHIVE_PROC_ACTIVATE | FS_ARCHIVE_PROC_IDLE);
        if ((fnt->offset == 0xFFFFFFFF) || (fnt->offset == 0x00000000) ||
            (fat->offset == 0xFFFFFFFF) || (fat->offset == 0x00000000))
        {
        }
        else
        {
            (void)FS_LoadArchive(&fsi_arc_rom, 0x00000000,
                                 fat->offset, fat->length,
                                 fnt->offset, fnt->length,
                                 FSi_ReadRomCallback, NULL);
        }
    }
    // その他、置換すべきROMの存在しない通常プログラムなら空のアーカイブをマウント。
    else if (!FSi_OverrideRomArchive(&fsi_arc_rom))
    {
    }
    if (!FS_IsArchiveLoaded(&fsi_arc_rom))
    {
        FS_SetArchiveProc(&fsi_arc_rom, FSi_EmptyArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
        (void)FS_LoadArchive(&fsi_arc_rom, 0x00000000,
                             0, 0, 0, 0, FSi_ReadDummyCallback, FSi_WriteDummyCallback);
    }

    (void)FS_SetCurrentDirectory("rom:");
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EndRomArchive

  Description:  "rom" アーカイブを解放

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_EndRomArchive(void)
{
    if (FS_IsArchiveLoaded(&fsi_arc_rom))
    {
        (void)FS_UnloadArchive(&fsi_arc_rom);
        FS_ReleaseArchiveName(&fsi_arc_rom);
    }
    OS_ReleaseLockID((u16)fsi_card_lock_id);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetDefaultDMA

  Description:  get default DMA channel

  Arguments:    None.

  Returns:      current DMA channel.
 *---------------------------------------------------------------------------*/
u32 FS_GetDefaultDMA(void)
{
    return fsi_default_dma_no;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetDefaultDMA

  Description:  set default DMA channel

  Arguments:    dma_no           default dma channel for FS
                                 if out of range(0-3),
                                 use CpuCopy instead of DMA.

  Returns:      previous DMA channel.
 *---------------------------------------------------------------------------*/
u32 FS_SetDefaultDMA(u32 dma_no)
{
    SDK_ASSERTMSG(dma_no != 0, "cannot use DMA channel 0 for ROM access");
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        u32     bak_dma_no = fsi_default_dma_no;
        BOOL    bak_stat = FS_SuspendArchive(&fsi_arc_rom);
        fsi_default_dma_no = dma_no;
        if (bak_stat)
        {
            (void)FS_ResumeArchive(&fsi_arc_rom);
        }
        (void)OS_RestoreInterrupts(bak_psr);
        return bak_dma_no;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_TryLoadTable

  Description:  preload FAT and FNT tables of file-system.
                if specified size is enough to load tables, load it to p_mem.
                then, all the operation using FAT and FNT is with p_mem.

  Arguments:    p_mem       buffer to preload
                size        byte length of p_mem

  Returns:      true size of memory for FAT and FNT.
 *---------------------------------------------------------------------------*/
u32 FS_TryLoadTable(void *p_mem, u32 size)
{
    return FS_LoadArchiveTables(&fsi_arc_rom, p_mem, size);
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromRom

  Description:  指定の CARD-ROM 領域をマップしたファイルを一時的に生成

  Arguments:    p_file           ファイルハンドルを格納する FSFile 構造体
                offset           READ の対象となる CARD-ROM 領域先頭オフセット
                size             対象領域の offset からのバイトサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileFromRom(FSFile *p_file, u32 offset, u32 size)
{
    return FS_OpenFileDirect(p_file, &fsi_arc_rom, offset, offset + size, 0xFFFF);
}

#else /* FS_IMPLEMENT */

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadRomDirect

  Description:  指定のROMアドレスを直接リード

  Arguments:    src              転送元オフセット
                dst              転送先アドレス
                len              転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ReadRomDirect(const void *src, void *dst, u32 len)
{
    u16     id = (u16)OS_GetLockID();
    (void)CARD_LockRom(id);
    CARD_ReadRom(4, src, dst, len);
    (void)CARD_UnlockRom(id);
    OS_ReleaseLockID(id);
}


#endif /* FS_IMPLEMENT */
