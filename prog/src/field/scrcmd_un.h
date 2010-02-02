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
extern VMCMD_RESULT EvCmdUn_SetContryInfo( VMHANDLE *core, void *wk );
