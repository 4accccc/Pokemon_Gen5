//======================================================================
/**
 * @file  fskill_sorawotobu.c
 * @brief  そらをとぶ
 * @author  Saito
 * @date  2009.11.19
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"

#include "system/main.h"  //HEAPID_PROC
#include "fieldmap.h"
//#include "weather.h"

#include "field_skill.h"
#include "eventwork.h"

//#include "script.h"
#include "fskill_sorawotobu.h"
//#include "fldmmdl.h"

#include "fld3d_ci.h"
#include "event_mapchange.h"
#include "event_fieldmap_control.h"
#include "field/zonedata.h"    //for ZONEDATA_～

#include "../../../resource/fld3d_ci/fldci_id_def.h"

#define FLYSKY_CAM_MOVE_FRAME (10)

typedef struct SORAWOTOBU_WORK_tag{
  GAMESYS_WORK* gsys;
//  GAMEDATA* gdata;
 
  const VecFx32 *Watch;
  int ZoneID;
  VecFx32 Pos;
  int OutCutinNo;
  int InCutinNo;
}SORAWOTOBU_WORK;

static GMEVENT_RESULT FSkillSorawotobuEvent(GMEVENT * event, int * seq, void *work);

//------------------------------------------------------------------
/*
 *  @brief  そらをとぶイベント起動
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSkillSorawotobu(
    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, const int inZoneID)
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, FSkillSorawotobuEvent, sizeof(SORAWOTOBU_WORK));
	SORAWOTOBU_WORK * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(SORAWOTOBU_WORK));
  wk->gsys = gsys;
//  wk->gdata = GAMESYSTEM_GetGameData(gsys);

  wk->ZoneID = inZoneID;

  //性別でカットインナンバー分岐
  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    if ( MyStatus_GetMySex( GAMEDATA_GetMyStatus( gdata ) ) == PTL_SEX_MALE )
    {
      wk->OutCutinNo = FLDCIID_FLY_OUT;
      wk->InCutinNo = FLDCIID_FLY_IN;
    }
    else
    {
      wk->OutCutinNo = FLDCIID_FLY_OUT;
      wk->InCutinNo = FLDCIID_FLY_IN;
    }
  }

  if( ZONEDATA_IsRailMap(inZoneID) )
  {
    // レール用初期化
    ZONEDATA_GetStartRailPos(inZoneID, &wk->Pos);
  }
  else
  {
    // グリッド用初期化
    ZONEDATA_GetStartPos(inZoneID, &wk->Pos);
  }

	return event;
}

static GMEVENT_RESULT FSkillSorawotobuEvent(GMEVENT * event, int * seq, void *work)
{
	SORAWOTOBU_WORK * wk = work;
  FIELDMAP_WORK *fieldWork;
  FLD3D_CI_PTR ciPtr;
  FIELD_CAMERA * camera;
  FLDNOGRID_MAPPER* mapper;

  fieldWork = GAMESYSTEM_GetFieldMapWork(wk->gsys);
  camera = FIELDMAP_GetFieldCamera(fieldWork);
  ciPtr = FIELDMAP_GetFld3dCiPtr(fieldWork);
  mapper = FIELDMAP_GetFldNoGridMapper(fieldWork);

  switch(*seq){
  case 0:
    //カメラエリア動作をフック
    FIELD_CAMERA_SetCameraAreaActive( camera, FALSE );
    //現在ウォッチターゲットを保存
    wk->Watch = FIELD_CAMERA_GetWatchTarget(camera);
    FIELD_CAMERA_StopTraceRequest(camera);
    (*seq)++;
    break;
  case 1:
    if ( (wk->Watch != NULL) && FIELD_CAMERA_CheckTrace(camera) ){
      break;
    }
    //レールシステム更新ストップ
    FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    //カメラパージ
    FIELD_CAMERA_FreeTarget(camera);
    //現在のカメラ設定を保存
    FIELD_CAMERA_SetRecvCamParam(camera);
    //カメラを空飛びカメラへ線形移動リクエスト
    {
      FLD_CAM_MV_PARAM param;
      VecFx32 player_vec;
      FIELD_PLAYER * player;
      player = FIELDMAP_GetFieldPlayer(fieldWork);
      //主人公の3Ｄ座標を取得
      FIELD_PLAYER_GetPos( player, &player_vec );
      param.Core.AnglePitch = 9688;
      param.Core.AngleYaw = 0;
      param.Core.Distance = 970752;
      param.Core.Shift.x = 0;
      param.Core.Shift.y = 16384;
      param.Core.Shift.z = 0;
      param.Core.TrgtPos = player_vec;
      param.Core.Fovy = 3640;
      param.Chk.Shift = TRUE;
      param.Chk.Pitch = TRUE;
      param.Chk.Yaw = FALSE;
      param.Chk.Dist = TRUE;
      param.Chk.Fovy = TRUE;
      param.Chk.Pos = TRUE;
      FIELD_CAMERA_SetLinerParam(camera, &param, FLYSKY_CAM_MOVE_FRAME);
    }
    (*seq)++;
    /*none break*/
  case 2:
    //カメラ移動待ち
    if ( !FIELD_CAMERA_CheckMvFunc(camera) )
    {
      GMEVENT *child;
      //カットイン演出開始
      child = FLD3D_CI_CreateCutInEvt(wk->gsys, ciPtr, wk->OutCutinNo);
      GMEVENT_CallEvent(event, child);
      (*seq)++;
    }
    break;
  case 3:
    //自機非表示
    {
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish(mmdl, TRUE);
    }
    //カメラ復帰パラメータクリア（復帰しないでマップ遷移するので）
    FIELD_CAMERA_ClearRecvCamParam(camera);
    //ブラックアウトリクエスト
    GMEVENT_CallEvent(event, EVENT_FlySkyBrightOut(wk->gsys, fieldWork, FIELD_FADE_BLACK, FIELD_FADE_WAIT));
    (*seq)++;
    break;
  case 4:
    //ノーフェードマップチェンジ
    {
      GMEVENT *child; 
      child = EVENT_ChangeMapSorawotobu(wk->gsys, fieldWork,
          wk->ZoneID, &wk->Pos, DIR_DOWN );
      GMEVENT_CallEvent(event, child);
      (*seq)++;
    }
    break;
  case 5:
    //地名表示オフ
    {
       FIELD_PLACE_NAME * p_sys = FIELDMAP_GetPlaceNameSys( fieldWork );
       FIELD_PLACE_NAME_Hide( p_sys );
    }
    //カメラエリア動作をフック
    FIELD_CAMERA_SetCameraAreaActive( camera, FALSE );
    //現在ウォッチターゲットを保存
    wk->Watch = FIELD_CAMERA_GetWatchTarget(camera);
    FIELD_CAMERA_StopTraceRequest(camera);
    (*seq)++;
    break;
  case 6:
    if ( (wk->Watch != NULL) && FIELD_CAMERA_CheckTrace(camera) ){
      break;
    }
    //レールシステム更新ストップ
    FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    //カメラパージ
    FIELD_CAMERA_FreeTarget(camera);
    //現在のカメラ設定を保存
    FIELD_CAMERA_SetRecvCamParam(camera);
    //カメラを空飛びカメラへ線形移動リクエスト
    {
      FLD_CAM_MV_PARAM param;
      VecFx32 player_vec;
      FIELD_PLAYER * player;
      player = FIELDMAP_GetFieldPlayer(fieldWork);
      //主人公の3Ｄ座標を取得
      FIELD_PLAYER_GetPos( player, &player_vec );
      param.Core.AnglePitch = 9688;
      param.Core.AngleYaw = 0;
      param.Core.Distance = 970752;
      param.Core.Shift.x = 0;
      param.Core.Shift.y = 16384;
      param.Core.Shift.z = 0;
      param.Core.TrgtPos = player_vec;
      param.Core.Fovy = 3640;
      param.Chk.Shift = TRUE;
      param.Chk.Pitch = TRUE;
      param.Chk.Yaw = FALSE;
      param.Chk.Dist = TRUE;
      param.Chk.Fovy = TRUE;
      param.Chk.Pos = TRUE;
      FIELD_CAMERA_SetLinerParam(camera, &param, 1);
    }
    (*seq)++;
    /*none break*/
  case 7:
    //カメラ移動待ち
    if ( !FIELD_CAMERA_CheckMvFunc(camera) )
    {
      //自機非表示
      {
        MMDL * mmdl;
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        mmdl = FIELD_PLAYER_GetMMdl( fld_player );
        MMDL_SetStatusBitVanish(mmdl, TRUE);
      }
      //ブラックインリクエスト
      GMEVENT_CallEvent(event, EVENT_FlySkyBrightIn(wk->gsys, fieldWork, FIELD_FADE_BLACK, FIELD_FADE_WAIT));
      (*seq)++;
    }
    break;
  case 8:
    {
      GMEVENT *child;
      //カットイン演出開始
      child = FLD3D_CI_CreateCutInEvt(wk->gsys, ciPtr, wk->InCutinNo);
      GMEVENT_CallEvent(event, child);
      (*seq)++;
    }
    break;
  case 9:
    //地名表示開始
    {
       FIELD_PLACE_NAME * p_sys = FIELDMAP_GetPlaceNameSys( fieldWork );
       FIELD_PLACE_NAME_DisplayForce( p_sys, wk->ZoneID );
    }

    {
      FLD_CAM_MV_PARAM_CHK chk;
      chk.Shift = TRUE;
      chk.Pitch = TRUE;
      chk.Yaw = FALSE;
      chk.Dist = TRUE;
      chk.Fovy = TRUE;
      chk.Pos = TRUE;
      //カメラ戻し
      FIELD_CAMERA_RecvLinerParam(camera, &chk, FLYSKY_CAM_MOVE_FRAME);
    }
    (*seq)++;
    /*none break*/
  case 10:
    //カメラ戻し待ち
    if ( !FIELD_CAMERA_CheckMvFunc(camera) ){
      //カメラバインド
      if (wk->Watch != NULL){
        FIELD_CAMERA_BindTarget(camera, wk->Watch);
      }
      FIELD_CAMERA_RestartTrace(camera);
      //レールシステム更新開始
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, TRUE );
      //復帰データのクリア
      FIELD_CAMERA_ClearRecvCamParam(camera);
      //カメラエリア動作をフック解除
      FIELD_CAMERA_SetCameraAreaActive( camera, TRUE );
      //ＯＢＪ停止解除
      ;
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

