/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     types.h

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


#ifndef NITRO_FS_TYPES_H_
#define NITRO_FS_TYPES_H_


#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/thread.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// FSライブラリAPIの返り値
typedef enum FSResult
{
    // FS_GetErrorCode関数で取得できる結果値
    FS_RESULT_SUCCESS = 0,
    FS_RESULT_FAILURE,
    FS_RESULT_BUSY,
    FS_RESULT_CANCELED,
    FS_RESULT_CANCELLED = FS_RESULT_CANCELED,
    FS_RESULT_UNSUPPORTED,
    FS_RESULT_ERROR,
    FS_RESULT_INVALID_PARAMETER,
    FS_RESULT_NO_MORE_RESOURCE,
    FS_RESULT_ALREADY_DONE,
    FS_RESULT_PERMISSION_DENIED,
    FS_RESULT_MEDIA_FATAL,
    FS_RESULT_NO_ENTRY,
    FS_RESULT_MEDIA_NOTHING,
    FS_RESULT_MEDIA_UNKNOWN,
    FS_RESULT_BAD_FORMAT,
    FS_RESULT_MAX,
    // プロシージャ内で使用する一時的な結果値
    FS_RESULT_PROC_ASYNC = 256,
    FS_RESULT_PROC_DEFAULT,
    FS_RESULT_PROC_UNKNOWN
}
FSResult;

// backward compatibility for typo
#define FS_RESULT_NO_MORE_RESOUCE   FS_RESULT_NO_MORE_RESOURCE

// アーカイブ名の最大長
#define FS_ARCHIVE_NAME_LEN_MAX     3

// ファイル名の最大長 (旧仕様のプロシージャに限定)
#define	FS_FILE_NAME_MAX	        127

// エントリ名の最大長 (旧仕様のプロシージャに限定)
#define	FS_ENTRY_SHORTNAME_MAX      16
#define	FS_ENTRY_LONGNAME_MAX	    260

// TWLで拡張された特殊アーカイブ名の最大長と完全パスの最大長
#define FS_ARCHIVE_NAME_LONG_MAX    15
#define FS_ARCHIVE_FULLPATH_MAX     (FS_ARCHIVE_NAME_LONG_MAX + 1 + 1 + FS_ENTRY_LONGNAME_MAX)

// ReadDirectoryで使用するエントリ属性
#define FS_ATTRIBUTE_IS_DIRECTORY   0x00000100UL
#define FS_ATTRIBUTE_IS_PROTECTED   0x00000200UL
#define FS_ATTRIBUTE_IS_OFFLINE     0x00000400UL
// MS-DOS FATベースのアーカイブに限り意味のある属性
#define FS_ATTRIBUTE_DOS_MASK       0x000000FFUL
#define FS_ATTRIBUTE_DOS_READONLY   0x00000001UL
#define FS_ATTRIBUTE_DOS_HIDDEN     0x00000002UL
#define FS_ATTRIBUTE_DOS_SYSTEM     0x00000004UL
#define FS_ATTRIBUTE_DOS_VOLUME     0x00000008UL
#define FS_ATTRIBUTE_DOS_DIRECTORY  0x00000010UL
#define FS_ATTRIBUTE_DOS_ARCHIVE    0x00000020UL

#define FS_PROPERTY_CTRL_SET_MTIME  0x01000000UL
#define FS_PROPERTY_CTRL_MASK       0x01000000UL

// SeekDirectory/OpenFileFastなどで使用する定数
#define FS_INVALID_FILE_ID          0xFFFFFFFFUL
#define FS_INVALID_DIRECTORY_ID     0xFFFFFFFFUL

// SeekDirectoryで使用する定数
#define FS_DIRECTORY_POSITION_HEAD  0x00000000UL

// SeekFileで使用するシーク起点
typedef enum FSSeekFileMode
{
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END
}
FSSeekFileMode;

// OpenFileで使用するアクセスモード
#define FS_FILEMODE_R               0x00000001UL
#define FS_FILEMODE_W               0x00000002UL
#define FS_FILEMODE_L               0x00000004UL
#define FS_FILEMODE_RW              (FS_FILEMODE_R | FS_FILEMODE_W)
#define FS_FILEMODE_RWL             (FS_FILEMODE_R | FS_FILEMODE_W | FS_FILEMODE_L)

// OpenDirectoryで使用するアクセスモード
#define FS_DIRMODE_SHORTNAME_ONLY   0x00001000UL

// CreateFileで使用する権限フラグ
#define FS_PERMIT_R                 0x00000001UL
#define FS_PERMIT_W                 0x00000002UL
#define FS_PERMIT_RW                (FS_PERMIT_R | FS_PERMIT_W)

// アーカイブ能力
#define FS_ARCHIVE_CAPS_UNICODE     0x00000001UL

//
// 以下の定数群はアーカイブを実装する場合のみ必要です
//

// ファイルコマンドおよびアーカイブメッセージ。
// アーカイブ実装時にユーザプロシージャで使用します。
typedef u32 FSCommandType;
// 旧仕様と互換性のあるコマンド
#define FS_COMMAND_READFILE             0UL
#define FS_COMMAND_WRITEFILE            1UL
#define FS_COMMAND_SEEKDIR              2UL
#define FS_COMMAND_READDIR              3UL
#define FS_COMMAND_FINDPATH             4UL
#define FS_COMMAND_GETPATH              5UL
#define FS_COMMAND_OPENFILEFAST         6UL
#define FS_COMMAND_OPENFILEDIRECT       7UL
#define FS_COMMAND_CLOSEFILE            8UL
#define FS_COMMAND_ACTIVATE             9UL
#define FS_COMMAND_IDLE                 10UL
#define FS_COMMAND_SUSPEND              11UL
#define FS_COMMAND_RESUME               12UL
// 旧仕様と互換性があるがコマンドではなかったもの
#define FS_COMMAND_OPENFILE             13UL
#define FS_COMMAND_SEEKFILE             14UL
#define FS_COMMAND_GETFILELENGTH        15UL
#define FS_COMMAND_GETFILEPOSITION      16UL
// 新仕様で拡張されたコマンド
#define FS_COMMAND_MOUNT                17UL
#define FS_COMMAND_UNMOUNT              18UL
#define FS_COMMAND_GETARCHIVECAPS       19UL
#define FS_COMMAND_CREATEFILE           20UL
#define FS_COMMAND_DELETEFILE           21UL
#define FS_COMMAND_RENAMEFILE           22UL
#define FS_COMMAND_GETPATHINFO          23UL
#define FS_COMMAND_SETPATHINFO          24UL
#define FS_COMMAND_CREATEDIRECTORY      25UL
#define FS_COMMAND_DELETEDIRECTORY      26UL
#define FS_COMMAND_RENAMEDIRECTORY      27UL
#define FS_COMMAND_GETARCHIVERESOURCE   28UL
// 29UL
#define FS_COMMAND_FLUSHFILE            30UL
#define FS_COMMAND_SETFILELENGTH        31UL
#define FS_COMMAND_OPENDIRECTORY        32UL
#define FS_COMMAND_CLOSEDIRECTORY       33UL
#define FS_COMMAND_SETSEEKCACHE         34UL
#define FS_COMMAND_MAX                  35UL
#define FS_COMMAND_INVALID              FS_COMMAND_MAX
#define FS_COMMAND_PROC_MAX             (FSCommandType)(FS_COMMAND_RESUME + 1)

// FSFile構造体で保持する情報サイズに起因した制限
SDK_COMPILER_ASSERT(FS_COMMAND_MAX < 256);

//
// 以下の定数群はライブラリ内部でのみ使用されます
//

// ファイルステータス (内部で使用)
#define FS_FILE_STATUS_BUSY                 0x00000001UL
#define FS_FILE_STATUS_CANCEL               0x00000002UL
#define FS_FILE_STATUS_BLOCKING             0x00000004UL
#define FS_FILE_STATUS_ASYNC_DONE           0x00000008UL
#define FS_FILE_STATUS_IS_FILE              0x00000010UL
#define FS_FILE_STATUS_IS_DIR               0x00000020UL
#define FS_FILE_STATUS_OPERATING            0x00000040UL
#define FS_FILE_STATUS_UNICODE_MODE         0x00000080UL
#define FS_FILE_STATUS_CMD_SHIFT            8UL
#define FS_FILE_STATUS_CMD_MASK             0x000000FFUL
#define	FS_FILE_STATUS_USER_RESERVED_BIT	0x00010000UL
#define	FS_FILE_STATUS_USER_RESERVED_MASK	0xFFFF0000UL

// アーカイブ内部状態フラグ (インライン関数用)
// ユーザはこれらの定数を直接使用することはありません。
#define FS_ARCHIVE_FLAG_REGISTER            0x00000001UL
#define FS_ARCHIVE_FLAG_LOADED              0x00000002UL
#define FS_ARCHIVE_FLAG_SUSPEND             0x00000008UL
#define FS_ARCHIVE_FLAG_RUNNING             0x00000010UL
#define FS_ARCHIVE_FLAG_CANCELING           0x00000020UL
#define FS_ARCHIVE_FLAG_SUSPENDING          0x00000040UL
#define FS_ARCHIVE_FLAG_UNLOADING           0x00000080UL
#define FS_ARCHIVE_FLAG_USER_RESERVED_BIT	0x00010000UL
#define FS_ARCHIVE_FLAG_USER_RESERVED_MASK	0xFFFF0000UL


/*---------------------------------------------------------------------------*/
/* declarations */

struct FSArchive;
struct FSFile;

// FSライブラリ全般で使用する時刻情報
typedef struct FSDateTime
{
    u32     year;    // 0-
    u32     month;   // 1-12
    u32     day;     // 1-31
    u32     hour;    // 0-23
    u32     minute;  // 0-59
    u32     second;  // 0-60
}
FSDateTime;

// ReadDirectoryで使用するエントリ情報
typedef struct FSDirectoryEntryInfo
{
    char        shortname[FS_ENTRY_SHORTNAME_MAX];
    u32         shortname_length;
    char        longname[FS_ENTRY_LONGNAME_MAX];
    u32         longname_length;
    u32         attributes;
    FSDateTime  atime;
    FSDateTime  mtime;
    FSDateTime  ctime;
    u32         filesize;
    u32         id;
}
FSDirectoryEntryInfo;

typedef struct FSDirectoryEntryInfoW
{
    u32         attributes;
    FSDateTime  atime;
    FSDateTime  mtime;
    FSDateTime  ctime;
    u32         filesize;
    u32         id;
    u32         shortname_length;
    u32         longname_length;
    char        shortname[FS_ENTRY_SHORTNAME_MAX];
    u16         longname[FS_ENTRY_LONGNAME_MAX];
}
FSDirectoryEntryInfoW;

// GetPathInfo,SetPathInfoで使用するエントリ情報
typedef struct FSPathInfo
{
    u32         attributes;
    FSDateTime  ctime;
    FSDateTime  mtime;
    FSDateTime  atime;
    u32         filesize;
    u32         id;
}
FSPathInfo;

typedef struct FSArchiveResource
{
    u64     totalSize;
    u64     availableSize;
    u32     maxFileHandles;
    u32     currentFileHandles;
    u32     maxDirectoryHandles;
    u32     currentDirectoryHandles;
    // for FAT archives.
    u32     bytesPerSector;
    u32     sectorsPerCluster;
    u32     totalClusters;
    u32     availableClusters;
}
FSArchiveResource;

SDK_COMPILER_ASSERT(sizeof(FSArchiveResource) == 48);

// for FS_TellDir(), FS_SeekDir(), FS_ReadDir()
typedef struct FSDirPos
{
// private:
    struct FSArchive   *arc;
    u16                 own_id;
    u16                 index;
    u32                 pos;
}
FSDirPos;

// for FS_OpenFileFast()
typedef struct FSFileID
{
// private:
    struct FSArchive   *arc;
    u32                 file_id;
}
FSFileID;

// for FS_ReadDir()
typedef struct
{
    union
    {
        FSFileID file_id;               // valid if !is_directory
        FSDirPos dir_id;                // valid if is_directory
    };
    u32     is_directory;               // directory ? 1 : 0
    u32     name_len;                   // strlen(name)
    char    name[FS_FILE_NAME_MAX + 1]; // string with '\0'
}
FSDirEntry;

/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_TYPES_H_ */
