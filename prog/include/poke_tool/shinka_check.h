//============================================================================================
/**
 * @file    shinka_check.h
 * @bfief   進化チェック関連関数群
 * @author  HisashiSogabe
 * @date    09.12.02
 */
//============================================================================================
#pragma once

#include "poke_tool/pokeparty.h"

///<ポケモン進化データの構造体
typedef struct pokemon_shinka_data POKEMON_SHINKA_DATA;
typedef struct pokemon_shinka_table POKEMON_SHINKA_TABLE;

typedef enum{
  SHINKA_PARAM_COND=0,   //進化条件
  SHINKA_PARAM_DATA,     //進化条件に付随するデータ（アイテム進化ならアイテムナンバーなど）
  SHINKA_PARAM_MONS      //進化ポケモン
}SHINKA_PARAM;

typedef enum{
  SHINKA_TYPE_LEVELUP = 0,  //レベルアップによる進化
  SHINKA_TYPE_TUUSHIN,      //通信による進化
  SHINKA_TYPE_ITEM,         //アイテムによる進化
  SHINKA_TYPE_ITEM_CHECK,   //アイテムによる進化（かわらずのいしを装備していてもチェックはする）

  SHINKA_TYPE_MAX,

  SHINKA_COND_START = 0,
  SHINKA_COND_MAX = 7,
}SHINKA_TYPE;

//=========================================================================
//		進化条件定義
//=========================================================================
typedef enum
{ 
  SHINKA_COND_NONE = 0,
  SHINKA_COND_FRIEND_HIGH,
  SHINKA_COND_FRIEND_HIGH_NOON,
  SHINKA_COND_FRIEND_HIGH_NIGHT,
  SHINKA_COND_LEVELUP,
  SHINKA_COND_TUUSHIN,
  SHINKA_COND_TUUSHIN_ITEM,
  SHINKA_COND_TUUSHIN_YUUGOU,
  SHINKA_COND_ITEM,
  SHINKA_COND_SPECIAL_POW,
  SHINKA_COND_SPECIAL_EVEN,
  SHINKA_COND_SPECIAL_DEF,
  SHINKA_COND_SPECIAL_RND_EVEN,
  SHINKA_COND_SPECIAL_RND_ODD,
  SHINKA_COND_SPECIAL_LEVELUP,
  SHINKA_COND_SPECIAL_NUKENIN,
  SHINKA_COND_SPECIAL_BEAUTIFUL,
  SHINKA_COND_ITEM_MALE,
  SHINKA_COND_ITEM_FEMALE,
  SHINKA_COND_SOUBI_NOON,
  SHINKA_COND_SOUBI_NIGHT,
  SHINKA_COND_WAZA,
  SHINKA_COND_POKEMON,
  SHINKA_COND_MALE,
  SHINKA_COND_FEMALE,
  SHINKA_COND_PLACE_TENGANZAN,
  SHINKA_COND_PLACE_KOKE,
  SHINKA_COND_PLACE_ICE,
  SHINKA_COND_PLACE_DENKIDOUKUTSU,
}SHINKA_COND;

#define	SHINKA_MAX	    ( 7 )	  //進化分岐のMAX
#define SHINKA_FRIEND   ( 220 ) //進化に必要ななつき度

extern  u16         SHINKA_Check( POKEPARTY *ppt, POKEMON_PARAM *pp, SHINKA_TYPE type, u32 param,
                                  SHINKA_COND *cond, HEAPID heapID );
extern  ARCHANDLE*  SHINKA_GetArcHandle( HEAPID heapID );
extern  u16         SHINKA_GetParamByHandle( ARCHANDLE* handle, int mons_no, int index, SHINKA_PARAM param );
