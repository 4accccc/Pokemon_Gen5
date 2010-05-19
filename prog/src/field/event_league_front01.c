////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ポケモンリーグフロントでのイベント
 * @file   event_league_front01.c
 * @author obata
 * @date   2010.04.08
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "fieldmap.h"
#include "field_gimmick_league_front01.h"

#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME

#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "arc/league_front.naix"  // for NARC_xxxx
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut
#include "field_gimmick_league_front02.h"  // for EVENT_LEAGUE_FRONT_02_GIMMICK_GetLiftDownEvent
#include "field/eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx
#include "sound/pm_sndsys.h" // for PMSND_xxxx
#include "event_league_front01.h"
#include "event_league_front02.h"
#include "field_task_manager.h"  // for FIELD_TASK_MAN
#include "field_camera_anime.h"


//==========================================================================================
// ■定数
//==========================================================================================
// 四天王ライトがリフトと一緒に降下するかどうか
//#define LIGHT_DOWN
// リフトアップ時のパラメータ
#define LIFT_UP_FRAME  (20)
#define LIFT_UP_PITCH  (0x2eff)
#define LIFT_UP_LENGTH (0x0115 << FX32_SHIFT)


static const FCAM_PARAM endParam = 
{
  0x1d7d, 0x0000, 0x016c << FX32_SHIFT,
  { 0x001f8 << FX32_SHIFT, 0xfffc0 << FX32_SHIFT, 0x00264 << FX32_SHIFT },
  { 0x00000 << FX32_SHIFT, 0x0009d << FX32_SHIFT, 0x00090 << FX32_SHIFT }
};


//==========================================================================================
// ■ イベントワーク
//==========================================================================================
typedef struct {

  GAMESYS_WORK*   gameSystem;
  FIELDMAP_WORK*  fieldmap;
  FIELD_CAMERA*   camera;
  FIELD_TASK_MAN* taskMan;
  ICA_ANIME*      liftAnime;  // リフトの移動アニメデータ
  FCAM_ANIME_DATA FCamAnimeData;
  FCAM_ANIME_WORK* FCamAnimeWork;

} EVENT_WORK;


//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベント処理
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT LiftDownEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work       = (EVENT_WORK*)wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  FIELD_CAMERA*   camera     = work->camera;
  FIELD_TASK_MAN* taskMan    = work->taskMan;

  switch( *seq ) {
  // カメラ移動開始
  case 0:
    FCAM_ANIME_SetupCamera( work->FCamAnimeWork );
    FCAM_PARAM_GetCurrentParam( work->camera, &work->FCamAnimeData.startParam );
    FCAM_ANIME_StartAnime( work->FCamAnimeWork );
    (*seq)++;
    break;

  // カメラ移動終了待ち
  case 1:
    if( FIELD_TASK_MAN_IsAllTaskEnd( taskMan ) ) { 
      FCAM_ANIME_DeleteWork( work->FCamAnimeWork );
      (*seq)++; 
    }
    break;

  // リフト移動開始
  case 2:
    {
      HEAPID heapID;
      heapID = FIELDMAP_GetHeapID( fieldmap );
      work->liftAnime = ICA_ANIME_CreateAlloc( heapID, ARCID_LEAGUE_FRONT_GIMMICK, NARC_league_front_pl_ele_00_ica_bin );
    }
    PMSND_PlaySE( SEQ_SE_FLD_148 ); // エレベータ稼動音開始
    ++(*seq);
    break;

  // リフト移動終了待ち
  case 3:
    // 自機, リフトの座標を更新
    {
      VecFx32 trans, pos;
      FIELD_PLAYER* player;
      GFL_G3D_OBJSTATUS* objstatus;
      FLD_EXP_OBJ_CNT_PTR exobj_cnt;
      // リフト
      ICA_ANIME_GetTranslate( work->liftAnime, &trans );
      exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIFT );
      objstatus->trans.y = trans.y;
#ifdef LIGHT_DOWN
      // ライト
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_FIGHT );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_EVIL );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_GHOST );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_ESPER );
      objstatus->trans.y = trans.y;
#endif
      // 自機
      player = FIELDMAP_GetFieldPlayer( fieldmap );
      FIELD_PLAYER_GetPos( player, &pos );
      pos.y = trans.y;
      FIELD_PLAYER_SetPos( player, &pos );
    }

    // アニメーション更新
    if( ICA_ANIME_IncAnimeFrame( work->liftAnime, FX32_ONE ) ) { 
      ICA_ANIME_Delete( work->liftAnime );
      ++(*seq); 
    }
    break;

  // フェードアウト
  case 4:
    {
      GMEVENT* new_event;
      new_event = EVENT_FieldFadeOut_Black( gameSystem, fieldmap, FIELD_FADE_WAIT );
      GMEVENT_CallEvent( event, new_event );
    }
    ++(*seq);
    break;

  // マップチェンジ
  case 5:
    {
      GMEVENT* new_event;
      VecFx32 pos;
      VEC_Set( &pos, 0, 0, 0 );
      new_event = EVENT_ChangeMapPosNoFade( gameSystem, fieldmap, ZONE_ID_C09P02, &pos, DIR_DOWN );
      GMEVENT_CallEvent( event, new_event );
    }
    ++(*seq);
    break;

  // 次のイベントへ切り替え
  case 6:
    {
      GMEVENT* next_event;
      next_event = EVENT_LFRONT02_LiftDown( gameSystem, fieldmap );
      GMEVENT_ChangeEvent( event, next_event );
    }
    ++(*seq);
    break;
  // 終了
  case 7:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを生成する
 * 
 * @param gameSystem
 * @param fieldmap
 *
 * @return リフト降下イベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_LFRONT01_LiftDown( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* evwork;

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, LiftDownEvent, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  evwork = GMEVENT_GetEventWork( event );
  evwork->gameSystem = gameSystem;
  evwork->fieldmap   = fieldmap;
  evwork->liftAnime  = NULL;
  evwork->camera     = FIELDMAP_GetFieldCamera( fieldmap );
  evwork->taskMan    = FIELDMAP_GetTaskManager( fieldmap );

  evwork->FCamAnimeData.frame = 20;
  evwork->FCamAnimeData.endParam = endParam;
  evwork->FCamAnimeData.targetBindOffFlag = TRUE;
  evwork->FCamAnimeData.cameraAreaOffFlag = TRUE;
  evwork->FCamAnimeWork = FCAM_ANIME_CreateWork( fieldmap );
  FCAM_ANIME_SetAnimeData( evwork->FCamAnimeWork, &evwork->FCamAnimeData );

  return event;
}
