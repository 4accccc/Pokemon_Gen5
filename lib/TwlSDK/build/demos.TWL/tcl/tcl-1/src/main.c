/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tcl - demos.TWL - tcl-1
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
#include <twl/ssp/ARM9/jpegdec.h>
#include <twl/ssp/ARM9/exifdec.h>

#include "DEMO.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define WIDTH       256         // イメージの幅
#define HEIGHT      192         // イメージの高さ

/*---------------------------------------------------------------------------*
    内部関数 定義
 *---------------------------------------------------------------------------*/
void VBlankIntr(void);
void Print(void);
void DownSampling( const u16* org_img, int org_wdt, int org_hgt,
                         u16* new_img, int new_wdt, int new_hgt );
int GetR ( u16 color );
int GetG ( u16 color );
int GetB ( u16 color );

/*---------------------------------------------------------------------------*
    内部変数 定義
 *---------------------------------------------------------------------------*/
// 画像を格納するバッファ
static u16 draw_buffer[ WIDTH * HEIGHT ] ATTRIBUTE_ALIGN(32);

int condition_id = 0;
int search = 0;
int num_pict = 0;
int draw_id = 0;

// 色の値を取得する （0～31）
int GetR ( u16 color ) { return (color & GX_RGB_R_MASK) >> GX_RGB_R_SHIFT; }
int GetG ( u16 color ) { return (color & GX_RGB_G_MASK) >> GX_RGB_G_SHIFT; }
int GetB ( u16 color ) { return (color & GX_RGB_B_MASK) >> GX_RGB_B_SHIFT; }

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
    const char* condition_name[] = { "All Picture", "Favorite1", "Favorite2", "Favorite3", "FavoriteAll" };

    ClearScreen();

    PutScreen(0, 0, 0xff, "TCL-1 Search Picture Demo");

    PutScreen(2, 4, 0xff, "TCL Condition : %s", condition_name[condition_id] );
    PutScreen(2, 5, 0xff, "Draw Index    : %d", draw_id );

    PutScreen(2, 7, 0xff, "Num Pictures  : %d", num_pict );

    PutScreen(1, 12, 0xff, "A Button > Draw Picture" );
    PutScreen(1, 13, 0xff, "B Button > Draw Thumbnail" );
    PutScreen(1, 14, 0xff, "X Button > Select Condition" );
    PutScreen(1, 15, 0xff, "Y Button > Change Search Method" );
    PutScreen(1, 16, 0xff, "R Button > Delete Picture" );

    if( search )
    {
        PutScreen(1, 2, 0xff, "Iterate Search" );
    }
    else
    {
        PutScreen(1, 2, 0xff, "Random Search" );
        PutScreen(1, 17, 0xff, "Left  Key > Select Draw Index" );
        PutScreen(1, 18, 0xff, "Right Key > Select Draw Index" );
    }
}

/*---------------------------------------------------------------------------*
  Name:         DownSampling

  Description:  縮小

  Arguments:    org_img 元画像
                org_wdt 元画像の幅
                org_hgt 元画像の高さ
                new_img 新画像
                new_wdt 新画像の幅
                new_hgt 新画像の高さ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DownSampling( const u16* org_img, int org_wdt, int org_hgt,
                         u16* new_img, int new_wdt, int new_hgt )
{
    int h, w, i = 0;

    if( org_wdt == TCL_JPEG_WIDTH )
    {
        // 通常の写真
        for ( h = 0; h < new_hgt; ++h )
        {
            for ( w = 0; w < new_wdt; ++w )
            {
                new_img[ h * new_wdt + w ] = org_img[ ( h * org_hgt / new_hgt ) * org_wdt +
                                                      w * org_wdt / new_wdt ];
            }
        }
    }
    else
    {
        // サムネイル
        u16* thumb_img = &new_img[ ( HEIGHT - SSP_JPEG_THUMBNAIL_HEIGHT ) / 2 * WIDTH +
                                   ( WIDTH - SSP_JPEG_THUMBNAIL_WIDTH ) / 2 ];

        for ( h = 0; h < org_hgt; ++h )
        {
            for ( w = 0; w < org_wdt; ++w )
            {
                thumb_img[ h * new_wdt + w ] = org_img[ i ];
                ++i;
            }
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
    char  pict_path[TCL_PATH_LEN];
    void* nstart;

    void* pTableBuf;
    void* pWorkBuf;

    FSResult fs_result;
    TCLResult result;

    // TCLアクセサ
    TCLAccessor accessor;

    // 検索条件オブジェクト
    TCLSearchObject search_obj;
    TCLSortType sort_type = TCL_SORT_TYPE_DATE;

    // 初期化処理
    DEMOInitCommon();
    DEMOInitVRAM();

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

    MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );
    GX_LoadBG3Bmp( draw_buffer, 0, WIDTH * HEIGHT * sizeof(u16) );

    // ヒープ作成
    nstart = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1 );
    OS_SetMainArenaLo( nstart );

    handle = OS_CreateHeap( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi() );
    if (handle < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
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

    switch ( result )
    {
      case TCL_RESULT_SUCCESS: break; // 成功
      case TCL_RESULT_ERROR_EXIST_OTHER_FILE:
      case TCL_RESULT_ERROR_ALREADY_MANAGED:
        // 写真の取得には支障が無いエラー
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );
        break;
      default:
        // それ以外のエラーの場合はDSiカメラの起動を促して処理終了
        //OS_TPrintf( "それらしい文言を" );

        OS_Panic("failed TCL_LoadTable : %d\n", result );
        break;
    }

#else

    // 管理ファイルの読み込みに失敗した場合に復旧を試みる

    switch( result )
    {
    case TCL_RESULT_SUCCESS: break; // 成功
    case TCL_RESULT_ERROR_EXIST_OTHER_FILE:
    case TCL_RESULT_ERROR_ALREADY_MANAGED:
        // 写真の取得には支障が無いエラー
        OS_TPrintf("failed TCL_LoadTable : %d\n", result );
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

        if( result != TCL_RESULT_SUCCESS )
        {
            OS_Panic("failed TCL_CreateTable : %d\n", result );
        }
        break;
    }

#endif

    // SearchObjectを初期化
    TCL_InitSearchObject( &search_obj, TCL_SEARCH_CONDITION_PICTURE );

    // 全部で何枚写真があるか
    num_pict = TCL_GetNumPictures( &accessor, &search_obj );

    // テーブルを条件に合わせてソートしておく
    TCL_SortTable( &accessor,  sort_type );

    Print();

    DEMOStartDisplay();

    // テキスト表示を更新
    UpdateScreen();

    while (1)
    {
        // パッド情報の読み込みと操作
        DEMOReadKey();

        if( num_pict != 0 && DEMO_IS_TRIG( PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_R ) )
        {
            BOOL success_flag = FALSE;

            // Bボタンを押した場合はサムネイルを表示する
            BOOL thumbnail_flag = DEMO_IS_TRIG( PAD_BUTTON_B );

            BOOL delete_flag = DEMO_IS_TRIG( PAD_BUTTON_R );

            const TCLPictureInfo* pPicInfo = NULL;
            const TCLMakerNote* pMakerNote = NULL;

            MI_CpuClear( pict_path, TCL_PATH_LEN );

            if( search )
            {
                do
                {
                    draw_id = search_obj.startIdx;

                    // 写真情報を取得
                    result = TCL_SearchNextPictureInfo( &accessor,
                                                        &pPicInfo,
                                                        &search_obj );

                    if( result == TCL_RESULT_ERROR_NO_FIND_PICTURE )
                    {
                        // １週Searchし終えたので、SearchObjectを初期化
                        TCL_InitSearchObject( &search_obj, search_obj.condition );
                        OS_TPrintf("finished TCL_SearchNextPicturePath\n" );
                    }
                } while( result == TCL_RESULT_ERROR_NO_FIND_PICTURE );
            }
            else
            {
                result = TCL_SearchPictureInfoByIndex( &accessor,
                                                       &pPicInfo,
                                                       &search_obj,
                                                       draw_id );
            }

            if( result != TCL_RESULT_SUCCESS )
            {
                OS_TPrintf("failed TCL_Search* : %d\n", result );
            }
            else if( delete_flag )
            {
                TCLPictureInfo* pGetPicInfo = NULL;
                FSResult result;

                // テストのため写真のパスを取得してから TCLPictureInfo を取得しなおす
                ( void ) TCL_PrintPicturePath( pict_path, TCL_PATH_LEN, pPicInfo );
                if(TCL_GetPictureInfoFromPath( &accessor, &pGetPicInfo, (const char*)pict_path, (u32)STD_StrLen(pict_path) ) == FALSE)
                {
                    OS_TPrintf("failed TCL_PrintPictureInfo\n");
                }
                else
                {
                    // 削除してもよい写真データは、自社タイトルが作成したもののみとしてください。
                    // また、自アプリが作成したもののみに限定してもらっても問題ありません。
                    if(TCL_DeletePicture(&accessor, pGetPicInfo, &result) != TCL_RESULT_SUCCESS)
                        OS_TPrintf("failed TCL_DeletePicture\n");
                    
                    search_obj.condition = TCL_SEARCH_CONDITION_PICTURE;
                    sort_type = TCL_SORT_TYPE_DATE;

                    switch( condition_id )
                    {
                    case 0: break;
                    case 1:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_1;
                        sort_type = TCL_SORT_TYPE_FAVORITE_1;
                        break;
                    case 2:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_2;
                        sort_type = TCL_SORT_TYPE_FAVORITE_2;
                        break;
                    case 3:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_3;
                        sort_type = TCL_SORT_TYPE_FAVORITE_3;
                        break;
                    case 4:
                        search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_ALL;
                        sort_type = TCL_SORT_TYPE_FAVORITE_ALL;
                        break;
                    }

                    if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
                    num_pict = TCL_GetNumPictures( &accessor, &search_obj );
                    TCL_SortTable( &accessor, sort_type );
                    draw_id = 0;

                    Print();
                }
            }
            else
            {
                FSFile file;
                u32 size;
                s32 read_size;
                s16 width, height;
                TCLResult decodeResult;
                u32 decodeOptiion;

                // JPEG用バッファ
                u8*  pJpegBuf = NULL;
                u16* pDecodeTmpBuf = NULL;

                if( thumbnail_flag )
                {
                    width  = SSP_JPEG_THUMBNAIL_WIDTH;
                    height = SSP_JPEG_THUMBNAIL_HEIGHT;
                    decodeOptiion = SSP_JPEG_THUMBNAIL | SSP_JPEG_RGB555;
                }
                else
                {
                    width  = TCL_JPEG_WIDTH;
                    height = TCL_JPEG_HEIGHT;
                    decodeOptiion = SSP_JPEG_RGB555;
                }

                // 写真のパスを取得
                ( void ) TCL_PrintPicturePath( pict_path, TCL_PATH_LEN, pPicInfo );

                // 写真取得
                FS_InitFile( &file );
                if( FS_OpenFileEx( &file, pict_path, FS_FILEMODE_R ) )
                {
                    pJpegBuf = (u8*)OS_Alloc( TCL_MAX_JPEG_SIZE );

                    // 普通の写真か、サムネイルか、必要な分だけメモリ確保
                    pDecodeTmpBuf = (u16*)OS_Alloc( width * height * sizeof(u16) );

                    if( pJpegBuf == NULL || pDecodeTmpBuf == NULL )
                    {
                        OS_Panic("Cannot allocate memory!");
                    }

                    size = FS_GetFileLength( &file );
                    read_size = FS_ReadFile( &file, pJpegBuf, (s32)size );

                    if( read_size > 0 )
                    {
                        // JPEGデコード
                        decodeResult = TCL_DecodePicture( pJpegBuf,
                                                          size,
                                                          (u8*)pDecodeTmpBuf,
                                                          width, height,
                                                          decodeOptiion );

                        if( decodeResult == TCL_RESULT_SUCCESS )
                        {
                            // メーカーノートの取得
                            pMakerNote = ( TCLMakerNote* ) SSP_GetJpegDecoderMakerNoteAddrEx( SSP_MAKERNOTE_PHOTO );

                            // もし画像タイプが同じなら表示処理
                            if( TCL_IsSameImageType( pPicInfo, pMakerNote ) != FALSE )
                            {
                                MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );

                                // 画面サイズに縮小
                                DownSampling( pDecodeTmpBuf, width, height,
                                              draw_buffer, WIDTH, HEIGHT );

                                success_flag = TRUE;
                            }
                            else OS_TPrintf("Not Same Image Type\n");
                        }
                        else OS_TPrintf("failed JPEG Decode %d\n", decodeResult);
                    }
                    else OS_TPrintf("failed FS_ReadFile\n");

                    ( void ) FS_CloseFile( &file );

                    OS_Free( pJpegBuf );
                    OS_Free( pDecodeTmpBuf );
                }
                else OS_TPrintf("failed FS_Open\n");
            }

            if( !success_flag ) MI_CpuClearFast( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );
            DC_FlushRange( draw_buffer, WIDTH * HEIGHT * sizeof(u16) );

            Print();
        }
        else if( DEMO_IS_TRIG( PAD_BUTTON_X ) )
        {
            // 検索条件の変更
            if( ++condition_id > 4 ) condition_id = 0;

            search_obj.condition = TCL_SEARCH_CONDITION_PICTURE;
            sort_type = TCL_SORT_TYPE_DATE;

            switch( condition_id )
            {
            case 0: break;
            case 1:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_1;
                sort_type = TCL_SORT_TYPE_FAVORITE_1;
                break;
            case 2:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_2;
                sort_type = TCL_SORT_TYPE_FAVORITE_2;
                break;
            case 3:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_3;
                sort_type = TCL_SORT_TYPE_FAVORITE_3;
                break;
            case 4:
                search_obj.condition |= TCL_SEARCH_CONDITION_FAVORITE_ALL;
                sort_type = TCL_SORT_TYPE_FAVORITE_ALL;
                break;
            }

            if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
            num_pict = TCL_GetNumPictures( &accessor, &search_obj );
            TCL_SortTable( &accessor, sort_type );
            draw_id = 0;

            Print();
        }
        else if( DEMO_IS_TRIG( PAD_BUTTON_Y ) )
        {
            // 検索方法の変更
            search ^= 1;
            draw_id = 0;

            if( search ) TCL_InitSearchObject( &search_obj, search_obj.condition );
            Print();
        }
        else if( num_pict != 0 && !search )
        {
            // 表示する写真のIDの変更
            if( DEMO_IS_TRIG( PAD_KEY_RIGHT ) )
            {
                if( ++draw_id >= num_pict ) draw_id = 0;
                Print();
            }
            else if( DEMO_IS_TRIG( PAD_KEY_LEFT ) )
            {
                if( --draw_id < 0 ) draw_id = num_pict - 1;
                Print();
            }
        }

        OS_WaitVBlankIntr();

        // テキスト表示を更新
        UpdateScreen();

        GX_LoadBG3Bmp( draw_buffer, 0, WIDTH * HEIGHT * sizeof(u16) );
    }

    OS_Free( pTableBuf );
    OS_Free( pWorkBuf );

    OS_Terminate();
}
