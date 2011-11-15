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


// ポケウッド　録画アプリ呼び出し
extern VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk );
// ポケウッド　再生アプリ呼び出し
extern VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk );
// ポケウッド　再生アプリ呼び出し（チュートリアル用）
extern VMCMD_RESULT EvCmdPokewoodShowTutorial( VMHANDLE *core, void *wk );
// ポケウッド　台本選択
extern VMCMD_RESULT EvCmdPokewoodSelectScenario( VMHANDLE *core, void *wk );
// ポケウッド　上映選択
extern VMCMD_RESULT EvCmdPokewoodSelectMovie( VMHANDLE * core, void *wk );

//　映画のセーブ
extern VMCMD_RESULT EvCmdPokewoodSaveMovie( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetScenarioStatus( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodSetScenarioStatus( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetFlagTotal( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetMoviesCount( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodCheckMovieEnable( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetMovieValue( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodCheckNewScenario( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodCheckNewStarRank( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodIsShowedMovie( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodSetShowedMovie( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodIsRentalPokeMovie( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodSetStudioMMdl( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodGetDirectorType( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodCheckAllSuccess( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdPokewoodRecordMonitor( VMHANDLE * core, void * wk );

// ご褒美アイテム
extern VMCMD_RESULT EvCmdPokewoodGetBonusItem( VMHANDLE * core, void * wk );

extern int  PokewoodStarPoint2StarRank( int point);

#ifdef _cplusplus
}	// extern "C"{
#endif



