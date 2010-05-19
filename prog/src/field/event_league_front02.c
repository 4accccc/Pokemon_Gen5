////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ポケモンリーグフロントでのイベント
 * @file   event_league_front02.c
 * @author obata
 * @date   2010.04.09
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "fieldmap.h"
#include "gmk_tmp_wk.h"
#include "field_gimmick_league_front01.h"
#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P01
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "arc/league_front.naix"  // for NARC_xxxx
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut
#include "field_gimmick_league_front02.h"  // for EVENT_LEAGUE_FRONT_02_GIMMICK_GetLiftDownEvent
#include "field/eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx
#include "sound/pm_sndsys.h" // for PMSND_xxxx
#include "event_camera_shake.h"
#include "event_league_front02.h"


//==========================================================================================
// ■各オブジェの配置
//========================================================================================== 
#define HALF_GRID ((FIELD_CONST_GRID_SIZE / 2) << FX32_SHIFT)
#define PLAYER_OFS_X (0)
#define PLAYER_OFS_Y (0)
#define PLAYER_OFS_Z ((FIELD_CONST_GRID_SIZE * 2) << FX32_SHIFT)


//==========================================================================================
// ■ イベント
//==========================================================================================
typedef struct {

  HEAPID         heap_id;
  GAMESYS_WORK*  gsys;
  FIELDMAP_WORK* fieldmap;
  FIELD_CAMERA*  camera;
  ICA_ANIME*     liftAnime;  // リフトの移動アニメデータ

} LIFTDOWN_EVENTWORK;

//------------------------------------------------------------------------------------------
/**
 * @brief リフトの移動アニメーションを更新する
 *
 * @return が終了したら TRUE, そうでなければ　FALSE
 */
//------------------------------------------------------------------------------------------
static BOOL UpdateLiftMove( LIFTDOWN_EVENTWORK* work )
{
  BOOL anime_end;

  // アニメーションを進める
  anime_end = ICA_ANIME_IncAnimeFrame( work->liftAnime, FX32_ONE );

  return anime_end;
}

//------------------------------------------------------------------------------------------
/**
 * @brief  リフトの座標を更新する
 */
//------------------------------------------------------------------------------------------
static void UpdateLiftPos( LIFTDOWN_EVENTWORK* work )
{ 
  VecFx32 trans;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  GFL_G3D_OBJSTATUS* objstatus;

  ICA_ANIME_GetTranslate( work->liftAnime, &trans );
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT );
  objstatus->trans.y = trans.y; 
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の座標をリフトの動きに合わせる
 */
//------------------------------------------------------------------------------------------
static void SetPlayerOnLift( LIFTDOWN_EVENTWORK* work )
{
  FIELD_PLAYER* player;
  VecFx32 pos, trans;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  GFL_G3D_OBJSTATUS* objstatus;
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT );
  player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  VEC_Set( &pos, 
           objstatus->trans.x + PLAYER_OFS_X, 
           objstatus->trans.y + PLAYER_OFS_Y, 
           objstatus->trans.z + PLAYER_OFS_Z );
  // x, z座標がグリッドの中心になるように調整
  pos.x = GRID_TO_FX32( FX32_TO_GRID(pos.x) ) + HALF_GRID;
  pos.z = GRID_TO_FX32( FX32_TO_GRID(pos.z) ) + HALF_GRID;
  FIELD_PLAYER_SetPos( player, &pos );
}


//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベント処理
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT LiftDownEvent( GMEVENT* event, int* seq, void* wk )
{
  int i;
  LIFTDOWN_EVENTWORK* work = (LIFTDOWN_EVENTWORK*)wk;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );

  switch( *seq ) {
  case 0:
    // 移動アニメデータ読み込み
    work->liftAnime = ICA_ANIME_CreateAlloc( work->heap_id, 
        ARCID_LEAGUE_FRONT_GIMMICK, NARC_league_front_pl_ele_01_ica_bin );
    // リフトと自機の座標を初期化
    UpdateLiftPos( work );
    SetPlayerOnLift( work );
    // カメラのトレース処理停止リクエスト発行
    FIELD_CAMERA_StopTraceRequest( work->camera );
    (*seq)++;
    break;

  case 1:
    // カメラのトレース処理終了待ち
    if( FIELD_CAMERA_CheckTrace( work->camera ) == FALSE ) { (*seq)++; }
    break;

  case 2:
    // リフトのアニメーション開始
    LEAGUE_FRONT_02_GIMMICK_StartLiftAnime( work->fieldmap );
    // リフト稼動エフェクトを表示する
    LEAGUE_FRONT_02_GIMMICK_ShowLiftEffect( work->fieldmap );
    // フェードイン
    {
      GMEVENT* new_event;
      new_event = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap, FIELD_FADE_WAIT );
      GMEVENT_CallEvent( event, new_event );
    }
    (*seq)++;
    break;

  // アニメ終了待ち
  case 3:
    {
      BOOL anime_end;

      // リフトを動かす
      anime_end = UpdateLiftMove( work );

      if( !anime_end ) {
        UpdateLiftPos( work );   // リフトの位置を更新
        SetPlayerOnLift( work ); // 自機をリフトに合わせる
      } 

      // アニメ終了
      if( anime_end ) { 
        ICA_ANIME_Delete( work->liftAnime );
        PMSND_StopSE(); // エレベータ稼動音を停止
        PMSND_PlaySE( SEQ_SE_FLD_94 ); // エレベータ到着音
        // カメラ振動イベントをコール
        {
          CAM_SHAKE_PARAM shakeParam;
          shakeParam.Width           = 2;
          shakeParam.Height          = 0;
          shakeParam.Sync            = 3;
          shakeParam.Time            = 2;
          shakeParam.TimeMax         = 2;
          shakeParam.NowSync         = 0;
          shakeParam.SubW            = 0;
          shakeParam.SubH            = 1;
          shakeParam.SubStartTime    = 0;
          shakeParam.SubMargineCount = 0;
          shakeParam.SubMargine      = 2;
          GMEVENT_CallEvent( event, EVENT_ShakeCamera( work->gsys, &shakeParam ) );
        }
        (*seq)++;
      } 
    }
    break;
    
  // リフトのアニメ終了待ち
  case 4:
    // リフト本体とエフェクトのアニメが終了
    if( LEAGUE_FRONT_02_GIMMICK_CheckLiftAnimeEnd( work->fieldmap ) &&
        LEAGUE_FRONT_02_GIMMICK_CheckLiftEffectEnd( work->fieldmap ) ) {
      LEAGUE_FRONT_02_GIMMICK_StopLiftAnime( work->fieldmap ); // リフトのアニメを止める
      LEAGUE_FRONT_02_GIMMICK_HideLiftEffect( work->fieldmap ); // リフト稼動エフェクトを消す
      (*seq)++;
    }
    break;

  // 終了処理
  case 5:
    // 自機を着地させる
    {
      VecFx32 pos;
      fx32 height;
      FIELD_PLAYER* player;
      MMDL* mmdl;
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      mmdl   = FIELD_PLAYER_GetMMdl( player );
      FIELD_PLAYER_GetPos( player, &pos );
      MMDL_GetMapPosHeight( mmdl, &pos, &height );
      pos.y = height;
      FIELD_PLAYER_SetPos( player, &pos );
    }
    (*seq)++;
    break;

  case 6:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを作成する
 * 
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return リフト降下イベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_LFRONT02_LiftDown( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  LIFTDOWN_EVENTWORK* evwork;

  // 生成
  event = GMEVENT_Create( gsys, NULL, LiftDownEvent, sizeof(LIFTDOWN_EVENTWORK) );

  // 初期化
  evwork = GMEVENT_GetEventWork( event );
  evwork->heap_id   = FIELDMAP_GetHeapID( fieldmap );
  evwork->gsys      = gsys;
  evwork->fieldmap  = fieldmap;
  evwork->camera    = FIELDMAP_GetFieldCamera( fieldmap );
  evwork->liftAnime = NULL;
  return event;
}
