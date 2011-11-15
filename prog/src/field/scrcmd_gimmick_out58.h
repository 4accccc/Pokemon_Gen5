//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_out58.h
 *	@brief  スクリプトコマンド  C14ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.12
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

// 水しぶき
extern VMCMD_RESULT EvCmdC14GMK_Splash( VMHANDLE *core, void *wk );
// 水からジャンプ
extern VMCMD_RESULT EvCmdC14GMK_ObjWaterJump( VMHANDLE *core, void *wk );

