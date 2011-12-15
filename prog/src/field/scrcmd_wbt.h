//======================================================================
/**
 * @file  scrcmd_wbt.h
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.21
 * @brief
 */
//======================================================================
#pragma once

#include "system/vm.h"

extern VMCMD_RESULT EvCmdWBTSystemStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSystemEnd( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSystemCheckEnable( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSystemSetStage( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSystemGetStage( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSystemEntry( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTCheckCupEnable( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTCheckCupUnlock( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTCheckBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetRegulationID( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdWBTGetNGMessageID( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetWBTCup( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetWBTCup( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetTournamentPokeType( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetTournamentPokeType( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetReceptionID( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetReceptionID( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetVictoryCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTIncVictoryCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetSurvivalCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetReward( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetTrainerFlag( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetLobbyTrainerID( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetLobbyMMdlCode( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetPartyMax( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTGetDownLoadCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetDownLoadName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetDLData( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetDLNumber( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTMixInfoWinClose( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixInfoWinOpen( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixInit( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixSetSelect( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixRandomSelect( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixGetSelectedName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixListStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTMixSetupCheckMsg( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetupMixParty( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTSetupRentalParty( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetupEnemyParty( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetEnemyMMdlCode( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTTrainerMsg( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetTournamentName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetEnemyName( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTBtlUtilPartySelect( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdWBTShowTornamentList( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSelectPokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSelectDLData( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTECheckerRecord( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTECheckerDownLoad( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTGMKVisionChange( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKEffectStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKWinEffectStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKLightSet( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKExplosion( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTBtlUtilCheckPartySelect( VMHANDLE * core, void *wk );

#include "wbt_system.h"
#include "scrcmd_work.h"
//--------------------------------------------------------------
//  ã§í ÉcÅ[Éãä÷êî
//--------------------------------------------------------------
extern WBTSYS * WBT_SCR_getWBTSys( SCRCMD_WORK * work );

