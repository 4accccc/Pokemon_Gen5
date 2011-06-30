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

#ifdef _cplusplus
}	// extern "C"{
#endif



