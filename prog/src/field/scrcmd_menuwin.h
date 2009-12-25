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
extern VMCMD_RESULT EvCmdBmpMenuStart_Breakable( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGoldWinOpen( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGoldWinUpdate( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGoldWinClose( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdSysWinMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSysWinMsgAllPut( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSysWinClose( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinWrite( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinTalkWrite( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinWriteMF( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinWriteWB( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBalloonWinClose( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdTrainerMessageSet( VMHANDLE *core, void *wk );

extern BOOL SCREND_CheckEndWin(SCREND_CHECK *end_check , int *seq);
extern BOOL SCREND_CheckEndBallonWin(SCREND_CHECK *end_check , int *seq);

extern VMCMD_RESULT EvCmdPlainWinMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlainWinClose( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndPlainWin( SCREND_CHECK *end_check, int *seq );

extern VMCMD_RESULT EvCmdSubWinMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSubWinClose( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndSubWin( SCREND_CHECK *end_check, int *seq );

extern VMCMD_RESULT EvCmdBGWinMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBGWinClose( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndBGWin( SCREND_CHECK *end_check, int *seq );

extern VMCMD_RESULT EvCmdSpWinMsg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSpWinClose( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndSpWin( SCREND_CHECK *end_check, int *seq );

extern VMCMD_RESULT EvCmdMsgWinClose( VMHANDLE *core, void *wk );
