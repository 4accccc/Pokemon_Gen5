/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - include
  File:     file.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-05#$
  $Rev: 9524 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#if	!defined(NITRO_FS_FILE_H_)
#define NITRO_FS_FILE_H_


#include <nitro/fs/archive.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

// ファイル構造体
typedef struct FSFile
{
// private:
    struct FSFile              *next;
    void                       *userdata;
    struct FSArchive           *arc;
    u32                         stat;
    void                       *argument;
    FSResult                    error;
    OSThreadQueue               queue[1];

    union
    {
        u8                      reserved1[16];
        FSROMFATProperty        prop;
    };
    union
    {
        u8                      reserved2[24];
        FSROMFATCommandInfo     arg;
    };
}
FSFile;


SDK_COMPILER_ASSERT(sizeof(FSFile) == 72);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FS_InitFile

  Description:  FSFile構造体を初期化

  Arguments:    file        FSFile構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_InitFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetAttachedArchive

  Description:  ファイルやディレクトリに関連付けられたアーカイブを取得

  Arguments:    file        ファイルハンドルまたはディレクトリハンドル

  Returns:      ハンドルを保持していればそのアーカイブ、そうでなければNULL
 *---------------------------------------------------------------------------*/
SDK_INLINE FSArchive *FS_GetAttachedArchive(const FSFile *file)
{
    return file->arc;
}
/*---------------------------------------------------------------------------*
  Name:         FS_IsBusy

  Description:  ファイルがコマンド処理中か判定

  Arguments:    file        コマンドを発行したFSFile構造体

  Returns:      コマンド処理中ならTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsBusy(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_BUSY) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsCanceling

  Description:  ファイルがコマンドキャンセル中か判定

  Arguments:    file        コマンドを発行したFSFile構造体

  Returns:      コマンドキャンセル中ならTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsCanceling(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_CANCEL) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsSucceeded

  Description:  最新のコマンドが完了して成功したか判定

  Arguments:    file        コマンドを発行したFSFile構造体

  Returns:      コマンド完了かつ成功であればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsSucceeded(volatile const FSFile *file)
{
    return (file->error == FS_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsFile

  Description:  オープンされたファイルハンドルか判定

  Arguments:    file        FSFile構造体

  Returns:      ファイルハンドルを保持していればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsFile(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_IS_FILE) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsDir

  Description:  オープンされたディレクトリハンドルか判定

  Arguments:    file        FSFile構造体

  Returns:      ディレクトリハンドルを保持していればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsDir(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_IS_DIR) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetResultCode

  Description:  最新のコマンドの結果値を取得

  Arguments:    file        FSFile構造体

  Returns:      最新のコマンドの結果値
 *---------------------------------------------------------------------------*/
SDK_INLINE FSResult FS_GetResultCode(volatile const FSFile *file)
{
    return file->error;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WaitAsync

  Description:  非同期コマンドの完了までブロッキングする

  Arguments:    file        ファイルハンドル

  Returns:      コマンドが成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_WaitAsync(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_CancelFile

  Description:  非同期コマンドのキャンセルを要求する

  Arguments:    file        ファイルハンドル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_CancelFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileHandle

  Description:  ファイル構造体をファイルハンドルとして初期化。
                この関数はOpenFileコマンドに対応するために
                アーカイブ実装側から呼び出す。

  Arguments:    file             FSFile構造体
                arc              関連付けるアーカイブ
                userdata         ファイルハンドル情報を保持する
                                 任意のアーカイブ定義データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_SetFileHandle(FSFile *file, FSArchive *arc, void *userdata)
{
    file->stat |= FS_FILE_STATUS_IS_FILE;
    file->stat &= ~FS_FILE_STATUS_IS_DIR;
    file->arc = arc;
    file->userdata = userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetDirectoryHandle

  Description:  ファイル構造体をディレクトリハンドルとして初期化。
                この関数はOpenDirectoryコマンドに対応するために
                アーカイブ実装側から呼び出す。

  Arguments:    file             FSFile構造体
                arc              関連付けるアーカイブ
                userdata         ディレクトリハンドル情報を保持する
                                 任意のアーカイブ定義データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_SetDirectoryHandle(FSFile *file, FSArchive *arc, void *userdata)
{
    file->stat |= FS_FILE_STATUS_IS_DIR;
    file->stat &= ~FS_FILE_STATUS_IS_FILE;
    file->arc = arc;
    file->userdata = userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DetachHandle

  Description:  ファイル構造体を無効なハンドルとして初期化。
                この関数はCloseFileおよびCloseDirectoryコマンドに対応するために
                アーカイブ実装側から呼び出す。

  Arguments:    file             FSFile構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_DetachHandle(FSFile *file)
{
    file->userdata = NULL;
    file->stat &= ~(FS_FILE_STATUS_IS_FILE | FS_FILE_STATUS_IS_DIR);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileUserData

  Description:  ファイル構造体に関連付けられたアーカイブ定義データを取得。
                アーカイブ実装側から呼び出す。

  Arguments:    file             FSFile構造体

  Returns:      ファイル構造体に関連付けられたアーカイブ定義データ
 *---------------------------------------------------------------------------*/
SDK_INLINE void* FS_GetFileUserData(const FSFile *file)
{
    return file->userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathName

  Description:  指定したファイルまたはディレクトリのフルパス名を取得

  Arguments:    file        ファイルまたはディレクトリハンドル
                buffer      パス名の格納先
                length      バッファサイズ

  Returns:      終端の'\0'も含めて正しく取得できればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_GetPathName(FSFile *file, char *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathLength

  Description:  指定したファイルまたはディレクトリのフルパス名の長さを取得

  Arguments:    file        ファイルまたはディレクトリハンドル

  Returns:      成功すれば終端の'\0'も含めたパス名の長さ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32     FS_GetPathLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFile

  Description:  ファイルを生成する

  Arguments:    path        パス名
                mode        アクセスモード

  Returns:      ファイルが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFile(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFile

  Description:  ファイルを削除する

  Arguments:    path        パス名

  Returns:      ファイルが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_DeleteFile(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFile

  Description:  ファイル名を変更する

  Arguments:    src         変換元のファイル名
                dst         変換先のファイル名

  Returns:      ファイル名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_RenameFile(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathInfo

  Description:  ファイルまたはディレクトリの情報を取得する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
BOOL    FS_GetPathInfo(const char *path, FSPathInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_SetPathInfo

  Description:  ファイルまたはディレクトリの情報を設定する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
BOOL    FS_SetPathInfo(const char *path, const FSPathInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectory

  Description:  ディレクトリを生成する

  Arguments:    path        パス名
                mode        アクセスモード

  Returns:      ディレクトリが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateDirectory(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectory

  Description:  ディレクトリを削除する

  Arguments:    path        パス名

  Returns:      ディレクトリが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_DeleteDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectory

  Description:  ディレクトリ名を変更する

  Arguments:    src         変換元のディレクトリ名
                dst         変換先のディレクトリ名

  Returns:      ディレクトリ名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_RenameDirectory(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileAuto

  Description:  必要な中間ディレクトリも含めてファイルを生成する

  Arguments:    path        パス名
                permit      アクセスモード

  Returns:      ファイルが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileAuto(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFileAuto

  Description:  必要な中間ディレクトリも含めてファイルを削除する

  Arguments:    path        パス名

  Returns:      ファイルが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFileAuto(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFileAuto

  Description:  必要な中間ディレクトリも含めてファイル名を変更する

  Arguments:    src         変換元のファイル名
                dst         変換先のファイル名

  Returns:      ファイル名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFileAuto(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectoryAuto

  Description:  必要な中間ディレクトリも含めてディレクトリを生成する

  Arguments:    path        生成するディレクトリ名
                permit      アクセスモード

  Returns:      ディレクトリが正常に生成されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectoryAuto(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectoryAuto

  Description:  ディレクトリを再帰的に削除する

  Arguments:    path        パス名

  Returns:      ディレクトリが正常に削除されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectoryAuto(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectoryAuto

  Description:  必要な中間ディレクトリを自動生成してディレクトリ名を変更する

  Arguments:    src         変換元のディレクトリ名
                dst         変換先のディレクトリ名

  Returns:      ディレクトリ名が正常に変更されればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectoryAuto(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResource

  Description:  指定したアーカイブのリソース情報を取得する

  Arguments:    path        アーカイブを特定できるパス名
                resource    取得したリソース情報の格納先

  Returns:      リソース情報が正常に取得できればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_GetArchiveResource(const char *path, FSArchiveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetSpaceToCreateDirectoryEntries

  Description:  ファイルの生成と同時に生成されるディレクトリエントリの格納に必要な容量を見積もる
                (パス中に存在するディレクトリも新規に作成することを想定する)

  Arguments:    path                生成したいファイルのパス名。
                bytesPerCluster     ファイルシステム上のクラスタあたりのバイト数。

  Returns:      ディレクトリエントリの格納に必要な容量
 *---------------------------------------------------------------------------*/
u32  FSi_GetSpaceToCreateDirectoryEntries(const char *path, const u32 bytesPerCluster);

/*---------------------------------------------------------------------------*
  Name:         FS_HasEnoughSpaceToCreateFile

  Description:  指定したパス名とサイズを持つファイルが現在生成可能か判定する

  Arguments:    resource    事前にFS_GetArchiveResource関数で取得したアーカイブ情報。
                            関数が成功すると、ファイルが消費する分だけ各サイズが減少する。
                path        生成したいファイルのパス名。
                size        生成したいファイルのサイズ。

  Returns:      指定したパス名とサイズを持つファイルが生成可能であればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource, const char *path, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveReady

  Description:  指定したアーカイブが現在使用可能か判定する

  Arguments:    path        "アーカイブ名:" で始まる絶対パス

  Returns:      指定したアーカイブが現在使用可能であればTRUE。
                スロットに挿入されていないSDメモリカードアーカイブや
                まだインポートされていないセーブデータアーカイブなどはFALSE。
 *---------------------------------------------------------------------------*/
BOOL FS_IsArchiveReady(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileEx

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名
                mode        アクセスモード

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_ConvertPathToFileID

  Description:  パス名からファイルIDを取得する

  Arguments:    p_fileid    FSFileIDの格納先
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileFast

  Description:  ファイルIDを指定してファイルを開く

  Arguments:    file        FSFile構造体
                fileid      FS_ReadDir関数などで得られるファイルID

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFileFast(FSFile *file, FSFileID fileid);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileDirect

  Description:  アーカイブ内のオフセットを直接マッピングしてファイルを開く

  Arguments:    file        FSFile構造体
                arc         ファイルをマップするアーカイブ
                top         ファイルイメージの先頭オフセット
                bottom      ファイルイメージの終端オフセット
                fileid      割り当てる任意のファイルID

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFileDirect(FSFile *file, FSArchive *arc,
                          u32 top, u32 bottom, u32 fileid);

/*---------------------------------------------------------------------------*
  Name:         FS_CloseFile

  Description:  ファイルハンドルを閉じる

  Arguments:    file        FSFile構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CloseFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileLength

  Description:  ファイルサイズを取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルサイズ
 *---------------------------------------------------------------------------*/
u32     FS_GetFileLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileLength

  Description:  ファイルサイズを変更する

  Arguments:    file        ファイルハンドル
                length      変更後のサイズ

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
FSResult FS_SetFileLength(FSFile *file, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFilePosition

  Description:  ファイルポインタの現在位置を取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルポインタの現在位置
 *---------------------------------------------------------------------------*/
u32     FS_GetFilePosition(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetSeekCacheSize

  Description:  高速逆シーク用のフルキャッシュに必要なバッファサイズを求める

  Arguments:    path

  Returns:      成功すればサイズ、失敗すれば0
 *---------------------------------------------------------------------------*/
u32 FS_GetSeekCacheSize( const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_SetSeekCache

  Description:  高速逆シーク用のキャッシュバッファを割り当てる

  Arguments:    file        ファイルハンドル
                buf         キャッシュバッファ
                buf_size    キャッシュバッファサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_SetSeekCache(FSFile *file, void* buf, u32 buf_size);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFile

  Description:  ファイルポインタを移動する

  Arguments:    file        ファイルハンドル
                offset      移動量
                origin      移動の起点

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFileToBegin

  Description:  ファイルポインタをファイル先頭まで移動する

  Arguments:    file        ファイルハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_SeekFileToBegin(FSFile *file)
{
    return FS_SeekFile(file, 0, FS_SEEK_SET);
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFileToEnd

  Description:  ファイルポインタをファイル終端まで移動する

  Arguments:    file        ファイルハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_SeekFileToEnd(FSFile *file)
{
    return FS_SeekFile(file, 0, FS_SEEK_END);
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFile

  Description:  ファイルからデータを読み出し

  Arguments:    file        ファイルハンドル
                buffer      転送先バッファ
                length      読み出しサイズ

  Returns:      成功すれば実際に読み出したサイズ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32     FS_ReadFile(FSFile *file, void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFileAsync

  Description:  ファイルからデータを非同期的に読み出し

  Arguments:    file        ファイルハンドル
                buffer      転送先バッファ
                length      読み出しサイズ

  Returns:      成功すれば単にlengthと同じ値、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32     FS_ReadFileAsync(FSFile *file, void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFile

  Description:  ファイルへデータを書き込み

  Arguments:    file        ファイルハンドル
                buffer      転送元バッファ
                length      書き込みサイズ

  Returns:      成功すれば実際に書き込んだサイズ、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32     FS_WriteFile(FSFile *file, const void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFileAsync

  Description:  ファイルへデータを非同期的に書き込み

  Arguments:    file        ファイルハンドル
                buffer      転送元バッファ
                length      書き込みサイズ

  Returns:      成功すれば単にlengthと同じ値、失敗すれば-1
 *---------------------------------------------------------------------------*/
s32     FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_FlushFile

  Description:  ファイルの変更内容をデバイスへ反映する

  Arguments:    file        ファイルハンドル

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
FSResult FS_FlushFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectory

  Description:  ディレクトリハンドルを開く

  Arguments:    file        FSFile構造体
                path        パス名
                mode        アクセスモード

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenDirectory(FSFile *file, const char *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_CloseDirectory

  Description:  ディレクトリハンドルを閉じる

  Arguments:    file        FSFile構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CloseDirectory(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectory

  Description:  ディレクトリのエントリを1個だけ読み進める

  Arguments:    file        FSFile構造体
                info        FSDirectoryEntryInfo構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_TellDir

  Description:  ディレクトリハンドルから現在のディレクトリ位置を取得

  Arguments:    dir         ディレクトリハンドル
                pos         ディレクトリ位置の格納先

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_TellDir(const FSFile *dir, FSDirPos *pos);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekDir

  Description:  ディレクトリハンドルをディレクトリ位置で指定して開く

  Arguments:    dir         ディレクトリハンドル
                pos         ディレクトリ位置

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_SeekDir(FSFile *p_dir, const FSDirPos *p_pos);

/*---------------------------------------------------------------------------*
  Name:         FS_RewindDir

  Description:  ディレクトリハンドルの列挙位置を先頭へ戻す

  Arguments:    dir         ディレクトリハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_RewindDir(FSFile *dir);


/*---------------------------------------------------------------------------*
 * Unicode support
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileExW

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectoryW

  Description:  ディレクトリハンドルを開く

  Arguments:    file        FSFile構造体
                path        パス名
                mode        アクセスモード

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectoryW

  Description:  ディレクトリのエントリを1個だけ読み進める

  Arguments:    file        FSFile構造体
                info        FSDirectoryEntryInfo構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info);


/*---------------------------------------------------------------------------*
 * obsolete functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFile

  Description:  パス名を指定してファイルを開く

  Arguments:    file        FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFile(FSFile *file, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_GetLength

  Description:  ファイルサイズを取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルサイズ
 *---------------------------------------------------------------------------*/
u32     FS_GetLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPosition

  Description:  ファイルポインタの現在位置を取得

  Arguments:    file        ファイルハンドル

  Returns:      ファイルポインタの現在位置
 *---------------------------------------------------------------------------*/
u32     FS_GetPosition(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_FindDir

  Description:  ディレクトリハンドルを開く

  Arguments:    dir         FSFile構造体
                path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_FindDir(FSFile *dir, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDir

  Description:  ディレクトリハンドルのエントリ情報を1個だけ読み進める

  Arguments:    dir         ディレクトリハンドル
                entry       エントリ情報の格納先

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ReadDir(FSFile *dir, FSDirEntry *entry);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileInfo

  Description:  ファイルまたはディレクトリの情報を取得する

  Arguments:    path        パス名
                info        情報の格納先

  Returns:      処理結果
 *---------------------------------------------------------------------------*/
typedef FSPathInfo  FSFileInfo;
FSResult FS_GetFileInfo(const char *path, FSFileInfo *info);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_FILE_H_ */
