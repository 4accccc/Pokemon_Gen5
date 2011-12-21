//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_sppoke.h
 *	@brief  スクリプトコマンド  特殊ポケギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//system
#include "system/vm.h"
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// レジ３体のギミック
extern VMCMD_RESULT EvCmdRegiGMK_SetEye( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdRegiGMK_SetDoorState( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdRegiGMK_AnmDoorStart( VMHANDLE *core, void *wk );

// 【配布】ケルディオ
extern VMCMD_RESULT EvCmdKerudhioGMK_RememberEvent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdKerudhioGMK_EffectDelete( VMHANDLE *core, void *wk );

