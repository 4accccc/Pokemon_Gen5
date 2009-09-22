//======================================================================
/**
 * @file	scrcmd_enviroments.h
 * @brief	スクリプトコマンド：システム管理データ関連
 * @date  2009.09.22
 * @author	tamada GAMEFREAK inc.
 *
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdChangeLangID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetLangID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetRand( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetNowMsgArcID( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetTimeZone( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetTrainerCardRank( VMHANDLE *core, void *wk );

