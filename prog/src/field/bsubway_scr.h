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

#include "savedata/bsubway_savedata.h"
#include "savedata/bsubway_savedata_def.h"

#include "bsubway_scr_def.h"
#include "bsubway_scr_proc.h"

#include "demo/comm_btl_demo.h"

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
extern void BSUBWAY_SCRWORK_ChangeCommMultiMode( BSUBWAY_SCRWORK *bsw_scr );
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
extern void BSUBWAY_SCRWORK_SetHomeWork(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_ChoiceBattlePartner( BSUBWAY_SCRWORK *bsw_scr );
extern u16 BSUBWAY_SCRWORK_GetTrainerOBJCode( BSUBWAY_SCRWORK *bsw_scr, u16 idx );
extern u16 BSUBWAY_SCRWORK_SetWifiRank(
    BSUBWAY_SCOREDATA *score, GAMESYS_WORK *gsys, BSWAY_SETMODE mode );
extern void BSUBWAY_SCRWORK_ChoiceBtlSeven( BSUBWAY_SCRWORK *wk, u8 sex );
extern void BSUBWAY_SCRWORK_PreparBattleBox( BSUBWAY_SCRWORK *bsw_scr );
extern const POKEPARTY * BSUBWAY_SCRWORK_GetPokePartyUse(
    const BSUBWAY_SCRWORK *bsw_scr );

extern u16 BSUBWAY_SCRWORK_GetNowRenshou( const BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_SetNowRenshou( BSUBWAY_SCRWORK *bsw_scr, u16 renshou );
extern void BSUBWAY_SCRWORK_IncNowRenshou( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_ResetNowRenshou( BSUBWAY_SCRWORK *bsw_scr );

extern u32 BSUBWAY_SCRWORK_GetPlayModeMemberNum( u16 mode );
extern u16 BSUBWAY_SCRWORK_RenshouToStageNo( u16 renshou );

//bsubway_tr.h
extern COMM_BTL_DEMO_PARAM * BSUBWAY_SCRWORK_CreateBattleDemoParam(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys );

//bsubway_comm.h
extern void BSUBWAY_COMM_Init( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_COMM_Exit( BSUBWAY_SCRWORK *bsw_scr );

extern void BSUBWAY_COMM_TimingSyncStart( u8 no );
extern BOOL BSUBWAY_COMM_IsTimingSync( u8 no );

extern void BSUBWAY_COMM_AddCommandTable( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_COMM_DeleteCommandTable( BSUBWAY_SCRWORK *bsw_scr );

extern void BSUBWAY_SCRWORK_CreateCommSendData(
    BSUBWAY_SCRWORK *bsw_scr, u16 mode, u16 param );
extern BOOL BSUBWAY_SCRWORK_CommSendData( BSUBWAY_SCRWORK *bsw_scr );
extern void BSUBWAY_SCRWORK_CommRecieveDataStart(
    BSUBWAY_SCRWORK *bsw_scr, u8 comm_mode );
extern BOOL BSUBWAY_SCRWORK_CommRecieveData( BSUBWAY_SCRWORK *bsw_scr, u16 *ret_buf );
#if 0
extern BOOL BSUBWAY_SCRWORK_CommFrWiFiCounterTowerSendBufTrainerData(
    BSUBWAY_SCRWORK *bsw_scr );
#endif
