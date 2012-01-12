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
extern VMCMD_RESULT EvCmdResortSetData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetMsgID( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortJobChange( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetWord( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortDeletePeople( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortMergeOff( VMHANDLE *p_core, void *p_wk_adrs );

extern VMCMD_RESULT EvCmdResortLot( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddKindly( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddLv( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddEffort( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortHatch( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetDayFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortEndService( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetSpot( VMHANDLE *p_core, void *p_wk_adrs );

extern VMCMD_RESULT EvCmdResortCallInputName( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortEndItem( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetNpcData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetMyData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetMyData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetMouthComm( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetWordMouthCommAnswer( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetOffreco( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetServiceData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortStartEffect( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortWaitEffect( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortEndEffect( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetPresetPoeple( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortPrintRankWindow( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddPeople( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortResetMmdl( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddExpShop( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddExpCity( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetMergeLock( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetSeceneVisible( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortCombo( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortCheckPrize( VMHANDLE *p_core, void *p_wk_adrs );
