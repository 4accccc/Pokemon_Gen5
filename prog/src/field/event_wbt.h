//======================================================================
/**
 * @file  event_wbt.h
 * @brief  バトルサブウェイ　イベント
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "event_fieldmap_control.h"
#include "app/pokelist.h"
#include "app/p_status.h"

//#include "bsubway_scr.h"

#include "wbt_system.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern GMEVENT * WBT_EVENT_SetSelectPokeList(
    WBTSYS *wbtsys, GAMESYS_WORK *gsys, BOOL btl_box );

extern GMEVENT * WBT_EVENT_ShowTournamentList(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    WBTLIST_PARAM * wbtlist_param, BOOL call_fade_flag );

extern GMEVENT * WBT_EVENT_TrainerBattle( WBTSYS *wbtsys, GAMESYS_WORK *gsys );
extern GMEVENT * WBT_EVENT_CommBattle(
    WBTSYS *wbtsys, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap );
extern GMEVENT * WBT_EVENT_TrainerBeforeMsg(
    WBTSYS *wbtsys, GAMESYS_WORK *gsys,
    int tr_idx, u16 obj_id, u8 pos_type );

