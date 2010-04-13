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
#include "eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx
#include "sound/pm_sndsys.h" // for PMSND_xxxx
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

  GAMESYS_WORK*  gsys;
  FIELDMAP_WORK* fieldmap;
  FIELD_CAMERA*  camera;
  ICA_ANIME*     liftAnime;  // リフトの移動アニメデータ

} LIFTDOWN_EVENTWORK;


//------------------------------------------------------------------------------------------
/**
 * @brief  リフトの座標を更新する
 * @return 移動が終了したら TRUE, そうでなければ　FALSE
 */
//------------------------------------------------------------------------------------------
static BOOL MoveLift( LIFTDOWN_EVENTWORK* work )
{
  BOOL anime_end;

  // アニメーションを進める
  anime_end = ICA_ANIME_IncAnimeFrame( work->liftAnime, FX32_ONE );

  // リフト・ライトの座標を更新
  if( !anime_end )
  {
    int obj_idx;
    VecFx32 trans;
    ICA_ANIME_GetTranslate( work->liftAnime, &trans );
    for( obj_idx=0; obj_idx<LF02_EXOBJ_NUM; obj_idx++ )
    {
      FLD_EXP_OBJ_CNT_PTR exobj_cnt;
      GFL_G3D_OBJSTATUS* objstatus;
      exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF02_EXUNIT_GIMMICK, obj_idx );
      objstatus->trans.y = trans.y; 
    }
  }
  return anime_end;
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
 * @brief リフトのitaアニメを開始する
 */
//------------------------------------------------------------------------------------------
static void StartLiftAnimation( LIFTDOWN_EVENTWORK* work )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  exobj_anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT, LIFT_ANM_TA );

  FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT, LIFT_ANM_TA, TRUE ); 
  FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフトのitaアニメを停止する
 */
//------------------------------------------------------------------------------------------
static void StopLiftAnimation( LIFTDOWN_EVENTWORK* work )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  exobj_anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT, LIFT_ANM_TA );

  FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベント処理
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT LiftDownEvent( GMEVENT* event, int* seq, void* wk )
{
  LIFTDOWN_EVENTWORK* work = (LIFTDOWN_EVENTWORK*)wk;

  switch( *seq ) {
  case 0:
    // アニメ開始
    StartLiftAnimation( work );
    { // アニメデータ読み込み
      HEAPID heap_id;
      heap_id = FIELDMAP_GetHeapID( work->fieldmap );
      work->liftAnime = ICA_ANIME_CreateAlloc( heap_id, ARCID_LEAGUE_FRONT_GIMMICK, 
                                               NARC_league_front_pl_ele_01_ica_bin );
    }
    // リフトと自機の座標を初期化
    MoveLift( work );
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
      anime_end = MoveLift( work );

      // 自機をリフトに合わせる
      if( !anime_end ){ SetPlayerOnLift( work ); }

      // アニメ終了判定
      if( anime_end ) { 
        PMSND_StopSE(); // エレベータ稼動音を停止
        PMSND_PlaySE( SEQ_SE_FLD_94 ); // エレベータ到着音
        StopLiftAnimation( work ); // リフトのアニメーションを停止
        ICA_ANIME_Delete( work->liftAnime );
        (*seq)++;
      } 
    }
    break;

  // 終了処理
  case 4:
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

  // 終了
  case 5:
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
  evwork->gsys      = gsys;
  evwork->fieldmap  = fieldmap;
  evwork->camera    = FIELDMAP_GetFieldCamera( fieldmap );
  evwork->liftAnime = NULL;
  return event;
}
