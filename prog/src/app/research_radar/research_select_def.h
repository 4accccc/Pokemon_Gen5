/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面 インデックス定義
 * @file   research_select_def.h
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once


//===============================================================================
// ■動作
//===============================================================================
#define SELECT_TOPIC_MAX_NUM (3)  // 選択する調査項目の数

// 調査項目のスクロール
#define SCROLL_TOP_MARGIN    (24)  // 上方スクロールの余白
#define SCROLL_BOTTOM_MARGIN (48)  // 下方スクロールの余白
#define SCROLL_FRAME         (4)   // スクロールに要するフレーム数

// スクロールバーつまみ部分
#define SCROLL_CONTROL_LEFT     (248)  // x 座標
#define SCROLL_CONTROL_TOP      (32)   // 上辺 y 座標
#define SCROLL_CONTROL_BOTTOM   (160)  // 底辺 y 座標
#define SCROLL_CONTROL_STEP_NUM (TOPIC_ID_NUM)  // 移動範囲の分割数

// 調査項目選択アイコン
#define SELECT_ICON_DRAW_OFFSET_X (0)  // 調査項目ボタン左上x座標からのオフセット
#define SELECT_ICON_DRAW_OFFSET_Y (10) // 調査項目ボタン左上y座標からのオフセット


//===============================================================================
// ■BG
//===============================================================================

//-------------
// □ウィンドウ
//-------------
#define WND0_LEFT   (0)    // 左上 x 座標
#define WND0_TOP    (24)   // 左上 y 座標
#define WND0_RIGHT  (255)  // 右下 x 座標
#define WND0_BOTTOM (168)  // 右下 y 座標

//----------
// □SUB-BG
//----------
// BG フレーム
#define SUB_BG_WINDOW (GFL_BG_FRAME2_S)  // ウィンドウ面
#define SUB_BG_FONT   (GFL_BG_FRAME3_S)  // フォント面

// 表示優先順位
#define SUB_BG_PRIORITY_WINDOW (1)  // ウィンドウ面
#define SUB_BG_PRIORITY_FONT   (0)  // フォント面

// αブレンディング
#define SUB_BG_BLEND_TARGET_1 (GX_BLEND_PLANEMASK_BG1)  // 第一対象面
#define SUB_BG_BLEND_TARGET_2 (GX_BLEND_PLANEMASK_BG0)  // 第二対象面
#define SUB_BG_BLEND_WEIGHT_1 (7)   // 第一対象面のブレンディング係数
#define SUB_BG_BLEND_WEIGHT_2 (15)  // 第二対象面のブレンディング係数

//-----------
// □MAIN-BG
//-----------
// BG フレーム
#define MAIN_BG_BAR    (GFL_BG_FRAME1_M)  // バー面
#define MAIN_BG_WINDOW (GFL_BG_FRAME2_M)  // ウィンドウ面
#define MAIN_BG_FONT   (GFL_BG_FRAME3_M)  // フォント面

// 表示優先順位
#define MAIN_BG_BAR_PRIORITY    (3)  // バー面
#define MAIN_BG_WINDOW_PRIORITY (2)  // ウィンドウ面
#define MAIN_BG_FONT_PRIORITY   (1)  // フォント面

// αブレンディング
#define MAIN_BG_BLEND_TARGET_1 (GX_BLEND_PLANEMASK_BG2)  // 第一対象面
#define MAIN_BG_BLEND_TARGET_2 (GX_BLEND_PLANEMASK_BG1)  // 第二対象面
#define MAIN_BG_BLEND_WEIGHT_1 (31)  // 第一対象面のブレンディング係数
#define MAIN_BG_BLEND_WEIGHT_2 (31)  // 第二対象面のブレンディング係数

//----------------
// □パレット番号
//----------------
// MAIN-BG
#define MAIN_BG_PALETTE_BACK_0     (0x0)
#define MAIN_BG_PALETTE_BACK_1     (0x1)
#define MAIN_BG_PALETTE_BACK_2     (0x2)
#define MAIN_BG_PALETTE_BACK_3     (0x3)
#define MAIN_BG_PALETTE_BACK_4     (0x4)
#define MAIN_BG_PALETTE_BACK_5     (0x5)
#define MAIN_BG_PALETTE_FONT       (0x8)
#define MAIN_BG_PALETTE_WINDOW_OFF (0x9)
#define MAIN_BG_PALETTE_WINDOW_ON  (0xa)
// SUB-BG
#define SUB_BG_PALETTE_BACK_0 (0x0)
#define SUB_BG_PALETTE_BACK_1 (0x1)
#define SUB_BG_PALETTE_BACK_2 (0x2)
#define SUB_BG_PALETTE_BACK_3 (0x3)
#define SUB_BG_PALETTE_BACK_4 (0x4)
#define SUB_BG_PALETTE_BACK_5 (0x5)
#define SUB_BG_PALETTE_WINDOW (0x6)
#define SUB_BG_PALETTE_FONT   (0x7)

//---------------------
// □ウィンドウ/ボタン
//---------------------
// 上画面 タイトル ウィンドウ
#define TITLE_WINDOW_X               (2)   // X座標   (キャラクター単位)
#define TITLE_WINDOW_Y               (0)   // Y座標   (キャラクター単位)
#define TITLE_WINDOW_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define TITLE_WINDOW_HEIGHT          (3)   // Yサイズ (キャラクター単位)
#define TITLE_WINDOW_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TITLE_WINDOW_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TITLE_WINDOW_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define TITLE_WINDOW_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define TITLE_WINDOW_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 調査項目名
#define TOPIC_TITLE_X               (0)   // X座標   (キャラクター単位)
#define TOPIC_TITLE_Y               (3)   // Y座標   (キャラクター単位)
#define TOPIC_TITLE_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define TOPIC_TITLE_HEIGHT          (3)   // Yサイズ (キャラクター単位)
#define TOPIC_TITLE_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TOPIC_TITLE_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TOPIC_TITLE_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define TOPIC_TITLE_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define TOPIC_TITLE_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 調査項目の補足文
#define TOPIC_CAPTION_X               (0)   // X座標   (キャラクター単位)
#define TOPIC_CAPTION_Y               (6)   // Y座標   (キャラクター単位)
#define TOPIC_CAPTION_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define TOPIC_CAPTION_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define TOPIC_CAPTION_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TOPIC_CAPTION_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TOPIC_CAPTION_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define TOPIC_CAPTION_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define TOPIC_CAPTION_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 質問1
#define QUESTION_1_X               (0)   // X座標   (キャラクター単位)
#define QUESTION_1_Y               (10)  // Y座標   (キャラクター単位)
#define QUESTION_1_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define QUESTION_1_HEIGHT          (2)   // Yサイズ (キャラクター単位)
#define QUESTION_1_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define QUESTION_1_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define QUESTION_1_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define QUESTION_1_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define QUESTION_1_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 質問2
#define QUESTION_2_X               (0)   // X座標   (キャラクター単位)
#define QUESTION_2_Y               (12)  // Y座標   (キャラクター単位)
#define QUESTION_2_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define QUESTION_2_HEIGHT          (2)   // Yサイズ (キャラクター単位)
#define QUESTION_2_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define QUESTION_2_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define QUESTION_2_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define QUESTION_2_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define QUESTION_2_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 質問3
#define QUESTION_3_X               (0)   // X座標   (キャラクター単位)
#define QUESTION_3_Y               (14)  // Y座標   (キャラクター単位)
#define QUESTION_3_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define QUESTION_3_HEIGHT          (2)   // Yサイズ (キャラクター単位)
#define QUESTION_3_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define QUESTION_3_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define QUESTION_3_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define QUESTION_3_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define QUESTION_3_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 説明文 ウィンドウ
#define DIRECTION_WINDOW_X               (0)   // X座標   (キャラクター単位)
#define DIRECTION_WINDOW_Y               (18)  // Y座標   (キャラクター単位)
#define DIRECTION_WINDOW_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define DIRECTION_WINDOW_HEIGHT          (6)   // Yサイズ (キャラクター単位)
#define DIRECTION_WINDOW_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define DIRECTION_WINDOW_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define DIRECTION_WINDOW_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define DIRECTION_WINDOW_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define DIRECTION_WINDOW_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 調査項目ボタン ( 先頭の項目 / 非選択時 )
#define TOPIC_BUTTON_X               (2)   // X座標   (キャラクター単位)
#define TOPIC_BUTTON_Y               (0)   // Y座標   (キャラクター単位)
#define TOPIC_BUTTON_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define TOPIC_BUTTON_HEIGHT          (3)   // Yサイズ (キャラクター単位)
#define TOPIC_BUTTON_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TOPIC_BUTTON_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TOPIC_BUTTON_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define TOPIC_BUTTON_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define TOPIC_BUTTON_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号


//===============================================================================
// ■OBJ
//===============================================================================

//------------------------
// □セルアクターユニット
//------------------------
// SUB-OBJ
#define CLUNIT_SUB_OBJ_WORK_SIZE (10)  // 最大ワーク数
#define CLUNIT_SUB_OBJ_PRIORITY  (0)   // 描画優先順位

// MAIN-OBJ
#define CLUNIT_MAIN_OBJ_WORK_SIZE (10)  // 最大ワーク数
#define CLUNIT_MAIN_OBJ_PRIORITY  (0)   // 描画優先順位

//----------------------
// □セルアクターワーク
//----------------------
// スクロールバー
#define CLWK_SCROLL_CONTROL_POS_X (0)  // x 座標
#define CLWK_SCROLL_CONTROL_POS_Y (0)  // y 座標
#define CLWK_SCROLL_CONTROL_ANIME_SEQ (NANR_obj_bar)  // アニメーションシーケンス
#define CLWK_SCROLL_CONTROL_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_SCROLL_CONTROL_BG_PRIORITY   (0)  // BG 優先順位

// 調査項目選択アイコン
#define CLWK_SELECT_ICON_POS_X (0)  // x 座標
#define CLWK_SELECT_ICON_POS_Y (0)  // y 座標
#define CLWK_SELECT_ICON_ANIME_SEQ (NANR_obj_search)  // アニメーションシーケンス
#define CLWK_SELECT_ICON_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_SELECT_ICON_BG_PRIORITY   (0)  // BG 優先順位
