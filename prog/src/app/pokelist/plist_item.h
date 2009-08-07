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

const BOOL PLIST_ITEM_IsDeathRecoverAllItem( PLIST_WORK *work , u16 itemNo );
const BOOL PLIST_ITEM_IsNeedSelectSkill( PLIST_WORK *work , u16 itemNo );

void PLIST_ITEM_MSG_CanNotUseItem( PLIST_WORK *work );
void PLIST_ITEM_MSG_UseItemFunc( PLIST_WORK *work );
