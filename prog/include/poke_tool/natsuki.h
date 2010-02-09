//============================================================================================
/**
 * @file    natsuki.h
 * @bfief   なつき度計算
 * @author  HisashiSogabe
 * @date    10.02.05
 */
//============================================================================================

#pragma once
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "gamesystem/playerwork.h"

//なつき計算
typedef enum{
  CALC_NATSUKI_LEVELUP = 0,       //レベルアップ
  CALC_NATSUKI_USE_ITEM,          //アイテム使用
  CALC_NATSUKI_BOSS_BATTLE,       //ボス戦闘
  CALC_NATSUKI_TSUREARUKI,        //連れ歩き
  CALC_NATSUKI_HINSHI,            //瀕死
  CALC_NATSUKI_LEVEL30_HINSHI,    //レベル差30以上の瀕死
  CALC_NATSUKI_MUSICAL,           //ミュージカル
}CALC_NATSUKI;


//なつき度計算
void  NATSUKI_Calc( POKEMON_PARAM* pp, CALC_NATSUKI calcID, ZONEID zoneID, HEAPID heapID );
void  NATSUKI_CalcUseItem( POKEMON_PARAM* pp, u16 item_no, ZONEID zoneID, HEAPID heapID );
void  NATSUKI_CalcTsurearuki( POKEMON_PARAM* pp, ZONEID zoneID, HEAPID heapID );
void  NATSUKI_CalcBossBattle( POKEPARTY* ppt, ZONEID zoneID, HEAPID heapID );

