//======================================================================
/**
 * @file  scrcmd_sppoke.h
 * @brief  スクリプトコマンド：特殊ポケイベントスクリプト
 * @author  Saito
 */
//======================================================================
#pragma once

extern VMCMD_RESULT EvCmdMoveBall( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdStartBallAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWaitBallAnm( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWaitBallPokeApp( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetEvtPokePos( VMHANDLE *core, void *wk );
