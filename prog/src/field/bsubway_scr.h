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
extern void BSUBWAY_SCRWORK_ReleaseWork(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SaveRestPlayData( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_LoadPokemonMember(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_SaveGameClearPlayData( BSUBWAY_SCRWORK *bsw_scr );
extern u16 BSUBWAY_SCRWORK_SetNGScore( GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_SetLoseScore(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr );
extern u16  BSUBWAY_SCRWORK_AddBattlePoint( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SetBtlTrainerNo( BSUBWAY_SCRWORK *wk );
extern BOOL BSUBWAY_SCRWORK_GetEntryPoke( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern BOOL BSUBWAY_SCRWORK_IsClear( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SetClearScore( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_ChoiceBattlePartner( BSUBWAY_SCRWORK *bsw_scr );
extern u16 BSUBWAY_SCRWORK_GetTrainerOBJCode( BSUBWAY_SCRWORK *bsw_scr, u16 idx );

//bsubway_comm.h
extern void BSUBWAY_COMM_AddCommandTable( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_COMM_DeleteCommandTable( BSUBWAY_SCRWORK *bsw_scr );

extern void BSUBWAY_COMM_TimingSyncStart( u8 no );
extern BOOL BSUBWAY_COMM_IsTimingSync( u8 no );

extern u16 BSUBWAY_SCRWORK_CommReceivePlayerData(
    BSUBWAY_SCRWORK *bsw_scr, const u16 *recv_buf );
extern u16 BSUBWAY_SCRWORK_CommReciveTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, const u16* recv_buf );
extern u16 BSUBWAY_SCRWORK_CommReciveRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, const u16 *recv_buf );
extern void BSUBWAY_SCRWORK_CommSendPlayerData(
    BSUBWAY_SCRWORK *bsw_scr, GAMEDATA *gdata );
extern void BSUBWAY_SCRWORK_CommSendTrainerData( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_CommSendRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, u16 retire );
extern BOOL BSUBWAY_SCRWORK_CommSendData(
    BSUBWAY_SCRWORK *bsw_scr, u16 mode, u16 param, GAMEDATA *gdata );
extern BOOL BSUBWAY_SCRWORK_CommFrWiFiCounterTowerSendBufTrainerData(
    BSUBWAY_SCRWORK *bsw_scr );
