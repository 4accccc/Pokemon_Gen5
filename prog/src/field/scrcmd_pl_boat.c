//======================================================================
/**
 * @file  scrcmd_pl_boat.c
 * @brief  スクリプトコマンド：遊覧船関連
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

#include "field_sound.h"

#include "scrcmd_pl_boat.h"

#include "pleasure_boat.h"

#include "pl_boat_def.h"

#include "demo\demo3d.h"

//--------------------------------------------------------------
/**
 * 遊覧船モード開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_Start( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );

  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  if ( *ptr == NULL ) *ptr = PL_BOAT_Init();
  else GF_ASSERT_MSG(0,"PL_BOAT_ALLOC_ERROR");
   
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 遊覧船モード終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_End( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);
  PL_BOAT_End(ptr);
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 船内トレーナー数を取得
 * @param   core          仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_GetTrNum( VMHANDLE *core, void *wk )
{
  u16 type;
  u16 *ret;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  type = VMGetU16( core );
  ret = SCRCMD_GetVMWork( core, work );

  *ret = PL_BOAT_GetTrNum(*ptr, type);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 船内時間強制加算
 * @param   core          仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_AddTime( VMHANDLE *core, void *wk )
{
  u16 add_time,whistle;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  add_time = VMGetU16( core );
  whistle = VMGetU16( core );

  PL_BOAT_AddTimeEvt(*ptr, add_time, whistle);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 部屋の情報をセットする　　
 * @note    2009年12月　現在はトレーナー戦闘フラグのセット以外は処理をスルーします
 * @param   core          仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_SetRoomInfo( VMHANDLE *core, void *wk )
{
  u16 room_idx;
  u16 info_kind;
  u16 param;
  u16 set_val;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  room_idx = SCRCMD_GetVMWorkValue( core, work );
  info_kind = VMGetU16( core );
  param = VMGetU16( core );
  set_val = SCRCMD_GetVMWorkValue( core, work );

  switch(info_kind){
  case PL_BOAT_ROOM_INFO_OBJ_CODE:
  case PL_BOAT_ROOM_INFO_NPC_TYPE:
  case PL_BOAT_ROOM_INFO_TR_ID:
  case PL_BOAT_ROOM_INFO_MSG:
    break;
  case PL_BOAT_ROOM_INFO_BTL_FLG:
    {
      BOOL flg;
      if ( set_val == PL_BOAT_TR_BTL_ALREADY ) flg = TRUE;
      else flg = FALSE;

      PL_BOAT_SetBtlFlg(*ptr, room_idx, flg);
    }
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 遊覧船ゲーム終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_EndGame( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);
  PL_BOAT_EndGame(*ptr);
  
  return VMCMD_RESULT_CONTINUE;
}
