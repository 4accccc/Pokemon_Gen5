/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_romfat_default.c

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


#include <nitro/fs.h>

#include <nitro/mi/memory.h>
#include <nitro/math/math.h>
#include <nitro/std.h>


#include "../include/util.h"
#include "../include/command.h"
#include "../include/rom.h"


#if defined(FS_IMPLEMENT)


/*---------------------------------------------------------------------------*/
/* declarations */

// 同期系コマンドのブロッキング読み込み構造体
typedef struct FSiSyncReadParam
{
    FSArchive  *arc;
    u32         pos;
}
FSiSyncReadParam;

// ビットとアラインメントのマクロ
#define	BIT_MASK(n)	((1 << (n)) - 1)


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_SeekAndReadSRL

  Description:  ROMハッシュコンテキストから呼び出されるSRLアクセスコールバック。

  Arguments:    userdata         任意のユーザ定義データ
                buffer           転送先バッファ
                offset           転送元ROMオフセット
                length           転送サイズ

  Returns:      転送サイズ
 *---------------------------------------------------------------------------*/
static int FSi_SeekAndReadSRL(void *userdata, void *buffer, u32 offset, u32 length)
{
    FSFile     *file = (FSFile*)userdata;
    if (file)
    {
        (void)FS_SeekFile(file, (int)offset, FS_SEEK_SET);
        (void)FS_ReadFile(file, buffer, (int)length);
    }
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_TranslateCommand

  Description:  ユーザプロシージャまたはデフォルト処理を呼び出し結果を返す。
                同期系コマンドが非同期的応答を返した場合は内部で完了を待つ。
                非同期系コマンドが非同期的応答を返した場合はそのまま返す。

  Arguments:    p_file           コマンド引数の格納されたFSFile構造体
                command          コマンドID
                block            ブロッキングするならTRUE

  Returns:      コマンドの処理結果.
 *---------------------------------------------------------------------------*/
static FSResult FSi_TranslateCommand(FSFile *p_file, FSCommandType command, BOOL block);

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadTable

  Description:  同期系コマンド内で同期リードを実行

  Arguments:    p                同期リード構造体
                dst              読み込みデータ格納先バッファ
                len              読み込みバイト数

  Returns:      同期リードの結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadTable(FSiSyncReadParam * p, void *dst, u32 len)
{
    FSResult                result;
    FSArchive       * const arc = p->arc;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);


    if (context->load_mem)
    {
        MI_CpuCopy8((const void *)p->pos, dst, len);
        result = FS_RESULT_SUCCESS;
    }
    else
    {
        result = (*context->read_func) (arc, dst, p->pos, len);
        result = FSi_WaitForArchiveCompletion(arc->list, result);
    }
    p->pos += len;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SeekDirDirect

  Description:  指定ディレクトリの先頭へ直接移動

  Arguments:    file             ディレクトリリストを格納するポインタ
                id               ディレクトリID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_SeekDirDirect(FSFile *file, u32 id)
{
    file->arg.seekdir.pos.arc = file->arc;
    file->arg.seekdir.pos.own_id = (u16)id;
    file->arg.seekdir.pos.index = 0;
    file->arg.seekdir.pos.pos = 0;
    (void)FSi_TranslateCommand(file, FS_COMMAND_SEEKDIR, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SeekDirDefault

  Description:  SEEKDIRコマンドのデフォルト実装

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_SeekDirDefault(FSFile *file)
{
    FSResult                result;
    FSArchive       * const arc = file->arc;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);

    const FSDirPos  * const arg = &file->arg.seekdir.pos;
    FSArchiveFNT            fnt;
    FSiSyncReadParam        param;
    param.arc = arc;
    param.pos = context->fnt + arg->own_id * sizeof(fnt);
    result = FSi_ReadTable(&param, &fnt, sizeof(fnt));
    if (result == FS_RESULT_SUCCESS)
    {
        file->prop.dir.pos = *arg;
        if ((arg->index == 0) && (arg->pos == 0))
        {
            file->prop.dir.pos.index = fnt.index;
            file->prop.dir.pos.pos = context->fnt + fnt.start;
        }
        file->prop.dir.parent = (u32)(fnt.parent & BIT_MASK(12));
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadDirDefault

  Description:  READDIRコマンドのデフォルト実装

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadDirDefault(FSFile *file)
{
    FSResult                result;
    FSDirEntry             *entry = file->arg.readdir.p_entry;
    u8                      len;

    FSiSyncReadParam        param;
    param.arc = file->arc;
    param.pos = file->prop.dir.pos.pos;

    result = FSi_ReadTable(&param, &len, sizeof(len));

    if (result == FS_RESULT_SUCCESS)
    {
        entry->name_len = (u32)(len & BIT_MASK(7));
        entry->is_directory = (u32)((len >> 7) & 1);
        if (entry->name_len == 0)
        {
            result = FS_RESULT_FAILURE;
        }
        else
        {
            if (!file->arg.readdir.skip_string)
            {
                result = FSi_ReadTable(&param, entry->name, entry->name_len);
                if (result != FS_RESULT_SUCCESS)
                {
                    return result;
                }
                entry->name[entry->name_len] = '\0';
            }
            else
            {
                param.pos += entry->name_len;
            }
            if (!entry->is_directory)
            {
                entry->file_id.arc = file->arc;
                entry->file_id.file_id = file->prop.dir.pos.index;
                ++file->prop.dir.pos.index;
            }
            else
            {
                u16     id;
                result = FSi_ReadTable(&param, &id, sizeof(id));
                if (result == FS_RESULT_SUCCESS)
                {
                    entry->dir_id.arc = file->arc;
                    entry->dir_id.own_id = (u16)(id & BIT_MASK(12));
                    entry->dir_id.index = 0;
                    entry->dir_id.pos = 0;
                }
            }
            if (result == FS_RESULT_SUCCESS)
            {
                file->prop.dir.pos.pos = param.pos;
            }
        }
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_FindPathDefault

  Description:  FINDPATHコマンドのデフォルト実装
                (SEEKDIR, READDIR コマンドを使用する)

  Arguments:    p_dir            コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_FindPathDefault(FSFile *p_dir)
{
    const char *path = p_dir->arg.findpath.path;
    const BOOL  is_dir = p_dir->arg.findpath.find_directory;

    // 指定のディレクトリを基準にパス名検索
    p_dir->arg.seekdir.pos = p_dir->arg.findpath.pos;
    (void)FSi_TranslateCommand(p_dir, FS_COMMAND_SEEKDIR, TRUE);
    for (; *path; path += (*path ? 1 : 0))
    {
        // 検索対象となるエントリ名を抽出
        int     name_len = FSi_IncrementSjisPositionToSlash(path, 0);
        u32     is_directory = ((path[name_len] != '\0') || is_dir);
        // "//" などの不正なディレクトリを検出したら失敗
        if (name_len == 0)
        {
            return FS_RESULT_INVALID_PARAMETER;
        }
        // 特殊なディレクトリ指定
        else if (*path == '.')
        {
            // "." はカレントディレクトリ
            if (name_len == 1)
            {
                path += 1;
                continue;
            }
            // ".." は親ディレクトリ
            else if ((name_len == 2) & (path[1] == '.'))
            {
                if (p_dir->prop.dir.pos.own_id != 0)
                {
                    FSi_SeekDirDirect(p_dir, p_dir->prop.dir.parent);
                }
                path += 2;
                continue;
            }
        }
        else if (*path == '*')
        {
            // "*" はワイルドカード指定 (アーカイブプロシージャではこれを無視する)
            break;
        }
        // 長すぎるエントリ名を検出したら失敗
        if (name_len > FS_FILE_NAME_MAX)
        {
            return FS_RESULT_NO_ENTRY;
        }
        // エントリを列挙して順次比較
        else
        {
            FSDirEntry etr;
            p_dir->arg.readdir.p_entry = &etr;
            p_dir->arg.readdir.skip_string = FALSE;
            for (;;)
            {
                // 終端に達しても該当エントリが存在しなかったら失敗
                if (FSi_TranslateCommand(p_dir, FS_COMMAND_READDIR, TRUE) != FS_RESULT_SUCCESS)
                {
                    return FS_RESULT_NO_ENTRY;
                }
                // 一致しなければ無視
                if ((is_directory == etr.is_directory) &&
                    (name_len == etr.name_len) && (FSi_StrNICmp(path, etr.name, (u32)name_len) == 0))
                {
                    // ディレクトリなら再びその先頭へ移動
                    if (is_directory)
                    {
                        path += name_len;
                        p_dir->arg.seekdir.pos = etr.dir_id;
                        (void)FSi_TranslateCommand(p_dir, FS_COMMAND_SEEKDIR, TRUE);
                        break;
                    }
                    // ファイルであり、ディレクトリを期待していれば失敗
                    else if (is_dir)
                    {
                        return FS_RESULT_NO_ENTRY;
                    }
                    // ファイルを発見
                    else
                    {
                        *p_dir->arg.findpath.result.file = etr.file_id;
                        return FS_RESULT_SUCCESS;
                    }
                }
            }
        }
    }
    // ファイルを発見できなければ失敗
    if (!is_dir)
    {
        return FS_RESULT_NO_ENTRY;
    }
    else
    {
        *p_dir->arg.findpath.result.dir = p_dir->prop.dir.pos;
        return FS_RESULT_SUCCESS;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetPathDefault

  Description:  GETPATH コマンドのデフォルト実装
                (SEEKDIR, READDIR コマンドを使用する)

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_GetPathDefault(FSFile *file)
{
    FSResult            result;
    FSArchive   * const arc = file->arc;
    FSGetPathInfo      *p_info = &file->arg.getpath;
    FSDirEntry          entry;
    u32                 dir_id;
    u32                 file_id;

    enum
    { INVALID_ID = 0x10000 };

    FSFile              tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;

    if (FS_IsDir(file))
    {
        dir_id = file->prop.dir.pos.own_id;
        file_id = INVALID_ID;
    }
    else
    {
        file_id = file->prop.file.own_id;
        {
            // ルートから全ディレクトリを検索
            u32     pos = 0;
            u32     num_dir = 0;
            dir_id = INVALID_ID;
            do
            {
                FSi_SeekDirDirect(tmp, pos);
                // 初回は全ディレクトリ数を取得
                if (!pos)
                {
                    num_dir = tmp->prop.dir.parent;
                }
                tmp->arg.readdir.p_entry = &entry;
                tmp->arg.readdir.skip_string = TRUE;
                while (FSi_TranslateCommand(tmp, FS_COMMAND_READDIR, TRUE) == FS_RESULT_SUCCESS)
                {
                    if (!entry.is_directory && (entry.file_id.file_id == file_id))
                    {
                        dir_id = tmp->prop.dir.pos.own_id;
                        break;
                    }
                }
            }
            while ((dir_id == INVALID_ID) && (++pos < num_dir));
        }
    }

    // 該当するディレクトリが見つからなければFALSE
    if (dir_id == INVALID_ID)
    {
        p_info->total_len = 0;
        result = FS_RESULT_NO_ENTRY;
    }
    else
    {
        // パス長を未計測なら今回一度計測
        {
            u32         id = dir_id;
            // まず"アーカイブ名:/" を加算
            const char *arcname = FS_GetArchiveName(arc);
            u32         len = (u32)STD_GetStringLength(arcname);
            len += 2;
            // 基準ディレクトリへ移動
            FSi_SeekDirDirect(tmp, id);
            // 必要ならファイル名を加算 (すでに取得済み)
            if (file_id != INVALID_ID)
            {
                len += entry.name_len;
            }
            // 順次さかのぼって自身の名前を加算
            if (id != 0)
            {
                do
                {
                    // 親に移動して自身を検索
                    FSi_SeekDirDirect(tmp, tmp->prop.dir.parent);
                    tmp->arg.readdir.p_entry = &entry;
                    tmp->arg.readdir.skip_string = TRUE;
                    while (FSi_TranslateCommand(tmp, FS_COMMAND_READDIR, TRUE) == FS_RESULT_SUCCESS)
                    {
                        if (entry.is_directory && (entry.dir_id.own_id == id))
                        {
                            len += entry.name_len + 1;
                            break;
                        }
                    }
                    id = tmp->prop.dir.pos.own_id;
                }
                while (id != 0);
            }
            // 今回計算済みのデータは保存しておく
            p_info->total_len = (u16)(len + 1);
            p_info->dir_id = (u16)dir_id;
        }

        // パス長の計測が目的なら今回は成功
        if (!p_info->buf)
        {
            result = FS_RESULT_SUCCESS;
        }
        // バッファ長が不足なら今回は失敗
        else if (p_info->buf_len < p_info->total_len)
        {
            result = FS_RESULT_NO_MORE_RESOURCE;
        }
        // 文字列を終端から格納
        else
        {
            u8         *dst = p_info->buf;
            u32         total = p_info->total_len;
            u32         pos = 0;
            u32         id = dir_id;
            // まず"アーカイブ名:/" を加算
            const char *arcname = FS_GetArchiveName(arc);
            u32         len = (u32)STD_GetStringLength(arcname);
            MI_CpuCopy8(arcname, dst + pos, len);
            pos += len;
            MI_CpuCopy8(":/", dst + pos, 2);
            pos += 2;
            // 基準ディレクトリへ移動
            FSi_SeekDirDirect(tmp, id);
            // 必要ならファイル名を加算
            if (file_id != INVALID_ID)
            {
                tmp->arg.readdir.p_entry = &entry;
                tmp->arg.readdir.skip_string = FALSE;
                while (FSi_TranslateCommand(tmp, FS_COMMAND_READDIR, TRUE) == FS_RESULT_SUCCESS)
                {
                    if (!entry.is_directory && (entry.file_id.file_id == file_id))
                    {
                        break;
                    }
                }
                len = entry.name_len + 1;
                MI_CpuCopy8(entry.name, dst + total - len, len);
                total -= len;
            }
            else
            {
                // ディレクトリなら終端だけ付加
                dst[total - 1] = '\0';
                total -= 1;
            }
            // 順次さかのぼって自身の名前を加算
            if (id != 0)
            {
                do
                {
                    // 親に移動して自身を検索
                    FSi_SeekDirDirect(tmp, tmp->prop.dir.parent);
                    tmp->arg.readdir.p_entry = &entry;
                    tmp->arg.readdir.skip_string = FALSE;
                    // "/" を付加
                    dst[total - 1] = '/';
                    total -= 1;
                    // 親の子(以前の自身)を検索
                    while (FSi_TranslateCommand(tmp, FS_COMMAND_READDIR, TRUE) == FS_RESULT_SUCCESS)
                    {
                        if (entry.is_directory && (entry.dir_id.own_id == id))
                        {
                            u32     len = entry.name_len;
                            MI_CpuCopy8(entry.name, dst + total - len, len);
                            total -= len;
                            break;
                        }
                    }
                    id = tmp->prop.dir.pos.own_id;
                }
                while (id != 0);
            }
            SDK_ASSERT(pos == total);
        }
        result = FS_RESULT_SUCCESS;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_OpenFileFastDefault

  Description:  OPENFILEFASTコマンドのデフォルト実装
                (OPENFILEDIRECT コマンドを使用する)

  Arguments:    p_file           コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_OpenFileFastDefault(FSFile *p_file)
{
    FSResult                result;
    FSArchive       * const p_arc = p_file->arc;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(p_arc);
    const FSFileID         *p_id = &p_file->arg.openfilefast.id;
    const u32               index = p_id->file_id;

    {
        // FAT アクセス領域の正当性判定
        u32     pos = (u32)(index * sizeof(FSArchiveFAT));
        if (pos >= context->fat_size)
        {
            result = FS_RESULT_NO_ENTRY;
        }
        else
        {
            // FAT から指定ファイルのイメージ領域を取得
            FSArchiveFAT fat;
            FSiSyncReadParam param;
            param.arc = p_arc;
            param.pos = context->fat + pos;
            result = FSi_ReadTable(&param, &fat, sizeof(fat));
            if (result == FS_RESULT_SUCCESS)
            {
                // 直接オープン
                p_file->arg.openfiledirect.top = fat.top;
                p_file->arg.openfiledirect.bottom = fat.bottom;
                p_file->arg.openfiledirect.index = index;
                result = FSi_TranslateCommand(p_file, FS_COMMAND_OPENFILEDIRECT, TRUE);
            }
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_OpenFileDirectDefault

  Description:  OPENFILEDIRECTコマンドのデフォルト実装

  Arguments:    p_file           コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_OpenFileDirectDefault(FSFile *p_file)
{
    p_file->prop.file.top = p_file->arg.openfiledirect.top;
    p_file->prop.file.pos = p_file->arg.openfiledirect.top;
    p_file->prop.file.bottom = p_file->arg.openfiledirect.bottom;
    p_file->prop.file.own_id = p_file->arg.openfiledirect.index;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadFileDefault

  Description:  READFILEコマンドのデフォルト実装

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadFileDefault(FSFile *file)
{
    FSArchive       * const arc = file->arc;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    const u32               pos = file->prop.file.pos;
    const u32               len = file->arg.readfile.len;
    void            * const dst = file->arg.readfile.dst;
    file->prop.file.pos += len;
    return (*context->read_func) (arc, dst, pos, len);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WriteFileDefault

  Description:  WRITEFILEコマンドのデフォルト実装

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_WriteFileDefault(FSFile *file)
{
    FSArchive       * const arc = file->arc;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    const u32               pos = file->prop.file.pos;
    const u32               len = file->arg.writefile.len;
    const void      * const src = file->arg.writefile.src;
    file->prop.file.pos += len;
    return (*context->write_func) (arc, src, pos, len);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IgnoredCommand

  Description:  デフォルトでは何もしないコマンドの実装

  Arguments:    file             コマンドを処理するハンドル

  Returns:      コマンド結果値
 *---------------------------------------------------------------------------*/
static FSResult FSi_IgnoredCommand(FSFile *file)
{
    (void)file;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_TranslateCommand

  Description:  ユーザプロシージャまたはデフォルト処理を呼び出す。
                同期系コマンドが非同期的応答を返した場合は内部で完了を待つ。
                非同期系コマンドが非同期的応答を返した場合はそのまま返す。

  Arguments:    file             引数を格納したFSFile構造体
                command          コマンド ID
                block            ブロッキングが必要ならTRUE

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
FSResult FSi_TranslateCommand(FSFile *file, FSCommandType command, BOOL block)
{
    FSResult                result = FS_RESULT_PROC_DEFAULT;
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(file->arc);
    // プロシージャが対応しているコマンドなら実行させる
    if ((context->proc_flag & (1 << command)) != 0)
    {
        result = (*context->proc) (file, command);
        switch (result)
        {
        case FS_RESULT_SUCCESS:
        case FS_RESULT_FAILURE:
        case FS_RESULT_UNSUPPORTED:
            break;
        case FS_RESULT_PROC_ASYNC:
            // 非同期処理の扱いはこの後で細かく制御
            break;
        case FS_RESULT_PROC_UNKNOWN:
            // 未知のコマンドなので今回以降はデフォルト処理に切り替える
            result = FS_RESULT_PROC_DEFAULT;
            context->proc_flag &= ~(1 << command);
            break;
        }
    }
    // プロシージャで処理されなかったならデフォルトの処理を呼び出す
    if (result == FS_RESULT_PROC_DEFAULT)
    {
        static FSResult (*const (default_table[])) (FSFile *) =
        {
            FSi_ReadFileDefault,
            FSi_WriteFileDefault,
            FSi_SeekDirDefault,
            FSi_ReadDirDefault,
            FSi_FindPathDefault,
            FSi_GetPathDefault,
            FSi_OpenFileFastDefault,
            FSi_OpenFileDirectDefault,
            FSi_IgnoredCommand,
            FSi_IgnoredCommand,
            FSi_IgnoredCommand,
            FSi_IgnoredCommand,
            FSi_IgnoredCommand,
        };
        if (command < sizeof(default_table) / sizeof(*default_table))
        {
            result = (*default_table[command])(file);
        }
        else
        {
            result = FS_RESULT_UNSUPPORTED;
        }
    }
    // 必要ならこの場でブロッキング
    if (block)
    {
        result = FSi_WaitForArchiveCompletion(file, result);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_ReadFile

  Description:  FS_COMMAND_READFILEコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                buffer           転送先メモリ
                length           転送サイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_ReadFile(FSArchive *arc, FSFile *file, void *buffer, u32 *length)
{
    FSROMFATProperty   *prop = (FSROMFATProperty*)FS_GetFileUserData(file);
    const u32 pos = prop->file.pos;
    const u32 rest = (u32)(prop->file.bottom - pos);
    const u32 org = *length;
    if (*length > rest)
    {
        *length = rest;
    }
    file->arg.readfile.dst = buffer;
    file->arg.readfile.len_org = org;
    file->arg.readfile.len = *length;
    (void)arc;
    return FSi_TranslateCommand(file, FS_COMMAND_READFILE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_WriteFile

  Description:  FS_COMMAND_WRITEFILEコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                buffer           転送元メモリ
                length           転送サイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_WriteFile(FSArchive *arc, FSFile *file, const void *buffer, u32 *length)
{
    FSROMFATProperty   *prop = (FSROMFATProperty*)FS_GetFileUserData(file);
    const u32 pos = prop->file.pos;
    const u32 rest = (u32)(prop->file.bottom - pos);
    const u32 org = *length;
    if (*length > rest)
    {
        *length = rest;
    }
    file->arg.writefile.src = buffer;
    file->arg.writefile.len_org = org;
    file->arg.writefile.len = *length;
    (void)arc;
    return FSi_TranslateCommand(file, FS_COMMAND_WRITEFILE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_SeekDirectory

  Description:  FS_COMMAND_SEEKDIRコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                id               一意なディレクトリID
                position         列挙状況を示す

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_SeekDirectory(FSArchive *arc, FSFile *file, u32 id, u32 position)
{
    FSResult                result;
    FSROMFATCommandInfo    *arg = &file->arg;
    file->arc = arc;
    arg->seekdir.pos.arc = arc;
    arg->seekdir.pos.own_id = (u16)(id >> 0);
    arg->seekdir.pos.index = (u16)(id >> 16);
    arg->seekdir.pos.pos = position;
    result = FSi_TranslateCommand(file, FS_COMMAND_SEEKDIR, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        FS_SetDirectoryHandle(file, arc, &file->prop);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_ReadDirectory

  Description:  FS_COMMAND_READDIRコマンド

  Arguments:    arc              呼び出し元アーカイブ
                file             対象ファイル
                info             情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_ReadDirectory(FSArchive *arc, FSFile *file, FSDirectoryEntryInfo *info)
{
    FSResult                result;
    FSDirEntry              entry[1];
    FSROMFATCommandInfo    *arg = &file->arg;
    arg->readdir.p_entry = entry;
    arg->readdir.skip_string = FALSE;
    result = FSi_TranslateCommand(file, FS_COMMAND_READDIR, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        info->shortname_length = 0;
        info->longname_length = entry->name_len;
        MI_CpuCopy8(entry->name, info->longname, info->longname_length);
        info->longname[info->longname_length] = '\0';
        if (entry->is_directory)
        {
            info->attributes = FS_ATTRIBUTE_IS_DIRECTORY;
            info->id = (u32)((entry->dir_id.own_id << 0) | (entry->dir_id.index << 16));
            info->filesize = 0;
        }
        else
        {
            info->attributes = 0;
            info->id = entry->file_id.file_id;
            info->filesize = 0;
            // 有効なファイルIDならファイルサイズ情報もFATから取得する。
            {
                FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
                u32                     pos = (u32)(info->id * sizeof(FSArchiveFAT));
                if (pos < context->fat_size)
                {
                    FSArchiveFAT        fat;
                    FSiSyncReadParam    param;
                    param.arc = arc;
                    param.pos = context->fat + pos;
                    if (FSi_ReadTable(&param, &fat, sizeof(fat)) == FS_RESULT_SUCCESS)
                    {
                        info->filesize = (u32)(fat.bottom - fat.top);
                        // NTRモードで動作中ならTWL専用領域が読み出せないことをフラグで記録する。
                        if (FSi_IsUnreadableRomOffset(arc, fat.top))
                        {
                            info->attributes |= FS_ATTRIBUTE_IS_OFFLINE;
                        }
                    }
                }
            }
        }
        info->mtime.year = 0;
        info->mtime.month = 0;
        info->mtime.day = 0;
        info->mtime.hour = 0;
        info->mtime.minute = 0;
        info->mtime.second = 0;
    }
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_FindPath

  Description:  FS_COMMAND_FINDPATHコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                base_dir_id         基点ディレクトリID (ルートなら0)
                path                検索パス
                target_id           ID格納先
                target_is_directory ディレクトリかどうかの格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_FindPath(FSArchive *arc, u32 base_dir_id, const char *path, u32 *target_id, BOOL target_is_directory)
{
    FSResult        result;
    union
    {
        FSFileID    file;
        FSDirPos    dir;
    }
    id;
    FSFile  tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;
    tmp->arg.findpath.pos.arc = arc;
    tmp->arg.findpath.pos.own_id = (u16)(base_dir_id >> 0);
    tmp->arg.findpath.pos.index = 0;
    tmp->arg.findpath.pos.pos = 0;
    tmp->arg.findpath.path = path;
    tmp->arg.findpath.find_directory = target_is_directory;
    if (target_is_directory)
    {
        tmp->arg.findpath.result.dir = &id.dir;
    }
    else
    {
        tmp->arg.findpath.result.file = &id.file;
    }
    result = FSi_TranslateCommand(tmp, FS_COMMAND_FINDPATH, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        if (target_is_directory)
        {
            *target_id = id.dir.own_id;
        }
        else
        {
            *target_id = id.file.file_id;
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetPath

  Description:  FS_COMMAND_GETPATHコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                is_directory        fileがファイルならFALSE、ディレクトリならTRUE
                buffer              パス格納先
                length              バッファサイズ

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetPath(FSArchive *arc, FSFile *file, BOOL is_directory, char *buffer, u32 *length)
{
    FSResult                result;
    FSROMFATCommandInfo    *arg = &file->arg;
    arg->getpath.total_len = 0;
    arg->getpath.dir_id = 0;
    arg->getpath.buf = (u8 *)buffer;
    arg->getpath.buf_len = *length;
    result = FSi_TranslateCommand(file, FS_COMMAND_GETPATH, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        *length = arg->getpath.buf_len;
    }
    (void)arc;
    (void)is_directory;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_OpenFileFast

  Description:  FS_COMMAND_OPENFILEFASTコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                id                  ファイルID
                mode                アクセスモード

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_OpenFileFast(FSArchive *arc, FSFile *file, u32 id, u32 mode)
{
    FSResult                result;
    FSROMFATCommandInfo    *arg = &file->arg;
    arg->openfilefast.id.arc = arc;
    arg->openfilefast.id.file_id = id;
    result = FSi_TranslateCommand(file, FS_COMMAND_OPENFILEFAST, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        FS_SetFileHandle(file, arc, &file->prop);
    }
    (void)mode;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_OpenFileDirect

  Description:  FS_COMMAND_OPENFILEDIRECTコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                top                 ファイル先頭オフセット
                bottom              ファイル終端オフセット
                id                  要求するファイルIDおよび結果の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_OpenFileDirect(FSArchive *arc, FSFile *file, u32 top, u32 bottom, u32 *id)
{
    FSResult                result;
    FSROMFATCommandInfo    *arg = &file->arg;
    arg->openfiledirect.top = top;
    arg->openfiledirect.bottom = bottom;
    arg->openfiledirect.index = *id;
    result = FSi_TranslateCommand(file, FS_COMMAND_OPENFILEDIRECT, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        FS_SetFileHandle(file, arc, &file->prop);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_CloseFile

  Description:  FS_COMMAND_CLOSEFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_CloseFile(FSArchive *arc, FSFile *file)
{
    FSResult            result;
    result = FSi_TranslateCommand(file, FS_COMMAND_CLOSEFILE, TRUE);
    FS_DetachHandle(file);
    (void)arc;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_Activate

  Description:  FS_COMMAND_ACTIVATEコマンド

  Arguments:    arc                 呼び出し元アーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ROMFAT_Activate(FSArchive* arc)
{
    FSFile  tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;
    (void)FSi_TranslateCommand(tmp, FS_COMMAND_ACTIVATE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_Idle

  Description:  FS_COMMAND_ACTIVATEコマンド

  Arguments:    arc                 呼び出し元アーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ROMFAT_Idle(FSArchive* arc)
{
    FSFile  tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;
    (void)FSi_TranslateCommand(tmp, FS_COMMAND_IDLE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_Suspend

  Description:  FS_COMMAND_SUSPENDコマンド

  Arguments:    arc                 呼び出し元アーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ROMFAT_Suspend(FSArchive* arc)
{
    FSFile  tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;
    (void)FSi_TranslateCommand(tmp, FS_COMMAND_SUSPEND, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_Resume

  Description:  FS_COMMAND_RESUMEコマンド

  Arguments:    arc                 呼び出し元アーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ROMFAT_Resume(FSArchive* arc)
{
    FSFile  tmp[1];
    FS_InitFile(tmp);
    tmp->arc = arc;
    (void)FSi_TranslateCommand(tmp, FS_COMMAND_RESUME, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_OpenFile

  Description:  FS_COMMAND_OPENFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                baseid              基点ディレクトリ (ルートなら0)
                path                ファイルパス
                mode                アクセスモード

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_OpenFile(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult    result;
    u32         fileid;
    result = FSi_ROMFAT_FindPath(arc, baseid, path, &fileid, FALSE);
    if (result == FS_RESULT_SUCCESS)
    {
        result = FSi_ROMFAT_OpenFileFast(arc, file, fileid, mode);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_SeekFile

  Description:  FS_COMMAND_SEEKFILEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                offset              移動量および移動後の位置
                from                シーク起点

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_SeekFile(FSArchive *arc, FSFile *file, int *offset, FSSeekFileMode from)
{
    FSROMFATProperty   *prop = (FSROMFATProperty*)FS_GetFileUserData(file);
    int pos = *offset;
    switch (from)
    {
    case FS_SEEK_SET:
        pos += prop->file.top;
        break;
    case FS_SEEK_CUR:
    default:
        pos += prop->file.pos;
        break;
    case FS_SEEK_END:
        pos += prop->file.bottom;
        break;
    }
    // 範囲外へのシーク処理は失敗とみなす。
    if ((pos < (int)prop->file.top) || (pos > (int)prop->file.bottom))
    {
        return FS_RESULT_INVALID_PARAMETER;
    }
    else
    {
        prop->file.pos = (u32)pos;
        *offset = pos;
        (void)arc;
        return FS_RESULT_SUCCESS;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetFileLength

  Description:  FS_COMMAND_GETFILELENGTHコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                length              取得したサイズの格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetFileLength(FSArchive *arc, FSFile *file, u32 *length)
{
    FSROMFATProperty   *prop = (FSROMFATProperty*)FS_GetFileUserData(file);
    *length = prop->file.bottom - prop->file.top;
    (void)arc;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetFilePosition

  Description:  FS_COMMAND_GETFILEPOSITIONコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                length              取得した位置の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetFilePosition(FSArchive *arc, FSFile *file, u32 *position)
{
    FSROMFATProperty   *prop = (FSROMFATProperty*)FS_GetFileUserData(file);
    *position = prop->file.pos - prop->file.top;
    (void)arc;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_Unmount

  Description:  FS_COMMAND_UNMOUNTコマンド

  Arguments:    arc                 呼び出し元アーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ROMFAT_Unmount(FSArchive *arc)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    if (FS_IsArchiveTableLoaded(arc))
    {
        OS_TWarning("memory may leak. preloaded-table of archive \"%s\" (0x%08X)",
                    FS_GetArchiveName(arc), context->load_mem);
    }
    context->base = 0;
    context->fat = 0;
    context->fat_size = 0;
    context->fnt = 0;
    context->fnt_size = 0;
    context->fat_bak = 0;
    context->fnt_bak = 0;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetArchiveCaps

  Description:  FS_COMMAND_GETARCHIVECAPSコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                caps                デバイス能力フラグの格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetArchiveCaps(FSArchive *arc, u32 *caps)
{
    (void)arc;
    *caps = 0;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_OpenDirectory

  Description:  FS_COMMAND_OPENDIRECTORYコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル
                baseid              基点ディレクトリID (ルートなら0)
                path                パス
                mode                アクセスモード

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_OpenDirectory(FSArchive *arc, FSFile *file, u32 baseid, const char *path, u32 mode)
{
    FSResult    result = FS_RESULT_ERROR;
    u32         id = 0;
    result = FSi_ROMFAT_FindPath(arc, baseid, path, &id, TRUE);
    if (result == FS_RESULT_SUCCESS)
    {
        result = FSi_ROMFAT_SeekDirectory(arc, file, id, 0);
    }
    (void)mode;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_CloseDirectory

  Description:  FS_COMMAND_CLOSEDIRECTORYコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                file                対象ファイル

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_CloseDirectory(FSArchive *arc, FSFile *file)
{
    FS_DetachHandle(file);
    (void)arc;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetPathInfo

  Description:  FS_COMMAND_GETPATHINFOコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                baseid              基点ディレクトリID (ルートなら0)
                path                パス
                info                ファイル情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetPathInfo(FSArchive *arc, u32 baseid, const char *path, FSPathInfo *info)
{
    FSResult    result = FS_RESULT_ERROR;
    u32         id = 0;
    MI_CpuFill8(info, 0x00, sizeof(*info));
    // かなり効率が悪いので関数を別途切り出したほうが良い
    if (FSi_ROMFAT_FindPath(arc, baseid, path, &id, TRUE) == FS_RESULT_SUCCESS)
    {
        info->attributes = FS_ATTRIBUTE_IS_DIRECTORY;
        info->id = id;
        result = FS_RESULT_SUCCESS;
    }
    else if (FSi_ROMFAT_FindPath(arc, baseid, path, &id, FALSE) == FS_RESULT_SUCCESS)
    {
        info->attributes = 0;
        info->id = id;
        info->filesize = 0;
        // 有効なファイルIDならファイルサイズ情報もFATから取得する。
        {
            FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
            u32                     pos = (u32)(id * sizeof(FSArchiveFAT));
            if (pos < context->fat_size)
            {
                FSArchiveFAT        fat;
                FSiSyncReadParam    param;
                param.arc = arc;
                param.pos = context->fat + pos;
                if (FSi_ReadTable(&param, &fat, sizeof(fat)) == FS_RESULT_SUCCESS)
                {
                    info->filesize = (u32)(fat.bottom - fat.top);
                    // NTRモードで動作中ならTWL専用領域が読み出せないことをフラグで記録する。
                    if (FSi_IsUnreadableRomOffset(arc, fat.top))
                    {
                        info->attributes |= FS_ATTRIBUTE_IS_OFFLINE;
                    }
                }
            }
        }
        result = FS_RESULT_SUCCESS;
    }
    // NitroROMフォーマットのアーカイブはデフォルトで書き込み禁止。
    info->attributes |= FS_ATTRIBUTE_IS_PROTECTED;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ROMFAT_GetPathInfo

  Description:  FS_COMMAND_GETARCHIVERESOURCEコマンド

  Arguments:    arc                 呼び出し元アーカイブ
                resource            リソース情報の格納先

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ROMFAT_GetArchiveResource(FSArchive *arc, FSArchiveResource *resource)
{
    const CARDRomHeader    *header = (const CARDRomHeader *)CARD_GetRomHeader();
    resource->bytesPerSector = 0;
    resource->sectorsPerCluster = 0;
    resource->totalClusters = 0;
    resource->availableClusters = 0;
    resource->totalSize = header->rom_size;
    resource->availableSize = 0;
    resource->maxFileHandles = 0x7FFFFFFF;
    resource->currentFileHandles = 0;
    resource->maxDirectoryHandles = 0x7FFFFFFF;
    resource->currentDirectoryHandles = 0;
    (void)arc;
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadSRLCallback

  Description:  SRLファイル読み込みコールバック

  Arguments:    p_arc            FSArchive構造体
                dst              転送先
                src              転送元
                len              転送サイズ

  Returns:      読み込み処理の結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadSRLCallback(FSArchive *arc, void *buffer, u32 offset, u32 length)
{
    CARDRomHashContext *hash = (CARDRomHashContext*)FS_GetArchiveBase(arc);
    CARD_ReadRomHashImage(hash, buffer, offset, length);
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SRLArchiveProc

  Description:  SRLファイルアーカイブのプロシージャ

  Arguments:    file             コマンド情報を格納したFSFile構造体
                cmd              コマンドタイプ

  Returns:      コマンド処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_SRLArchiveProc(FSFile *file, FSCommandType cmd)
{
    (void)file;
    switch (cmd)
    {
    case FS_COMMAND_WRITEFILE:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSRLFile

  Description:  SRLファイルに含まれるROMファイルシステムをマウント

  Arguments:    arc              マウントに使用するFSArchive構造体。
                                 名前は登録済みでなければならない。
                file             すでにオープンされているマウント対象ファイル。
                                 マウント中はこの構造体を破棄してはならない。
                hash             ハッシュコンテキスト構造体。

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FSi_MountSRLFile(FSArchive *arc, FSFile *file, CARDRomHashContext *hash)
{
    BOOL                    retval = FALSE;
    static CARDRomHeaderTWL header[1] ATTRIBUTE_ALIGN(32);
    if (file &&
        (FS_SeekFileToBegin(file) &&
        (FS_ReadFile(file, header, sizeof(header)) == sizeof(header))))
    {
        // セキュリティのためNANDアプリはハッシュテーブル必須とする。
        // 将来的に「NITRO互換モードだがファイルシステムを使用したい」
        // というROMが登場したらその時に運用方針を検討する。
        if ((((const u8 *)header)[0x1C] & 0x01) != 0)
        {
            FSResult (*proc)(FSFile*, FSCommandType) = FSi_SRLArchiveProc;
            FSResult (*read)(FSArchive*, void*, u32, u32) = FSi_ReadSRLCallback;
            FSResult (*write)(FSArchive*, const void*, u32, u32) = NULL;
            FS_SetArchiveProc(arc, proc, FS_ARCHIVE_PROC_WRITEFILE);
            // arc -> hash -> file の順にアクセスチェーンが形成される。
            if (FS_LoadArchive(arc, (u32)hash,
                               header->ntr.fat.offset, header->ntr.fat.length,
                               header->ntr.fnt.offset, header->ntr.fnt.length,
                               read, write))
            {
                // システムワーク領域のROMヘッダと異なる内容だった場合
                // CARDi_InitRom関数で事前に確保していたバッファは無駄になる。
                extern u8  *CARDiHashBufferAddress;
                extern u32  CARDiHashBufferLength;
                u32         len = CARD_CalcRomHashBufferLength(header);
                if (len > CARDiHashBufferLength)
                {
                    u8     *lo = (u8 *)MATH_ROUNDUP((u32)OS_GetMainArenaLo(), 32);
                    u8     *hi = (u8 *)MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 32);
                    if (&lo[len] > hi)
                    {
                        OS_TPanic("cannot allocate memory for ROM-hash from ARENA");
                    }
                    else
                    {
                        OS_SetMainArenaLo(&lo[len]);
                        CARDiHashBufferAddress = lo;
                        CARDiHashBufferLength = len;
                    }
                }
                CARD_InitRomHashContext(hash, header,
                                        CARDiHashBufferAddress, CARDiHashBufferLength,
                                        FSi_SeekAndReadSRL, NULL, file);
                // 同じバッファを競合して使用されないようポインタを破棄。
                CARDiHashBufferAddress = NULL;
                CARDiHashBufferLength = 0;
                retval = TRUE;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadMemCallback

  Description:  デフォルトのメモリリードコールバック。

  Arguments:    p_arc            操作するアーカイブ
                dst              読み込むメモリの格納先
                pos              読み込み位置
                size             読み込みサイズ

  Returns:      常に FS_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
static FSResult FSi_ReadMemCallback(FSArchive *p_arc, void *dst, u32 pos, u32 size)
{
    MI_CpuCopy8((const void *)FS_GetArchiveOffset(p_arc, pos), dst, size);
    return FS_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WriteMemCallback

  Description:  デフォルトのメモリライトコールバック

  Arguments:    p_arc            操作するアーカイブ
                dst              書き込むメモリの参照先
                pos              書き込み位置
                size             書き込みサイズ

  Returns:      常に FS_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
static FSResult FSi_WriteMemCallback(FSArchive *p_arc, const void *src, u32 pos, u32 size)
{
    MI_CpuCopy8(src, (void *)FS_GetArchiveOffset(p_arc, pos), size);
    return FS_RESULT_SUCCESS;
}

static const FSArchiveInterface FSiArchiveProcInterface =
{
    // 旧仕様と互換性のあるコマンド
    FSi_ROMFAT_ReadFile,
    FSi_ROMFAT_WriteFile,
    FSi_ROMFAT_SeekDirectory,
    FSi_ROMFAT_ReadDirectory,
    FSi_ROMFAT_FindPath,
    FSi_ROMFAT_GetPath,
    FSi_ROMFAT_OpenFileFast,
    FSi_ROMFAT_OpenFileDirect,
    FSi_ROMFAT_CloseFile,
    FSi_ROMFAT_Activate,
    FSi_ROMFAT_Idle,
    FSi_ROMFAT_Suspend,
    FSi_ROMFAT_Resume,
    // 旧仕様と互換性があるがコマンドではなかったもの
    FSi_ROMFAT_OpenFile,
    FSi_ROMFAT_SeekFile,
    FSi_ROMFAT_GetFileLength,
    FSi_ROMFAT_GetFilePosition,
    // 新仕様で拡張されたコマンド (NULLならUNSUPPORTED)
    NULL,               // Mount
    FSi_ROMFAT_Unmount,
    FSi_ROMFAT_GetArchiveCaps,
    NULL,               // CreateFile
    NULL,               // DeleteFile
    NULL,               // RenameFile
    FSi_ROMFAT_GetPathInfo,
    NULL,               // SetFileInfo
    NULL,               // CreateDirectory
    NULL,               // DeleteDirectory
    NULL,               // RenameDirectory
    FSi_ROMFAT_GetArchiveResource,
    NULL,               // 29UL
    NULL,               // FlushFile
    NULL,               // SetFileLength
    FSi_ROMFAT_OpenDirectory,
    FSi_ROMFAT_CloseDirectory,
};

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchive

  Description:  アーカイブをファイルシステムにロードする。
                すでにアーカイブリストに名前が登録されている必要がある。

  Arguments:    arc              ロードするアーカイブ
                base             独自に使用可能な任意の u32 値
                fat              FAT テーブルの先頭オフセット
                fat_size         FAT テーブルのサイズ
                fnt              FNT テーブルの先頭オフセット
                fnt_size         FNT テーブルのサイズ
                read_func        リードアクセスコールバック
                write_func       ライトアクセスコールバック

  Returns:      アーカイブが正しくロードされれば TRUE
 *---------------------------------------------------------------------------*/
BOOL FS_LoadArchive(FSArchive *arc, u32 base,
                    u32 fat, u32 fat_size,
                    u32 fnt, u32 fnt_size,
                    FS_ARCHIVE_READ_FUNC read_func, FS_ARCHIVE_WRITE_FUNC write_func)
{
    BOOL    retval = FALSE;
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);
    SDK_ASSERT(!FS_IsArchiveLoaded(arc));

    // アーカイブ用の情報を初期化
    {
        FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)arc->reserved2;
        context->base = base;
        context->fat_size = fat_size;
        context->fat = fat;
        context->fat_bak = fat;
        context->fnt_size = fnt_size;
        context->fnt = fnt;
        context->fnt_bak = fnt;
        context->read_func = read_func ? read_func : FSi_ReadMemCallback;
        context->write_func = write_func ? write_func : FSi_WriteMemCallback;
        context->load_mem = NULL;
        return FS_MountArchive(arc, context, &FSiArchiveProcInterface, 0);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchive

  Description:  アーカイブをファイルシステムからアンロードする。
                現在処理中のタスクが全て完了するまでブロッキングする。

  Arguments:    arc              アンロードするアーカイブ

  Returns:      アーカイブが正しくアンロードされれば TRUE
 *---------------------------------------------------------------------------*/
BOOL FS_UnloadArchive(FSArchive *arc)
{
    return FS_UnmountArchive(arc);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFileLengthIfProc

  Description:  指定されたファイルがアーカイブプロシージャであればサイズを取得

  Arguments:    file             ファイルハンドル
                length           サイズの格納先

  Returns:      指定されたファイルがアーカイブプロシージャであればそのサイズ
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFileLengthIfProc(FSFile *file, u32 *length)
{
    return (file->arc->vtbl == &FSiArchiveProcInterface) &&
           (FSi_ROMFAT_GetFileLength(file->arc, file, length) == FS_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFilePositionIfProc

  Description:  指定されたファイルがアーカイブプロシージャであれば現在位置を取得

  Arguments:    file             ファイルハンドル
                length           サイズの格納先

  Returns:      指定されたファイルがアーカイブプロシージャであればその現在位置
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFilePositionIfProc(FSFile *file, u32 *length)
{
    return (file->arc->vtbl == &FSiArchiveProcInterface) &&
           (FSi_ROMFAT_GetFilePosition(file->arc, file, length) == FS_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetArchiveProc

  Description:  アーカイブのユーザプロシージャを設定する。
                proc == NULL または flags = 0 なら
                単にユーザプロシージャを無効にする。

  Arguments:    arc              ユーザプロシージャを設定するアーカイブ
                proc             ユーザプロシージャ
                flags            プロシージャへフックするコマンドのビット集合

  Returns:      常に合計のテーブルサイズを返す
 *---------------------------------------------------------------------------*/
void FS_SetArchiveProc(FSArchive *arc, FS_ARCHIVE_PROC_FUNC proc, u32 flags)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)arc->reserved2;
    if (!flags)
    {
        proc = NULL;
    }
    else if (!proc)
    {
        flags = 0;
    }
    context->proc = proc;
    context->proc_flag = flags;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchiveTables

  Description:  アーカイブの FAT + FNT をメモリ上にプリロードする。
                指定サイズ以内の場合のみ読み込みを実行し, 必要サイズを返す。

  Arguments:    p_arc            テーブルをプリロードするアーカイブ
                p_mem            テーブルデータの格納先バッファ
                max_size         p_mem のサイズ

  Returns:      常に, 合計のテーブルサイズを返す
 *---------------------------------------------------------------------------*/
u32 FS_LoadArchiveTables(FSArchive *p_arc, void *p_mem, u32 max_size)
{
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);

    // すでにロード済みのテーブルがあればアンロードする。
    if ((p_mem != NULL) && FS_IsArchiveTableLoaded(p_arc))
    {
        (void)FS_UnloadArchiveTables(p_arc);
    }

    {
        FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(p_arc);
        // プリロードサイズは前後 32 BYTE アライン
        u32     total_size = (u32)((context->fat_size + context->fnt_size + 32 + 31) & ~31);
        if (total_size <= max_size)
        {
            // サイズが充分ならメモリへロード
            u8     *p_cache = (u8 *)(((u32)p_mem + 31) & ~31);
            FSFile  tmp;
            FS_InitFile(&tmp);
            // テーブルはリードできないこともある。
            // その場合は, 元々テーブルにアクセスできないので何もしない。
            if (FS_OpenFileDirect(&tmp, p_arc, context->fat, context->fat + context->fat_size, (u32)~0))
            {
                if (FS_ReadFile(&tmp, p_cache, (s32)context->fat_size) < 0)
                {
                    MI_CpuFill8(p_cache, 0x00, context->fat_size);
                }
                (void)FS_CloseFile(&tmp);
            }
            context->fat = (u32)p_cache;
            p_cache += context->fat_size;
            if (FS_OpenFileDirect(&tmp, p_arc, context->fnt, context->fnt + context->fnt_size, (u32)~0))
            {
                if (FS_ReadFile(&tmp, p_cache, (s32)context->fnt_size) < 0)
                {
                    MI_CpuFill8(p_cache, 0x00, context->fnt_size);
                }
                (void)FS_CloseFile(&tmp);
            }
            context->fnt = (u32)p_cache;
            // 以降はテーブルリード系でプリロードメモリが働く。
            context->load_mem = p_mem;
            p_arc->flag |= FS_ARCHIVE_FLAG_TABLE_LOAD;
        }
        return total_size;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchiveTables

  Description:  アーカイブのプリロードメモリを解放

  Arguments:    arc              プリロードメモリを解放するアーカイブ

  Returns:      プリロードメモリとしてユーザから与えられていたバッファ
 *---------------------------------------------------------------------------*/
void   *FS_UnloadArchiveTables(FSArchive *arc)
{
    void   *retval = NULL;

    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);

    if (FS_IsArchiveLoaded(arc))
    {
        FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
        BOOL    bak_stat = FS_SuspendArchive(arc);
        if (FS_IsArchiveTableLoaded(arc))
        {
            arc->flag &= ~FS_ARCHIVE_FLAG_TABLE_LOAD;
            retval = context->load_mem;
            context->fat = context->fat_bak;
            context->fnt = context->fnt_bak;
            context->load_mem = NULL;
        }
        if (bak_stat)
        {
            (void)FS_ResumeArchive(arc);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveBase

  Description:  ROMFAT型アーカイブのベースオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのベースオフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetArchiveBase(const struct FSArchive *arc)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    return context->base;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFAT

  Description:  ROMFAT型アーカイブのFATオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのFATオフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetArchiveFAT(const struct FSArchive *arc)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    return context->fat;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFNT

  Description:  ROMFAT型アーカイブのFNTオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのFNTオフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetArchiveFNT(const struct FSArchive *arc)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    return context->fnt;
}

/* アーカイブのベースからの指定位置オフセットを取得 */
/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveOffset

  Description:  ROMFAT型アーカイブのベースからの指定位置オフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ベースを加算した指定位置オフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetArchiveOffset(const struct FSArchive *arc, u32 pos)
{
    FSROMFATArchiveContext *context = (FSROMFATArchiveContext*)FS_GetArchiveUserData(arc);
    return context->base + pos;
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveTableLoaded

  Description:  ROMFAT型アーカイブが現在テーブルをプリロード済みか判定

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      テーブルをプリロード済みならTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_IsArchiveTableLoaded(volatile const struct FSArchive *arc)
{
    return ((arc->flag & FS_ARCHIVE_FLAG_TABLE_LOAD) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageTop

  Description:  ROMFAT型アーカイブからオープンしたファイルの
                先頭オフセットを取得

  Arguments:    file        ファイルハンドル

  Returns:      アーカイブ上でのファイル先頭オフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetFileImageTop(const struct FSFile *file)
{
    return file->prop.file.top;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageBottom

  Description:  ROMFAT型アーカイブからオープンしたファイルの
                終端オフセットを取得

  Arguments:    file        ファイルハンドル

  Returns:      アーカイブ上でのファイル終端オフセット
 *---------------------------------------------------------------------------*/
u32 FS_GetFileImageBottom(const struct FSFile *file)
{
    return file->prop.file.bottom;
}


#endif /* FS_IMPLEMENT */
