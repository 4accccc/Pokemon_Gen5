//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.h
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//system
#include "system/vm.h"

//def
#include "scrcmd_resort_def.h"
//=============================================================================
/**
 *					スクリプトコマンド外部公開
*/
//=============================================================================
extern VMCMD_RESULT EvCmdResortTalkStart( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortTalkEnd( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetMsgID( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAction( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortJobChange( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortKeepPeople( VMHANDLE *p_core, void *p_wk_adrs );
