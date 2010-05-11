//============================================================================
/**
 *  @file   manual_category.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_CATEGORY
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
typedef enum
{
  MANUAL_CATEGORY_RESULT_RETURN,
  MANUAL_CATEGORY_RESULT_TITLE,
}
MANUAL_CATEGORY_RESULT;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// パラメータ
//=====================================
typedef struct
{
  // in,out
  u16                     head_pos;     // 全項目の中での番号(画面の1番上(UP_HALFは除く)にある項目の番号)  // スクロールバーのカーソルの位置はこれで決まる
  u16                     cursor_pos;   // 全項目の中での番号
  // out 
  MANUAL_CATEGORY_RESULT  result;
  u16                     cate_no;      // resultがMANUAL_CATEGORY_RESULT_TITLEのときのみ使用する
}
MANUAL_CATEGORY_PARAM;

//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_CATEGORY_WORK MANUAL_CATEGORY_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_CATEGORY_WORK*  MANUAL_CATEGORY_Init(
    MANUAL_CATEGORY_PARAM*   param,
    MANUAL_COMMON_WORK*      cmn_wk
);
// 終了処理
extern  void  MANUAL_CATEGORY_Exit(
    MANUAL_CATEGORY_WORK*    work
);
// 主処理(終了したときTRUEを返す)
extern  BOOL  MANUAL_CATEGORY_Main(
    MANUAL_CATEGORY_WORK*    work
);

