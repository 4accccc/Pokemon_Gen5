/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_file.c

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


#include <nitro/types.h>
#include <nitro/misc.h>
#include <nitro/mi.h>
#include <nitro/os.h>
#include <nitro/pxi.h>
#include <nitro/std/string.h>
#include <nitro/std/unicode.h>
#include <nitro/math/math.h>

#include <nitro/fs.h>

#include "../include/rom.h"
#include "../include/util.h"
#include "../include/command.h"


#define FS_DEBUG_TRACE(...) (void)0
//#define FS_DEBUG_TRACE  OS_TPrintf


/*---------------------------------------------------------------------------*/
/* functions */

#if defined(FS_IMPLEMENT)

/*---------------------------------------------------------------------------*
  Name:         FSi_IsValidTransferRegion

  Description:  Read/Write操作に使用するバッファが危険な範囲でないか簡易判定。

  Arguments:    buffer : 転送対象となるバッファ。
                length : 転送サイズ。

  Returns:      posを1文字戻した参照位置か-1。
 *---------------------------------------------------------------------------*/
static BOOL FSi_IsValidTransferRegion(const void *buffer, s32 length)
{
    BOOL    retval = FALSE;
    if (buffer == NULL)
    {
        OS_TWarning("specified transfer buffer is NULL.\n");
    }
    else if (((u32)buffer >= HW_IOREG) && ((u32)buffer < HW_IOREG_END))
    {
        OS_TWarning("specified transfer buffer is in I/O register %08X. (seems to be dangerous)\n", buffer);
    }
    else if (length < 0)
    {
        OS_TWarning("specified transfer size is minus. (%d)\n", length);
    }
    else
    {
#if !defined(SDK_TWL)
        s32     mainmem_size = HW_MAIN_MEM_EX_SIZE;
#else
        s32     mainmem_size = OS_IsRunOnTwl() ? HW_TWL_MAIN_MEM_EX_SIZE : HW_MAIN_MEM_EX_SIZE;
#endif
        if (length > mainmem_size)
        {
            OS_TWarning("specified transfer size is over mainmemory-size. (%d)\n", length);
        }
        else
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPosition

  Description:  Shift_JIS文字列の参照位置を1文字分戻す。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      posを1文字戻した参照位置か-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPosition(const char *str, int pos)
{
    // まず1バイト分だけは確実に戻る。
    int     prev = --pos;
    // Shift_JISの文字区切りとなるのはシングルバイトか後続バイトであり
    // 先行バイトと後続バイトがマッピングを一部共有しているため、
    // そこが先行バイトに見えるうちは文字種別が確定しないのでさらに戻る。
    for (; (prev > 0) && STD_IsSjisLeadByte(str[prev - 1]); --prev)
    {
    }
    // "＝＝＝b"のようにあいまいな後続バイトを持つ全角文字が並んでいたら
    // 2の倍数だけ余分に戻っているためこれを無視する。(2の剰余を取る)
    return pos - ((pos - prev) & 1);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPositionToSlash

  Description:  Shift_JIS文字列の参照位置を
                ディレクトリ区切り文字か終端まで進める。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos以降に現れる最初のディレクトリ区切りか終端の位置。
 *---------------------------------------------------------------------------*/
int FSi_IncrementSjisPositionToSlash(const char *str, int pos)
{
    while (str[pos] && !FSi_IsSlash((u8)str[pos]))
    {
        pos = FSi_IncrementSjisPosition(str, pos);
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPositionToSlash

  Description:  Shift_JIS文字列の参照位置を
                ディレクトリ区切り文字か先頭まで戻す。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos未満に現れる最初のディレクトリ区切りか-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPositionToSlash(const char *str, int pos)
{
    for (;;)
    {
        pos = FSi_DecrementSjisPosition(str, pos);
        if ((pos < 0) || FSi_IsSlash((u8)str[pos]))
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_TrimSjisTrailingSlash

  Description:  Shift_JIS文字列の終端がディレクトリ区切り文字であれば除去。

  Arguments:    str              Shift_JIS文字列。

  Returns:      文字列長。
 *---------------------------------------------------------------------------*/
int FSi_TrimSjisTrailingSlash(char *str)
{
    int     length = STD_GetStringLength(str);
    int     lastpos = FSi_DecrementSjisPosition(str, length);
    if ((lastpos >= 0) && FSi_IsSlash((u8)str[lastpos]))
    {
        length = lastpos;
        str[length] = '\0';
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePosition

  Description:  Unicode文字列の参照位置を1文字分戻す。

  Arguments:    str              Unicode文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      posを1文字戻した参照位置か-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePosition(const u16 *str, int pos)
{
    // まず1文字分だけは確実に戻る。
    int     prev = --pos;
    // 有効なサロゲートペアだった場合はもう1文字戻る。
    if ((pos > 0) &&
        ((str[pos - 1] >= 0xD800) && (str[pos - 1] <= 0xDC00)) &&
        ((str[pos - 0] >= 0xDC00) && (str[pos - 0] <= 0xE000)))
    {
        --pos;
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePositionToSlash

  Description:  Unicode文字列の参照位置を
                ディレクトリ区切り文字か先頭まで戻す。

  Arguments:    str              Unicode文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos未満に現れる最初のディレクトリ区切りか-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos)
{
    for (;;)
    {
        pos = FSi_DecrementUnicodePosition(str, pos);
        if ((pos < 0) || FSi_IsUnicodeSlash(str[pos]))
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FS_InitFile

  Description:  FSFile構造体を初期化

  Arguments:    file        FSFile構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_InitFile(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    {
        file->arc = NULL;
        file->userdata = NULL;
        file->next = NULL;
        OS_InitThreadQueue(file->queue);
        file->stat = 0;
        file->stat |= (FS_COMMAND_INVALID << FS_FILE_STATUS_CMD_SHIFT);
        file->argument = NULL;
        file->error = FS_RESULT_SUCCESS;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_CancelFile

  Description:  非同期コマンドのキャンセルを要求する

  Arguments:    file        ファイルハンドル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_CancelFile(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (FS_IsBusy(file))
        {
            file->stat |= FS_FILE_STATUS_CANCEL;
            file->arc->flag |= FS_ARCHIVE_FLAG_CANCELING;
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFile

  Description:  ファイルを生成する

  Arguments:    path        パス名
                mode        アクセスモード

  Returns:      ファイルが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFile(const char *path, u32 permit)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                  file[1];
            FSArgumentForCreateFile arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->permit = permit;
            retval = FSi_SendCommand(file, FS_COMMAND_CREATEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFile

  Description:  ファイルを削除する

  Arguments:    path        パス名

  Returns:      ファイルが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFile(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                  file[1];
            FSArgumentForDeleteFile arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            retval = FSi_SendCommand(file, FS_COMMAND_DELETEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFile

  Description:  ファイル名を変更する

  Arguments:    src         変換元のファイル名
                dst         変換先のファイル名

  Returns:      ファイル名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFile(const char *src, const char *dst)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    SDK_NULL_ASSERT(src);
    SDK_NULL_ASSERT(dst);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
        char        relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid_src = 0;
        u32         baseid_dst = 0;
        FSArchive  *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
        FSArchive  *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
        if (arc_src != arc_dst)
        {
            OS_TWarning("cannot rename between defferent archives.\n");
        }
        else
        {
            FSFile                  file[1];
            FSArgumentForRenameFile arg[1];
            FS_InitFile(file);
            file->arc = arc_src;
            file->argument = arg;
            arg->baseid_src = baseid_src;
            arg->relpath_src = relpath_src;
            arg->baseid_dst = baseid_dst;
            arg->relpath_dst = relpath_dst;
            retval = FSi_SendCommand(file, FS_COMMAND_RENAMEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathInfo

  Description:  ファイルまたはディレクトリの情報を取得する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
BOOL FS_GetPathInfo(const char *path, FSPathInfo *info)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                      file[1];
            FSArgumentForGetPathInfo    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->info = info;
            retval = FSi_SendCommand(file, FS_COMMAND_GETPATHINFO, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetPathInfo

  Description:  ファイル情報を設定する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
BOOL FS_SetPathInfo(const char *path, const FSPathInfo *info)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                      file[1];
            FSArgumentForSetPathInfo    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->info = (FSPathInfo*)info; //info->attributesのFATFS_PROPERTY_CTRL_MASKを落とすため
            retval = FSi_SendCommand(file, FS_COMMAND_SETPATHINFO, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectory

  Description:  ディレクトリを生成する

  Arguments:    path        パス名
                mode        アクセスモード

  Returns:      ディレクトリが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectory(const char *path, u32 permit)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForCreateDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->permit = permit;
            retval = FSi_SendCommand(file, FS_COMMAND_CREATEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectory

  Description:  ディレクトリを削除する

  Arguments:    path        パス名

  Returns:      ディレクトリが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectory(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForDeleteDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            retval = FSi_SendCommand(file, FS_COMMAND_DELETEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectory

  Description:  ディレクトリ名を変更する

  Arguments:    src         変換元のディレクトリ名
                dst         変換先のディレクトリ名

  Returns:      ディレクトリ名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectory(const char *src, const char *dst)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    SDK_NULL_ASSERT(src);
    SDK_NULL_ASSERT(dst);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
        char        relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid_src = 0;
        u32         baseid_dst = 0;
        FSArchive  *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
        FSArchive  *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
        if (arc_src != arc_dst)
        {
            OS_TWarning("cannot rename between defferent archives.\n");
        }
        else
        {
            FSFile                          file[1];
            FSArgumentForRenameDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc_src;
            file->argument = arg;
            arg->baseid_src = baseid_src;
            arg->relpath_src = relpath_src;
            arg->baseid_dst = baseid_dst;
            arg->relpath_dst = relpath_dst;
            retval = FSi_SendCommand(file, FS_COMMAND_RENAMEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFullPath

  Description:  指定されたパスをフルパスへ変換する。

  Arguments:    dst         得られたフルパスを格納するバッファ。
                            FS_ARCHIVE_FULLPATH_MAX+1バイト以上でなければならない。
                path        ファイルまたはディレクトリのパス名。

  Returns:      フルパスが正常に取得できればTRUE
 *---------------------------------------------------------------------------*/
static BOOL FSi_GetFullPath(char *dst, const char *path)
{
    FSArchive  *arc = FS_NormalizePath(path, NULL, dst);
    if (arc)
    {
        const char *arcname = FS_GetArchiveName(arc);
        int     m = STD_GetStringLength(arcname);
        int     n = STD_GetStringLength(dst);
        (void)STD_MoveMemory(&dst[m + 2], &dst[0], (u32)n + 1);
        (void)STD_MoveMemory(&dst[0], arcname, (u32)m);
        dst[m + 0] = ':';
        dst[m + 1] = '/';
    }
    return (arc != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ComplementDirectory

  Description:  指定されたパスに至る全ての親ディレクトリの存在を確認し、
                存在しない階層は自動的に生成して補完する。

  Arguments:    path        ファイルまたはディレクトリのパス名。
                            これより1階層上までのディレクトリを補完する。
                autogen     自動生成された最上位ディレクトリを記録するバッファ。
                            FS_ARCHIVE_FULLPATH_MAX+1バイト以上でなければならない。
                            結果の成否にかかわらずautogenには結果が返され、
                            空文字列であればすでに全て存在していたことを示す。

  Returns:      ディレクトリが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
static BOOL FSi_ComplementDirectory(const char *path, char *autogen)
{
    BOOL    retval = FALSE;
    int     root = 0;
    // いったんフルパス名に正規化。(スタック節約のためautogenを流用)
    char   *tmppath = autogen;
    if (FSi_GetFullPath(tmppath, path))
    {
        int     length = STD_GetStringLength(tmppath);
        if (length > 0)
        {
            int     pos = 0;
            FS_DEBUG_TRACE("  trying to complete \"%s\"\n", tmppath);
            // 終端の"/"は除去。
            length = FSi_TrimSjisTrailingSlash(tmppath);
            // 最下層のエントリは無視。
            length = FSi_DecrementSjisPositionToSlash(tmppath, length);
            // 存在する最深の階層を探索。
            for (pos = length; pos >= 0;)
            {
                FSPathInfo  info[1];
                BOOL        exists;
                tmppath[pos] = '\0';
                exists = FS_GetPathInfo(tmppath, info);
                FS_DEBUG_TRACE("    - \"%s\" is%s existent (result:%d)\n", tmppath, exists ? "" : " not",
                               FS_GetArchiveResultCode(tmppath));
                tmppath[pos] = '/';
                // エントリが存在しなければさらに上位の階層へ。
                if (!exists)
                {
                    pos = FSi_DecrementSjisPositionToSlash(tmppath, pos);
                }
                // 何か存在していればひとまずここで探索終了。
                else
                {
                    // 同名のファイルが存在している場合は確実に失敗。
                    if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0)
                    {
                        pos = -1;
                    }
                    // 同名のディレクトリが存在していればその下層を自動生成する。
                    else
                    {
                        ++pos;
                    }
                    break;
                }
            }
            // 基点を求められたならばそれ以降の階層を繰り返し生成。
            if (pos >= 0)
            {
                for (;;)
                {
                    // 終端に達したら成功。
                    if (pos >= length)
                    {
                        retval = TRUE;
                        break;
                    }
                    else
                    {
                        pos = FSi_IncrementSjisPositionToSlash(tmppath, pos);
                        tmppath[pos] = '\0';
                        if (!FS_CreateDirectory(tmppath, FS_PERMIT_R | FS_PERMIT_W))
                        {
                            break;
                        }
                        else
                        {
                            // 最上位の階層は記憶しておく。
                            if (root == 0)
                            {
                                FS_DEBUG_TRACE("    - we have created \"%s\" as root\n", tmppath);
                                root = pos;
                            }
                            tmppath[pos++] = '/';
                        }
                    }
                }
            }
        }
    }
    // 自動生成された最上位ディレクトリを記録。
    autogen[root] = '\0';
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileAuto

  Description:  必要な中間ディレクトリも含めてファイルを生成する

  Arguments:    path        パス名
                permit      アクセスモード

  Returns:      ファイルが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileAuto(const char *path, u32 permit)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (FSi_ComplementDirectory(path, autogen))
    {
        result = FS_CreateFile(path, permit);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFileAuto

  Description:  必要な中間ディレクトリも含めてファイルを削除する

  Arguments:    path        パス名

  Returns:      ファイルが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFileAuto(const char *path)
{
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    // 命名の統一性のために存在しているが実際には補完処理が不要。
    return FS_DeleteFile(path);
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFileAuto

  Description:  必要な中間ディレクトリも含めてファイル名を変更する

  Arguments:    src         変換元のファイル名
                dst         変換先のファイル名

  Returns:      ファイル名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFileAuto(const char *src, const char *dst)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__);
    if (FSi_ComplementDirectory(dst, autogen))
    {
        result = FS_RenameFile(src, dst);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectoryAuto

  Description:  必要な中間ディレクトリも含めてディレクトリを生成する

  Arguments:    path        生成するディレクトリ名
                permit      アクセスモード

  Returns:      ディレクトリが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectoryAuto(const char *path, u32 permit)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (FSi_ComplementDirectory(path, autogen))
    {
        result = FS_CreateDirectory(path, permit);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectoryAuto

  Description:  ディレクトリを再帰的に削除する

  Arguments:    path        パス名

  Returns:      ディレクトリが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectoryAuto(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (path && *path)
    {
        char    tmppath[FS_ARCHIVE_FULLPATH_MAX + 1];
        if (FSi_GetFullPath(tmppath, path))
        {
            int     pos;
            BOOL    mayBeEmpty;
            int     length = FSi_TrimSjisTrailingSlash(tmppath);
            FS_DEBUG_TRACE("  trying to force-delete \"%s\"\n", tmppath);
            mayBeEmpty = TRUE;
            for (pos = 0; pos >= 0;)
            {
                BOOL    failure = FALSE;
                // まず直接このディレクトリを削除してみて成功するなら1階層上へ。
                tmppath[length + pos] = '\0';
                if (mayBeEmpty && (FS_DeleteDirectory(tmppath) ||
                    (FS_GetArchiveResultCode(tmppath) == FS_RESULT_ALREADY_DONE)))
                {
                    FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
                    pos = FSi_DecrementSjisPositionToSlash(&tmppath[length], pos);
                }
                else
                {
                    // ディレクトリが開けるようなら全エントリを列挙。
                    FSFile  dir[1];
                    FS_InitFile(dir);
                    if (!FS_OpenDirectory(dir, tmppath, FS_FILEMODE_R))
                    {
                        FS_DEBUG_TRACE("  -> failed to delete & open \"%s\"\n", tmppath);
                        failure = TRUE;
                    }
                    else
                    {
                        FSDirectoryEntryInfo    info[1];
                        tmppath[length + pos] = '/';
                        mayBeEmpty = TRUE;
                        while (FS_ReadDirectory(dir, info))
                        {
                            (void)STD_CopyString(&tmppath[length + pos + 1], info->longname);
                            // ファイルが存在すれば削除。
                            if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0)
                            {
                                if (!FS_DeleteFile(tmppath))
                                {
                                    FS_DEBUG_TRACE("  -> failed to delete file \"%s\"\n", tmppath);
                                    failure = TRUE;
                                    break;
                                }
                                FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
                            }
                            // "." または ".." なら無視。
                            else if ((STD_CompareString(info->longname, ".") == 0) ||
                                     (STD_CompareString(info->longname, "..") == 0))
                            {
                            }
                            // 空でないディレクトリが存在すればさらに下層へ移動。
                            else if (!FS_DeleteDirectory(tmppath))
                            {
                                pos += 1 + STD_GetStringLength(info->longname);
                                mayBeEmpty = FALSE;
                                break;
                            }
                        }
                        (void)FS_CloseDirectory(dir);
                    }
                }
                // 成功すべき操作さえ失敗した場合は処理を中止。(ALREADY_DONEなど)
                if (failure)
                {
                    break;
                }
            }
            retval = (pos < 0);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectoryAuto

  Description:  必要な中間ディレクトリを自動生成してディレクトリ名を変更する

  Arguments:    src         変換元のディレクトリ名
                dst         変換先のディレクトリ名

  Returns:      ディレクトリ名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectoryAuto(const char *src, const char *dst)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    if (FSi_ComplementDirectory(dst, autogen))
    {
        result = FS_RenameDirectory(src, dst);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResource

  Description:  指定したアーカイブのリソース情報を取得する

  Arguments:    path        アーカイブを特定できるパス名
                resource    取得したリソース情報の格納先

  Returns:      リソース情報が正常に取得できればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_GetArchiveResource(const char *path, FSArchiveResource *resource)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArchive  *arc = FS_NormalizePath(path, NULL, NULL);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForGetArchiveResource arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->resource = resource;
            retval = FSi_SendCommand(file, FS_COMMAND_GETARCHIVERESOURCE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetSpaceToCreateDirectoryEntries

  Description:  ファイルの生成と同時に生成されるディレクトリエントリの容量を見積もる
                (パス中に存在するディレクトリも新規に作成することを想定する)

  Arguments:    path                生成したいファイルのパス名。
                bytesPerCluster     ファイルシステム上のクラスタあたりのバイト数。

  Returns:      容量
 *---------------------------------------------------------------------------*/
u32 FSi_GetSpaceToCreateDirectoryEntries(const char *path, u32 bytesPerCluster)
{
    static const u32    bytesPerEntry = 32UL;
    static const u32    longnamePerEntry = 13UL;
    // フルパスならスキームを除去しつつ各エントリを個別に判定。
    const char         *root = STD_SearchString(path, ":");
    const char         *current = (root != NULL) ? (root + 1) : path;
    u32                 totalBytes = 0;
    u32                 restBytesInCluster = 0;
    current += (*current == '/');
    while (*current)
    {
        BOOL    isShortName = FALSE;
        u32     entries = 0;
        // エントリ名の長さを算出。
        u32     len = (u32)FSi_IncrementSjisPositionToSlash(current, 0);
        // 8.3形式で表現できるエントリ名かどうか判定。
#if 0
        // (TWLで採用しているFATドライバは常に長いファイル名を保存するので
        //  実際にはここまで厳密な判定処理は必要なかった)
        {
            static const char  *alnum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            static const char  *special = "!#$%&'()*+-<>?@^_`{}~";
            if ((len <= 8 + 1 + 3) && STD_SearchChar(alnum, current[0]))
            {
                u32     namelen = 0;
                u32     extlen = 0;
                u32     scanned = 0;
                for (; namelen < len; ++namelen)
                {
                    char    c = current[scanned + namelen];
                    if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                    {
                        break;
                    }
                }
                scanned += namelen;
                if ((scanned < len) && (current[scanned] == '.'))
                {
                    ++scanned;
                    for (; scanned + extlen < len; ++extlen)
                    {
                        char    c = current[scanned + extlen];
                        if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                        {
                            break;
                        }
                    }
                    scanned += extlen;
                }
                if ((scanned == len) && (namelen <= 8) && (extlen <= 3))
                {
                    isShortName = TRUE;
                }
            }
        }
#endif
        // 8.3形式でなければ長いファイル名のために追加エントリが必要。
        if (!isShortName)
        {
            entries += ((len + longnamePerEntry - 1UL) / longnamePerEntry);
        }
        // いずれにせよ1エントリ分は常に消費したとみなす。
        entries += 1;
        current += len;
        // すでに自分が生成したディレクトリのクラスタ余白で足りないなら
        // 超過分のエントリをクラスタ単位で消費する。
        {
            int     over = (int)(entries * bytesPerEntry - restBytesInCluster);
            if (over > 0)
            {
                totalBytes += MATH_ROUNDUP(over, bytesPerCluster);
            }
        }
        // さらに下層があるならディレクトリとして1クラスタ消費し、
        // そのうち "." ".." の2エントリ分を引いたサイズが余白となる。
        if (*current != '\0')
        {
            current += 1;
            totalBytes += bytesPerCluster;
            restBytesInCluster = bytesPerCluster - (2 * bytesPerEntry);
        }
    }
    return totalBytes;
}

/*---------------------------------------------------------------------------*
  Name:         FS_HasEnoughSpaceToCreateFile

  Description:  指定したパス名とサイズを持つファイルが現在生成可能か判定する

  Arguments:    resource    事前にFS_GetArchiveResource関数で取得したアーカイブ情報。
                            関数が成功すると、ファイルが消費する分だけ各サイズが減少する。
                path        生成したいファイルのパス名。
                size        生成したいファイルのサイズ。

  Returns:      指定したパス名とサイズを持つファイルが生成可能であればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource, const char *path, u32 size)
{
    BOOL    retval = FALSE;
    u32 bytesPerCluster = resource->bytesPerSector * resource->sectorsPerCluster;
    if (bytesPerCluster != 0)
    {
        u32     needbytes = (FSi_GetSpaceToCreateDirectoryEntries(path, bytesPerCluster) +
                             MATH_ROUNDUP(size, bytesPerCluster));
        u32     needclusters = needbytes / bytesPerCluster;
        if (needclusters <= resource->availableClusters)
        {
            resource->availableClusters -= needclusters;
            resource->availableSize -= needbytes;
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveReady

  Description:  指定したアーカイブが現在使用可能か判定する

  Arguments:    path        "アーカイブ名:"で始まる絶対パス

  Returns:      指定したアーカイブが現在使用可能であればTRUE。
                スロットに挿入されていないSDメモリカードアーカイブや
                まだインポートされていないセーブデータアーカイブならFALSE。
 *---------------------------------------------------------------------------*/
BOOL FS_IsArchiveReady(const char *path)
{
    FSArchiveResource   resource[1];
    return FS_GetArchiveResource(path, resource);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FlushFile

  Description:  ファイルの変更内容をデバイスへ反映する

  Arguments:    file        ファイルハンドル

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
FSResult FS_FlushFile(FSFile *file)
{
    FSResult    retval = FS_RESULT_ERROR;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        (void)FSi_SendCommand(file, FS_COMMAND_FLUSHFILE, TRUE);
        retval = FS_GetResultCode(file);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileLength

  Description:  ファイルサイズを変更する

  Arguments:    file        ファイルハンドル
                length      変更後のサイズ

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
FSResult FS_SetFileLength(FSFile *file, u32 length)
{
    FSResult    retval = FS_RESULT_ERROR;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForSetFileLength  arg[1];
        file->argument = arg;
        arg->length = length;
        (void)FSi_SendCommand(file, FS_COMMAND_SETFILELENGTH, TRUE);
        retval = FS_GetResultCode(file);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathName

  Description:  指定したハンドルのパス名を取得する

  Arguments:    file        ファイルまたはディレクトリ
                buffer      パス格納先
                length      バッファサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_GetPathName(FSFile *file, char *buffer, u32 length)
{
    BOOL    retval = FALSE;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) || FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForGetPath    arg[1];
        file->argument = arg;
        arg->is_directory = FS_IsDir(file);
        arg->buffer = buffer;
        arg->length = length;
        retval = FSi_SendCommand(file, FS_COMMAND_GETPATH, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathLength

  Description:  指定したファイルまたはディレクトリのフルパス名の長さを取得

  Arguments:    file        ファイルまたはディレクトリハンドル

  Returns:      成功すれば終端の'\0'も含めたパス名の長さ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32 FS_GetPathLength(FSFile *file)
{
    s32     retval = -1;
    if (FS_GetPathName(file, NULL, 0))
    {
        retval = file->arg.getpath.total_len;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ConvertPathToFileID

  Description:  パス名からファイルIDを取得する

  Arguments:    p_fileid    FSFileIDの格納先
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(p_fileid);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile      file[1];
            FSArgumentForFindPath   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->target_is_directory = FALSE;
            if (FSi_SendCommand(file, FS_COMMAND_FINDPATH, TRUE))
            {
                p_fileid->arc = arc;
                p_fileid->file_id = arg->target_id;
                retval = TRUE;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileDirect

  Description:  アーカイブ内の領域を直接指定してファイルを開く

  Arguments:    file         ハンドル情報を保持するFSFile
                arc          マップ元になるアーカイブ
                image_top    ファイルイメージ先頭のオフセット
                image_bottom ファイルイメージ終端のオフセット
                id           任意に指定するファイルID

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileDirect(FSFile *file, FSArchive *arc,
                       u32 image_top, u32 image_bottom, u32 id)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(arc);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(!FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForOpenFileDirect arg[1];
        file->arc = arc;
        file->argument = arg;
        arg->id = id;
        arg->top = image_top;
        arg->bottom = image_bottom;
        arg->mode = 0;
        retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEDIRECT, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileFast

  Description:  IDを指定してファイルを開く

  Arguments:    file         ハンドル情報を保持するFSFile
                id           開くべきファイルを指すFSFileID

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileFast(FSFile *file, FSFileID id)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(!FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    if (id.arc)
    {
        FSArgumentForOpenFileFast   arg[1];
        file->arc = id.arc;
        file->argument = arg;
        arg->id = id.file_id;
        arg->mode = 0;
        retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEFAST, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileEx

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // FS_FILEMODE_L に関する論理チェック。
    // (creationモードで開きつつサイズ制限したら意味が無い)
    if (((mode & FS_FILEMODE_L) != 0) &&
        ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W))
    {
        OS_TWarning("\"FS_FILEMODE_WL\" seems useless.\n"
                    "(this means creating empty file and prohibiting any modifications)");
    }
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSArgumentForOpenFile   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->mode = mode;
            if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CloseFile

  Description:  ファイルを閉じる

  Arguments:    file        ファイルハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CloseFile(FSFile *file)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        retval = FSi_SendCommand(file, FS_COMMAND_CLOSEFILE, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetSeekCacheSize

  Description:  高速逆シーク用のフルキャッシュに必要なバッファサイズを求める

  Arguments:    path

  Returns:      成功すればサイズ、失敗すれば0
 *---------------------------------------------------------------------------*/
u32 FS_GetSeekCacheSize(const char *path)
{
    u32         retval = 0;
    // ファイルが存在するならサイズを取得する。
    FSPathInfo  info;
    if (FS_GetPathInfo(path, &info) &&
        ((info.attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0))
    {
        // 該当するアーカイブのFAT情報を取得。
        FSArchiveResource   resource;
        if (FS_GetArchiveResource(path, &resource))
        {
            // 実際にFATベースのアーカイブであればキャッシュサイズを算出。
            u32     bytesPerCluster = resource.sectorsPerCluster * resource.bytesPerSector;
            if (bytesPerCluster != 0)
            {
                static const u32    fatBits = 32;
                retval = (u32)((info.filesize + bytesPerCluster - 1) / bytesPerCluster) * ((fatBits + 4) / 8);
				// バッファ前後をキャッシュラインに整合させるため余白を追加。
                retval += (u32)(HW_CACHE_LINE_SIZE * 2);
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetSeekCache

  Description:  高速逆シーク用のキャッシュバッファを割り当てる

  Arguments:    file        ファイルハンドル
                buf         キャッシュバッファ
                buf_size    キャッシュバッファサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SetSeekCache(FSFile *file, void* buf, u32 buf_size)
{
    FSArgumentForSetSeekCache    arg[1];
    BOOL     retval = FALSE;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));

    file->argument = arg;
    arg->buf      = buf;
    arg->buf_size = buf_size;
    retval = FSi_SendCommand(file, FS_COMMAND_SETSEEKCACHE, TRUE);

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileLength

  Description:  ファイルサイズを取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルサイズ
 *---------------------------------------------------------------------------*/
u32 FS_GetFileLength(FSFile *file)
{
    u32     retval = 0;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    // アーカイブプロシージャの場合はこの場で直接参照可能。
    if (!FSi_GetFileLengthIfProc(file, &retval))
    {
        FSArgumentForGetFileLength    arg[1];
        file->argument = arg;
        arg->length = 0;
        if (FSi_SendCommand(file, FS_COMMAND_GETFILELENGTH, TRUE))
        {
            retval = arg->length;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFilePosition

  Description:  ファイルポインタの現在位置を取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルポインタの現在位置
 *---------------------------------------------------------------------------*/
u32 FS_GetFilePosition(FSFile *file)
{
    u32     retval = 0;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    // アーカイブプロシージャの場合はこの場で直接参照可能。
    if (!FSi_GetFilePositionIfProc(file, &retval))
    {
        FSArgumentForGetFilePosition    arg[1];
        file->argument = arg;
        arg->position = 0;
        if (FSi_SendCommand(file, FS_COMMAND_GETFILEPOSITION, TRUE))
        {
            retval = arg->position;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFile

  Description:  ファイルポインタを移動

  Arguments:    file        ファイルハンドル
                offset      移動量
                origin      移動起点

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    {
        FSArgumentForSeekFile   arg[1];
        file->argument = arg;
        arg->offset = (int)offset;
        arg->from = origin;
        retval = FSi_SendCommand(file, FS_COMMAND_SEEKFILE, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFile

  Description:  ファイルからデータを読み出し

  Arguments:    file        ファイルハンドル
                buffer      転送先バッファ
                length      読み出しサイズ

  Returns:      成功すれば実際に読み出したサイズ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32 FS_ReadFile(FSFile *file, void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForReadFile   arg[1];
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        if (FSi_SendCommand(file, FS_COMMAND_READFILE, TRUE))
        {
            length = (s32)arg->length;
        }
        else
        {
            if( file->error == FS_RESULT_INVALID_PARAMETER) {
                length = -1; //そもそもリードしなかった場合
            }else{
                length = (s32)arg->length; //リードを試みた場合は-1以上の値が入っている
            }
        }
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFileAsync

  Description:  ファイルからデータを非同期的に読み出し

  Arguments:    file        ファイルハンドル
                buffer      転送先バッファ
                length      読み出しサイズ

  Returns:      成功すれば単にlengthと同じ値、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32 FS_ReadFileAsync(FSFile *file, void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    // アーカイブプロシージャの場合はこの場でサイズ補正
    {
        u32     end, pos;
        if (FSi_GetFilePositionIfProc(file, &pos) &&
            FSi_GetFileLengthIfProc(file, &end) &&
            (pos + length > end))
        {
            length = (s32)(end - pos);
        }
    }
    {
        FSArgumentForReadFile  *arg = (FSArgumentForReadFile*)file->reserved2;
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        (void)FSi_SendCommand(file, FS_COMMAND_READFILE, FALSE);
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFile

  Description:  ファイルへデータを書き込み

  Arguments:    file        ファイルハンドル
                buffer      転送元バッファ
                length      書き込みサイズ

  Returns:      成功すれば実際に書き込んだサイズ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32 FS_WriteFile(FSFile *file, const void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForWriteFile  arg[1];
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        if (FSi_SendCommand(file, FS_COMMAND_WRITEFILE, TRUE))
        {
            length = (s32)arg->length;
        }
        else
        {
            if( file->error == FS_RESULT_INVALID_PARAMETER) {
                length = -1; //そもそもライトしなかった場合
            }else{
                length = (s32)arg->length; //ライトを試みた場合は-1以上の値が入っている
            }
        }
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFileAsync

  Description:  ファイルへデータを非同期的に書き込み

  Arguments:    file        ファイルハンドル
                buffer      転送元バッファ
                length      書き込みサイズ

  Returns:      成功すれば単にlengthと同じ値、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32 FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    // アーカイブプロシージャの場合はこの場でサイズ補正
    {
        u32     end, pos;
        if (FSi_GetFilePositionIfProc(file, &pos) &&
            FSi_GetFileLengthIfProc(file, &end) &&
            (pos + length > end))
        {
            length = (s32)(end - pos);
        }
    }
    {
        FSArgumentForWriteFile *arg = (FSArgumentForWriteFile*)file->reserved2;
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        (void)FSi_SendCommand(file, FS_COMMAND_WRITEFILE, FALSE);
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectory

  Description:  ディレクトリハンドルを開く

  Arguments:    file        FSFile構造体
                path        パス名
                mode        アクセスモード

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectory(FSFile *file, const char *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSArgumentForOpenDirectory  arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->mode = mode;
            if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CloseDirectory

  Description:  ディレクトリハンドルを閉じる

  Arguments:    file        FSFile構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CloseDirectory(FSFile *file)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        if (FSi_SendCommand(file, FS_COMMAND_CLOSEDIRECTORY, TRUE))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectory

  Description:  ディレクトリのエントリを1個だけ読み進める

  Arguments:    file        FSFile構造体
                info        FSDirectoryEntryInfo構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForReadDirectory  arg[1];
        file->argument = arg;
        arg->info = info;
        MI_CpuFill8(info, 0x00, sizeof(info));
        info->id = FS_INVALID_FILE_ID;
        if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekDir

  Description:  ディレクトリ位置を指定して開く

  Arguments:    file        FSFile構造体
                pos         FS_ReadDirやFS_TellDirで取得したディレクトリ位置

  Returns:      成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SeekDir(FSFile *file, const FSDirPos *pos)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(pos);
    SDK_NULL_ASSERT(pos->arc);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForSeekDirectory  arg[1];
        arg->id = (u32)((pos->own_id << 0) | (pos->index << 16));
        arg->position = pos->pos;
        file->arc = pos->arc;
        file->argument = arg;
        if (FSi_SendCommand(file, FS_COMMAND_SEEKDIR, TRUE))
        {
            file->stat |= FS_FILE_STATUS_IS_DIR;
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_TellDir

  Description:  ディレクトリハンドルから現在のディレクトリ位置を取得

  Arguments:    dir         ディレクトリハンドル
                pos         ディレクトリ位置の格納先

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_TellDir(const FSFile *dir, FSDirPos *pos)
{
    BOOL        retval = FALSE;
    SDK_NULL_ASSERT(dir);
    SDK_NULL_ASSERT(pos);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(dir));
    {
        *pos = dir->prop.dir.pos;
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RewindDir

  Description:  ディレクトリハンドルの列挙位置を先頭へ戻す

  Arguments:    dir         ディレクトリハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RewindDir(FSFile *dir)
{
    BOOL        retval = FALSE;
    SDK_NULL_ASSERT(dir);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(dir));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    {
        FSDirPos pos;
        pos.arc = dir->arc;
        pos.own_id = dir->prop.dir.pos.own_id;
        pos.pos = 0;
        pos.index = 0;
        retval = FS_SeekDir(dir, &pos);
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * Unicode support
 *---------------------------------------------------------------------------*/

enum
{
    FS_UNICODE_CONVSRC_ASCII,
    FS_UNICODE_CONVSRC_SHIFT_JIS,
    FS_UNICODE_CONVSRC_UNICODE
};

/*---------------------------------------------------------------------------*
  Name:         FSi_CopySafeUnicodeString

  Description:   バッファサイズを確認しつつ文字列をUnicodeとしてコピー。

  Arguments:    dst           転送先バッファ
                dstlen        転送先サイズ
                src           転送元バッファ
                srclen        転送先文字サイズ
                srctype       転送元の文字セット
                stickyFailure 転送元の切り捨てが起こったらFALSE
                
  Returns:      実際に格納された文字数。
 *---------------------------------------------------------------------------*/
static int FSi_CopySafeUnicodeString(u16 *dst, int dstlen,
                                     const void *srcptr, int srclen,
                                     int srctype, BOOL *stickyFailure)
{
    int     srcpos = 0;
    int     dstpos = 0;
    if (srctype == FS_UNICODE_CONVSRC_ASCII)
    {
        const char *src = (const char *)srcptr;
        int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
        while ((dstpos < n) && src[srcpos])
        {
            dst[dstpos++] = (u8)src[srcpos++];
        }
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    else if (srctype == FS_UNICODE_CONVSRC_UNICODE)
    {
        const u16 *src = (const u16 *)srcptr;
        int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
        while ((dstpos < n) && src[srcpos])
        {
            dst[dstpos++] = src[srcpos++];
        }
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    else if (srctype == FS_UNICODE_CONVSRC_SHIFT_JIS)
    {
        const char *src = (const char *)srcptr;
        srcpos = srclen;
        dstpos = dstlen - 1;
        (void)FSi_ConvertStringSjisToUnicode(dst, &dstpos, src, &srcpos, NULL);
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    dst[dstpos] = L'\0';
    return dstpos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_NormalizePathWtoW

  Description:  Unicodeパスをアーカイブ名まで含むUnicodeフルパスへ変換。

  Arguments:    path        正規化されていないパス文字列
                baseid      基準ディレクトリIDの格納先またはNULL
                relpath     変換後のパス名格納先またはNULL

  Returns:      アーカイブポインタまたはNULL
 *---------------------------------------------------------------------------*/
FSArchive* FSi_NormalizePathWtoW(const u16 *path, u32*baseid, u16 *relpath);
FSArchive* FSi_NormalizePathWtoW(const u16 *path, u32*baseid, u16 *relpath)
{
    FSArchive  *arc = NULL;
    int         pathlen = 0;
    int         pathmax = FS_ARCHIVE_FULLPATH_MAX + 1;
    BOOL        stickyFailure = FALSE;
    // まずコマンド対象となるアーカイブを特定。
    // 指定されたUnicodeパスが絶対パスならアーカイブを取得。
    BOOL        absolute = FALSE;
    int         arcnameLen;
    for (arcnameLen = 0; arcnameLen < FS_ARCHIVE_NAME_LONG_MAX + 1; ++arcnameLen)
    {
        if (path[arcnameLen] == L'\0')
        {
            break;
        }
        else if (FSi_IsUnicodeSlash(path[arcnameLen]))
        {
            break;
        }
        else if (path[arcnameLen] == L':')
        {
            char    arcname[FS_ARCHIVE_NAME_LONG_MAX + 1];
            int     j;
            for (j = 0; j < arcnameLen; ++j)
            {
                arcname[j] = (char)path[j];
            }
            arcname[arcnameLen] = '\0';
            arc = FS_FindArchive(arcname, arcnameLen);
            break;
        }
    }
    if (arc)
    {
        absolute = TRUE;
        *baseid = 0;
    }
    else
    {
        arc = FS_NormalizePath("", baseid, NULL);
    }
    if (arc)
    {
        // アーカイブがUnicode対応可能でなければここで失敗。
        u32     caps = 0;
        (void)arc->vtbl->GetArchiveCaps(arc, &caps);
        if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0)
        {
            arc = NULL;
        }
        else
        {
            // 先頭にアーカイブ名を格納。
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 FS_GetArchiveName(arc), FS_ARCHIVE_NAME_LONG_MAX,
                                                 FS_UNICODE_CONVSRC_ASCII, &stickyFailure);
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 L":", 1,
                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
            // 絶対パスならルート以下をそのまま連結。
            if (absolute)
            {
                path += arcnameLen + 1 + FSi_IsUnicodeSlash(path[arcnameLen + 1]);
            }
            // カレントルートならルート以下を直接連結。
            else if (FSi_IsUnicodeSlash(*path))
            {
                path += 1;
            }
            // カレントディレクトリならShift_JIS->Unicode変換して連結。
            else
            {
                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                     L"/", 1,
                                                     FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                     FS_GetCurrentDirectory(), FS_ENTRY_LONGNAME_MAX,
                                                     FS_UNICODE_CONVSRC_SHIFT_JIS, &stickyFailure);
            }
            // 残りの部分を連結。
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 L"/", 1,
                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
            {
                // 特殊エントリ名に注意しつつ相対パスを正規化。
                int     curlen = 0;
                while (!stickyFailure)
                {
                    u16     c = path[curlen];
                    if ((c != L'\0') && !FSi_IsUnicodeSlash(c))
                    {
                        curlen += 1;
                    }
                    else
                    {
                        // 空ディレクトリは無視。
                        if (curlen == 0)
                        {
                        }
                        // "." (カレントディレクトリ)は無視。
                        else if ((curlen == 1) && (path[0] == L'.'))
                        {
                        }
                        // ".." (親ディレクトリ)はルートを上限として1階層上がる。
                        else if ((curlen == 2) && (path[0] == '.') && (path[1] == '.'))
                        {
                            if ((pathlen > 2) && (relpath[pathlen - 2] != L':'))
                            {
                                --pathlen;
                                pathlen = FSi_DecrementUnicodePositionToSlash(relpath, pathlen) + 1;
                            }
                        }
                        // それ以外はエントリ追加。
                        else
                        {
                            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                                 path, curlen,
                                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                            if (c != L'\0')
                            {
                                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                                     L"/", 1,
                                                                     FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                            }
                        }
                        if (c == L'\0')
                        {
                            break;
                        }
                        path += curlen + 1;
                        curlen = 0;
                    }
                }
            }
            relpath[pathlen] = L'\0';
        }
    }
    return stickyFailure ? NULL : arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileExW

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // FS_FILEMODE_L に関する論理チェック。
    // (creationモードで開きつつサイズ制限したら意味が無い)
    if (((mode & FS_FILEMODE_L) != 0) &&
        ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W))
    {
        OS_TWarning("\"FS_FILEMODE_WL\" seems useless.\n"
                    "(this means creating empty file and prohibiting any modifications)");
    }
    {
        u16         relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);
        // 現時点ではUnicodeを使用可能な最小の変更にとどめるために
        // ROMなどUnicode非対応のアーカイブではUnsupportedを返す。
        if (!arc)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForOpenFile   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = (char*)relpath;
            arg->mode = mode;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectoryW

  Description:  ディレクトリハンドルを開く

  Arguments:    file        FSFile構造体
                path        パス名
                mode        アクセスモード

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        u16         relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);
        // 現時点ではUnicodeを使用可能な最小の変更にとどめるために
        // ROMなどUnicode非対応のアーカイブではUnsupportedを返す。
        if (!arc)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForOpenDirectory  arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = (char*)relpath;
            arg->mode = mode;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectoryW

  Description:  ディレクトリのエントリを1個だけ読み進める

  Arguments:    file        FSFile構造体
                info        FSDirectoryEntryInfo構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArchive  *arc = file->arc;
        // 現在はUnicodeを使用できる最小の変更にとどめるため、
        // ROMなどUnicode非対応のアーカイブではUnsupportedを返す。
        u32     caps = 0;
        (void)arc->vtbl->GetArchiveCaps(arc, &caps);
        if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForReadDirectory  arg[1];
            file->argument = arg;
            arg->info = (FSDirectoryEntryInfo*)info;
            MI_CpuFill8(info, 0x00, sizeof(info));
            info->id = FS_INVALID_FILE_ID;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE))
            {
                retval = TRUE;
            }
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * obsolete functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertToDirEntry

  Description:  FSDirectoryEntryInfo構造体からFSDirEntry構造体への変換

  Arguments:    entry       変換先のFSDirEntry構造体
                info        変換元のFSDirectoryEntryInfo構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
static void FSi_ConvertToDirEntry(FSDirEntry *entry, const FSDirectoryEntryInfo *info)
{
    entry->name_len = info->longname_length;
    if (entry->name_len > sizeof(entry->name) - 1)
    {
        entry->name_len = sizeof(entry->name) - 1;
    }
    MI_CpuCopy8(info->longname, entry->name, entry->name_len);
    entry->name[entry->name_len] = '\0';
    if (info->id == FS_INVALID_FILE_ID)
    {
        entry->is_directory = FALSE;
        entry->file_id.file_id = FS_INVALID_FILE_ID;
    }
    else if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0)
    {
        entry->is_directory = TRUE;
        entry->dir_id.own_id = (u16)(info->id >> 0);
        entry->dir_id.index = (u16)(info->id >> 16);
        entry->dir_id.pos = 0;
    }
    else
    {
        entry->is_directory = FALSE;
        entry->file_id.file_id = info->id;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFile

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFile(FSFile *file, const char *path)
{
    return FS_OpenFileEx(file, path, FS_FILEMODE_R);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetLength

  Description:  ファイルサイズを取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルサイズ
 *---------------------------------------------------------------------------*/
u32 FS_GetLength(FSFile *file)
{
    return FS_GetFileLength(file);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPosition

  Description:  ファイルポインタの現在位置を取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルポインタの現在位置
 *---------------------------------------------------------------------------*/
u32 FS_GetPosition(FSFile *file)
{
    return FS_GetFilePosition(file);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FindDir

  Description:  ディレクトリハンドルを開く

  Arguments:    dir         FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_FindDir(FSFile *dir, const char *path)
{
    return FS_OpenDirectory(dir, path, FS_FILEMODE_R);
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDir

  Description:  ディレクトリのエントリを1個だけ読み進める

  Arguments:    file        FSFile構造体
                entry       FSDirEntry構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDir(FSFile *file, FSDirEntry *entry)
{
    BOOL                    retval = FALSE;
    FSDirectoryEntryInfo    info[1];
    if (FS_ReadDirectory(file, info))
    {
        FSi_ConvertToDirEntry(entry, info);
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ChangeDir

  Description:  カレントディレクトリを変更する

  Arguments:    path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ChangeDir(const char *path)
{
    return FS_SetCurrentDirectory(path);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileInfo

  Description:  ファイル情報を取得する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
FSResult FS_GetFileInfo(const char *path, FSFileInfo *info)
{
    return FS_GetPathInfo(path, info) ? FS_RESULT_SUCCESS : FS_GetArchiveResultCode(path);
}


#endif /* FS_IMPLEMENT */

// 以下はFSライブラリ以外でも使用されるのでFS_IMPLEMENTの対象外。
// ARM7でもUnicode対応が必要になるのはTWL動作時のみなので拡張メモリへ配置。
#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_begin.h>
#endif

static const int        FSiUnicodeBufferQueueMax = 4;
static OSMessageQueue   FSiUnicodeBufferQueue[1];
static OSMessage        FSiUnicodeBufferQueueArray[FSiUnicodeBufferQueueMax];
static BOOL             FSiUnicodeBufferQueueInitialized = FALSE;
static u16              FSiUnicodeBufferTable[FSiUnicodeBufferQueueMax][FS_ARCHIVE_FULLPATH_MAX + 1];

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeBuffer

  Description:  Unicode変換用の一時バッファを取得。
                FSライブラリがShift_JISを変換するために使用される。

  Arguments:    src : Unicode変換の必要なShift_JIS文字列またはNULL

  Returns:      必要に応じてUTF16-LEへ変換された文字列バッファ
 *---------------------------------------------------------------------------*/
u16* FSi_GetUnicodeBuffer(const char *src)
{
    u16        *retval = NULL;
    // 初回呼び出し時にバッファをメッセージキューへ追加。
    OSIntrMode  bak = OS_DisableInterrupts();
    if (!FSiUnicodeBufferQueueInitialized)
    {
        int     i;
        FSiUnicodeBufferQueueInitialized = TRUE;
        OS_InitMessageQueue(FSiUnicodeBufferQueue, FSiUnicodeBufferQueueArray, 4);
        for (i = 0; i < FSiUnicodeBufferQueueMax; ++i)
        {
            (void)OS_SendMessage(FSiUnicodeBufferQueue, FSiUnicodeBufferTable[i], OS_MESSAGE_BLOCK);
        }
    }
    (void)OS_RestoreInterrupts(bak);
    // メッセージキューからバッファを確保。(無ければここでブロッキング)
    (void)OS_ReceiveMessage(FSiUnicodeBufferQueue, (OSMessage*)&retval, OS_MESSAGE_BLOCK);
    if (src)
    {
        int     dstlen = FS_ARCHIVE_FULLPATH_MAX;
        (void)FSi_ConvertStringSjisToUnicode(retval, &dstlen, src, NULL, NULL);
        retval[dstlen] = L'\0';
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReleaseUnicodeBuffer

  Description:  Unicode変換用の一時バッファを解放。

  Arguments:    buf : FSi_GetUnicodeBuffer()で確保したバッファ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ReleaseUnicodeBuffer(const void *buf)
{
    if (buf)
    {
        // 使用したバッファをメッセージキューへ返却。
        (void)OS_SendMessage(FSiUnicodeBufferQueue, (OSMessage)buf, OS_MESSAGE_BLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringSjisToUnicode

  Description:  ShiftJIS文字列をUnicode文字列に変換。
                扱うパス名が明らかにASCIIのみである場合など
                UnicodeとShiftJISの相互変換を簡略化できる場合は
                この関数をオーバーライドすることによって
                STDライブラリの標準処理がリンクされるのを防ぐことができる。

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
 __attribute__((never_inline))
{
    return STD_ConvertStringSjisToUnicode(dst, dst_len, src, src_len, callback);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringUnicodeToSjis

  Description:  Unicode文字列をShiftJIS文字列に変換。
                扱うパス名が明らかにASCIIのみである場合など
                UnicodeとShiftJISの相互変換を簡略化できる場合は
                この関数をオーバーライドすることによって
                STDライブラリの標準処理がリンクされるのを防ぐことができる。

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
 __attribute__((never_inline))
{
    return STD_ConvertStringUnicodeToSjis(dst, dst_len, src, src_len, callback);
}

#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_end.h>
#endif
