/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     main.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  CRYPTO ライブラリ RC4 アルゴリズムの動作確認デモ
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>

#define TEST_DATA_MAX_SIZE 256

static void InitializeAllocateSystem(void);
static void VBlankIntr(void);
static void DisplayInit(void);
static void FillScreen(u16 col);
static BOOL RC4Test(void);
static BOOL CompareBinary(void* ptr1, void* ptr2, u32 n);
static u32 GetStringLength(char* str);
static void PrintBinary(u8* ptr, u32 len);

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // 各種初期化
    OS_Init();
    InitializeAllocateSystem();

    DisplayInit();

    if (RC4Test())
    {
        // 成功
        OS_TPrintf("------ Test Succeeded ------\n");
        FillScreen(GX_RGB(0, 31, 0));
    }
    else
    {
        // 失敗
        OS_TPrintf("****** Test Failed ******\n");
        FillScreen(GX_RGB(31, 0, 0));
    }

    // メインループ
    while (TRUE)
    {
        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // IRQ チェックフラグをセット
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics Initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit(void)
{

    GX_Init();
    FX_Init();

    GX_DispOff();
    GXS_DispOff();

    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);         // to generate VBlank interrupt request
    (void)OS_EnableIrq();


    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // clear the standard palette
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A,B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode.
                       (GXBGMode)0,    // dummy
                       (GXBG0As)0);    // dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // make OBJ visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();              // Waiting the end of VBlank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         FillScreen

  Description:  画面を塗りつぶす

  Arguments:    col - FillColor.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FillScreen(u16 col)
{
    MI_CpuFill16((void *)HW_LCDC_VRAM_C, col, 256 * 192 * 2);
}

/*---------------------------------------------------------------------------*
  Name:         RC4Test

  Description:  RC4 アルゴリズムのテストルーチン

  Arguments:    None.

  Returns:      テストが成功したら true.
 *---------------------------------------------------------------------------*/
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }
#define PrintResultBinaryEq( a, b, l, f ) \
    { OS_TPrintf( (CompareBinary((a), (b), (l))) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && (CompareBinary((a), (b), (l))); }

static BOOL RC4Test(void)
{
    int i;
    BOOL flag = TRUE;

    // RC4 の動作テストを行う
    {
        char* d[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Copyright 2003-2005 Nintendo.  All rights reserved.",
        };
        char* key[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Nintendo DS Software Development Kit: Wi-Fi Library", // 先頭 16 バイトのみ使用される
        };
        u8 result_rc4[][TEST_DATA_MAX_SIZE] = {
            {
                0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96
            },
            {
                0xc6, 0x34, 0x75, 0x38, 0x0a, 0xea, 0x34, 0xe5, 0xda, 0x43, 0xb7, 0xb6, 0x07, 0x55, 0x24, 0xa8,
                0x07, 0x38, 0x96, 0x21, 0x41, 0x03, 0x8e, 0xc8, 0xbc, 0x6c, 0x0e, 0xfc, 0x2a, 0x10, 0xf0, 0xa9,
                0xee, 0x81, 0x05, 0x53, 0x60, 0xc7, 0xd7, 0xaf, 0x6a, 0x7a, 0x82, 0x6a, 0x05, 0x1f, 0x4a, 0x8c,
                0x85, 0xd4, 0xc1
            },
        };

        for (i = 0; i < sizeof(d) / sizeof(char*); i++)
        {
            CRYPTORC4Context context;
            u8 result[TEST_DATA_MAX_SIZE];
            u32 len;

            CRYPTO_RC4Init(&context, key[i], GetStringLength(key[i]));
            len = GetStringLength(d[i]);
            CRYPTO_RC4Encrypt(&context, d[i], len, result);
            PrintResultBinaryEq(result, result_rc4[i], len, flag);
            OS_TPrintf("CRYPTO_RC4: Test Case %d: encryption phase\n", i+1);

//            PrintBinary(result, len);
//            OS_TPrintf("\n");

            CRYPTO_RC4(key[i], GetStringLength(key[i]), result, len);
            PrintResultBinaryEq(d[i], result, len, flag);
            OS_TPrintf("CRYPTO_RC4: Test Case %d: decryption phase\n", i+1);
        }
    }

    {
        char* d[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Copyright 2003-2005 Nintendo.  All rights reserved.",
        };
        char* key[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Nintendo DS Software Development Kit: Wi-Fi Library", // 先頭 16 バイトのみ使用される
        };
        u8 result_rc4[][TEST_DATA_MAX_SIZE] = {
            {
                0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96
            },
            {
                0xc6, 0x34, 0x75, 0x38, 0x0a, 0xea, 0x34, 0xe5, 0xda, 0x43, 0xb7, 0xb6, 0x07, 0x55, 0x24, 0xa8,
                0x07, 0x38, 0x96, 0x21, 0x41, 0x03, 0x8e, 0xc8, 0xbc, 0x6c, 0x0e, 0xfc, 0x2a, 0x10, 0xf0, 0xa9,
                0xee, 0x81, 0x05, 0x53, 0x60, 0xc7, 0xd7, 0xaf, 0x6a, 0x7a, 0x82, 0x6a, 0x05, 0x1f, 0x4a, 0x8c,
                0x85, 0xd4, 0xc1
            },
        };

        for (i = 0; i < sizeof(d) / sizeof(char*); i++)
        {
            CRYPTORC4FastContext context;
            u8 result[TEST_DATA_MAX_SIZE];
            u32 len;

            CRYPTO_RC4FastInit(&context, key[i], GetStringLength(key[i]));
            len = GetStringLength(d[i]);
            CRYPTO_RC4FastEncrypt(&context, d[i], len, result);
            PrintResultBinaryEq(result, result_rc4[i], len, flag);
            OS_TPrintf("CRYPTO_RC4Fast: Test Case %d: encryption phase\n", i+1);

//            PrintBinary(result, len);
//            OS_TPrintf("\n");

            CRYPTO_RC4Fast(key[i], GetStringLength(key[i]), result, len);
            PrintResultBinaryEq(d[i], result, len, flag);
            OS_TPrintf("CRYPTO_RC4Fast: Test Case %d: decryption phase\n", i+1);
        }
    }





    return flag;
}


/*---------------------------------------------------------------------------*
  Name:         CompareBinary

  Description:  メモリ内容比較

  Arguments:    ptr1, ptr2 - 比較するメモリ位置.
                n - 比較する長さ

  Returns:      一致したら TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CompareBinary(void* ptr1, void* ptr2, u32 n)
{
    u32 i;
    u8* p1 = (u8*)ptr1;
    u8* p2 = (u8*)ptr2;

    for (i = 0; i < n; i++)
    {
        if (*(p1++) != *(p2++))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GetStringLength

  Description:  文字列長を得る

  Arguments:    str - 文字列へのポインタ.

  Returns:      文字列長.
 *---------------------------------------------------------------------------*/
static u32 GetStringLength(char* str)
{
    u32 i;
    for (i = 0; ; i++)
    {
        if (*(str++) == '\0')
        {
            return i;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintBinary

  Description:  バイナリ列出力

  Arguments:    ptr - 出力するメモリ位置.
                len - 出力する長さ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBinary(u8* ptr, u32 len)
{
#ifndef SDK_FINALROM
    u32 i;
    for (i = 0; i < len; i++)
    {
        if (i != 0)
        {
            OS_PutString(", ");
        }
        OS_TPrintf("0x%02x", ptr[i]);
    }
#else
#pragma unused(ptr,len)
#endif
    return;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
