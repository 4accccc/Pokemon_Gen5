//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：通信関連
 * @file   scrcmd_network.h
 * @author obata
 * @date   2009.10.19
 *
 */
//////////////////////////////////////////////////////////////////////
#pragma once


extern VMCMD_RESULT EvCmdFieldCommExitWait( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdAvailableWireless( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdRebootBeaconSearch( VMHANDLE * core, void * wk );
