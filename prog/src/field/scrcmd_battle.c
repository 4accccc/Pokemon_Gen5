//======================================================================
/**
 * @file	scrcmd_battle.c
 * @brief	スクリプトコマンド：勝敗処理など
 * @author  tamada GAMEFREAK inc.
 * @date	09.10.23
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "script_trainer.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_gameover.h" //EVENT_NormalLose
#include "event_battle.h"   //FIELD_BATTLE_IsLoseResult

#include "scrcmd_battle.h"

//======================================================================
// トレーナーフラグ関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief トレーナーフラグのセット(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerFlagSet( VMHANDLE * core, void *wk )
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
 * @brief トレーナーフラグのリセット(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerFlagReset( VMHANDLE * core, void *wk )
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
 * @brief トレーナーフラグのチェック(トレーナーIDを渡す)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerFlagCheck( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
  u16 flag = SCRCMD_GetVMWorkValue(core,work);  //トレーナーIDを渡す！ ワークナンバーを渡すのはダメ！
  u16 * ret_wk = SCRCMD_GetVMWork(core,work);
  *ret_wk = SCRIPT_CheckEventFlagTrainer( ev, flag );
  //core->cmp_flag = SCRIPT_CheckEventFlagTrainer( ev, flag );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//
//  勝敗チェック・敗北処理関連
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief トレーナー敗北処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @note
 * スクリプトを強制終了し、敗北処理イベントへと遷移する。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerLose( VMHANDLE *core, void *wk )
{
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  call_event = EVENT_NormalLose( gsys );
  SCRIPT_EntryNextEvent( sc, call_event );

  VM_End( core );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief トレーナー敗北チェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerLoseCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  BtlResult res = GAMEDATA_GetLastBattleResult(gdata);

  if (FIELD_BATTLE_IsLoseResult(res, BTL_COMPETITOR_TRAINER) == TRUE)
  {
    *ret_wk = SCR_BATTLE_RESULT_LOSE;
  } else {
    *ret_wk = SCR_BATTLE_RESULT_WIN;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 野生戦敗北処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @note
 * スクリプトを強制終了し、敗北処理イベントへと遷移する。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWildLose( VMHANDLE *core, void *wk )
{
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  call_event = EVENT_NormalLose( gsys );
  SCRIPT_EntryNextEvent( sc, call_event );

  VM_End( core );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief 野生戦敗北チェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWildLoseCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  BtlResult res = GAMEDATA_GetLastBattleResult(gdata);

  if (FIELD_BATTLE_IsLoseResult(res, BTL_COMPETITOR_WILD) == TRUE)
  {
    *ret_wk = SCR_BATTLE_RESULT_LOSE;
  } else {
    *ret_wk = SCR_BATTLE_RESULT_WIN;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 観覧車トレーナーのOBJIDを返す
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetWheelTrainerObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
 
  static const u16 obj_tbl[] = {
    GIRL2, DANCER,
    MOUNTMAN, TRAINERM,
    OL, WAITRESS,
    BABYGIRL2, BOY4,
  };
	*ret_wk = obj_tbl[id];
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 観覧車トレーナーのトレーナーIDを返す
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetWheelTrainerTrID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
  
  static const u16 trid_tbl[] = {
    TRID_MINI_06, TRID_DANCER_03,
    TRID_MOUNT_13, TRID_ELITEM_14,
    TRID_OL_04, TRID_WAITRESS_04,
    TRID_KINDERGARTENW_05,TRID_PRINCE_05,
  };

	*ret_wk = trid_tbl[id];
	return VMCMD_RESULT_CONTINUE;
}


