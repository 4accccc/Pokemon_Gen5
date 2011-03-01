//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		scrcmd_infinity_dungeon.h
//   @brief   無限ダンジョン　スクリプト
//   @author	Masayuki Onoue
//   @date		2011.02.24
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/vm.h"

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern VMCMD_RESULT EvCmdInfinityDungeonStart( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdInfinityDungeonWarp( VMHANDLE *core, void *wk );

