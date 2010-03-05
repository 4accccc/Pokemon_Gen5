////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 全画面共通のインデックス定義
 * @file   research_common_index.h
 * @author obata
 * @date   2010.02.26
 */
////////////////////////////////////////////////////////////////
#pragma once

//--------------
// □タッチ範囲
//--------------
typedef enum {
  COMMON_TOUCH_AREA_RETURN_BUTTON, //「もどる」ボタン
  COMMON_TOUCH_AREA_DUMMY,         // 終端コード
  COMMON_TOUCH_AREA_NUM,           // 総数
} COMMON_TOUCH_AREA_INDEX;

//--------------------------
// □パレットアニメーション
//--------------------------
typedef enum {
  COMMON_PALETTE_ANIME_RETURN, //「もどる」アイコン選択リアクション
  COMMON_PALETTE_ANIME_NUM,    // 総数
} COMMON_PALETTE_ANIME_INDEX;

//--------
// □OBJ
//--------
// OBJリソース
typedef enum {
  COMMON_OBJ_RESOURCE_MAIN_CHARACTER,      // MAIN-OBJ キャラクタ
  COMMON_OBJ_RESOURCE_MAIN_PALETTE_ICON,   // MAIN-OBJ パレット ( アイコン )
  COMMON_OBJ_RESOURCE_MAIN_CELL_ANIME,     // MAIN-OBJ セル アニメ
  COMMON_OBJ_RESOURCE_NUM,                 // 総数
} COMMON_OBJ_RESOURCE_ID;

// セルアクターユニット
typedef enum {
  COMMON_CLUNIT_MAIN_OBJ, // MAIN-OBJ
  COMMON_CLUNIT_NUM,      // 総数
} COMMON_CLUNIT_INDEX;

// セルアクターワーク
typedef enum {
  COMMON_CLWK_RETURN, //「もどる」ボタン
  COMMON_CLWK_NUM,    // 総数
} COMMON_CLWK_INDEX; 
