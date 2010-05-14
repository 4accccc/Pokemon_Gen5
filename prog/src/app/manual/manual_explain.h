//============================================================================
/**
 *  @file   manual_explain.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_EXPLAIN
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "gamesystem/gamedata_def.h"

#include "manual_common.h"


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define MANUAL_EXPLAIN_PAGE_MAX  (9)         // 最大9ページ

typedef enum
{
  MANUAL_EXPLAIN_RESULT_RETURN,
}
MANUAL_EXPLAIN_RESULT;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 1ページの情報
//=====================================
typedef struct
{
  u16         image;       // 画像ファイルなしのときMANUAL_EXPLAIN_NO_IMAGE
  u16         str_id;      // gmm中の表示する文章のID
}
MANUAL_EXPLAIN_PAGE;

//-------------------------------------
/// パラメータ
//=====================================
typedef struct
{
  // in
  u8                     page_num;                       // ページ数  // page_num<=MANUAL_EXPLAIN_PAGE_MAX
  u16                    title_str_id;                   // gmm中の表示する文章のID  // タイトル
  MANUAL_EXPLAIN_PAGE    page[MANUAL_EXPLAIN_PAGE_MAX];  // 使用するのは添え字0<= <page_num
  // out 
  MANUAL_EXPLAIN_RESULT  result;
}
MANUAL_EXPLAIN_PARAM;

//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_EXPLAIN_WORK MANUAL_EXPLAIN_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_EXPLAIN_WORK*  MANUAL_EXPLAIN_Init(
    MANUAL_EXPLAIN_PARAM*    param,
    MANUAL_COMMON_WORK*      cmn_wk
);
// 終了処理
extern  void  MANUAL_EXPLAIN_Exit(
    MANUAL_EXPLAIN_WORK*     work
);
// 主処理(終了したときTRUEを返す)
extern  BOOL  MANUAL_EXPLAIN_Main(
    MANUAL_EXPLAIN_WORK*     work
);

