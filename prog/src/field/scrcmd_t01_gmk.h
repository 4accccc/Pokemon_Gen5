//======================================================================
/**
 * @file  scrcmd_t01_gmk.h
 * @brief  スクリプトコマンド：T01ギミック
 * @author  Saito
 */
//======================================================================
#pragma once

extern VMCMD_RESULT EvCmdT01Gmk_Disp( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdT01Gmk_Start( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdT01Gmk_WaitEnd( VMHANDLE *core, void *wk );

