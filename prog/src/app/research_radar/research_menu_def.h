/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面の定数定義
 * @file   research_menu_def.h
 * @author obata
 * @date   2010.02.13
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once


//===============================================================================
// ■BG
//===============================================================================

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
#define MAIN_BG_WINDOW (GFL_BG_FRAME2_M)  // ウィンドウ面
#define MAIN_BG_FONT   (GFL_BG_FRAME3_M)  // フォント面

// 表示優先順位
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
#define MAIN_BG_PALETTE_WINDOW_ON  (0xa)
#define MAIN_BG_PALETTE_WINDOW_OFF (0x7)
#define MAIN_BG_PALETTE_FONT       (0x8)
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
// 上画面 説明文ウィンドウ
#define CAPTION_WINDOW_X               (0)   // X座標   (キャラクター単位)
#define CAPTION_WINDOW_Y               (17)  // Y座標   (キャラクター単位)
#define CAPTION_WINDOW_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define CAPTION_WINDOW_HEIGHT          (6)   // Yサイズ (キャラクター単位)
#define CAPTION_WINDOW_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define CAPTION_WINDOW_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define CAPTION_WINDOW_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define CAPTION_WINDOW_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define CAPTION_WINDOW_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 タイトル ウィンドウ
#define TITLE_WINDOW_X               (2)   // X座標   (キャラクター単位)
#define TITLE_WINDOW_Y               (0)   // Y座標   (キャラクター単位)
#define TITLE_WINDOW_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define TITLE_WINDOW_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define TITLE_WINDOW_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TITLE_WINDOW_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TITLE_WINDOW_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define TITLE_WINDOW_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define TITLE_WINDOW_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 ボタン ( 調査内容の決定 )
#define CHANGE_BUTTON_X               (3)   // X座標   (キャラクター単位)
#define CHANGE_BUTTON_Y               (6)   // Y座標   (キャラクター単位)
#define CHANGE_BUTTON_WIDTH           (26)  // Xサイズ (キャラクター単位)
#define CHANGE_BUTTON_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define CHANGE_BUTTON_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define CHANGE_BUTTON_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define CHANGE_BUTTON_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define CHANGE_BUTTON_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define CHANGE_BUTTON_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 ボタン ( 調査報告を見る )
#define CHECK_BUTTON_X               (3)   // X座標   (キャラクター単位)
#define CHECK_BUTTON_Y               (13)  // Y座標   (キャラクター単位)
#define CHECK_BUTTON_WIDTH           (26)  // Xサイズ (キャラクター単位)
#define CHECK_BUTTON_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define CHECK_BUTTON_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define CHECK_BUTTON_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define CHECK_BUTTON_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define CHECK_BUTTON_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define CHECK_BUTTON_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号


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
// "new" アイコン
#define CLWK_NEW_ICON_POS_X (32)   // x 座標
#define CLWK_NEW_ICON_POS_Y (104)  // y 座標
#define CLWK_NEW_ICON_ANIME_SEQ (NANR_obj_new)  // アニメーションシーケンス
#define CLWK_NEW_ICON_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_NEW_ICON_BG_PRIORITY   (0)  // BG 優先順位
