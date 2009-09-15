/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     rfat.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_FS_RFAT_H_
#define NITRO_FS_RFAT_H_


#include <nitro/fs/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

//#define FS_DISABLE_OLDPROTOTYPES

#define FS_COMMAND_ASYNC_BEGIN  (FSCommandType)(FS_COMMAND_READFILE)
#define FS_COMMAND_ASYNC_END    (FSCommandType)(FS_COMMAND_WRITEFILE + 1)
#define FS_COMMAND_SYNC_BEGIN   (FSCommandType)(FS_COMMAND_SEEKDIR)
#define FS_COMMAND_SYNC_END     (FSCommandType)(FS_COMMAND_CLOSEFILE + 1)
#define FS_COMMAND_STATUS_BEGIN (FSCommandType)(FS_COMMAND_ACTIVATE)
#define FS_COMMAND_STATUS_END   (FSCommandType)(FS_COMMAND_RESUME + 1)

// 互換性のための欠番
#define FS_ARCHIVE_FLAG_IS_SYNC     0x00000100
#define FS_ARCHIVE_FLAG_TABLE_LOAD  0x00000004
#define FS_ARCHIVE_FLAG_OBSOLETE    0x00000104

// ユーザプロシージャ設定フラグ。
// FS_SetArchiveProc() で引数 flags に指定します。
// コールバックを要求するものだけビットを有効にします。

// 非同期系コマンド
#define	FS_ARCHIVE_PROC_READFILE		(1 << FS_COMMAND_READFILE)
#define	FS_ARCHIVE_PROC_WRITEFILE		(1 << FS_COMMAND_WRITEFILE)
// 非同期系の全コマンド
#define	FS_ARCHIVE_PROC_ASYNC	\
	(FS_ARCHIVE_PROC_READFILE | FS_ARCHIVE_PROC_WRITEFILE)

// 同期系のコマンド
#define	FS_ARCHIVE_PROC_SEEKDIR			(1 << FS_COMMAND_SEEKDIR)
#define	FS_ARCHIVE_PROC_READDIR			(1 << FS_COMMAND_READDIR)
#define	FS_ARCHIVE_PROC_FINDPATH		(1 << FS_COMMAND_FINDPATH)
#define	FS_ARCHIVE_PROC_GETPATH			(1 << FS_COMMAND_GETPATH)
#define	FS_ARCHIVE_PROC_OPENFILEFAST	(1 << FS_COMMAND_OPENFILEFAST)
#define	FS_ARCHIVE_PROC_OPENFILEDIRECT	(1 << FS_COMMAND_OPENFILEDIRECT)
#define	FS_ARCHIVE_PROC_CLOSEFILE		(1 << FS_COMMAND_CLOSEFILE)
// 同期系の全コマンド
#define	FS_ARCHIVE_PROC_SYNC	\
	(FS_ARCHIVE_PROC_SEEKDIR | FS_ARCHIVE_PROC_READDIR |	\
	 FS_ARCHIVE_PROC_FINDPATH | FS_ARCHIVE_PROC_GETPATH |	\
	FS_ARCHIVE_PROC_OPENFILEFAST | FS_ARCHIVE_PROC_OPENFILEDIRECT | FS_ARCHIVE_PROC_CLOSEFILE)

// 状態変化時のメッセージ
#define	FS_ARCHIVE_PROC_ACTIVATE		(1 << FS_COMMAND_ACTIVATE)
#define	FS_ARCHIVE_PROC_IDLE			(1 << FS_COMMAND_IDLE)
#define	FS_ARCHIVE_PROC_SUSPENDING		(1 << FS_COMMAND_SUSPEND)
#define	FS_ARCHIVE_PROC_RESUME			(1 << FS_COMMAND_RESUME)
// 状態変化時の全メッセージ
#define	FS_ARCHIVE_PROC_STATUS	\
	(FS_ARCHIVE_PROC_ACTIVATE | FS_ARCHIVE_PROC_IDLE |	\
	 FS_ARCHIVE_PROC_SUSPENDING | FS_ARCHIVE_PROC_RESUME)

// すべてのメッセージ
#define	FS_ARCHIVE_PROC_ALL	(~0)


/*---------------------------------------------------------------------------*/
/* declarations */

typedef FSResult (*FS_ARCHIVE_PROC_FUNC) (struct FSFile *, FSCommandType);
typedef FSResult (*FS_ARCHIVE_READ_FUNC) (struct FSArchive *p, void *dst, u32 pos, u32 size);
typedef FSResult (*FS_ARCHIVE_WRITE_FUNC) (struct FSArchive *p, const void *src, u32 pos, u32 size);

typedef struct FSArchiveFAT
{
    u32     top;
    u32     bottom;
}
FSArchiveFAT;

typedef struct FSArchiveFNT
{
    u32     start;
    u16     index;
    u16     parent;
}
FSArchiveFNT;

#define FS_ROMFAT_CONTEXT_DEFINITION()      \
{                                           \
    u32                     base;           \
    u32                     fat;            \
    u32                     fat_size;       \
    u32                     fnt;            \
    u32                     fnt_size;       \
    u32                     fat_bak;        \
    u32                     fnt_bak;        \
    void                   *load_mem;       \
    FS_ARCHIVE_READ_FUNC    read_func;      \
    FS_ARCHIVE_WRITE_FUNC   write_func;     \
    u8                      reserved3[4];   \
    FS_ARCHIVE_PROC_FUNC    proc;           \
    u32                     proc_flag;      \
}

typedef struct FSROMFATArchiveContext
FS_ROMFAT_CONTEXT_DEFINITION()
FSROMFATArchiveContext;

#ifdef FS_DISABLE_OLDPROTOTYPES
#undef FS_ROMFAT_CONTEXT_DEFINITION
#define FS_ROMFAT_CONTEXT_DEFINITION() { u8 reserved[52]; } obsolete
#endif // FS_DISABLE_OLDPROTOTYPES

// for FS_COMMAND_SEEKDIR
typedef struct
{
    FSDirPos pos;
}
FSSeekDirInfo;

// for FS_COMMAND_READDIR
typedef struct
{
    FSDirEntry *p_entry;
    BOOL    skip_string;
}
FSReadDirInfo;

// for FS_COMMAND_FINDPATH
typedef struct
{
    FSDirPos pos;
    const char *path;
    BOOL    find_directory;
    union
    {
        FSFileID *file;
        FSDirPos *dir;
    }
    result;
}
FSFindPathInfo;

// for FS_COMMAND_GETPATH
typedef struct
{
    u8     *buf;
    u32     buf_len;
    u16     total_len;
    u16     dir_id;
}
FSGetPathInfo;

// for FS_COMMAND_OPENFILEFAST
typedef struct
{
    FSFileID id;
}
FSOpenFileFastInfo;

// for FS_COMMAND_OPENFILEDIRECT
typedef struct
{
    u32     top;
    u32     bottom;
    u32     index;
}
FSOpenFileDirectInfo;

// for FS_COMMAND_CLOSEFILE
typedef struct
{
    u32     reserved;
}
FSCloseFileInfo;

// for FS_COMMAND_READFILE
typedef struct
{
    void   *dst;
    u32     len_org;
    u32     len;
}
FSReadFileInfo;

// for FS_COMMAND_WRITEFILE
typedef struct
{
    const void *src;
    u32     len_org;
    u32     len;
}
FSWriteFileInfo;

typedef union FSROMFATCommandInfo
{
    FSReadFileInfo          readfile;
    FSWriteFileInfo         writefile;
    FSSeekDirInfo           seekdir;
    FSReadDirInfo           readdir;
    FSFindPathInfo          findpath;
    FSGetPathInfo           getpath;
    FSOpenFileFastInfo      openfilefast;
    FSOpenFileDirectInfo    openfiledirect;
    FSCloseFileInfo         closefile;
}
FSROMFATCommandInfo;

typedef struct FSROMFATFileProperty
{
    u32                 own_id;
    u32                 top;
    u32                 bottom;
    u32                 pos;
}
FSROMFATFileProperty;

typedef struct FSROMFATDirProperty
{
    FSDirPos            pos;
    u32                 parent;
}
FSROMFATDirProperty;

typedef union FSROMFATProperty
{
    FSROMFATFileProperty    file;
    FSROMFATDirProperty     dir;
}
FSROMFATProperty;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveBase

  Description:  ROMFAT型アーカイブのベースオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのベースオフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveBase(const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFAT

  Description:  ROMFAT型アーカイブのFATオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのFATオフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveFAT(const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveFNT

  Description:  ROMFAT型アーカイブのFNTオフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブのFNTオフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveFNT(const struct FSArchive *arc);

/* アーカイブのベースからの指定位置オフセットを取得 */
/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveOffset

  Description:  ROMFAT型アーカイブのベースからの指定位置オフセットを取得

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ベースを加算した指定位置オフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetArchiveOffset(const struct FSArchive *arc, u32 pos);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveTableLoaded

  Description:  ROMFAT型アーカイブが現在テーブルをプリロード済みか判定

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      テーブルをプリロード済みならTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_IsArchiveTableLoaded(volatile const struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchive

  Description:  ROMFAT型アーカイブをファイルシステムにロードする

  Arguments:    arc              ROMFAT型アーカイブ
                base             独自に使用可能な任意のu32値
                fat              FATテーブルの先頭オフセット
                fat_size         FATテーブルのサイズ
                fnt              FNTテーブルの先頭オフセット
                fnt_size         FNTテーブルのサイズ
                read_func        リードアクセスコールバック
                write_func       ライトアクセスコールバック

  Returns:      ROMFAT型アーカイブが正しくロードされればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_LoadArchive(struct FSArchive *arc, u32 base,
                       u32 fat, u32 fat_size, u32 fnt, u32 fnt_size,
                       FS_ARCHIVE_READ_FUNC read_func, FS_ARCHIVE_WRITE_FUNC write_func);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchive

  Description:  ROMFAT型アーカイブをファイルシステムからアンロードする

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      ROMFAT型アーカイブが正しくアンロードされればTRUE
 *---------------------------------------------------------------------------*/
BOOL    FS_UnloadArchive(struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_LoadArchiveTables

  Description:  FS_UnloadArchiveアーカイブのFAT+FNTをメモリ上にプリロードする。
                指定サイズ以内の場合のみ読み込みを実行して必要サイズを返す。

  Arguments:    arc              ROMFAT型アーカイブ
                mem              テーブルデータの格納先バッファ
                size             memのサイズ

  Returns:      常に合計のテーブルサイズを返す
 *---------------------------------------------------------------------------*/
u32     FS_LoadArchiveTables(struct FSArchive *arc, void *mem, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_UnloadArchiveTables

  Description:  ROMFAT型アーカイブのプリロードメモリを解放

  Arguments:    arc              ROMFAT型アーカイブ

  Returns:      プリロードメモリとしてユーザから与えられていたバッファ
 *---------------------------------------------------------------------------*/
void   *FS_UnloadArchiveTables(struct FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_SetArchiveProc

  Description:  ROMFAT型アーカイブのユーザプロシージャを設定する。
                proc == NULL または flags = 0 なら
                単にユーザプロシージャを無効にする。

  Arguments:    arc              ROMFAT型アーカイブ
                proc             ユーザプロシージャ
                flags            プロシージャへフックするコマンドのビット集合

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_SetArchiveProc(struct FSArchive *arc, FS_ARCHIVE_PROC_FUNC proc, u32 flags);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageTop

  Description:  ROMFAT型アーカイブからオープンしたファイルの
                先頭オフセットを取得

  Arguments:    file        ファイルハンドル

  Returns:      アーカイブ上でのファイル先頭オフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetFileImageTop(const struct FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileImageBottom

  Description:  ROMFAT型アーカイブからオープンしたファイルの
                終端オフセットを取得

  Arguments:    file        ファイルハンドル

  Returns:      アーカイブ上でのファイル終端オフセット
 *---------------------------------------------------------------------------*/
u32     FS_GetFileImageBottom(const struct FSFile *file);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_RFAT_H_ */
