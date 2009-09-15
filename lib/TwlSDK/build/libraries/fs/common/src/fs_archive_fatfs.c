/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_archive_fatfs.c

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

#ifdef SDK_TWL

#include <nitro/fs.h>
#include <nitro/os/ARM9/tcm.h>
#include <nitro/os/ARM9/cache.h>
#include <nitro/math/math.h>
#include <nitro/std/string.h>
#include <nitro/gx/gx_vramcnt.h>
#include <nitro/spi/ARM9/pm.h>
#include <twl/fatfs.h>
#include <twl/mi/common/sharedWram.h>

#include "../include/util.h"
#include "../include/command.h"
#include "../include/rom.h"


#if defined(FS_IMPLEMENT)


#include <twl/ltdmain_begin.h>


/*---------------------------------------------------------------------------*/
/* constants */

// パスのエントリ名に使用できない文字群。
extern const u16  *FSiPathInvalidCharactersW;
const u16  *FSiPathInvalidCharactersW = L":*?\"<>|";


/*---------------------------------------------------------------------------*/
/* variables */

static int                      FSiSwitchableWramSlots = 0;

static FSFATFSArchiveContext    FSiFATFSDriveDefault[FS_MOUNTDRIVE_MAX] ATTRIBUTE_ALIGN(32);
static FATFSRequestBuffer       FSiFATFSAsyncRequestDefault[FS_MOUNTDRIVE_MAX] ATTRIBUTE_ALIGN(32);
static u8                       FSiTemporaryBufferDefault[FS_TEMPORARY_BUFFER_MAX] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* function */

/*---------------------------------------------------------------------------*
 * カスタマイズに関する実装
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FSiFATFSDrive

  Description:  FATベースのアーカイブを管理するコンテキスト配列。
                通常はFS_MOUNTDRIVE_MAX個だけの配列サイズを必要とし、
                デフォルトではLTDMAINセグメント内の静的変数が使用される。
                特殊なメモリ配置で構築されたアプリケーションでは
                オーバーライドしたFSi_SetupFATBuffers関数の呼び出し内部で
                この変数を変更して適切なバッファを設定することができる。

  Arguments:    None.

  Returns:      FSFATFSArchiveContext構造体FS_MOUNTDRIVE_MAX個分のバッファ
 *---------------------------------------------------------------------------*/
FSFATFSArchiveContext *FSiFATFSDrive = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSiFATFSAsyncRequest

  Description:  FATベースのアーカイブで非同期処理に使用するコマンドバッファ。
                通常はFS_MOUNTDRIVE_MAX個だけの配列サイズを必要とし、
                デフォルトではLTDMAINセグメント内の静的変数が使用される。
                特殊なメモリ配置で構築されたアプリケーションでは
                オーバーライドしたFSi_SetupFATBuffers関数の呼び出し内部で
                この変数を変更して適切なバッファを設定することができる。

  Arguments:    None.

  Returns:      FS_TEMPORARY_BUFFER_MAXバイトの静的なコマンドバッファ
 *---------------------------------------------------------------------------*/
FATFSRequestBuffer *FSiFATFSAsyncRequest = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSiTemporaryBuffer

  Description:  Read/Writeコマンド発行用の静的な一時バッファへのポインタ。
                ARM9/ARM7ともに参照可能なメモリである必要があり、
                デフォルトではLTDMAINセグメント内の静的変数が使用される。
                特殊なメモリ配置で構築されたアプリケーションでは
                この変数をFS_Init関数の呼び出しより前に変更することで
                適切なバッファを設定することができる。

  Arguments:    None.

  Returns:      FS_TEMPORARY_BUFFER_MAXバイトの静的なコマンドバッファ
 *---------------------------------------------------------------------------*/
u8 *FSiTemporaryBuffer = NULL;

/*---------------------------------------------------------------------------*
  Name:         FSi_SetupFATBuffers

  Description:  FATベースのアーカイブに必要な各種バッファを初期化。
                特殊なメモリ配置で構築されたアプリケーションでは
                この関数をオーバーライドして各種バッファを独自に設定し
                所要メモリサイズを必要最低限に抑止することができる。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void FSi_SetupFATBuffers(void)
{
    FSiFATFSDrive = FSiFATFSDriveDefault;
    FSiFATFSAsyncRequest = FSiFATFSAsyncRequestDefault;
    FSiTemporaryBuffer = FSiTemporaryBufferDefault;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsValidAddressForARM7

  Description:  ARM7からアクセス可能なバッファか判定する。
                特殊なメモリ配置で構築されたアプリケーションでは
                この関数をオーバーライドすることで
                適切な判定ルーチンを再定義することができる。

  Arguments:    buffer              判定すべきバッファ
                length              バッファ長

  Returns:      ARM7からアクセス可能なバッファであればTRUE
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length)
 __attribute__((never_inline))
{
    u32     addr = (u32)buffer;
    u32     dtcm = OS_GetDTCMAddress();
    if ((addr + length > dtcm) && (addr < dtcm + HW_DTCM_SIZE))
    {
        return FALSE;
    }
    if ((addr >= HW_TWL_MAIN_MEM) && (addr + length <= HW_TWL_MAIN_MEM_END))
    {
        return TRUE;
    }
    if ((addr >= HW_EXT_WRAM_ARM7) && (addr + length <= GX_GetSizeOfARM7()))
    {
        return TRUE;
    }
    // スロット境界をまたがない切り替え可能なWRAMなら許可。
    if ((addr >= HW_WRAM_AREA) && (addr <= HW_WRAM_AREA_END) &&
        (MATH_ROUNDDOWN(addr, MI_WRAM_B_SLOT_SIZE) + MI_WRAM_B_SLOT_SIZE ==
         MATH_ROUNDUP(addr + length, MI_WRAM_B_SLOT_SIZE)))
    {
        MIWramPos   type;
        int         slot = MIi_AddressToWramSlot(buffer, &type);
        if (slot >= 0)
        {
            if ((((FSiSwitchableWramSlots >> ((type == MI_WRAM_B) ? 0 : 8)) & (1 << slot)) != 0) &&
                // 勝手に切り替えて必ずしも成功するとは限らない場合にはアプリケーションが保証する。
                ((MI_GetWramBankMaster(type, slot)  == MI_WRAM_ARM7) ||
                 (MI_SwitchWramSlot(type, slot, MI_WRAM_SIZE_32KB,
                                    MI_GetWramBankMaster(type, slot), MI_WRAM_ARM7) >= 0)))
            {
                
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SetSwitchableWramSlots

  Description:  状況に応じてFSライブラリがARM7へ切り替えてよいWRAMスロットを指定。

  Arguments:    bitsB               WRAM-Bスロットのビット集合
                bitsC               WRAM-Cスロットのビット集合

  Returns:      None
 *---------------------------------------------------------------------------*/
void FSi_SetSwitchableWramSlots(int bitsB, int bitsC)
{
    FSiSwitchableWramSlots = (bitsB << 0) | (bitsC << 8);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_AllocUnicodeFullPath

  Description:  アーカイブ名と相対パスを連結したフルパスを取得。

  Arguments:    context             FSFATFSArchiveContext構造体
                relpath             アーカイブ以下の相対パス

  Returns:      アーカイブ名から始まるUnicodeのフルパスを格納したバッファ。
 *---------------------------------------------------------------------------*/
static u16* FSi_AllocUnicodeFullPath(FSFATFSArchiveContext *context, const char *relpath)
{
    u16        *dst = FSi_GetUnicodeBuffer(NULL);
    {
        int     pos = 0;
        int     dstlen;
        // まずアーカイブ名と相対パスを連結。
        dstlen = FS_ARCHIVE_FULLPATH_MAX - pos - 2;
        (void)FSi_ConvertStringSjisToUnicode(&dst[pos], &dstlen, FS_GetArchiveName(context->arc), NULL, NULL);
        pos += dstlen;
        dst[pos++] = L':';
        dst[pos++] = L'/';
        dstlen = FS_ARCHIVE_FULLPATH_MAX - pos;
        (void)FSi_ConvertStringSjisToUnicode(&dst[pos], &dstlen, relpath, NULL, NULL);
        pos += dstlen;
        // 終端に付与された'/'があれば除去。
        if ((pos > 0) && ((dst[pos - 1] == L'\\') || (dst[pos - 1] == L'/')))
        {
            --pos;
        }
        dst[pos] = L'\0';
    }
    return dst;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DostimeToFstime

  Description:  FATのDOSタイムスタンプをFSDateTime形式に変換。

  Arguments:    dst                 変換結果を格納するFSDateTime構造体
                src                 変換元となるDOSタイムスタンプ

  Returns:      None
 *---------------------------------------------------------------------------*/
static void FSi_DostimeToFstime(FSDateTime *dst, u32 src)
{
    dst->year = FATFS_DOSTIME_TO_YEAR(src);
    dst->month = FATFS_DOSTIME_TO_MON(src);
    dst->day = FATFS_DOSTIME_TO_MDAY(src);
    dst->hour = FATFS_DOSTIME_TO_HOUR(src);
    dst->minute = FATFS_DOSTIME_TO_MIN(src);
    dst->second = FATFS_DOSTIME_TO_SEC(src);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FstimeToDostime

  Description:  FSDateTimeをFATのDOSタイムスタンプ形式に変換。

  Arguments:    dst                 変換結果を格納するDOSタイムスタンプ
                src                 変換元となるFSDateTime構造体

  Returns:      None
 *---------------------------------------------------------------------------*/
static void FSi_FstimeToDostime(u32 *dst, const FSDateTime *src)
{
    *dst = FATFS_DATETIME_TO_DOSTIME(src->year, src->month, src->day,
                                     src->hour, src->minute, src->second);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_CheckFstime

  Description:  FSDateTimeをチェックする

  Arguments:    src                 FSDateTime構造体

  Returns:      TRUE / FALSE
 *---------------------------------------------------------------------------*/
static BOOL FSi_CheckFstime( const FSDateTime *fstime)
{
    if( fstime->month / 13 != 0) { return FALSE;}
//    if( fstime->day   / 32 != 0) { return FALSE;}
    if( fstime->hour  / 24 != 0) { return FALSE;}
    if( fstime->minute/ 60 != 0) { return FALSE;}
    if( fstime->second/ 61 != 0) { return FALSE;}
    return( TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeSpanExcluding

  Description:  指定された文字群を含まない文字列長を取得。

  Arguments:    src : 走査対象文字列
                pattern : 検索すべき文字群

  Returns:      指定された文字群を含まない文字列長
 *---------------------------------------------------------------------------*/
static int FSi_GetUnicodeSpanExcluding(const u16 *src, const u16 *pattern)
{
    int     pos = 0;
    BOOL    found = FALSE;
    for (; src[pos]; ++pos)
    {
        int     i;
        for (i = 0; pattern[i]; ++i)
        {
            if (src[pos] == pattern[i])
            {
                found = TRUE;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_UsingInvalidCharacterW

  Description:  パス名として不適切な文字を使用しているか判定。
                ただし "arcname:/<***>" の形式は特殊パスとして許可。

  Arguments:    path : 検査対象となるフルパス文字列

  Returns:      不適切な文字を使用していたらTRUE
 *---------------------------------------------------------------------------*/
static BOOL FSi_UsingInvalidCharacterW(const u16 *path)
{
    BOOL        retval = FALSE;
    const u16  *list = FSiPathInvalidCharactersW;
    if (list && *list)
    {
        BOOL    foundLT = FALSE;
        int     pos = 0;
        // アーカイブ名をスキップ。
        while (path[pos] && (path[pos] != L':'))
        {
            ++pos;
        }
        // アーカイブ区切りをスキップ。
        pos += (path[pos] == L':');
        pos += (path[pos] == L'/');
        // 特殊パスを考慮しつつ禁止文字の検索。
        if (path[pos] == L'<')
        {
            foundLT = TRUE;
            ++pos;
        }
        // 禁止文字を使用していないか確認。
        while (path[pos])
        {
            pos += FSi_GetUnicodeSpanExcluding(&path[pos], list);
            if (path[pos])
            {
                if (foundLT && (path[pos] == L'>') && (path[pos + 1] == L'\0'))
                {
                    foundLT = FALSE;
                    pos += 1;
                }
                else
                {
                    retval = TRUE;
                    break;
                }
            }
        }
        retval |= foundLT;
        // 禁止文字を使用していたら警告。
        if (retval)
        {
            static BOOL logOnce = FALSE;
            if (!logOnce)
            {
                OS_TWarning("specified path includes invalid character '%c'\n", (char)path[pos]);
                logOnce = TRUE;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertError

  Description:  FATFSライブラリのエラーコードをFSライブラリのエラーコードへ変換。

  Arguments:    error               FATFSライブラリのエラーコード

  Returns:      FSライブラリのエラーコード
 *---------------------------------------------------------------------------*/
FSResult FSi_ConvertError(u32 error)
{
    if (error == FATFS_RESULT_SUCCESS)
    {
        return FS_RESULT_SUCCESS;
    }
    else if (error == FATFS_RESULT_BUSY)
    {
        return FS_RESULT_BUSY;
    }
    else if (error == FATFS_RESULT_FAILURE)
    {
        return FS_RESULT_FAILURE;
    }
    else if (error == FATFS_RESULT_UNSUPPORTED)
    {
        return FS_RESULT_UNSUPPORTED;
    }
    else if (error == FATFS_RESULT_INVALIDPARAM)
    {
        return FS_RESULT_INVALID_PARAMETER;
    }
    else if (error == FATFS_RESULT_ALREADYDONE)
    {
        return FS_RESULT_ALREADY_DONE;
    }
    else if (error == FATFS_RESULT_PERMISSIONDENIDED)
    {
        return FS_RESULT_PERMISSION_DENIED;
    }
    else if (error == FATFS_RESULT_NOMORERESOURCE)
    {
        return FS_RESULT_NO_MORE_RESOURCE;
    }
    else if (error == FATFS_RESULT_MEDIAFATAL)
    {
        return FS_RESULT_MEDIA_FATAL;
    }
    else if (error == FATFS_RESULT_NOENTRY)
    {
        return FS_RESULT_NO_ENTRY;
    }
    else if (error == FATFS_RESULT_MEDIANOTHING)
    {
        return FS_RESULT_MEDIA_NOTHING;
    }
    else if (error == FATFS_RESULT_MEDIAUNKNOWN)
    {
        return FS_RESULT_MEDIA_UNKNOWN;
    }
    else if (error == FATFS_RESULT_BADFORMAT)
    {
        return FS_RESULT_BAD_FORMAT;
    }
    else if (error == FATFS_RESULT_CANCELED)
    {
        return FS_RESULT_CANCELED;
    }
    else
    {
        return FS_RESULT_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFSAsyncDone

  Description:  FATFS非同期処理完了通知

  Arguments:    buffer           リクエスト構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_FATFSAsyncDone(FATFSRequestBuffer *buffer)
{
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)buffer->userdata;
    FS_NotifyArchiveAsyncEnd(context->arc, FSi_ConvertError(buffer->header.result));
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetArchiveCaps

  Description:  FS_COMMAND_GETARCHIVECAPSコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                caps                デバイス能力フラグの格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetArchiveCaps(FSArchive *arc, u32 *caps)
{
    (void)arc;
    *caps = FS_ARCHIVE_CAPS_UNICODE;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetPathInfo

  Description:  FS_COMMAND_GETPATHINFOコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                baseid              基点ディレクトリID (ルートなら0)
                relpath             パス
                info                ファイル情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetPathInfo(FSArchive *arc, u32 baseid, const char *relpath, FSPathInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileInfoW          stat[1];
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    (void)baseid;
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_GetFileInfoW(path, stat))
    {
        info->attributes = stat->attributes;
        if ((stat->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
        {
            info->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
        }
        info->filesize = stat->length;
        FSi_DostimeToFstime(&info->atime, stat->dos_atime);
        FSi_DostimeToFstime(&info->mtime, stat->dos_mtime);
        FSi_DostimeToFstime(&info->ctime, stat->dos_ctime);
        info->id = FS_INVALID_FILE_ID;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetPathInfo

  Description:  FS_COMMAND_SETPATHINFOコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                baseid              基点ディレクトリID (ルートなら0)
                relpath             パス
                info                ファイル情報の格納元

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetPathInfo(FSArchive *arc, u32 baseid, const char *relpath, FSPathInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileInfoW          stat[1];
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    (void)baseid;
    FSi_FstimeToDostime(&stat->dos_atime, &info->atime);
    FSi_FstimeToDostime(&stat->dos_mtime, &info->mtime);
    FSi_FstimeToDostime(&stat->dos_ctime, &info->ctime);
    stat->attributes = (info->attributes & (FATFS_ATTRIBUTE_DOS_MASK | FATFS_PROPERTY_CTRL_MASK));
    info->attributes &= ~FATFS_PROPERTY_CTRL_MASK;
    if (((stat->attributes & FATFS_PROPERTY_CTRL_MASK) != 0) && !FSi_CheckFstime(&info->mtime))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_SetFileInfoW(path, stat))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CreateFile

  Description:  FATFS_COMMAND_CREATE_FILEコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CreateFile(FSArchive *arc, u32 baseid, const char *relpath, u32 permit)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    permitstring[16];
    char                   *s = permitstring;
    BOOL                    tranc = TRUE;
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((permit & FS_PERMIT_R) != 0)
    {
        *s++ = 'r';
    }
    if ((permit & FS_PERMIT_W) != 0)
    {
        *s++ = 'w';
    }
    *s++ = '\0';
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_CreateFileW(path, tranc, permitstring))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_DeleteFile

  Description:  FATFS_COMMAND_DELETE_FILEコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_DeleteFile(FSArchive *arc, u32 baseid, const char *relpath)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_DeleteFileW(path))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_RenameFile

  Description:  FATFS_COMMAND_RENAME_FILEコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_RenameFile(FSArchive *arc, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *src = FSi_AllocUnicodeFullPath(context, relpath_src);
    u16                    *dst = FSi_AllocUnicodeFullPath(context, relpath_dst);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(src))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(dst))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_RenameFileW(src, dst))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(src);
    FSi_ReleaseUnicodeBuffer(dst);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid_src;
    (void)baseid_dst;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CreateDirectory

  Description:  FATFS_COMMAND_CREATE_DIRECTORYコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CreateDirectory(FSArchive *arc, u32 baseid, const char *relpath, u32 permit)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    permitstring[16];
    char                   *s = permitstring;
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((permit & FS_PERMIT_R) != 0)
    {
        *s++ = 'r';
    }
    if ((permit & FS_PERMIT_W) != 0)
    {
        *s++ = 'w';
    }
    *s++ = '\0';
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_CreateDirectoryW(path, permitstring))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_DeleteDirectory

  Description:  FATFS_COMMAND_DELETE_DIRECTORYコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_DeleteDirectory(FSArchive *arc, u32 baseid, const char *relpath)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, relpath);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_DeleteDirectoryW(path))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_RenameDirectory

  Description:  FATFS_COMMAND_RENAME_DIRECTORYコマンド

  Arguments:    

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_RenameDirectory(FSArchive *arc, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *src = FSi_AllocUnicodeFullPath(context, relpath_src);
    u16                    *dst = FSi_AllocUnicodeFullPath(context, relpath_dst);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(src))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FSi_UsingInvalidCharacterW(dst))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_RenameDirectoryW(src, dst))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(src);
    FSi_ReleaseUnicodeBuffer(dst);
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid_src;
    (void)baseid_dst;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetArchiveResource

  Description:  FS_COMMAND_GETARCHIVERESOURCEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                resource            リソース情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetArchiveResource(FSArchive *arc, FSArchiveResource *resource)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    u16                    *path = FSi_AllocUnicodeFullPath(context, "/");
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FSi_UsingInvalidCharacterW(path))
    {
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if (FATFS_GetDriveResourceW(path, context->resource))
    {
        resource->totalSize = context->resource->totalSize;
        resource->availableSize = context->resource->availableSize;
        resource->maxFileHandles = context->resource->maxFileHandles;
        resource->currentFileHandles = context->resource->currentFileHandles;
        resource->maxDirectoryHandles = context->resource->maxDirectoryHandles;
        resource->currentDirectoryHandles = context->resource->currentDirectoryHandles;
        // for FAT archives.
        resource->bytesPerSector = context->resource->bytesPerSector;
        resource->sectorsPerCluster = context->resource->sectorsPerCluster;
        resource->totalClusters = context->resource->totalClusters;
        resource->availableClusters = context->resource->availableClusters;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FSi_ReleaseUnicodeBuffer(path);
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_OpenFile

  Description:  FS_COMMAND_OPENFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                baseid              基点ディレクトリ (ルートなら0)
                path                ファイルパス
                mode                アクセスモード

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_OpenFile(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSFileHandle handle = FATFS_INVALID_HANDLE;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    char                    modestring[16];
    char                   *s = modestring;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((mode & FS_FILEMODE_R) != 0)
    {
        *s++ = 'r';
        if ((mode & FS_FILEMODE_W) != 0)
        {
            *s++ = '+';
        }
    }
    else if ((mode & FS_FILEMODE_W) != 0)
    {
        *s++ = 'w';
    }
    if ((mode & FS_FILEMODE_L) != 0)
    {
        *s++ = 'l';
    }
    *s++ = '\0';
    {
        const u16  *unipath = NULL;
        u16        *fpath = NULL;
        // Unicode版の構造体を渡されていればUnicodeのまま格納。
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            unipath = (const u16 *)path;
        }
        // そうでなければShift_JIS版として格納。
        // (ただしこれは暫定的な実装であり、将来的には
        //  CAPS_UNICODEを宣言したアーカイブにUnicode版以外は渡されない)
        else
        {
            fpath = FSi_AllocUnicodeFullPath(context, path);
            unipath = fpath;
        }
        if (FSi_UsingInvalidCharacterW(unipath))
        {
            result = FS_RESULT_INVALID_PARAMETER;
        }
        else
        {
            handle = FATFS_OpenFileW(unipath, modestring);
            if (handle != FATFS_INVALID_HANDLE)
            {
                FS_SetFileHandle(file, arc, (void*)handle);
                result = FS_RESULT_SUCCESS;
            }
            else
            {
                result = FSi_ConvertError(tls->result);
            }
        }
        FSi_ReleaseUnicodeBuffer(fpath);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CloseFile

  Description:  FS_COMMAND_CLOSEFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CloseFile(FSArchive *arc, FSFile *file)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_CloseFile(handle))
    {
        FS_DetachHandle(file);
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_ReadFile

  Description:  FS_COMMAND_READFILEコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                buffer           転送先メモリ
                length           転送サイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_ReadFile(FSArchive *arc, FSFile *file, void *buffer, u32 *length)
{
    FSResult                result = FS_RESULT_SUCCESS;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileHandle         handle = (FATFSFileHandle)FS_GetFileUserData(file);
    u32                     rest = *length;
    BOOL                    async = ((file->stat & FS_FILE_STATUS_BLOCKING) == 0);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    while (rest > 0)
    {
        void   *dst = buffer;
        u32     len = rest;
        int     read;
        // ARM7が直接アクセスできないメモリを指定されたら一時バッファで代替。
        if (!FSi_IsValidAddressForARM7(buffer, rest))
        {
            dst = context->tmpbuf;
            len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
        }
        // 32バイト整合していないメモリを指定されたら一時バッファで代替。
        else if ((((u32)buffer | len) & 31) != 0)
        {
            // 次の転送で終わるのなら特殊な調整は無し。
            if (len <= FS_TMPBUF_LENGTH)
            {
                dst = context->tmpbuf;
                len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
            }
            // バッファ先端が不整合ならば次の転送で整合するよう調整。
            else if (((u32)buffer & 31) != 0)
            {
                dst = context->tmpbuf;
                len = MATH_ROUNDUP((u32)buffer, 32) - (u32)buffer;
            }
            // 転送サイズだけの不整合ならば終端を切り詰めて直接転送。
            else
            {
                len = MATH_ROUNDDOWN(len, 32);
            }
        }
        // 一時バッファを使用しなくともメインメモリならば
        // ダーティなキャッシュをライトバックしておく。
        if ((dst == buffer) &&
            (((u32)dst >= HW_TWL_MAIN_MEM) && ((u32)dst + len <= HW_TWL_MAIN_MEM_END)))
        {
            DC_FlushRange(dst, len);
        }
        // 非同期処理が求められていて実際に非同期処理可能であれば
        // ここでリクエストバッファを設定する。
        async &= ((dst == buffer) && (len == rest));
        if (async)
        {
            FATFSi_SetRequestBuffer(&FSiFATFSAsyncRequest[context - FSiFATFSDrive], FSi_FATFSAsyncDone, context);
        }
        // 実際の呼び出し
        read = FATFS_ReadFile(handle, dst, (int)len);
        if (async)
        {
            rest -= len;
            result = FS_RESULT_PROC_ASYNC;
            break;
        }
        // 一時バッファで代替したならここでコピー
        if ((dst != buffer) && (read > 0))
        {
            DC_InvalidateRange(dst, (u32)read);
            MI_CpuCopy8(dst, buffer, (u32)read);
        }
        buffer = (u8 *)buffer + read;
        rest -= read;
        if (read != len)
        {
            result = FSi_ConvertError(tls->result);
            break;
        }
    }
    *length -= rest;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_WriteFile

  Description:  FS_COMMAND_WRITEFILEコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                buffer           転送元メモリ
                length           転送サイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_WriteFile(FSArchive *arc, FSFile *file, const void *buffer, u32 *length)
{
    FSResult                result = FS_RESULT_SUCCESS;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSFileHandle         handle = (FATFSFileHandle)FS_GetFileUserData(file);
    u32                     rest = *length;
    BOOL                    async = ((file->stat & FS_FILE_STATUS_BLOCKING) == 0);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    while (rest > 0)
    {
        // ARM7が直接アクセスできないメモリを指定されたら一時バッファで代替。
        const void *dst = buffer;
        u32         len = rest;
        int         written;
        if (!FSi_IsValidAddressForARM7(buffer, rest) || ((((u32)buffer | len) & 31) != 0))
        {
            dst = context->tmpbuf;
            len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
        }
        // 32バイト整合していないメモリを指定されたら一時バッファで代替。
        else if ((((u32)buffer | len) & 31) != 0)
        {
            // 次の転送で終わるのなら特殊な調整は無し。
            if (len <= FS_TMPBUF_LENGTH)
            {
                dst = context->tmpbuf;
                len = (u32)MATH_MIN(len, FS_TMPBUF_LENGTH);
            }
            // バッファ先端が不整合ならば次の転送で整合するよう調整。
            else if (((u32)buffer & 31) != 0)
            {
                dst = context->tmpbuf;
                len = MATH_ROUNDUP((u32)buffer, 32) - (u32)buffer;
            }
            // 転送サイズだけの不整合ならば終端を切り詰めて直接転送。
            else
            {
                len = MATH_ROUNDDOWN(len, 32);
            }
        }
        // 一時バッファを使用するなら書き込むべきデータをコピー。
        if (dst != buffer)
        {
            MI_CpuCopy8(buffer, context->tmpbuf, len);
        }
        // バッファがメインメモリならARM7から見えるようライトバックが必要。
        if (((u32)dst >= HW_TWL_MAIN_MEM) && ((u32)dst + len <= HW_TWL_MAIN_MEM_END))
        {
            DC_StoreRange(dst, len);
        }
        // 非同期処理が求められていて実際に非同期処理可能であれば
        // ここでリクエストバッファを設定する。
        async &= ((dst == buffer) && (len == rest));
        if (async)
        {
            FATFSi_SetRequestBuffer(&FSiFATFSAsyncRequest[context - FSiFATFSDrive], FSi_FATFSAsyncDone, context);
        }
        // 実際の呼び出し
        written = FATFS_WriteFile(handle, dst, (int)len);
        if (async)
        {
            rest -= len;
            result = FS_RESULT_PROC_ASYNC;
            break;
        }
        buffer = (u8 *)buffer + written;
        rest -= written;
        if (written != len)
        {
            result = FSi_ConvertError(tls->result);
            break;
        }
    }
    *length -= rest;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetSeekCache

  Description:  FS_COMMAND_SETSEEKCACHEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                buf                 キャッシュバッファ
                buf_size            キャッシュバッファサイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetSeekCache(FSArchive *arc, FSFile *file, void* buf, u32 buf_size)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    
    if ((buf != NULL) &&
        (((u32)buf < HW_TWL_MAIN_MEM) || (((u32)buf + buf_size) > HW_TWL_MAIN_MEM_END)))
    {
        // メインメモリ上のバッファではない場合はパラメータエラー
        result = FS_RESULT_INVALID_PARAMETER;
    }
    else if ((buf != NULL) && (buf_size < 32))
    {
        // ARM7とのメモリ共有にサイズが足りない場合は失敗
        result = FS_RESULT_FAILURE;
    }
    else if ((((u32)buf & 31) != 0) && ((buf_size - (32 - ((u32)buf & 31))) < 32))
    {
        // 32バイト整合後にサイズが足りない場合も失敗
        result = FS_RESULT_FAILURE;
    }
    else
    {
        void   *cache;
        u32     cache_size;
        
        if (buf != NULL) {
            // 本関数で使用する前にキャッシュにのっていた可能性があるため、フラッシュ
            DC_FlushRange(buf, buf_size);
        }
        
        // ARM7と共有できるように32バイト整合した領域を抜き出す
        if (((u32)buf & 31) != 0) {
            cache = (void *)((u32)buf + (32 - ((u32)buf & 31)));    // 32バイト境界
            cache_size = buf_size - (32 - ((u32)buf & 31));
        } else {
            cache = buf;
            cache_size = buf_size;
        }
        cache_size = cache_size & ~31;                              // 32バイト切り下げ

        if( FATFS_SetSeekCache(handle, cache, cache_size) != FALSE)
        {
            result = FS_RESULT_SUCCESS;
        }
        else
        {
            result = FSi_ConvertError(tls->result);
        }
    }
    (void)arc;
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SeekFile

  Description:  FS_COMMAND_SEEKFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                offset              移動量および移動後の位置
                from                シーク起点

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SeekFile(FSArchive *arc, FSFile *file, int *offset, FSSeekFileMode from)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSSeekMode   mode = FATFS_SEEK_CUR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (from == FS_SEEK_SET)
    {
        mode = FATFS_SEEK_SET;
    }
    else if (from == FS_SEEK_CUR)
    {
        mode = FATFS_SEEK_CUR;
    }
    else if (from == FS_SEEK_END)
    {
        mode = FATFS_SEEK_END;
    }
    *offset = FATFS_SeekFile(handle, *offset, mode);
    if (*offset >= 0)
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetFileLength

  Description:  FS_COMMAND_GETFILELENGTHコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                length              取得したサイズの格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetFileLength(FSArchive *arc, FSFile *file, u32 *length)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    int             n = FATFS_GetFileLength(handle);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (n >= 0)
    {
        *length = (u32)n;
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_SetFileLength

  Description:  FS_COMMAND_SETFILELENGTHコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                length              設定するファイルサイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_SetFileLength(FSArchive *arc, FSFile *file, u32 length)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_SetFileLength(handle, (int)length))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_GetFilePosition

  Description:  FS_COMMAND_GETFILEPOSITIONコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                position            取得した位置の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_GetFilePosition(FSArchive *arc, FSFile *file, u32 *position)
{
    *position = 0;
    return FSi_FATFS_SeekFile(arc, file, (int*)position, FS_SEEK_CUR);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_FlushFile

  Description:  FS_COMMAND_FLUSHFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_FlushFile(FSArchive *arc, FSFile *file)
{
    FSResult        result = FS_RESULT_ERROR;
    FATFSFileHandle handle = (FATFSFileHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_FlushFile(handle))
    {
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_OpenDirectory

  Description:  FS_COMMAND_OPENDIRECTORYコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                baseid              基点ディレクトリID (ルートなら0)
                path                パス
                mode                アクセスモード

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_OpenDirectory(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult                result = FS_RESULT_ERROR;
    FSFATFSArchiveContext  *context = (FSFATFSArchiveContext *)FS_GetArchiveUserData(arc);
    FATFSDirectoryHandle    handle = FATFS_INVALID_HANDLE;
    char                    modestring[16];
    char                   *s = modestring;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if ((mode & FS_FILEMODE_R) != 0)
    {
        *s++ = 'r';
    }
    if ((mode & FS_FILEMODE_W) != 0)
    {
        *s++ = 'w';
    }
    if ((mode & FS_DIRMODE_SHORTNAME_ONLY) != 0)
    {
        *s++ = 's';
    }
    *s++ = '\0';
    {
        u16    *unipath = NULL;
        u16    *fpath = NULL;
        // Unicode版の構造体を渡されていればUnicodeのまま格納。
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            unipath = (u16 *)path;
        }
        // そうでなければShift_JIS版として格納。
        // (ただしこれは暫定的な実装であり、将来的には
        //  CAPS_UNICODEを宣言したアーカイブにUnicode版以外は渡されない)
        else
        {
            fpath = FSi_AllocUnicodeFullPath(context, path);
            unipath = fpath;
        }
        // 終端の特殊なワイルドカード指定を無効化。
        // パスはFSライブラリが常に正規化して渡しているため
        // バッファの内容は変更可能であることが保証されている。
        if (*unipath)
        {
            int     pos;
            for (pos = 0; unipath[pos]; ++pos)
            {
            }
            for (--pos; (pos > 0) && (unipath[pos] == L'*'); --pos)
            {
            }
            if (unipath[pos] != L'/')
            {
                unipath[++pos] = L'/';
            }
            unipath[++pos] = L'*';
            unipath[++pos] = L'\0';
        }
        handle = FATFS_OpenDirectoryW(unipath, modestring);
        if (handle != FATFS_INVALID_HANDLE)
        {
            FS_SetDirectoryHandle(file, arc, (void*)handle);
            result = FS_RESULT_SUCCESS;
        }
        else
        {
            result = FSi_ConvertError(tls->result);
        }
        FSi_ReleaseUnicodeBuffer(fpath);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)baseid;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_CloseDirectory

  Description:  FS_COMMAND_CLOSEDIRECTORYコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_CloseDirectory(FSArchive *arc, FSFile *file)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSDirectoryHandle    handle = (FATFSDirectoryHandle)FS_GetFileUserData(file);
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_CloseDirectory(handle))
    {
        FS_DetachHandle(file);
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FATFS_ReadDirectory

  Description:  FS_COMMAND_READDIRコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                info             情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_FATFS_ReadDirectory(FSArchive *arc, FSFile *file, FSDirectoryEntryInfo *info)
{
    FSResult                result = FS_RESULT_ERROR;
    FATFSDirectoryHandle    handle = (FATFSDirectoryHandle)FS_GetFileUserData(file);
    FATFSFileInfoW          tmp[1];
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (FATFS_ReadDirectoryW(handle, tmp))
    {
        // Unicode版の構造体を渡されていればUnicodeのまま格納。
        if ((file->stat & FS_FILE_STATUS_UNICODE_MODE) != 0)
        {
            FSDirectoryEntryInfoW  *infow = (FSDirectoryEntryInfoW *)info;
            infow->shortname_length = (u32)STD_GetStringLength(tmp->shortname);
            (void)STD_CopyLString(infow->shortname, tmp->shortname, sizeof(infow->shortname));
            {
                int     n;
                for (n = 0; (n < FS_ENTRY_LONGNAME_MAX - 1) && tmp->longname[n]; ++n)
                {
                    infow->longname[n] = tmp->longname[n];
                }
                infow->longname[n] = L'\0';
                infow->longname_length = (u32)n;
            }
            infow->attributes = tmp->attributes;
            if ((tmp->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
            {
                infow->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
            }
            infow->filesize = tmp->length;
            FSi_DostimeToFstime(&infow->atime, tmp->dos_atime);
            FSi_DostimeToFstime(&infow->mtime, tmp->dos_mtime);
            FSi_DostimeToFstime(&infow->ctime, tmp->dos_ctime);
        }
        // そうでなければShift_JIS版として格納。
        // (ただしこれは暫定的な実装であり、将来的には
        //  CAPS_UNICODEを宣言したアーカイブにUnicode版以外は渡されない)
        else
        {
            FSDirectoryEntryInfo   *infoa = (FSDirectoryEntryInfo *)info;
            infoa->shortname_length = (u32)STD_GetStringLength(tmp->shortname);
            (void)STD_CopyLString(infoa->shortname, tmp->shortname, sizeof(infoa->shortname));
            {
                int     dstlen = sizeof(infoa->longname) - 1;
                int     srclen = 0;
                (void)FSi_ConvertStringUnicodeToSjis(infoa->longname, &dstlen, tmp->longname, NULL, NULL);
                infoa->longname[dstlen] = L'\0';
                infoa->longname_length = (u32)dstlen;
            }
            infoa->attributes = tmp->attributes;
            if ((tmp->attributes & FATFS_ATTRIBUTE_DOS_DIRECTORY) != 0)
            {
                infoa->attributes |= FS_ATTRIBUTE_IS_DIRECTORY;
            }
            infoa->filesize = tmp->length;
            FSi_DostimeToFstime(&infoa->atime, tmp->dos_atime);
            FSi_DostimeToFstime(&infoa->mtime, tmp->dos_mtime);
            FSi_DostimeToFstime(&infoa->ctime, tmp->dos_ctime);
        }
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = FSi_ConvertError(tls->result);
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    (void)arc;
    return result;
}

static PMSleepCallbackInfo  FSiPreSleepInfo;
static PMExitCallbackInfo   FSiPostExitInfo;
static void FSi_SleepProcedure(void *arg)
{
    (void)arg;
    (void)FATFS_FlushAll();
}

static void FSi_ShutdownProcedure(void *arg)
{
    (void)arg;
    (void)FATFS_UnmountAll();
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountFATFSArchive

  Description:  FATFSアーカイブをマウント

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_MountFATFSArchive(FSFATFSArchiveContext *context, const char *arcname, u8 *tmpbuf)
{
    static const FSArchiveInterface FSiArchiveFATFSInterface =
    {
        // 旧仕様と互換性のあるコマンド
        FSi_FATFS_ReadFile,
        FSi_FATFS_WriteFile,
        NULL,               // SeekDirectory
        FSi_FATFS_ReadDirectory,
        NULL,               // FindPath
        NULL,               // GetPath
        NULL,               // OpenFileFast
        NULL,               // OpenFileDirect
        FSi_FATFS_CloseFile,
        NULL,               // Activate
        NULL,               // Idle
        NULL,               // Suspend
        NULL,               // Resume
        // 旧仕様と互換性があるがコマンドではなかったもの
        FSi_FATFS_OpenFile,
        FSi_FATFS_SeekFile,
        FSi_FATFS_GetFileLength,
        FSi_FATFS_GetFilePosition,
        // 新仕様で拡張されたコマンド (NULLならUNSUPPORTED)
        NULL,               // Mount
        NULL,               // Unmount
        FSi_FATFS_GetArchiveCaps,
        FSi_FATFS_CreateFile,
        FSi_FATFS_DeleteFile,
        FSi_FATFS_RenameFile,
        FSi_FATFS_GetPathInfo,
        FSi_FATFS_SetPathInfo,
        FSi_FATFS_CreateDirectory,
        FSi_FATFS_DeleteDirectory,
        FSi_FATFS_RenameDirectory,
        FSi_FATFS_GetArchiveResource,
        NULL,               // 29UL
        FSi_FATFS_FlushFile,
        FSi_FATFS_SetFileLength,
        FSi_FATFS_OpenDirectory,
        FSi_FATFS_CloseDirectory,
        FSi_FATFS_SetSeekCache,
    };
    u32     arcnamelen = (u32)STD_GetStringLength(arcname);
    context->tmpbuf = tmpbuf;
    FS_InitArchive(context->arc);
    if (FS_RegisterArchiveName(context->arc, arcname, arcnamelen))
    {
        (void)FS_MountArchive(context->arc, context, &FSiArchiveFATFSInterface, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountDefaultArchives

  Description:  IPLに与えられた起動引数を参照してデフォルトアーカイブをマウント

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_MountDefaultArchives(void)
{
    // FATFSが各種の物理メディアをマウントするまで待つ。
    FATFS_Init();
    FSi_SetupFATBuffers();
    // スリープ前とシャットダウン前にフラッシュするためのコールバックを設定。
    FSiPreSleepInfo.callback = FSi_SleepProcedure;
    FSiPostExitInfo.callback = FSi_ShutdownProcedure;
    PMi_InsertPreSleepCallbackEx(&FSiPreSleepInfo, PM_CALLBACK_PRIORITY_FS);
    PMi_InsertPostExitCallbackEx(&FSiPostExitInfo, PM_CALLBACK_PRIORITY_FS);
    // 実際にアクセス権限を持つ論理ドライブのみFSへマウントする。
    if (FSiFATFSDrive && FSiTemporaryBuffer)
    {
        int                 index = 0;
        static const int    max = FS_MOUNTDRIVE_MAX;
        const char         *arcname = FATFSi_GetArcnameList();
        while ((index < max) && *arcname)
        {
            FSi_MountFATFSArchive(&FSiFATFSDrive[index], arcname, &FSiTemporaryBuffer[FS_TMPBUF_LENGTH * index]);
            arcname += STD_GetStringLength(arcname) + 1;
            ++index;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSpecialArchive

  Description:  特殊なアーカイブを直接マウント

  Arguments:    param   : マウント対象を指定するパラメータ
                arcname : マウントするアーカイブ名
                          "otherPub"または"otherPrv"が指定可能で
                          NULLを指定すると以前のアーカイブをアンマウント
                pWork   : マウントに使用するワーク領域。
                          マウントされている間保持される必要があります。

  Returns:      処理が成功すればFS_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
FSResult FSi_MountSpecialArchive(u64 param, const char *arcname, FSFATFSArchiveWork* pWork)
{
    FSResult    result = FS_RESULT_ERROR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (!FATFS_MountSpecial(param, arcname, &pWork->slot))
    {
        result = FSi_ConvertError(tls->result);
    }
    else
    {
        if (arcname && *arcname)
        {
            FSi_MountFATFSArchive(&pWork->context, arcname, pWork->tmpbuf);
        }
        else
        {
            (void)FS_UnmountArchive(pWork->context.arc);
            (void)FS_ReleaseArchiveName(pWork->context.arc);
        }
        result = FS_RESULT_SUCCESS;
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FormatSpecialArchive

  Description:  以下の条件を満たす特殊なアーカイブの内容をフォーマット。
                  - 現在すでにマウントされている。
                  - 自身が所有権を持つ。 (dataPub, dataPrv, share*)

  Arguments:    path : アーカイブ名を含んだパス名

  Returns:      処理が成功すればFS_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
FSResult FSi_FormatSpecialArchive(const char *path)
{
    FSResult    result = FS_RESULT_ERROR;
    FATFSResultBuffer       tls[1];
    FATFS_RegisterResultBuffer(tls, TRUE);
    if (!FATFS_FormatSpecial(path))
    {
        result = FSi_ConvertError(tls->result);
    }
    else
    {
        result = FS_RESULT_SUCCESS;
    }
    FATFS_RegisterResultBuffer(tls, FALSE);
    return result;
}


#include <twl/ltdmain_end.h>


#endif /* FS_IMPLEMENT */

#endif /* SDK_TWL */

/*---------------------------------------------------------------------------*
  Name:         FS_ForceToEnableLatencyEmulation

  Description:  劣化したNANDデバイスへのアクセス時に生じる
                ランダムなウェイト時間をドライバで擬似的に再現する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ForceToEnableLatencyEmulation(void)
{
#if defined(SDK_TWL)
    if (OS_IsRunOnTwl())
    {
        (void)FATFS_SetLatencyEmulation(TRUE);
    }
#endif
}

