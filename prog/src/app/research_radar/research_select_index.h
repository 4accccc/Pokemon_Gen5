////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面のインデックス定義
 * @file   research_select_index.h
 * @author obata
 * @date   2010.02.15
 */
////////////////////////////////////////////////////////////////
#pragma once


// 処理シーケンス
typedef enum {
  RESEARCH_SELECT_SEQ_SETUP,    // 準備
  RESEARCH_SELECT_SEQ_KEY_WAIT, // キー入力待ち
  RESEARCH_SELECT_SEQ_CLEAN_UP, // 後片付け
  RESEARCH_SELECT_SEQ_FINISH,   // 終了
} RESEARCH_SELECT_SEQ;

// 選択項目
typedef enum {
  MENU_ITEM_DETERMINATION_OK,     //「けってい」
  MENU_ITEM_DETERMINATION_CANCEL, //「やめる」
  MENU_ITEM_NUM,                  // 総数
} MENU_ITEM;

// メッセージデータ
typedef enum {
  MESSAGE_STATIC,         // 定型文
  MESSAGE_QUESTIONNAIRE,  // アンケート関連
  MESSAGE_NUM,            // 総数
} MESSAGE_INDEX;

// BGFont オブジェクト
typedef enum {
  BG_FONT_TITLE,      // 上画面 タイトル
  BG_FONT_DIRECTION,  // 上画面 説明文
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
