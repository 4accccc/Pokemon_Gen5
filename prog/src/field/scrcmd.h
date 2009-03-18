//======================================================================
/**
 * @file	scrcmd.h
 * @bfief	スクリプトコマンド用関数
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

//======================================================================
//	定義
//======================================================================
//#define EVCMD_MAX	(565)		//scrcmd.c ScriptCmdTblの要素数
#define SCRCMD_PL_NULL //無効化定義

//======================================================================
//	グローバル変数
//======================================================================
extern const VMCMD_FUNC ScriptCmdTbl[];
extern const u32 ScriptCmdMax;

//======================================================================
//	extern宣言
//======================================================================
#ifndef SCRCMD_PL_NULL
extern BOOL EvCmdWaitSubProcEnd( VM_MACHINE * core );
extern BOOL EvWaitSubProcAndFree( VM_MACHINE * core );
#endif

#endif	/* SCRCMD_H */
