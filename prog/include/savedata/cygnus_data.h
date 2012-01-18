//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		cygnus_data.h
 *	@brief  3DS連動：cygnus_savedataの実装部分
 *	@author	Toru=Nagihashi
 *	@date		2012.01.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/cygnus_savedata.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//----------------------------------------------------------
/**
 * @brief CYGDATA_POKE01～CYGDATA_POKE06の４バイトは以下になっているため、
 * 取り出すためのマクロを定義する
 *
 * struct{
 *  u32 mons_no:16;
 *  u32 form_no:8:
 *  u32 sex:8;
 * }
 */
//----------------------------------------------------------
#define CYGNUS_DATA_GET_POKE_NO(v)  ((v>>16)&0xFFFF)
#define CYGNUS_DATA_GET_FORM_NO(v)  ((v>>8)&0xFF)
#define CYGNUS_DATA_GET_SEX(v)      (v&0xFF)


//----------------------------------------------------------
/**
 * @brief CYGDATA_ITEMID01～CYGDATA_ITEMID06の４バイトは以下になっているため、
 * 取り出すためのマクロを定義する
 *
 * struct{
 *  u32 item_no:16;
 *  u32 item_num:16:
 * }
 */
//----------------------------------------------------------
#define CYGNUS_DATA_GET_ITEM_NO(v)  ((v>>16)&0xFFFF)
#define CYGNUS_DATA_GET_ITEM_NUM(v)  (v&0xFFFF)

//----------------------------------------------------------
/**
 * @brief 特別な受け取りに使用するマジックナンバー
 *
 * トルネロスなど伝説系のポケモンは一度しか受け取れない。
 * 安易に書き換えられないようにマジックナンバーで指定する。
 */
//----------------------------------------------------------
typedef enum {
  MAGICNUMBER_SP_POKE01 = 0x80808080,
  MAGICNUMBER_SP_POKE02 = 0x92567284,
  MAGICNUMBER_SP_POKE03 = 0x87643567,
  MAGICNUMBER_SP_POKE04 = 0x96436763,
  MAGICNUMBER_SP_POKE05 = 0x43867368,
  MAGICNUMBER_SP_POKE06 = 0x17693572,
  MAGICNUMBER_SP_POKE07 = 0x44798367,
  MAGICNUMBER_SP_POKE08 = 0x96636983,
}CYGNUSDATA_MAGICNUMBER;

//----------------------------------------------------------
/**
 * @brief CYGNUS送信データ領域のインデックス
 */
//----------------------------------------------------------
enum {
  CYGDATA_SP_POKE01 = 0,  ///<特別なポケモンCYGNUSDATA_MAGICNUMBERが入ります。
  CYGDATA_SP_POKE02,
  CYGDATA_SP_POKE03,
  CYGDATA_SP_POKE04,
  CYGDATA_SP_POKE05,
  CYGDATA_SP_POKE06,
  CYGDATA_SP_POKE07,
  CYGDATA_SP_POKE08,

  CYGDATA_POKE01,         ///<汎用ポケモン指定領域 CYGNUS_DATA_GET_POKE_～を使います
  CYGDATA_POKE02,
  CYGDATA_POKE03,
  CYGDATA_POKE04,
  CYGDATA_POKE05,
  CYGDATA_POKE06,

  CYGDATA_ITEMID01,        ///<汎用アイテムID指定領域 CYGNUS_DATA_GET_ITEM_～を使います
  CYGDATA_ITEMID02,
  CYGDATA_ITEMID03,
  CYGDATA_ITEMID04,
  CYGDATA_ITEMID05,
  CYGDATA_ITEMID06,

  CYGDATA_MAX
};

//----------------------------------------------------------
/**
 * @brief CYGNUSのビット
 */
//----------------------------------------------------------
typedef enum {
  CYGFLAG_SP_POKE01 = 1<<0,
  CYGFLAG_SP_POKE02 = 1<<1,
  CYGFLAG_SP_POKE03 = 1<<2,
  CYGFLAG_SP_POKE04 = 1<<3,
  CYGFLAG_SP_POKE05 = 1<<4,
  CYGFLAG_SP_POKE06 = 1<<5,
  CYGFLAG_SP_POKE07 = 1<<6,
  CYGFLAG_SP_POKE08 = 1<<7,
}CYGNUSDATA_FLAG;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//有効なマジックナンバーかチェック
extern BOOL CUGNUS_DATA_IsValidMagicNumber( CYGNUSDATA_MAGICNUMBER magic_num );
//マジックナンバーからポケモン情報の取得
extern void CYGNUS_DATA_GetMonsNoByMagicNum( CYGNUSDATA_MAGICNUMBER magic_num, u16 *p_mons_no, u8 *p_form_no, u8 *p_sex );
//マジックナンバーからフラグの取得
extern CYGNUSDATA_FLAG CYGNUS_DATA_GetFlagByMagicNum( CYGNUSDATA_MAGICNUMBER magic_num );
//フラグからマジックナンバーの取得
extern CYGNUSDATA_MAGICNUMBER CYGNUS_DATA_GetMagicNumByFlag( CYGNUSDATA_FLAG flag );
//フラグからポケモン情報の取得
extern void CYGNUS_DATA_GetMonsNoByFlag( CYGNUSDATA_FLAG flag, u16 *p_mons_no, u8 *p_form_no, u8 *p_sex );
