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

extern TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start( void );
extern void TRIAL_HOUSE_End( TRIAL_HOUSE_WORK_PTR *ptr );
extern void TRIAL_HOUSE_SetPlayMode( TRIAL_HOUSE_WORK_PTR ptr, const u32 inPlayMode );
extern void TRIAL_HOUSE_SetDLFlg( TRIAL_HOUSE_WORK_PTR ptr, const BOOL inDLFlg );

extern GMEVENT *TRIAL_HOUSE_CreateBeforeMsgEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr );

extern int TRIAL_HOUSE_MakeTrainer( TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount );

extern GMEVENT *TRIAL_HOUSE_CreatePokeSelEvt(  GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr,
                                        const int inRegType, const int inPartyType, u16 *outRet );
extern GMEVENT *TRIAL_HOUSE_CreateBtlEvt( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr );



