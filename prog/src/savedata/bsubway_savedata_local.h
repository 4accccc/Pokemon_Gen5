//======================================================================
/**
 * @file  bsubway_savedata_local.h
 * @brief  バトルサブウェイ　セーブデータ構造体定義
 * @authaor  iwasawa
 * @date  2008.12.11
 * @note プラチナより移植 kagaya
 */
//======================================================================
#ifndef __H_BSUBWAY_SAVEDATA_LOCAL_H__
#define __H_BSUBWAY_SAVEDATA_LOCAL_H__

//#include "multiboot/src/dl_child/pt_save.h"

#include "system/pms_data.h"
#include "system/gf_date.h"

#include "battle/bsubway_battle_data.h"
#include "savedata/bsubway_savedata_def.h"
#include "savedata/bsubway_savedata.h"


//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// バトルサブウェイ プレイヤーメッセージデータ
/// 8*4=32byte
//--------------------------------------------------------------
struct _BSUBWAY_PLAYER_MSG
{
  PMS_DATA  msg[4];  ///<自分用メッセージ
};

//--------------------------------------------------------------
/// バトルサブウェイ WIFI プレイヤーデータ(Dpw_Bt_Player)
/// 56*3+60=228byte
//--------------------------------------------------------------
struct _BSUBWAY_WIFI_PLAYER
{
  struct _BSUBWAY_POKEMON  poke[3];
  
  //トレーナー名((PERSON_NAME_SIZE:7)+(EOM_SIZE:1))*(STRCODE:u16)=16
  STRCODE  name[PERSON_NAME_SIZE+EOM_SIZE];

  u8  casette_version;          //カセットversion
  u8  lang_version;            //言語バージョン
  u8  country_code;            //国コード
  u8  address;              //住んでいるところ

  u32  id_no;                //IDNo  

  u16  leader_word[4];            //リーダーメッセージ

  union{
    struct{
    u8  ngname_f:1;            //NGネームフラグ
    u8  gender:1;            //性別
    u8      :6;            //あまり6bit
    };
    u8  flags;
  };
  u8  tr_type;              //トレーナータイプ
  u16  appear_word[4];            //登場メッセージ  
  u16  win_word[4];            //勝利メッセージ  
  u16  lose_word[4];            //敗退メッセージ  
  u16  result;                //成績
};

//--------------------------------------------------------------
/// バトルサブウェイ WIFI DLデータ
/// 280byte+1596(228*7)byte+1020(34*30)byte=2836byte
//--------------------------------------------------------------
struct _BSUBWAY_WIFI_DATA
{
  GFDATE  day;  ///<DLした日付4byte
  u8 flags[BSUBWAY_ROOM_DATA_FLAGS_LEN];  ///<DL済みフラグエリア(2000bit)
  u8 player_data_f;  ///<プレイヤーデータが有効かどうか？
  u8 leader_data_f;  ///<リーダーデータが有効かどうか？

  //DLデータのルームID
  u8  player_room;  ///<プレイヤーデータルームNo
  u8  player_rank;  ///<プレイヤーデータルームランク
  u8  leader_room;  ///<リーダーデータルームNo
  u8  leader_rank;  ///<リーダーデータルームランク

  ///プレイヤーデータ
  struct _BSUBWAY_WIFI_PLAYER player[BSUBWAY_STOCK_WIFI_PLAYER_MAX];
  ///リーダーデータ
  struct _BSUBWAY_LEADER_DATA leader[BSUBWAY_STOCK_WIFI_LEADER_MAX];
};

//======================================================================
//  extern
//======================================================================

#endif  //__H_BSUBWAY_SAVEDATA_LOCAL_H__
