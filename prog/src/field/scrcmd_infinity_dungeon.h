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
extern VMCMD_RESULT EvCmdInfinityDungeonGetStatus( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonResetSeed( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetFloorNo( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonFloorMove( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdInfinityDungeonGetAreaNo( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonBattleTrainer( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetBossNo( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetBossOBJCode( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonSetTrainerBattleFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetTrainerBattleFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetHintMsgID( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetAfterBtlMsgType( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonGetRecoverFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdInfinityDungeonSetRecoverFlag( VMHANDLE *p_core, void *p_wk_adrs );

