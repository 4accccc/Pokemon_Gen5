//==============================================================================
/**
 * @file    union_subproc.h
 * @brief   
 * @author  matsuda
 * @date    2009.07.16(木)
 */
//==============================================================================
#pragma once

#include "app/pokelist.h"
#include "app/p_status.h"
#include "field/event_colosseum_battle.h"


//==============================================================================
//  構造体定義
//==============================================================================
///ポケモンリストサブProc呼び出しようのParentWork
typedef struct{
  PLIST_DATA plist;
  PSTATUS_DATA pstatus;
  COLOSSEUM_BATTLE_SETUP *battle_setup;
}UNION_SUBPROC_PARENT_POKELIST;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * UnionSubProc_Create(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork, UNION_SYSTEM_PTR unisys);
extern void UnionSubProc_EventSet(UNION_SYSTEM_PTR unisys, UNION_SUBPROC_ID sub_proc_id, void *parent_wk);
extern BOOL UnionSubProc_IsExits(UNION_SYSTEM_PTR unisys);

