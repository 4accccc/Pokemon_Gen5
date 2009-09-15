/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - include
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
#if	!defined(NITRO_FS_API_H_)
#define NITRO_FS_API_H_


#include <nitro/fs/archive.h>
#include <nitro/fs/file.h>
#include <nitro/fs/romfat.h>
#include <nitro/fs/overlay.h>
#include <nitro/std.h>

#ifdef SDK_TWL
#include <twl/fatfs.h>
#endif // SDK_TWL

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FS_Init

  Description:  FSライブラリを初期化

  Arguments:    default_dma_no   必要に応じて内部で使用するDMA番号。 (0-3)
                                 新規DMAを使用する場合はMI_DMA_USING_NEWを加え、
                                 DMAを使用させない場合はMI_DMA_NOT_USEを指定する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_Init(u32 default_dma_no);

/*---------------------------------------------------------------------------*
  Name:         FS_InitFatDriver

  Description:  FATドライバを初期化。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_TWL)
void FS_InitFatDriver(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_IsAvailable

  Description:  FSライブラリが初期化済みか判定

  Arguments:    None.

  Returns:      FS_Init関数がすでに呼び出されていればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_IsAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         FS_End

  Description:  全てのアーカイブをアンマウントしてFSライブラリを終了する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_End(void);

/*---------------------------------------------------------------------------*
  Name:         FS_GetDefaultDMA

  Description:  現在設定されているFSライブラリ用DMAチャンネルを取得

  Arguments:    None.

  Returns:      current DMA channel.
 *---------------------------------------------------------------------------*/
u32     FS_GetDefaultDMA(void);

/*---------------------------------------------------------------------------*
  Name:         FS_SetDefaultDMA

  Description:  FSライブラリ用DMAチャンネルを設定する

  Arguments:    dma_no      必要に応じて内部で使用するDMA番号。 (0-3)
                            新規DMAを使用する場合はMI_DMA_USING_NEWを加え、
                            DMAを使用させない場合はMI_DMA_NOT_USEを指定する。

  Returns:      以前に設定されていたDMAチャンネル
 *---------------------------------------------------------------------------*/
u32     FS_SetDefaultDMA(u32 dma_no);

/*---------------------------------------------------------------------------*
  Name:         FS_SetCurrentDirectory

  Description:  カレントディレクトリを変更する

  Arguments:    path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_SetCurrentDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromMemory

  Description:  メモリ領域をマップしたファイルを一時的に生成

  Arguments:    file             ファイルハンドルを格納するFSFile構造体
                buf              READ および WRITE の対象となるメモリ
                size             buf のバイトサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFileFromMemory(FSFile *file, void *buf, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromRom

  Description:  指定の CARD-ROM 領域をマップしたファイルを一時的に生成

  Arguments:    file             ファイルハンドルを格納する FSFile 構造体
                offset           READ の対象となる CARD-ROM 領域先頭オフセット
                size             対象領域の offset からのバイトサイズ

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFileFromRom(FSFile *file, u32 offset, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenTopLevelDirectory

  Description:  アーカイブ名を列挙できる特殊な最上位ディレクトリを開く

  Arguments:    dir              ディレクトリハンドルを格納する FSFile 構造体

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenTopLevelDirectory(FSFile *dir);

/*---------------------------------------------------------------------------*
  Name:         FS_TryLoadTable

  Description:  "rom"アーカイブのテーブルテータをプリロードする

  Arguments:    mem              テーブルデータの格納先バッファ
                                 NULlなら単にサイズだけを計算して返す
                size             memのサイズ

  Returns:      "rom"アーカイブのFATとFNTをプリロードするのに必要なバイト数
 *---------------------------------------------------------------------------*/
u32     FS_TryLoadTable(void *mem, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_GetTableSize

  Description:  "ROM"アーカイブのテーブルサイズを取得

  Arguments:    None.

  Returns:      "rom"アーカイブのFATとFNTをプリロードするのに必要なバイト数
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 FS_GetTableSize(void)
{
    return FS_TryLoadTable(NULL, 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadTable

  Description:  "rom"アーカイブのテーブルテータをプリロードする

  Arguments:    mem              テーブルデータの格納先バッファ
                size             memのサイズ

  Returns:      プリロードに成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_LoadTable(void *mem, u32 size)
{
    return (FS_TryLoadTable(mem, size) <= size);
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadTable

  Description:  "rom"アーカイブにプリロードしたテーブルを解放する

  Arguments:    None.

  Returns:      テーブルのために割り当てられていたバッファへのポインタ
 *---------------------------------------------------------------------------*/
SDK_INLINE void *FS_UnloadTable(void)
{
    return FS_UnloadArchiveTables(FS_FindArchive("rom", 3));
}

/*---------------------------------------------------------------------------*
  Name:         FS_ForceToEnableLatencyEmulation

  Description:  劣化したNANDデバイスへのアクセス時に生じる
                ランダムなウェイト時間をドライバで擬似的に再現する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ForceToEnableLatencyEmulation(void);


/*---------------------------------------------------------------------------*
 * internal functions
 *---------------------------------------------------------------------------*/

#define FS_TMPBUF_LENGTH        2048
#define FS_MOUNTDRIVE_MAX       OS_MOUNT_INFO_MAX
#define FS_TEMPORARY_BUFFER_MAX (FS_TMPBUF_LENGTH * FS_MOUNTDRIVE_MAX)

#ifdef SDK_TWL
typedef struct FSFATFSArchiveContext
{
    FSArchive       arc[1];
    char            fullpath[2][FATFS_PATH_MAX];
    u8             *tmpbuf;
    FATFSDriveResource  resource[1];
}
FSFATFSArchiveContext;

typedef struct FSFATFSArchiveWork
{
    u8 tmpbuf[FS_TMPBUF_LENGTH];
    FSFATFSArchiveContext context;
    int slot;
}
FSFATFSArchiveWork;


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
extern FSFATFSArchiveContext *FSiFATFSDrive;

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
extern FATFSRequestBuffer *FSiFATFSAsyncRequest;

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
FSResult FSi_MountSpecialArchive(u64 param, const char *arcname, FSFATFSArchiveWork* pWork);

/*---------------------------------------------------------------------------*
  Name:         FSi_FormatSpecialArchive

  Description:  以下の条件を満たす特殊なアーカイブの内容をフォーマット。
                  - 現在すでにマウントされている。
                  - 自身が所有権を持つ。 (dataPub, dataPrv, share*)

  Arguments:    path : アーカイブ名を含んだパス名

  Returns:      処理が成功すればFS_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
FSResult FSi_FormatSpecialArchive(const char *path);

#endif // SDK_TWL

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
extern u8 *FSiTemporaryBuffer/* [FS_TEMPORARY_BUFFER_MAX] ATTRIBUTE_ALIGN(32)*/;

/*---------------------------------------------------------------------------*
  Name:         FSi_SetupFATBuffers

  Description:  FATベースのアーカイブに必要な各種バッファを初期化。
                特殊なメモリ配置で構築されたアプリケーションでは
                この関数をオーバーライドして各種バッファを独自に設定し
                所要メモリサイズを必要最低限に抑止することができる。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_SetupFATBuffers(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_OverrideRomArchive

  Description:  必要に応じてROMアーカイブの内容を置き換える。

  Arguments:    arc              マウントすべきROMアーカイブ構造体

  Returns:      カード上にある標準のROMアーカイブを使用せず
                別の実装に置き換えるべきであればTRUEを返す必要がある。
 *---------------------------------------------------------------------------*/
BOOL FSi_OverrideRomArchive(FSArchive *arc);

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
BOOL FSi_IsValidAddressForARM7(const void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FSi_SetSwitchableWramSlots

  Description:  状況に応じてFSライブラリがARM7へ切り替えてよいWRAMスロットを指定。

  Arguments:    bitsB               WRAM-Bスロットのビット集合
                bitsC               WRAM-Cスロットのビット集合

  Returns:      None
 *---------------------------------------------------------------------------*/
void FSi_SetSwitchableWramSlots(int bitsB, int bitsC);

/*---------------------------------------------------------------------------*
  Name:         FSi_UnmountRomAndCloseNANDSRL

  Description:  NANDアプリケーション用のROMアーカイブを無効化しSRLファイルを閉じる。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_UnmountRomAndCloseNANDSRL(void);

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
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback);

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
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeBuffer

  Description:  Unicode変換用の一時バッファを取得。
                FSライブラリがShift_JISを変換するために使用される。

  Arguments:    src : Unicode変換の必要なShift_JIS文字列またはNULL

  Returns:      必要に応じてUTF16-LEへ変換された文字列バッファ
 *---------------------------------------------------------------------------*/
u16* FSi_GetUnicodeBuffer(const char *src);

/*---------------------------------------------------------------------------*
  Name:         FSi_ReleaseUnicodeBuffer

  Description:  Unicode変換用の一時バッファを解放。

  Arguments:    buf : FSi_GetUnicodeBuffer()で確保したバッファ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ReleaseUnicodeBuffer(const void *buf);


/*---------------------------------------------------------------------------*
 * obsolete functions
 *---------------------------------------------------------------------------*/

#define	FS_DMA_NOT_USE	MI_DMA_NOT_USE

/*---------------------------------------------------------------------------*
  Name:         FS_ChangeDir

  Description:  カレントディレクトリを変更する

  Arguments:    path        パス名

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_ChangeDir(const char *path);


#ifdef SDK_ARM7
#define FS_CreateReadServerThread(priority) (void)CARD_SetThreadPriority(priority)
#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_API_H_ */
