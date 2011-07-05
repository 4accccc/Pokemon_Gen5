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

// ポケウッド　再生
extern VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk );
// ポケウッド　録画
extern VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk );
// ポケウッド　台本
extern VMCMD_RESULT EvCmdPokewoodBook( VMHANDLE *core, void *wk );
// ポケウッド　上映
extern VMCMD_RESULT EvCmdPokewoodMovies( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetMoviesCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodDeleteMovie( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetFirstTimeSeeingFlag( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetScenarioAddFlag( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



