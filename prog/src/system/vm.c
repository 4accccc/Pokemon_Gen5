//============================================================================================
/**
 * @file	vm.c
 * @brief	仮想インタープリタマシン　メイン部分(スクリプトコマンド実行に使用される)
 * @author	tamada GAME FREAK inc.
 * @date	01.11.07
 *
 * 08.11.12	tamada	DPから持ってきて汎用性・拡張性を持たせるよう改造開始
 */
//============================================================================================

#include <gflib.h>
#include "system/vm.h"
#include "system/vm_cmd.h"


//============================================================================================
//	定義
//============================================================================================
#define	VM_ERROR(...)	GF_ASSERT_MSG(0, __VA_ARGS__)

// 仮想マシンの動作状態定義
typedef enum{
	VMSTAT_HALT,		///< 停止(動作終了）
	VMSTAT_RUN,			///< 動作中
	VMSTAT_WAIT,		///< 待ち状態（チェックルーチン呼び出し）
}VMSTATUS;

//レジスタ比較の結果定義
enum {
	MINUS_RESULT = 0,	///<比較結果がマイナス
	EQUAL_RESULT,		///<比較結果がイコール
	PLUS_RESULT			///<比較結果がプラス
};

//============================================================================================
//
//	グローバル変数
//
//============================================================================================
//条件分岐用テーブル
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
//const u32 ScriptBreakPoint = 0;


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	仮想マシンワークの初期化
 */
//------------------------------------------------------------------
static void clear(VMHANDLE * core)
{
	core->adrs = NULL;
	core->cmp_flag = 0;
	core->status = VMSTAT_HALT;
	core->routine = NULL;
	//スタッククリア
	core->stackcount = 0;
	GFL_STD_MemFill(core->stack, 0, sizeof(VM_VALUE) * core->init_value.stack_size);
	//レジスタクリア
	GFL_STD_MemFill(core->vm_register, 0, sizeof(VM_VALUE) * core->init_value.reg_size);

  core->check_func = NULL;
  core->check_work = NULL;
}

//============================================================================================
//
//
// 仮想マシン制御用関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
VMHANDLE * VM_Create(HEAPID heapID, const VM_INITIALIZER * init)
{
	VMHANDLE * core;
	u32 size;
	u8 * ptr;

	size = sizeof(VMHANDLE)
		+ sizeof(u32) * init->reg_size
		+ sizeof(u32) * init->stack_size;
	core = GFL_HEAP_AllocMemory(heapID, size);
	GFL_STD_MemFill(core, 0, size);
	core->init_value = * init;
	ptr = (u8*)core;
	core->vm_register = (u32 *)(ptr + sizeof(VMHANDLE));
	core->stack = (u32*)(ptr + sizeof(VMHANDLE) + sizeof(u32) * init->reg_size);

	return core;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void VM_Delete(VMHANDLE * core)
{
	GFL_HEAP_FreeMemory(core);
}

//------------------------------------------------------------------
/**
 * 仮想マシン初期化
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	cmd_tbl		命令テーブル開始アドレス
 * @param	cmd_max		命令テーブルの大きさ
 *
 * @return	none
 */
//------------------------------------------------------------------
void VM_Init(VMHANDLE * core, void * context_work)
{
	clear(core);
	core->context = context_work;
}

//------------------------------------------------------------------
/**
 * 仮想マシンにコードを設定
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	start		実行コードの開始アドレス
 *
 * @return	常にTRUE
 */
//------------------------------------------------------------------
void VM_Start( VMHANDLE * core, const VM_CODE * start )
{
	GF_ASSERT(core->status == VMSTAT_HALT);
	clear(core);
	core->adrs = start;
	core->status = VMSTAT_RUN;
}

//------------------------------------------------------------------
/**
 * 仮想マシン実行終了
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	none
 */
//------------------------------------------------------------------
void VM_End( VMHANDLE * core )
{
	core->status = VMSTAT_HALT;
	core->adrs = NULL;
}

//------------------------------------------------------------------
/**
 * 仮想マシン制御メイン
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @retval	"TRUE = 実行中"
 * @retval	"FALSE = 停止中・実行終了"
 */
//------------------------------------------------------------------
BOOL VM_Control( VMHANDLE * core )
{
	u16 code;

	switch( (VMSTATUS)core->status ){
	case VMSTAT_HALT:
		return FALSE;

	case VMSTAT_WAIT:
		if( core->routine ){
			if( core->routine(core, core->context) == TRUE ){
				core->routine = NULL;
				core->status = VMSTAT_RUN;
			}
			return TRUE;
		} else {
			VM_ERROR("vm wait routine error\n");
			core->status = VMSTAT_RUN;
		}
		/* FALL THROUGH */

	case VMSTAT_RUN:
		while( TRUE ){
			if( core->adrs == NULL ){
				VM_ERROR("vm adrs null error\n");
				core->status = VMSTAT_HALT;
				return FALSE;
			}
/*	デバッグ処理
			if( core->adrs == (VM_CODE *)ScriptBreakPoint ){
				while( TRUE ){
					Halt();
				}
			}
*/

			code = VMGetU16( core );

			if( code >= core->init_value.command_max ) {
				VM_ERROR("vm command error %04x:%08x\n",code, core->adrs - 2);
				core->status = VMSTAT_HALT;
				return FALSE;
			}
      if( core->check_func ) {
        if ( core->check_func( core, core->context, core->check_work, code ) == FALSE )
        {
          VM_ERROR("vm command error %04x:%08x\n",code, core->adrs - 2);
          core->status = VMSTAT_HALT;
          return FALSE;
        }
      }
			if( core->init_value.command_table[code]( core, core->context )
					== VMCMD_RESULT_SUSPEND ){
				break;
			}
		}
		break;
	default:
		VM_ERROR("vm error status %02x\n", core->status);
		core->status = VMSTAT_HALT;
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * 仮想マシンコンテキスト取得
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	void *		仮想マシンが保持しているコンテキストポインタを返す
 *
 * コンテキストはVM_Initで与えたもの
 *
 */
//------------------------------------------------------------------
void * VM_GetContext( VMHANDLE * core)
{
	return core->context;
}

//------------------------------------------------------------------
/**
 * @brief   仮想マシンコマンドチェック関数登録
 * @param core      仮想マシン制御構造体へのポインタ
 * @param callback  チェック関数
 * @param work      チェック関数が利用するワークへのポインタ
 */
//------------------------------------------------------------------
void VM_SetCheckFunc( VMHANDLE * core, VM_CHECK_FUNC callback, void * work )
{
  core->check_func = callback;
  core->check_work = work;
}

//============================================================================================
// 仮想マシン制御用サブルーチン
//============================================================================================

//------------------------------------------------------------------
/**
 * adrsのアドレスから16bit(2byte)データ取得
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	取得データ
 */
//------------------------------------------------------------------
u16 VMGetU16( VMHANDLE * core )
{
	u16	val;

	val = (u16)VMGetU8( core );
	val += (u16)VMGetU8( core ) << 8;

	return val;
}

//------------------------------------------------------------------
/**
 * adrsのアドレスから32bit(4byte)データ取得
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	取得データ
 */
//------------------------------------------------------------------
u32 VMGetU32( VMHANDLE * core )
{
	u32	val;
	u8	a,b,c,d;

	a = VMGetU8( core );
	b = VMGetU8( core );
	c = VMGetU8( core );
	d = VMGetU8( core );

	val = 0;
	val += (u32)d;
	val <<= 8;
	val += (u32)c;
	val <<= 8;
	val += (u32)b;
	val <<= 8;
	val += (u32)a;

	return val;
}

//--------------------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------------------
void VMCompare(VMHANDLE * core, VM_VALUE r1, VM_VALUE r2 )
{
	if( r1 < r2 ){
		core->cmp_flag = MINUS_RESULT;
	}else if( r1 == r2 ){
		core->cmp_flag = EQUAL_RESULT;
	}else {
		core->cmp_flag = PLUS_RESULT;
	}
}

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
/**
 * 仮想マシンスタックプッシュ
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	val
 *
 * @retval	"0 = 正常"
 * @retval	"1 = エラー"
 */
//------------------------------------------------------------------
void VMCMD_Push( VMHANDLE * core, VM_VALUE value)
{
	if( core->stackcount + 1 >= core->init_value.stack_size ){
		VM_ERROR("vm stack overflow(%d)\n",core->stackcount);
		return;
	}
	core->stack[core->stackcount] = value;
	core->stackcount ++;
}

//------------------------------------------------------------------
/**
 * 仮想マシンスタックポップ
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return
 */
//------------------------------------------------------------------
VM_VALUE VMCMD_Pop( VMHANDLE * core )
{
	if( core->stackcount == 0 ){
		VM_ERROR("vm stack pop error\n");
		return NULL;
	}
	core->stackcount --;

	return core->stack[core->stackcount];
}

//------------------------------------------------------------------
/**
 * 仮想マシンジャンプ命令
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	adrs		ジャンプ先アドレス
 *
 * @return	none
 */
//------------------------------------------------------------------
void VMCMD_Jump( VMHANDLE * core, const VM_CODE * adrs )
{
	core->adrs = adrs;
}

//------------------------------------------------------------------
/**
 * 仮想マシンコール命令
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	adrs		呼び出すアドレス
 *
 * @return	none
 */
//------------------------------------------------------------------
void VMCMD_Call( VMHANDLE * core, const VM_CODE * adrs )
{
	VMCMD_Push( core, (VM_VALUE)core->adrs );
	core->adrs = adrs;
}

//------------------------------------------------------------------
/**
 * 仮想マシンリターン命令
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	none
 */
//------------------------------------------------------------------
void VMCMD_Return( VMHANDLE * core )
{
	core->adrs = (const VM_CODE *)VMCMD_Pop( core );
}

//------------------------------------------------------------------
/**
 * 仮想マシンをウェイト状態に設定
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param	func		ウェイト関数
 *
 * @return	none
 *
 * @li	TRUEが返ってくるまでウェイト関数を毎回呼びだす
 */
//------------------------------------------------------------------
void VMCMD_SetWait( VMHANDLE * core, VM_WAIT_FUNC func )
{
	core->status = VMSTAT_WAIT;
	core->routine = func;
}

//============================================================================================
//============================================================================================


