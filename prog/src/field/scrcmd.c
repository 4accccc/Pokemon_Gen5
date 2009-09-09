//======================================================================
/**
 * @file  scrcmd.c
 * @bfief  スクリプトコマンド用関数
 * @author  Satoshi Nohara
 * @date  05.08.04
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
#include "print/wordset.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field_sound.h"
#include "field_player.h"

#include "eventdata_local.h"
#include "field/eventdata_sxy.h"

#include "tr_tool/tr_tool.h"

#include "field/zonedata.h"

#include "scrcmd_trainer.h"
#include "scrcmd_sound.h"
#include "scrcmd_musical.h"
#include "scrcmd_word.h"
#include "scrcmd_item.h"

#include "../../../resource/fldmapdata/script/usescript.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// レジスタ比較の結果定義
//--------------------------------------------------------------
enum {
  MINUS_RESULT = 0,  //比較結果がマイナス
  EQUAL_RESULT,    //比較結果がイコール
  PLUS_RESULT      //比較結果がプラス
};

//======================================================================
//  struct
//======================================================================
typedef u16 (* pMultiFunc)();

//======================================================================
//  proto
//======================================================================
//parts
static VMCMD_RESULT evcmd_ObjPauseAll( VMHANDLE *core, SCRCMD_WORK *work );
static void evcmd_PauseClearAll( SCRCMD_WORK *work );
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work );

//data
const u32 ScriptCmdMax;
const VMCMD_FUNC ScriptCmdTbl[];

static const u8 ConditionTable[6][3];

//======================================================================
///  動作モデル監視に使用するインライン関数定義
//======================================================================
#define  PLAYER_BIT      (1<<0)
#define  PLAYER_PAIR_BIT    (1<<1)
#define  OTHER_BIT      (1<<2)
#define  OTHER_PAIR_BIT    (1<<3)

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
//  基本システム命令
//======================================================================
//--------------------------------------------------------------
/**
 * ＮＯＰ命令（なにもしない）
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdNop( VMHANDLE *core, void *wk )
{
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 何もしない（デバッガで引っ掛けるための命令）
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdDummy( VMHANDLE *core, void *wk )
{
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプトの終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdEnd( VMHANDLE *core, void *wk )
{
  VM_End( core );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ウェイト処理 ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitTime(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u32 * counter = &core->vm_register[0];

  (*counter)--;
  if( *counter == 0 ){ 
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ウェイト処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTimeWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 num        = VMGetU16( core );
  
  //仮想マシンの汎用レジスタにウェイトを格納
  core->vm_register[0] = num;
  VMCMD_SetWait( core, EvWaitTime );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * スクリプトコール
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 * @li  EVCMD_CALL
 *  表記：  EVCMD_CALL  CallOffset(s16)
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGlobalCall( VMHANDLE *core, void *wk )
{
  s32  pos = (s32)VMGetU32(core);
  VMCMD_Call( core, (VM_CODE *)(core->adrs+pos) );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプトリターン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdRet( VMHANDLE *core, void *wk )
{
  VMCMD_Return( core );
  return VMCMD_RESULT_CONTINUE;
}

#if 0
static VMCMD_RESULT EvCmdDebugWatch(VMHANDLE * core, void *wk )
{
  u16 value = SCRCMD_GetVMWorkValue(core);
  KAGAYA_Printf("SCR WORK: %d\n", value);
  return VMCMD_RESULT_CONTINUE;
}
#endif

//======================================================================
//  データロード・ストア関連
//======================================================================
//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタに1byteの値を格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegValue( VMHANDLE *core, void *wk )
{
  u8  r = VMGetU8( core );
  core->vm_register[r] = VMGetU8( core );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタに4byteの値を格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegWData( VMHANDLE *core, void *wk )
{
  u8  r;
  u32  wdata;
  
  r = VMGetU8( core );
  wdata = VMGetU32( core );
  core->vm_register[r] = wdata;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタにアドレスを格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegAdrs( VMHANDLE *core, void *wk )
{
  u8  r;
  VM_CODE * adrs;

  r = VMGetU8( core );
  adrs = (VM_CODE *)VMGetU32( core );
  core->vm_register[r] = *adrs;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身に値を代入
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsValue( VMHANDLE *core, void *wk )
{
  VM_CODE * adrs;
  u8  r;

  adrs = (VM_CODE *)VMGetU32( core );
  r = VMGetU8( core );
  *adrs = r;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身に仮想マシンの汎用レジスタの値を代入
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsReg( VMHANDLE *core, void *wk )
{
  VM_CODE * adrs;
  u8  r;
  
  adrs = (VM_CODE *)VMGetU32( core );
  r = VMGetU8( core);
  *adrs = core->vm_register[r];
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタの値を汎用レジスタにコピー
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadRegReg( VMHANDLE *core, void *wk )
{
  u8  r1, r2;

  r1 = VMGetU8( core );
  r2 = VMGetU8( core );
  core->vm_register[r1] = core->vm_register[r2];
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身にアドレスの中身を代入
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadAdrsAdrs( VMHANDLE *core, void *wk )
{
  VM_CODE * adr1;
  VM_CODE * adr2;

  adr1 = (VM_CODE *)VMGetU32( core );
  adr2 = (VM_CODE *)VMGetU32( core );
  *adr1 = * adr2;
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  比較命令
//======================================================================
//--------------------------------------------------------------
/**
 * ２つの値を比較
 * @param  r1    値１
 * @param  r2    値２
 * @retval  "r1 < r2 : MISUS_RESULT"
 * @retval  "r1 = r2 : EQUAL_RESULT"
 * @retval  "r1 > r2 : PLUS_RESULT"
 */
//--------------------------------------------------------------
static u8 EvCmdCmpMain( u32 r1, u32 r2 )
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
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegReg( VMHANDLE *core, void *wk )
{
  u8  r1, r2;

  r1 = core->vm_register[ VMGetU8(core) ];
  r2 = core->vm_register[ VMGetU8(core) ];
  core->cmp_flag = EvCmdCmpMain( r1, r2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタと値を比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegValue( VMHANDLE *core, void *wk )
{
  u8  r1, r2;

  r1 = core->vm_register[ VMGetU8(core) ];
  r2 = VMGetU8(core);
  core->cmp_flag = EvCmdCmpMain( r1, r2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 仮想マシンの汎用レジスタとアドレスの中身を比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpRegAdrs( VMHANDLE *core, void *wk )
{
  u8  r1, r2;

  r1 = core->vm_register[ VMGetU8(core) ];
  r2 = *(VM_CODE *)VMGetU32(core);
  core->cmp_flag = EvCmdCmpMain( r1, r2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身と仮想マシンの汎用レジスタを比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsReg( VMHANDLE *core, void *wk )
{
  u8  r1, r2;

  r1 = *(VM_CODE *)VMGetU32(core);
  r2 = core->vm_register[ VMGetU8(core) ];
  core->cmp_flag = EvCmdCmpMain(r1,r2);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身と値を比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsValue(VMHANDLE * core, void *wk)
{
  u8 r1,r2;

  r1 = *(VM_CODE *)VMGetU32(core);
  r2 = VMGetU8(core);
  core->cmp_flag = EvCmdCmpMain(r1,r2);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アドレスの中身とアドレスの中身を比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpAdrsAdrs(VMHANDLE * core, void *wk)
{
  u8 r1,r2;

  r1 = *(VM_CODE *)VMGetU32(core);
  r2 = *(VM_CODE *)VMGetU32(core);
  core->cmp_flag = EvCmdCmpMain(r1,r2);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークと値を比較
 *
 * @param  core    仮想マシン制御構造体へのポインタ
 *
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpWkValue( VMHANDLE *core, void *wk )
{
  u16 *p;
  u16  r1, r2;
  
  p = SCRCMD_GetVMWork( core, wk );
  r1 = *p;
  r2 = VMGetU16( core );
  core->cmp_flag = EvCmdCmpMain( r1, r2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークとワークを比較
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpWkWk( VMHANDLE *core, void *wk )
{
  u16 * wk1;
  u16 * wk2;

  wk1 = SCRCMD_GetVMWork( core, wk );
  wk2 = SCRCMD_GetVMWork( core, wk );
  core->cmp_flag = EvCmdCmpMain( *wk1, *wk2 );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPushValue( VMHANDLE *core, void *wk )
{
  u32 value;
  value = VMGetU16(core);
  VMCMD_Push( core, value );
  TAMADA_Printf("SCRCMD:PUSH_VALUE:%d\n", value);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPushWork( VMHANDLE *core, void *wk )
{
  u32 work_value;
  work_value = SCRCMD_GetVMWorkValue( core, wk );
  VMCMD_Push( core, work_value );
  TAMADA_Printf("SCRCMD:PUSH_WORK:%d\n", work_value);
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPopWork( VMHANDLE *core, void *wk )
{
  u32 value;
  u16 * work;
  value = VMCMD_Pop( core );
  work = SCRCMD_GetVMWork( core, wk );
  *work = value;
  TAMADA_Printf("SCRCMD:POP_WORK:%d\n", value);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPop( VMHANDLE *core, void *wk )
{
  VMCMD_Pop( core );
  TAMADA_Printf("SCRCMD:POP\n");

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCalcAdd( VMHANDLE * core, void *wk )
{
  u32 val1,val2;
  val2 = VMCMD_Pop( core );
  val1 = VMCMD_Pop( core );
  VMCMD_Push( core, val1 + val2 );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCalcSub( VMHANDLE * core, void *wk )
{
  u32 val1,val2;
  val2 = VMCMD_Pop( core );
  val1 = VMCMD_Pop( core );
  VMCMD_Push( core, val1 - val2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCalcMul( VMHANDLE * core, void *wk )
{
  u32 val1,val2;
  val2 = VMCMD_Pop( core );
  val1 = VMCMD_Pop( core );
  VMCMD_Push( core, val1 * val2 );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCalcDiv( VMHANDLE * core, void *wk )
{
  u32 val1,val2;
  val2 = VMCMD_Pop( core );
  val1 = VMCMD_Pop( core );
  VMCMD_Push( core, val1 / val2 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPushFlagValue( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  //u16  flag = VMGetU16( core );
  //フラグIDはワークIDより明らかに小さいので、両対応
  u16 flag = SCRCMD_GetVMWorkValue( core, work );
  BOOL value  = EVENTWORK_CheckEventFlag( evwork, flag );
  VMCMD_Push( core, value );
  TAMADA_Printf("SCRCMD:Push Flag Value(id=%d, value=%d)\n", flag, value );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 判定処理
 *
 * スタック上の二つの値をPOPして、指定条件での比較を行う
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCmpStack( VMHANDLE * core, void *wk )
{
  u16  cond;
  u32 val1, val2, result;
  val2 = VMCMD_Pop( core );
  val1 = VMCMD_Pop( core );
  //core->cmp_flag = EvCmdCmpMain(val1 , val2);
  cond   = VMGetU16(core);

  switch (cond)
  {
  case CMPID_LT:
    result = (val1 < val2);
    break;
  case CMPID_EQ:
    result = (val1 == val2);
    break;
  case CMPID_GT:
    result = (val1 > val2);
    break;
  case CMPID_LT_EQ:
    result = (val1 <= val2);
    break;
  case CMPID_GT_EQ:
    result = (val1 >= val2);
    break;
  case CMPID_NE:   
    result = (val1 != val2);
    break;
  case CMPID_OR:
    result = ( (val1 == TRUE) || (val2 == TRUE) );
    break;
  case CMPID_AND:
    result = ((val1 == TRUE) && (val2 == TRUE));
    break;
  default:
    result = 0;
  }
  VMCMD_Push( core, result );
  TAMADA_Printf("SCRCMD:CMP_STACK:v1(%08x),v2(%08x),cond(%d),result(%d)\n",
      val1, val2, cond, result);
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  仮想マシン関連
//======================================================================
//--------------------------------------------------------------
/**
 * 仮想マシン追加(切り替えはせず、並列で動作します！)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
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
  *vm = SCRIPT_AddVMachine( gsys, sc,
      SCRCMD_WORK_GetHeapID(work), SCRCMD_WORK_GetHeapID(work), id );
  (*vm_machine_count)++;
  
  //イベントと切り離したTCB動作にするかも？
  //*vm=VMMachineAddTCB(fsys,id,&ScriptCmdTbl[0],&ScriptCmdTbl[EVCMD_MAX]);
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ローカルスクリプトをウェイト状態にして共通スクリプトを動作 ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvChangeCommonScrWait(VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u8* common_scr_flag = SCRIPT_GetMemberWork(sc,ID_EVSCR_COMMON_SCR_FLAG);

  if( *common_scr_flag == 0 ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ローカルスクリプトをウェイト状態にして、共通スクリプトを動作させます
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdChangeCommonScr( VMHANDLE *core, void *wk )
{
  u16 scr_id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u8 *common_scr_flag = SCRIPT_GetMemberWork(sc, ID_EVSCR_COMMON_SCR_FLAG);
  u8 *vm_machine_count = SCRIPT_GetMemberWork( sc, ID_EVSCR_VMHANDLE_COUNT );
  VMHANDLE **vm = SCRIPT_GetMemberWork( sc, ID_EVSCR_VM_SUB1 );

  scr_id = VMGetU16(core);

  //共通スクリプト以外にも切り替え可能になっている！

  //共通スクリプト切り替えフラグON
  *common_scr_flag = 1;
  
  //仮想マシン追加
  //*vm = VMMachineAdd(fsys,scr_id,&ScriptCmdTbl[0],&ScriptCmdTbl[EVCMD_MAX]);
  *vm = SCRIPT_AddVMachine( gsys, sc,
      SCRCMD_WORK_GetHeapID(work), SCRCMD_WORK_GetHeapID(work), scr_id );
  (*vm_machine_count)++;

  VMCMD_SetWait( core, EvChangeCommonScrWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 共通スクリプトを終了して、ローカルスクリプトを再開させます
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdChangeLocalScr( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u8* common_scr_flag = SCRIPT_GetMemberWork(sc, ID_EVSCR_COMMON_SCR_FLAG);

  //共通スクリプト切り替えフラグOFF
  *common_scr_flag = 0;

  VM_End( core );
  return VMCMD_RESULT_SUSPEND;
  //return VMCMD_RESULT_CONTINUE;  //注意！　この後に"END"に行くようにする
}

//======================================================================
//  分岐命令
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプトジャンプ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 * @li  EVCMD_JUMP
 *  表記：  EVCMD_JUMP  JumpOffset(s16)
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGlobalJump( VMHANDLE *core, void *wk )
{
  s32  pos;
  pos = (s32)VMGetU32(core);
  VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );  //JUMP
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプト条件ジャンプ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdIfJump( VMHANDLE *core, void *wk )
{
  u8  r;
  s32  pos;

  r   = VMGetU8(core);
  pos = (s32)VMGetU32(core);

  if ( r == CMPID_GET)
  {
    if (VMCMD_Pop( core ) != TRUE)
    {
      VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );
    }
  }
  else if( ConditionTable[r][core->cmp_flag] == TRUE ){
    VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプト条件コール
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdIfCall( VMHANDLE *core, void *wk )  
{
  u8  r;
  s32  pos;

  r   = VMGetU8(core);
  pos = (s32)VMGetU32(core);

  if( ConditionTable[r][core->cmp_flag] == TRUE ){
    VMCMD_Call( core, (VM_CODE *)(core->adrs+pos) );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 話し掛け対象OBJID比較ジャンプ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjIDJump( VMHANDLE *core, void *wk )
{
  u8 id;
  s32  pos;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL **fldobj;
  
  //話し掛け対象OBJ
  fldobj = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  //比較する値
  id = VMGetU8(core);
  
  //飛び先
  pos = (s32)VMGetU32(core);
  
  //話し掛け対象OBJと、比較する値が同じか
  if( MMDL_GetOBJID(*fldobj) == id ){
    VMCMD_Jump( core, (VM_CODE *)(core->adrs+pos) );  //JUMP
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 話し掛け対象BG比較ジャンプ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBgIDJump( VMHANDLE *core, void *wk )
{
#ifndef SCRCMD_PL_NULL
  u8 id;
  s32  pos;
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
    VMJump( core, (VM_CODE *)(core->adrs+pos) );  //JUMP
  }
  
  return VMCMD_RESULT_CONTINUE;
#else
  GF_ASSERT( 0 && "WB未対応\n" );
  return VMCMD_RESULT_CONTINUE;
#endif
}

//--------------------------------------------------------------
/**
 * イベント起動時の主人公の向き比較ジャンプ
 * (現在の向きではないので注意！)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPlayerDirJump( VMHANDLE *core, void *wk )
{
#ifndef SCRCMD_PL_NULL
  u8 dir;
  s32  pos;
  int* player_dir;
  FIELDSYS_WORK* fsys = core->fsys;

  player_dir = GetEvScriptWorkMemberAdrs( fsys, ID_EVSCR_PLAYER_DIR );

  //比較する値
  dir = VMGetU8(core);
  
  //飛び先
  pos = (s32)VMGetU32(core);

  if( *player_dir == dir ){
    VMJump( core, (VM_CODE *)(core->adrs+pos) );  //JUMP
  }
  return VMCMD_RESULT_CONTINUE;
#else
  GF_ASSERT( 0 && "WB未対応\n" );
  return VMCMD_RESULT_CONTINUE;
#endif
}

//======================================================================
//  イベントワーク関連
//======================================================================
//--------------------------------------------------------------
/*
 * フラグのセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdFlagSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16  flag = VMGetU16( core );
  EVENTWORK_SetEventFlag( evwork, flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フラグのリセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdFlagReset( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16  flag = VMGetU16( core );
  EVENTWORK_ResetEventFlag( evwork, flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フラグのチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdFlagCheck( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16  flag = VMGetU16( core );
  core->cmp_flag = EVENTWORK_CheckEventFlag( evwork, flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークの値をフラグナンバーとしてフラグチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdFlagCheckWk( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16 *flag_work = SCRCMD_GetVMWork( core, work );
  u16 *ret_work  = SCRCMD_GetVMWork( core, work );
  *ret_work = EVENTWORK_CheckEventFlag( evwork, *flag_work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークの値をフラグナンバーとしてフラグセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdFlagSetWk( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16 *flag_work = SCRCMD_GetVMWork( core, work );
  EVENTWORK_SetEventFlag( evwork, *flag_work );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  ワーク操作関連
//======================================================================
//--------------------------------------------------------------
/**
 * ワークに値を足す
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdWkAdd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 *work_val;
  u16 num;
  work_val = SCRCMD_GetVMWork( core, work );
  *work_val += SCRCMD_GetVMWorkValue( core, work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークから値を引く
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdWkSub( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 * work_val;
  work_val = SCRCMD_GetVMWork( core, work );
  *work_val -= SCRCMD_GetVMWorkValue( core, work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークに値を格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadWkValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 *work_val;
  work_val = SCRCMD_GetVMWork( core, work );
  *work_val = VMGetU16( core );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークにワークの値を格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadWkWk( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 * wk1;
  u16 * wk2;
  wk1 = SCRCMD_GetVMWork( core, work );
  wk2 = SCRCMD_GetVMWork( core, work );
  *wk1 = *wk2;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ワークに値かワークの値を格納
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLoadWkWkValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 * work_val;
  work_val = SCRCMD_GetVMWork( core, work );
  *work_val = SCRCMD_GetVMWorkValue( core, work );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
/// フィールドイベント共通処理
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドイベント開始時の共通処理まとめ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCommonProcFieldEventStart( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
  //全動作モデルポーズ
  return( evcmd_ObjPauseAll(core,work) );
}

//--------------------------------------------------------------
/**
 * フィールドイベント終了時の共通処理まとめ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCommonProcFieldEventEnd( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
  //サウンド開放忘れ回避
  {
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
    FIELD_SOUND_ForcePopBGM( fsnd );
  }
  
  //動作モデルポーズ解除
  evcmd_PauseClearAll( work );

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  キー入力関連
//======================================================================
//--------------------------------------------------------------
/**
 * キーウェイト ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitABKey( VMHANDLE * core, void *wk )
{
  int trg = GFL_UI_KEY_GetTrg();
  
  if( trg & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
    return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * キーウェイト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdABKeyWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, EvWaitABKey );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  フィールド　会話ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 会話ウィンドウメッセージ表示　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
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
 * 登録された単語を使って文字列展開　会話ウィンドウメッセージ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkMsg( VMHANDLE *core, void *wk )
{
  FLDMSGWIN_STREAM *msgWin;
  SCRCMD_WORK *work = wk;
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  
  {
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
    STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
    WORDSET_ExpandStr( *wordset, *msgbuf, *tmpbuf );
  }
  
  msgWin = SCRCMD_WORK_GetFldMsgWinStream( work );
  FLDMSGWIN_STREAM_PrintStrBufStart( msgWin, 0, 0, *msgbuf );
  VMCMD_SetWait( core, TalkMsgWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdTalkMsg( VMHANDLE *core, void *wk )
{
  u16 dir;
  VecFx32 pos;
  const VecFx32 *pos_p;
  WORDSET **wordset;
  STRBUF **msgbuf;
  STRBUF **tmpbuf;
  MMDL *fmmdl;
  FLDTALKMSGWIN *tmsg;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u8 msg_id = VMGetU8(core);
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  
  fmmdl = MMDLSYS_SearchOBJID( SCRCMD_WORK_GetMMdlSys(work), 0xff );
  MMDL_GetVectorPos( fmmdl, &pos );
  dir = MMDL_GetDirDisp( fmmdl );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  
  if( dir == DIR_UP ){        //下から
    pos.x += FX32_ONE*8;
    pos.z -= FX32_ONE*8;
  }else if( dir == DIR_DOWN ){ //上から
    pos.x += FX32_ONE*8;
    pos.z -= FX32_ONE*8;
  }else if( dir == DIR_LEFT ){ //右から
    pos.x += -FX32_ONE*8;
    pos.z += FX32_ONE*16;
  }else{                       //左から
    pos.x += FX32_ONE*8;
    pos.z += FX32_ONE*16;
  }
  
  SCRCMD_WORK_SetTalkMsgWinTailPos( work, &pos );
  pos_p = SCRCMD_WORK_GetTalkMsgWinTailPos( work );
  
  {
    wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
    msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
    tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    KAGAYA_Printf( "EvCmdTalkMsg MSG ID = %d\n", msg_id );
    GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
    WORDSET_ExpandStr( *wordset, *msgbuf, *tmpbuf );
  }
  
  {
    FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
    
    if( dir == DIR_UP ){
      idx = FLDTALKMSGWIN_IDX_UPPER;
    }
    
    tmsg = FLDTALKMSGWIN_AddStrBuf( fparam->msgBG, idx, pos_p, *msgbuf );
  }
  
  SCRCMD_WORK_SetFldMsgWinStream( work, (FLDMSGWIN_STREAM*)tmsg );
  PMSND_PlaySystemSE( SEQ_SE_MESSAGE );

  VMCMD_SetWait( core, TalkMsgWait );
  return VMCMD_RESULT_SUSPEND;
}
#endif

//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　会話ウィンドウメッセージ全表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
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
  return VMCMD_RESULT_CONTINUE;
#else
  return( EvCmdTalkMsg(core,wk) );
#endif
}

//--------------------------------------------------------------
/**
 * 会話ウィンドウ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
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
  *win_open_flag = TRUE;  //ON;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 会話ウィンドウを閉じる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
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
  *win_open_flag = FALSE;  //OFF;
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  吹き出しウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ描画
 * @param work  SCRCMD_WORK
 * @param objID 吹き出しを出すOBJID
 * @param arcID 表示するメッセージのアーカイブ指定ID
 * @param msgID 表示するメッセージID
 * @retval BOOL TRUE=表示 FALSE=エラー
 *
 * arcID=0x400の場合、デフォルトのメッセージアーカイブを使用する
 */
//--------------------------------------------------------------
static BOOL balloonWin_Write( SCRCMD_WORK *work, u16 objID, u16 arcID, u16 msgID )
{
  VecFx32 pos,jiki_pos;
  const VecFx32 *p_pos;
  STRBUF **msgbuf;
  FLDTALKMSGWIN *tmsg;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *jiki = MMDLSYS_SearchOBJID( mmdlsys, MMDL_ID_PLAYER );
  MMDL *npc = MMDLSYS_SearchOBJID( mmdlsys, objID );
  
  if( npc == NULL ){
    OS_Printf("スクリプトエラー 吹き出し対象のOBJが存在しません\n");
    return( FALSE );
  }
  
  MMDL_GetVectorPos( npc, &pos );
  MMDL_GetVectorPos( jiki, &jiki_pos );

  SCRCMD_WORK_SetTalkMsgWinTailPos( work, &pos );
  p_pos = SCRCMD_WORK_GetTalkMsgWinTailPos( work );
  
  msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  
  {
    WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
    STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    if (arcID == 0x400)
    {
      GFL_MSG_GetString( msgData, msgID, *tmpbuf );
    } else {
      msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          arcID, SCRCMD_WORK_GetHeapID( work ) );
      GFL_MSG_GetString( msgData, msgID, *tmpbuf );
      GFL_MSG_Delete( msgData );
    }
    WORDSET_ExpandStr( *wordset, *msgbuf, *tmpbuf );
  }
  
  {
    FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
    
    if( pos.z < jiki_pos.z ){
      idx = FLDTALKMSGWIN_IDX_UPPER;
    }
    
    tmsg = FLDTALKMSGWIN_AddStrBuf( fparam->msgBG, idx, p_pos, *msgbuf );
  }
  
  SCRCMD_WORK_SetFldMsgWinStream( work, (FLDMSGWIN_STREAM*)tmsg );
  PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　表示待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL BallonWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDTALKMSGWIN *tmsg;
  tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetFldMsgWinStream( work );
  
  if( FLDTALKMSGWIN_Print(tmsg) == TRUE ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 OBJID指定有り
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBalloonWinWrite( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u8 obj_id = VMGetU8( core );
  
  KAGAYA_Printf( "吹き出しウィンドウ OBJID =%d\n", obj_id );

  if( balloonWin_Write(work,obj_id,arc_id,msg_id) == TRUE ){
    VMCMD_SetWait( core, BallonWinMsgWait );
    return VMCMD_RESULT_SUSPEND;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 話し掛けOBJ対象
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBalloonWinTalkWrite( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL **mmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  
  if( *mmdl == NULL ){
    OS_Printf( "スクリプトエラー 話し掛け対象のOBJが居ません\n" );
    return VMCMD_RESULT_SUSPEND;
  }

  {
    u8 obj_id = MMDL_GetOBJID( *mmdl );
    
    if( balloonWin_Write(work,obj_id,arc_id,msg_id) == TRUE ){
      VMCMD_SetWait( core, BallonWinMsgWait );
      return VMCMD_RESULT_SUSPEND;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウを閉じる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBalloonWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDTALKMSGWIN *tmsg;
  
  tmsg = (FLDTALKMSGWIN*) SCRCMD_WORK_GetFldMsgWinStream( work );
  FLDTALKMSGWIN_Delete( tmsg );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  動作モデル  
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param  fsys  FIELDSYS_WORK型のポインタ
 * @param  obj_id  OBJID
 * @retval  "FIELD_OBJ_PTR"
 */
//--------------------------------------------------------------
static MMDL * FieldObjPtrGetByObjId( SCRCMD_WORK *work, u16 obj_id )
{
  MMDL *dummy;
  MMDL *fmmdl;
  MMDLSYS *fmmdlsys;
  
  fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
  //連れ歩きOBJ判別IDが渡された時
  if( obj_id == SCR_OBJID_MV_PAIR ){
    fmmdl = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  //透明ダミーOBJ判別IDが渡された時
  }else if( obj_id == SCR_OBJID_DUMMY ){
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    dummy = SCRIPT_GetMemberWork( sc, ID_EVSCR_DUMMY_OBJ );
  //対象のフィールドOBJのポインタ取得
  }else{
    fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, obj_id );
  }
  
  return fmmdl;
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk )
{
  u8 *num;
  VM_CODE *p;
  GFL_TCB *anm_tcb;
  MMDL *fmmdl; //対象のフィールドOBJのポインタ
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work); //obj ID
  u32 pos = VMGetU32(core); //list pos
  
  fmmdl = FieldObjPtrGetByObjId( work, obj_id );
  
  //エラーチェック
  if( fmmdl == NULL ){
    OS_Printf( "obj_id = %d\n", obj_id );
    GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
    return VMCMD_RESULT_CONTINUE;        //08.06.12 プラチナで追加
  }
  
  //アニメーションコマンドリストセット
  p = (VM_CODE*)(core->adrs+pos);
  anm_tcb = MMDL_SetAcmdList( fmmdl, (MMDL_ACMD_LIST*)p );
  
  //アニメーションの数を足す
  num = SCRIPT_GetMemberWork( sc, ID_EVSCR_ANMCOUNT );
  (*num)++;
  
  //TCBセット
  SCRCMD_WORK_SetMMdlAnmTCB( work, anm_tcb );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション終了待ち　ウェイと部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
  if( SCRCMD_WORK_CheckMMdlAnmTCB(work) == FALSE ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * アニメーション終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, EvObjAnimeWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdMoveCodeGet( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 *ret_wk  = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = 0;
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  GF_ASSERT( mmdl != NULL && "SCRCMD GET MOVECODE NON OBJ" );
  *ret_wk = MMDL_GetMoveCode( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJの位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  
  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdPlayerPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  
  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAdd( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  u16 gx = VMGetU16( core );
  u16 gz = VMGetU16( core );
  u16 dir = VMGetU16( core );
  u16 id = VMGetU16( core );
  u16 code = VMGetU16( core );
  u16 move = VMGetU16( core );
  int zone_id = FIELDMAP_GetZoneID( fparam->fieldMap );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
  mmdl = MMDLSYS_AddMMdlParam(
      mmdlsys, gx, gz, dir, id, code, move, zone_id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを削除
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjDel( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL && "OBJ DEL 対象のOBJが居ません\n" );
  MMDL_Delete( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加 配置データから
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAddEvent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
	u16 count = EVENTDATA_GetNpcCount( evdata );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
  int zone_id = PLAYERWORK_getZoneID( player );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  
  if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
		const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
		MMDL *mmdl = MMDLSYS_AddMMdlHeaderID(
        mmdlsys, header, zone_id, count, evwork, id );
    GF_ASSERT( mmdl != NULL );
  }else{
    GF_ASSERT( 0 );
  }
  
  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJを削除　OBJ指定イベントフラグをONに
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjDelEvent( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL );
  
  MMDL_DeleteEvent( mmdl, evwork );
  return( VMCMD_RESULT_CONTINUE );
}

//======================================================================
//  動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief  OBJ動作終了待ち　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitTalkObj( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );

  //自機動作停止チェック
  if( CheckStepWatchBit(PLAYER_BIT) &&
    MMDL_CheckEndAcmd(player) == TRUE ){
    MMDL_OnStatusBitMoveProcPause( player );
    ResetStepWatchBit(PLAYER_BIT);
  }
  
  //話しかけ対象動作停止チェック
  if( CheckStepWatchBit(OTHER_BIT) &&
    MMDL_CheckStatusBitMove(*fmmdl) == FALSE ){
    MMDL_OnStatusBitMoveProcPause( *fmmdl );
    ResetStepWatchBit(OTHER_BIT);
  }
  
  //自機の連れ歩き動作停止チェック
  if( CheckStepWatchBit(PLAYER_PAIR_BIT) ){
    MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
    
    if( MMDL_CheckStatusBitMove(player_pair) == FALSE ){
      MMDL_OnStatusBitMoveProcPause( player_pair );
      ResetStepWatchBit(PLAYER_PAIR_BIT);
    }
  }
  
  //話しかけ対象の連れ歩き動作停止チェック
  if( CheckStepWatchBit(OTHER_PAIR_BIT) ){
    #ifndef SCRCMD_PL_NULL
    MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
    if (FieldOBJ_StatusBitCheck_Move(other_pair) == 0) {
      FieldOBJ_MovePause(other_pair);
      ResetStepWatchBit(OTHER_PAIR_BIT);
    }
    #else
    GF_ASSERT( 0 );
    #endif
  }
  
  if( step_watch_bit == 0 ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 会話イベント用全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTalkObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  
#ifndef SCRCMD_PL_NULL
  MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
#else
  MMDL *other_pair = NULL;
#endif

  InitStepWatchBit();
  MMDLSYS_PauseMoveProc( fmmdlsys );
  
  if( MMDL_CheckEndAcmd(player) == FALSE ){
    SetStepWatchBit(PLAYER_BIT);
    MMDL_OffStatusBitMoveProcPause( player );
  }
  
  if( *fmmdl != NULL ){
    if( MMDL_CheckStatusBitMove(*fmmdl) == TRUE ){
      SetStepWatchBit(OTHER_BIT);
      MMDL_OffStatusBitMoveProcPause( *fmmdl );
    }
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
    if( MMDL_CheckStatusBitMove(other_pair) == TRUE ){
      SetStepWatchBit(OTHER_PAIR_BIT);
      MMDL_OffStatusBitMoveProcPause( other_pair );
    }
  }
  
  VMCMD_SetWait( core, EvWaitTalkObj );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc;
  MMDL **fmmdl;
  
  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );
    
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
          return VMCMD_RESULT_SUSPEND;
        }
      }
    }
    #endif
  }else{
    EvCmdTalkObjPauseAll( core, wk );
  }
  
  return VMCMD_RESULT_SUSPEND;
}
#endif

static VMCMD_RESULT evcmd_ObjPauseAll( VMHANDLE *core, SCRCMD_WORK *work )
{
  SCRIPT_WORK *sc;
  MMDL **fmmdl;
  
  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );
    
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
          return VMCMD_RESULT_SUSPEND;
        }
      }
    }
    #endif
  }else{
    return( EvCmdTalkObjPauseAll(core,work) );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作再開
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseClearAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
  return VMCMD_RESULT_SUSPEND;
}
#endif
static void evcmd_PauseClearAll( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
}

//--------------------------------------------------------------
/**
 * 話しかけたOBJ自機方向への振り向き
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjTurn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL *jiki,**fmmdl;
  u16 dir;
  
  jiki = scmd_GetMMdlPlayer( work );

  dir = MMDL_GetDirDisp( jiki );
  
  dir = MMDL_TOOL_FlipDir( dir );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    return VMCMD_RESULT_CONTINUE;
  }
  
  MMDL_SetDirDisp( *fmmdl, dir );
  MMDL_SetDrawStatus( *fmmdl, DRAW_STA_STOP );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/**
 * 「はい・いいえ」処理 ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvYesNoWinSelect( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FLDMENUFUNC **mw = SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, core->vm_register[0] );
  FLDMENUFUNC_YESNO ret = FLDMENUFUNC_ProcYesNoMenu( *mw );
  
  if( ret == FLDMENUFUNC_YESNO_NULL ){
    return FALSE;
  }
  
  if( ret == FLDMENUFUNC_YESNO_YES ){
    *ret_wk = 0;
  }else{
    *ret_wk = 1;
  }
  
  FLDMENUFUNC_DeleteMenu( *mw );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 「はい・いいえ」処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdYesNoWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  FLDMENUFUNC **mw  = SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
  u16 wk_id      = VMGetU16( core );
  
  *mw = FLDMENUFUNC_AddYesNoMenu( fparam->msgBG, 0 );
  core->vm_register[0] = wk_id;
   
  VMCMD_SetWait( core, EvYesNoWinSelect );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
// トレーナーフラグ関連
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナーフラグのセット(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTrainerFlagSet( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
  u16 flag = SCRCMD_GetVMWorkValue(core,work);  //トレーナーIDを渡す！　ワークナンバーを渡すのはダメ！
  SCRIPT_SetEventFlagTrainer( ev, flag );
  KAGAYA_Printf( "トレーナーフラグセット ID=%d", flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナーフラグのリセット(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTrainerFlagReset( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
  u16 flag = SCRCMD_GetVMWorkValue(core,work);  //トレーナーIDを渡す！　ワークナンバーを渡すのはダメ！
  SCRIPT_ResetEventFlagTrainer( ev, flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナーフラグのチェック(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTrainerFlagCheck( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
  u16 flag = SCRCMD_GetVMWorkValue(core,work);  //トレーナーIDを渡す！ ワークナンバーを渡すのはダメ！
  core->cmp_flag = SCRIPT_CheckEventFlagTrainer( ev, flag );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  トレーナーメッセージ
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナー会話呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdTrainerMessageSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 *script_id = SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
  
  u16 tr_id = SCRCMD_GetVMWorkValue( core, work );
  u16 kind_id = SCRCMD_GetVMWorkValue( core, work );
  
  WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  u8 *win_open_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_OPEN_FLAG );
  u8 *msg_index      = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGINDEX );
  
  KAGAYA_Printf( "TR ID =%d, KIND ID =%d\n", tr_id, kind_id );

  TT_TrainerMessageGet(
      tr_id, kind_id, *msgbuf, SCRCMD_WORK_GetHeapID(work) );
  
  {
    u16 dir;
    VecFx32 pos;
    const VecFx32 *pos_p;
    MMDL *fmmdl;
    
    fmmdl = MMDLSYS_SearchOBJID( SCRCMD_WORK_GetMMdlSys(work), 0xff );
    MMDL_GetVectorPos( fmmdl, &pos );
    dir = MMDL_GetDirDisp( fmmdl );
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
    if( dir == DIR_UP ){        //下から
      pos.x += FX32_ONE*8;
      pos.z -= FX32_ONE*8;
    }else if( dir == DIR_DOWN ){ //上から
      pos.x += FX32_ONE*8;
      pos.z -= FX32_ONE*8;
    }else if( dir == DIR_LEFT ){ //右から
      pos.x += -FX32_ONE*8;
      pos.z += FX32_ONE*16;
    }else{                       //左から
      pos.x += FX32_ONE*8;
      pos.z += FX32_ONE*16;
    }
    
    SCRCMD_WORK_SetTalkMsgWinTailPos( work, &pos );
    pos_p = SCRCMD_WORK_GetTalkMsgWinTailPos( work );
    
    {
      FLDTALKMSGWIN *tmsg;
      FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
      SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork(
          sc, ID_EVSCR_WK_FLDPARAM );
      
      if( dir == DIR_UP ){
        idx = FLDTALKMSGWIN_IDX_UPPER;
      }
      
      tmsg = FLDTALKMSGWIN_AddStrBuf( fparam->msgBG, idx, pos_p, *msgbuf );
      
      SCRCMD_WORK_SetFldMsgWinStream( work, (FLDMSGWIN_STREAM*)tmsg );
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      
      VMCMD_SetWait( core, BallonWinMsgWait );
      return VMCMD_RESULT_SUSPEND;
    }
  }
}

//======================================================================
//  メニュー
//======================================================================
//--------------------------------------------------------------
/**
 * BMPメニュー初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBmpMenuInit( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  u8 x = VMGetU8(core);
  u8 y = VMGetU8(core);
  u8 cursor = VMGetU8(core);
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );

  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, ret,
      *wordset, NULL );
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー初期化(読み込んでいるgmmファイルを使用する) 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBmpMenuInitEx( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  u8 x = VMGetU8(core);
  u8 y = VMGetU8(core);
  u8 cursor = VMGetU8(core);
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );

  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, ret,
      *wordset, msgData );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	リスト作成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBmpMenuMakeList( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 msg_id	= VMGetU16( core );
	u16 param	= VMGetU16( core );
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
  SCRCMD_WORK_AddMenuList( work, msg_id, param, *msgbuf, *tmpbuf );
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvSelWinWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  if( SCRCMD_WORK_ProcMenu(work) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * BMPメニュー	開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdBmpMenuStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRCMD_WORK_StartMenu( work );
	VMCMD_SetWait( core, EvSelWinWait );
	return VMCMD_RESULT_SUSPEND;
}


//======================================================================
//  その他
//======================================================================
//--------------------------------------------------------------
/**
 * 言語IDを切り替え
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
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
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 乱数の取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGetRand( VMHANDLE *core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 limit = SCRCMD_GetVMWorkValue( core, wk );
  *ret_wk = GFUser_GetPublicRand( limit );
// return VMCMD_RESULT_SUSPEND;
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdGetNowMsgArcID( VMHANDLE * core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  { //とりあえず。
    //本来はスクリプト起動時に使用するメッセージアーカイブIDを保存しておき、
    //そこからわたすような仕組みとするべき
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u16 zone_id = PLAYERWORK_getZoneID( player );
    *ret_wk = ZONEDATA_GetMessageArcID( zone_id );
  }
  return VMCMD_RESULT_CONTINUE;
}
//======================================================================
//  画面フェード
//======================================================================
//--------------------------------------------------------------
/**
 * 画面フェード
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdDispFadeStart( VMHANDLE *core, void *wk )
{
  u16 mode = VMGetU16( core );
  u16 start_evy = VMGetU16( core );
  u16 end_evy = VMGetU16( core );
  u16 wait = VMGetU16( core );
  GFL_FADE_SetMasterBrightReq( mode, start_evy, end_evy, wait );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 画面フェード終了チェック ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvCmdDispFadeWait( VMHANDLE *core, void *wk )
{
  if( GFL_FADE_CheckFade() == TRUE ){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 画面フェード終了チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdDispFadeCheck( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvCmdDispFadeWait );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * ワークを取得する。
 * 次の2バイトをワークを指定するIDとみなして、ワークへのポインタを取得する
 * @param  core  仮想マシン制御ワークへのポインタ
 * @retval  u16 *  ワークへのポインタ
 */
//--------------------------------------------------------------
u16 * SCRCMD_GetVMWork( VMHANDLE *core, SCRCMD_WORK *work )
{
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 work_id = VMGetU16(core);
  TAMADA_Printf("WORKID:%04x\n", work_id);
  return SCRIPT_GetEventWork( sc, gdata, work_id );
}

//--------------------------------------------------------------
/**
 * ワークから値を取得する。
 * 次の2バイトがSVWK_START（0x4000以下）であれば値として受け取る。
 * それ以上の場合はワークを指定するIDとみなして、そのワークから値を取得する
 * @param  core  仮想マシン制御ワークへのポインタ
 * @retval  u16    値
 */
//--------------------------------------------------------------
u16 SCRCMD_GetVMWorkValue( VMHANDLE * core, SCRCMD_WORK *work )
{
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 work_id = VMGetU16(core);
  TAMADA_Printf("WORKID:%04x\n", work_id);
  return SCRIPT_GetEventWorkValue( sc, gdata, work_id );
}

//--------------------------------------------------------------
/**
 * 自機MMDL取得　仮
 * @param work SCRCMD_WORK
 * @retval
 */
//--------------------------------------------------------------
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
  return( fmmdl );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
///  スクリプトコマンドテーブル
//--------------------------------------------------------------
const VMCMD_FUNC ScriptCmdTbl[] = {
  //基本システム命令
  EvCmdNop,        
  EvCmdDummy,        
  EvCmdEnd,
  EvCmdTimeWait,
  EvCmdGlobalCall,
  EvCmdRet,
  
  EvCmdPushValue,
  EvCmdPushWork,
  EvCmdPopWork,
  EvCmdPop,
  EvCmdCalcAdd,
  EvCmdCalcSub,
  EvCmdCalcMul,
  EvCmdCalcDiv,
  EvCmdPushFlagValue,
  EvCmdCmpStack,

  //データロード・ストア関連
  EvCmdLoadRegValue,
  EvCmdLoadRegWData,
  EvCmdLoadRegAdrs,
  EvCmdLoadAdrsValue,
  EvCmdLoadAdrsReg,
  EvCmdLoadRegReg,
  EvCmdLoadAdrsAdrs,
  
  //比較命令
  EvCmdCmpRegReg,
  EvCmdCmpRegValue,
  EvCmdCmpRegAdrs,    
  EvCmdCmpAdrsReg,    
  EvCmdCmpAdrsValue,
  EvCmdCmpAdrsAdrs,
  EvCmdCmpWkValue,
  EvCmdCmpWkWk,
  
  //仮想マシン関連
  EvCmdVMMachineAdd,
  EvCmdChangeCommonScr,
  EvCmdChangeLocalScr,
  
  //分岐命令
  EvCmdGlobalJump,
  EvCmdIfJump,
  EvCmdIfCall,
  EvCmdObjIDJump,
  EvCmdBgIDJump,
  EvCmdPlayerDirJump,
    
  //イベントワーク関連
  EvCmdFlagSet,
  EvCmdFlagReset,
  EvCmdFlagCheck,
  EvCmdFlagCheckWk,
  EvCmdFlagSetWk,
  
  //ワーク操作関連
  EvCmdWkAdd,
  EvCmdWkSub,
  EvCmdLoadWkValue,
  EvCmdLoadWkWk,
  EvCmdLoadWkWkValue,
  
  //フィールドイベント共通処理
  EvCmdCommonProcFieldEventStart,
  EvCmdCommonProcFieldEventEnd,

  //キー入力関連
  EvCmdABKeyWait,
  
  //会話ウィンドウ
  EvCmdTalkMsg,
  EvCmdTalkMsgAllPut,
  EvCmdTalkWinOpen,
  EvCmdTalkWinClose,
  
  //吹き出しウィンドウ
  EvCmdBalloonWinWrite,
  EvCmdBalloonWinTalkWrite,
  EvCmdBalloonWinClose,
  
  //動作モデル
  EvCmdObjAnime,
  EvCmdObjAnimeWait,
  EvCmdMoveCodeGet,
  EvCmdObjPosGet,
  EvCmdPlayerPosGet,
  EvCmdObjAdd,
  EvCmdObjDel,
  EvCmdObjAddEvent,
  EvCmdObjDelEvent,
  
  //動作モデル　イベント関連
  EvCmdObjTurn,
  
  //はい、いいえ　処理
  EvCmdYesNoWin,
  
  //WORDSET関連
  EvCmdPlayerName,
  EvCmdItemName,
  EvCmdItemWazaName,
  EvCmdWazaName,
  EvCmdPocketName,
  
  //scrcmd_trainer.c 視線トレーナー関連
  EvCmdEyeTrainerMoveSet,
  EvCmdEyeTrainerMoveSingle,
  EvCmdEyeTrainerMoveDouble,
  EvCmdEyeTrainerTypeGet,
  EvCmdEyeTrainerIdGet,
  
  //scrcmd_trainer.c トレーナーバトル関連
  EvCmdTrainerIdGet,
  EvCmdTrainerBattleSet,
  EvCmdTrainerMultiBattleSet,
  EvCmdTrainerMessageSet,
  EvCmdTrainerTalkTypeGet,
  EvCmdRevengeTrainerTalkTypeGet,
  EvCmdTrainerTypeGet,
  EvCmdTrainerBgmSet,
  EvCmdTrainerLose,
  EvCmdTrainerLoseCheck,
  EvCmdSeacretPokeRetryCheck,
  EvCmdHaifuPokeRetryCheck,
  EvCmd2vs2BattleCheck,
  EvCmdDebugBattleSet,
  EvCmdBattleResultGet,
  
  //トレーナーフラグ関連
  EvCmdTrainerFlagSet,
  EvCmdTrainerFlagReset,
  EvCmdTrainerFlagCheck,
  
  //BGM
  EvCmdBgmPlay,
  EvCmdBgmStop,
  EvCmdBgmPlayerPause,
  EvCmdBgmPlayCheck,
  EvCmdBgmFadeOut,
  EvCmdBgmFadeIn,
  EvCmdBgmNowMapPlay,
  
  //SE
  EvCmdSePlay,
  EvCmdSeStop,
  EvCmdSeWait,
  
  //ME
  EvCmdMePlay,
  EvCmdMeWait,
  
  //メニュー
  EvCmdBmpMenuInit,
  EvCmdBmpMenuInitEx,
  EvCmdBmpMenuMakeList,
  EvCmdBmpMenuStart,
  
  //画面フェード
  EvCmdDispFadeStart,
  EvCmdDispFadeCheck,
  
  //アイテム関連
  EvCmdAddItem,
  EvCmdSubItem,
  EvCmdAddItemChk,
  EvCmdCheckItem,
  EvCmdGetItemNum,
  EvCmdWazaMachineItemNoCheck,
  EvCmdGetPocketID,
  
  //ミュージカル関連
  EvCmdMusicalCall,
  
  //その他
  EvCmdChangeLangID,
  EvCmdGetRand,
  EvCmdGetNowMsgArcID,
};


//--------------------------------------------------------------
///  スクリプトコマンドの最大数
//--------------------------------------------------------------
const u32 ScriptCmdMax = NELEMS(ScriptCmdTbl);

//--------------------------------------------------------------
///  条件分岐用テーブル
//--------------------------------------------------------------
static const u8 ConditionTable[6][3] =
{
//    MINUS  EQUAL  PLUS
  { TRUE,   FALSE, FALSE },  // LT
  { FALSE, TRUE,  FALSE },  // EQ
  { FALSE, FALSE, TRUE  },  // GT
  { TRUE,  TRUE,  FALSE },  // LE
  { FALSE, TRUE,  TRUE  },  // GE
  { TRUE,  FALSE, TRUE  }    // NE
};
