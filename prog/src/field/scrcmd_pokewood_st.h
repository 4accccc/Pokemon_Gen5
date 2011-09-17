//============================================================================================
/**
 *	@file		scrcmd_pokewood_st.h
 *	@brief  ポケウッドスクリプトコマンド（常駐部分）
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.09.17
 *
 */
//============================================================================================
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "system/vm.h"

extern VMCMD_RESULT EvCmdPokewoodGetStarRank( VMHANDLE * core, void * wk );

