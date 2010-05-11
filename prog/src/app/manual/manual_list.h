//============================================================================
/**
 *  @file   manual_list.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_LIST
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
  MANUAL_LIST_RESULT_RETURN,  // タッチバーのリターンアイコンを選んで終了
  MANUAL_LIST_RESULT_ITEM,    // 項目のどれかを選んで終了
}
MANUAL_LIST_RESULT;

typedef enum
{
  MANUAL_LIST_ICON_NONE,    // アイコンなし
  MANUAL_LIST_ICON_NEW,     // NEWアイコンあり
}
MANUAL_LIST_ICON;

typedef enum
{
  MANUAL_LIST_TYPE_CATEGORY,
  MANUAL_LIST_TYPE_TITLE,
}
MANUAL_LIST_TYPE;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 項目の情報
//=====================================
typedef struct
{
  u16                    no;          // カテゴリのリストのときはcate_no、タイトルのリストのときはserial_no
  u16                    str_id;      // gmm中の表示する文章のID
  MANUAL_LIST_ICON       icon;
}
MANUAL_LIST_ITEM;

//-------------------------------------
/// パラメータ
//=====================================
typedef struct
{
  // in
  MANUAL_LIST_TYPE   type;        // タイプ
  u16                num;         // 全項目数
  MANUAL_LIST_ITEM*  item;        // 全項目のitem[num]
  // in,out
  u16                head_pos;    // 全項目の中での番号(画面の1番上(UP_HALFは除く)にある項目の番号)  // スクロールバーのカーソルの位置はこれで決まる
  u16                cursor_pos;  // 全項目の中での番号
  // out 
  MANUAL_LIST_RESULT result;
}
MANUAL_LIST_PARAM;


//-------------------------------------
/// ワーク
//=====================================
typedef struct _MANUAL_LIST_WORK MANUAL_LIST_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_LIST_WORK*  MANUAL_LIST_Init(
    MANUAL_LIST_PARAM*   param,
    MANUAL_COMMON_WORK*  cmn_wk
);
// 終了処理
extern  void  MANUAL_LIST_Exit(
    MANUAL_LIST_WORK*    work
);
// 主処理(終了したときTRUEを返す)
extern  BOOL  MANUAL_LIST_Main(
    MANUAL_LIST_WORK*    work
);

