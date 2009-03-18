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
#include "scrcmd.h"
#include "scrcmd_work.h"

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

static VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk );
static VMCMD_RESULT EvCmdTalkWinClose( VMHANDLE *core, void *wk );

//static VMCMD_RESULT EvCmdFldTalkWindow( VMHANDLE *core, void *wk );

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
 * @brief	インライン関数：ワークを取得する
 * @param	core	仮想マシン制御ワークへのポインタ
 * @return	u16 *	ワークへのポインタ
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
 * @brief	インライン関数：ワークから値を取得する
 * @param	core	仮想マシン制御ワークへのポインタ
 * @return	u16		値
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
//	基本システム命令
//======================================================================
//--------------------------------------------------------------
/**
 * ＮＯＰ命令（なにもしない）
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdNop( VMHANDLE *core, void *wk )
{
	return 0;
}

//--------------------------------------------------------------
/**
 * 何もしない（デバッガで引っ掛けるための命令）
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdDummy( VMHANDLE *core, void *wk )
{
	return 0;
}

//--------------------------------------------------------------
/**
 * スクリプトの終了
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"1"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *  
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	r1		値１
 * @param	r2		値２
 *
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"1"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"1"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 *
 * @li	EVCMD_JUMP
 *
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 *
 * @li	EVCMD_CALL
 *
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
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
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	常に1
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
//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　メッセージ表示(MSG_ALLPUT版)
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkMsgAllPut( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	u8 msg_id = VMGetU8(core);
	FLDMSGWIN *msgWin;
	
	msgWin = SCRCMD_WORK_GetFldMsgWin( work );
	FLDMSGWIN_Print( msgWin, 0, 0, msg_id );
	return 0;
}

//======================================================================
//	フィールド　会話ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 会話ウィンドウ表示
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc;
	SCRIPT_FLDPARAM *fparam;
	GFL_MSGDATA *msgData;
	FLDMSGWIN *msgWin;
	u8 *win_open_flag;
	
	sc = SCRCMD_WORK_GetScriptWork( work );
	fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
	msgData = SCRCMD_WORK_GetMsgData( work );
	msgWin = FLDMSGWIN_AddTalkWin( fparam->msgBG, msgData );
	SCRCMD_WORK_SetFldMsgWin( work, msgWin );

	win_open_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_OPEN_FLAG );
	*win_open_flag = TRUE;	//ON;
	return 0;
}

//--------------------------------------------------------------
/**
 * 会話ウィンドウを閉じる
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkWinClose( VMHANDLE *core, void *wk )
{
	SCRCMD_WORK *work = wk;
	SCRIPT_WORK *sc;
	FLDMSGWIN *msgWin;
	u8 *win_open_flag;
	
	msgWin = SCRCMD_WORK_GetFldMsgWin( work );
	FLDMSGWIN_Delete( msgWin );
	
	sc = SCRCMD_WORK_GetScriptWork( work );
	win_open_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_OPEN_FLAG );
	*win_open_flag = FALSE;	//OFF;
	return 0;
}

//======================================================================
//	動作モデル	
//======================================================================

