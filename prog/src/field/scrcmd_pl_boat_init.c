//======================================================================
/**
 * @file  scrcmd_pl_boat_init.c
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

//--------------------------------------------------------------
/**
 * 部屋の情報を取得する 
 * @note    ゾーンチェンジ時に呼ぶので常駐関数にしています
 * @param   core          仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_GetRoomInfo( VMHANDLE *core, void *wk )
{
  u16 room_idx;
  u16 info_kind;
  u16 param;
  u16 *ret;

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  room_idx = SCRCMD_GetVMWorkValue( core, work );
  info_kind = VMGetU16( core );
  param = SCRCMD_GetVMWorkValue( core, work );
  ret = SCRCMD_GetVMWork( core, work );

  switch(info_kind){
  case PL_BOAT_ROOM_INFO_OBJ_CODE:
    *ret = PL_BOAT_GetObjCode(*ptr, room_idx);
    break;
  case PL_BOAT_ROOM_INFO_NPC_TYPE:
    *ret = PL_BOAT_GetNpcType(*ptr, room_idx);
    break;
  case PL_BOAT_ROOM_INFO_TR_ID:
    *ret = PL_BOAT_GetTrID(*ptr, room_idx);
    break;
  case PL_BOAT_ROOM_INFO_MSG:
    *ret = PL_BOAT_GetMsg(*ptr, room_idx, param);
    break;
  case PL_BOAT_ROOM_INFO_BTL_FLG:
    if ( PL_BOAT_CheckBtlFlg(*ptr, room_idx) ) *ret = PL_BOAT_TR_BTL_ALREADY;  //戦った
    else *ret = PL_BOAT_TR_BTL_YET;  //戦ってない
    break;
/**    
  case PL_BOAT_ROOM_INFO_BTL_TYPE:
    if ( PL_BOAT_CheckDblBtl(*ptr, room_idx) ) *ret = PL_BOAT_BTL_TYPE_DOUBLE;  //ダブルバトルの部屋
    else *ret = PL_BOAT_BTL_TYPE_SINGLE;  //シングルバトルの部屋
    break;
*/    
  default:
    GF_ASSERT(0);
    *ret = 0;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 甲板デモコール
 * @param   core          仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlBoat_CallDemo( VMHANDLE *core, void *wk )
{
  u16 *ret;
  GMEVENT *call_event;
  
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);

  ret = SCRCMD_GetVMWork( core, work );

  call_event = PL_BOAT_CreateDemoEvt(gsys, *ptr, ret);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );

  return VMCMD_RESULT_SUSPEND;
}

