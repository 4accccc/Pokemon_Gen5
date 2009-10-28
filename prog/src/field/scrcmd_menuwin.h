//======================================================================
/**
 * @file	scrcmd_menuwin.h
 * @brief	スクリプトコマンド：メニュー、会話ウィンドウ関連
 * @date  2009.09.13
 * @author	tamada GAMEFREAK inc.
 *
 * 2009.09.13 scrcmd.cより分離させた
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================

extern VMCMD_RESULT EvCmdYesNoWin( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBmpMenuInit( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBmpMenuInitEx( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBmpMenuMakeList( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBmpMenuStart( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGoldWinOpen( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGoldWinUpdate( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGoldWinClose( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdTalkMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdTalkMsgAllPut( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdTalkWinClose( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinWrite( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinTalkWrite( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinClose( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdTrainerMessageSet( VMHANDLE *core, void *wk );

extern BOOL SCREND_CheckEndWin(SCREND_CHECK *end_check , int *seq);

