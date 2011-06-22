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
  u16   pl_trace_data_id;   // NULL追従用データID
  u16   en_model_id;        // 敵ドラゴン用モデルID
  u16   en_trace_data_id;   // NULL追従用データID
  u16   en_obj_id;          // 敵トレーナーのOBJID
  BOOL  is_enemy_move;      // 敵も移動するかどうか
}BUTTON_EVENT_DATA;

//-------------------------------------
///	プレイヤー移動イベントシーケンス
//=====================================
typedef enum{
  MOVE_EVENT_SEQ_START_SE,
  MOVE_EVENT_SEQ_START_SE_WAIT,
  MOVE_EVENT_SEQ_INIT,
  MOVE_EVENT_SEQ_MOVE,
  MOVE_EVENT_SEQ_END_SE_WAIT,
  MOVE_EVENT_SEQ_BUTTON_SET,
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

//-------------------------------------
///	ICA追従データID
//=====================================
typedef enum{
  ICA_MOVE_ID_PLAYER,
  ICA_MOVE_ID_ENEMY,
  ICA_MOVE_ID_MAX,
}ICA_MOVE_ID;

// 拡張高さ用アトリビュート
#define EX_HEIGHT_ATTRIBUTE (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL)



