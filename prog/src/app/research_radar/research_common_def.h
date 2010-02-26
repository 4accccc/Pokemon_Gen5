/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面 インデックス定義
 * @file   research_select_def.h
 * @author obata
 * @date   2010.02.15
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once


//------------------------
// □セルアクターユニット
//------------------------
// MAIN-OBJ
#define COMMON_CLUNIT_MAIN_OBJ_WORK_SIZE (10) // 最大ワーク数
#define COMMON_CLUNIT_MAIN_OBJ_PRIORITY  (1)  // 描画優先順位

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
#define CLWK_RETURN_SOFT_PRIORITY (0) // ソフト優先順位
#define CLWK_RETURN_BG_PRIORITY   (0) // BG 優先順位
