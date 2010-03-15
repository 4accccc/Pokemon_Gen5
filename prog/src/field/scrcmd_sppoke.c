//======================================================================
/**
 * @file  scrcmd_sppoke.c
 * @brief  特殊ポケイベントスクリプト
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"
#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_sppoke.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "sp_poke_gimmick.h"

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMoveBall( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );
  u16 height = SCRCMD_GetVMWorkValue( core, work );
  u16 sync = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 start, end;
  VecFx32 *dst;
  VecFx32 *src;
  fx32 fx_height;

  //開始地点は自機の位置
  {
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &start );
  }
  //目的座標セット
  end.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  end.y = y * GRID_HALF_SIZE;
  end.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  if (type == SCR_BALL_ANM_TYPE_OUT)
  {
    src = &start;
    dst = &end;
  }
  else {
    src = &end;
    dst = &start;
  }

  fx_height = height * GRID_HALF_SIZE;

  event = SPPOKE_GMK_MoveBall( gsys, type, src, dst, fx_height, sync);

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdStartBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 pos;
  pos.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  pos.y = y * GRID_HALF_SIZE;
  pos.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  SPPOKE_GMK_StartBallAnm( gsys, type, &pos );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWaitBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );

  event = SPPOKE_GMK_WaitBallAnmEnd( gsys, type );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * ポケモン表示、非表示していいフレームまで待つ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWaitBallPokeApp( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );

  event = SPPOKE_GMK_WaitPokeAppear( gsys, type );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

