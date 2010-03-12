//======================================================================
/**
 * @file	plist_item.c
 * @brief	ポケモンリスト アイテムチェック系
 * @author	ariizumi
 * @data	09/08/05
 *
 * モジュール名：PLIST_ITEM
 */
//======================================================================

#pragma once

#include "poke_tool\poke_tool.h"
#include "plist_local_def.h"
//GSから定義移植
typedef enum
{
  ITEM_TYPE_BTL_ST_UP = 0,  // 戦闘用ステータスアップ系
  ITEM_TYPE_ALLDETH_RCV,    // 全員瀕死回復
  ITEM_TYPE_LV_UP,      // LvUp系
  ITEM_TYPE_NEMURI_RCV,   // 眠り回復
  ITEM_TYPE_DOKU_RCV,     // 毒回復
  ITEM_TYPE_YAKEDO_RCV,   // 火傷回復
  ITEM_TYPE_KOORI_RCV,    // 氷回復
  ITEM_TYPE_MAHI_RCV,     // 麻痺回復
  ITEM_TYPE_KONRAN_RCV,   // 混乱回復
  ITEM_TYPE_ALL_ST_RCV,   // 全快
  ITEM_TYPE_MEROMERO_RCV,   // メロメロ回復
  ITEM_TYPE_HP_RCV,     // HP回復
  ITEM_TYPE_DEATH_RCV,  // 瀕死回復 (WB追加
  
  //メッセージのステータスと順番依存！！
  ITEM_TYPE_HP_UP,      // HP努力値UP
  ITEM_TYPE_ATC_UP,     // 攻撃努力値UP
  ITEM_TYPE_DEF_UP,     // 防御努力値UP
  ITEM_TYPE_AGL_UP,     // 素早さ努力値UP
  ITEM_TYPE_SPA_UP,     // 特攻努力値UP
  ITEM_TYPE_SPD_UP,     // 特防努力値UP

  //メッセージのステータスと順番依存！！
  ITEM_TYPE_HP_DOWN,      // HP努力値DOWN
  ITEM_TYPE_ATC_DOWN,     // 攻撃努力値DOWN
  ITEM_TYPE_DEF_DOWN,     // 防御努力値DOWN
  ITEM_TYPE_AGL_DOWN,     // 素早さ努力値DOWN
  ITEM_TYPE_SPA_DOWN,     // 特攻努力値DOWN
  ITEM_TYPE_SPD_DOWN,     // 特防努力値DOWN

  ITEM_TYPE_EVO,        // 進化系
  ITEM_TYPE_PP_UP,      // ppUp系
  ITEM_TYPE_PP_3UP,     // pp3Up系
  ITEM_TYPE_PP_RCV,     // pp回復系
  ITEM_TYPE_ETC,        // その他
}PLIST_ITEM_USE_TYPE;


extern void PLIST_UpdateDispParam( PLIST_WORK *work );

extern const BOOL PLIST_ITEM_IsDeathRecoverAllItem( PLIST_WORK *work , u16 itemNo );
extern const BOOL PLIST_ITEM_IsNeedSelectSkill( PLIST_WORK *work , u16 itemNo );
//-1で対象無しそれ以外は対象番号
extern const int PLIST_ITEM_CanUseDeathRecoverAllItem( PLIST_WORK *work );

extern u32 PLIST_ITEM_GetWazaListMessage( PLIST_WORK *work , u16 itemNo );
extern void PLIST_ITEM_CangeAruseusuForm( PLIST_WORK *work , POKEMON_PARAM *pp , const u16 itemNo );

extern void PLIST_ITEM_MSG_CanNotUseItem( PLIST_WORK *work );
extern const PLIST_ITEM_USE_TYPE PLIST_ITEM_MSG_UseItemFunc( PLIST_WORK *work );

extern void PLIST_MSGCB_LvUp_EvoCheck( PLIST_WORK *work );
extern void PLIST_HPANMCB_ReturnRecoverHp( PLIST_WORK *work );
extern void PLIST_HPANMCB_ReturnRecoverAllDeath( PLIST_WORK *work );
extern void PLIST_MSGCB_RecoverAllDeath_NextPoke( PLIST_WORK *work );
