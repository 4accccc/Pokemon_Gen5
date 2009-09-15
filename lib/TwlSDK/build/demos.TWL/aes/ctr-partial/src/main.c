/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ctr-partial
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
    内部変数宣言
 *---------------------------------------------------------------------------*/

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
static AESResult    DEMO_CtrDecryptPartial(
                        const AESCounter*   pCounter,
                        const void*         pEncrypted,
                        u32                 decryptOffset,
                        u32                 decryptSize,
                        void*               dst,
                        AESCallback         callback,
                        void*               arg );



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
    BOOL bSuccess = FALSE;

    AESResult aesResult;
    AESCounter counter;
    const char* pPlainText = DEMO_TEXT;
    void* pEncrypted;
    void* pDecrypted;
    u32 srcSize;

    // AES 処理の待ち受けのためのメッセージキューを初期化します。
    OS_InitMessageQueue(&sMsgQ, sMsgQBuffer, DEMO_MESSAGE_Q_SIZE);

    // 暗号化/復号化したデータを格納するためのバッファを確保します。
    // 暗号化/復号化でデータのサイズは変化しません。
    srcSize = (u32)STD_GetStringLength(pPlainText) + 1;
    pEncrypted = OS_Alloc(srcSize);
    pDecrypted = OS_Alloc(srcSize);

    if( ! SDK_IS_VALID_POINTER(pEncrypted)
     || ! SDK_IS_VALID_POINTER(pDecrypted) )
    {
        OS_TPrintf("fail to OS_Alloc\n");
        return FALSE;
    }

    // AES API の最初に AES_Init を呼び出す必要があります。
    AES_Init();

    // カウンタ初期値を生成するために AES_Rand を使用します。
    // カウンタ初期値は任意の数値でかまいませんが、
    // できるだけ同じ値を使わないようにしなければなりません。
    aesResult = AES_Rand(&counter, sizeof(counter));
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
    OS_TPrintf("-- counter initial value\n");
    DEMO_PrintBytes(&counter, sizeof(counter));


    // 暗号化
    {
        // AES CTR モードでの暗号化を行います。
        aesResult = AES_CtrEncrypt( &counter,           // カウンタ初期値
                                    pPlainText,         // 暗号化するデータ
                                    srcSize,            // 暗号化するデータのサイズ
                                    pEncrypted,         // 暗号化したデータを格納するバッファ
                                    DEMO_AESCallback,   // 処理終了時に呼び出されるコールバック
                                    &sMsgQ );           // コールバックパラメータ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CtrEncrypt failed(%d)\n", aesResult);
            goto error_exit;
        }

        // 暗号化処理が終了するのを待ちます。
        aesResult = DEMO_WaitAes(&sMsgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CtrEncrypt failed(%d)\n", aesResult);
            goto error_exit;
        }


        // 結果の表示
        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- plain text (ascii)\n");
        OS_PutString(pPlainText);   // OS_*Printf には 256 文字制限があるため OS_PutString を使用
        OS_TPrintf("\n");

        OS_TPrintf("-- plain text (hex)\n");
        DEMO_PrintBytes(pPlainText, srcSize);

        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);
    }

    // 復号化
    {
        // AES CTR モードでの復号化を部分的に行います。
        const u32 decryptOffset = 80;       // AES_BLOCK_SIZE の倍数でなければなりません
        const u32 decryptSize   = 68;

        aesResult = DEMO_CtrDecryptPartial(
                        &counter,           // カウンタ初期値（暗号化時と同じでなければなりません）
                        pEncrypted,         // 復号化するデータ
                        decryptOffset,      // 復号化を開始するオフセット（AES_BLOCK_SIZE の倍数でなければなりません）
                        decryptSize,        // 復号化するトサイズ
                        pDecrypted,         // 復号化したデータを格納するバッファ
                        DEMO_AESCallback,   // 処理終了時に呼び出されるコールバック
                        &sMsgQ );           // コールバックパラメータ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CtrDecrypt failed(%d)\n", aesResult);
            goto error_exit;
        }

        // 復号化処理が終了するのを待ちます。
        aesResult = DEMO_WaitAes(&sMsgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("AES_CtrDecrypt failed(%d)\n", aesResult);
            goto error_exit;
        }

        // テキストとして表示したいので NUL 終端する
        ((char*)pDecrypted)[decryptSize] = '\0';

        // 結果の表示
        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);

        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, decryptSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        OS_PutString(pDecrypted);   // OS_*Printf には 256 文字制限があるため OS_PutString を使用
        OS_TPrintf("\n");

        // 暗号化して復号化したものがオリジナルと一致することを確認
        if( MI_CpuComp8(pDecrypted, pPlainText + decryptOffset, decryptSize) == 0 )
        {
            OS_TPrintf("** pDecrypted == pPlainText\n");
            bSuccess = TRUE;
        }
        else
        {
            OS_TPrintf("** pDecrypted != pPlainText\n");
        }
    }

error_exit:
    OS_Free(pDecrypted);
    OS_Free(pEncrypted);

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
  Name:         DEMO_CtrDecryptPartial

  Description:  CTR モードでの暗号文を部分的に復号化します。
                pEncrypted の decryptOffset バイト目から decryptSize バイトを
                復号化します。

  Arguments:    pCounter:       暗号化時に使用したカウンタ初期値を指定します。
                pEncrypted:     暗号文の先頭へのポインタを指定します。
                                ポインタは 4 byte アライメントでなければなりません。
                decryptOffset:  復号化を開始するオフセットを指定します。
                                AES_BLOCK_SIZE の倍数でなければなりません。
                decryptSize:    復号化するサイズを指定します。
                dst:            暗号文を書き出すバッファへのポインタを指定します。
                                ポインタは 4 byte アライメントでなければなりません。
                callback:       処理が完了したときに呼び出されるコールバックを
                                指定します。NULL を指定することができ、その場合
                                コールバックは呼び出されません。
                arg:            上記コールバックに渡されるパラメータを指定します。
                                NULL を指定することができます。

  Returns:      処理結果を返します。
 *---------------------------------------------------------------------------*/
static AESResult DEMO_CtrDecryptPartial(
    const AESCounter*   pCounter,
    const void*         pEncrypted,
    u32                 decryptOffset,
    u32                 decryptSize,
    void*               dst,
    AESCallback         callback,
    void*               arg )
{
    AESCounter counter;

    SDK_POINTER_ASSERT( pCounter );
    SDK_ASSERT( (decryptOffset % AES_BLOCK_SIZE) == 0 );

    // 部分復号化はカウンタ初期値をオフセットぶん加算して復号化すればよい。
    // カウンタ値は AES_BLOCK_SIZE につき 1 加算される。
    counter = *pCounter;
    AES_AddToCounter(&counter, decryptOffset / AES_BLOCK_SIZE);

    return AES_CtrDecrypt(
                &counter,
                (const u8*)pEncrypted + decryptOffset,
                decryptSize,
                dst,
                callback,
                arg );
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
