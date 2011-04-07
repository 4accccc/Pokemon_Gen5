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
extern VMCMD_RESULT EvCmdWBTSystemSetStage( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTShowTornamentList( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTCheckBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetBattleRule( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetTournamentType( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetTournamentType( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetDownLoadCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetDownLoadName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSelectDLData( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetDLNumber( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetDLNumber( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTSelectPokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetEnemyMMdlCode( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTTrainerMsg( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetTournamentName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTSetEnemyName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGetVictoryCount( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdWBTGMKVisionChange( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKEffectStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdWBTGMKWinEffectStart( VMHANDLE * core, void * wk );

