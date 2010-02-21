//======================================================================
/**
 * @file  scrcmd_trial_house.c
 * @brief  スクリプトコマンド：トライアルハウス
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_fieldmap_control.h"
#include "scrcmd_trial_house.h"
#include "trial_house.h"

//--------------------------------------------------------------
/**
 * トライアルハウス開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_Start( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );

  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  if ( *ptr == NULL ) *ptr = TRIAL_HOUSE_Start();
  else GF_ASSERT_MSG(0,"TRIAL_HOUSE_ALLOC_ERROR");

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トライアルハウス終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_End( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);
  TRIAL_HOUSE_End(ptr);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トライアルハウス プレイモードのセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_SetPlayMode( VMHANDLE *core, void *wk )
{
  u16 play_mode;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  play_mode = SCRCMD_GetVMWorkValue( core, work );  
  TRIAL_HOUSE_SetPlayMode(*ptr, play_mode );
  return VMCMD_RESULT_CONTINUE;

}

//--------------------------------------------------------------
/**
 * トライアルハウス ポケモン選択
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_SelectPoke( VMHANDLE *core, void *wk )
{
  u16 *ret;
  u16 reg_type;
  u16 party_type;
  GMEVENT* event;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  reg_type = SCRCMD_GetVMWorkValue( core, work );
  party_type = SCRCMD_GetVMWorkValue( core, work );
  ret = SCRCMD_GetVMWork( core, work );
  
  event = TRIAL_HOUSE_CreatePokeSelEvt(gsys, *ptr, reg_type, party_type, ret );
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * トライアルハウス トレーナーセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_SetTrainer( VMHANDLE *core, void *wk )
{
  u16 *obj_id;
  u16 btl_count;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  btl_count = SCRCMD_GetVMWorkValue( core, work );
  obj_id = SCRCMD_GetVMWork( core, work );

  *obj_id = TRIAL_HOUSE_MakeTrainer( *ptr, btl_count );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トライアルハウス 対戦前メッセージ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_DispBeforeMsg( VMHANDLE *core, void *wk )
{
  GMEVENT* event;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  event = TRIAL_HOUSE_CreateBeforeMsgEvt(gsys, *ptr);

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * トライアルハウス 戦闘開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_CallBattle( VMHANDLE *core, void *wk )
{
  GMEVENT* event;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);

  event = TRIAL_HOUSE_CreateBtlEvt(gsys, *ptr);

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * トライアルハウス ランクセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_SetRank( VMHANDLE *core, void *wk )
{
  u16 rank;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );

  rank = SCRCMD_GetVMWorkValue( core, work );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トライアルハウス ランク取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTH_GetRank( VMHANDLE *core, void *wk )
{
  u16 *rank;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );

  rank = SCRCMD_GetVMWork( core, work );

  rank = 0;//@todo
  return VMCMD_RESULT_CONTINUE;
}



