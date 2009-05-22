//======================================================================
/**
 * @file	scrcmd.c
 * @bfief	スクリプトコマンド用関数
 * @author	Satoshi Nohara
 * @date	05.08.04
 *
 * 05.04.26 Hiroyuki Nakamura
 */
//======================================================================
#include <nitro/code16.h> 
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "msgdata.h"

//======================================================================
//	定義
//======================================================================
typedef u16 (* pMultiFunc)();

//レジスタ比較の結果定義
enum {
	MINUS_RESULT = 0,	//比較結果がマイナス
	EQUAL_RESULT,		//比較結果がイコール
	PLUS_RESULT			//比較結果がプラス
};

//======================================================================
//	プロトタイプ宣言
//======================================================================
static VMCMD_RESULT EvCmdNop( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdDummy( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdEnd( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdTimeWait( VMHANDLE * core, void *wk );
static BOOL EvWaitTime(VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdLoadRegValue( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadRegWData( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadRegAdrs( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadAdrsValue( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadAdrsReg( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadRegReg( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdLoadAdrsAdrs( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdCmpMain( u16 r1, u16 r2 );
static VMCMD_RESULT EvCmdCmpRegReg( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpRegValue( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpRegAdrs( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpAdrsReg( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpAdrsValue(VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpAdrsAdrs(VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpWkValue( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdCmpWkWk( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdVMMachineAdd( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdChangeCommonScr( VMHANDLE * core, void *wk );
static BOOL EvChangeCommonScrWait(VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdChangeLocalScr( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdGlobalJump( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdObjIDJump( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdBgIDJump( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdPlayerDirJump( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdGlobalCall( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdRet( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdIfJump( VMHANDLE * core, void *wk );
static VMCMD_RESULT EvCmdIfCall(VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdABKeyWait( VMHANDLE * core, void *wk );
static BOOL EvWaitABKey( VMHANDLE * core, void *wk );

static VMCMD_RESULT EvCmdTalkMsgAllPut( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdChangeLangID( VMHANDLE *core, void *wk );

static VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdTalkWinClose( VMHANDLE *core, void *wk );

static VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk );
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk);

static VMCMD_RESULT EvCmdObjPauseAll( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdTalkObjPauseAll( VMHANDLE *core, void *wk );
static BOOL EvWaitTalkObj( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdObjPauseClearAll( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdObjTurn( VMHANDLE *core, void *wk );

static VMCMD_RESULT EvCmdYesNoWin( VMHANDLE *core, void *wk );
static BOOL EvYesNoWinSelect( VMHANDLE *core, void *wk );

//======================================================================
//	グローバル変数
//======================================================================
//--------------------------------------------------------------
///	条件分岐用テーブル
//--------------------------------------------------------------
static const u8 ConditionTable[6][3] =
{
//	  MINUS  EQUAL  PLUS
	{ TRUE,	 FALSE, FALSE },	// LT
	{ FALSE, TRUE,  FALSE },	// EQ
	{ FALSE, FALSE, TRUE  },	// GT
	{ TRUE,  TRUE,  FALSE },	// LE
	{ FALSE, TRUE,  TRUE  },	// GE
	{ TRUE,  FALSE, TRUE  }		// NE
};

//--------------------------------------------------------------
///	スクリプトコマンドテーブル
//--------------------------------------------------------------
const VMCMD_FUNC ScriptCmdTbl[] = {
	EvCmdNop,				
	EvCmdDummy,				
	
	EvCmdEnd,			
	EvCmdTimeWait,
	
	EvCmdLoadRegValue,
	EvCmdLoadRegWData,
	EvCmdLoadRegAdrs,
	EvCmdLoadAdrsValue,
	EvCmdLoadAdrsReg,
	EvCmdLoadRegReg,
	EvCmdLoadAdrsAdrs,
	
	EvCmdCmpRegReg,
	EvCmdCmpRegValue,
	EvCmdCmpRegAdrs,		
	EvCmdCmpAdrsReg,		
	EvCmdCmpAdrsValue,
	EvCmdCmpAdrsAdrs,
	EvCmdCmpWkValue,
	EvCmdCmpWkWk,
	
	EvCmdVMMachineAdd,
	EvCmdChangeCommonScr,
	EvCmdChangeLocalScr,
	
	EvCmdGlobalJump,
	EvCmdObjIDJump,
	EvCmdBgIDJump,
	EvCmdPlayerDirJump,
	
	EvCmdGlobalCall,
	EvCmdRet,
	
	EvCmdIfJump,
	EvCmdIfCall,

	EvCmdABKeyWait,
	
	EvCmdTalkMsgAllPut,
	
	EvCmdTalkWinOpen,
	EvCmdTalkWinClose,
	
	EvCmdObjAnime,
	EvCmdObjAnimeWait,
	
	EvCmdObjPauseAll,
	EvCmdTalkObjPauseAll,
	EvCmdObjPauseClearAll,
	EvCmdObjTurn,
  
  EvCmdYesNoWin,

  EvCmdChangeLangID,
};

//--------------------------------------------------------------
///	スクリプトコマンドの最大数
//--------------------------------------------------------------
const u32 ScriptCmdMax = NELEMS(ScriptCmdTbl);

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
///	動作モデル監視に使用するインライン関数定義
//======================================================================
#define	PLAYER_BIT			(1<<0)
#define	PLAYER_PAIR_BIT		(1<<1)
#define	OTHER_BIT			(1<<2)
#define	OTHER_PAIR_BIT		(1<<3)

static u8 step_watch_bit;

static inline void InitStepWatchBit(void)
{
	step_watch_bit = 0;
}

static inline BOOL CheckStepWatchBit(int mask)
{
	return (step_watch_bit & mask) != 0;
}

static inline void SetStepWatchBit(int mask)
{
	step_watch_bit |= mask;
}

static inline void ResetStepWatchBit(int mask)
{
	step_watch_bit &= (0xff ^ mask);
}

//======================================================================
//	基本システム命令
//======================================================================
//--------------------------------------------------------------
/**
 * ＮＯＰ命令（なにもしない）
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdNop( VMHANDLE *core, void *wk )
{
	return 0;
}

//--------------------------------------------------------------
/**
 * 何もしない（デバッガで引っ掛けるための命令）
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdDummy( VMHANDLE *core, void *wk )
{
	return 0;
}

//--------------------------------------------------------------
/**
 * スクリプトの終了
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdEnd( VMHANDLE *core, void *wk )
{
	VM_End( core );
	return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ウェイト処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTimeWait( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 num				= VMGetU16( core );
	u16 wk_id			= VMGetU16( core );
	u16 *ret_wk			= SCRIPT_GetEventWork( sc, gdata, wk_id );
	
	*ret_wk = num;
	
	//仮想マシンの汎用レジスタにワークのIDを格納
	core->vm_register[0] = wk_id;
	VMCMD_SetWait( core, EvWaitTime );
	return 1;
}

//return 1 = 終了
static BOOL EvWaitTime(VMHANDLE * core, void *wk )
{
	SCRCMD_WORK *work = wk;
	GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, core->vm_register[0] ); //注意！

	(*ret_wk)--;
	if( *ret_wk == 0 ){ 
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
#ifndef SCRCMD_PL_NULL
static VMCMD_RESULT EvCmdDebugWatch(VMHANDLE * core, void *wk )
{
	u16 value = VMGetWorkValue(core);
	OS_Printf("SCR WORK: %d\n", value);
	return 0;
}
#endif

//======================================================================
//	データロード・ストア関連
//======================================================================
//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタに1byteの値を格納
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegValue( VMHANDLE *core, void *wk )
{
	u8	r = VMGetU8( core );
	core->vm_register[r] = VMGetU8( core );
	return 0;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタに4byteの値を格納
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegWData( VMHANDLE *core, void *wk )
{
	u8	r;
	u32	wdata;
	
	r = VMGetU8( core );
	wdata = VMGetU32( core );
	core->vm_register[r] = wdata;
	return 0;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタにアドレスを格納
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegAdrs( VMHANDLE *core, void *wk )
{
	u8	r;
	VM_CODE * adrs;

	r = VMGetU8( core );
	adrs = (VM_CODE *)VMGetU32( core );
	core->vm_register[r] = *adrs;
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身に値を代入
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsValue( VMHANDLE *core, void *wk )
{
	VM_CODE * adrs;
	u8	r;

	adrs = (VM_CODE *)VMGetU32( core );
	r = VMGetU8( core );
	*adrs = r;
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身に仮想マシンの汎用レジスタの値を代入
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsReg( VMHANDLE *core, void *wk )
{
	VM_CODE * adrs;
	u8	r;

	adrs = (VM_CODE *)VMGetU32( core );
	r = VMGetU8( core) ;
	*adrs = core->vm_register[r];
	return 0;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタの値を汎用レジスタにコピー
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegReg( VMHANDLE *core, void *wk )
{
	u8	r1, r2;

	r1 = VMGetU8( core );
	r2 = VMGetU8( core );
	core->vm_register[r1] = core->vm_register[r2];
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身にアドレスの中身を代入
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsAdrs( VMHANDLE *core, void *wk )
{
	VM_CODE * adr1;
	VM_CODE * adr2;

	adr1 = (VM_CODE *)VMGetU32( core );
	adr2 = (VM_CODE *)VMGetU32( core );
	*adr1 = * adr2;
	return 0;
}

//======================================================================
//	比較命令
//======================================================================
//--------------------------------------------------------------
/**
 * ２つの値を比較
 * @param	r1		値１
 * @param	r2		値２
 * @retval	"r1 < r2 : MISUS_RESULT"
 * @retval	"r1 = r2 : EQUAL_RESULT"
 * @retval	"r1 > r2 : PLUS_RESULT"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpMain( u16 r1, u16 r2 )
{
	if( r1 < r2 ){
		return MINUS_RESULT;
	}else if( r1 == r2 ){
		return EQUAL_RESULT;
	}
	return PLUS_RESULT;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタを比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegReg( VMHANDLE *core, void *wk )
{
	u8	r1, r2;

	r1 = core->vm_register[ VMGetU8(core) ];
	r2 = core->vm_register[ VMGetU8(core) ];
	core->cmp_flag = EvCmdCmpMain( r1, r2 );
	return 0;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタと値を比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegValue( VMHANDLE *core, void *wk )
{
	u8	r1, r2;

	r1 = core->vm_register[ VMGetU8(core) ];
	r2 = VMGetU8(core);
	core->cmp_flag = EvCmdCmpMain( r1, r2 );
	return 0;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタとアドレスの中身を比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegAdrs( VMHANDLE *core, void *wk )
{
	u8	r1, r2;

	r1 = core->vm_register[ VMGetU8(core) ];
	r2 = *(VM_CODE *)VMGetU32(core);
	core->cmp_flag = EvCmdCmpMain( r1, r2 );
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身と仮想マシンの汎用レジスタを比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsReg( VMHANDLE *core, void *wk )
{
	u8	r1, r2;

	r1 = *(VM_CODE *)VMGetU32(core);
	r2 = core->vm_register[ VMGetU8(core) ];
	core->cmp_flag = EvCmdCmpMain(r1,r2);
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身と値を比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsValue(VMHANDLE * core, void *wk)
{
	u8 r1,r2;

	r1 = *(VM_CODE *)VMGetU32(core);
	r2 = VMGetU8(core);
	core->cmp_flag = EvCmdCmpMain(r1,r2);
	return 0;
}

//--------------------------------------------------------------
/**
 * アドレスの中身とアドレスの中身を比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsAdrs(VMHANDLE * core, void *wk)
{
	u8 r1,r2;

	r1 = *(VM_CODE *)VMGetU32(core);
	r2 = *(VM_CODE *)VMGetU32(core);
	core->cmp_flag = EvCmdCmpMain(r1,r2);
	return 0;
}

//--------------------------------------------------------------
/**
 * ワークと値を比較
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpWkValue( VMHANDLE *core, void *wk )
{
	u16 *p;
	u16	r1, r2;

	p = VMGetWork( core, wk );
	r1 = *p;
	r2 = VMGetU16( core );
	core->cmp_flag = EvCmdCmpMain( r1, r2 );
	return 0;
}

//--------------------------------------------------------------
/**
 * ワークとワークを比較
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpWkWk( VMHANDLE *core, void *wk )
{
	u16 * wk1;
	u16 * wk2;

	wk1 = VMGetWork( core, wk );
	wk2 = VMGetWork( core, wk );
	core->cmp_flag = EvCmdCmpMain( *wk1, *wk2 );
	return 0;
}

//======================================================================
//	仮想マシン関連
//======================================================================
//--------------------------------------------------------------
/**
 * 仮想マシン追加(切り替えはせず、並列で動作します！)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdVMMachineAdd( VMHANDLE *core, void *wk )
{
	u16 id;
	SCRCMD_WORK *work = wk;
	GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	
	u8 *vm_machine_count = SCRIPT_GetMemberWork( sc, ID_EVSCR_VMHANDLE_COUNT );
	VMHANDLE **vm = SCRIPT_GetMemberWork( sc, ID_EVSCR_VM_SUB1 );

	id = VMGetU16(core);

	//仮想マシン追加
	//*vm = VMMachineAdd(fsys, id, &ScriptCmdTbl[0], &ScriptCmdTbl[EVCMD_MAX]);
	*vm = SCRIPT_AddVMachine( gsys, sc, SCRCMD_WORK_GetHeapID(work), id );
	(*vm_machine_count)++;

	//イベントと切り離したTCB動作にするかも？
	//*vm = VMMachineAddTCB( fsys, id, &ScriptCmdTbl[0], &ScriptCmdTbl[EVCMD_MAX] );
	return 1;
}

//--------------------------------------------------------------
/**
 * ローカルスクリプトをウェイト状態にして、共通スクリプトを動作させます
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdChangeCommonScr( VMHANDLE *core, void *wk )
{
	u16 scr_id;
	SCRCMD_WORK *work = wk;
	GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u8* common_scr_flag = SCRIPT_GetMemberWork(sc, ID_EVSCR_COMMON_SCR_FLAG);
	u8 *vm_machine_count = SCRIPT_GetMemberWork( sc, ID_EVSCR_VMHANDLE_COUNT );
	VMHANDLE **vm = SCRIPT_GetMemberWork( sc, ID_EVSCR_VM_SUB1 );

	scr_id = VMGetU16(core);

	//共通スクリプト以外にも切り替え可能になっている！

	//共通スクリプト切り替えフラグON
	*common_scr_flag = 1;

	//仮想マシン追加
	//*vm = VMMachineAdd(fsys, scr_id, &ScriptCmdTbl[0], &ScriptCmdTbl[EVCMD_MAX]);
	*vm = SCRIPT_AddVMachine( gsys, sc, SCRCMD_WORK_GetHeapID(work), scr_id );
	(*vm_machine_count)++;

	VMCMD_SetWait( core, EvChangeCommonScrWait );
	return 1;
}

//return 1 = 終了
static BOOL EvChangeCommonScrWait(VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u8* common_scr_flag = SCRIPT_GetMemberWork(sc,ID_EVSCR_COMMON_SCR_FLAG);

	if( *common_scr_flag == 0 ){
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------
/**
 * 共通スクリプトを終了して、ローカルスクリプトを再開させます
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdChangeLocalScr( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u8* common_scr_flag = SCRIPT_GetMemberWork(sc, ID_EVSCR_COMMON_SCR_FLAG);

	//共通スクリプト切り替えフラグOFF
	*common_scr_flag = 0;

	//VM_End( core );
	return 0;	//注意！　この後に"END"に行くようにする
}

//======================================================================
//	分岐命令
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプトジャンプ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 * @li	EVCMD_JUMP
 *	表記：	EVCMD_JUMP	JumpOffset(s16)
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGlobalJump( VMHANDLE *core, void *wk )
{
	s32	pos;
	pos = (s32)VMGetU32(core);
	VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );	//JUMP
	return 0;
}

//--------------------------------------------------------------
/**
 * 話し掛け対象OBJID比較ジャンプ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjIDJump( VMHANDLE *core, void *wk )
{
	u8 id;
	s32	pos;
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	FLDMMDL **fldobj;
	
	//話し掛け対象OBJ
	fldobj = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
	
	//比較する値
	id = VMGetU8(core);
	
	//飛び先
	pos = (s32)VMGetU32(core);
	
	//話し掛け対象OBJと、比較する値が同じか
	if( FLDMMDL_GetOBJID(*fldobj) == id ){
		VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );	//JUMP
	}
	return 0;
}

//--------------------------------------------------------------
/**
 * 話し掛け対象BG比較ジャンプ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBgIDJump( VMHANDLE *core, void *wk )
{
#ifndef SCRCMD_PL_NULL
	u8 id;
	s32	pos;
	//u32 *targetbg;
	u32 targetbg;

	//話し掛け対象BG
	targetbg = Event_GetTargetBg( core->event_work );

	//比較する値
	id = VMGetU8(core);
	
	//飛び先
	pos = (s32)VMGetU32(core);

	//話し掛け対象BGと、比較する値が同じか
	//if( *targetbg == id ){
	if( targetbg == id ){
		VMJump( core, (VM_CODE *)(core->adrs+pos) );	//JUMP
	}
	return 0;
#else
	GF_ASSERT( 0 && "WB未対応\n" );
	return 0;
#endif
}

//--------------------------------------------------------------
/**
 * イベント起動時の主人公の向き比較ジャンプ
 * (現在の向きではないので注意！)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPlayerDirJump( VMHANDLE *core, void *wk )
{
#ifndef SCRCMD_PL_NULL
	u8 dir;
	s32	pos;
	int* player_dir;
	FIELDSYS_WORK* fsys = core->fsys;

	player_dir = GetEvScriptWorkMemberAdrs( fsys, ID_EVSCR_PLAYER_DIR );

	//比較する値
	dir = VMGetU8(core);
	
	//飛び先
	pos = (s32)VMGetU32(core);

	if( *player_dir == dir ){
		VMJump( core, (VM_CODE *)(core->adrs+pos) );	//JUMP
	}
	return 0;
#else
	GF_ASSERT( 0 && "WB未対応\n" );
	return 0;
#endif
}

//--------------------------------------------------------------
/**
 * スクリプトコール
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 * @li	EVCMD_CALL
 *	表記：	EVCMD_CALL	CallOffset(s16)
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGlobalCall( VMHANDLE *core, void *wk )
{
	s32	pos = (s32)VMGetU32(core);
	VMCMD_Call( core, (VM_CODE *)(core->adrs+pos) );
	return 0;
}

//--------------------------------------------------------------
/**
 * スクリプトリターン
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdRet( VMHANDLE *core, void *wk )
{
	VMCMD_Return( core );
	return 0;
}

//--------------------------------------------------------------
/**
 * スクリプト条件ジャンプ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdIfJump( VMHANDLE *core, void *wk )
{
	u8	r;
	s32	pos;

	r   = VMGetU8(core);
	pos = (s32)VMGetU32(core);

	if( ConditionTable[r][core->cmp_flag] == TRUE ){
		VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );
	}
	return 0;
}

//--------------------------------------------------------------
/**
 * スクリプト条件コール
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdIfCall( VMHANDLE *core, void *wk )	
{
	u8	r;
	s32	pos;

	r   = VMGetU8(core);
	pos = (s32)VMGetU32(core);

	if( ConditionTable[r][core->cmp_flag] == TRUE ){
		VMCMD_Call( core, (VM_CODE *)(core->adrs+pos) );
	}
	return 0;
}

//======================================================================
//	キー入力関連
//======================================================================
//--------------------------------------------------------------
/**
 * キーウェイト
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	常に1
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdABKeyWait( VMHANDLE * core, void *wk )
{
	VMCMD_SetWait( core, EvWaitABKey );
	return VMCMD_RESULT_SUSPEND;
}

//return 1 = 終了
static BOOL EvWaitABKey( VMHANDLE * core, void *wk )
{
	int trg = GFL_UI_KEY_GetTrg();
	
	if( trg & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
		return 1;
	}

	return 0;
}

//======================================================================
//	フィールド　メッセージ表示
//======================================================================
static BOOL TalkMsgWait( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	FLDMSGWIN_STREAM *msgWin;
  msgWin = SCRCMD_WORK_GetFldMsgWinStream( work );
  
  if( FLDMSGWIN_STREAM_Print(msgWin) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　メッセージ表示
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	常に1
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkMsg( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	u8 msg_id = VMGetU8(core);
	FLDMSGWIN_STREAM *msgWin;
  
	msgWin = SCRCMD_WORK_GetFldMsgWinStream( work );
	FLDMSGWIN_STREAM_PrintStart( msgWin, 0, 0, msg_id );
  VMCMD_SetWait( core, TalkMsgWait );
	return 1;
}

//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　メッセージ表示
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkMsgAllPut( VMHANDLE *core, void *wk )
{
#if 0
	SCRCMD_WORK *work = wk;
	u8 msg_id = VMGetU8(core);
	FLDMSGWIN_STREAM *msgWin;
	
	msgWin = SCRCMD_WORK_GetFldMsgWinStream( work );
	FLDMSGWIN_STREAM_PrintStart( msgWin, 0, 0, msg_id );
	return 0;
#else
  return( EvCmdTalkMsg(core,wk) );
#endif
}

//--------------------------------------------------------------
/**
 * 言語IDを切り替え
 * @param
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdChangeLangID( VMHANDLE *core, void *wk )
{
  u8 id = GFL_MSGSYS_GetLangID();
  if( id == 0 ){ //ひらがな
    id = 1;
  }else if( id == 1 ){ //漢字
    id = 0;
  }
  GFL_MSGSYS_SetLangID( id );
  return 0;
}

//======================================================================
//	フィールド　会話ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 会話ウィンドウ表示
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc;
	SCRIPT_FLDPARAM *fparam;
	GFL_MSGDATA *msgData;
	FLDMSGWIN_STREAM *msgWin;
	u8 *win_open_flag;
	
	sc = SCRCMD_WORK_GetScriptWork( work );
	fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
	msgData = SCRCMD_WORK_GetMsgData( work );
	msgWin = FLDMSGWIN_STREAM_AddTalkWin( fparam->msgBG, msgData );
	SCRCMD_WORK_SetFldMsgWinStream( work, msgWin );

	win_open_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_OPEN_FLAG );
	*win_open_flag = TRUE;	//ON;
	return 0;
}

//--------------------------------------------------------------
/**
 * 会話ウィンドウを閉じる
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkWinClose( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc;
	FLDMSGWIN_STREAM *msgWin;
	u8 *win_open_flag;
	
	msgWin = SCRCMD_WORK_GetFldMsgWinStream( work );
	FLDMSGWIN_STREAM_Delete( msgWin );
	
	sc = SCRCMD_WORK_GetScriptWork( work );
	win_open_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_OPEN_FLAG );
	*win_open_flag = FALSE;	//OFF;
	return 0;
}

//======================================================================
//	動作モデル	
//======================================================================
static FLDMMDL * FieldObjPtrGetByObjId( SCRCMD_WORK *work, u16 obj_id );
static void EvAnmSetTCB(
	SCRCMD_WORK *work, GFL_TCB *anm_tcb, FLDMMDL_ACMD_LIST *list );

//--------------------------------------------------------------
/**
 * アニメーション
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk )
{
	u8 *num;
	VM_CODE *p;
	GFL_TCB *anm_tcb;
	FLDMMDL *fmmdl; //対象のフィールドOBJのポインタ
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 obj_id = VMGetWorkValue(core,work); //obj ID
	u32 pos = VMGetU32(core); //list pos
	
	fmmdl = FieldObjPtrGetByObjId( work, obj_id );
	
	//エラーチェック
	if( fmmdl == NULL ){
		OS_Printf( "obj_id = %d\n", obj_id );
		GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
		return 0;				//08.06.12 プラチナで追加
	}
	
	//アニメーションコマンドリストセット
	p = (VM_CODE*)(core->adrs+pos);
	anm_tcb = FLDMMDL_SetAcmdList( fmmdl, (FLDMMDL_ACMD_LIST*)p );
	
	//アニメーションの数を足す
	num = SCRIPT_GetMemberWork( sc, ID_EVSCR_ANMCOUNT );
	(*num)++;
	
	//TCBセット
	EvAnmSetTCB( work, anm_tcb, NULL );
	return 0;
}

//--------------------------------------------------------------
/**
 * アニメーション終了待ち
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk )
{
	VMCMD_SetWait( core, EvObjAnimeWait );
	return 1;
}

//return 1 = 終了
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk )
{
	SCRCMD_WORK *work = wk;
	
	if( SCRCMD_WORK_CheckFldMMdlAnmTCB(work) == FALSE ){
		return 1;
	}
	
	return 0;
}

//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param	fsys	FIELDSYS_WORK型のポインタ
 * @param	obj_id	OBJID
 * @retval	"FIELD_OBJ_PTR"
 */
//--------------------------------------------------------------
static FLDMMDL * FieldObjPtrGetByObjId( SCRCMD_WORK *work, u16 obj_id )
{
	FLDMMDL *dummy;
	FLDMMDL *fmmdl;
	FLDMMDLSYS *fmmdlsys;
	
	fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );
	
	//連れ歩きOBJ判別IDが渡された時
	if( obj_id == SCR_OBJID_MV_PAIR ){
		fmmdl = FLDMMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
	//透明ダミーOBJ判別IDが渡された時
	}else if( obj_id == SCR_OBJID_DUMMY ){
		SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
		dummy = SCRIPT_GetMemberWork( sc, ID_EVSCR_DUMMY_OBJ );
	//対象のフィールドOBJのポインタ取得
	}else{
		fmmdl = FLDMMDLSYS_SearchOBJID( fmmdlsys, obj_id );
	}
	
	return fmmdl;
}

//--------------------------------------------------------------
/**
 * アニメ終了監視TCB セット
 * @param	fsys	FIELDSYS_WORK型のポインタ
 * @param	anm_tcb	TCB_PTR型
 * @retval	none
 */
//--------------------------------------------------------------
static void EvAnmSetTCB(
	SCRCMD_WORK *work, GFL_TCB *anm_tcb, FLDMMDL_ACMD_LIST *list )
{
#if 0
	EV_ANM_WORK* wk = NULL;
	wk = sys_AllocMemory(HEAPID_FIELD, sizeof(EV_ANM_WORK));

	if( wk == NULL ){
		GF_ASSERT( (0) && "scrcmd.c メモリ確保失敗！" );
		return;
	}

	wk->fsys	= fsys;
	wk->anm_tcb	= anm_tcb;
	wk->list	= list;
	wk->tcb		= TCB_Add( EvAnmMainTCB, wk, 0 );
	return;
#else
	SCRCMD_WORK_SetFldMMdlAnmTCB( work, anm_tcb );
#endif
}

//======================================================================
//	動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 * 自機FLDMMDL取得　仮
 * @param
 * @retval
 */
//--------------------------------------------------------------
static FLDMMDL * PlayerGetFldMMdl( SCRCMD_WORK *work )
{
	FLDMMDLSYS *fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );
	FLDMMDL *fmmdl = FLDMMDLSYS_SearchOBJID( fmmdlsys, FLDMMDL_ID_PLAYER );
	return( fmmdl );
}

//--------------------------------------------------------------
/**
 * 全OBJ動作停止
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	0
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjPauseAll( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc;
	FLDMMDL **fmmdl;
	
	sc = SCRCMD_WORK_GetScriptWork( work );
	fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
	
	if( (*fmmdl) == NULL ){
		FLDMMDLSYS *fmmdlsys;
		fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );
		FLDMMDLSYS_PauseMoveProc( fmmdlsys );
		
		#ifndef SCRCMD_PL_NULL
		//08.06.18
		//話しかけの対象がいないBGやPOSの時
		//連れ歩きOBJの移動動作中かのチェックをしていない
		//
		//ふれあい広場などで、連れ歩きOBJに対して、
		//スクリプトでアニメを発行すると、
		//アニメが行われず終了待ちにいかないでループしてしまう
    
		{
			FIELD_OBJ_PTR player_pair =
				FieldOBJSys_MoveCodeSearch( fsys->fldobjsys, MV_PAIR );
			//ペアが存在している
			if (player_pair) {
				//連れ歩きフラグが立っていて、移動動作中なら
				if( SysFlag_PairCheck(
					SaveData_GetEventWork(fsys->savedata)) == 1
					&& FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
					
					//ペアの動作ポーズ解除
					FieldOBJ_MovePauseClear( player_pair );
					
					//移動動作の終了待ちをセット
					VM_SetWait( core, EvWaitPairObj );
					return 1;
				}
			}
		}
		#endif
	}else{
		EvCmdTalkObjPauseAll( core, wk );
	}
	
	return 1;
}

//--------------------------------------------------------------
/**
 * 会話イベント用全OBJ動作停止
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkObjPauseAll( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	FLDMMDLSYS *fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );
	FLDMMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
	FLDMMDL *player = PlayerGetFldMMdl( work );
	FLDMMDL *player_pair = FLDMMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
#ifndef SCRCMD_PL_NULL
	FLDMMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
#else
	FLDMMDL *other_pair = NULL;
#endif

	InitStepWatchBit();
	FLDMMDLSYS_PauseMoveProc( fmmdlsys );
	
	if( FLDMMDL_CheckEndAcmd(player) == FALSE ){
		SetStepWatchBit(PLAYER_BIT);
		FLDMMDL_OffStatusBitMoveProcPause( player );
	}
	
	if( FLDMMDL_CheckStatusBitMove(*fmmdl) == TRUE ){
		SetStepWatchBit(OTHER_BIT);
		FLDMMDL_OffStatusBitMoveProcPause( *fmmdl );
	}
	
	if( player_pair ){
		#ifndef SCRCMD_PL_NULL
		if( SysFlag_PairCheck(SaveData_GetEventWork(fsys->savedata)) == 1
				&& FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
			SetStepWatchBit(PLAYER_PAIR_BIT);
			FieldOBJ_MovePauseClear( player_pair );
		}
		#else
		GF_ASSERT( 0 );
		#endif
	}
	
	if( other_pair ){
		if( FLDMMDL_CheckStatusBitMove(other_pair) == TRUE ){
			SetStepWatchBit(OTHER_PAIR_BIT);
			FLDMMDL_OffStatusBitMoveProcPause( other_pair );
		}
	}
	
	VMCMD_SetWait( core, EvWaitTalkObj );
	return 1;
}

//--------------------------------------------------------------
/**
 * @brief	OBJ動作終了待ち
 */
//--------------------------------------------------------------
static BOOL EvWaitTalkObj( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	FLDMMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
	FLDMMDL *player = PlayerGetFldMMdl( work );
	FLDMMDLSYS *fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );

	//自機動作停止チェック
	if( CheckStepWatchBit(PLAYER_BIT) &&
		FLDMMDL_CheckEndAcmd(player) == TRUE ){
		FLDMMDL_OnStatusBitMoveProcPause( player );
		ResetStepWatchBit(PLAYER_BIT);
	}
	
	//話しかけ対象動作停止チェック
	if( CheckStepWatchBit(OTHER_BIT) &&
		FLDMMDL_CheckStatusBitMove(*fmmdl) == FALSE ){
		FLDMMDL_OnStatusBitMoveProcPause( *fmmdl );
		ResetStepWatchBit(OTHER_BIT);
	}
	
	//自機の連れ歩き動作停止チェック
	if( CheckStepWatchBit(PLAYER_PAIR_BIT) ){
		FLDMMDL *player_pair = FLDMMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
		
		if( FLDMMDL_CheckStatusBitMove(player_pair) == FALSE ){
			FLDMMDL_OnStatusBitMoveProcPause( player_pair );
			ResetStepWatchBit(PLAYER_PAIR_BIT);
		}
	}
	
	//話しかけ対象の連れ歩き動作停止チェック
	if( CheckStepWatchBit(OTHER_PAIR_BIT) ){
		#ifndef SCRCMD_PL_NULL
		FLDMMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
		if (FieldOBJ_StatusBitCheck_Move(other_pair) == 0) {
			FieldOBJ_MovePause(other_pair);
			ResetStepWatchBit(OTHER_PAIR_BIT);
		}
		#else
		GF_ASSERT( 0 );
		#endif
	}
	
	if( step_watch_bit == 0 ){
		return 1;
	}
	
	return 0;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作再開
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjPauseClearAll( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	FLDMMDLSYS *fmmdlsys = SCRCMD_WORK_GetFldMMdlSys( work );
	FLDMMDLSYS_ClearPauseMoveProc( fmmdlsys );
	return 1;
}

//--------------------------------------------------------------
/**
 * 話しかけたOBJ自機方向への振り向き
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjTurn( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	FLDMMDL *jiki,**fmmdl;
	u16 dir;
	
	jiki = PlayerGetFldMMdl( work );
	dir = FLDMMDL_GetDirDisp( jiki );
	
	dir = FLDMMDL_TOOL_FlipDir( dir );
	fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
	
	if( (*fmmdl) == NULL ){
		return 0;
	}
	
	FLDMMDL_SetDirDisp( *fmmdl, dir );
	return 0;
}

//======================================================================
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/**
 * 「はい・いいえ」処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval 1
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdYesNoWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  FLDMENUFUNC **mw	= SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
	u16 wk_id			= VMGetU16( core );
  
  *mw = FLDMENUFUNC_AddYesNoMenu( fparam->msgBG, 0 );
  core->vm_register[0] = wk_id;
   
  VMCMD_SetWait( core, EvYesNoWinSelect );
  return 1;
}

static BOOL EvYesNoWinSelect( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FLDMENUFUNC **mw = SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, core->vm_register[0] );
  FLDMENUFUNC_YESNO ret = FLDMENUFUNC_ProcYesNoMenu( *mw );
  
  if( ret == FLDMENUFUNC_YESNO_NULL ){
    return 0;
  }
  
  if( ret == FLDMENUFUNC_YESNO_YES ){
    *ret_wk = 0;
  }else{
    *ret_wk = 1;
  }
  
  FLDMENUFUNC_DeleteMenu( *mw );
  return 1;
}

