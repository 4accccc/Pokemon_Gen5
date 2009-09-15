/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - sharedFont
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
#include <twl/na.h>
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
    OS_Printf("*** start sharedFont demo\n");

    PrintBootType();

    // 本体内蔵フォントを読み込みます。
    {
        BOOL bSuccess;
        int sizeTable;
        int sizeFont;
        void* pBufferTable;
        void* pBufferFont;

        // 本体内蔵フォント API を初期化します。
        bSuccess = NA_InitSharedFont();
        SDK_ASSERT( bSuccess );

        // 管理情報バッファサイズを取得
        sizeTable = NA_GetSharedFontTableSize();
        SDK_ASSERT( sizeTable >= 0 );
        OS_TPrintf("shared font table size = %d bytes\n", sizeTable);

        pBufferTable = OS_Alloc((u32)sizeTable);
        SDK_POINTER_ASSERT(pBufferTable);

        // 管理情報をロード
        bSuccess = NA_LoadSharedFontTable(pBufferTable);
        SDK_ASSERT( bSuccess );

        // フォントサイズを取得
        sizeFont = NA_GetSharedFontSize(NA_SHARED_FONT_WW_M);
        SDK_ASSERT( sizeFont >= 0 );
        OS_TPrintf("shared font(M) size = %d bytes\n", sizeFont);

        pBufferFont = OS_Alloc((u32)sizeFont);
        SDK_POINTER_ASSERT(pBufferFont);

        // フォントをロード
        bSuccess = NA_LoadSharedFont(NA_SHARED_FONT_WW_M, pBufferFont);
        SDK_ASSERT( bSuccess );

        OS_TPrintf("shared font loaded\n");

        /*
        ロードしたフォントを使用するには TWL-System が必要です...
        {
            NNSG2dFont font;

            NNS_G2dFontInitUTF16(&font, pBufferFont);
            ...
        }
        */
    }

    OS_Printf("*** End of demo\n");
    DEMO_DrawFlip();

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
