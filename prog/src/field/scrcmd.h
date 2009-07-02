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

#include "scrcmd_work.h"

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
//	スクリプトコマンドを使用するのに必要なインライン関数定義
//======================================================================
//--------------------------------------------------------------
/**
 * インライン関数：ワークを取得する
 * @param	core	仮想マシン制御ワークへのポインタ
 * @retval	u16 *	ワークへのポインタ
 *
 * 次の2バイトをワークを指定するIDとみなして、ワークへのポインタを取得する
 */
//--------------------------------------------------------------
static inline u16 * VMGetWork(VMHANDLE *core, SCRCMD_WORK *work )
{
	GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	return SCRIPT_GetEventWork( sc, gdata, VMGetU16(core) );
}

//--------------------------------------------------------------
/**
 * インライン関数：ワークから値を取得する
 * @param	core	仮想マシン制御ワークへのポインタ
 * @retval	u16		値
 *
 * 次の2バイトがSVWK_START（0x4000以下）であれば値として受け取る。
 * それ以上の場合はワークを指定するIDとみなして、そのワークから値を取得する
 */
//--------------------------------------------------------------
static inline u16 VMGetWorkValue(VMHANDLE * core, SCRCMD_WORK *work)
{
	GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	return SCRIPT_GetEventWorkValue( sc, gdata, VMGetU16(core) );
}

//======================================================================
//	extern宣言
//======================================================================
#ifndef SCRCMD_PL_NULL
extern BOOL EvCmdWaitSubProcEnd( VM_MACHINE * core );
extern BOOL EvWaitSubProcAndFree( VM_MACHINE * core );
#endif

#endif	/* SCRCMD_H */
