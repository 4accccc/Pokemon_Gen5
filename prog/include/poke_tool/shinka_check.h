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
  SHINKA_COND_LEVELUP = 0,  //レベルアップによる進化
  SHINKA_COND_TUUSHIN,      //通信による進化
  SHINKA_COND_ITEM,         //アイテムによる進化
  SHINKA_COND_ITEM_CHECK,   //アイテムによる進化（かわらずのいしを装備していてもチェックはする）
}SHINKA_COND;

extern  u16	        SHINKA_Check( POKEPARTY *ppt, POKEMON_PARAM *pp, SHINKA_COND cond, u16 param,
                                  SHINKA_COND *shinka_cond, HEAPID heapID );
extern  ARCHANDLE*  SHINKA_GetArcHandle( HEAPID heapID );
extern  u16         SHINKA_GetParamByHandle( ARCHANDLE* handle, int mons_no, int index, SHINKA_PARAM param );
