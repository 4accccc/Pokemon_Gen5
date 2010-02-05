//======================================================================
/**
 * @file  scrcmd_un.h
 * @brief  スクリプトコマンド：国連関連
 * @author  Saito
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdUn_GetCountryNum( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_SetTalkFlg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_CheckTalkFlg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_SetPlayerNature( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_SetCountryInfo( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_GetRoomObjCode( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_GetRoomObjNum( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdUn_GetRoomObjMsg( VMHANDLE *core, void *wk );
