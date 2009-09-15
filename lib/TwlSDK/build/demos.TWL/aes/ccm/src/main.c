/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ccm
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9567 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/aes.h>


/*---------------------------------------------------------------------------*
    内部変数
 *---------------------------------------------------------------------------*/

typedef struct DEMOSizeData
{
    u32             aSize;
    u32             pSize;
    AESMacLength    macLength;
}
DEMOSizeData;


// AES 処理の終了を待ち受けるためのメッセージキュー
#define DEMO_MESSAGE_Q_SIZE     1
static OSMessageQueue   sMsgQ;
static OSMessage        sMsgQBuffer[DEMO_MESSAGE_Q_SIZE];

// 暗号化するサンプルのデータ
static const char DEMO_TEXT[] =
    "These coded instructions, statements, and computer programs contain "
    "proprietary information of Nintendo of America Inc. and/or Nintendo "
    "Company Ltd., and are protected by Federal copyright law.  They may "
    "not be disclosed to third parties or copied or duplicated in any form, "
    "in whole or in part, without the prior written consent of Nintendo.";

// 暗号化に使用する適当な鍵
static const AESKey DEMO_KEY =
    { 0xb3,0x2f,0x3a,0x91,0xe6,0x98,0xc2,0x76,
      0x70,0x6d,0xfd,0x71,0xbc,0xdd,0xb3,0x93, };



/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void         DEMO_InitInteruptSystem(void);
static void         DEMO_InitAllocSystem(void);
static void         DEMO_AESCallback(AESResult result, void* arg);
static AESResult    DEMO_WaitAes(OSMessageQueue* pQ);
static void         DEMO_PrintBytes(const void* pvoid, u32 size);



/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  サンプルの本体です。

  Arguments:    なし。

  Returns:      サンプルの処理が成功したか否か。
 *---------------------------------------------------------------------------*/
static BOOL SampleMain(void)
{
    const AESMacLength macLength    = AES_MAC_LENGTH_16;
    BOOL bSuccess = FALSE;

    AESResult aesResult;
    AESNonce nonce;
    void* pPlainText;
    void* pEncrypted;
    void* pDecrypted;
    void* pPlainTextA;
    void* pPlainTextP;
    void* pEncryptedA;
    void* pEncryptedC;
    void* pEncryptedM;
    u32 srcASize;
    u32 srcPSize;
    u32 macSize;
    DEMOSizeData* pSizeData;

    // AES 処理の待ち受けのためのメッセージキューを初期化します。
    OS_InitMessageQueue(&sMsgQ, sMsgQBuffer, DEMO_MESSAGE_Q_SIZE);

    // サイズの計算
    // srcASize は AES_BLOCK_SIZE の倍数でなければなりません。
    srcASize    = MATH_ROUNDUP(sizeof(DEMOSizeData), AES_BLOCK_SIZE);
    srcPSize    = (u32)STD_GetStringLength(DEMO_TEXT) + 1;
    macSize     = AES_GetMacLengthValue(macLength);

    // メモリの確保とポインタの計算
    // 暗号化/復号化ともに Pdata と Adata は一続きの
    // バッファに格納されている必要があります。
    pPlainText  = OS_Alloc(srcASize + srcPSize);
    pEncrypted  = OS_Alloc(srcASize + srcPSize + macSize);
    pDecrypted  = OS_Alloc(srcPSize);

    if( ! SDK_IS_VALID_POINTER(pPlainText)
     || ! SDK_IS_VALID_POINTER(pEncrypted)
     || ! SDK_IS_VALID_POINTER(pDecrypted) )
    {
        OS_TPrintf("fail to OS_Alloc\n");
        return FALSE;
    }

    pPlainTextA = pPlainText;
    pPlainTextP = (u8*)pPlainTextA + srcASize;
    pEncryptedA = pEncrypted;
    pEncryptedC = (u8*)pEncryptedA + srcASize;
    pEncryptedM = (u8*)pEncryptedC + srcPSize;

    // このサンプルでは Adata として各領域のサイズを格納します。
    pSizeData = (DEMOSizeData*)pPlainTextA;
    pSizeData->aSize        = srcASize;
    pSizeData->pSize        = srcPSize;
    pSizeData->macLength    = macLength;
    MI_CpuClear8(pSizeData + 1, srcASize - sizeof(DEMOSizeData));

    // Adata と一続きのバッファに Pdata をコピー
    MI_CpuCopy8(DEMO_TEXT, (u8*)pPlainText + srcASize, srcPSize);

    // 復号化用に暗号化用の Adata をコピーします。
    // 暗号化処理で出力されるのは Pdata を暗号化した Cdata のみです。
    // 復号化時に正当性検査を行うには別途 Adata が必要なためコピーしています。
    MI_CpuCopy8(pPlainText, pEncrypted, srcASize);


    // AES API の最初に AES_Init を呼び出す必要があります。
    AES_Init();

    // nonce を生成するために AES_Rand を使用します。
    // nonce は任意の数値でかまいませんが、
    // できるだけ同じ値を使わないようにしなければなりません。
    aesResult = AES_Rand(&nonce, sizeof(nonce));
    if( aesResult != AES_RESULT_SUCCESS )
    {
        OS_TPrintf("AES_Rand failed(%d)\n", aesResult);
        goto error_exit;
    }

    // 暗号の鍵を設定します。
    aesResult = AES_SetKey(&DEMO_KEY);
    if( aesResult != AES_RESULT_SUCCESS )
    {
        OS_TPrintf("AES_SetKey failed(%d)\n", aesResult);
        goto error_exit;
    }


    // パラメータの表示
    OS_TPrintf("---- parameter -----------\n");
    OS_TPrintf("-- key\n");
    DEMO_PrintBytes(&DEMO_KEY, sizeof(DEMO_KEY));
    OS_TPrintf("-- nonce\n");
    DEMO_PrintBytes(&nonce, sizeof(nonce));


    // 暗号化
    {
        // AES CCM モードでの暗号化を行います。
        aesResult = AES_CcmEncryptAndSign(
                        &nonce,             // nonce
                        pPlainText,         // Adata と Pdata を一続きに配置したバッファ
                        srcASize,           // Adata のサイズ
                        srcPSize,           // Pdata のサイズ
                        macLength,          // MAC のサイズ
                        pEncryptedC,        // 暗号化したデータを格納するバッファ
                        DEMO_AESCallback,   // 処理終了時に呼び出されるコールバック
                        &sMsgQ );           // コールバックパラメータ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmEncryptAndSign failed(%d)\n", aesResult);
            goto error_exit;
        }

        // 暗号化処理が終了するのを待ちます。
        aesResult = DEMO_WaitAes(&sMsgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmEncryptAndSign failed(%d)\n", aesResult);
            goto error_exit;
        }


        // 結果の表示
        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- plain text (ascii)\n");
        OS_PutString(pPlainTextP);   // OS_*Printf には 256 文字制限があるため OS_PutString を使用
        OS_TPrintf("\n");

        OS_TPrintf("-- plain text (hex)\n");
        DEMO_PrintBytes(pPlainText, srcASize + srcPSize);

        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcASize + srcPSize + macSize);
    }

    // 復号化
    {
        // AES CCM モードでの復号化を行います。
        aesResult = AES_CcmDecryptAndVerify(
                        &nonce,             // nonce（暗号化時と同じでなければなりません）
                        pEncrypted,         // Adata と Cdata を一続きに配置したバッファ
                        srcASize,           // Adata のサイズ
                        srcPSize,           // Cdata のサイズ
                        macLength,          // MAC のサイズ
                        pDecrypted,         // 復号化したデータを格納するバッファ
                        DEMO_AESCallback,   // 処理終了時に呼び出されるコールバック
                        &sMsgQ );           // コールバックパラメータ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmDecryptAndVerify failed(%d)\n", aesResult);
            goto error_exit;
        }

        // 復号化処理が終了するのを待ちます。
        aesResult = DEMO_WaitAes(&sMsgQ);

        // 暗号化したデータが改竄されていると
        // ここで得られる aesResult が AES_RESULT_VERIFICATION_FAILED になります。
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CcmDecryptAndVerify failed(%d)\n", aesResult);
            goto error_exit;
        }


        // 結果の表示
        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcASize + srcPSize + macSize);

        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, srcPSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        OS_PutString(pDecrypted);   // OS_*Printf には 256 文字制限があるため OS_PutString を使用
        OS_TPrintf("\n");
    }

    // 暗号化して復号化したものがオリジナルと一致することを確認
    if( MI_CpuComp8(pDecrypted, pPlainTextP, srcPSize) == 0 )
    {
        OS_TPrintf("** pDecrypted == pPlainTextP\n");
        bSuccess = TRUE;
    }
    else
    {
        OS_TPrintf("** pDecrypted != pPlainTextP\n");
    }

error_exit:
    OS_Free(pDecrypted);
    OS_Free(pEncrypted);
    OS_Free(pPlainText);

    return bSuccess;
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    BOOL bSampleSucceeded = FALSE;

    // 初期化
    OS_Init();
    DEMO_InitInteruptSystem();
    DEMO_InitAllocSystem();
    OS_TPrintf("*** start aes demo\n");


    // デモを実行
    if( OS_IsRunOnTwl() )
    {
        bSampleSucceeded = SampleMain();
    }
    else
    {
        OS_Warning("demo is not run on TWL");
    }


    // デモの実行結果を表示
    OS_TPrintf("*** End of demo\n");
    OS_TPrintf("demo %s\n", (bSampleSucceeded ? "succeeded": "failed"));
    GX_Init();
    *(GXRgb*)HW_BG_PLTT = (GXRgb)(bSampleSucceeded ? GX_RGB(0, 31, 0): GX_RGB(31, 0, 0));
    GX_DispOn();
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         DEMO_AESCallback

  Description:  AES 処理終了時に呼び出されるコールバックです。
                AESCallback 型です。

  Arguments:    result: AES 処理の結果。
                arg:    AES_Ctr で渡した最後の引数

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_AESCallback(AESResult result, void* arg)
{
    OSMessageQueue* pQ = (OSMessageQueue*)arg;
    (void)OS_SendMessage(pQ, (OSMessage)result, OS_MESSAGE_BLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_WaitAes

  Description:  AES 処理の終了を待ち受け結果を返します。
                AES_Ctr で DEMO_AESCallback を指定していなければなりません。

  Arguments:    pQ: AES_Ctr の最後の引数として渡したメッセージキュー

  Returns:      AES 処理の結果。
 *---------------------------------------------------------------------------*/
static AESResult DEMO_WaitAes(OSMessageQueue* pQ)
{
    OSMessage msg;
    (void)OS_ReceiveMessage(pQ, &msg, OS_MESSAGE_BLOCK);
    return (AESResult)msg;
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_PrintBytes

  Description:  指定されたバイナリ列を 16 進でデバッグ出力に出力します。

  Arguments:    pvoid:  対象のバイナリ列へのポインタ。
                size:   対象のバイナリ列のバイト数。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_PrintBytes(const void* pvoid, u32 size)
{
    const u8* p = (const u8*)pvoid;
    u32 i;

    for( i = 0; i < size; ++i )
    {
        OS_TPrintf("%02X ", p[i]);
        if( i % 16 == 15 )
        {
            OS_TPrintf("\n");
        }
    }

    if( i % 16 != 0 )
    {
        OS_TPrintf("\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitInteruptSystem

  Description:  割り込みを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_InitInteruptSystem(void)
{
    // マスター割り込みフラグを許可に
    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitAllocSystem

  Description:  ヒープを作成して OS_Alloc が使えるようにします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_InitAllocSystem(void)
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
