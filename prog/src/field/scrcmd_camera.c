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

//static GMEVENT_RESULT WaitTraceStopEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitTraceStopForCamMvEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitCamMovEvt( GMEVENT* event, int* seq, void* work );
static void EndCamera(FIELD_CAMERA *camera);

static GMEVENT_RESULT CameraShakeEvt( GMEVENT* event, int* seq, void* work );

typedef struct CAM_PARAM_tag
{
  u16 Pitch;
  u16 Yaw;
  fx32 Dist;
  VecFx32 Pos;
  FLD_CAM_MV_PARAM_CHK Chk;
}CAM_PARAM;

typedef struct CAM_SHAKE_WORK_tag
{
  u16 Width;
  u16 Height;
  u16 SpRad;
  u16 Time;

  u32 NowRad;

  const VecFx32 *WatchTarget;
}CAM_SHAKE_WORK;


// 構造体が想定のサイズとなっているかチェック
#ifdef PM_DEBUG
#ifdef _NITRO
SDK_COMPILER_ASSERT(sizeof(CAM_PARAM) == 44);
#endif
#endif		//PM_DEBUG


//--------------------------------------------------------------
/**
 * カメラ終了処理
 * @param   camera   カメラポインタ
 * @param   chk     チェック構造体
 * @retval none
 */
//--------------------------------------------------------------
static void EndCamera(FIELD_CAMERA *camera)
{
  //復帰パラメータをクリアする
  FIELD_CAMERA_ClearRecvCamParam(camera);
  if ( FIELD_CAMERA_CheckTraceSys(camera) )
  {
    //カメラトレース再開
    FIELD_CAMERA_RestartTrace(camera);
  }
  //終了チェックフラグをオフ
  SCREND_CHK_SetBitOff(SCREND_CHK_CAMERA);
}

//--------------------------------------------------------------
/**
 *スクリプト終了時カメラ終了チェック
 * @param   end_chk     チェック構造体
 * @param   seq     サブシーケンサ
 * @retval  BOOL    TRUEでチェック終了
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndCamera(SCREND_CHECK *end_check , int *seq)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(end_check->gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  EndCamera(camera);

  return  TRUE;
}

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
 
  //終了チェックフラグをオン
  SCREND_CHK_SetBitOn(SCREND_CHK_CAMERA);
  
  if ( FIELD_CAMERA_CheckTraceSys(camera) )
  {
    //カメラトレースを停止するリクエストを出す
    FIELD_CAMERA_StopTraceRequest(camera);
    //カメラトレース処理が停止するのを待つイベントをコール
    {
      GMEVENT *call_event;
      SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
      call_event = GMEVENT_Create( gsys, NULL, WaitTraceStopForCamMvEvt, 0 );
      SCRIPT_CallEvent( sc, call_event );
    }
  }
  else
  {
    //復帰パラメータをセットする
    FIELD_CAMERA_SetRecvCamParam(camera);
  }
  
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

  EndCamera(camera);

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
    param.Core.Distance = VMGetU32( core );
    param.Core.TrgtPos.x = VMGetU32( core );
    param.Core.TrgtPos.y = VMGetU32( core );
    param.Core.TrgtPos.z = VMGetU32( core );
    param.Chk.Shift = FALSE;
    param.Chk.Pitch = TRUE;
    param.Chk.Yaw = TRUE;
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
 * カメラの移動(ID指定型)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_MoveByID( VMHANDLE *core, void *wk )
{

  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  CAM_PARAM cam_param;
  u16 param_id;
  u16 frame;
  param_id = VMGetU16( core );
  frame = VMGetU16( core );
  
  //アーカイブからリソース定義をロード
  GFL_ARC_LoadData(&cam_param, ARCID_SCR_CAM_PRM, param_id);
  
  {
    FLD_CAM_MV_PARAM param;
    param.Core.AnglePitch = cam_param.Pitch;
    param.Core.AngleYaw = cam_param.Yaw;
    param.Core.Distance = cam_param.Dist;
    param.Core.TrgtPos = cam_param.Pos;
    param.Chk = cam_param.Chk;

    NOZOMU_Printf("cam_prm = %d,%d,%d\n",cam_param.Pitch, cam_param.Yaw, cam_param.Dist);
    TAMADA_Printf("cam_prm = %d,%d,%d\n",cam_param.Pitch, cam_param.Yaw, cam_param.Dist);
    TAMADA_Printf("cam_prm = %d,%d,%d\n",cam_param.Pos.x, cam_param.Pos.y, cam_param.Pos.z);

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
  {
    FLD_CAM_MV_PARAM_CHK chk;
    chk.Shift = TRUE;
    chk.Pitch = TRUE;
    chk.Yaw = TRUE;
    chk.Dist = TRUE;
    chk.Fovy = TRUE;
    chk.Pos = TRUE;
    FIELD_CAMERA_RecvLinerParam(camera, &chk, frame);
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * デフォルトカメラへの復帰移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_DefaultCamMove( VMHANDLE *core, void *wk )
{
  u16 frame;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  frame = VMGetU16( core );

  FIELD_CAMERA_RecvLinerParamDefault( camera, frame);

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
 * カメラ揺れ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCamera_Shake( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;

  u16 width = SCRCMD_GetVMWorkValue( core, work );
  u16 height = SCRCMD_GetVMWorkValue( core, work );
  u16 rad = SCRCMD_GetVMWorkValue( core, work );
  u16 time = SCRCMD_GetVMWorkValue( core, work );

  //カメラ揺らすイベントをコール
  {
    GMEVENT *call_event;
    CAM_SHAKE_WORK *wk;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
    int size = sizeof(CAM_SHAKE_WORK);
    call_event = GMEVENT_Create( gsys, NULL, CameraShakeEvt, size );
    wk = GMEVENT_GetEventWork(call_event);

    wk->Width = width;
    wk->Height = height;
    wk->SpRad = rad;
    wk->Time = time;
    wk->NowRad = 0;

    SCRIPT_CallEvent( sc, call_event );
  }

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}


#if 0
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
#endif
//--------------------------------------------------------------
/**
 * カメラトレース処理停止待ちイベント
 * @note    トレース終了時に、復帰用パラメータをセットする
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   work　　　　      ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WaitTraceStopForCamMvEvt( GMEVENT* event, int* seq, void* work )
{
  BOOL rc;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );
  
  rc = FIELD_CAMERA_CheckTrace(camera);

  if (!rc){
    //復帰パラメータをセットする
    FIELD_CAMERA_SetRecvCamParam(camera);
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

//--------------------------------------------------------------
/**
 * カメラ揺れイベント
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   work　　　　      ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CameraShakeEvt( GMEVENT* event, int* seq, void* work )
{

  FIELD_CAMERA * camera;
  BOOL end = FALSE;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  CAM_SHAKE_WORK *wk = GMEVENT_GetEventWork(event);

  camera = FIELDMAP_GetFieldCamera( fieldmap );
  switch(*seq){
  case 0:
    //トレース終了
    FIELD_CAMERA_StopTraceRequest(camera);
    (*seq)++;
    break;
  case 1:
    if ( !FIELD_CAMERA_CheckTrace(camera) )
    {
      //カメラのターゲットを取得
      wk->WatchTarget = FIELD_CAMERA_GetWatchTarget(camera);
      //バインド解除
      FIELD_CAMERA_FreeTarget(camera);
      
      (*seq)++;
    }
    break;
  case 2:
    //ターゲットを動かす
    wk->NowRad += wk->SpRad;
    if (wk->NowRad >= 0x10000)
    {
      wk->Time--;
      wk->NowRad -= 0x10000;
    }
    if (wk->Time == 0)
    {
      wk->NowRad = 0;
      end = TRUE;
    }

    {
      fx32 w, h;
      VecFx32 target;
      w = wk->Width * FX_SinIdx(wk->NowRad);
      h = wk->Height * FX_SinIdx(wk->NowRad);
      target = *wk->WatchTarget;
      target.x += w;
      target.y += h;
      //座標セット
      FIELD_CAMERA_SetTargetPos( camera, &target );
    }

    if (end)
    {
      //際バインド
      FIELD_CAMERA_BindTarget(camera, wk->WatchTarget);
      //トレース再開
      FIELD_CAMERA_RestartTrace(camera);
      (*seq)++;
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;

}

#ifdef PM_DEBUG

void DEBUG_CreateCamShakeEvt(GAMESYS_WORK *gsys, u16 width, u16 height, u16 rad, u16 time)
{
  GMEVENT *event;
  //カメラ揺らすイベントをコール
  {
    CAM_SHAKE_WORK *wk;
    int size = sizeof(CAM_SHAKE_WORK);
    event = GMEVENT_Create( gsys, NULL, CameraShakeEvt, size );
    wk = GMEVENT_GetEventWork(event);

    wk->Width = width;
    wk->Height = height;
    wk->SpRad = rad;
    wk->Time = time;
    wk->NowRad = 0;
  }

  GAMESYSTEM_SetEvent(gsys, event);
}
#endif

