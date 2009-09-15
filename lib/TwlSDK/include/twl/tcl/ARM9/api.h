/*---------------------------------------------------------------------------*
  Project:  TwlSDK - TCL - include
  File:     api.h

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
#ifndef TWL_TCL_API_H_
#define TWL_TCL_API_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <nitro/fs.h>
#include <twl/tcl/ARM9/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*
  Name: TCL_GetTableBufferSize

  Description: 管理ファイルを読み込むのに必要なバッファサイズ

  Arguments: 

  Returns: バッファサイズ
*-------------------------------------------------------------------------*/
extern u32 TCL_GetTableBufferSize( void );

/*-------------------------------------------------------------------------*
  Name: TCL_GetWorkBufferSize

  Description: 管理ファイル操作に必要なワークサイズ

  Arguments: 

  Returns: バッファサイズ
*-------------------------------------------------------------------------*/
extern u32 TCL_GetWorkBufferSize( void );

/*-------------------------------------------------------------------------*
  Name: TCL_LoadTable

  Description: 管理テーブルを読み込む
               バッファは32バイトアライメントで確保して下さい

  Arguments: pAccessor       : アクセサ
             tableBuffer     : 読み込みバッファ
             tableBufferSize : バッファサイズ
                               ( TCL_GetTableBufferSize() で与えられるサイズ)
             workBuffer      : ワークバッファ
             workBufferSize  : ワークバッファサイズ
                               ( TCL_GetWorkBufferSize() で与えられるサイズ)
             pFSResult       : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_LoadTable( TCLAccessor* pAccessor ,
                                void* tableBuffer ,
                                u32 tableBufferSize ,
                                void* workBuffer ,
                                u32 workBufferSize ,
                                FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_CreateTable

  Description: 管理テーブルを構築する
               バッファは32バイトアライメントで確保して下さい

  Arguments: pAccessor       : アクセサ
             tableBuffer     : 読み込みバッファ
             tableBufferSize : バッファサイズ
                               ( TCL_GetTableBufferSize() で与えられるサイズ)
             workBuffer      : ワークバッファ
             workBufferSize  : ワークバッファサイズ
                               ( TCL_GetWorkBufferSize() で与えられるサイズ)
             pFSResult       : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_CreateTable( TCLAccessor* pAccessor ,
                                  void* tableBuffer ,
                                  u32 tableBufferSize ,
                                  void* workBuffer ,
                                  u32 workBufferSize ,
                                  FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_RepairTable

  Description: 管理テーブルを修復する

  Arguments: pAccessor : アクセサ
             pFSResult : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_RepairTable( TCLAccessor* pAccessor , FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_SortTable

  Description: 管理テーブルをソートする

  Arguments: pAccessor : アクセサ
             sortType  : ソートタイプ

  Returns: なし
*-------------------------------------------------------------------------*/
extern void TCL_SortTable( TCLAccessor* pAccessor , TCLSortType sortType );

/*-------------------------------------------------------------------------*
  Name: TCL_InitSearchObject

  Description: 検索オブジェクトの初期化

  Arguments: pSearchObj : 検索オブジェクト
             condition  : 検索条件

  Returns: なし
*-------------------------------------------------------------------------*/
extern void TCL_InitSearchObject( TCLSearchObject* pSearchObj , u32 condition );

/*-------------------------------------------------------------------------*
  Name: TCL_GetNumPictures

  Description: 検索条件に合致する写真が何枚あるか

  Arguments: pAccessor  : アクセサ
             pSearchObj : 検索オブジェクト

  Returns: 枚数
*-------------------------------------------------------------------------*/
extern int TCL_GetNumPictures( const TCLAccessor* pAccessor , const TCLSearchObject* pSearchObj );

/*-------------------------------------------------------------------------*
  Name: TCL_SearchNextPictureInfo

  Description: 検索条件に合致する、次の写真情報を取得する

  Arguments: pAccessor  : アクセサ
             ppPicInfo  : 写真情報格納先
             pSearchObj : 検索オブジェクト

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_SearchNextPictureInfo( const TCLAccessor* pAccessor ,
                                            const TCLPictureInfo** ppPicInfo ,
                                            TCLSearchObject* pSearchObj );

/*-------------------------------------------------------------------------*
  Name: TCL_SearchPictureInfoByIndex

  Description: 検索条件に合致する、
               指定されたインデクス番目の写真情報を取得する

  Arguments: pAccessor  : アクセサ
             ppPicInfo  : 写真情報格納先
             pSearchObj : 検索オブジェクト
             index      : インデクス

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_SearchPictureInfoByIndex( const TCLAccessor* pAccessor ,
                                               const TCLPictureInfo** ppPicInfo ,
                                               const TCLSearchObject* pSearchObj ,
                                               int index );

/*-------------------------------------------------------------------------*
  Name: TCL_SearchNextPicturePath

  Description: 検索条件に合致する、次の写真のパスを取得する

  Arguments: pAccessor  : アクセサ
             path       : パス格納先
             len        : パス長
             pSearchObj : 検索オブジェクト

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_SearchNextPicturePath( const TCLAccessor* pAccessor ,
                                            char* path ,
                                            size_t len ,
                                            TCLSearchObject* pSearchObj );

/*-------------------------------------------------------------------------*
  Name: TCL_SearchPicturePathByIndex

  Description: 検索条件に合致する、
               指定されたインデクス番目の写真のパスを取得する

  Arguments: pAccessor  : アクセサ
             path       : パス格納先
             len        : パス長
             pSearchObj : 検索オブジェクト
             index      : インデクス

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_SearchPicturePathByIndex( const TCLAccessor* pAccessor ,
                                               char* path ,
                                               size_t len ,
                                               const TCLSearchObject* pSearchObj ,
                                               int index );

/*-------------------------------------------------------------------------*
  Name: TCL_PrintPicturePath

  Description: 写真パスの取得

  Arguments: path     : パス格納先
             len      : パス長
             pPicInfo : 写真情報

  Returns: STD_TSNPrintf の返り値
*-------------------------------------------------------------------------*/
extern int TCL_PrintPicturePath( char* path ,
                                 size_t len ,
                                 const TCLPictureInfo* pPicInfo );

/*-------------------------------------------------------------------------*
  Name: TCL_GetPictureInfoFromPath

  Description: パスから写真情報を取得

  Arguments: pAccessor  : アクセサ
             pPicInfo   : 写真情報
             path       : パス格納先
             len        : パス長

  Returns: 取得できたならば TRUE を返す
*-------------------------------------------------------------------------*/
extern BOOL TCL_GetPictureInfoFromPath( const TCLAccessor* pAccessor ,
                          TCLPictureInfo** ppPicInfo ,
                          const char* path ,
                          size_t len );

/*-------------------------------------------------------------------------*
  Name: TCL_CalcNumEnableToTakePictures

  Description: 写真撮影可能枚数の計算

  Arguments: pAccessor : アクセサ

  Returns: 撮影可能枚数(負の場合は何らかの失敗)
*-------------------------------------------------------------------------*/
extern int TCL_CalcNumEnableToTakePictures( const TCLAccessor* pAccessor );

/*-------------------------------------------------------------------------*
  Name: TCL_GetJpegEncoderBufferSize

  Description: JPEGエンコードに必要なバッファサイズを返します
               内部で SSP_GetJpegEncoderBufferSize() を呼び出します

  Arguments: option : SSP_GetJpegEncoderBufferSize() に渡すオプション

  Returns: SSP_GetJpegEncoderBufferSize() の返り値
*-------------------------------------------------------------------------*/
extern u32 TCL_GetJpegEncoderBufferSize( u32 option );

/*-------------------------------------------------------------------------*
  Name: TCL_EncodeAndWritePicture

  Description: 画像のJPEGエンコードと保存、管理ファイルの更新
               内部で TCL_GetCurrentSecond() を呼び出します

  Arguments: pAccessor      : アクセサ
             imageBuffer    : 画像バッファ
             jpegBuffer     : JPEGバッファ
             jpegBufferSize : JPEGバッファサイズ
             workBuffer     : JPEGエンコードに必要なワークバッファ
             quality        : JPEGエンコードクオリティ
             option         : JPEGエンコードオプション
             pFSResult      : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_EncodeAndWritePicture( TCLAccessor* pAccessor ,
                                            const void* imageBuffer ,
                                            u8* jpegBuffer ,
                                            u32 jpegBufferSize ,
                                            u8* workBuffer ,
                                            u32 quality ,
                                            u32 option ,
                                            FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_EncodeAndWritePictureEx

  Description: 画像のJPEGエンコード(ユーザメーカーノートの設定)と保存、管理ファイルの更新
               内部で TCL_GetCurrentSecond() を呼び出します

  Arguments: pAccessor           : アクセサ
             imageBuffer         : 画像バッファ
             jpegBuffer          : JPEGバッファ
             jpegBufferSize      : JPEGバッファサイズ
             workBuffer          : JPEGエンコードに必要なワークバッファ
             quality             : JPEGエンコードクオリティ
             option              : JPEGエンコードオプション
             makerNoteBuffer     : ユーザメーカーノートバッファ
             makerNoteBufferSize : ユーザメーカーノートバッファサイズ
             pFSResult           : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_EncodeAndWritePictureEx( TCLAccessor* pAccessor ,
                                              const void* imageBuffer ,
                                              u8* jpegBuffer ,
                                              u32 jpegBufferSize ,
                                              u8* workBuffer ,
                                              u32 quality ,
                                              u32 option ,
                                              u8* makerNoteBuffer ,
                                              u16 makerNoteBufferSize ,
                                              FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_GetLastWrittenPicturePath

  Description: 最後に書き出した画像のパスを返す

  Arguments:

  Returns: 最後に書き出した画像のパス
*-------------------------------------------------------------------------*/
extern const char* TCL_GetLastWrittenPicturePath( void );

/*-------------------------------------------------------------------------*
  Name: TCL_GetLastWrittenPictureInfo

  Description: 最後に書き出した画像の TCLPictureInfo を返す

  Arguments:

  Returns: 最後に書き出した画像の TCLPictureInfo
*-------------------------------------------------------------------------*/
extern const TCLPictureInfo* TCL_GetLastWrittenPictureInfo( void );

/*-------------------------------------------------------------------------*
  Name: TCL_DecodePicture

  Description: JPEGデコードを行う

  Arguments: fileBuffer     : ファイルバッファ
             fileBufferSize : ファイルバッファサイズ
             imageBuffer    : 画像バッファ
             width          : デコード画像横幅
             height         : デコード画像縦幅
             decodeOption   : デコードオプション

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_DecodePicture( u8* fileBuffer ,
                                    u32 fileBufferSize ,
                                    void* imageBuffer ,
                                    s16 width ,
                                    s16 height ,
                                    u32 decodeOption );

/*-------------------------------------------------------------------------*
  Name: TCL_DeletePicture

  Description: JPEGファイルの削除を行う

  Arguments: pAccessor : アクセサ
             pPicInfo  : 写真情報
             pFSResult : FS関数でエラーが発生した場合の FSResult の格納先

  Returns: TCLResult
*-------------------------------------------------------------------------*/
extern TCLResult TCL_DeletePicture( TCLAccessor* pAccessor ,
                                    const TCLPictureInfo* pPicInfo ,
                                    FSResult* pFSResult );

/*-------------------------------------------------------------------------*
  Name: TCL_IsSameImageType

  Description: 画像タイプが同じとみなせるかどうか

  Arguments: pPicInfo   : 写真情報
             pMakerNote : メーカーノート

  Returns: 同じとみなせれば TRUE
*-------------------------------------------------------------------------*/
extern BOOL TCL_IsSameImageType( const TCLPictureInfo* pPicInfo , const TCLMakerNote* pMakerNote );

/*-------------------------------------------------------------------------*
  Name: TCL_GetCurrentSecond

  Description: 現在時刻(秒)を取得する
               内部で RTC_GetDateTime() を呼び出します
               この関数は weak シンボルで定義されています
               RTC への排他処理が必要な場合などは再定義してください

  Arguments:

  Returns: 現在時刻(秒)
*-------------------------------------------------------------------------*/
extern s64 TCL_GetCurrentSecond( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_TCL_API_H_ */
#endif
