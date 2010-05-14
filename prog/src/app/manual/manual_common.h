//============================================================================
/**
 *  @file   manual_common.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_COMMON
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "gamesystem/gamedata_def.h"

#include "manual_data.h"
#include "manual_touchbar.h"


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
typedef struct
{
  // MANUALを呼んだ場所からの借り物
  GAMEDATA*                   gamedata;
 
  // MANUALで生成したもの
  HEAPID                      heap_id;
  
  // グラフィック、フォントなど
  MANUAL_GRAPHIC_WORK*        graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que_title;  // タイトルのprint_que
  PRINT_QUE*                  print_que_main;   // リストや本文のprint_que
  PRINT_QUE*                  print_que_tb;     // タッチバーのprint_que

  // メッセージ
  GFL_MSGDATA*                msgdata_system;  // システムのメッセージ
  GFL_MSGDATA*                msgdata_main;    // カテゴリやタイトル、本文のメッセージ

  // ファイルハンドル
  ARCHANDLE*                  handle_system;   // システムのファイルハンドル
  ARCHANDLE*                  handle_explain;  // 説明画面のファイルハンドル

  // マニュアルのデータ
  MANUAL_DATA_WORK*           data_wk;

  // 上画面に表示しているダイレクトカラー画像のID
  u16                         bg_m_dcbmp_id;  // MANUAL_BG_M_DCBMP_NO_IMAGEのとき画像ファイルを表示しておらず、真っ暗

  // マニュアルタッチバー
  MANUAL_TOUCHBAR_WORK*       mtb_wk;  // MANUAL_TOUCHBAR_Mainは各画面で呼んで下さい
}
MANUAL_COMMON_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
// 初期化処理
extern  MANUAL_COMMON_WORK*  MANUAL_COMMON_Init(
    GAMEDATA*  gamedata,
    HEAPID     heap_id
);
// 終了処理
extern  void  MANUAL_COMMON_Exit(
    MANUAL_COMMON_WORK*  work
);
// 主処理
extern  void  MANUAL_COMMON_Main(
    MANUAL_COMMON_WORK*  work
);

// 上画面にダイレクトカラー画像を表示する
extern  void  MANUAL_COMMON_DrawBgMDcbmp(
    MANUAL_COMMON_WORK*  work,
    u16                  bg_m_dcbmp_id
);

