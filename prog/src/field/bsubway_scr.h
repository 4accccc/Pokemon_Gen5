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
#include "gamesystem/game_event.h"

#include "bsubway_scr_common.h"
#include "bsubway_scr_def.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void BSUBWAY_SCRWORK_ClearWork( BSUBWAY_SCRWORK **bsw_scr );
extern BSUBWAY_SCRWORK * BSUBWAY_SCRWORK_CreateWork(
    GAMEDATA *gdata, u16 init, u16 playmode );
extern void BSUBWAY_SCRWORK_ReleaseWork( BSUBWAY_SCRWORK *bsw_scr );

extern void BSUBWAY_SCRWORK_SelectPoke(
    BSUBWAY_SCRWORK *wk, GMEVENT *ev, void **proc_wk );
extern BOOL BSUBWAY_SCRWORK_GetEntryPoke(
    BSUBWAY_SCRWORK *wk, void **app_work, GAMEDATA *gdata );
extern int BSUBWAY_SCRWORK_CheckEntryPoke(
    BSUBWAY_SCRWORK *wk, GAMEDATA *gdata );
extern void BSUBWAY_SCRWORK_BtlTrainerNoSet(
    BSUBWAY_SCRWORK *wk, GAMEDATA *gdata );
extern u16 BSUBWAY_SCRWORK_GetNowRound( BSUBWAY_SCRWORK *wk );
extern BOOL BSUBWAY_SCRWORK_IsClear( BSUBWAY_SCRWORK *wk );
extern void BSUBWAY_SCRWORK_SetLoseScore(
    BSUBWAY_SCRWORK *wk, GAMEDATA *gdata );
extern void BSUBWAY_SCRWORK_SetClearScore(
    BSUBWAY_SCRWORK *wk, GAMEDATA *gdata );
extern void BSUBWAY_SCRWORK_SaveRestPlayData( BSUBWAY_SCRWORK *wk );
extern void BSUBWAY_SCRWORK_ChoiceBtlSeven( BSUBWAY_SCRWORK *wk );
extern u16 BSUBWAY_SCRWORK_GetEnemyObj( BSUBWAY_SCRWORK *wk, u16 idx );
#if 0
extern void BSUBWAY_SCRWORK_LocalBattleCall(
    GMEVENT_CONTROL *event,BSUBWAY_SCRWORK *wk,BOOL *win_flag );
#endif
extern u16 BSUBWAY_SCRWORK_GetPlayMode( BSUBWAY_SCRWORK *wk );
extern u16 BSUBWAY_SCRWORK_GetLeaderClearFlag( BSUBWAY_SCRWORK *wk );
extern u16  BSUBWAY_SCRWORK_AddBtlPoint( BSUBWAY_SCRWORK *wk );
