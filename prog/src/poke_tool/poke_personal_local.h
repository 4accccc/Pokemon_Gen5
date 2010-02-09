//=============================================================================================
/**
 * @file  poke_personal.h
 * @brief ポケモンパーソナルデータアクセス関数群（poke_toolグループ内）
 * @author  taya
 *
 * @date  2008.11.06  作成
 */
//=============================================================================================
#ifndef __POKE_PERSONAL_LOCAL_H__
#define __POKE_PERSONAL_LOCAL_H__

#include    "poke_tool/poke_personal.h"
#include    "poke_tool/monsno_def.h"


//--------------------------------------------------------------
/**
 * ポケモンパーソナルデータの構造体定義
 */
//--------------------------------------------------------------
struct  pokemon_personal_data
{
  u8    basic_hp;       //基本ＨＰ
  u8    basic_pow;      //基本攻撃力
  u8    basic_def;      //基本防御力
  u8    basic_agi;      //基本素早さ

  u8    basic_spepow;   //基本特殊攻撃力
  u8    basic_spedef;   //基本特殊防御力
  u8    type1;          //属性１
  u8    type2;          //属性２

  u8    get_rate;       //捕獲率
  u8    rank;           //ポケモンランク
  u16   pains_hp    :2; //贈与努力値ＨＰ
  u16   pains_pow   :2; //贈与努力値攻撃力
  u16   pains_def   :2; //贈与努力値防御力
  u16   pains_agi   :2; //贈与努力値素早さ
  u16   pains_spepow:2; //贈与努力値特殊攻撃力
  u16   pains_spedef:2; //贈与努力値特殊防御力
  u16               :4; //贈与努力値予備

  u16   item1;          //アイテム１
  u16   item2;          //アイテム２

  u16   item3;          //アイテム３
  u8    sex;            //性別ベクトル
  u8    egg_birth;      //タマゴの孵化歩数

  u8    friend;         //なつき度初期値
  u8    grow;           //成長曲線識別
  u8    egg_group1;     //こづくりグループ1
  u8    egg_group2;     //こづくりグループ2

  u8    speabi1;        //特殊能力１
  u8    speabi2;        //特殊能力２
  u8    speabi3;        //特殊能力３
  u8    escape;         //逃げる率

  u16   form_index;     //別フォルムパーソナルデータ開始位置
  u16   form_gra_index; //別フォルムグラフィックデータ開始位置

  u8    form_max;       //別フォルムMAX
  u8    color     :6;   //色（図鑑で使用）
  u8    reverse   :1;   //反転フラグ
  u8    pltt_only :1;   //別フォルム時パレットのみ変化
  u16   give_exp;       //贈与経験値

  u16   height;         //高さ
  u16   weight;         //重さ

  u32   machine1;       //技マシンフラグ１
  u32   machine2;       //技マシンフラグ２
  u32   machine3;       //技マシンフラグ２
  u32   machine4;       //技マシンフラグ２

  u32   waza_oshie1;    //技教え１
};


//=============================================================================================
/**
 * システム初期化（プログラム起動後に１度だけ呼び出す）
 *
 * @param   heapID    システム初期化用ヒープID（アーカイブハンドル用に数十バイト必要）
 */
//=============================================================================================
extern void POKE_PERSONAL_InitSystem( HEAPID heapID );


extern void POKE_PERSONAL_LoadData( u16 mons_no, u16 form_no, POKEMON_PERSONAL_DATA* ppd );

extern u16 POKE_PERSONAL_GetPersonalID( u16 mons_no, u16 form_no );



#endif
