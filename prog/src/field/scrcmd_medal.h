//======================================================================
/**
 * @file  scrcmd_medal.h
 * @brief  スクリプトコマンド：メダルイベント関連
 * @date  2011.02.05
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdGetMedalCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdUpdateMedalInfo( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdGetLastAchieveMedalID( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdGetMedal( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdGetMedalOyajiObjID( VMHANDLE * core, void * wk );

