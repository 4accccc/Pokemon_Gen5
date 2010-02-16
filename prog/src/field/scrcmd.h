//======================================================================
/**
 * @file	scrcmd.h
 * @brief	スクリプトコマンド用関数
 * @author	Satoshi Nohara
 * @date	05.08.08
 *
 * 05.04.26 Hiroyuki Nakamura
 */
//======================================================================
#ifndef SCRCMD_H
#define SCRCMD_H

#include <gflib.h>
#include "system/vm.h"

#include "scrcmd_work.h"

//======================================================================
//	定義
//======================================================================
#define SCRCMD_PL_NULL //無効化定義

//--------------------------------------------------------------
/// スクリプトイベント　キーシンボル
//--------------------------------------------------------------
enum
{
  EVENT_WAIT_AB_KEY = PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL,
  EVENT_WAIT_PAD_KEY = PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT,
  EVENT_WAIT_LAST_KEY = EVENT_WAIT_AB_KEY|EVENT_WAIT_PAD_KEY 
};

//======================================================================
//	グローバル変数
//======================================================================
extern const VMCMD_FUNC ScriptCmdTbl[];
extern const u32 ScriptCmdMax;

//======================================================================
//	extern宣言
//======================================================================
extern u16 * SCRCMD_GetVMWork( VMHANDLE *core, SCRCMD_WORK *work );
extern u16 SCRCMD_GetVMWorkValue( VMHANDLE * core, SCRCMD_WORK *work );



#endif	/* SCRCMD_H */
