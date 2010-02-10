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
 * @brief 野生戦敗北処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @todo  強制終了されるスクリプトに解放忘れがないか、検討
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
 * @brief トレーナー敗北処理
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @todo  強制終了されるスクリプトに解放忘れがないか、検討
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

#if 0
/*
 * 一応要らないと思う
 * scrcmd_encount.c に EvCmdWildBattleRetryCheck()を定義
 */
//--------------------------------------------------------------
/**
 * @brief 隠しポケモン戦闘　再戦可不可チェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 *
 * @todo  本当にいるのか検討する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSeacretPokeRetryCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	VMCMD_RESULT* win_flag	= SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
#if 0
	*ret_wk = BattleParam_IsSeacretPokeRetry(*win_flag);
	return 1;
#else //wb kari
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
#endif
}

//--------------------------------------------------------------
/**
 * @brief 配布ポケモン戦闘　再戦可不可チェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 *
 * @todo  本当にいるのか検討する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdHaifuPokeRetryCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	VMCMD_RESULT* win_flag	= SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
#if 0
	*ret_wk = BattleParam_IsHaifuPokeRetry(*win_flag);
	return 1;
#else //wb kari
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
#endif
}

#endif
