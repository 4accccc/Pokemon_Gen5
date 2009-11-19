/*
 *  @file field_encount_local.h
 *  @brief  field_encount.c ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   09.10.19
 */

#pragma once

#include "effect_encount.h"

//======================================================================
//  struct
//======================================================================

/////////////////////////////////////////////////////
///通常エンカウント制御　プレイヤー情報
typedef struct _EWK_PLAYER{
  u16 pos_x;    //最後にエンカウントしたプレイヤーグリッドポジション
  u16 pos_y;
  u16 pos_z;
  u16 move_f;   //
  u32 walk_ct;
}EWK_PLAYER;

/////////////////////////////////////////////////////
///フィールドエンカウント制御ワーク fieldWork常駐
struct _TAG_FIELD_ENCOUNT
{
  FIELDMAP_WORK *fwork;
  GAMESYS_WORK *gsys;
  GAMEDATA *gdata;
  ENCOUNT_DATA* encdata;
  EFFECT_ENCOUNT* eff_enc;
};


///エンカウントエフェクト制御ワーク
typedef struct _EWK_EFFECT_ENCOUNT
{
  u32 walk_ct;
}EWK_EFFECT_ENCOUNT;

///エンカウント制御ワーク
struct _TAG_ENCOUNT_WORK
{
  EWK_PLAYER player;
  EWK_EFFECT_ENCOUNT  effect_encount;
};



