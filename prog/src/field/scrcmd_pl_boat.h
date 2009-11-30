//======================================================================
/**
 * @file  scrcmd_pl_boat.h
 * @brief  スクリプトコマンド：遊覧船関連
 * @author  Saito
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdPlBoat_Start( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlBoat_End( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlBoat_GetTrNum( VMHANDLE *core, void *wk );
