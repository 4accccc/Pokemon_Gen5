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
extern VMCMD_RESULT EvCmdResortJobChange( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortKeepPeople( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetWord( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortDeletePeple( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortCheckVisitor( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortJobChangeEx( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortMergeOff( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortNewInstatitutionEntrance( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSearchReception( VMHANDLE *p_core, void *p_wk_adrs );

extern VMCMD_RESULT EvCmdResortLot( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddKindly( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddLv( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAddEffort( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortHatch( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetDayFlag( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetInstitutionData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetInstitutionData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortEndService( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetSpot( VMHANDLE *p_core, void *p_wk_adrs );
