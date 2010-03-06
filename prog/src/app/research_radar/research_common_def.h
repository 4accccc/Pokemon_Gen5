/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面 定数定義
 * @file   research_common_def.h
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once 


#define PRINT_TARGET (2)  // デバッグ情報の出力先

#define DOT_PER_CHARA     (8)                      // 1キャラ = 8ドット
#define ONE_CHARA_SIZE    (0x20)                   // 1キャラクターデータのサイズ [Byte]
#define ONE_COLOR_SIZE    (2)                      // 1カラーデータのサイズ [Byte]
#define ONE_PALETTE_SIZE  (0x20)                   // 1パレット(16色分)のサイズ [Byte]
#define FULL_PALETTE_SIZE (ONE_PALETTE_SIZE * 16)  // 16パレットのサイズ [Byte]


//----------
// □SUB-BG
//----------
// 背景面
#define SUB_BG_BACK (GFL_BG_FRAME0_S)  // BG フレーム
#define SUB_BG_BACK_PRIORITY      (3)  // 表示優先順位
#define SUB_BG_BACK_FIRST_PLT_IDX (0)  // 先頭パレット番号
#define SUB_BG_BACK_PLT_NUM       (1)  // 使用パレット数
// レーダー面
#define SUB_BG_RADAR (GFL_BG_FRAME1_S)    // BG フレーム
#define SUB_BG_RADAR_PRIORITY        (2)  // 表示優先順位
#define SUB_BG_RADAR_FIRST_PLT_IDX   (0)  // 先頭パレット番号
#define SUB_BG_RADAR_PLT_NUM         (6)  // 使用パレット数
#define SUB_BG_RADAR_PLT_ANIME_FRAME (10) // パレットアニメーションの更新間隔[frame]


//-----------
// □MAIN-BG
//-----------
// 背景面
#define MAIN_BG_BACK (GFL_BG_FRAME1_M)  // 背景BG面
#define MAIN_BG_BACK_PRIORITY      (3)  // 背景BG面の表示優先順位
#define MAIN_BG_BACK_FIRST_PLT_IDX (0)  // 背景BG面の先頭パレット番号
#define MAIN_BG_BACK_PLT_NUM       (1)  // 背景BG面のパレット数


//------------------------
// □セルアクターユニット
//------------------------
// MAIN-OBJ
#define COMMON_CLUNIT_MAIN_OBJ_WORK_SIZE (1)  // 最大ワーク数
#define COMMON_CLUNIT_MAIN_OBJ_PRIORITY  (2)  // 描画優先順位


//-----------------------
// □セルアクターワーク
//-----------------------

//「もどる」ボタン
#define RETURN_BUTTON_X      (27) // X座標 ( キャラクタ単位 )
#define RETURN_BUTTON_Y      (21) // Y座標 ( キャラクタ単位 )
#define RETURN_BUTTON_WIDTH  (3)  // 幅   ( キャラクタ単位 )
#define RETURN_BUTTON_HEIGHT (3)  // 高さ ( キャラクタ単位 )

#define TOUCH_AREA_RETURN_BUTTON_X (RETURN_BUTTON_X * DOT_PER_CHARA) // X座標 ( ドット単位 )
#define TOUCH_AREA_RETURN_BUTTON_Y (RETURN_BUTTON_Y * DOT_PER_CHARA) // Y座標 ( ドット単位 )
#define TOUCH_AREA_RETURN_BUTTON_WIDTH  (RETURN_BUTTON_WIDTH * DOT_PER_CHARA) // Xサイズ ( ドット単位 )
#define TOUCH_AREA_RETURN_BUTTON_HEIGHT (RETURN_BUTTON_HEIGHT * DOT_PER_CHARA) // Yサイズ ( ドット単位 )

#define CLWK_RETURN_POS_X (RETURN_BUTTON_X * DOT_PER_CHARA) // x 座標 ( ドット単位 )
#define CLWK_RETURN_POS_Y (RETURN_BUTTON_Y * DOT_PER_CHARA) // y 座標 ( ドット単位 )
#define CLWK_RETURN_ANIME_SEQ     (1) // アニメーションシーケンス
#define CLWK_RETURN_SOFT_PRIORITY (3) // ソフト優先順位
#define CLWK_RETURN_BG_PRIORITY   (1) // BG 優先順位
