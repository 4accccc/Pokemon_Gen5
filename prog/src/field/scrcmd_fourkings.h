//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_fourkings.h
 *	@brief  四天王スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2009.11.25
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/vm.h"

#ifdef _cplusplus
extern "C"{
#endif


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define FIELD_EVENT_FOURKINGS_MAX (4)
#define RAIL_NUM  (3)   // 各部屋レールのライン数は一定(変更する可能性も有る)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// 共通
extern VMCMD_RESULT EvCmdFourKings_RailOff( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFourKings_SetCameraParam( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFourKings_SetSoundSystem( VMHANDLE *core, void *wk );

// 悪
extern VMCMD_RESULT EvCmdFourKings_Evil_Fire( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFourKings_Evil_Claw( VMHANDLE *core, void *wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



