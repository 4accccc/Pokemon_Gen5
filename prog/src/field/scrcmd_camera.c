//======================================================================
/**
 * @file  scrcmd_camera.c
 * @brief  カメラ操作スクリプトコマンド用関数
 * @author  Nozomu Satio
 *
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

#include "scrcmd_camera.h"

static GMEVENT_RESULT WaitTraceStopEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitCamMovEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * カメラ移動開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_Start( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  //復帰パラメータをセットする
  FIELD_CAMERA_SetRecvCamParam(camera);
  //カメラトレースを停止するリクエストを出す
  FIELD_CAMERA_StopTraceRequest(camera);
  //カメラトレース処理が停止するのを待つイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = GMEVENT_Create( gsys, NULL, WaitTraceStopEvt, 0 );
    SCRIPT_CallEvent( sc, call_event );
  }
#if 0
  //終了チェックフラグをオン
  {
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    SCREND_CHECK_WK *chk = SCRIPT_GetScrEndChkWkPtr( sc );
    chk->CameraEndCheck = 1;
  }
#endif
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * カメラ移動終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_End( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  //復帰パラメータをクリアする
  FIELD_CAMERA_ClearRecvCamParam(camera);
  //カメラトレース再開
  FIELD_CAMERA_RestartTrace(camera);
#if 0
  //終了チェックフラグをオフ
  {
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    SCREND_CHECK_WK *chk = SCRIPT_GetScrEndChkWkPtr( sc );
    chk->CameraEndCheck = 0;
  }
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 対象物とカメラのバインドを解く
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_Purge( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  FIELD_CAMERA_FreeTarget(camera);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 対象物とカメラをバインドする
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_Bind( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  FIELD_CAMERA_BindDefaultTarget(camera);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * カメラの移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_Move( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );


  {
    FLD_CAM_MV_PARAM param;
    u16 frame;
    param.Core.AnglePitch = VMGetU16( core );
    param.Core.AngleYaw = VMGetU16( core );
    param.Core.Distance = VMGetU16( core );
    param.Core.TrgtPos.x = VMGetU32( core );
    param.Core.TrgtPos.y = VMGetU32( core );
    param.Core.TrgtPos.z = VMGetU32( core );
    param.Chk.Shift = FALSE;
    param.Chk.Angle = TRUE;
    param.Chk.Dist = TRUE;
    param.Chk.Fovy = FALSE;
    param.Chk.Pos = TRUE;
    frame = VMGetU16( core );
    FIELD_CAMERA_SetLinerParam(camera, &param, frame);
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * カメラの復帰移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_RecoverMove( VMHANDLE *core, void *wk )
{
  u16 frame;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  frame = VMGetU16( core );

  FIELD_CAMERA_RecvLinerParam(camera, frame);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * カメラ移動終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_WaitMove( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  //カメラ移動終了待ちをするイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
    call_event = GMEVENT_Create( gsys, NULL, WaitCamMovEvt, 0 );
    SCRIPT_CallEvent( sc, call_event );
  }

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * カメラトレース処理停止待ちイベント
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   work　　　　      ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WaitTraceStopEvt( GMEVENT* event, int* seq, void* work )
{
  BOOL rc;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  
  rc = FIELD_CAMERA_CheckTrace(camera);

  if (!rc){
    //停止
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * カメラ移動終了待ちイベント
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   work　　　　      ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WaitCamMovEvt( GMEVENT* event, int* seq, void* work )
{
  BOOL rc;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  rc = FIELD_CAMERA_CheckMvFunc(camera);

  if (!rc){
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}











