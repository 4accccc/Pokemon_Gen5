/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_archive.c

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
#include <nitro/std.h>


#include "../include/util.h"
#include "../include/command.h"


#if defined(FS_IMPLEMENT)


/*---------------------------------------------------------------------------*/
/* constants */

// 長いアーカイブ名を内部的にサポートする場合は有効にする
#define FS_SUPPORT_LONG_ARCNAME


/*---------------------------------------------------------------------------*/
/* variables */

// ファイルシステムに登録済みのアーカイブリスト
static FSArchive   *arc_list = NULL;

// カレントディレクトリ
static FSDirPos     current_dir_pos;
static char         current_dir_path[FS_ENTRY_LONGNAME_MAX];

#if defined(FS_SUPPORT_LONG_ARCNAME)
// 長いアーカイブ名を内部的にサポートするための静的バッファ
#define FS_LONG_ARCNAME_LENGTH_MAX  15
#define FS_LONG_ARCNAME_TABLE_MAX   16
static char FSiLongNameTable[FS_LONG_ARCNAME_TABLE_MAX][FS_LONG_ARCNAME_LENGTH_MAX + 1];
#endif


/*---------------------------------------------------------------------------*/
/* functions */

FSArchive* FSi_GetArchiveChain(void)
{
    return arc_list;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsEventCommand

  Description:  コマンドがイベント通知か判定

  Arguments:    command          コマンド種別

  Returns:      コマンドがイベント通知ならTRUE
 *---------------------------------------------------------------------------*/
static BOOL FSi_IsEventCommand(FSCommandType command)
{
    return
        ((command == FS_COMMAND_ACTIVATE) ||
        (command == FS_COMMAND_IDLE) ||
        (command == FS_COMMAND_SUSPEND) ||
        (command == FS_COMMAND_RESUME) ||
        (command == FS_COMMAND_MOUNT) ||
        (command == FS_COMMAND_UNMOUNT) ||
        (command == FS_COMMAND_INVALID));
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EndCommand

  Description:  コマンドを完了し待機スレッドがあれば復帰

  Arguments:    file             完了したコマンド
                ret              コマンドの結果値

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_EndCommand(FSFile *file, FSResult ret)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    // コマンドをリストから削除
    FSArchive *const arc = file->arc;
    if (arc)
    {
        FSFile    **pp = &arc->list;
        for (; *pp; pp = &(*pp)->next)
        {
            if (*pp == file)
            {
                *pp = file->next;
                break;
            }
        }
        file->next = NULL;
    }
    // 結果を格納
    {
        FSCommandType   command = FSi_GetCurrentCommand(file);
        if (!FSi_IsEventCommand(command))
        {
            arc->command = command;
            arc->result = ret;
        }
        file->error = ret;
        file->stat &= ~(FS_FILE_STATUS_CANCEL | FS_FILE_STATUS_BUSY |
                        FS_FILE_STATUS_BLOCKING | FS_FILE_STATUS_OPERATING |
                        FS_FILE_STATUS_ASYNC_DONE | FS_FILE_STATUS_UNICODE_MODE);
    }
    // 待機中のスレッドがいれば通知
    OS_WakeupThread(file->queue);
    (void)OS_RestoreInterrupts(bak_psr);
}


// プロシージャのために一時的に以下の2個を公開。
// プロシージャから再帰的に呼び出している輩は全員同期コマンドなので、
// 直接vtbl経由で呼び出してFSi_WaitForArchiveCompletion()するだけでも良い。

FSResult FSi_WaitForArchiveCompletion(FSFile *file, FSResult result)
{
    if (result == FS_RESULT_PROC_ASYNC)
    {
        FSi_WaitConditionOn(&file->stat, FS_FILE_STATUS_ASYNC_DONE, file->queue);
        file->stat &= ~FS_FILE_STATUS_ASYNC_DONE;
        result = file->error;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_InvokeCommand

  Description:  アーカイブにコマンドを発行する。
                コマンドリストの管理はこの関数呼び出しの前に解決している。

  Arguments:    file             引数を格納したFSFile構造体
                command          コマンドID

  Returns:      コマンドの処理結果
 *---------------------------------------------------------------------------*/
static FSResult FSi_InvokeCommand(FSFile *file, FSCommandType command)
{
    FSResult            result = FS_RESULT_UNSUPPORTED;
    FSArchive   * const arc = file->arc;

    {
        const void   *(*table) = (const void*)arc->vtbl;
        // 範囲外の未定義コマンド
        if ((command < 0) || (command >= FS_COMMAND_MAX))
        {
            OS_TWarning("undefined command (%d)\n", command);
            result = FS_RESULT_UNSUPPORTED;
        }
        // ドライバ側で実装を放棄したコマンドでなければ引数を復元して呼び出し
        else if (table[command] == NULL)
        {
            result = FS_RESULT_UNSUPPORTED;
        }
        else
        {
#define FS_DECLARE_ARGUMENT_(type)                              \
            type *arg = (type*) file->argument;                 \
            (void)arg

#define FS_INVOKE_METHOD_(command, ...)                         \
            do                                                  \
            {                                                   \
                FS_DECLARE_ARGUMENT_(FSArgumentFor ## command); \
                result = arc->vtbl->command(__VA_ARGS__);       \
            }                                                   \
            while(0)
#define FS_NOTIFY_EVENT_(command, ...)                          \
            do                                                  \
            {                                                   \
                FS_DECLARE_ARGUMENT_(FSArgumentFor ## command); \
                (void)arc->vtbl->command(__VA_ARGS__);          \
                return FS_RESULT_SUCCESS;                       \
            }                                                   \
            while(0)
            switch (command)
            {
            case FS_COMMAND_READFILE:
                FS_INVOKE_METHOD_(ReadFile, arc, file, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_WRITEFILE:
                FS_INVOKE_METHOD_(WriteFile, arc, file, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_SEEKDIR:
                FS_INVOKE_METHOD_(SeekDirectory, arc, file, arg->id, arg->position);
                break;
            case FS_COMMAND_READDIR:
                FS_INVOKE_METHOD_(ReadDirectory, arc, file, arg->info);
                break;
            case FS_COMMAND_FINDPATH:
                FS_INVOKE_METHOD_(FindPath, arc, arg->baseid, arg->relpath, &arg->target_id, arg->target_is_directory);
                break;
            case FS_COMMAND_GETPATH:
                FS_INVOKE_METHOD_(GetPath, arc, file, arg->is_directory, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_OPENFILEFAST:
                FS_INVOKE_METHOD_(OpenFileFast, arc, file, arg->id, arg->mode);
                break;
            case FS_COMMAND_OPENFILEDIRECT:
                FS_INVOKE_METHOD_(OpenFileDirect, arc, file, arg->top, arg->bottom, &arg->id);
                break;
            case FS_COMMAND_CLOSEFILE:
                FS_INVOKE_METHOD_(CloseFile, arc, file);
                break;
            case FS_COMMAND_ACTIVATE:
                FS_NOTIFY_EVENT_(Activate, arc);
                break;
            case FS_COMMAND_IDLE:
                FS_NOTIFY_EVENT_(Idle, arc);
                break;
            case FS_COMMAND_SUSPEND:
                FS_NOTIFY_EVENT_(Suspend, arc);
                break;
            case FS_COMMAND_RESUME:
                FS_NOTIFY_EVENT_(Resume, arc);
                break;
            case FS_COMMAND_OPENFILE:
                FS_INVOKE_METHOD_(OpenFile, arc, file, arg->baseid, arg->relpath, arg->mode);
                break;
            case FS_COMMAND_SEEKFILE:
                FS_INVOKE_METHOD_(SeekFile, arc, file, &arg->offset, arg->from);
                break;
            case FS_COMMAND_GETFILELENGTH:
                FS_INVOKE_METHOD_(GetFileLength, arc, file, &arg->length);
                break;
            case FS_COMMAND_GETFILEPOSITION:
                FS_INVOKE_METHOD_(GetFilePosition, arc, file, &arg->position);
                break;
                // ここからは拡張コマンド
            case FS_COMMAND_MOUNT:
                FS_NOTIFY_EVENT_(Mount, arc);
                break;
            case FS_COMMAND_UNMOUNT:
                FS_NOTIFY_EVENT_(Unmount, arc);
                break;
            case FS_COMMAND_GETARCHIVECAPS:
                FS_INVOKE_METHOD_(GetArchiveCaps, arc, &arg->caps);
                break;
            case FS_COMMAND_CREATEFILE:
                FS_INVOKE_METHOD_(CreateFile, arc, arg->baseid, arg->relpath, arg->permit);
                break;
            case FS_COMMAND_DELETEFILE:
                FS_INVOKE_METHOD_(DeleteFile, arc, arg->baseid, arg->relpath);
                break;
            case FS_COMMAND_RENAMEFILE:
                FS_INVOKE_METHOD_(RenameFile, arc, arg->baseid_src, arg->relpath_src, arg->baseid_dst, arg->relpath_dst);
                break;
            case FS_COMMAND_GETPATHINFO:
                FS_INVOKE_METHOD_(GetPathInfo, arc, arg->baseid, arg->relpath, arg->info);
                break;
            case FS_COMMAND_SETPATHINFO:
                FS_INVOKE_METHOD_(SetPathInfo, arc, arg->baseid, arg->relpath, arg->info);
                break;
            case FS_COMMAND_CREATEDIRECTORY:
                FS_INVOKE_METHOD_(CreateDirectory, arc, arg->baseid, arg->relpath, arg->permit);
                break;
            case FS_COMMAND_DELETEDIRECTORY:
                FS_INVOKE_METHOD_(DeleteDirectory, arc, arg->baseid, arg->relpath);
                break;
            case FS_COMMAND_RENAMEDIRECTORY:
                FS_INVOKE_METHOD_(RenameDirectory, arc, arg->baseid_src, arg->relpath_src, arg->baseid_dst, arg->relpath_dst);
                break;
            case FS_COMMAND_GETARCHIVERESOURCE:
                FS_INVOKE_METHOD_(GetArchiveResource, arc, arg->resource);
                break;
            case FS_COMMAND_FLUSHFILE:
                FS_INVOKE_METHOD_(FlushFile, arc, file);
                break;
            case FS_COMMAND_SETFILELENGTH:
                FS_INVOKE_METHOD_(SetFileLength, arc, file, arg->length);
                break;
            case FS_COMMAND_OPENDIRECTORY:
                FS_INVOKE_METHOD_(OpenDirectory, arc, file, arg->baseid, arg->relpath, arg->mode);
                break;
            case FS_COMMAND_CLOSEDIRECTORY:
                FS_INVOKE_METHOD_(CloseDirectory, arc, file);
                break;
            case FS_COMMAND_SETSEEKCACHE:
                FS_INVOKE_METHOD_(SetSeekCache, arc, file, arg->buf, arg->buf_size);
                break;
            default:
                result = FS_RESULT_UNSUPPORTED;
            }
#undef FS_DECLARE_ARGUMENT_
#undef FS_INVOKE_METHOD_
#undef FS_NOTIFY_EVENT_
        }
    }
    // イベント通知でない通常のコマンドなら発行時の条件を満たすように返す。
    if (!FSi_IsEventCommand(command))
    {
        // パス名を間違えてUNSUPPORTEDというケースが多いので、
        // その可能性があれば警告出力をしておく。
        if (result == FS_RESULT_UNSUPPORTED)
        {
            OS_TWarning("archive \"%s:\" cannot support command %d.\n", FS_GetArchiveName(arc), command);
        }
        // 必要ならブロッキング
        if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
        {
            result = FSi_WaitForArchiveCompletion(file, result);
        }
        // 誰もブロッキングしないまま完了した場合はここで解放
        else if (result != FS_RESULT_PROC_ASYNC)
        {
            FSi_EndCommand(file, result);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_NextCommand

  Description:  アーカイブが次に処理すべきコマンドを選択する。
                非同期コマンドが選択されたらそのポインタを返す。
                NULL 以外が返された場合は呼び出し元で処理する必要がある。

  Arguments:    arc              次のコマンドを取得するアーカイブ
                owner            呼び出し元がすでにコマンド実行権を持っていればTRUE

  Returns:      この場で処理を必要とする次のコマンド
 *---------------------------------------------------------------------------*/
static FSFile *FSi_NextCommand(FSArchive *arc, BOOL owner)
{
    FSFile  *next = NULL;
    // まず全てのコマンドのキャンセル要求をチェック
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if ((arc->flag & FS_ARCHIVE_FLAG_CANCELING) != 0)
        {
            FSFile *p = arc->list;
            arc->flag &= ~FS_ARCHIVE_FLAG_CANCELING;
            while (p != NULL)
            {
                FSFile *q = p->next;
                if (FS_IsCanceling(p))
                {
                    FSi_EndCommand(p, FS_RESULT_CANCELED);
                    if (!q)
                    {
                        q = arc->list;
                    }
                }
                p = q;
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    // 次のコマンドを実行可能か判定
    {
        OSIntrMode  bak_psr = OS_DisableInterrupts();
        if (((arc->flag & FS_ARCHIVE_FLAG_SUSPENDING) == 0) &&
            ((arc->flag & FS_ARCHIVE_FLAG_SUSPEND) == 0) && arc->list)
        {
            // アーカイブが停止していたらここで起動
            const BOOL  is_started = owner && ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0);
            if (is_started)
            {
                arc->flag |= FS_ARCHIVE_FLAG_RUNNING;
            }
            (void)OS_RestoreInterrupts(bak_psr);
            if (is_started)
            {
                (void)FSi_InvokeCommand(arc->list, FS_COMMAND_ACTIVATE);
            }
            bak_psr = OS_DisableInterrupts();
            // リスト先頭のコマンドが実行権を獲得
            if (owner || is_started)
            {
                next = arc->list;
                next->stat |= FS_FILE_STATUS_OPERATING;
            }
            // コマンド発行元がブロッキング中なら実行権を委譲
            if (owner && ((next->stat & FS_FILE_STATUS_BLOCKING) != 0))
            {
                OS_WakeupThread(next->queue);
                next = NULL;
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        // サスペンドまたはアイドル状態
        else
        {
            // 実行権を持っていたなら停止してアイドル状態へ
            if (owner)
            {
                if ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) != 0)
                {
                    FSFile  tmp;
                    FS_InitFile(&tmp);
                    tmp.arc = arc;
                    arc->flag &= ~FS_ARCHIVE_FLAG_RUNNING;
                    (void)FSi_InvokeCommand(&tmp, FS_COMMAND_IDLE);
                }
                // サスペンド実行中であればサスペンド関数の呼び出し元へ通知
                if ((arc->flag & FS_ARCHIVE_FLAG_SUSPENDING) != 0)
                {
                    arc->flag &= ~FS_ARCHIVE_FLAG_SUSPENDING;
                    arc->flag |= FS_ARCHIVE_FLAG_SUSPEND;
                    OS_WakeupThread(&arc->queue);
                }
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
    }
    return next;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ExecuteAsyncCommand

  Description:  非同期系コマンドの実行。
                最初の 1 回はユーザスレッドから割り込み許可で呼ばれる。
                アーカイブが同期的に動作する限りここでコマンド処理を繰り返し
                1 回でも非同期処理になれば続きは NotifyAsyncEnd() で行う。

                よってアーカイブ処理が同期 / 非同期で切り替わる場合は
                NotifyAsyncEnd() の呼び出し環境に注意する必要がある。

  Arguments:    file             実行する非同期コマンドを格納した FSFile 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ExecuteAsyncCommand(FSFile *file)
{
    FSArchive   * const arc = file->arc;
    while (file)
    {
        // 呼び出し元が完了待ちならそれを起こして処理を任せる
        {
            OSIntrMode bak_psr = OS_DisableInterrupts();
            file->stat |= FS_FILE_STATUS_OPERATING;
            if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
            {
                OS_WakeupThread(file->queue);
                file = NULL;
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        if (!file)
        {
            break;
        }
        // 処理が非同期なら一時終了
        else if (FSi_InvokeCommand(file, FSi_GetCurrentCommand(file)) == FS_RESULT_PROC_ASYNC)
        {
            break;
        }
        // 結果が同期完了ならここで続きを選択
        else
        {
            file = FSi_NextCommand(arc, TRUE);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ExecuteSyncCommand

  Description:  ブロッキングしたコマンドの実行。

  Arguments:    file             実行するコマンドを格納したFSFile構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ExecuteSyncCommand(FSFile *file)
{
    FSArchive   * const arc = file->arc;
    FSResult            result;
    // 必要なら順番を待ってからコマンドを処理する
    FSi_WaitConditionOn(&file->stat, FS_FILE_STATUS_OPERATING, file->queue);
    result = FSi_InvokeCommand(file, FSi_GetCurrentCommand(file));
    FSi_EndCommand(file, result);
    // ここで処理すべき非同期型のコマンドがあればかわりに実行
    file = FSi_NextCommand(arc, TRUE);
    if (file)
    {
        FSi_ExecuteAsyncCommand(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SendCommand

  Description:  アーカイブへコマンドを発行する。
                起動タイミングの調整とともに, 同期系ならここでブロッキング。

  Arguments:    file             コマンド引数を指定された FSFile 構造体
                command          コマンド ID
                blocking         ブロッキングするならTRUE

  Returns:      コマンドが成功すれば TRUE
 *---------------------------------------------------------------------------*/
BOOL FSi_SendCommand(FSFile *file, FSCommandType command, BOOL blocking)
{
    BOOL                retval = FALSE;
    FSArchive   * const arc = file->arc;
    BOOL                owner = FALSE;

    if (FS_IsBusy(file))
    {
        OS_TPanic("specified file is now still proceccing previous command!");
    }
    if (!arc)
    {
        OS_TWarning("specified handle is not related by any archive\n");
        file->error = FS_RESULT_INVALID_PARAMETER;
        return FALSE;
    }

    // コマンドの初期化
    file->error = FS_RESULT_BUSY;
    file->stat &= ~(FS_FILE_STATUS_CMD_MASK << FS_FILE_STATUS_CMD_SHIFT);
    file->stat |= (command << FS_FILE_STATUS_CMD_SHIFT);
    file->stat |= FS_FILE_STATUS_BUSY;
    file->next = NULL;
    if (blocking)
    {
        file->stat |= FS_FILE_STATUS_BLOCKING;
    }
    // アンロード中なら処理をキャンセルし、そうでなければリスト終端に追加
    {
        OSIntrMode          bak_psr = OS_DisableInterrupts();
        if ((arc->flag & FS_ARCHIVE_FLAG_UNLOADING) != 0)
        {
            FSi_EndCommand(file, FS_RESULT_CANCELED);
        }
        else
        {
            FSFile    **pp;
            for (pp = &arc->list; *pp; pp = &(*pp)->next)
            {
            }
            *pp = file;
        }
        owner = (arc->list == file) && ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0);
        (void)OS_RestoreInterrupts(bak_psr);
    }
    // リストに追加されたならコマンド実行権をチェック
    if (file->error != FS_RESULT_CANCELED)
    {
        // ここで処理すべきコマンドがあればかわりに実行
        FSFile *next = FSi_NextCommand(arc, owner);
        // 必要ならここでブロッキング
        if (blocking)
        {
            FSi_ExecuteSyncCommand(file);
            retval = FS_IsSucceeded(file);
        }
        // リストに追加されたのでいずれ誰かに処理される
        else
        {
            if (next != NULL)
            {
                FSi_ExecuteAsyncCommand(next);
            }
            retval = TRUE;
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EndArchive

  Description:  全てのアーカイブを終了させて解放

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_EndArchive(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    while (arc_list)
    {
        FSArchive *p_arc = arc_list;
        arc_list = arc_list->next;
        (void)FS_UnloadArchive(p_arc);
        FS_ReleaseArchiveName(p_arc);
    }
    (void)OS_RestoreInterrupts(bak_psr);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FindArchive

  Description:  アーカイブ名を検索する。
                一致する名前が無ければ NULL を返す。

  Arguments:    name             検索するアーカイブ名の文字列
                name_len         name の文字列長

  Returns:      検索して見つかったアーカイブのポインタか NULL.
 *---------------------------------------------------------------------------*/
FSArchive *FS_FindArchive(const char *name, int name_len)
{
    OSIntrMode  bak_psr = OS_DisableInterrupts();
    FSArchive  *arc = arc_list;
    for (; arc; arc = arc->next)
    {
        if (FS_IsArchiveLoaded(arc))
        {
            const char *arcname = FS_GetArchiveName(arc);
            if ((STD_CompareNString(arcname, name, name_len) == 0) && (arcname[name_len] == '\0'))
            {
                break;
            }
        }
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResultCode

  Description:  指定したアーカイブの最新のエラーコードを取得。

  Arguments:    path_or_archive  対象のアーカイブを指す
                                 FSArchive構造体またはパス文字列

  Returns:      指定されたアーカイブの最新のエラーコード。
                該当するアーカイブが存在しない場合はFS_RESULT_ERROR
 *---------------------------------------------------------------------------*/
FSResult FS_GetArchiveResultCode(const void *path_or_archive)
{
    OSIntrMode  bak_psr = OS_DisableInterrupts();
    FSArchive  *arc = arc_list;
    while (arc && (arc != (const FSArchive *)path_or_archive))
    {
        arc = arc->next;
    }
    if (!arc)
    {
        arc = FS_NormalizePath((const char *)path_or_archive, NULL, NULL);
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return arc ? arc->result : FS_RESULT_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetCurrentDirectoryPath

  Description:  現在のディレクトリをパス名で取得。

  Arguments:    None.

  Returns:      現在のディレクトリを表す文字列
 *---------------------------------------------------------------------------*/
const char *FS_GetCurrentDirectory(void)
{
    return current_dir_path;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetCurrentDirectory

  Description:  カレントディレクトリを変更する

  Arguments:    path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SetCurrentDirectory(const char *path)
{
    BOOL        retval = FALSE;
    FSArchive  *arc = NULL;
    u32         baseid = 0;
    char        relpath[FS_ENTRY_LONGNAME_MAX];

    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // 正規化されたパス名を取得
    arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc)
    {
        // いずれにせよ設定は成功
        current_dir_pos.arc = arc;
        current_dir_pos.own_id = 0;
        current_dir_pos.index = 0;
        current_dir_pos.pos = 0;
        (void)STD_CopyLString(current_dir_path, relpath, sizeof(current_dir_path));
        // 可能ならディレクトリIDの取得も試行
        if (arc->vtbl->FindPath != NULL)
        {
            FSFile                  dir[1];
            FSArgumentForFindPath   arg[1];
            FS_InitFile(dir);
            dir->arc = arc;
            dir->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->target_is_directory = TRUE;
            if (FSi_SendCommand(dir, FS_COMMAND_FINDPATH, TRUE))
            {
                current_dir_pos.own_id = (u16)arg->target_id;
                (void)STD_CopyLString(current_dir_path, relpath, sizeof(current_dir_path));
            }
        }
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_CopySafeString

  Description:  バッファサイズを確認しつつ文字列をコピー。

  Arguments:    dst           転送先バッファ
                dstlen        転送先サイズ
                src           転送元バッファ
                srclen        転送先サイズ
                stickyFailure 転送元の切り捨てが起こったらFALSE
                
  Returns:      アーカイブポインタまたはNULL
 *---------------------------------------------------------------------------*/
static int FSi_CopySafeString(char *dst, int dstlen, const char *src, int srclen, BOOL *stickyFailure)
{
    int     i;
    int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
    for (i = 0; (i < n) && src[i]; ++i)
    {
        dst[i] = src[i];
    }
    if ((i < srclen) && src[i])
    {
        *stickyFailure = TRUE;
    }
    dst[i] = '\0';
    return i;
}

/*---------------------------------------------------------------------------*
  Name:         FS_NormalizePath

  Description:  指定のパス名にカレントディレクトリを考慮して
                (アーカイブポインタ) + (基準ディレクトリID) + (パス名)
                の形式へ正規化する。

  Arguments:    path        正規化されていないパス文字列
                baseid      基準ディレクトリIDの格納先またはNULL
                relpath     変換後のパス名格納先またはNULL

  Returns:      アーカイブポインタまたはNULL
 *---------------------------------------------------------------------------*/
FSArchive* FS_NormalizePath(const char *path, u32 *baseid, char *relpath)
{
    FSArchive  *arc = NULL;
    int         pathlen = 0;
    int         pathmax = FS_ENTRY_LONGNAME_MAX;
    BOOL        stickyFailure = FALSE;
    // カレントディレクトリが指定されていない場合は再評価
    if (current_dir_pos.arc == NULL)
    {
        current_dir_pos.arc = arc_list;
        current_dir_pos.own_id = 0;
        current_dir_pos.pos = 0;
        current_dir_pos.index = 0;
        current_dir_path[0] = '\0';
    }
    // "/path" の場合は現在のアーカイブのルートを基点とする
    if (FSi_IsSlash((u8)*path))
    {
        arc = current_dir_pos.arc;
        ++path;
        if (baseid)
        {
            *baseid = 0;
        }
    }
    else
    {
        int     i;
        for (i = 0; ; i = FSi_IncrementSjisPosition(path, i))
        {
            u32     c = (u8)path[i];
            // "(path/)*path" の場合は現在位置からの相対パス
            if (!c || FSi_IsSlash(c))
            {
                arc = current_dir_pos.arc;
                if (baseid)
                {
                    *baseid = current_dir_pos.own_id;
                }
                if(relpath)
                {
                    // ディレクトリがパス名だけで保持されているなら連結
                    if ((current_dir_pos.own_id == 0) && (current_dir_path[0] != '\0'))
                    {
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      current_dir_path, FS_ENTRY_LONGNAME_MAX, &stickyFailure);
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      "/", 1, &stickyFailure);
                    }
                }
                break;
            }
            // "arc:/path" の場合はフルパス
            else if (c == ':')
            {
                arc = FS_FindArchive(path, i);
                if (!arc)
                {
                    OS_Warning("archive \"%*s\" is not found.", i, path);
                }
                path += i + 1;
                if (FSi_IsSlash((u8)*path))
                {
                    ++path;
                }
                if (baseid)
                {
                    *baseid = 0;
                }
                break;
            }
        }
    }
    if(relpath)
    {
        // 特殊エントリ名に注意しつつ相対パスを正規化。
        int     curlen = 0;
        while (!stickyFailure)
        {
            char    c = path[curlen];
            if ((c != '\0') && !FSi_IsSlash((u8)c))
            {
                curlen += STD_IsSjisCharacter(&path[curlen]) ? 2 : 1;
            }
            else
            {
                // 空ディレクトリは無視。
                if (curlen == 0)
                {
                }
                // "." (カレントディレクトリ)は無視。
                else if ((curlen == 1) && (path[0] == '.'))
                {
                }
                // ".." (親ディレクトリ)はルートを上限として1階層上がる。
                else if ((curlen == 2) && (path[0] == '.') && (path[1] == '.'))
                {
                    if (pathlen > 0)
                    {
                        --pathlen;
                    }
                    pathlen = FSi_DecrementSjisPositionToSlash(relpath, pathlen) + 1;
                }
                // それ以外はエントリ追加。
                else
                {
                    pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                  path, curlen, &stickyFailure);
                    if (c != '\0')
                    {
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      "/", 1, &stickyFailure);
                    }
                }
                if (c == '\0')
                {
                    break;
                }
                path += curlen + 1;
                curlen = 0;
            }
        }
        relpath[pathlen] = '\0';
        pathlen = FSi_TrimSjisTrailingSlash(relpath);
    }
    return stickyFailure ? NULL : arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_InitArchive

  Description:  アーカイブ構造体を初期化

  Arguments:    p_arc            初期化するアーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_InitArchive(FSArchive *p_arc)
{
    SDK_NULL_ASSERT(p_arc);
    MI_CpuClear8(p_arc, sizeof(FSArchive));
    OS_InitThreadQueue(&p_arc->queue);
}

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterArchiveName

  Description:  アーカイブ名をファイルシステムへ登録し, 関連付ける。
                アーカイブ自体はまだファイルシステムにロードされない。
                アーカイブ名 "rom" はファイルシステムに予約済み。

  Arguments:    p_arc            名前を関連付けるアーカイブ
                name             登録する名前の文字列
                name_len         name の文字列長

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL FS_RegisterArchiveName(FSArchive *p_arc, const char *name, u32 name_len)
{
    BOOL    retval = FALSE;

    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);
    SDK_NULL_ASSERT(name);

    {
        OSIntrMode bak_intr = OS_DisableInterrupts();
        if (!FS_FindArchive(name, (s32)name_len))
        {
            // リストの終端に追加
            FSArchive **pp;
            for (pp = &arc_list; *pp; pp = &(*pp)->next)
            {
            }
            *pp = p_arc;
            // 現在の仕様ではアーカイブは最大3文字
            if (name_len <= FS_ARCHIVE_NAME_LEN_MAX)
            {
                p_arc->name.pack = 0;
                (void)STD_CopyLString(p_arc->name.ptr, name, (int)(name_len + 1));
            }
            else
            {
#if defined(FS_SUPPORT_LONG_ARCNAME)
            // アーカイブ名の文字数上限を拡張する機能。
            // FSArchive構造体のサイズを変更しないで対応しようとすると
            // このように外部のメモリを割り当てる必要がある。
                if (name_len <= FS_LONG_ARCNAME_LENGTH_MAX)
                {
                    int i;
                    for (i = 0; ; ++i)
                    {
                        if (i >= FS_LONG_ARCNAME_TABLE_MAX)
                        {
                            OS_TPanic("failed to allocate memory for long archive-name(%.*s)!", name_len, name);
                        }
                        else if (FSiLongNameTable[i][0] == '\0')
                        {
                            (void)STD_CopyLString(FSiLongNameTable[i], name, (int)(name_len + 1));
                            p_arc->name.pack = (u32)FSiLongNameTable[i];
                            break;
                        }
                    }
                }
#endif
                // 長すぎるアーカイブ名は登録できない。
                else
                {
                    OS_TPanic("too long archive-name(%.*s)!", name_len, name);
                }
            }
            p_arc->flag |= FS_ARCHIVE_FLAG_REGISTER;
            retval = TRUE;
        }
        (void)OS_RestoreInterrupts(bak_intr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReleaseArchiveName

  Description:  登録済みのアーカイブ名を解放する。
                ファイルシステムからアンロードされている必要がある。

  Arguments:    p_arc            名前を解放するアーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ReleaseArchiveName(FSArchive *p_arc)
{
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);

	if(p_arc == arc_list)
    {
        OS_TPanic("[file-system] cannot modify \"rom\" archive.\n");
    }

    if (p_arc->name.pack)
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        // リストから切断
        FSArchive **pp;
        for (pp = &arc_list; *pp; pp = &(*pp)->next)
        {
            if(*pp == p_arc)
            {
                *pp = (*pp)->next;
                break;
            }
        }
#if defined(FS_SUPPORT_LONG_ARCNAME)
        // 長いアーカイブ名であればバッファをシステムへ解放
        if (p_arc->name.ptr[3] != '\0')
        {
            ((char *)p_arc->name.pack)[0] = '\0';
        }
#endif
        p_arc->name.pack = 0;
        p_arc->next = NULL;
        p_arc->flag &= ~FS_ARCHIVE_FLAG_REGISTER;
        // カレントアーカイブなら解除
        if (current_dir_pos.arc == p_arc)
        {
            current_dir_pos.arc = NULL;
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveName

  Description:  アーカイブ名を取得

  Arguments:    p_arc            名前を取得するアーカイブ

  Returns:      ファイルシステムに登録されたアーカイブ名
 *---------------------------------------------------------------------------*/
const char *FS_GetArchiveName(const FSArchive *arc)
{
#if defined(FS_SUPPORT_LONG_ARCNAME)
    return (arc->name.ptr[3] != '\0') ? (const char *)arc->name.pack : arc->name.ptr;
#else
    return arc->name.ptr;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         FS_MountArchive

  Description:  アーカイブをマウントする。

  Arguments:    arc              マウントするアーカイブ
                userdata         アーカイブに関連付けるユーザ定義変数
                vtbl             コマンドインタフェース
                reserved         将来のための予約 (常に0を指定)


  Returns:      マウント処理に成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_MountArchive(FSArchive *arc, void *userdata,
                     const FSArchiveInterface *vtbl, u32 reserved)
{
    (void)reserved;
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);
    SDK_ASSERT(!FS_IsArchiveLoaded(arc));
    // 新仕様でアーカイブを初期化
    arc->userdata = userdata;
    arc->vtbl = vtbl;
    // マウント通知イベント発行
    {
        FSFile  tmp[1];
        FS_InitFile(tmp);
        tmp->arc = arc;
        (void)FSi_InvokeCommand(tmp, FS_COMMAND_MOUNT);
    }
    arc->flag |= FS_ARCHIVE_FLAG_LOADED;
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnmountArchive

  Description:  アーカイブをアンマウントする。

  Arguments:    arc              アンマウントするアーカイブ

  Returns:      アンマウント処理に成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_UnmountArchive(FSArchive *arc)
{
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        // ロードされていないなら無視
        if (FS_IsArchiveLoaded(arc))
        {
            // 全ての待機中コマンドをキャンセル
            {
                BOOL    bak_state = FS_SuspendArchive(arc);
                FSFile *file = arc->list;
                arc->flag |= FS_ARCHIVE_FLAG_UNLOADING;
                while (file)
                {
                    FSFile *next = file->next;
                    FSi_EndCommand(file, FS_RESULT_CANCELED);
                    file = next;
                }
                arc->list = NULL;
                if (bak_state)
                {
                    (void)FS_ResumeArchive(arc);
                }
            }
            // アンマウント通知イベント発行
            {
                FSFile  tmp[1];
                FS_InitFile(tmp);
                tmp->arc = arc;
                (void)FSi_InvokeCommand(tmp, FS_COMMAND_UNMOUNT);
            }
            arc->flag &= ~(FS_ARCHIVE_FLAG_CANCELING |
                           FS_ARCHIVE_FLAG_LOADED | FS_ARCHIVE_FLAG_UNLOADING);
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    return TRUE;

}

/*---------------------------------------------------------------------------*
  Name:         FS_SuspendArchive

  Description:  アーカイブの処理機構自体を停止する。
                現在実行中の処理があればその完了を待機。

  Arguments:    p_arc            停止するアーカイブ

  Returns:      呼び出し以前にサスペンド状態でなければ TRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SuspendArchive(FSArchive *p_arc)
{
    BOOL    retval = FALSE;

    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        retval = !FS_IsArchiveSuspended(p_arc);
        if (retval)
        {
            if ((p_arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0)
            {
                p_arc->flag |= FS_ARCHIVE_FLAG_SUSPEND;
            }
            else
            {
                p_arc->flag |= FS_ARCHIVE_FLAG_SUSPENDING;
                FSi_WaitConditionOff(&p_arc->flag, FS_ARCHIVE_FLAG_SUSPENDING, &p_arc->queue);
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResumeArchive

  Description:  停止していたアーカイブの処理を再開

  Arguments:    arc              再開するアーカイブ

  Returns:      呼び出し以前にサスペンド状態でなければ TRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ResumeArchive(FSArchive *arc)
{
    BOOL    retval;
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);
    {
        OSIntrMode bak_irq = OS_DisableInterrupts();
        retval = !FS_IsArchiveSuspended(arc);
        if (!retval)
        {
            arc->flag &= ~FS_ARCHIVE_FLAG_SUSPEND;
        }
        (void)OS_RestoreInterrupts(bak_irq);
    }
    {
        FSFile *file = NULL;
        file = FSi_NextCommand(arc, TRUE);
        if (file)
        {
            FSi_ExecuteAsyncCommand(file);
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         FS_NotifyArchiveAsyncEnd

  Description:  非同期で実行していたアーカイブ処理の完了を通知するために
                アーカイブ実装側から呼び出す。

  Arguments:    arc              完了を通知するアーカイブ
                ret              処理結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_NotifyArchiveAsyncEnd(FSArchive *arc, FSResult ret)
{
    FSFile     *file = arc->list;
    if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        file->stat |= FS_FILE_STATUS_ASYNC_DONE;
        file->error = ret;
        OS_WakeupThread(file->queue);
        (void)OS_RestoreInterrupts(bak_psr);
    }
    else
    {
        FSi_EndCommand(file, ret);
        file = FSi_NextCommand(arc, TRUE);
        if (file)
        {
            FSi_ExecuteAsyncCommand(file);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         FS_WaitAsync

  Description:  wait for end of asynchronous function and return result.

  Arguments:    file to wait

  Returns:      if succeeded, True.
 *---------------------------------------------------------------------------*/
BOOL FS_WaitAsync(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    {
        BOOL    is_owner = FALSE;
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (FS_IsBusy(file))
        {
            // 未処理の非同期モードならここで処理を引き受ける
            is_owner = !(file->stat & (FS_FILE_STATUS_BLOCKING | FS_FILE_STATUS_OPERATING));
            if (is_owner)
            {
                file->stat |= FS_FILE_STATUS_BLOCKING;
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
        if (is_owner)
        {
            FSi_ExecuteSyncCommand(file);
        }
        else
        {
            FSi_WaitConditionOff(&file->stat, FS_FILE_STATUS_BUSY, file->queue);
        }
    }

    return FS_IsSucceeded(file);
}


#endif /* FS_IMPLEMENT */
