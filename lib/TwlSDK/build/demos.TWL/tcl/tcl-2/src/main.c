/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tcl - demos.TWL - tcl-2
  File:     main.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

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
#include <twl/tcl.h>
#include <twl/ssp/ARM9/jpegenc.h>
#include <twl/ssp/ARM9/exifenc.h>

#include "DEMO.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define BMP_HEADER_SIZE 54 // BMPファイルのヘッダサイズ（Windows bitmap）

/*---------------------------------------------------------------------------*
    内部関数 定義
 *---------------------------------------------------------------------------*/
void VBlankIntr(void);
void Print(void);
void BMPToRGB( const u8* p_bmp, u16* p_rgb );

/*---------------------------------------------------------------------------*
    内部変数 定義
 *---------------------------------------------------------------------------*/
int  state = 0;
int  num_pict = 0;
int  rest_pict = 0;

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みハンドラ

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

/*---------------------------------------------------------------------------*
  Name:         Print

  Description:  デバグ情報描画

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Print(void)
{
    ClearScreen();

    PutScreen(0, 0, 0xff, "TCL-2 Write Picture Demo");

    PutScreen(2, 4, 0xff, "Num Pictures       : %d", num_pict );
    PutScreen(2, 3, 0xff, "Rest Num Pictures  : %d", rest_pict );

    switch( state )
    {
    case 0: PutScreen( 1, 8, 0xff, "A Button > Write Picture" ); break;
    case 1: PutScreen( 1, 8, 0xff, "Now Writing..." );           break;
    case 2: PutScreen( 1, 8, 0xff, "Finish!" );                  break;
    case 3: PutScreen( 1, 8, 0xff, "Not enable Write Picture" ); break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         BMPToRGB

  Description:  24bitBPMをRGB555に変換

  Arguments:    p_bmp 元Bitmap画像
                p_rgb 新RGB画像

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BMPToRGB( const u8* p_bmp, u16* p_rgb )
{
    int h, w;
    int b, g, r;

    for ( h = 0; h < TCL_JPEG_HEIGHT; ++h )
    {
        for ( w = 0; w < TCL_JPEG_WIDTH; ++w )
        {
            b = ( p_bmp[ 3 * ( h * TCL_JPEG_WIDTH + w ) + 0 ] >> 3 ) & 0x1f;
            g = ( p_bmp[ 3 * ( h * TCL_JPEG_WIDTH + w ) + 1 ] >> 3 ) & 0x1f;
            r = ( p_bmp[ 3 * ( h * TCL_JPEG_WIDTH + w ) + 2 ] >> 3 ) & 0x1f;

            p_rgb[ ( TCL_JPEG_HEIGHT - h - 1 ) * TCL_JPEG_WIDTH + w ] = (u16)( 0x8000 |
                                                                               ( b << GX_RGBA_B_SHIFT ) |
                                                                               ( g << GX_RGBA_G_SHIFT ) |
                                                                               ( r << GX_RGBA_R_SHIFT ) );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OSHeapHandle handle;
    void* nstart;

    void* pTableBuf;
    void* pWorkBuf;

    FSResult fs_result;
    TCLResult result;

    // TCLアクセサ
    TCLAccessor accessor;

    // 検索条件オブジェクト
    TCLSearchObject search_obj;

    BOOL write_enable = TRUE;
    int count;

    // 初期化処理
    DEMOInitCommon();
    DEMOInitVRAM();

    OS_InitTick();
    OS_InitAlarm();
    RTC_Init();
    FS_Init( FS_DMA_NOT_USE );

    InitScreen();
    ClearScreen();

    // GX関連設定
    GX_SetBankForBG( GX_VRAM_BG_128_A );

    GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BG0_AS_2D );

    GX_SetVisiblePlane( GX_PLANEMASK_BG3 );
    G2_SetBG3Priority( 0 );
    G2_BG3Mosaic( FALSE );

    G2_SetBG3ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000 );

    // ヒープ作成
    nstart = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1 );
    OS_SetMainArenaLo( nstart );

    handle = OS_CreateHeap( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi() );
    if (handle < 0)
    {
        OS_Panic("ARM9: Fail to create heap...");
    }
    handle = OS_SetCurrentHeap( OS_ARENA_MAIN, handle );


    // TCL初期化
    // 32byteアライメントが必要
    pTableBuf = OS_Alloc( TCL_GetTableBufferSize() );
    pWorkBuf = OS_Alloc( TCL_GetWorkBufferSize() );
    if( pTableBuf == NULL || pWorkBuf == NULL )
    {
        OS_Panic("Cannot allocate memory!");
    }

    result = TCL_LoadTable( &accessor,
                            pTableBuf,
                            TCL_GetTableBufferSize(),
                            pWorkBuf,
                            TCL_GetWorkBufferSize(),
                            &fs_result );

#if 1

    // 管理ファイルの読み込みに失敗した場合に以降の処理を終了する

    if ( result != TCL_RESULT_SUCCESS )
    {
        // それ以外のエラーの場合はDSiカメラの起動を促して処理終了
        //OS_TPrintf( "それらしい文言を" );

        OS_Panic("failed TCL_LoadTable : %d\n", result );
    }

#else

    // 管理ファイルの読み込みに失敗した場合に復旧を試みる

    switch( result )
    {
    case TCL_RESULT_SUCCESS: break; // 成功
    case TCL_RESULT_ERROR_EXIST_OTHER_FILE:
    case TCL_RESULT_ERROR_ALREADY_MANAGED:
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );

        // 書き出し処理が行えないので次回保存先を再度計算する
        result = TCL_RepairTable( &accessor, &fs_result );

        if( result != TCL_RESULT_SUCCESS )
        {
            OS_TPrintf("failed TCL_RepairTable : %d\n", result );

            // 画像書き出し処理が出来ない状態なので管理ファイルを作り直す
            result = TCL_CreateTable( &accessor,
                                      pTableBuf,
                                      TCL_GetTableBufferSize(),
                                      pWorkBuf,
                                      TCL_GetWorkBufferSize(),
                                      &fs_result );

            switch( result )
            {
            case TCL_RESULT_SUCCESS:
                // テーブル作成成功
                break;
            case TCL_RESULT_ERROR_OVER_NUM_PICTURES:
                // 500枚以上写真が存在したが、テーブル作成は成功
                OS_TPrintf("failed TCL_CreateTable : %d", result );
                break;
            case TCL_RESULT_ERROR_NO_NEXT_INDEX:
                // 保存は出来ないが続行可能なエラー
                write_enable = FALSE;
                OS_TPrintf("failed TCL_CreateTable : %d", result );
                break;
            default:
                // 続行不可能なエラー
                OS_Panic("failed TCL_CreateTable : %d", result );
                break;
            }
        }
        break;
    default:
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );

        // 読み込み処理も行えないので管理ファイルを作り直す
        result = TCL_CreateTable( &accessor,
                                  pTableBuf,
                                  TCL_GetTableBufferSize(),
                                  pWorkBuf,
                                  TCL_GetWorkBufferSize(),
                                  &fs_result );

        switch( result )
        {
        case TCL_RESULT_SUCCESS: break;
        case TCL_RESULT_ERROR_OVER_NUM_PICTURES:
            OS_TPrintf("failed TCL_CreateTable : %d", result );
            break;
        case TCL_RESULT_ERROR_NO_NEXT_INDEX:
            write_enable = FALSE;
            OS_TPrintf("failed TCL_CreateTable : %d", result );
            break;
        default:
            OS_Panic("failed TCL_CreateTable : %d", result );
            break;
        }
        break;
    }

#endif

    // あと何枚写真を保存できるか
    rest_pict = TCL_CalcNumEnableToTakePictures( &accessor );

    // SearchObjectを初期化
    TCL_InitSearchObject( &search_obj, TCL_SEARCH_CONDITION_PICTURE );

    // 全部で何枚写真があるか
    num_pict = TCL_GetNumPictures( &accessor, &search_obj );

    Print();

    DEMOStartDisplay();

    // テキスト表示を更新
    UpdateScreen();

    while (1)
    {
        // パッド情報の読み込みと操作
        DEMOReadKey();

        if( DEMO_IS_TRIG( PAD_BUTTON_A ) )
        {
            if( rest_pict == 0 || !write_enable )
            {
                // これ以上保存出来ない
                count = 0;
                state = 3;
                Print();
            }
            else if( state == 0 )
            {
                count = 0;
                ++state;
                Print();
            }
        }
        else if( state == 1 )
        {
            // jpeg画像を格納するバッファ
            u8*  pJpegBuf = NULL;
            u8*  pData    = NULL;
            u8*  pTmpBuf  = NULL;
            u16* pRGBBuf  = NULL;
            u32  length;
            FSFile file;
            const u32 option = SSP_JPEG_THUMBNAIL | SSP_JPEG_RGB555;

            // BMP画像取得
            FS_InitFile( &file );
            if( !FS_OpenFileEx( &file, "rom:/demo.bmp", FS_FILEMODE_R ) )
            {
                OS_Panic("failed FS_OpenFileEx : %d ", FS_GetResultCode( &file ) );
            }

            length = FS_GetFileLength( &file );
            pData = OS_Alloc( length );
            if( pData == NULL )
            {
                OS_Panic("Cannot allocate memory ... pData!");
            }

            if( FS_ReadFile( &file, pData, (s32)length ) == -1 )
            {
                OS_Panic("failed FS_ReadFile");
            }
            ( void ) FS_CloseFile( &file );

            pData += BMP_HEADER_SIZE;

            // BMP画像をRGB555に変換
            pRGBBuf = OS_Alloc( TCL_JPEG_WIDTH * TCL_JPEG_HEIGHT * sizeof(u16) );
            if( pRGBBuf == NULL )
            {
                OS_Panic("Cannot allocate memory ... pRGBBuf!");
            }

            BMPToRGB( pData, pRGBBuf );
            pData -= BMP_HEADER_SIZE;

            OS_Free( pData );

            // JPEGエンコード
            length = TCL_GetJpegEncoderBufferSize( option );
            pTmpBuf = OS_Alloc( length );
            if( pTmpBuf == NULL )
            {
                OS_Panic("Cannot allocate memory ... pTmpBuf!");
            }
            OS_TPrintf("SSP_GetJpegEncoderBufferSize() = %d\n", length );

            pJpegBuf = OS_Alloc( TCL_MAX_JPEG_SIZE );
            if( pJpegBuf == NULL )
            {
                OS_Panic("Cannot allocate memory ... pJpegBuf!");
            }

            // エンコード＋書き込み
            result = TCL_EncodeAndWritePicture( &accessor,
                                                pRGBBuf ,
                                                pJpegBuf,
                                                TCL_MAX_JPEG_SIZE ,
                                                pTmpBuf ,
                                                TCL_JPEG_DEFAULT_QUALITY ,
                                                option ,
                                                &fs_result );

            if( result != TCL_RESULT_SUCCESS )
            {
                OS_TPrintf("failed TCL_WritePicture : %d\n", result );

                // これ以上保存出来ない
                if( result == TCL_RESULT_ERROR_NO_NEXT_INDEX ) write_enable = FALSE;
            }
            else
            {
                OS_TPrintf("success TCL_WritePicture! %s\n" , TCL_GetLastWrittenPicturePath() );
            }

            OS_Free( pJpegBuf );
            OS_Free( pTmpBuf );
            OS_Free( pRGBBuf );

            // あと何枚写真を保存できるか
            rest_pict = TCL_CalcNumEnableToTakePictures( &accessor );

            // 全部で何枚写真があるか
            num_pict = TCL_GetNumPictures( &accessor, &search_obj );

            ++state;
            Print();
        }
        else if( state == 2 || state == 3 )
        {
            ++count;
            if( count > 180 )
            {
                state = 0;
                Print();
            }
        }

        OS_WaitVBlankIntr();

        // テキスト表示を更新
        UpdateScreen();
    }

    OS_Free( pWorkBuf );
    OS_Free( pTableBuf );

    OS_Terminate();
}
