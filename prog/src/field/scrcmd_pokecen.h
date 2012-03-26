//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_pokecen.h
 *	@brief  ポケモンセンター専用スクリプトコマンド
 *	@author	Masayuki Onoue
 *	@date		2011.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/vm.h"

extern VMCMD_RESULT EvCmdUpdateMedalInfo( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdGetLastAchieveMedalID( VMHANDLE * core, void * wk );

