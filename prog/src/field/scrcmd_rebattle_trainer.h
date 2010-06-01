//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_rebattle_trainer.h
 *	@brief  再戦トレーナー用スクリプト
 *	@author	tomoya takahashi
 *	@date		2010.03.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern VMCMD_RESULT EvCmdReBattleTrainerStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdReBattleTrainerEnd( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdReBattleTrainerSetUpTrID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdReBattleTrainerSetUp2vs2TrID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdReBattleTrainerSetUpRndTr( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdReBattleTrainerClearTrEventFlag( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdReBattleTrainerGetRndTrIndex( VMHANDLE *core, void *wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



