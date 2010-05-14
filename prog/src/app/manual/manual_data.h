//============================================================================
/**
 *  @file   manual_data.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_DATA
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "gamesystem/gamedata_def.h"


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct  _MANUAL_DATA_WORK  MANUAL_DATA_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// Load処理
extern  MANUAL_DATA_WORK*  MANUAL_DATA_Load( ARCHANDLE* handle, HEAPID heap_id );
// Unload処理
extern  void               MANUAL_DATA_Unload( MANUAL_DATA_WORK* work );

// タイトルファイル
extern  u16  MANUAL_DATA_TitleGetTitleGmmId( MANUAL_DATA_WORK* work, u16 title_idx );
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトルのgmmのIDを戻り値として返す
extern  u16  MANUAL_DATA_TitleGetOpenFlag( MANUAL_DATA_WORK* work, u16 title_idx );
extern  u16  MANUAL_DATA_TitleGetReadFlag( MANUAL_DATA_WORK* work, u16 title_idx );
extern  u16  MANUAL_DATA_TitleGetPageNum( MANUAL_DATA_WORK* work, u16 title_idx );
extern  u16  MANUAL_DATA_TitleGetPageGmmId( MANUAL_DATA_WORK* work, u16 title_idx, u16 page_order );
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトル内におけるpage_order番目のページの、gmmのIDを戻り値として返す
extern  u16  MANUAL_DATA_TitleGetPageImageId( MANUAL_DATA_WORK* work, u16 title_idx, u16 page_order );
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトル内におけるpage_order番目のページの、画像のIDを戻り値として返す

// タイトル開始場所ファイル
extern  u16  MANUAL_DATA_TitleRefGetTotalTitleNum( MANUAL_DATA_WORK* work );

// カテゴリファイル
extern  u16  MANUAL_DATA_CateGetCateGmmId( MANUAL_DATA_WORK* work, u16 cate_idx );
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリのgmmのIDを戻り値として返す
extern  u16  MANUAL_DATA_CateGetTitleNum( MANUAL_DATA_WORK* work, u16 cate_idx );
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリ内におけるタイトルの個数を戻り値として返す
extern  u16  MANUAL_DATA_CateGetTitleIdx( MANUAL_DATA_WORK* work, u16 cate_idx, u16 title_order );
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリ内におけるtitle_order番目のタイトルの、全タイトルにおける出席番号を戻り値として返す

// カテゴリ開始場所ファイル
extern  u16  MANUAL_DATA_CateRefGetTotalCateNum( MANUAL_DATA_WORK* work );

// オープンフラグ
extern  BOOL MANUAL_DATA_OpenFlagIsOpen( MANUAL_DATA_WORK* work, u16 open_flag, GAMEDATA* gamedata );

// リードフラグ
extern  BOOL MANUAL_DATA_ReadFlagIsRead( MANUAL_DATA_WORK* work, u16 read_flag, GAMEDATA* gamedata );
extern  void MANUAL_DATA_ReadFlagSetRead( MANUAL_DATA_WORK* work, u16 read_flag, GAMEDATA* gamedata );

// 画像のID
extern  BOOL MANUAL_DATA_ImageIdIsValid( MANUAL_DATA_WORK* work, u16 image_id );  // 画像のIDが有効(画像あり)か無効(画像なし)かを返す(有効のときTRUE)
extern  u16  MANUAL_DATA_ImageIdGetNoImage( MANUAL_DATA_WORK* work );  // 画像なしのときに表示する画像のIDを得る

