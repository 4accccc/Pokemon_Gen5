//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		gym_dragon_local.h
//   @brief   ドラゴンジムローカルヘッダ
//   @author	Masayuki Onoue
//   @date		2011.06.09
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================
//-------------------------------------
///	ドラゴンジムレイヤー
//=====================================
typedef enum{
  FLOOR_LAYER_ENTRANCE,
  FLOOR_LAYER_TAIL,
  FLOOR_LAYER_FOOT,
  FLOOR_LAYER_ARM,
  FLOOR_LAYER_HEAD,
  FLOOR_LAYER_MAX,
}FLOOR_LAYER;

//-------------------------------------
///	ボタンの状態
//=====================================
typedef enum{
  GYM_BUTTON_NONE,
  GYM_BUTTON_UDLR,
  GYM_BUTTON_DLR,
  GYM_BUTTON_UD,
  GYM_BUTTON_LR,
  GYM_BUTTON_L,
  GYM_BUTTON_R,
  GYM_BUTTON_D,
  GYM_BUTTON_U,
  GYM_BUTTON_MAX,
}GYM_BUTTON_SEQ;

//-------------------------------------
///	ボタンイベントデータ
//=====================================
typedef struct {
  ARCDATID ica_dat_id;  // プレイヤー移動用ICAデータ
}BUTTON_EVENT_DATA;

//-------------------------------------
///	プレイヤー移動イベントシーケンス
//=====================================
typedef enum{
  MOVE_EVENT_SEQ_INIT,
  MOVE_EVENT_SEQ_MOVE,
  MOVE_EVENT_SEQ_EXIT,
}MOVE_EVENT_SEQ;

//-------------------------------------
///	ボタン有効化イベントシーケンス
//=====================================
typedef enum{
  BA_EVENT_SEQ_INIT,
  BA_EVENT_SEQ_SET_STATUS,
  BA_EVENT_SEQ_EXIT,
}BA_EVENT_SEQ;

