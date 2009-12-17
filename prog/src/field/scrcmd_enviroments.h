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
extern VMCMD_RESULT EvCmdGetRomVersion( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdChangeLangID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetLangID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetRand( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetNowMsgArcID( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetTimeZone( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetTrainerCardRank( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetWeek( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetDate( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetBadgeFlag( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetBadgeFlag( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetBadgeCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetSaveDataStatus( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetWarpID( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetSpLocationHere( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSetSpLocationDirect( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdChangeMapReplaceFlag( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetSeasonID( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdGetZoneID( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdGetBirthDay( VMHANDLE * core, void*wk );
extern VMCMD_RESULT EvCmdGetMySex( VMHANDLE *core, void * wk );
extern VMCMD_RESULT EvCmdSetZukanFlag( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetZukanFlag( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetZukanCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetCGearFlag( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetWirelessSaveMode( VMHANDLE *core, void *wk );

