/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FATFS - include
  File:     api.h

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

#ifndef NITRO_FATFS_API_H_
#define NITRO_FATFS_API_H_


#include <twl/fatfs/common/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FATFS_Init

  Description:  FATFSライブラリを初期化

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
void FATFS_Init(void);
#else
BOOL FATFS_Init(u32 dma1, u32 dma2, u32 priority);
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFSi_IsInitialized

  Description:  FATFSライブラリが初期化済みかどうかを返す

  Arguments:    None

  Returns:      初期化済みならTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFSi_IsInitialized(void);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetArcnameList

  Description:  アクセス可能なアーカイブ名の一覧を取得。

  Arguments:    None

  Returns:      "\0"で区切られ終端は"\0\0"で与えられたアーカイブ名一覧。
 *---------------------------------------------------------------------------*/
const char* FATFSi_GetArcnameList(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetLastError

  Description:  最後に発行したリクエストの結果を取得。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
u32 FATFS_GetLastError(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RegisterResultBuffer

  Description:  呼び出し元スレッドに関連付けるコマンド結果バッファを登録。

  Arguments:    buffer : FATFSResultBuffer構造体。
                enable : 登録ならTRUE, 解除ならFALSE。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFS_RegisterResultBuffer(FATFSResultBuffer *buffer, BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDrive

  Description:  指定のNANDパーティションをドライブにマウント

  Arguments:    name : ドライブ文字名 ("A"-"Z")
                partition : NAND内のパーティション番号

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDrive(const char *name, FATFSMediaType media, u32 partition);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountNAND

  Description:  指定のNANDパーティションをドライブにマウント

  Arguments:    name : ドライブ文字名 ("A"-"Z")
                partition : NAND内のパーティション番号

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FATFS_MountNAND(const char *name, u32 partition)
{
    return FATFS_MountDrive(name, FATFS_MEDIA_TYPE_NAND, partition);
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_UnmountDrive

  Description:  指定のドライブをアンマウント

  Arguments:    name : ドライブ文字名 ("A"-"Z")

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_UnmountDrive(const char *name);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetDefaultDrive

  Description:  デフォルトのドライブを選択

  Arguments:    path : ドライブ名を含んだパス名

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetDefaultDrive(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FormatDrive

  Description:  指定のパスが指すドライブを初期化

  Arguments:    path : ドライブ名を含んだパス名

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_FormatDrive(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_FormatDriveEx

  Description:  指定のパスが指すドライブ全体またはメディア全体を初期化

  Arguments:    path        : ドライブ名を含んだパス名
                formatMedia : メディア全体を消去するならTRUE

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFSi_FormatDriveEx(const char *path, BOOL formatMedia);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_FormatMedia

  Description:  指定のパスが指すメディア全体を初期化

  Arguments:    path : ドライブ名を含んだパス名

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FATFSi_FormatMedia(const char *path)
{
    return FATFSi_FormatDriveEx(path, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_CheckDisk

  Description:  ディスクの内容をチェックし、必要なら修正する

  Arguments:    name : ドライブ文字名 ("A"-"Z")
                info : 結果を格納するFATFSDiskInfo構造体
                verbose : チェック内容をデバッグ出力する場合はTRUE
                fixProblems : 検出されたFATの異常をその場で修正する場合はTRUE
                writeChains : 孤立したチェーンからファイルを復元する場合はTRUE
                              (fixProblemsが有効の場合にのみ有効)

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CheckDisk(const char *name, FATFSDiskInfo *info, BOOL verbose, BOOL fixProblems, BOOL writeChains);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDriveResource

  Description:  ディスクの空き容量を取得

  Arguments:    path     : ドライブ名を含んだパス名
                resource : 結果を格納するFATFSDriveResource構造体

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetDriveResource(const char *path, FATFSDriveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDiskSpace

  Description:  ディスクの空き容量を取得

  Arguments:    name : ドライブ文字名 ("A"-"Z")
                totalBlocks : 合計ブロック数を取得するポインタまたはNULL
                freeBlocks : 空きブロック数を取得するポインタまたはNULL

  Returns:      成功すれば空き容量を示すバイト数、失敗すれば-1
 *---------------------------------------------------------------------------*/
int FATFS_GetDiskSpace(const char *name, u32 *totalBlocks, u32 *freeBlocks);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileInfo

  Description:  ファイルまたはディレクトリの情報を取得

  Arguments:    path : パス名
                info : 結果を格納するFATFSFileInfo構造体

  Returns:      指定のパスが存在すれば情報を取得してTRUE、存在しなければFALSE
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetFileInfo(const char *path, FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileInfo

  Description:  ファイルまたはディレクトリの情報を変更

  Arguments:    path : パス名
                info : 変更すべき情報が格納されたFATFSFileInfo構造体

  Returns:      指定のパスが存在すれば情報を変更してTRUE、存在しなければFALSE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileInfo(const char *path, const FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateFile

  Description:  指定のパスにファイルを生成

  Arguments:    path : パス名
                trunc : すでに存在するファイルを破棄するならTRUE
                permit : ファイルのアクセス権 (rwx){1,3}

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateFile(const char *path, BOOL trunc, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteFile

  Description:  指定のファイルを削除

  Arguments:    path : パス名

  Returns:      削除に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteFile(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameFile

  Description:  指定のファイルを改名

  Arguments:    path : パス名
                newpath : 新しいパス名

  Returns:      改名に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameFile(const char *path, const char *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateDirectory

  Description:  指定のパスにディレクトリを生成

  Arguments:    path : パス名
                permit : ディレクトリのアクセス権 (rwx){1,3}

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateDirectory(const char *path, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteDirectory

  Description:  指定のディレクトリを削除

  Arguments:    path : パス名

  Returns:      削除に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameDirectory

  Description:  指定のディレクトリを改名

  Arguments:    path : パス名
                newpath : 新しいパス名

  Returns:      改名に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameDirectory(const char *path, const char *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenFile

  Description:  指定パスのファイルを開く

  Arguments:    path : パス名
                mode : ファイルのアクセスモード ("r/w/r+/w+" + "b/t")

  Returns:      開いたファイルのハンドルまたはNULL
 *---------------------------------------------------------------------------*/
FATFSFileHandle FATFS_OpenFile(const char *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CloseFile

  Description:  ファイルを閉じる

  Arguments:    file : ファイルハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CloseFile(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadFile

  Description:  ファイルからデータを読み出し

  Arguments:    file : ファイルハンドル
                buffer : 読み出したデータの格納先
                length : 読み出すサイズ

  Returns:      実際に読み出したサイズまたは-1
 *---------------------------------------------------------------------------*/
int FATFS_ReadFile(FATFSFileHandle file, void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_WriteFile

  Description:  ファイルへデータを書き込み

  Arguments:    file : ファイルハンドル
                buffer : 書き込むデータの格納元
                length : 書き込むサイズ

  Returns:      実際に書き込んだサイズまたは-1
 *---------------------------------------------------------------------------*/
int FATFS_WriteFile(FATFSFileHandle file, const void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetSeekCache

  Description:  高速逆シーク用にキャッシュバッファを割り当て

  Arguments:    file     : ファイルハンドル
                buf      : キャッシュバッファ
                buf_size : キャッシュバッファサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetSeekCache(FATFSFileHandle file, void* buf, u32 buf_size);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SeekFile

  Description:  ファイルポインタを移動

  Arguments:    file : ファイルハンドル
                offset : 移動量
                origin : 移動基点

  Returns:      移動後のオフセットまたは-1
 *---------------------------------------------------------------------------*/
int FATFS_SeekFile(FATFSFileHandle file, int offset, FATFSSeekMode origin);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FlushFile

  Description:  ファイルの内容をドライブへフラッシュ

  Arguments:    file : ファイルハンドル

  Returns:      正常にフラッシュされればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_FlushFile(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileLength

  Description:  ファイルサイズを取得

  Arguments:    file : ファイルハンドル

  Returns:      ファイルサイズまたは-1
 *---------------------------------------------------------------------------*/
int FATFS_GetFileLength(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileLength

  Description:  ファイルサイズを設定

  Arguments:    file : ファイルハンドル

  Returns:      設定が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileLength(FATFSFileHandle file, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenDirectory

  Description:  指定パスのディレクトリを開く

  Arguments:    path : パス名
                mode : ディレクトリのアクセスモード (現在は無視される)

  Returns:      開いたディレクトリのハンドルまたはNULL
 *---------------------------------------------------------------------------*/
FATFSDirectoryHandle FATFS_OpenDirectory(const char *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CloseDirectory

  Description:  ディレクトリを閉じる

  Arguments:    dir : ディレクトリハンドル

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CloseDirectory(FATFSDirectoryHandle dir);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadDirectory

  Description:  ディレクトリから次のエントリ情報を読み出し

  Arguments:    dir : ディレクトリハンドル
                info : エントリ情報の格納先

  Returns:      正常にエントリ情報を読み出したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_ReadDirectory(FATFSDirectoryHandle dir, FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FlushAll

  Description:  全てのドライブの内容をデバイスへフラッシュ

  Arguments:    None

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_FlushAll(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_UnmountAll

  Description:  全てのドライブをアンマウント

  Arguments:    None

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_UnmountAll(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountSpecial

  Description:  特殊ドライブをマウント

  Arguments:    param   : マウント対象を指定するパラメータ
                arcname : マウントするアーカイブ名
                          "otherPub", "otherPrv", "share", が指定可能。
                          NULLを指定すると以前のドライブをアンマウント
                slot    : 割り当てられたインデックスを格納しておくバッファ。
                          マウント時に設定され、アンマウント時に参照される

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountSpecial(u64 param, const char *arcname, int *slot);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FormatSpecial

  Description:  自身が所有権を持つ特殊ドライブを再初期化。

  Arguments:    path : ドライブ名を含んだパス名

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_FormatSpecial(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetLatencyEmulation

  Description:  劣化したデバイスへのアクセス時に生じるウェイト時間を
                ドライバ層で擬似的に再現させるよう指示。

  Arguments:    enable : 有効にする場合はTRUE。

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetLatencyEmulation(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SearchWildcard

  Description:  SDカードの所定ディレクトリ内をワイルドカードで検索。

  Arguments:    directory : 8文字以内の任意のディレクトリ名。
                            アプリのルートディレクトリ直下に
                            ファイルを書き出すならばNULL。
                prefix    : ファイル名のプレフィックス部(1～5文字)
                suffix    : ファイルの拡張子名(1～3文字)
                buffer    : 条件にマッチした既存ファイルを記録するバッファ。
                            (buffer[i / 8] & (1 << (i % 8))) が真ならば
                            "prefix{i}.suffix" が存在することを示す。
                length    : bufferの長さ。

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SearchWildcard(const char* directory, const char *prefix, const char *suffix,
                          void *buffer, u32 length);

/*---------------------------------------------------------------------------*
 * Unicode対応版
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDriveResourceW

  Description:  ディスクの空き容量を取得

  Arguments:    path     : ドライブ名を含んだパス名
                resource : 結果を格納するFATFSDriveResource構造体

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetDriveResourceW(const u16 *path, FATFSDriveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileInfoW

  Description:  ファイルまたはディレクトリの情報を取得

  Arguments:    path : パス名
                info : 結果を格納するFATFSFileInfoW構造体

  Returns:      指定のパスが存在すれば情報を取得してTRUE、存在しなければFALSE
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetFileInfoW(const u16 *path, FATFSFileInfoW *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileInfoW

  Description:  ファイルまたはディレクトリの情報を変更

  Arguments:    path : パス名
                info : 変更すべき情報が格納されたFATFSFileInfoW構造体

  Returns:      指定のパスが存在すれば情報を変更してTRUE、存在しなければFALSE
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileInfoW(const u16 *path, const FATFSFileInfoW *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateFileW

  Description:  指定のパスにファイルを生成

  Arguments:    path : パス名
                trunc : すでに存在するファイルを破棄するならTRUE
                permit : ファイルのアクセス権 (rwx){1,3}

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateFileW(const u16 *path, BOOL trunc, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteFileW

  Description:  指定のファイルを削除

  Arguments:    path : パス名

  Returns:      削除に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteFileW(const u16 *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameFileW

  Description:  指定のファイルを改名

  Arguments:    path : パス名
                newpath : 新しいパス名

  Returns:      改名に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameFileW(const u16 *path, const u16 *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateDirectoryW

  Description:  指定のパスにディレクトリを生成

  Arguments:    path : パス名
                permit : ファイルのアクセス権 (rwx){1,3}

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateDirectoryW(const u16 *path, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteDirectoryW

  Description:  指定のディレクトリを削除

  Arguments:    path : パス名

  Returns:      削除に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteDirectoryW(const u16 *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameDirectoryW

  Description:  指定のディレクトリを改名

  Arguments:    path : パス名
                newpath : 新しいパス名

  Returns:      改名に成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameDirectoryW(const u16 *path, const u16 *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenFileW

  Description:  指定パスのファイルを開く

  Arguments:    path : パス名
                mode : ファイルのアクセスモード ("r/w/r+/w+" + "b/t")

  Returns:      開いたファイルのハンドルまたはNULL
 *---------------------------------------------------------------------------*/
FATFSFileHandle FATFS_OpenFileW(const u16 *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenDirectoryW

  Description:  指定パスのディレクトリを開く

  Arguments:    path : パス名
                mode : ディレクトリのアクセスモード (現在は無視される)

  Returns:      開いたディレクトリのハンドルまたはNULL
 *---------------------------------------------------------------------------*/
FATFSDirectoryHandle FATFS_OpenDirectoryW(const u16 *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadDirectoryW

  Description:  ディレクトリから次のエントリ情報を読み出し

  Arguments:    dir : ディレクトリハンドル
                info : エントリ情報の格納先

  Returns:      正常にエントリ情報を読み出したらTRUE
 *---------------------------------------------------------------------------*/
BOOL FATFS_ReadDirectoryW(FATFSDirectoryHandle dir, FATFSFileInfoW *info);


/*---------------------------------------------------------------------------*
 * internal functions
 *---------------------------------------------------------------------------*

 /*---------------------------------------------------------------------------*
  Name:         FATFSiArcnameList

  Description:  ARM9から参照可能なアーカイブ名のリストを保持する静的なバッファ。
                ARM9/ARM7ともに参照可能なメモリである必要があり、
                デフォルトではLTDMAINセグメント内の静的変数が使用される。
                特殊なメモリ配置で構築されたアプリケーションでは
                この変数をFATFS_Init関数の呼び出しより前に変更することで
                適切なバッファを設定することができる。

  Arguments:    None.

  Returns:      ARM9から参照可能なアーカイブ名のリストを保持する静的なバッファ
 *---------------------------------------------------------------------------*/
extern char *FATFSiArcnameList/* [MATH_ROUNDUP(OS_MOUNT_ARCHIVE_NAME_LEN * OS_MOUNT_INFO_MAX + 1, 32)] ATTRIBUTE_ALIGN(32) */;

/*---------------------------------------------------------------------------*
  Name:         FATFSiCommandBuffer

  Description:  リクエスト発行用の静的なコマンドバッファへのポインタ。
                ARM9/ARM7ともに参照可能なメモリである必要があり、
                デフォルトではLTDMAINセグメント内の静的変数が使用される。
                特殊なメモリ配置で構築されたアプリケーションでは
                この変数をFATFS_Init関数の呼び出しより前に変更することで
                適切なバッファを設定することができる。

  Arguments:    None.

  Returns:      FATFS_COMMAND_BUFFER_MAXバイトの静的なコマンドバッファ
 *---------------------------------------------------------------------------*/
extern u8 *FATFSiCommandBuffer/* [FATFS_COMMAND_BUFFER_MAX] ATTRIBUTE_ALIGN(32)*/;

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetUnicodeConversionTable

  Description:  ARM7に共有させるためのUnicode変換テーブルを取得する。
                ASCIIパス名しか扱わないことが明らかなアプリケーションでは
                この関数をオーバーライドして無効化することにより
                80kB弱のUnicode変換テーブルを削除することができる。

  Arguments:    u2s : UnicodeからShift_JISへの変換テーブル
                s2u : Shift_JISからUnicodeへの変換テーブル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetNdmaParameters

  Description:  ARM7のFATドライバが使用するNDMA設定を直接変更する。

  Arguments:    ndmaNo        : 設定を変更するNDMAチャンネル (0-3)
                blockWord     : ブロック転送ワード数 (MI_NDMA_BWORD_1-MI_NDMA_BWORD_32768)
                intervalTimer : インターバル (0x0000-0xFFFF)
                prescaler     : プリスケーラ (MI_NDMA_INTERVAL_PS_1-MI_NDMA_INTERVAL_PS_64)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_SetNdmaParameters(u32 ndmaNo, u32 blockWord, u32 intervalTimer, u32 prescaler);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetRequestBuffer

  Description:  次に発行するコマンドを非同期処理へ変更するために
                リクエストバッファを設定する。

  Arguments:    buffer : リクエスト管理に使用するFATFSRequestBuffer構造体。
                         32バイトの整数倍で境界整合していなければならない9。

                callback : コマンド完了時に呼び出されるコールバック。

                userdata : リクエストバッファに関連付ける任意のユーザ定義値。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_SetRequestBuffer(FATFSRequestBuffer *buffer, void (*callback)(FATFSRequestBuffer *), void *userdata);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_FATFS_API_H_ */
#endif
