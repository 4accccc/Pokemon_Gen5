//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_wfbc.h
 *	@brief  WFBC用スクリプトコマンド郡
 *	@author	tomoya takahashi
 *	@date		2009.12.17
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


// WFBCの会話開始・終了
extern VMCMD_RESULT EvCmdWfbc_TalkStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWfbc_TalkEnd( VMHANDLE *core, void *wk );

// 話し相手の前の履歴があるか？
extern VMCMD_RESULT EvCmdWfbc_SetRirekiPlayerName( VMHANDLE *core, void *wk );


// 各パラメータの取得
extern VMCMD_RESULT EvCmdWfbc_GetData( VMHANDLE *core, void *wk );

// BC NPC勝利目標値　加算
extern VMCMD_RESULT EvCmdWfbc_AddBCNpcWinTarget( VMHANDLE *core, void *wk );

// WF 目的ポケモンを持っているのかチェック
extern VMCMD_RESULT EvCmdWfbc_CheckWFTargetPokemon( VMHANDLE *core, void *wk );


#ifdef _cplusplus
}	// extern "C"{
#endif



