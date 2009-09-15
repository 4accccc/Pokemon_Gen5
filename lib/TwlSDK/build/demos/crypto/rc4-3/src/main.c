/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     main.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  CRYPTO ライブラリ RC4 アルゴリズムを使用した攻撃耐性のある暗号化のデモ

  PRNG を使用して、毎回異なるランダムな IV (初期ベクタ) を使用することにより
  RC4 アルゴリズムによる暗号化を安全に使用します。
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>

#include "prng.h"
#include "rc4enc.h"

#define TEST_BUFFER_SIZE 256

static void InitializeAllocateSystem(void);
static void VBlankIntr(void);
static void DisplayInit(void);
static void PRNGInit(void);
static void FillScreen(u16 col);
static BOOL RC4EncText(void);
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
    OS_InitTick();
    InitializeAllocateSystem();

    DisplayInit();

    PRNGInit();

    if (RC4EncText())
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
    // PRNG の内部状態を現在のシステム状態で攪拌する。
    // PRNGInit() での種の初期化だけでは不十分のため、安全性を高めるために
    // 定期的な攪拌を行う。
    // 一回の呼び出しだけでは十分な攪拌が行われないので、必ず定期的に呼び出すこと。
    ChurnRandomPool();

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
  Name:         PRNGInit

  Description:  Pseudo Random Number Generator Initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PRNGInit(void)
{
    int i;
    for ( i = 0; i < 10; i++ )
    {
        ChurnRandomPool();
        OS_WaitVBlankIntr();
    }
    // 数回の呼び出しでは十分にエントロピーが集まらないため、
    // 定期的に ChurnRandomPool() を呼び出し続ける必要がある。

    // もし、より安全性を求めるのであれば、
    // 数キロバイト程度のマイク入力データを引数に
    // AddRandomSeed 関数を呼び出して PRNG の初期化を行う方法もある。
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
  Name:         RC4EncText

  Description:  RC4 アルゴリズムのテストルーチン

  Arguments:    None.

  Returns:      テストが成功したら true.
 *---------------------------------------------------------------------------*/
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }
#define PrintResultBinaryEq( a, b, l, f ) \
    { OS_TPrintf( (CompareBinary((a), (b), (l))) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && (CompareBinary((a), (b), (l))); }

static BOOL RC4EncText(void)
{
    int i;
    int testcase = 1;
    BOOL flag = TRUE;

    // RC4Enc でエンコードとデコードを行い、結果が一致するかを確認する
    {
        char* d[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",
            "Copyright 2006 Nintendo.  All rights reserved.",
        };
        char* key[] = {
            "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67",
            "Nintendo DS.",
        };

        for (i = 0; i < sizeof(d) / sizeof(char*); i++)
        {
            RC4EncoderContext enc_context;
            RC4DecoderContext dec_context;
            u8 result[TEST_BUFFER_SIZE];
            u8 result2[TEST_BUFFER_SIZE];
            u32 len;
            u32 encoded_len;
            u32 decoded_len;

            InitRC4Encoder(&enc_context, key[i]);
            len = GetStringLength(d[i]);
            encoded_len = EncodeRC4(&enc_context, d[i], len, result, sizeof(result));

            PrintResultEq(encoded_len, len + RC4ENC_ADDITIONAL_SIZE, flag);
            OS_TPrintf("RC4Enc: Test Case %d: encryption phase: length check\n", testcase);

            PrintBinary(result, encoded_len);
            OS_TPrintf("\n");

            InitRC4Decoder(&dec_context, key[i]);
            decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, sizeof(result2));

            PrintResultEq(decoded_len, len, flag);
            OS_TPrintf("RC4Enc: Test Case %d: decryption phase: length check\n", testcase);

            PrintResultBinaryEq(d[i], result2, len, flag);
            OS_TPrintf("RC4Enc: Test Case %d: decryption phase: binary comparing\n", testcase);

            testcase++;
        }
    }

    {
        char* d = "abcdefghijklmnopqrstuvwxyz";
        char* key = "123456789012";
        u8 result[TEST_BUFFER_SIZE];
        u8 result2[TEST_BUFFER_SIZE];
        u8 result_backup[TEST_BUFFER_SIZE];
        u32 len;
        u32 encoded_len;
        u32 decoded_len;
        RC4EncoderContext enc_context;
        RC4DecoderContext dec_context;

        // 以下、エンコード時のバッファサイズチェックの確認

        InitRC4Encoder(&enc_context, key);
        len = GetStringLength(d);

        encoded_len = EncodeRC4(&enc_context, d, len, result, 0);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, 1);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, len + RC4ENC_ADDITIONAL_SIZE - 1);
        PrintResultEq(encoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        encoded_len = EncodeRC4(&enc_context, d, len, result, len + RC4ENC_ADDITIONAL_SIZE);
        PrintResultEq(encoded_len, len + RC4ENC_ADDITIONAL_SIZE, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        // 以下、デコード時のバッファサイズチェックの確認

        InitRC4Decoder(&dec_context, key);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, 0);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, 1);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len - 1);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, len, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        // 以下、エンコードされたデータの一部を変更した場合、
        // デコード時にエラーになるかのチェック

        // エンコードデータのバックアップを作成
        MI_CpuCopy8(result, result_backup, encoded_len);

        result[0] ^= 1;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // バックアップから復元

        result[3] ^= 0xff;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // バックアップから復元

        result[4] ^= 1;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // バックアップから復元

        result[4+len] ^= 0x55;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);

        MI_CpuCopy8(result_backup, result, encoded_len); // バックアップから復元

        result[encoded_len-1] ^= 0x80;
        decoded_len = DecodeRC4(&dec_context, result, encoded_len, result2, len);
        PrintResultEq(decoded_len, 0, flag);
        OS_TPrintf("RC4Enc: Test Case %d\n", testcase++);
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
