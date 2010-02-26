////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内報告確認画面のインデックス定義
 * @file   research_check_index.h
 * @author obata
 * @date   2010.02.20
 */
////////////////////////////////////////////////////////////////
#pragma once


// 処理シーケンス
typedef enum {
  RESEARCH_CHECK_SEQ_SETUP,     // 準備シーケンス
  RESEARCH_CHECK_SEQ_KEY_WAIT,  // キー入力待ちシーケンス
  RESEARCH_CHECK_SEQ_ANALYZE,   // 解析シーケンス
  RESEARCH_CHECK_SEQ_FLASH,     // 画面フラッシュシーケンス
  RESEARCH_CHECK_SEQ_UPDATE,    // 更新シーケンス
  RESEARCH_CHECK_SEQ_FADE_OUT,  // フェードアウト
  RESEARCH_CHECK_SEQ_CLEAN_UP,  // 後片付けシーケンス
  RESEARCH_CHECK_SEQ_FINISH,    // 終了シーケンス
} RESEARCH_CHECK_SEQ;

// 選択項目
typedef enum {
  MENU_ITEM_QUESTION,  // 質問
  MENU_ITEM_ANSWER,    // 回答
  MENU_ITEM_MY_ANSWER, // 自分の回答
  MENU_ITEM_COUNT,     // 回答人数
  MENU_ITEM_NUM,       // 総数
} MENU_ITEM;

// タッチ範囲
typedef enum {
  TOUCH_AREA_CONTROL_CURSOR_L,// 左カーソル
  TOUCH_AREA_CONTROL_CURSOR_R,// 右カーソル
  TOUCH_AREA_QUESTION,        // 質問
  TOUCH_AREA_RETURN_BUTTON,   //「もどる」ボタン
  TOUCH_AREA_DUMMY,           // 終端コード
  TOUCH_AREA_NUM,             // 総数
} TOUCH_AREA_INDEX;

// 調査データの表示タイプ
typedef enum {
  DATA_DISP_TYPE_TODAY, // 今日のデータを表示 
  DATA_DISP_TYPE_TOTAL, // 合計のデータを表示
  DATA_DISP_TYPE_NUM,   // 総数
} DATA_DISP_TYPE;

// メッセージデータ
typedef enum {
  MESSAGE_STATIC,         // 定型文
  MESSAGE_QUESTIONNAIRE,  // アンケート関連
  MESSAGE_NUM,            // 総数
} MESSAGE_INDEX;

// BGFont オブジェクト
typedef enum {
  SUB_BG_FONT_TITLE,            // 上画面 タイトル
  SUB_BG_FONT_TOPIC_CAPTION,    // 上画面 調査項目の補足文
  SUB_BG_FONT_QUESTION_CAPTION, // 上画面 質問の補足文
  MAIN_BG_FONT_QUESTION,        // 下画面 質問
  MAIN_BG_FONT_ANSWER,          // 下画面 回答
  MAIN_BG_FONT_MY_ANSWER,       // 下画面 自分の回答
  MAIN_BG_FONT_COUNT,           // 下画面 回答人数
  MAIN_BG_FONT_NO_DATA,         // 下画面「ただいまちょうさちゅう」
  MAIN_BG_FONT_DATA_RECEIVING,  // 下画面「データしゅとくちゅう」
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
  CLUNIT_BMPOAM,   // BMP-OAM アクターで使用
  CLUNIT_NUM,      // 総数
} CLUNIT_INDEX;

// セルアクターワーク
typedef enum {
  CLWK_CONTROL_CURSOR_L, // 左カーソル
  CLWK_CONTROL_CURSOR_R, // 右カーソル
  CLWK_MY_ANSWER_ICON,   // 自分の回答アイコン
  CLWK_NUM,              // 総数
} CLWK_INDEX; 

// BMP-OAM
typedef enum {
  BMPOAM_ACTOR_ANALYZING,  //「…かいせきちゅう…」
  BMPOAM_ACTOR_NUM,        // 総数
} BMPOAM_ACTOR_INDEX;
