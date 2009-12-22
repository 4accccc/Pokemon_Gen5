//======================================================================
/**
 * @file bsubway_scr.c
 * @brief バトルサブウェイ　スクリプト関連
 * @authaor	kagaya
 * @date	2008.12.11
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"

#include "bsubway_scr_def.h"

#include "bsubway_scr_proc.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void BSUBWAY_SCRWORK_ClearWork( GAMESYS_WORK *gsys );
extern BSUBWAY_SCRWORK * BSUBWAY_SCRWORK_CreateWork(
    GAMESYS_WORK *gsys, u16 init, u16 playmode );
extern void BSUBWAY_SCRWORK_ReleaseWork( GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SaveRestPlayData( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SaveGameClearPlayData( BSUBWAY_SCRWORK *bsw_scr );
extern u16 BSUBWAY_SCRWORK_SetNGScore( GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_SetLoseScore(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr );
extern u16 BSUBWAY_SCRWORK_AddBattlePoint( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SetBtlTrainerNo( BSUBWAY_SCRWORK *wk );
extern void BSUBWAY_SCRWORK_ChoiceBattlePartner(BSUBWAY_SCRWORK *wk );

extern void BSUBWAY_SCRWORK_SelectPoke(
    BSUBWAY_SCRWORK *wk, GMEVENT *ev, void **proc_wk );
extern BOOL BSUBWAY_SCRWORK_GetEntryPoke(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern int BSUBWAY_SCRWORK_CheckEntryPoke(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_BtlTrainerNoSet(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern u16 BSUBWAY_SCRWORK_GetNowRound( BSUBWAY_SCRWORK *wk );
extern BOOL BSUBWAY_SCRWORK_IsClear( BSUBWAY_SCRWORK *wk );
extern void BSUBWAY_SCRWORK_SetClearScore(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_ChoiceBtlSeven( BSUBWAY_SCRWORK *wk );
extern u16 BSUBWAY_SCRWORK_GetEnemyObj( BSUBWAY_SCRWORK *wk, u16 idx );
#if 0
extern void BSUBWAY_SCRWORK_LocalBattleCall(
    GMEVENT_CONTROL *event,BSUBWAY_SCRWORK *wk,BOOL *win_flag );
#endif
extern u16 BSUBWAY_SCRWORK_GetPlayMode( BSUBWAY_SCRWORK *wk );
extern u16 BSUBWAY_SCRWORK_GetLeaderClearFlag( BSUBWAY_SCRWORK *wk );
extern u16  BSUBWAY_SCRWORK_AddBtlPoint( BSUBWAY_SCRWORK *wk );
