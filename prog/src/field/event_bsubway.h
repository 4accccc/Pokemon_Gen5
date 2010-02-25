//======================================================================
/**
 * @file  event_bsubway.h
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

//======================================================================
//  extern
//======================================================================
extern GMEVENT * BSUBWAY_EVENT_SetSelectPokeList(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys );
extern GMEVENT * BSUBWAY_EVENT_TrainerBattle(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap );
extern GMEVENT * BSUBWAY_EVENT_CommBattle(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap );
extern GMEVENT * BSUBWAY_EVENT_TrainerBeforeMsg(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, int tr_idx );



//======================================================================
//  歴代リーダー情報の表示
//======================================================================
extern GMEVENT * BSUBWAY_EVENT_CallLeaderBoard(
    GAMESYS_WORK *gsys );

