//======================================================================
/**
 * @file	scrcmd_mapchange.h
 * @brief	スクリプトコマンド：マップ遷移関連
 * @date  2009.09.11
 * @author	tamada GAMEFREAK inc.
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdMapChange( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapChangeBGMKeep( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapChangeBySandStream(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdMapChangeToUnion( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapChangeNoFade( VMHANDLE *core, void *wk );


