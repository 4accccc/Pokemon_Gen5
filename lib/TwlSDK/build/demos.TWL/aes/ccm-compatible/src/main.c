/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - demos - ccm-compatible
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

#define DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE    0xFEFF
#define DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE    AES_ADATA_SIZE_MAX  // 本来は 0xFFFFFFFF
#define DEMO_CCM_COMPATIBLE_2_HEADER_SIZE       2
#define DEMO_CCM_COMPATIBLE_6_HEADER_SIZE       6
#define DEMO_CCM_COMPATIBLE_6_MAGIC_NUMBER      0xFFFE

static u32  DEMO_CalcAdataHeaderSize(u32 adataSize);
static void DEMO_MakeAdataHeader(void* pv, u32 adataSize);
static AESResult    DEMO_CcmEncryptCompatible(
                        const AESKey*   pKey,
                        const AESNonce* pNonce,
                        void*           pWorkBuffer,
                        const void*     pAdata,
                        u32             adataSize,
                        const void*     pPdata,
                        u32             pdataSize,
                        AESMacLength    macLength,
                        void*           dst );
static AESResult    DEMO_CcmDecryptCompatible(
                        const AESKey*   pKey,
                        const AESNonce* pNonce,
                        void*           pWorkBuffer,
                        const void*     pAdata,
                        u32             adataSize,
                        const void*     pCdata,
                        u32             cdataSize,
                        AESMacLength    macLength,
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

    // CCM モード暗号化
    {
        const AESMacLength macLength = AES_MAC_LENGTH_16;

        const u32 macSize = AES_GetMacLengthValue(macLength);
        void* pAdata;
        void* pPdata;
        void* pEncrypted;
        void* pKey;
        void* pNonce;
        u32 adataSize;
        u32 pdataSize;

        // 暗号化するためのデータをロードします。
        // このサンプルでは PC で暗号化したデータと比較するために
        // Nonce をファイルから読み込んでいますが、
        // 通常は Nonce には固定値を使用してはいけません。
        pKey    = DEMO_LoadFile(NULL, "key.bin");
        pNonce  = DEMO_LoadFile(NULL, "nonce.bin");
        pAdata  = DEMO_LoadFile(&adataSize, "sample_adata.txt");
        pPdata  = DEMO_LoadFile(&pdataSize, "sample_pdata.txt");

        if( (pdataSize % AES_BLOCK_SIZE) != 0 )
        {
            OS_TPrintf("pdataSize(=%d) must be multiple of 16.\n", pdataSize);
            return FALSE;
        }


        // 暗号化したデータを格納するためのバッファを確保します。
        // MAC の分だけ大きいバッファが必要です。
        pEncrypted = OS_Alloc(pdataSize + macSize);

        if( ! SDK_IS_VALID_POINTER(pEncrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }


        // パラメータの表示
        OS_TPrintf("== CCM encrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- nonce\n");
        DEMO_PrintBytes(pNonce, sizeof(AESNonce));

        OS_TPrintf("---- encrypt -------------\n");
        OS_TPrintf("-- Adata (ascii)\n");
        DEMO_PrintText(pAdata, adataSize);
        OS_TPrintf("-- Pdata (ascii)\n");
        DEMO_PrintText(pPdata, pdataSize);
        OS_TPrintf("\n");
        OS_TPrintf("-- Adata (hex)\n");
        DEMO_PrintBytes(pAdata, adataSize);
        OS_TPrintf("-- Pdata (hex)\n");
        DEMO_PrintBytes(pPdata, pdataSize);


        // 一般的な AES ライブラリと互換のある暗号化を行います。
        if( adataSize <= DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE )
        {
            const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
            const u32 workSize   = headerSize + adataSize + pdataSize;
            void* pWorkBuffer    = OS_Alloc(workSize);

            aesResult = DEMO_CcmEncryptCompatible(
                            pKey,               // 鍵
                            pNonce,             // Nonce
                            pWorkBuffer,        // 作業用バッファ
                            pAdata,             // Adata が格納されているバッファへのポインタ
                            adataSize,          // Adata のサイズ
                            pPdata,             // Pdata が格納されているバッファへのポインタ
                            pdataSize,          // Pdata のサイズ
                            macLength,          // MAC のサイズ
                            pEncrypted );       // 暗号化したデータを格納するバッファ

            OS_Free(pWorkBuffer);
        }
        else
        {
            OS_TPrintf("Too huge Adata size(=%d)\n", adataSize);
            OS_Free(pEncrypted);
            OS_Free(pPdata);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CcmEncryptCompatible failed(%d)\n", aesResult);
            OS_Free(pEncrypted);
            OS_Free(pPdata);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }


        // 結果の表示
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, pdataSize);

        OS_TPrintf("-- mac (hex)\n");
        DEMO_PrintBytes((u8*)pEncrypted + pdataSize, macSize);

        // PC で暗号化したものと一致することを確認
        {
            void* pEncryptedOnPC;
            u32 size;

            pEncryptedOnPC = DEMO_LoadFile(&size, "encrypted.bin");

            if( (pdataSize + macSize == size) && MI_CpuComp8(pEncrypted, pEncryptedOnPC, size) == 0 )
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
        OS_Free(pPdata);
        OS_Free(pAdata);
        OS_Free(pNonce);
        OS_Free(pKey);
    }

    // CCM モード復号化
    {
        const AESMacLength macLength = AES_MAC_LENGTH_16;

        const u32 macSize = AES_GetMacLengthValue(macLength);
        void* pAdata;
        void* pEncrypted;
        void* pDecrypted;
        void* pKey;
        void* pNonce;
        u32 adataSize;
        u32 cdataSize;
        u32 srcFileSize;

        // PC で暗号化したデータをロードします。
        // サンプルでは鍵も暗号文も ROM に置いていますが
        // 鍵と暗号文を同じ経路で取得するべきではありません。
        pKey        = DEMO_LoadFile(NULL, "key.bin");
        pNonce      = DEMO_LoadFile(NULL, "nonce.bin");
        pAdata      = DEMO_LoadFile(&adataSize, "sample_adata.txt");
        pEncrypted  = DEMO_LoadFile(&srcFileSize, "encrypted.bin");
        cdataSize = srcFileSize - macSize;

        // 復号化したデータを格納するためのバッファを確保します。
        pDecrypted = OS_Alloc(cdataSize);

        if( ! SDK_IS_VALID_POINTER(pDecrypted) )
        {
            OS_TPrintf("fail to OS_Alloc\n");
            return FALSE;
        }

        // パラメータの表示
        OS_TPrintf("== CCM decrypt ==========================\n");
        OS_TPrintf("---- parameter -----------\n");
        OS_TPrintf("-- key\n");
        DEMO_PrintBytes(pKey, sizeof(AESKey));
        OS_TPrintf("-- nonce\n");
        DEMO_PrintBytes(pNonce, sizeof(AESNonce));

        OS_TPrintf("---- decrypt -------------\n");
        OS_TPrintf("-- Adata (ascii)\n");
        DEMO_PrintText(pAdata, adataSize);
        OS_TPrintf("\n");
        OS_TPrintf("-- Adata (hex)\n");
        DEMO_PrintBytes(pAdata, adataSize);
        OS_TPrintf("-- encrypted (hex)\n");
        DEMO_PrintBytes(pEncrypted, cdataSize + macSize);

        // 一般的な AES ライブラリと互換のある復号化を行います。
        if( adataSize <= DEMO_CCM_COMPATIBLE_6_MAX_ADATA_SIZE )
        {
            const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
            const u32 workSize   = headerSize + adataSize + cdataSize + macSize;
            void* pWorkBuffer    = OS_Alloc(workSize);

            aesResult = DEMO_CcmDecryptCompatible(
                            pKey,               // 鍵
                            pNonce,             // Nonce
                            pWorkBuffer,        // 作業用バッファ
                            pAdata,             // Adata が格納されているバッファへのポインタ
                            adataSize,          // Adata のサイズ
                            pEncrypted,         // 暗号文と MAC が格納されているバッファへのポインタ
                            cdataSize,          // 暗号文のサイズ
                            macLength,          // MAC のサイズ
                            pDecrypted );       // 復号化したデータを格納するバッファ

            OS_Free(pWorkBuffer);
        }
        else
        {
            OS_TPrintf("Too huge Adata size(=%d)\n", adataSize);
            OS_Free(pEncrypted);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }
        // 暗号化したデータが改竄されていると
        // ここで得られる aesResult が AES_RESULT_VERIFICATION_FAILED になります。
        if( aesResult != AES_RESULT_SUCCESS )
        {
            OS_TPrintf("DEMO_CcmDecryptCompatible failed(%d)\n", aesResult);
            OS_Free(pDecrypted);
            OS_Free(pEncrypted);
            OS_Free(pAdata);
            OS_Free(pNonce);
            OS_Free(pKey);
            return FALSE;
        }


        // 結果の表示
        OS_TPrintf("-- decrypted (hex)\n");
        DEMO_PrintBytes(pDecrypted, cdataSize);

        OS_TPrintf("-- decrypted (ascii)\n");
        DEMO_PrintText(pDecrypted, cdataSize);
        OS_TPrintf("\n");

        // オリジナルと一致することを確認
        {
            void* pOriginal;
            u32 size;

            pOriginal = DEMO_LoadFile(&size, "sample_pdata.txt");

            if( (cdataSize == size) && MI_CpuComp8(pDecrypted, pOriginal, size) == 0 )
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
        OS_Free(pAdata);
        OS_Free(pNonce);
        OS_Free(pKey);
    }

    return bSuccess;
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

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
  Name:         DEMO_CalcAdataHeaderSize

  Description:  TWL-SDK の AES ライブラリは一般的な AES ライブラリと
                出力結果に互換性がありません。
                互換性が取れていないものの一つとして
                CCM ヘッダの直後に配置される Adata のサイズが自動的に
                付加されないというものがあります。
                この関数は Adata のサイズを格納するのに必要な領域サイズを
                計算します。

  Arguments:    adataSize:      Adata のサイズ。

  Returns:      Adata のサイズを格納するのに必要な領域サイズを返します。
 *---------------------------------------------------------------------------*/
static u32 DEMO_CalcAdataHeaderSize(u32 adataSize)
{
    return (u32)( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE)
                    ? DEMO_CCM_COMPATIBLE_2_HEADER_SIZE:
                      DEMO_CCM_COMPATIBLE_6_HEADER_SIZE );
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_MakeAdataHeader

  Description:  TWL-SDK の AES ライブラリは一般的な AES ライブラリと
                出力結果に互換性がありません。
                互換性が取れていないものの一つとして
                CCM ヘッダの直後に配置される Adata のサイズが自動的に
                付加されないというものがあります。
                この関数は Adata のサイズを規定されているフォーマットで
                バッファに書き出します。

  Arguments:    pv:             Adata のサイズを書き出すバッファへのポインタ。
                adataSize:      Adata のサイズ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void DEMO_MakeAdataHeader(void* pv, u32 adataSize)
{
    u8* p = (u8*)pv;

    if( adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE )
    {
        MI_StoreBE16(p, (u16)adataSize);
    }
    else
    {
        MI_StoreBE16(p + 0, DEMO_CCM_COMPATIBLE_6_MAGIC_NUMBER);
        MI_StoreBE32(p + 2, adataSize);
    }
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_CcmEncryptCompatible

  Description:  TWL-SDK の AES ライブラリは一般的な AES ライブラリと
                出力結果に互換性がありません。
                この関数では AES 処理の前後で適切な処理を行うことで
                一般的な AES ライブラリと互換のある暗号化を行います。
                ただし、AES_CcmEncryptAndSign と比較して pdataSize が
                AES_BLOCK_SIZE(=16) の倍数でなければならないという
                制限が加わります。また adataSize の制限が 16 の倍数ではなく、
                adataSize によって 16 の倍数 - 2 または 16 の倍数 - 4 になります。

  Arguments:    pKey:           使用する鍵を指定します。
                pNonce:         使用する Nonce を指定します。
                pWorkBuffer:    関数内部で使用する作業用バッファへのポインタを
                                渡します。
                                adataSize + pdataSize
                                          + DEMO_CalcAdataHeaderSize(adataSize)
                                のサイズが必要です。
                                ポインタは 4 byte アライメントでなければなりません。
                pAdata          Adata へのポインタを指定します。
                adataSize:      Adata のサイズを指定します。
                                16 の倍数 - DEMO_CalcAdataHeaderSize(adataSize)
                                でなければなりません。
                pPdata:         Pdata へのポインタを指定します。
                pdataSize:      Pdata のサイズを指定します。
                                16 の倍数でなければなりません。
                macLength:      MAC のサイズを指定します。
                dst:            暗号文を書き出すバッファへのポインタを指定します。
                                pdataSize + DEMO_CalcAdataHeaderSize(adataSize)
                                のサイズが必要です。
                                ポインタは 4 byte アライメントでなければなりません。

  Returns:      処理結果を返します。
 *---------------------------------------------------------------------------*/
AESResult DEMO_CcmEncryptCompatible(
    const AESKey*   pKey,
    const AESNonce* pNonce,
    void*           pWorkBuffer,
    const void*     pAdata,
    u32             adataSize,
    const void*     pPdata,
    u32             pdataSize,
    AESMacLength    macLength,
    void*           dst )
{
    const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
    AESKey    key;
    AESNonce  nonce;

    SDK_POINTER_ASSERT(pKey);
    SDK_POINTER_ASSERT(pNonce);
    SDK_POINTER_ASSERT(pWorkBuffer);
    SDK_POINTER_ASSERT(pAdata);
    SDK_POINTER_ASSERT(pPdata);
    SDK_POINTER_ASSERT(dst);
    SDK_MAX_ASSERT( adataSize, AES_ADATA_SIZE_MAX );
    SDK_MAX_ASSERT( pdataSize, AES_PDATA_SIZE_MAX );
    SDK_ASSERT( (adataSize >  DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 14) );
    SDK_ASSERT( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 10) );
    SDK_ASSERT( (pdataSize % 16) ==  0 );

    // 入力データを作成します。
    {
        const u32 offsetHeader = 0;
        const u32 offsetAdata  = offsetHeader + headerSize;
        const u32 offsetPdata  = offsetAdata  + adataSize;

        u8* pWork = (u8*)pWorkBuffer;
        DEMO_MakeAdataHeader(pWork + offsetHeader, adataSize);  // Adata サイズ
        MI_CpuCopy(pAdata, pWork + offsetAdata, adataSize);     // Adata
        MI_CpuCopy(pPdata, pWork + offsetPdata, pdataSize);     // Pdata
    }

    // 全ての入力のバイトオーダーを入れ替えます。
    {
        const u32 workSize = headerSize + adataSize + pdataSize;

        AES_ReverseBytes(pKey, &key, sizeof(key));                  // 鍵
        AES_ReverseBytes(pNonce, &nonce, sizeof(nonce));            // Nonce
        AES_SwapEndianEach128(pWorkBuffer, pWorkBuffer, workSize);  // Adata サイズ、Adata、Pdata
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

        aesResult = AES_CcmEncryptAndSign(
                        &nonce,
                        pWorkBuffer,
                        adataSize + headerSize,
                        pdataSize,
                        macLength,
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
        void* mac = (u8*)dst + pdataSize;
        u32 macSize = AES_GetMacLengthValue(macLength);

        AES_SwapEndianEach128(dst, dst, pdataSize);     // Cdata
        AES_ReverseBytes(mac, mac, macSize);            // MAC
    }

    return AES_RESULT_SUCCESS;
}



/*---------------------------------------------------------------------------*
  Name:         DEMO_CcmDecryptCompatible

  Description:  TWL-SDK の AES ライブラリは一般的な AES ライブラリと
                出力結果に互換性がありません。
                この関数では AES 処理の前後で適切な処理を行うことで
                一般的な AES ライブラリと互換のある復号化を行います。
                ただし、AES_CcmDecryptAndVerify と比較して cdataSize が
                AES_BLOCK_SIZE(=16) の倍数でなければならないという
                制限が加わります。また adataSize の制限が 16 の倍数ではなく、
                adataSize によって 16 の倍数 - 2 または 16 の倍数 - 4 になります。

  Arguments:    pKey:           使用する鍵を指定します。
                pNonce:         使用する Nonce を指定します。
                pWorkBuffer:    関数内部で使用する作業用バッファへのポインタを
                                渡します。
                                adataSize + pdataSize
                                          + DEMO_CalcAdataHeaderSize(adataSize)
                                          + AES_GetMacLengthValue(macLength)
                                のサイズが必要です。
                                ポインタは 4 byte アライメントでなければなりません。
                pAdata          Adata へのポインタを指定します。
                adataSize:      Adata のサイズを指定します。
                                16 の倍数 - DEMO_CalcAdataHeaderSize(adataSize)
                                でなければなりません。
                pPdata:         暗号文と MAC が格納されたバッファへのポインタを指定します。
                pdataSize:      暗号文のサイズを指定します。
                                16 の倍数でなければなりません。
                macLength:      MAC のサイズを指定します。
                dst:            暗号文を書き出すバッファへのポインタを指定します。
                                ポインタは 4 byte アライメントでなければなりません。

  Returns:      処理結果を返します。
 *---------------------------------------------------------------------------*/
AESResult DEMO_CcmDecryptCompatible(
    const AESKey*   pKey,
    const AESNonce* pNonce,
    void*           pWorkBuffer,
    const void*     pAdata,
    u32             adataSize,
    const void*     pCdata,
    u32             cdataSize,
    AESMacLength    macLength,
    void*           dst )
{
    const u32 headerSize = DEMO_CalcAdataHeaderSize(adataSize);
    const u32 macSize    = AES_GetMacLengthValue(macLength);
    AESKey    key;
    AESNonce  nonce;

    SDK_POINTER_ASSERT(pKey);
    SDK_POINTER_ASSERT(pNonce);
    SDK_POINTER_ASSERT(pWorkBuffer);
    SDK_POINTER_ASSERT(pAdata);
    SDK_POINTER_ASSERT(pCdata);
    SDK_POINTER_ASSERT(dst);
    SDK_MAX_ASSERT( adataSize, AES_ADATA_SIZE_MAX );
    SDK_MAX_ASSERT( cdataSize, AES_PDATA_SIZE_MAX );
    SDK_ASSERT( (adataSize >  DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 14) );
    SDK_ASSERT( (adataSize <= DEMO_CCM_COMPATIBLE_2_MAX_ADATA_SIZE) || ((adataSize % 16) == 10) );
    SDK_ASSERT( (cdataSize % 16) ==  0 );

    // 入力データを作成します。
    {
        const u32 offsetHeader = 0;
        const u32 offsetAdata  = offsetHeader + headerSize;
        const u32 offsetCdata  = offsetAdata  + adataSize;

        u8* pWork = (u8*)pWorkBuffer;
        DEMO_MakeAdataHeader(pWork + offsetHeader, adataSize);          // Adata サイズ
        MI_CpuCopy(pAdata, pWork + offsetAdata, adataSize);             // Adata
        MI_CpuCopy(pCdata, pWork + offsetCdata, cdataSize + macSize);   // Cdata、MAC
    }

    // 全ての入力のバイトオーダーを入れ替えます。
    {
        const u32 workSize = headerSize + adataSize + cdataSize + macSize;

        AES_ReverseBytes(pKey, &key, sizeof(key));                  // 鍵
        AES_ReverseBytes(pNonce, &nonce, sizeof(nonce));            // Nonce
        AES_SwapEndianEach128(pWorkBuffer, pWorkBuffer, workSize);  // Adata サイズ、Adata、Cdata、MAC
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

        aesResult = AES_CcmDecryptAndVerify(
                        &nonce,
                        pWorkBuffer,
                        adataSize + headerSize,
                        cdataSize,
                        macLength,
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
        AES_SwapEndianEach128(dst, dst, cdataSize);     // Pdata
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
