//======================================================================
/**
 *
 * @file 	 scrcmd_player.h
 * @brief	 スクリプトコマンド：プレイヤー関連
 * @date   2009.10.09
 * @author obata GAMEFREAK inc.
 *
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================

extern VMCMD_RESULT EvCmdAddGold( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSubtractGold( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdCheckGold( VMHANDLE * core, void * wk );
