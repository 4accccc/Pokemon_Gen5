//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		gym_dragon_sv.h
//   @brief   ドラゴンジムセーブでデータ
//   @author	Masayuki Onoue
//   @note    evで読み込んでいます。
//   @date		2011.06.10
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define GYM_DRAGON_EVENT_SEQ_UP_BUTTON      (0)
#define GYM_DRAGON_EVENT_SEQ_DOWN_BUTTON    (1)
#define GYM_DRAGON_EVENT_SEQ_LEFT_BUTTON    (2)
#define GYM_DRAGON_EVENT_SEQ_RIGHT_BUTTON   (3)

//-------------------------------------
///	ボタン操作用スクリプトワーク
//=====================================
#define SC_BUTTON_WK_U  (LOCALWORK0)
#define SC_BUTTON_WK_D  (LOCALWORK1)
#define SC_BUTTON_WK_L  (LOCALWORK2)
#define SC_BUTTON_WK_R  (LOCALWORK3)

//=============================================================================
/**
 *    ソース用、構造体宣言
 */
//=============================================================================
#ifndef __ASM_NO_DEF_

//-------------------------------------
///	ロケーション
//=====================================
typedef enum{
  GYM_LOCATION_LEFT,
  GYM_LOCATION_RIGHT,
  GYM_LOCATION_CENTER,
  GYM_LOCATION_MAX,
}GYM_LOCATION;

//-------------------------------------
///	モデル管理ID
//=====================================
typedef enum
{
  GYM_DRAGON_MODELID_HERO_DRAGON,
  GYM_DRAGON_MODELID_ENEMY_DRAGON_TAIL,
  GYM_DRAGON_MODELID_ENEMY_DRAGON_FOOT_D,
  GYM_DRAGON_MODELID_ENEMY_DRAGON_FOOT_U,
  GYM_DRAGON_MODELID_ENEMY_DRAGON_HEAD,
  GYM_DRAGON_MODELID_MAX,
}MODELID;

//ドラゴンジムセーブワーク
typedef struct GYM_DRAGON_SV_WORK_tag
{
  u8 now_location;
  u8 now_floor_layer;
  fx32 model_frame[GYM_DRAGON_MODELID_MAX];
}GYM_DRAGON_SV_WORK;

#endif //__ASM_NO_DEF_

