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

//アイテムを使えるかのチェック
typedef enum
{
  PIUC_OK,  //使える
  PIUC_NG,  //使えない
  PIUC_SELECT_SKILL,  //スキル選択へ
}PLIST_ITEM_USE_CHECK;

const BOOL PLIST_ITEM_IsDeathRecoverAllItem( PLIST_WORK *work , u16 itemNo );
const PLIST_ITEM_USE_CHECK PLIST_ITEM_CanUseRecoverItem( PLIST_WORK *work , u16 itemNo , POKEMON_PARAM *pp );

void PLIST_ITEM_UseRecoverItem( PLIST_WORK *work );
void PLIST_ITEM_UseAllDeathRecoverItem( PLIST_WORK *work );

void PLIST_ITEM_MSG_CanNotUseItem( PLIST_WORK *work );
