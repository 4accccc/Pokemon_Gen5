//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_person.h
 *	@brief  人物状態
 *	@author	Toru=Nagihashi
 *	@date		2011.01.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern VMCMD_RESULT EvCmdPersonState_SetState( VMHANDLE *p_vm, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdPersonState_SetLTvtCall( VMHANDLE *p_vm, void *p_wk_adrs );

//隠しキャラ関係
extern VMCMD_RESULT EvCmdPersonState_CheckSecretCalled( VMHANDLE *p_vm, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdPersonState_GetDateType( VMHANDLE *p_vm, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdPersonState_SetDateEnd( VMHANDLE *p_vm, void *p_wk_adrs );
