//======================================================================
/**
 * @file  trial_house.h
 * @brief  トライアルハウス
 * @author  Saito
 */
//======================================================================
#pragma once

#include "gamesystem/gamesystem.h"
#include "trial_house_ptr.h"
#include "savedata/bsubway_savedata.h"  //for BSWAY_PLAYMODE

extern TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start( GAMESYS_WORK * gsys );
extern void TRIAL_HOUSE_End( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR *ptr );
extern void TRIAL_HOUSE_SetPlayMode( TRIAL_HOUSE_WORK_PTR ptr, const u32 inPlayMode );
extern void TRIAL_HOUSE_SetDLFlg( TRIAL_HOUSE_WORK_PTR ptr, const BOOL inDLFlg );

extern GMEVENT *TRIAL_HOUSE_CreateBeforeMsgEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr );

extern TRIAL_HOUSE_MakeTrainer( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount );

extern void TRIAL_HOUSE_AddBtlPoint( TRIAL_HOUSE_WORK_PTR ptr, BATTLE_SETUP_PARAM *prm );

extern GMEVENT *TRIAL_HOUSE_CreatePokeSelEvt(  GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr,
                                        const int inRegType, const int inPartyType, u16 *outRet );
extern GMEVENT *TRIAL_HOUSE_CreateBtlEvt( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr );

extern GMEVENT *TRIAL_HOUSE_CreateBeaconSearchEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, u16* outRet );

extern void TRIAL_HOUSE_CalcBtlResult( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, u16 *outRank, u16 *outPoint );

extern u32 TRIAL_HOUSE_GetDLDataType( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr );
extern void TRIAL_HOUSE_InvalidDLData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr );
extern GMEVENT *TRIAL_HOUSE_CreateRankAppEvt( GAMESYS_WORK * gsys, const BOOL inIsDL );
extern u32 TRIAL_HOUSE_GetRankDataState( GAMESYS_WORK * gsys );


