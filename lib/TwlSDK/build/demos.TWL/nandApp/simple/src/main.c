/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - simple
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

// OS_Printfのフック関数
#ifndef SDK_FINALROM
void OS_Printf(const char *fmt, ...)
{
    char dst[256];
    
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);

    OS_PutString(dst);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
    OS_Printf("*** start nandApp demo\n");

    PrintBootType();

    // ROM からファイルを読み込んで表示する
    // NAND アプリでも ROM ファイルシステムを使用することができます。
    {
        BOOL bSuccess;
        FSFile f;
        u32 fileSize;
        s32 readSize;
        void* pBuffer;

        FS_InitFile(&f);

        bSuccess = FS_OpenFileEx(&f, "build_time.txt", FS_FILEMODE_R);
        SDK_ASSERT( bSuccess );

        fileSize = FS_GetFileLength(&f);
        pBuffer = OS_Alloc(fileSize + 1);
        SDK_POINTER_ASSERT(pBuffer);

        readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
        SDK_ASSERT( readSize == fileSize );

        bSuccess = FS_CloseFile(&f);
        SDK_ASSERT( bSuccess );

        ((char*)pBuffer)[fileSize] = '\0';
        OS_Printf("%s\n", pBuffer);
        OS_Free(pBuffer);
    }

    OS_Printf("*** End of demo\n");

    // ランチャーに戻れるように、
    // 終了しない
    for (;;)
    {
        // フレーム更新。
        {
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
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
    case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
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
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
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

  Description:  FS を初期化して rom にアクセスできるようにします。
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
