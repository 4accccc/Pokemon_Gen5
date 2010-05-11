//============================================================================
/**
 *  @file   manual_title.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_TITLE
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
  MANUAL_TITLE_RESULT_RETURN,
  MANUAL_TITLE_RESULT_ITEM,
}
MANUAL_TITLE_RESULT;

typedef enum
{
  MANUAL_TITLE_TYPE_CATEGORY,
  MANUAL_TITLE_TYPE_ALL,
}
MANUAL_TITLE_TYPE;


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
  // in
  MANUAL_TITLE_TYPE    type;
  u16                  cate_no;      // typeがMANUAL_TITLE_TYPE_CATEGORYのときのみ使用する
  // in,out
  u16                  head_pos;     // 全項目の中での番号(画面の1番上(UP_HALFは除く)にある項目の番号)  // スクロールバーのカーソルの位置はこれで決まる
  u16                  cursor_pos;   // 全項目の中での番号
  // out 
  MANUAL_TITLE_RESULT  result;
  u16                  serial_no;    // resultがMANUAL_TITLE_RESULT_ITEMのときのみ使用する
}
MANUAL_TITLE_PARAM;

//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_TITLE_WORK MANUAL_TITLE_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_TITLE_WORK*  MANUAL_TITLE_Init(
    MANUAL_TITLE_PARAM*   param,
    MANUAL_COMMON_WORK*   cmn_wk
);
// 終了処理
extern  void  MANUAL_TITLE_Exit(
    MANUAL_TITLE_WORK*    work
);
// 主処理(終了したときTRUEを返す)
extern  BOOL  MANUAL_TITLE_Main(
    MANUAL_TITLE_WORK*    work
);

