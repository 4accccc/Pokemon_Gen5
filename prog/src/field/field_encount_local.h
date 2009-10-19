/*
 *  @file field_encount_local.h
 *  @brief  field_encount.c ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   09.10.19
 */

#pragma once

///エンカウント制御　プレイヤー情報
typedef struct _EWK_PLAYER{
  u16 pos_x;    //最後にエンカウントしたプレイヤーグリッドポジション
  u16 pos_y;
  u16 pos_z;
  u16 move_f;   //
  u32 walk_ct;
}EWK_PLAYER;

///エンカウント制御ワーク
struct _TAG_ENCOUNT_WORK
{
  EWK_PLAYER player;
};



