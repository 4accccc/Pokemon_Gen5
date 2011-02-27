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

#include "bsubway_scr.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
typedef BSUBWAY_SCRWORK WBT_SCRWORK; 
//======================================================================
//  extern
//======================================================================
extern GMEVENT * WBT_EVENT_SetSelectPokeList(
    WBT_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, BOOL btl_box );
extern GMEVENT * WBT_EVENT_TrainerBattle(
    WBT_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap );
extern GMEVENT * WBT_EVENT_CommBattle(
    WBT_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap );
extern GMEVENT * WBT_EVENT_TrainerBeforeMsg(
    WBT_SCRWORK *bsw_scr, GAMESYS_WORK *gsys,
    int tr_idx, u16 obj_id, u8 pos_type );

//======================================================================
//  歴代リーダー情報の表示
//======================================================================
