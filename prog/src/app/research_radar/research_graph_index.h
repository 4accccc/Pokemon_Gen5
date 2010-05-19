////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー グラフ画面
 * @file   research_graph_index.h
 * @author obata
 * @date   2010.02.20
 */
////////////////////////////////////////////////////////////////
#pragma once


// グラフ画面の状態
typedef enum {
  RRG_STATE_SETUP,             // 準備
  RRG_STATE_INTRUDE_SHUTDOWN,  // 侵入切断
  RRG_STATE_STANDBY,           // 最初のキー入力待ち
  RRG_STATE_KEYWAIT,           // キー入力待ち
  RRG_STATE_ANALYZE,           // 解析
  RRG_STATE_PERCENTAGE,        // ％表示
  RRG_STATE_FLASHOUT,          // 画面フラッシュ ( アウト )
  RRG_STATE_FLASHIN,           // 画面フラッシュ ( イン )
  RRG_STATE_UPDATE_AT_STANDBY, // 更新 ( 最初のキー入力待ち時 )
  RRG_STATE_UPDATE_AT_KEYWAIT, // 更新 ( キー入力待ち時 )
  RRG_STATE_FADEOUT,           // フェードアウト
  RRG_STATE_WAIT,              // フレーム経過待ち
  RRG_STATE_CLEANUP,           // 後片付け
  RRG_STATE_FINISH,            // 終了
} RRG_STATE;

// 選択項目
typedef enum {
  MENU_ITEM_QUESTION,  // 質問
  MENU_ITEM_ANSWER,    // 回答
  MENU_ITEM_MY_ANSWER, // 自分の回答
  MENU_ITEM_COUNT,     // 回答人数
  MENU_ITEM_ANALYZE,   //「報告を見る」ボタン
  MENU_ITEM_NUM,       // 総数
} MENU_ITEM;

// タッチ範囲
typedef enum {
  TOUCH_AREA_CONTROL_CURSOR_L,// 左カーソル
  TOUCH_AREA_CONTROL_CURSOR_R,// 右カーソル
  TOUCH_AREA_QUESTION,        // 質問
  TOUCH_AREA_ANSWER,          // 回答
  TOUCH_AREA_MY_ANSWER,       // 自分の回答
  TOUCH_AREA_COUNT,           // 回答人数
  TOUCH_AREA_GRAPH,           // 円グラフ
  TOUCH_AREA_ANALYZE_BUTTON,  //「ほうこくをみる」ボタン
  TOUCH_AREA_DUMMY,           // 終端コード
  TOUCH_AREA_NUM,             // 総数
} TOUCH_AREA_INDEX;

// パレットアニメーション
typedef enum {
  PALETTE_ANIME_CURSOR_ON, // カーソルON
  PALETTE_ANIME_CURSOR_SET,// カーソルSET
  PALETTE_ANIME_SELECT,    //「ほうこくをみる」ボタン選択リアクション
  PALETTE_ANIME_ANALYZE_CURSOR_ON, //「ほうこくをみる」ボタンカーソルON
  PALETTE_ANIME_ANALYZE_CURSOR_SET, //「ほうこくをみる」ボタンカーソルSET
  PALETTE_ANIME_HOLD,      //「ほうこくをみる」ボタン暗転
  PALETTE_ANIME_RECOVER,   //「ほうこくをみる」ボタン復帰
  PALETTE_ANIME_RECEIVE_BUTTON, //「データじゅしんちゅう」ボタンフェード
  PALETTE_ANIME_NUM,       // 総数
} PALETTE_ANIME_INDEX;

// グラフの表示モード
typedef enum {
  GRAPH_DISP_MODE_TODAY, // 今日のデータを表示 
  GRAPH_DISP_MODE_TOTAL, // 合計のデータを表示
  GRAPH_DISP_MODE_NUM,   // 総数
} GRAPH_DISP_MODE;

// メッセージデータ
typedef enum {
  MESSAGE_STATIC,         // 定型文
  MESSAGE_QUESTIONNAIRE,  // アンケート関連
  MESSAGE_ANSWER,         // 回答
  MESSAGE_NUM,            // 総数
} MESSAGE_INDEX;

// BGFont オブジェクト
typedef enum {
  SUB_BG_FONT_TITLE,            // 上画面 タイトル
  SUB_BG_FONT_TOPIC_TITLE,      // 上画面 調査項目名
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
  OBJ_RESOURCE_MAIN_CHARACTER,      // MAIN-OBJ キャラクタ
  OBJ_RESOURCE_MAIN_PALETTE,        // MAIN-OBJ パレット
  OBJ_RESOURCE_MAIN_CELL_ANIME,     // MAIN-OBJ セル アニメ
  OBJ_RESOURCE_MAIN_COMMON_PALETTE, // MAIN-OBJ パレット ( 共通素材ボタン用 )
  OBJ_RESOURCE_SUB_CHARACTER,       // SUB-OBJ  キャラクタ
  OBJ_RESOURCE_SUB_PALETTE,         // SUB-OBJ  パレット
  OBJ_RESOURCE_SUB_CELL_ANIME,      // SUB-OBJ  セル アニメ
  OBJ_RESOURCE_NUM,                 // 総数
} OBJ_RESOURCE_ID;

// セルアクターユニット
typedef enum {
  CLUNIT_SUB_OBJ,    // SUB-OBJ
  CLUNIT_MAIN_OBJ,   // MAIN-OBJ
  CLUNIT_BMPOAM,     // BMP-OAM アクターで使用
  CLUNIT_PERCENTAGE, // 数値の表示に使用
  CLUNIT_NUM,        // 総数
} CLUNIT_INDEX;

// セルアクターワーク
typedef enum {
  CLWK_CONTROL_CURSOR_L,         // 左カーソル
  CLWK_CONTROL_CURSOR_R,         // 右カーソル
  CLWK_MY_ANSWER_ICON_ON_BUTTON, // 自分の回答アイコン ( ボタン上 )
  CLWK_MY_ANSWER_ICON_ON_GRAPH,  // 自分の回答アイコン ( グラフ上 )
  CLWK_NUM,                      // 総数
} CLWK_INDEX; 

// BMP-OAM
typedef enum {
  BMPOAM_ACTOR_ANALYZING,      //「…かいせきちゅう…」
  BMPOAM_ACTOR_ANALYZE_BUTTON, //「ほうこくをみる」ボタン
  BMPOAM_ACTOR_NUM,            // 総数
} BMPOAM_ACTOR_INDEX;
