//============================================================================
/**
 *  @file   manual_top.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_TOP
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
  MANUAL_TOP_RESULT_CLOSE,
  MANUAL_TOP_RESULT_RETURN,
  MANUAL_TOP_RESULT_CATEGORY,
  MANUAL_TOP_RESULT_ALL,
}
MANUAL_TOP_RESULT;


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
  u8                 cursor_pos;
  // out 
  MANUAL_TOP_RESULT  result;
}
MANUAL_TOP_PARAM;

//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_TOP_WORK MANUAL_TOP_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_TOP_WORK*  MANUAL_TOP_Init(
    MANUAL_TOP_PARAM*    param,
    MANUAL_COMMON_WORK*  cmn_wk
);
// 終了処理
extern  void  MANUAL_TOP_Exit(
    MANUAL_TOP_WORK*     work
);
// 主処理(終了したときTRUEを返す)
extern  BOOL  MANUAL_TOP_Main(
    MANUAL_TOP_WORK*     work
);

