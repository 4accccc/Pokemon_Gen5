/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ctr-compatible
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

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void         DEMO_InitInteruptSystem(void);
static void         DEMO_InitAllocSystem(void);
static void         DEMO_InitFileSystem(void);
static void         DEMO_AESCallback(AESResult result, void* arg);
static AESResult    DEMO_WaitAes(OSMessageQueue* pQ);
static void         DEMO_PrintText(const void* pvoid, u32 size);
static void         DEMO_PrintBytes(const void* pvoid, u32 size);
static void*        DEMO_LoadFile(u32* pSize, const char* path);

static AESResult    DEMO_CtrCompatible(
                        const AESKey*   pKey,
                        AESCounter*     pCounter,
                        void*           src,
                        u32             srcSize,
                        void*           dst );



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

    // AES API の最初に AES_Init を呼び出す必要があります。
    AES_Init();

    // CTR モード暗号化
    {
        void* pPlaintext;
        void* pEncrypted;
        void* pKey;
        void* pCounter;
        u32 srcSize;

        // 暗号化するためのデータをロードします。
        // このサンプルでは PC で暗号化したデータと比較するために
        // カウンタ初期値をファイルから読み込んでいますが、
        // 通常はカウンタ初期値には固定値を使用してはいけません。
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pCounter    = DEMO_LoadFile(NULL, "counter.bin");
        pPlaintext  = DEMO_LoadFile(&srcSize, "sample16.txt");

        // 暗号化したデータを格納するためのバッファを確保します。
        // 暗号化でデータのサイズは変化しません。
        pEncrypted = OS_Alloc(srcSize);

        if( ! SDK_IS_VALID_POINTER(pEncrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // パラメータの表示
        OS_TPrintf("== CTR encrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- counter initial value\n");
        DEMO_PrintBytes(pCounter, sizeof(AESCounter));

        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- plaintext (ascii)\n");
        DEMO_PrintText(pPlaintext, srcSize);
        OS_TPrintf("-- plaintext (hex)\n");
        DEMO_PrintBytes(pPlaintext, srcSize);


        // 一般的な AES ライブラリと互換のある暗号化を行います。
        aesResult = DEMO_CtrCompatible(
                        pKey,               // 鍵
                        pCounter,           // カウンタ初期値
                        pPlaintext,         // 暗号化するデータ
                        srcSize,            // 暗号化するデータのサイズ
                        pEncrypted );       // 暗号化したデータを格納するバッファ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CtrCompatible failed(%d)\n", aesResult);
            OS_Free(pEncrypted);
            OS_Free(pPlaintext);
            OS_Free(pCounter);
            OS_Free(pKey);
            return FALSE;
        }


        // 結果の表示
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);

        // PC で暗号化したものと一致することを確認
        {
            void* pEncryptedOnPC;
            u32 size;

            pEncryptedOnPC = DEMO_LoadFile(&size, "encrypted.bin");

            if( (srcSize == size) && MI_CpuComp8(pEncrypted, pEncryptedOnPC, size) == 0 )
            {
                OS_TPrintf("** pEncrypted == pEncryptedOnPC\n");
            }
            else
            {
                OS_TPrintf("** pEncrypted != pEncryptedOnPC\n");
            }

            OS_Free(pEncryptedOnPC);
        }

        OS_Free(pEncrypted);
        OS_Free(pPlaintext);
        OS_Free(pCounter);
        OS_Free(pKey);
    }

    // CTR モード復号化
    {
        void* pEncrypted;
        void* pDecrypted;
        void* pKey;
        void* pCounter;
        u32 srcSize;

        // PC で暗号化したデータをロードします。
        // サンプルでは鍵も暗号文も ROM に置いていますが
        // 鍵と暗号文を同じ経路で取得するべきではありません。
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pCounter    = DEMO_LoadFile(NULL, "counter.bin");
        pEncrypted  = DEMO_LoadFile(&srcSize, "encrypted.bin");

        // 復号化したデータを格納するためのバッファを確保します。
        // 復号化でデータのサイズは変化しません。
        pDecrypted = OS_Alloc(srcSize);

        if( ! SDK_IS_VALID_POINTER(pDecrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // パラメータの表示
        OS_TPrintf("== CTR decrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- counter initial value\n");
        DEMO_PrintBytes(pCounter, sizeof(AESCounter));

        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, srcSize);


        // 一般的な AES ライブラリと互換のある復号化を行います。
        aesResult = DEMO_CtrCompatible(
                        pKey,               // 鍵（暗号化時と同じでなければなりません）
                        pCounter,           // カウンタ初期値（暗号化時と同じでなければなりません）
                        pEncrypted,         // 復号化するデータ
                        srcSize,            // 復号化するデータのサイズ
                        pDecrypted );       // 復号化したデータを格納するバッファ
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CtrCompatible failed(%d)\n", aesResult);
            OS_Free(pDecrypted);
            OS_Free(pEncrypted);
            OS_Free(pCounter);
            OS_Free(pKey);
            return FALSE;
        }


        // 結果の表示
        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, srcSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        DEMO_PrintText(pDecrypted, srcSize);
        OS_TPrintf("\n");

        // オリジナルと一致することを確認
        {
            void* pOriginal;
            u32 size;

            pOriginal = DEMO_LoadFile(&size, "sample16.txt");

            if( (srcSize == size) && MI_CpuComp8(pDecrypted, pOriginal, size) == 0 )
            {
                OS_TPrintf("** pDecrypted == pOriginal\n");
                bSuccess = TRUE;
            }
            else
            {
                OS_TPrintf("** pDecrypted != pOriginal\n");
            }

            OS_Free(pOriginal);
        }


        OS_Free(pDecrypted);
        OS_Free(pEncrypted);
        OS_Free(pCounter);
        OS_Free(pKey);
    }

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
    DEMO_InitFileSystem();
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
  Name:         DEMO_CtrCompatible

  Description:  TWL-SDK の AES ライブラリは一般的な AES ライブラリと出力結果に
                互換性がありません。
                この関数では AES 処理の前後で適切な処理を行うことで
                一般的な AES ライブラリと互換のある暗号化/復号化を行います。
                ただし、AES_Ctr と比較して srcSize が
                AES_BLOCK_SIZE(=16) の倍数でなければならないという
                制限が加わります。

  Arguments:    pKey:       使用する鍵を指定します。
                pCounter:   使用するカウンタ初期値を指定します。
                src:        平文または暗号文へのポインタを指定します。
                            ポインタは 4 byte アライメントでなければなりません。
                srcSize:    平文または暗号文のサイズを指定します。
                            16 の倍数でなければなりません。
                dst:        暗号文または平文を書き出すバッファへのポインタを
                            指定します。
                            ポインタは 4 byte アライメントでなければなりません。

  Returns:      処理結果を返します。
 *---------------------------------------------------------------------------*/
static AESResult DEMO_CtrCompatible(
    const AESKey*       pKey,
    AESCounter*         pCounter,
    void*               src,
    u32                 srcSize,
    void*               dst )
{
    AESKey          key;

    // 全ての入力のバイトオーダーを入れ替えます。
    {
        AES_ReverseBytes(pKey, &key, sizeof(key));
        AES_ReverseBytes(pCounter, pCounter, sizeof(*pCounter));
        AES_SwapEndianEach128(src, src, srcSize);
    }

    // AES 処理
    {
        OSMessageQueue  msgQ;
        OSMessage       msgQBuffer[1];
        AESResult       aesResult;

        OS_InitMessageQueue(&msgQ, msgQBuffer, sizeof(msgQBuffer)/sizeof(*msgQBuffer));

        aesResult = AES_SetKey(&key);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }

        aesResult = AES_Ctr( pCounter,
                             src,
                             srcSize,
                             dst,
                             DEMO_AESCallback,
                             &msgQ );
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }

        aesResult = DEMO_WaitAes(&msgQ);
        if( aesResult != AES_RESULT_SUCCESS )
        {
            return aesResult;
        }
    }

    // 出力のバイトオーダーも入れ替えます。
    {
        AES_SwapEndianEach128(dst, dst, srcSize);
    }

    return AES_RESULT_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         DEMO_LoadFile

  Description:  メモリを確保してファイルを読み込みます。

  Arguments:    pSize:  読み込んだファイルのサイズを格納するバッファへのポインタ。
                        サイズが不要な場合は NULL を指定します。
                path:   読み込むファイルのパス。

  Returns:      ファイルの内容が格納されているバッファへのポインタを返します。
                このバッファは不要になったら OS_Free で解放する必要があります。
                読み込みに失敗した場合は NULL を返します。
 *---------------------------------------------------------------------------*/
static void* DEMO_LoadFile(u32* pSize, const char* path)
{
    BOOL bSuccess;
    FSFile f;
    u32 fileSize;
    s32 readSize;
    void* pBuffer;

    FS_InitFile(&f);

    bSuccess = FS_OpenFileEx(&f, path, FS_FILEMODE_R);
    if( ! bSuccess )
    {
        OS_Warning("fail to FS_OpenFileEx(%s)\n", path);
        return NULL;
    }

    fileSize = FS_GetFileLength(&f);
    pBuffer = OS_Alloc(fileSize + 1);
    if( pBuffer == NULL )
    {
        (void)FS_CloseFile(&f);
        OS_Warning("fail to OS_Alloc(%d)\n", fileSize + 1);
        return NULL;
    }

    readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
    if( readSize != fileSize )
    {
        OS_Free(pBuffer);
        (void)FS_CloseFile(&f);
        OS_Warning("fail to FS_ReadFile(%d)\n", fileSize + 1);
        return NULL;
    }

    bSuccess = FS_CloseFile(&f);
    SDK_ASSERT( bSuccess );

    ((char*)pBuffer)[fileSize] = '\0';

    if( pSize != NULL )
    {
        *pSize = fileSize;
    }

    return pBuffer;
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
  Name:         DEMO_PrintText

  Description:  指定された長さの文字列をデバッグ出力に出力します。

  Arguments:    pvoid:  対象の文字列
                size:   対象の文字列のバイト数。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_PrintText(const void* pvoid, u32 size)
{
    static const u32 TMP_BUFFER_LEN = 128 - 1;
    char tmp_buffer[TMP_BUFFER_LEN + 1];
    const u8* p = (const u8*)pvoid;

    while( size >= TMP_BUFFER_LEN )
    {
        MI_CpuCopy8(p, tmp_buffer, TMP_BUFFER_LEN);
        tmp_buffer[TMP_BUFFER_LEN] = '\0';
        OS_PutString(tmp_buffer);

        size -= TMP_BUFFER_LEN;
        p    += TMP_BUFFER_LEN;
    }

    if( size > 0 )
    {
        MI_CpuCopy8(p, tmp_buffer, size);
        tmp_buffer[size] = '\0';
        OS_PutString(tmp_buffer);
    }
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

/*---------------------------------------------------------------------------*
  Name:         DEMO_InitFileSystem

  Description:  FS を初期化して rom にアクセスできるようにします。
                この関数を呼び出す前に DEMO_InitInteruptSystem() と
                DEMO_InitAllocSystem() が呼ばれている必要があります。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_InitFileSystem(void)
{
    void* p_table;
    u32 need_size;

    // ARM7との通信FIFO割り込み許可
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // ファイルシステム初期化
    FS_Init( FS_DMA_NOT_USE );

    // ファイルテーブルキャッシュ
    need_size = FS_GetTableSize();
    p_table = OS_Alloc(need_size);
    SDK_POINTER_ASSERT(p_table);
    (void)FS_LoadTable(p_table, need_size);
}
