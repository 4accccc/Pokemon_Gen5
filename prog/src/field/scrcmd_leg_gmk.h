//======================================================================
/**
 * @file  scrcmd_leg_gmk.h
 * @brief  スクリプトコマンド：伝説ポケギミックスクリプト
 * @author  Saito
 */
//======================================================================
#pragma once

extern VMCMD_RESULT EvCmdStartLegGmk( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdLegMoveBall( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdLegStartBallAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdLegWaitBallAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdLegWaitBallPokeApp( VMHANDLE *core, void *wk );

