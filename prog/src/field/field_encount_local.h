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

//エフェクトエンカウント　パラメータ
typedef struct _EFFENC_PARAM{
  u16   zone_id;
  u16   gx,gy,gz;
  fx32  height;
  u8    type;
  u8    valid_f:4;
  u8    push_f:2;
  u8    push_cancel_f:2;
}EFFENC_PARAM;

///エンカウントエフェクト制御ワーク
typedef struct _EWK_EFFECT_ENCOUNT
{
  u32 walk_ct;
  EFFENC_PARAM  param;

#ifdef PM_DEBUG
  u8  deb_interval;
  u8  deb_prob;
  u8  deb_ofsx;
  u8  deb_ofsz;
#endif

}EWK_EFFECT_ENCOUNT;

///エンカウント制御ワーク
struct _TAG_ENCOUNT_WORK
{
  EWK_PLAYER player;
  EWK_EFFECT_ENCOUNT  effect_encount;
};



