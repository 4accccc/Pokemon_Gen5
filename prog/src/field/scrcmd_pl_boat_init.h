//======================================================================
/**
 * @file  scrcmd_pl_boat_init.h
 * @brief  スクリプトコマンド：遊覧船関連 NOTWIFI領域
 * @author  Toru=Nagihashi
 */
//======================================================================
#pragma once
#include "system/vm.h"

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdPlBoat_GetRoomInfo( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlBoat_CallDemo( VMHANDLE *core, void *wk );
