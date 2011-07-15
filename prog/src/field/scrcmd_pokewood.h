//============================================================================================
/**
 *	@file		scrcmd_pokewood.h
 *	@brief  ポケウッドスクリプトコマンド
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.06.30
 *
 */
//============================================================================================
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "system/vm.h"

extern VMCMD_RESULT EvCmdPokewoodSystemStart( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodSystemEnd( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdPKWDSelectPokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPKWDSetJoinMypoke( VMHANDLE *core, void *wk );


// ポケウッド　再生
extern VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk );
// ポケウッド　録画
extern VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk );
// ポケウッド　台本選択
extern VMCMD_RESULT EvCmdPokewoodSelectScenario( VMHANDLE *core, void *wk );
// ポケウッド　上映選択
extern VMCMD_RESULT EvCmdPokewoodSelectMovie( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetScenarioStatus( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetMoviesCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodDeleteMovie( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetFirstTimeSeeingFlag( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodCheckMovieEnable( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetMovieValue( VMHANDLE * core, void * wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



