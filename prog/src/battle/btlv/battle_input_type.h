//==============================================================================
/**
 * @file  battle_input_type.h
 * @brief 戦闘入力画面の外側で使用される構造体の定義など
 * @author  matsuda
 * @date  2006.05.11(木)
 */
//==============================================================================
#ifndef __BATTLE_INPUT_TYPE_H__
#define __BATTLE_INPUT_TYPE_H__

#include "../btl_common.h"

//==============================================================================
//  構造体定義
//==============================================================================
//--------------------------------------------------------------
//  SCENEワーク
//--------------------------------------------------------------
///コマンド選択画面構成用のSceneワーク
typedef struct{
  u8 client_type;
  u8 client_no;
  u8 sel_mons_no;
  u8    icon_status;

  s16   icon_hp;        //現在のHP
  u16   icon_hpmax;       //現在のHPMAX

  u8 cancel_escape;       ///<「もどる」or「にげる」
  u8 padding[3];
}BINPUT_SCENE_COMMAND;

///技選択画面構成用のSceneワーク
typedef struct{
  u16     wazano[ PTL_WAZA_MAX ];   //wazano
  u8      pp[ PTL_WAZA_MAX ];     //pp
  u8      ppmax[ PTL_WAZA_MAX ];      //ppmax
  u8      client_type;          //
}BINPUT_SCENE_WAZA;

//DIR_SELECT用（プラチナから持ってきた）
typedef struct{
  u8                  sex   :2;       //性別
  u8                  exist :1;       //選択可否
  u8                        :5;       //
  u8                  status;
  u16                 dummy;          //4バイト境界ダミー

  s16                 hp;             //現在のHP
  u16                 hpmax;          //現在のHPMAX
  const POKEMON_PARAM *pp;
}DIR_SELECT_POKE_PARAM;

///ポケモン選択画面構成用のSceneワーク
typedef struct{
  DIR_SELECT_POKE_PARAM dspp[ BTL_CLIENT_MAX ];
  u8      client_type;
  u8      pokesele_type;  ///<対象選択タイプ
}BINPUT_SCENE_POKE;

///「はい・いいえ」選択画面構成用のSceneワーク
typedef struct{
  u16 waza_no;

  u16 dummy;    ///<4バイト境界ダミー
}BINPUT_SCENE_YESNO;



#endif  //__BATTLE_INPUT_TYPE_H__

