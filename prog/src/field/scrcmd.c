//======================================================================
/**
 * @file  scrcmd.c
 * @brief  スクリプトコマンド用関数
 * @author  Satoshi Nohara
 * @date  05.08.04
 *
 * 05.04.26 Hiroyuki Nakamura
 *
 * @note CheckEndFuncTblの並びは、scrend_check_bit.hのSCREND_CHKと1対1で対応するようにしてください
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "msgdata.h"
#include "print/wordset.h"

#include "sound/pm_sndsys.h"
//#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field_sound.h"
#include "field_player.h"

#include "event_gameover.h" //EVENT_NormalLose
#include "event_battle.h"   //FIELD_BATTLE_IsLoseResult

#include "debug/debug_flg.h" //DEBUG_FLG_～


#include "scrcmd_battle.h"
#include "scrcmd_fld_battle.h"
#include "scrcmd_trainer.h"
#include "scrcmd_sound.h"
#include "scrcmd_musical.h"
#include "scrcmd_word.h"
#include "scrcmd_item.h"
#include "scrcmd_mapchange.h"
#include "scrcmd_menuwin.h"
#include "scrcmd_fldmmdl.h"
#include "scrcmd_gym.h"
#include "scrcmd_pokemon.h"
#include "scrcmd_pokemon_fld.h"
#include "scrcmd_screeneffects.h"
#include "scrcmd_enviroments.h"
#include "scrcmd_sodateya.h"
#include "scrcmd_hiden.h"
#include "scrcmd_player.h"
#include "scrcmd_network.h"
#include "scrcmd_camera.h"
#include "scrcmd_shop.h"
#include "scrcmd_wifi.h"
#include "scrcmd_wifi_st.h"
#include "scrcmd_proc.h"
#include "scrcmd_proc_fld.h"
#include "scrcmd_intrude.h"
#include "scrcmd_encount.h"
#include "scrcmd_gate.h"
#include "scrcmd_fldci.h"
#include "scrcmd_fourkings.h"
#include "scrcmd_pl_boat.h"
#include "scrcmd_bsubway.h"
#include "scrcmd_league_front.h"
#include "scrcmd_palpark.h"
#include "scrcmd_wfbc.h"
#include "scrcmd_gimmick_st.h"
#include "scrcmd_c03center.h"

#include "../../../resource/fldmapdata/script/usescript.h"

//======================================================================
//  define
//======================================================================

#define CHECK_BIT_MAX (32)

#ifdef PM_DEBUG
BOOL MapFadeReqFlg = FALSE;    //マップフェードリクエストフラグ
#endif

//--------------------------------------------------------------
/// レジスタ比較の結果定義
//--------------------------------------------------------------
enum {
  MINUS_RESULT = 0,  //比較結果がマイナス
  EQUAL_RESULT,    //比較結果がイコール
  PLUS_RESULT      //比較結果がプラス
};


//======================================================================
//  proto
//======================================================================
//parts
static u16 getZoneID( SCRCMD_WORK *work );

//data
const u32 ScriptCmdMax;
const VMCMD_FUNC ScriptCmdTbl[];

static const u8 ConditionTable[6][3];

typedef BOOL (*SCR_END_CHECK_FUNC)(SCREND_CHECK *end_check, int *seq);

#define FUNC_MAX ( NELEMS(CheckEndFuncTbl) )

typedef struct
{
  int CheckBit;
}SCREND_CHECK_BITS;

//※CheckEndFuncTblの並びは、scrend_check_bit.hのSCREND_CHKと1対1で対応するようにしてください
static const SCR_END_CHECK_FUNC CheckEndFuncTbl[] = {
  SCREND_CheckEndCamera,
  SCREND_CheckEndWin,
  SCREND_CheckEndBallonWin,
  /*ここにスクリプト終了時の終了関数を追加してください*/
  SCREND_CheckEndPlainWin,
  SCREND_CheckEndBGWin,
  SCREND_CheckEndSubWin,
  SCREND_CheckEndSpWin,
  NULL,   //テーブル終了検出用
};

typedef struct CHECK_WORK_tag
{
  SCREND_CHECK EndCheck;
  int Seq;
  int Next;
  int FuncMax;
}CHECK_WORK;

SCREND_CHECK_BITS ScrEndCheckBits;

static GMEVENT *CreateEndCheckEvt(SCRCMD_WORK *work);
static GMEVENT_RESULT ScrCheckEvt( GMEVENT* event, int* seq, void* work );
static BOOL CheckEndBit(const SCREND_CHK inCheckBit);

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れチェックビット初期化
 */
//--------------------------------------------------------------
void SCREND_CHK_ClearBits(void)
{
  MI_CpuClear32( &ScrEndCheckBits, sizeof(SCREND_CHECK_BITS) );
}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れチェックビットオン
 */
//--------------------------------------------------------------
void SCREND_CHK_SetBitOn(const SCREND_CHK inCheckBit)
{
  if (inCheckBit >= SCREND_CHK_MAX){
    GF_ASSERT_MSG(0,"CHECK_BIT_OVER\n");
    return;
  }

  if ( CheckEndBit(inCheckBit) ){
    OS_Printf("%d::already bit on\n",inCheckBit);
    return;
  }

  ScrEndCheckBits.CheckBit |= (1 <<inCheckBit);
}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れチェックビットオフ
 */
//--------------------------------------------------------------
void SCREND_CHK_SetBitOff(const SCREND_CHK inCheckBit)
{
  if (inCheckBit >= SCREND_CHK_MAX){
    GF_ASSERT_MSG(0,"CHECK_BIT_OVER\n");
    return;
  }

  if ( !CheckEndBit(inCheckBit) ){
    OS_Printf("%d::already bit off\n",inCheckBit);
    return;
  }
#ifdef DEBUG_ONLY_FOR_saitou 
  OS_Printf("ScrEndCheckBit off before = %d\n",ScrEndCheckBits.CheckBit);
#endif //DEBUG_ONLY_FOR_saitou  

  ScrEndCheckBits.CheckBit = 
    ScrEndCheckBits.CheckBit & ( 0xffffffff ^ (1<<inCheckBit) );

#ifdef DEBUG_ONLY_FOR_saitou 
  OS_Printf("ScrEndCheckBit off after = %d\n",ScrEndCheckBits.CheckBit);
#endif //DEBUG_ONLY_FOR_saitou
}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れチェックビットをチェック
 */
//--------------------------------------------------------------
BOOL SCREND_CHK_CheckBit(const SCREND_CHK inCheckBit)
{
  return CheckEndBit(inCheckBit);
}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れチェックビットをチェック
 */
//--------------------------------------------------------------
static BOOL CheckEndBit(const SCREND_CHK inCheckBit)
{
  int bit;
  if (inCheckBit >= SCREND_CHK_MAX){
    GF_ASSERT_MSG(0,"CHECK_BIT_OVER\n");
    return FALSE;
  }
  bit = ScrEndCheckBits.CheckBit >> inCheckBit;
  bit &= 0x1;
  if ( bit ) return TRUE;

  return FALSE;
}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れをチェックイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ScrCheckEvt( GMEVENT* event, int* seq, void* work )
{
  CHECK_WORK *check_wk;
  SCR_END_CHECK_FUNC func;

  check_wk = work;

  do{
    BOOL rc;
    //ファンクション取得
    func = CheckEndFuncTbl[check_wk->Next];
    //ファンクションはＮＵＬＬか？（終了か？）
    if (func == NULL) return GMEVENT_RES_FINISH;    //全ファンクション終了

    //ビットチェック
    if ( CheckEndBit(check_wk->Next) )
    {
#ifdef SCR_ASSERT_ON      
      GF_ASSERT_MSG(0,"%d::EndFunc Not Call\n", check_wk->Next);
#else
      OS_Printf("=============%d::EndFunc Not Call===============\n",check_wk->Next);
#endif
      //ファンクション実行
      rc = func(&check_wk->EndCheck, &check_wk->Seq);
      if (rc)
      {
        //ファンクション終了ならば次のファンクションへ
        check_wk->Next++;
        //次のファンクションのために、サブシーケンサを初期化
        check_wk->Seq = 0;
      }
      else{
        return GMEVENT_RES_CONTINUE;  //一度処理を返す
      }
    }
    else
    {
      //ファンクション終了ならば次のファンクションへ
      check_wk->Next++;
      //次のファンクションのために、サブシーケンサを初期化
      check_wk->Seq = 0;
    }
  }while(check_wk->Next < check_wk->FuncMax);

  //通常ここにくることはないが不測の事態のために。
  return GMEVENT_RES_FINISH;

}

//--------------------------------------------------------------
/**
 * スクリプト終了時のコマンド記述忘れをチェックイベント作成
 */
//--------------------------------------------------------------
static GMEVENT *CreateEndCheckEvt(SCRCMD_WORK *work)
{
  GMEVENT * event;
  CHECK_WORK *check_wk;
  int work_size;

  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  work_size = sizeof(CHECK_WORK);
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ScrCheckEvt, work_size );

  check_wk = GMEVENT_GetEventWork( event );
  //ワークの初期化
  check_wk->EndCheck.gsys = gsys;
  check_wk->EndCheck.ScrCmdWk = work;
  check_wk->EndCheck.ScrWk = sc;
  check_wk->Next = 0;
  check_wk->Seq = 0;
  check_wk->FuncMax = FUNC_MAX;

  GF_ASSERT_MSG(check_wk->FuncMax -1 <= CHECK_BIT_MAX,"チェックビットが足りません。増やしますので、担当まで連絡を。");

#ifdef DEBUG_ONLY_FOR_saitou 
  OS_Printf("ScrEndFuncNum = %d\n",check_wk->FuncMax);
#endif //DEBUG_ONLY_FOR_saitou

  if ( check_wk->FuncMax ){
    GF_ASSERT(check_wk->FuncMax - 1 == SCREND_CHK_MAX);
  }
  return event;
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
static VMCMD_RESULT EvCmdCall( VMHANDLE *core, void *wk )
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

//--------------------------------------------------------------
/**
 * ワークの値を出力する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDebugPrintWk( VMHANDLE * core, void *wk )
{
  GAMEDATA* gdata = SCRCMD_WORK_GetGameData( wk );
  u16         val = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第1引数

  OS_Printf( "EvCmdDebugPrintWk: %d\n", val );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT  EvCmdDebugAssert( VMHANDLE * core, void *wk )
{
  u16 val1 = VMCMD_Pop( core );
  u16 val2 = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第1引数
  GF_ASSERT_MSG( val1 == TRUE, "スクリプトレベルのAssertです！\n" ) 
  return VMCMD_RESULT_CONTINUE;
}


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
/**
 * @brief スクリプトコマンド：ワークの値をプッシュ
 */
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
/**
 * @brief  スクリプトコマンド：スタックポップ→ワーク
 */
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
/**
 * @brief スクリプトコマンド：スタックポップ
 */
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
  u16 scr_id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  
  scr_id = VMGetU16(core);

  //仮想マシン追加
  SCRIPT_AddVMachine( sc, getZoneID(work), scr_id, VMHANDLE_SUB1 );
  
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

  if ( SCRIPT_GetVMExists( sc, VMHANDLE_SUB1 ) != TRUE) {
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

  scr_id = VMGetU16(core);

  //共通スクリプト以外にも切り替え可能になっている！
  //仮想マシン追加
  SCRIPT_AddVMachine( sc, getZoneID(work), scr_id, VMHANDLE_SUB1 );

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
static VMCMD_RESULT EvCmdJump( VMHANDLE *core, void *wk )
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
 * @brief   スクリプト呼び出しの予約
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  work
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdReserveScript( VMHANDLE *core, void *wk )
{
  u16 reserve_scr_id = VMGetU16(core);
  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
  FIELD_STATUS * fldstatus = GAMEDATA_GetFieldStatus( gamedata );

  FIELD_STATUS_SetReserveScript( fldstatus, reserve_scr_id );

  return VMCMD_RESULT_CONTINUE;
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
  //u16  flag = VMGetU16( core );
  u16 flag = SCRCMD_GetVMWorkValue( core, work );
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
  //u16  flag = VMGetU16( core );
  u16 flag = SCRCMD_GetVMWorkValue( core, work );
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
  //u16  flag = VMGetU16( core );
  u16 flag = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  *ret_wk = EVENTWORK_CheckEventFlag( evwork, flag );
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
#include "field/field_comm/intrude_types.h"
#include "field/field_comm/intrude_main.h"  //Intrude_ObjPauseCheck
static VMCMD_RESULT EvCmdCommonProcFieldEventStart( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
#if 1
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  if(Intrude_ObjPauseCheck(gsys) == FALSE){
    return VMCMD_RESULT_SUSPEND;
  }
#endif

  //全動作モデルポーズ
  return( SCRCMD_SUB_ObjPauseAll(core,work) );
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
  SCRCMD_WORK*  work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  
  //動作モデルポーズ解除
  SCRCMD_SUB_PauseClearAll( work );

  //サウンド開放忘れ回避
  {
    GMEVENT* event;
    event = EVENT_FSND_AllPopBGM( gsys, FSND_FADE_NONE );
    SCRIPT_CallEvent( sc, event );
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 終了時コマンド記述忘れチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdCheckScrEnd( VMHANDLE * core, void *wk )
{
  SCREND_CHECK end_check;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  //スクリプトチェックイベント作成
  call_event = CreateEndCheckEvt(work);
  if ( call_event == NULL ){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  
  SCRIPT_CallEvent( sc, call_event );
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
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

//--------------------------------------------------------------
/**
 * ウェイトルーチン：最後のキー待ち
 * @retval  TRUE=終了
 * @retval  FALSE=待ち状態のまま
 */
//--------------------------------------------------------------
static BOOL evWaitLastKey( VMHANDLE * core, void *wk )
{
#if 0
  enum {
    WAIT_AB_KEY = PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL,
    WAIT_PAD_KEY = PAD_KEY_UP | PAD_KEY_DOWN | PAD_KEY_LEFT | PAD_KEY_RIGHT,
    WAIT_LAST_KEY = WAIT_AB_KEY | WAIT_PAD_KEY 
  };
#endif

  u32 trg = GFL_UI_KEY_GetTrg();
  if( (trg & EVENT_WAIT_LAST_KEY) != 0)
  {
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 * スクリプトコマンド：最後のキーウェイト
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdLastKeyWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, evWaitLastKey );
  return VMCMD_RESULT_SUSPEND;
}
//======================================================================
//  その他
//======================================================================
//--------------------------------------------------------------
/**
 * セーブ・ウェイト処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitSave(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  SAVE_RESULT result;

  // 分割セーブ実行
  result = GAMEDATA_SaveAsyncMain( gdata );

  // 結果を返す
  switch( result )
  {
  case SAVE_RESULT_CONTINUE:
  case SAVE_RESULT_LAST:
    return FALSE;
  case SAVE_RESULT_OK:
    {
      u16* ret_wk = SCRCMD_GetVMWork( core, work );
      *ret_wk = TRUE;
    }
    return TRUE;
  case SAVE_RESULT_NG:
    {
      u16* ret_wk = SCRCMD_GetVMWork( core, work );
      *ret_wk = FALSE;
    }
    return TRUE;
  } 
  return FALSE;
}
//--------------------------------------------------------------
/**
 * @brief   セーブ機能呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 *
 * @todo  セーブ呼び出しの詳細が確定したら機能を呼び出す
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdReportCall( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );

#ifdef  PM_DEBUG
  if (DEBUG_FLG_GetFlg(DEBUG_FLG_DisableReport) ) {
    u16* ret_wk = SCRCMD_GetVMWork( core, work );
    *ret_wk = TRUE;
    return VMCMD_RESULT_SUSPEND;
  }
#endif
  // 分割セーブ開始
  GAMEDATA_SaveAsyncStart( gdata );
  
  // 毎フレーム処理
  VMCMD_SetWait( core, EvWaitSave ); 

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
//--------------------------------------------------------------
static u16 getZoneID( SCRCMD_WORK *work )
{
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
  u16 zone_id = PLAYERWORK_getZoneID( player );
  return zone_id;
}

//======================================================================
//  data
//======================================================================

#define INIT_CMD  const VMCMD_FUNC ScriptCmdTbl[] = {
#define DEF_CMD( symfunc, symname )   symfunc,
#define NO_CMD()                        NULL,
#define END_CMD   };

//--------------------------------------------------------------
/**
 * スクリプトコマンドテーブル
 *
 * @val ScriptCmdTbl
 *
 * @note
 * scrcmd_table.cdatの中にあるINIT_CMD/DEF_CMD/NO_CMD/END_CMDが
 * 展開されることで、スクリプトコマンド定義テーブルScriptCmdTblが
 * 定義される。
 */
//--------------------------------------------------------------
#include "scrcmd_table.cdat"


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
