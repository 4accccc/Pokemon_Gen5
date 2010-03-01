/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査報告確認画面 インデックス定義
 * @file   research_check_def.h
 * @author obata
 * @date   2010.02.20
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
#define MAIN_BG_3D     (GFL_BG_FRAME0_M)  // 3D面
#define MAIN_BG_WINDOW (GFL_BG_FRAME2_M)  // ウィンドウ面
#define MAIN_BG_FONT   (GFL_BG_FRAME3_M)  // フォント面

// 表示優先順位
#define MAIN_BG_3D_PRIORITY     (2)  // 3D面
#define MAIN_BG_WINDOW_PRIORITY (1)  // ウィンドウ面
#define MAIN_BG_FONT_PRIORITY   (0)  // フォント面

// αブレンディング
#define MAIN_BG_BLEND_TARGET_1 (GX_BLEND_PLANEMASK_BG2)  // 第一対象面
#define MAIN_BG_BLEND_TARGET_2 (GX_BLEND_PLANEMASK_BG1)  // 第二対象面
#define MAIN_BG_BLEND_WEIGHT_1 (16)  // 第一対象面のブレンディング係数
#define MAIN_BG_BLEND_WEIGHT_2 (5)  // 第二対象面のブレンディング係数

//----------------
// □パレット番号
//----------------
// MAIN-BG
#define MAIN_BG_PALETTE_BACK_0   (0x0)
#define MAIN_BG_PALETTE_BACK_1   (0x1)
#define MAIN_BG_PALETTE_BACK_2   (0x2)
#define MAIN_BG_PALETTE_BACK_3   (0x3)
#define MAIN_BG_PALETTE_BACK_4   (0x4)
#define MAIN_BG_PALETTE_BACK_5   (0x5)
#define MAIN_BG_PALETTE_MENU_OFF (0x6)
#define MAIN_BG_PALETTE_MENU_ON  (0xa)
#define MAIN_BG_PALETTE_FONT     (0xf)
// SUB-BG
#define SUB_BG_PALETTE_BACK_0 (0x0)
#define SUB_BG_PALETTE_BACK_1 (0x1)
#define SUB_BG_PALETTE_BACK_2 (0x2)
#define SUB_BG_PALETTE_BACK_3 (0x3)
#define SUB_BG_PALETTE_BACK_4 (0x4)
#define SUB_BG_PALETTE_BACK_5 (0x5)
#define SUB_BG_PALETTE_WINDOW (0x6)
#define SUB_BG_PALETTE_FONT   (0xf)

//---------------------
// □ウィンドウ/ボタン
//---------------------
// 上画面 タイトル
#define TITLE_X               (2)   // X座標   (キャラクター単位)
#define TITLE_Y               (0)   // Y座標   (キャラクター単位)
#define TITLE_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define TITLE_HEIGHT          (3)   // Yサイズ (キャラクター単位)
#define TITLE_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TITLE_STRING_OFFSET_Y (4)   // 文字列の書き込み先オフセットY
#define TITLE_STRING_COLOR_L  (3)   // フォント (文字) のカラー番号
#define TITLE_STRING_COLOR_S  (4)   // フォント (影)　 のカラー番号
#define TITLE_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 調査項目の補足文
#define TOPIC_CAPTION_X               (0)   // X座標   (キャラクター単位)
#define TOPIC_CAPTION_Y               (18)  // Y座標   (キャラクター単位)
#define TOPIC_CAPTION_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define TOPIC_CAPTION_HEIGHT          (2)   // Yサイズ (キャラクター単位)
#define TOPIC_CAPTION_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define TOPIC_CAPTION_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define TOPIC_CAPTION_STRING_COLOR_L  (3)   // フォント (文字) のカラー番号
#define TOPIC_CAPTION_STRING_COLOR_S  (4)   // フォント (影)　 のカラー番号
#define TOPIC_CAPTION_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 上画面 質問の補足文
#define QUESTION_CAPTION_X               (0)   // X座標   (キャラクター単位)
#define QUESTION_CAPTION_Y               (20)  // Y座標   (キャラクター単位)
#define QUESTION_CAPTION_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define QUESTION_CAPTION_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define QUESTION_CAPTION_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define QUESTION_CAPTION_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define QUESTION_CAPTION_STRING_COLOR_L  (3)   // フォント (文字) のカラー番号
#define QUESTION_CAPTION_STRING_COLOR_S  (4)   // フォント (影)　 のカラー番号
#define QUESTION_CAPTION_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 質問
#define QUESTION_X               (2)   // X座標   (キャラクター単位)
#define QUESTION_Y               (0)   // Y座標   (キャラクター単位)
#define QUESTION_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define QUESTION_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define QUESTION_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define QUESTION_STRING_OFFSET_Y (5)   // 文字列の書き込み先オフセットY
#define QUESTION_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define QUESTION_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define QUESTION_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号
#define QUESTION_LEFT_CURSOR_OFFSET_X  (0) // 左カーソル x オフセット
#define QUESTION_LEFT_CURSOR_OFFSET_Y  (12) // 左カーソル y オフセット
#define QUESTION_RIGHT_CURSOR_OFFSET_X (0) // 右カーソル x オフセット
#define QUESTION_RIGHT_CURSOR_OFFSET_Y (12) // 右カーソル y オフセット

// 下画面 回答 ( 上下2分割 )
#define ANSWER_UPPER_X         (13)  // X座標   (キャラクター単位, 上半分)
#define ANSWER_UPPER_Y         (4)   // Y座標   (キャラクター単位, 上半分)
#define ANSWER_UPPER_WIDTH     (19)  // Xサイズ (キャラクター単位, 上半分)
#define ANSWER_UPPER_HEIGHT    (4)   // Yサイズ (キャラクター単位, 上半分)
#define ANSWER_LOWER_X         (16)  // X座標   (キャラクター単位, 下半分)
#define ANSWER_LOWER_Y         (8)   // Y座標   (キャラクター単位, 下半分)
#define ANSWER_LOWER_WIDTH     (16)  // Xサイズ (キャラクター単位, 下半分)
#define ANSWER_LOWER_HEIGHT    (3)   // Yサイズ (キャラクター単位, 下半分)
#define ANSWER_STRING_OFFSET_X (24)  // 文字列の書き込み先オフセットX
#define ANSWER_STRING_OFFSET_Y (4)   // 文字列の書き込み先オフセットY
#define ANSWER_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define ANSWER_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define ANSWER_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号
#define ANSWER_LEFT_CURSOR_OFFSET_X  (0)  // 左カーソル x オフセット
#define ANSWER_LEFT_CURSOR_OFFSET_Y  (32) // 左カーソル y オフセット
#define ANSWER_RIGHT_CURSOR_OFFSET_X (0)  // 右カーソル x オフセット
#define ANSWER_RIGHT_CURSOR_OFFSET_Y (32) // 右カーソル y オフセット

// 下画面 自分の回答
#define MY_ANSWER_X               (17)  // X座標   (キャラクター単位)
#define MY_ANSWER_Y               (11)  // Y座標   (キャラクター単位)
#define MY_ANSWER_WIDTH           (15)  // Xサイズ (キャラクター単位)
#define MY_ANSWER_HEIGHT          (5)   // Yサイズ (キャラクター単位)
#define MY_ANSWER_STRING_OFFSET_X (12)  // 文字列の書き込み先オフセットX
#define MY_ANSWER_STRING_OFFSET_Y (4)   // 文字列の書き込み先オフセットY
#define MY_ANSWER_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define MY_ANSWER_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define MY_ANSWER_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号
#define MY_ANSWER_LEFT_CURSOR_OFFSET_X  (256) // 左カーソル x オフセット
#define MY_ANSWER_LEFT_CURSOR_OFFSET_Y  (256) // 左カーソル y オフセット
#define MY_ANSWER_RIGHT_CURSOR_OFFSET_X (256) // 右カーソル x オフセット
#define MY_ANSWER_RIGHT_CURSOR_OFFSET_Y (256) // 右カーソル y オフセット

// 下画面 回答人数
#define COUNT_X               (13)  // X座標   (キャラクター単位)
#define COUNT_Y               (16)  // Y座標   (キャラクター単位)
#define COUNT_WIDTH           (19)  // Xサイズ (キャラクター単位)
#define COUNT_HEIGHT          (5)   // Yサイズ (キャラクター単位)
#define COUNT_STRING_OFFSET_X (34)  // 文字列の書き込み先オフセットX
#define COUNT_STRING_OFFSET_Y (4)   // 文字列の書き込み先オフセットY
#define COUNT_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define COUNT_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define COUNT_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号
#define COUNT_LEFT_CURSOR_OFFSET_X  (0)  // 左カーソル x オフセット
#define COUNT_LEFT_CURSOR_OFFSET_Y  (20) // 左カーソル y オフセット
#define COUNT_RIGHT_CURSOR_OFFSET_X (0)  // 右カーソル x オフセット
#define COUNT_RIGHT_CURSOR_OFFSET_Y (20) // 右カーソル y オフセット

// 下画面「ただいまちょうさちゅう」
#define NO_DATA_X               (2)   // X座標   (キャラクター単位)
#define NO_DATA_Y               (7)   // Y座標   (キャラクター単位)
#define NO_DATA_WIDTH           (13)  // Xサイズ (キャラクター単位)
#define NO_DATA_HEIGHT          (5)   // Yサイズ (キャラクター単位)
#define NO_DATA_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define NO_DATA_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define NO_DATA_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define NO_DATA_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define NO_DATA_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面「データじゅしんちゅう」
#define DATA_RECEIVING_X               (13)  // X座標   (キャラクター単位)
#define DATA_RECEIVING_Y               (4)   // Y座標   (キャラクター単位)
#define DATA_RECEIVING_WIDTH           (28)  // Xサイズ (キャラクター単位)
#define DATA_RECEIVING_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define DATA_RECEIVING_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define DATA_RECEIVING_STRING_OFFSET_Y (4)   // 文字列の書き込み先オフセットY
#define DATA_RECEIVING_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define DATA_RECEIVING_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define DATA_RECEIVING_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

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
#define CLUNIT_MAIN_OBJ_PRIORITY  (1)   // 描画優先順位

// BMP-OAM
#define CLUNIT_BMPOAM_WORK_SIZE (30)  // 最大ワーク数
#define CLUNIT_BMPOAM_PRIORITY  (0)   // 描画優先順位

//----------------------
// □セルアクターワーク
//----------------------
// 左カーソル
#define CLWK_CONTROL_CURSOR_L_POS_X (0)  // x 座標
#define CLWK_CONTROL_CURSOR_L_POS_Y (0)  // y 座標
#define CLWK_CONTROL_CURSOR_L_WIDTH (16) // 幅
#define CLWK_CONTROL_CURSOR_L_HEIGHT (16) // 高さ
#define CLWK_CONTROL_CURSOR_L_ANIME_SEQ (NANR_obj_l)  // アニメーションシーケンス
#define CLWK_CONTROL_CURSOR_L_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_CONTROL_CURSOR_L_BG_PRIORITY   (0)  // BG 優先順位
// 右カーソル
#define CLWK_CONTROL_CURSOR_R_POS_X (0)  // x 座標
#define CLWK_CONTROL_CURSOR_R_POS_Y (0)  // y 座標
#define CLWK_CONTROL_CURSOR_R_WIDTH (16) // 幅
#define CLWK_CONTROL_CURSOR_R_HEIGHT (16) // 高さ
#define CLWK_CONTROL_CURSOR_R_ANIME_SEQ (NANR_obj_r)  // アニメーションシーケンス
#define CLWK_CONTROL_CURSOR_R_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_CONTROL_CURSOR_R_BG_PRIORITY   (0)  // BG 優先順位

// 自分の回答アイコン
#define CLWK_MY_ANSWER_ICON_POS_X (0)  // x 座標
#define CLWK_MY_ANSWER_ICON_POS_Y (0)  // y 座標
#define CLWK_MY_ANSWER_ICON_ANIME_SEQ (NANR_obj_me)  // アニメーションシーケンス
#define CLWK_MY_ANSWER_ICON_SOFT_PRIORITY (0)  // ソフト優先順位 0>0xff
#define CLWK_MY_ANSWER_ICON_BG_PRIORITY   (0)  // BG 優先順位

//-------------------
// □BMP-OAM アクター
//-------------------
//「…かいせきちゅう…」
#define BMPOAM_ACTOR_ANALYZING_CHARA_SIZE_X   (32)  // x サイズ ( キャラ単位 )
#define BMPOAM_ACTOR_ANALYZING_CHARA_SIZE_Y   (5)   // y サイズ ( キャラ単位 )
#define BMPOAM_ACTOR_ANALYZING_POS_X          (0)   // 左上x座標
#define BMPOAM_ACTOR_ANALYZING_POS_Y          (76)  // 左上y座標
#define BMPOAM_ACTOR_ANALYZING_PALETTE_OFFSET (1)   // パレットオフセット
#define BMPOAM_ACTOR_ANALYZING_SOFT_PRIORITY  (0)   // ソフトプライオリティ
#define BMPOAM_ACTOR_ANALYZING_BG_PRIORITY    (0)   // BGプライオリティ
#define BMPOAM_ACTOR_ANALYZING_STRING_OFFSET_X (8)  // 文字の書き込み先 x 座標オフセット
#define BMPOAM_ACTOR_ANALYZING_STRING_OFFSET_Y (9)  // 文字の書き込み先 y 座標オフセット
#define BMPOAM_ACTOR_ANALYZING_STRING_COLOR_L (1)   // 文字のカラー番号
#define BMPOAM_ACTOR_ANALYZING_STRING_COLOR_S (2)   // 影のカラー番号
#define BMPOAM_ACTOR_ANALYZING_STRING_COLOR_B (0)   // 背景のカラー番号


//===============================================================================
// ■動作
//===============================================================================
#define SEQ_QUEUE_SIZE (5)  // シーケンスキューのサイズ

// シーケンスの動作フレーム数
#define SEQ_ANALYZE_FRAMES (120) // RESEARCH_CHECK_SEQ_ANALYZE
#define SEQ_FLASH_FRAMES   (30)  // RESEARCH_CHECK_SEQ_FLASH
#define SEQ_UPDATE_FRAMES  (60)  // RESEARCH_CHECK_SEQ_UPDATE

// パレットフェード ( アウト )
#define MAIN_BG_PALETTE_FADE_OUT_TARGET_BITMASK (0xffff) // フェード対象パレット番号のマスク
#define MAIN_BG_PALETTE_FADE_OUT_WAIT           (2)      // フェード計算待ち時間
#define MAIN_BG_PALETTE_FADE_OUT_START_STRENGTH (0)      // フェード初期濃度 [0, 16]
#define MAIN_BG_PALETTE_FADE_OUT_END_STRENGTH   (10)     // フェード最終濃度 [0, 16]
#define MAIN_BG_PALETTE_FADE_OUT_COLOR          (0x0000) // 変更後の色

#define MAIN_OBJ_PALETTE_FADE_OUT_TARGET_BITMASK (0x0001) // フェード対象パレット番号のマスク
#define MAIN_OBJ_PALETTE_FADE_OUT_WAIT           (2)      // フェード計算待ち時間
#define MAIN_OBJ_PALETTE_FADE_OUT_START_STRENGTH (0)      // フェード初期濃度 [0, 16]
#define MAIN_OBJ_PALETTE_FADE_OUT_END_STRENGTH   (10)     // フェード最終濃度 [0, 16]
#define MAIN_OBJ_PALETTE_FADE_OUT_COLOR          (0x0000) // 変更後の色

// パレットフェード ( イン )
#define MAIN_BG_PALETTE_FADE_IN_TARGET_BITMASK (0xffff)  // フェード対象パレット番号のマスク
#define MAIN_BG_PALETTE_FADE_IN_WAIT           (2)       // フェード計算待ち時間
#define MAIN_BG_PALETTE_FADE_IN_START_STRENGTH (10)      // フェード初期濃度 [0, 16]
#define MAIN_BG_PALETTE_FADE_IN_END_STRENGTH   (0)       // フェード最終濃度 [0, 16]
#define MAIN_BG_PALETTE_FADE_IN_COLOR          (0x0000)  // 変更後の色

#define MAIN_OBJ_PALETTE_FADE_IN_TARGET_BITMASK (0x0001)  // フェード対象パレット番号のマスク
#define MAIN_OBJ_PALETTE_FADE_IN_WAIT           (2)       // フェード計算待ち時間
#define MAIN_OBJ_PALETTE_FADE_IN_START_STRENGTH (10)      // フェード初期濃度 [0, 16]
#define MAIN_OBJ_PALETTE_FADE_IN_END_STRENGTH   (0)       // フェード最終濃度 [0, 16]
#define MAIN_OBJ_PALETTE_FADE_IN_COLOR          (0x0000)  // 変更後の色

//===============================================================================
// ■タッチ範囲
//===============================================================================
// 質問
#define TOUCH_AREA_QUESTION_X      (QUESTION_X * DOT_PER_CHARA)      // X座標   (ドット単位)
#define TOUCH_AREA_QUESTION_Y      (QUESTION_Y * DOT_PER_CHARA)      // Y座標   (ドット単位)
#define TOUCH_AREA_QUESTION_WIDTH  (QUESTION_WIDTH * DOT_PER_CHARA)  // Xサイズ (ドット単位)
#define TOUCH_AREA_QUESTION_HEIGHT (QUESTION_HEIGHT * DOT_PER_CHARA) // Yサイズ (ドット単位)

// カーソル
#define TOUCH_AREA_CONTROL_CURSOR_WIDTH  (16) // Xサイズ (ドット単位)
#define TOUCH_AREA_CONTROL_CURSOR_HEIGHT (16) // Yサイズ (ドット単位)

//===============================================================================
// ■矢印
//===============================================================================
#define ARROW_START_X (120) // 矢印の始点x座標
#define ARROW_START_Y (40)  // 矢印の始点y座標
