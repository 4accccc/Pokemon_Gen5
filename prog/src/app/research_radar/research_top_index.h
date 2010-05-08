////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面のインデックス定義
 * @file   research_top_index.h
 * @author obata
 * @date   2010.02.13
 */
////////////////////////////////////////////////////////////////
#pragma once


// 処理シーケンス
typedef enum {
  RRT_STATE_SETUP,    // 準備
  RRT_STATE_STANDBY,  // 最初のキー入力待ち
  RRT_STATE_KEYWAIT,  // キー入力待ち
  RRT_STATE_WAIT,     // フレーム経過待ち
  RRT_STATE_FADEOUT,  // フェードアウト
  RRT_STATE_CLEANUP,  // 後片付け
  RRT_STATE_FINISH,   // 終了
  RRT_STATE_NUM,
} RRT_STATE;

// メッセージデータ
typedef enum {
  MESSAGE_STATIC,         // 定型文
  MESSAGE_QUESTIONNAIRE,  // アンケート関連
  MESSAGE_NUM,            // 総数
} MESSAGE_INDEX;

// メニュー項目
typedef enum {
  MENU_ITEM_CHANGE_RESEARCH,  //「調査内容を変更する」
  MENU_ITEM_CHECK_RESEARCH,   //「調査報告を確認する」
  MENU_ITEM_NUM,              // メニュー項目数
} MENU_ITEM;

// タッチ範囲
typedef enum {
  TOUCH_AREA_CHANGE_BUTTON,  //「調査内容を変更する」
  TOUCH_AREA_CHECK_BUTTON,   //「調査報告を確認する」
  TOUCH_AREA_DUMMY,          // 終端コード
  TOUCH_AREA_NUM,            // 総数
} TOUCH_AREA_INDEX;

// パレットアニメーション ( プライオリティ逆順 )
typedef enum {
  PALETTE_ANIME_CURSOR_ON,   // カーソルON
  PALETTE_ANIME_CURSOR_SET,  // カーソルセット時の点滅
  PALETTE_ANIME_SELECT,      // 選択時の点滅
  PALETTE_ANIME_CHANGE_BUTTON_HOLD_WINDOW, //『調査を決める』ボタンを非アクティブにする 
  PALETTE_ANIME_CHANGE_BUTTON_HOLD_FONT, //『調査を決める』ボタンを非アクティブにする 
  PALETTE_ANIME_NUM,         // 総数
} PALETTE_ANIME_INDEX;

// BGFont オブジェクト
typedef enum {
  BG_FONT_CAPTION,       // 上画面 説明文
  BG_FONT_TITLE,         // 下画面 タイトル
  BG_FONT_CHANGE_BUTTON, // 下画面「調査内容の決定」
  BG_FONT_CHECK_BUTTON,  // 下画面「調査報告の確認」
  BG_FONT_NUM,
} BG_FONT_INDEX;

// OBJ リソースID
typedef enum {
  OBJ_RESOURCE_MAIN_CHARACTER,  // MAIN-OBJ キャラクタ
  OBJ_RESOURCE_MAIN_PALETTE,    // MAIN-OBJ パレット
  OBJ_RESOURCE_MAIN_CELL_ANIME, // MAIN-OBJ セル アニメ
  OBJ_RESOURCE_SUB_CHARACTER,   // SUB-OBJ  キャラクタ
  OBJ_RESOURCE_SUB_PALETTE,     // SUB-OBJ  パレット
  OBJ_RESOURCE_SUB_CELL_ANIME,  // SUB-OBJ  セル アニメ
  OBJ_RESOURCE_NUM,             // 総数
} OBJ_RESOURCE_ID;

// セルアクターユニット
typedef enum {
  CLUNIT_SUB_OBJ,  // SUB-OBJ
  CLUNIT_MAIN_OBJ, // MAIN-OBJ
  CLUNIT_NUM,      // 総数
} CLUNIT_INDEX;

// セルアクターワーク
typedef enum {
  CLWK_NEW_ICON,  // "new" アイコン
  CLWK_NUM,       // 総数
} CLWK_INDEX; 
