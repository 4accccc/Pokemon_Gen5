/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     archive.h

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


#ifndef NITRO_FS_ARCHIVE_H_
#define NITRO_FS_ARCHIVE_H_


#include <nitro/fs/types.h>
#include <nitro/fs/romfat.h>
#include <nitro/os/common/thread.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

// アーカイブコマンドインタフェース
typedef struct FSArchiveInterface
{
    // 旧仕様と互換性のあるコマンド
    FSResult (*ReadFile)(struct FSArchive*, struct FSFile*, void *buffer, u32 *length);
    FSResult (*WriteFile)(struct FSArchive*, struct FSFile*, const void *buffer, u32 *length);
    FSResult (*SeekDirectory)(struct FSArchive*, struct FSFile*, u32 id, u32 position);
    FSResult (*ReadDirectory)(struct FSArchive*, struct FSFile*, FSDirectoryEntryInfo *info);
    FSResult (*FindPath)(struct FSArchive*, u32 base_dir_id, const char *path, u32 *target_id, BOOL target_is_directory);
    FSResult (*GetPath)(struct FSArchive*, struct FSFile*, BOOL is_directory, char *buffer, u32 *length);
    FSResult (*OpenFileFast)(struct FSArchive*, struct FSFile*, u32 id, u32 mode);
    FSResult (*OpenFileDirect)(struct FSArchive*, struct FSFile*, u32 top, u32 bottom, u32 *id);
    FSResult (*CloseFile)(struct FSArchive*, struct FSFile*);
    void (*Activate)(struct FSArchive*);
    void (*Idle)(struct FSArchive*);
    void (*Suspend)(struct FSArchive*);
    void (*Resume)(struct FSArchive*);
    // 旧仕様と互換性があるがコマンドではなかったもの
    FSResult (*OpenFile)(struct FSArchive*, struct FSFile*, u32 base_dir_id, const char *path, u32 mode);
    FSResult (*SeekFile)(struct FSArchive*, struct FSFile*, int *offset, FSSeekFileMode from);
    FSResult (*GetFileLength)(struct FSArchive*, struct FSFile*, u32 *length);
    FSResult (*GetFilePosition)(struct FSArchive*, struct FSFile*, u32 *position);
    // 新仕様で拡張されたコマンド
    void (*Mount)(struct FSArchive*);
    void (*Unmount)(struct FSArchive*);
    FSResult (*GetArchiveCaps)(struct FSArchive*, u32 *caps);
    FSResult (*CreateFile)(struct FSArchive*, u32 baseid, const char *relpath, u32 permit);
    FSResult (*DeleteFile)(struct FSArchive*, u32 baseid, const char *relpath);
    FSResult (*RenameFile)(struct FSArchive*, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst);
    FSResult (*GetPathInfo)(struct FSArchive*, u32 baseid, const char *relpath, FSPathInfo *info);
    FSResult (*SetPathInfo)(struct FSArchive*, u32 baseid, const char *relpath, FSPathInfo *info);
    FSResult (*CreateDirectory)(struct FSArchive*, u32 baseid, const char *relpath, u32 permit);
    FSResult (*DeleteDirectory)(struct FSArchive*, u32 baseid, const char *relpath);
    FSResult (*RenameDirectory)(struct FSArchive*, u32 baseid, const char *relpath_src, u32 baseid_dst, const char *relpath_dst);
    FSResult (*GetArchiveResource)(struct FSArchive*, FSArchiveResource *resource);
    void   *unused_29;
    FSResult (*FlushFile)(struct FSArchive*, struct FSFile*);
    FSResult (*SetFileLength)(struct FSArchive*, struct FSFile*, u32 length);
    FSResult (*OpenDirectory)(struct FSArchive*, struct FSFile*, u32 base_dir_id, const char *path, u32 mode);
    FSResult (*CloseDirectory)(struct FSArchive*, struct FSFile*);
    FSResult (*SetSeekCache)(struct FSArchive*, struct FSFile*, void* buf, u32 buf_size);
    // 将来拡張分の予約。
    u8      reserved[116];
}
FSArchiveInterface;

SDK_COMPILER_ASSERT(sizeof(FSArchiveInterface) == 256);


typedef struct FSArchive
{
// private:

    // アーカイブを識別する一意の名前。
    // 英数字1～3文字で大小文字を区別しない。
    union
    {
        char    ptr[FS_ARCHIVE_NAME_LEN_MAX + 1];
        u32     pack;
    }
    name;
    struct FSArchive   *next;       // アーカイブ登録リスト
    struct FSFile      *list;       // 処理待ちのコマンドリスト
    OSThreadQueue       queue;      // イベント待機用の汎用キュー
    u32                 flag;       // 内部状態フラグ (FS_ARCHIVE_FLAG_*)
    FSCommandType       command;    // 最新のコマンド
    FSResult            result;     // 最新の処理結果
    void               *userdata;   // ユーザ定義ポインタ
    const FSArchiveInterface *vtbl; // コマンドインタフェース

    union
    {
        // ユーザ定義のアーカイブで自由に利用できる情報
        u8      reserved2[52];
        // ROMFAT型アーカイブで使用する情報 (互換性のために保持)
        struct  FS_ROMFAT_CONTEXT_DEFINITION();
    };
}
FSArchive;

// NITRO-SDKとの互換性を重視して当面はサイズ変更厳禁
SDK_COMPILER_ASSERT(sizeof(FSArchive) == 92);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FS_FindArchive

  Description:  アーカイブ名を検索する。
                一致する名前が無ければ NULL を返す。

  Arguments:    name             検索するアーカイブ名の文字列
                name_len         name の文字列長

  Returns:      検索して見つかったアーカイブのポインタか NULL
 *---------------------------------------------------------------------------*/
FSArchive *FS_FindArchive(const char *name, int name_len);

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
FSArchive* FS_NormalizePath(const char *path, u32 *baseid, char *relpath);

/*---------------------------------------------------------------------------*
  Name:         FS_GetCurrentDirectory

  Description:  現在のディレクトリをパス名で取得。

  Arguments:    None.

  Returns:      現在のディレクトリを表す文字列
 *---------------------------------------------------------------------------*/
const char *FS_GetCurrentDirectory(void);

/*---------------------------------------------------------------------------*
  Name:         FS_GetLastArchiveCommand

  Description:  アーカイブが処理した最新のコマンドを取得。

  Arguments:    arc              FSArchive構造体

  Returns:      アーカイブが処理した最新のコマンドの結果値
 *---------------------------------------------------------------------------*/
SDK_INLINE FSCommandType FS_GetLastArchiveCommand(const FSArchive *arc)
{
    return arc->command;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResultCode

  Description:  指定したアーカイブの最新のエラーコードを取得。

  Arguments:    path_or_archive  対象のアーカイブを指す
                                 FSArchive構造体またはパス文字列

  Returns:      指定されたアーカイブの最新のエラーコード。
                該当するアーカイブが存在しない場合はFS_RESULT_ERROR
 *---------------------------------------------------------------------------*/
FSResult FS_GetArchiveResultCode(const void *path_or_archive);

/*---------------------------------------------------------------------------*
  Name:         FSi_EndArchive

  Description:  全てのアーカイブを終了させて解放

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_EndArchive(void);

/*---------------------------------------------------------------------------*
  Name:         FS_InitArchive

  Description:  アーカイブ構造体を初期化

  Arguments:    arc              初期化するアーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_InitArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveName

  Description:  アーカイブ名を取得

  Arguments:    arc              名前を取得するアーカイブ

  Returns:      ファイルシステムに登録されたアーカイブ名
 *---------------------------------------------------------------------------*/
const char *FS_GetArchiveName(const FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveLoaded

  Description:  アーカイブが現在ファイルシステムにロード済みか判定

  Arguments:    arc              判定するアーカイブ

  Returns:      ファイルシステムにロード済みなら TRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsArchiveLoaded(volatile const FSArchive *arc)
{
    return ((arc->flag & FS_ARCHIVE_FLAG_LOADED) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveSuspended

  Description:  アーカイブが現在サスペンド中か判定

  Arguments:    arc              判定するアーカイブ

  Returns:      サスペンド中なら TRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsArchiveSuspended(volatile const FSArchive *arc)
{
    return ((arc->flag & FS_ARCHIVE_FLAG_SUSPEND) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveUserData

  Description:  アーカイブに関連付けられたユーザ定義ポインタを取得

  Arguments:    arc              FSArchive構造体

  Returns:      ユーザ定義ポインタ
 *---------------------------------------------------------------------------*/
SDK_INLINE void* FS_GetArchiveUserData(const FSArchive *arc)
{
    return arc->userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterArchiveName

  Description:  アーカイブ名をファイルシステムへ登録し関連付ける。
                アーカイブ自体はまだファイルシステムにロードされない。
                アーカイブ名 "rom" はファイルシステムに予約済み。

  Arguments:    arc              名前を関連付けるアーカイブ
                name             登録する名前の文字列
                name_len         name の文字列長

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL    FS_RegisterArchiveName(FSArchive *arc, const char *name, u32 name_len);

/*---------------------------------------------------------------------------*
  Name:         FS_ReleaseArchiveName

  Description:  登録済みのアーカイブ名を解放する

  Arguments:    arc              名前を解放するアーカイブ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_ReleaseArchiveName(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_MountArchive

  Description:  アーカイブをマウントする

  Arguments:    arc              マウントするアーカイブ
                userdata         アーカイブに関連付けるユーザ定義変数
                vtbl             コマンドインタフェース
                reserved         将来のための予約 (常に0を指定)


  Returns:      マウント処理に成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_MountArchive(FSArchive *arc, void *userdata,
                        const FSArchiveInterface *vtbl, u32 reserved);

/*---------------------------------------------------------------------------*
  Name:         FS_UnmountArchive

  Description:  アーカイブをアンマウントする

  Arguments:    arc              アンマウントするアーカイブ

  Returns:      アンマウント処理に成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_UnmountArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_SuspendArchive

  Description:  アーカイブの処理機構自体を停止する。
                現在実行中の処理があれば, その完了を待機。

  Arguments:    arc              停止するアーカイブ

  Returns:      呼び出し以前にサスペンド状態でなければ TRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_SuspendArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_ResumeArchive

  Description:  停止していたアーカイブの処理を再開

  Arguments:    arc              再開するアーカイブ

  Returns:      呼び出し以前にサスペンド状態でなければ TRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ResumeArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_NotifyArchiveAsyncEnd

  Description:  非同期で実行していたアーカイブ処理の完了を通知するために
                アーカイブ実装側から呼び出す。

  Arguments:    arc              完了を通知するアーカイブ
                ret              処理結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_NotifyArchiveAsyncEnd(FSArchive *arc, FSResult ret);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_ARCHIVE_H_ */
