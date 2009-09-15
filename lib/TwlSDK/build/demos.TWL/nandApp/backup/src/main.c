/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - backup
  File:     main.c

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
#include <twl.h>
#include <DEMO.h>

static void PrintBootType();
static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

static char* LoadFile(const char* path);
static BOOL SaveFile(const char* path, void* pData, u32 size);

static void PrintDirectory(const char* path);
static void CreateTree(const char* arc_name);
static void DeleteData(char *path);
static BOOL WriteData(const char *path, void* pData, u32 size);
static void ReadData(const char* arc_name);

static char* GetTestData(char *out, u32 size);
static void DrawString(const char *fmt, ...);
static void PrintTree(const char* path, u32 space);

static const u32 BUF_SIZE = 256;

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    RTC_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
    DrawString("*** start nandApp demo\n");

    DrawString("Build:%s %s\n", __DATE__, __TIME__);
    PrintBootType();
    DrawString("\n");
    DrawString("A - Delete, Create, Read\n");
    DrawString("B - Print Directory Tree\n");


    // NAND アプリのバックアップデータは NAND 上に保存されます。
    // そのため使用する API は CARD API ではなく FS API となり、
    // ファイルシステムとしてアクセスすることができます。
    //
    // 本プログラムでは
    // 1.ファイルの読み込み 
    // 2.バックアップデータの消去 
    // 3.ディレクトリツリー・ファイルの作成
    // の例を示します。 
    
    // 「dataPub」は SD カードにコピーされる
    // バックアップデータ領域のアーカイブ名です。
    // 一般的なデータはこちらを使用します。

    // 「dataPrv」は SD カードにコピーされない
    // バックアップデータ領域のアーカイブ名です。
    // コピーされたくないデータを保存するのに使用します。
    {
        u32 mode = 0;
        // HWリセットを有効にするため終了しない
        for (;;)
        {
            // フレーム更新
            DEMOReadKey();
            if(DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                // 画面のクリア
                DrawString("");
                DEMOFillRect(0, 0, 256, 192, GX_RGBA(0, 0, 0, 1));
                switch(mode)
                {
                    case 0: // ファイル読み込みの実行 
                        DrawString("Read\n");
                        ReadData("dataPub:");
                        ReadData("dataPrv:");
                        break;
                    case 1: // 消去の実行 
                        DrawString("Delete\n");
                        DeleteData("dataPub:");
                        DeleteData("dataPrv:");
                        break;
                    case 2: // ディレクトリ・ファイル作成の実行 
                        DrawString("Create\n");
                        CreateTree("dataPub:");
                        CreateTree("dataPrv:");
                        break;
                }
                mode = (mode + 1) % 3;
            }else if(DEMO_IS_TRIG(PAD_BUTTON_B))
            {
                // ディレクトリツリーの表示
                DrawString("");
                DEMOFillRect(0, 0, 256, 192, GX_RGBA(0, 0, 0, 1));
                DrawString("Tree\ndataPub:\n");
                PrintTree("dataPub:", 1);
                DrawString("\ndataPrv:\n");
                PrintTree("dataPrv:", 1);
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  内部でメモリを確保しファイルを読み込みます。

  Arguments:    path:   読み込むファイルのパス。

  Returns:      ファイルが存在するならファイルの内容が読み込まれた
                内部で確保したバッファへのポインタを返します。
                このポインタは FS_Free で解放する必要があります。
 *---------------------------------------------------------------------------*/
static char* LoadFile(const char* path)
{
    FSFile f;
    BOOL bSuccess;
    char* pBuffer;
    u32 fileSize;
    s32 readSize;

    FS_InitFile(&f);

    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);
    if( ! bSuccess )
    {
        return NULL;
    }

    fileSize = FS_GetFileLength(&f);
    pBuffer = (char*)OS_Alloc(fileSize + 1);
    SDK_POINTER_ASSERT(pBuffer);

    readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
    SDK_ASSERT( readSize == fileSize );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );

    pBuffer[fileSize] = '\0';
    return pBuffer;
}

/*---------------------------------------------------------------------------*
  Name:         SaveFile

  Description:  ファイルを作成しデータを書き込みます。
                途中のディレクトリは作成しません。

  Arguments:    path:   作成するファイルのパス。
                pData:  書き込むデータ。
                size:   書き込むデータのサイズ。

  Returns:      成功すればTRUE
 *---------------------------------------------------------------------------*/
static BOOL SaveFile(const char* path, void* pData, u32 size)
{
    FSFile f;
    BOOL bSuccess;
    FSResult fsResult;
    s32 writtenSize;

    FS_InitFile(&f);

    (void)FS_CreateFile(path, (FS_PERMIT_R|FS_PERMIT_W));
    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_W);
    if (bSuccess == FALSE)
    {
        FSResult res = FS_GetArchiveResultCode(path);
        DrawString("Failed create file:%d\n", res);
        return FALSE;
    }
    SDK_ASSERT( bSuccess );

    fsResult = FS_SetFileLength(&f, 0);
    SDK_ASSERT( fsResult == FS_RESULT_SUCCESS );

    writtenSize = FS_WriteFile(&f, pData, (s32)size);
    SDK_ASSERT( writtenSize == size );

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );
    return TRUE;
}




/*---------------------------------------------------------------------------*
  Name:         CreateTree

  Description:  ディレクトリツリーを作成する 
                
  Arguments:    arc_name: 対象アーカイブ。 

  Returns:      なし。 
 *---------------------------------------------------------------------------*/
static void CreateTree(const char* arc_name){
    char *dir_path[] = {
        "/",
        "/testDir/", 
        "/testDir2/test/", 
    };
    char *filename = "test";
    
    u32 PATH_COUNT = 3;
    u32 FILE_COUNT = 2;
    char buf[BUF_SIZE];
    BOOL bSuccess;

    
    DrawString("Create:%s\n", arc_name);
    // ディレクトリの生成
    {
        u32 i = 0, j = 0;
        for(i = 1; i < PATH_COUNT; ++i)
        {
            (void)STD_TSNPrintf(buf, BUF_SIZE, "%s%s", arc_name, dir_path[i]);

            bSuccess = FS_CreateDirectoryAuto(buf, FS_PERMIT_W | FS_PERMIT_R);
            if(!bSuccess)
            {
                DrawString("Failed Create Directory\n");
                return;
            }
            DrawString("  %s\n", buf);
        }
    }

    // ファイル生成
    {
        u32 i = 0, j = 0;
        for(i = 0; i < PATH_COUNT; ++i)
        {
            for(j = 0; j < FILE_COUNT; ++j)
            {
                char data[BUF_SIZE];
                (void)STD_TSNPrintf(buf, BUF_SIZE, "%s%s%s%d", arc_name, dir_path[i], filename, j);
                (void)SaveFile(buf, GetTestData(data, BUF_SIZE), BUF_SIZE);
                DrawString("  %s\n", buf);
            }
        }
    }
    DrawString("\n");    
}


/*---------------------------------------------------------------------------*
  Name:         DeleteData

  Description:  ディレクトリ内のファイル・ディレクトリを全て削除する
                
  Arguments:    path: 対象ディレクトリのパス。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DeleteData(char *path)
{
    FSFile fp;
    FSDirectoryEntryInfo entry;
    char buf[BUF_SIZE];
    BOOL bSuccess;
    BOOL bDeleted = FALSE;
    
    DrawString("Delete:%s\n", path);
    FS_InitFile(&fp);
    bSuccess = FS_OpenDirectory(&fp, path, FS_PERMIT_W | FS_PERMIT_R);
    if(!bSuccess)
    {
        DrawString("Failed Open Directory\n");
        return;
    }

    while(FS_ReadDirectory(&fp, &entry))
    {
        if(!STD_StrCmp(".", entry.longname) || !STD_StrCmp("..", entry.longname))
            continue;

        (void)STD_TSNPrintf(buf, BUF_SIZE, "%s/%s", path, entry.longname);
        if(entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY)
        {
            // ディレクトリの場合
            bSuccess = FS_DeleteDirectoryAuto(buf);
        }
        else
        {
            // ファイルの場合
            bSuccess = FS_DeleteFile(buf);
        }
        if(!bSuccess)
        {
            DrawString("Failed Delete %s\n", buf);
            continue;
        }
        else
        {
            DrawString("  %s\n", buf);
            bDeleted = TRUE;
        }
    }
    
    if(!bDeleted)
    {
        DrawString("No File\n");
    }
    
    bSuccess = FS_CloseDirectory(&fp);
    SDK_ASSERT(bSuccess);
    DrawString("\n");
}

/*---------------------------------------------------------------------------*
  Name:         GetTestData

  Description:  テストファイルに書き込むデータの作成。
                
  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static char* GetTestData(char* out, u32 size)
{
    RTCResult rtcResult;
    RTCDate date;
    RTCTime time;

    // ファイルに書き込む内容を生成します。
    rtcResult = RTC_GetDateTime(&date, &time);
    SDK_ASSERT( rtcResult == RTC_RESULT_SUCCESS );

    (void)STD_TSNPrintf(out, size,
        "Hello. %04d/%02d/%02d %02d:%02d:%02d\n",
        date.year + 2000,
        date.month,
        date.day,
        time.hour,
        time.minute,
        time.second );
    
    return out;
}

/*---------------------------------------------------------------------------*
  Name:         ReadData

  Description:  ディレクトリ内を走査し、最初に見つかったファイルの内容を表示する
                
  Arguments:    path: 対象ディレクトリのパス。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void ReadData(const char* arc_name)
{
    FSFile fp;
    FSDirectoryEntryInfo entry;
    
    char buf[BUF_SIZE];
    BOOL bSuccess;
    BOOL bRead = FALSE;
    
    DrawString("Read:%s\n", arc_name);
    // ディレクトリのオープン
    FS_InitFile(&fp);
    bSuccess = FS_OpenDirectory(&fp, arc_name, FS_PERMIT_W | FS_PERMIT_R);
    if(!bSuccess)
    {
        DrawString("Failed Open Directory\n");
        return;
    }    

    // ディレクトリを操作して、最初に見つかったファイルの内容を表示
    while(FS_ReadDirectory(&fp, &entry))
    {
        if(!STD_StrCmp(".", entry.longname) || !STD_StrCmp("..", entry.longname))
            continue;

        (void)STD_TSNPrintf(buf, BUF_SIZE, "%s/%s", arc_name, entry.longname);
        if(!(entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY))
        {
            // ファイルの場合
            char *data = NULL;
            data = LoadFile(buf);
            SDK_POINTER_ASSERT(buf);
            
            DrawString("%s\n%s\n", buf, data);
            OS_Free(data);
            bRead = TRUE;
            break;
        }
    }
    if(!bRead)
    {
        DrawString("No File\n");
    }
    
    bSuccess = FS_CloseDirectory(&fp);
    SDK_ASSERT(bSuccess);
    DrawString("\n");
}

/*---------------------------------------------------------------------------*
  Name:         DrawString

  Description:  DEMODrawStringを使った描画。
                
  Arguments:    fmt: DEMODrawStringに準拠。
                     ただし、fmtが""の場合、カーソル位置を0,0に戻す。
  
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DrawString(const char* fmt, ...)
{
    static s32 x = 0, y = 0;
    char dst[256];
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);

    if(fmt[0] == '\0')
    {
        x = y = 0;
        return;
    }    
    DEMODrawText(x, y, dst);
    {
        s32 i, max = STD_StrLen(dst) - 1;
        u32 cr = 0;
        for(i = max; i >= 0; --i)
        {
            if(dst[i] == '\n')
            {
                x = (cr == 0) ? (max - i) * 8 : x;
                cr++;
            }
        }
        y += cr * 8;
    }
    
}


/*---------------------------------------------------------------------------*
  Name:         PrintTree

  Description:  ディレクトリツリーを表示する
                
  Arguments:    path: ルートのパス。
                space: 再帰呼び出し用。 通常は0を指定。 
  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintTree(const char* path, u32 space)
{
    FSFile f;
    FSDirectoryEntryInfo entry;
    BOOL bSuccess;
    char buf[BUF_SIZE];

    FS_InitFile(&f);
    bSuccess = FS_OpenDirectory(&f, path, FS_PERMIT_R);
    if(!bSuccess)
    {
        DrawString("Failed Open Directory\n");
        return;
    }    
    while( FS_ReadDirectory(&f, &entry) )
    {
        MI_CpuFill8(buf, ' ', space);
        buf[space] = '\0';
        // 自分や親を指すエントリはスキップします
        if( (STD_StrCmp(entry.longname, ".")  == 0)
         || (STD_StrCmp(entry.longname, "..") == 0) )
        {
             continue;
        }
        if( (entry.attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0 )
        {
            // ディレクトリ
            (void)STD_StrCat(buf, entry.longname);
            DrawString("%s\n", buf);
            
            // パスを合成して再帰呼び出し
            (void)STD_StrCpy(buf, path);
            (void)STD_StrCat(buf, "/");
            (void)STD_StrCat(buf, entry.longname);
            PrintTree(buf, space + 1);
        }
        else
        {
            // ファイル
            (void)STD_StrCat(buf, entry.longname);
            DrawString("%s\n", buf);
        }
    }
    bSuccess = FS_CloseDirectory(&f);
    SDK_ASSERT( bSuccess );
}

/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  BootType を print します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   DrawString("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  DrawString("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  コンソールの画面出力用の表示設定を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // 画面表示の初期化。
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  割り込みを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // マスター割り込みフラグを許可に
    (void)OS_EnableIrq();

    // IRQ 割り込みを許可します
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  ヒープを作成して OS_Alloc が使えるようにします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // メインアリーナのアロケートシステムを初期化
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // メインアリーナ上にヒープを作成
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  FS を初期化します。
                この関数を呼び出す前に InitInteruptSystem() が
                呼ばれている必要があります。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // ファイルシステム初期化
    FS_Init( FS_DMA_NOT_USE );
}
