//============================================================================
/**
 *  @file   manual_touchbar.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_TOUCHBAR
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
// タイプ
typedef enum
{
  MANUAL_TOUCHBAR_TYPE_TOP,
  MANUAL_TOUCHBAR_TYPE_CATEGORY,
  MANUAL_TOUCHBAR_TYPE_TITLE,
  MANUAL_TOUCHBAR_TYPE_EXPLAIN,
  MANUAL_TOUCHBAR_TYPE_MAX,

  MANUAL_TOUCHBAR_TYPE_INIT        = MANUAL_TOUCHBAR_TYPE_MAX,      // 最初の1回のみ
}
MANUAL_TOUCHBAR_TYPE;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_TOUCHBAR_WORK MANUAL_TOUCHBAR_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_TOUCHBAR_WORK*  MANUAL_TOUCHBAR_Init(
    void*                    cmn_wk  // キャスト(MANUAL_COMMON_WORK*)して使用
);
// 終了処理
extern  void  MANUAL_TOUCHBAR_Exit(
    MANUAL_TOUCHBAR_WORK*     work
);
// 主処理
extern  void  MANUAL_TOUCHBAR_Main(
    MANUAL_TOUCHBAR_WORK*     work
);

// タイプ設定
extern void MANUAL_TOUCHBAR_SetType(
    MANUAL_TOUCHBAR_WORK*     work,
    MANUAL_TOUCHBAR_TYPE      type
);

// ページ設定(MANUAL_TOUCHBAR_TYPE_EXPLAINのときのみ有効)
extern void MANUAL_TOUCHBAR_SetPage(
    MANUAL_TOUCHBAR_WORK*     work,
    u8                        page_num,   // ページ数(0のときページ表示なし)
    u8                        page_no     // 現在のページ(0<=page_no<page_num)
);

// TOUCHBARを被せただけの関数
extern TOUCHBAR_ICON MANUAL_TOUCHBAR_GetTrg(
    MANUAL_TOUCHBAR_WORK*     work
);
extern TOUCHBAR_ICON MANUAL_TOUCHBAR_GetTouch(
    MANUAL_TOUCHBAR_WORK*     work
);

