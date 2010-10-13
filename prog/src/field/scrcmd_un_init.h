//======================================================================
/**
 * @file  scrcmd_un_init.h
 * @brief  スクリプトコマンド：国連関連 NOTWIFI関連
 * @author  Toru=Nagihashi
 */
//======================================================================
#pragma once
#include "system/vm.h"

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdUn_GetRoomObjCode( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_GetRoomInfo( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_CallFloorSelApp( VMHANDLE *core, void *wk );
