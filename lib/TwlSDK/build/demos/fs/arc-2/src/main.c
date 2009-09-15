/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - demos - arc-2
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>


struct FixedDirectoryList;

// カスタムアーカイブのファイルイメージ構造体。
typedef struct FixedFile
{
    char    buffer[256];
}
FixedFile;

// カスタムアーカイブのディレクトリ構造体。
typedef struct FixedDirectory
{
    const char *name;
    union
    {
        void   *common_ptr;
        struct FixedDirectoryList *dir;
        FixedFile *file;
    }
    obj;
}
FixedDirectory;

// カスタムアーカイブのディレクトリリスト構造体。 (FNTと同等)
typedef struct FixedDirectoryList
{
    int     parent;
    FixedDirectory *own;
}
FixedDirectoryList;


// カスタムアーカイブのディレクトリ構成を定義。
extern FixedFile file_list[];
extern FixedDirectoryList dir_list[];
extern const int file_max;
extern const int dir_max;

static FixedFile file_list[] = {
    {
     "hello, world!\n" "fixed file of root.\n"},
    {
     "fixed file 1.\n"},
    {
     "fixed file 2.\n"},
    {
     "fixed file 3.\n"},
};
static FixedDirectory sub_dir1[] = {
    {"file1.txt", file_list + 1,},
    {"file2.txt", file_list + 2,},
    {"file3.txt", file_list + 3,},
    {NULL,},
};
static FixedDirectory sub_dir2[] = {
    {"test1.txt", file_list + 1,},
    {"test2.txt", file_list + 2,},
    {"test3.txt", file_list + 3,},
    {NULL,},
};
static FixedDirectory root_dir[] = {
    {"root.txt", file_list + 0,},
    {"sub1", dir_list + 1,},
    {"sub2", dir_list + 2,},
    {NULL,},
};
FixedDirectoryList dir_list[] = {
    {-1, root_dir,},
    {0, sub_dir1,},
    {0, sub_dir2,},
};
const int dir_max = sizeof(dir_list) / sizeof(*dir_list);
const int file_max = sizeof(file_list) / sizeof(*file_list);


// FSからアーカイブへのアクセスコールバック。
// 単にメモリを読み書きする。
static FSResult CustomRom_ReadCallback(FSArchive *arc, void *dst, u32 pos, u32 size)
{
    (void)arc;
    MI_CpuCopy8((const void *)pos, dst, size);
    return FS_RESULT_SUCCESS;
}
static FSResult CustomRom_WriteCallback(FSArchive *arc, const void *src, u32 pos, u32 size)
{
    (void)arc;
    MI_CpuCopy8(src, (void *)pos, size);
    return FS_RESULT_SUCCESS;
}

// カスタムアーカイブのユーザプロシージャ。
static FSResult CustomRom_ArchiveProc(FSFile *file, FSCommandType cmd)
{
    FSArchive *const p_rom = FS_GetAttachedArchive(file);

    switch (cmd)
    {

        // FSライブラリの仕様に従って下位層のコマンドだけをカスタマイズ。

    case FS_COMMAND_SEEKDIR:
        {
            const FSDirPos *const arg = &file->arg.seekdir.pos;
            const int dir_id = arg->own_id;
            file->prop.dir.pos = *arg;
            // indexとposが両方とも0ならディレクトリの先頭へシーク。
            if ((arg->index == 0) && (arg->pos == 0))
            {
                file->prop.dir.pos.index = 0; /* not use */
                file->prop.dir.pos.pos = (u32)dir_list[dir_id].own;
            }
            // ルートディレクトリの場合、parent-IDはディレクトリ総数を表す。
            file->prop.dir.parent = (u16)((dir_id != 0) ? dir_list[dir_id].parent : dir_max);
            return FS_RESULT_SUCCESS;
        }

    case FS_COMMAND_READDIR:
        {
            FSDirEntry *const p_entry = file->arg.readdir.p_entry;
            const FixedDirectory *const cur = (const FixedDirectory *)file->prop.dir.pos.pos;
            // ディレクトリの終端ではFS_RESULT_FAILUREを返す。
            if (!cur->name)
            {
                return FS_RESULT_FAILURE;
            }
            p_entry->name_len = (u32)STD_GetStringLength(cur->name);
            // skip_stringが指定されていないならエントリ名も返す。
            if (!file->arg.readdir.skip_string)
            {
                MI_CpuCopy8(cur->name, p_entry->name, p_entry->name_len + 1);
            }
            // エントリがディレクトリを指すならFSDirPosとして情報を返す。
            if ((cur->obj.dir >= dir_list) && (cur->obj.dir < dir_list + dir_max))
            {
                p_entry->is_directory = 1;
                p_entry->dir_id.arc = file->arc;
                p_entry->dir_id.own_id = (u16)(cur->obj.dir - dir_list);
                p_entry->dir_id.index = 0;
                p_entry->dir_id.pos = 0;
            }
            // エントリがファイルを指すならFSFileIDとして情報を返す。
            else
            {
                p_entry->is_directory = 0;
                p_entry->file_id.arc = file->arc;
                p_entry->file_id.file_id = (u32)(cur->obj.file - file_list);
            }
            // 正常にエントリを読み込めたら位置を１つ進める。
            file->prop.dir.pos.pos = (u32)(cur + 1);
            return FS_RESULT_SUCCESS;
        }

    case FS_COMMAND_OPENFILEFAST:
        {
            const int id = (int)file->arg.openfilefast.id.file_id;
            // 不正なファイルIDを指定されたらFS_RESULT_FAILUREを返す。
            if ((id < 0) || (id >= file_max))
            {
                return FS_RESULT_FAILURE;
            }
            else
            {
                // ファイルがリニアイメージならtopとbottomを適切に指定。
                const char *text = file_list[id].buffer;
                file->arg.openfiledirect.top = (u32)text;
                file->arg.openfiledirect.bottom = (u32)(text + STD_GetStringLength(text));
                file->arg.openfiledirect.index = (u32)id;
            }
            // FS_COMMAND_OPENFILEDIRECTの処理へ続く。
        }

    case FS_COMMAND_OPENFILEDIRECT:
        {
            // 呼び出し元から指定されたパラメータをそのまま設定。
            file->prop.file.top = file->arg.openfiledirect.top;
            file->prop.file.pos = file->arg.openfiledirect.top;
            file->prop.file.bottom = file->arg.openfiledirect.bottom;
            file->prop.file.own_id = file->arg.openfiledirect.index;
            return FS_RESULT_SUCCESS;
        }

        // 上位層のコマンドにFS_RESULT_PROC_UNKNOWNを返してデフォルト動作を使用すれば、
        // ここでカスタマイズした下位層のコマンドが必要に応じて呼び出される。

    default:
    case FS_COMMAND_FINDPATH:
    case FS_COMMAND_GETPATH:
    case FS_COMMAND_CLOSEFILE:
    case FS_COMMAND_READFILE:
    case FS_COMMAND_WRITEFILE:
    case FS_COMMAND_ACTIVATE:
    case FS_COMMAND_IDLE:
        return FS_RESULT_PROC_UNKNOWN;

    }

}

// カスタムアーカイブの初期化。
static void CreateCustomArchive(FSArchive *arc, const char *name)
{
    FS_InitArchive(arc);
    if (!FS_RegisterArchiveName(arc, name, (u32)STD_GetStringLength(name)))
    {
        OS_TPanic("error! FS_RegisterArchiveName(%s) failed.\n", name);
    }
    FS_SetArchiveProc(arc, CustomRom_ArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
    // デフォルト動作でFATとFNTが必要になるコマンドは全てカスタマイズしたので、
    // ここでは特に指定しなくてもよい。
    if (!FS_LoadArchive(arc, 0, NULL, 0, NULL, 0, CustomRom_ReadCallback, CustomRom_WriteCallback))
    {
        OS_TPanic("error! FS_LoadArchive() failed.\n");
    }
}

// ROMアーカイブのディレクトリを列挙。
static void DumpRomDirectorySub(int tab, FSDirEntry *pe)
{
    FSFile  d;
    FS_InitFile(&d);
    OS_TPrintf("%*s%s/\n", tab, "", pe->name);
    if (FS_SeekDir(&d, &pe->dir_id))
    {
        tab += 4;
        while (FS_ReadDir(&d, pe))
        {
            if (pe->is_directory)
            {
                DumpRomDirectorySub(tab, pe);
            }
            else
            {
                OS_Printf("%*s%s\n", tab, "", pe->name);
            }
        }
    }
}
static void DumpRomDir(const char *path)
{
    FSDirEntry  entry;
    FSFile      dir;
    FS_InitFile(&dir);
    (void)FS_ChangeDir(path);
    (void)FS_FindDir(&dir, "");
    entry.name[0] = '\0';
    (void)FS_TellDir(&dir, &entry.dir_id);
    DumpRomDirectorySub(0, &entry);
}


void NitroMain(void)
{
    static FSArchive custom_rom;

    // OSとメモリアロケータを初期化。
    OS_Init();
    {
        OSHeapHandle hh;
        void   *tmp;
        tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
        OS_SetArenaLo(OS_ARENA_MAIN, tmp);
        hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
        if (hh < 0)
        {
            OS_TPanic("ARM9: Fail to create heap...\n");
        }
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
    }
    (void)OS_EnableIrq();
    // FSを初期化。
    FS_Init(3);
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

    OS_TPrintf("\n"
               "++++++++++++++++++++++++++++++++++++++++\n"
               "test 1 : query default \"rom\" directories ... \n\n");

    DumpRomDir("rom:/");


    OS_TPrintf("\n"
               "++++++++++++++++++++++++++++++++++++++++\n"
               "test 2 : query custom archive \"cst\" directories ... \n\n");

    CreateCustomArchive(&custom_rom, "cst");
    DumpRomDir("cst:/");

    OS_TPrintf("\n" "++++++++++++++++++++++++++++++++++++++++\n" "end\n\n");
    OS_Terminate();
}
