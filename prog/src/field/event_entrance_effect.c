//============================================================================================
/**
 * @file event_entrance_effect.c
 * @brief
 * @date  2009.06.13
 * @author  tamada GAME FREAK inc.
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_player.h"
#include "field_buildmodel.h"

#include "event_mapchange.h"        //MAPCHANGE_setPlayerVanish
#include "event_fieldmap_control.h" //EVENT_FieldFadeOut
#include "event_fldmmdl_control.h"  //EVENT_PlayerOneStepAnime
#include "event_entrance_effect.h"

#include "event_camera_act.h" 

#include "sound/pm_sndsys.h"      //PMSND_PlaySE
#include "field/field_sound.h"    // for FIELD_SOUND_ChangePlayZoneBGM

//============================================================================================
//============================================================================================
static GMEVENT_RESULT FieldDoorAnimeEvent(GMEVENT * event, int *seq, void * work);

static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos);


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  //以下はワーク生成時にセットされる
  GAMESYS_WORK * gsys;  //<<<ゲームシステムへのポインタ
  LOCATION loc_req;     //<<<次のマップを指すロケーション指定（BGMフェード処理の係数）
  VecFx32 pos;          //<<<ドアを検索する場所
  BOOL cam_anm_flag;    //<<<カメラアニメーションの有無

  //以下はシーケンス動作中にセットされる
  //FIELD_BMODEL * entry;
  //G3DMAPOBJST * obj;
  FIELD_BMODEL * ctrl;
}FIELD_DOOR_ANIME_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorOut(GMEVENT * event, int *seq, void * work)
{
  enum {
    SEQ_DOOROUT_INIT = 0,
    SEQ_DOOROUT_WAIT_SOUND_LOAD,
    SEQ_DOOROUT_OPENANIME_START,
    SEQ_DOOROUT_CAMERA_ACT,
    SEQ_DOOROUT_FADEIN,
    SEQ_DOOROUT_OPENANIME_WAIT,
    SEQ_DOOROUT_PLAYER_STEP,
    SEQ_DOOROUT_CLOSEANIME_START,
    SEQ_DOOROUT_CLOSEANIME_WAIT,
    SEQ_DOOROUT_END,
  };
	FIELD_DOOR_ANIME_WORK * fdaw = work;
	GAMESYS_WORK  * gsys = fdaw->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fieldmap);
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);

  switch (*seq)
  {
  case SEQ_DOOROUT_INIT:
    // フェードインする前に, カメラの初期設定を行う
    if( fdaw->cam_anm_flag )
    {
      EVENT_CAMERA_ACT_PrepareForDoorOut( fieldmap );
    }
    //自機を消す
    MAPCHANGE_setPlayerVanish( fieldmap, TRUE );
    *seq = SEQ_DOOROUT_WAIT_SOUND_LOAD;
    break;

  case SEQ_DOOROUT_WAIT_SOUND_LOAD:
    if( PMSND_IsLoading() != TRUE )
    {
      *seq = SEQ_DOOROUT_OPENANIME_START;
    }
    break;

  case SEQ_DOOROUT_OPENANIME_START:
    fdaw->ctrl = FIELD_BMODEL_Create_Search( bmodel_man, BM_SEARCH_ID_DOOR, &fdaw->pos );
    if (fdaw->ctrl)
    {
      u16 seNo;
      FIELD_BMODEL_StartAnime( fdaw->ctrl, BMANM_INDEX_DOOR_OPEN );
      if( FIELD_BMODEL_GetCurrentSENo( fdaw->ctrl, &seNo) )
      {
        PMSND_PlaySE( seNo );
      }
    }
    *seq = SEQ_DOOROUT_CAMERA_ACT;
    break;

  case SEQ_DOOROUT_CAMERA_ACT:
    if( fdaw->cam_anm_flag )
    {
      EVENT_CAMERA_ACT_CallDoorOutEvent( event, gsys, fieldmap );
    }
    *seq = SEQ_DOOROUT_FADEIN;
    break;

  case SEQ_DOOROUT_FADEIN:
    { // フェードイン
      GMEVENT* fade_event;
      FIELD_STATUS* fstatus; 
      fstatus = GAMEDATA_GetFieldStatus( gamedata );
      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
      { // 季節フェード
        fade_event = EVENT_FieldFadeIn_Season(gsys, fieldmap);
      }
      else
      { // クロスフェード
        fade_event = EVENT_FieldFadeIn_Cross(gsys, fieldmap);
      }
      GMEVENT_CallEvent( event, fade_event );
    }
    if (fdaw->ctrl == NULL)
    { /* エラーよけ、ドアがない場合 */
      *seq = SEQ_DOOROUT_PLAYER_STEP;
    } 
    else
    {
      *seq = SEQ_DOOROUT_OPENANIME_WAIT;
    }
    break;

  case SEQ_DOOROUT_OPENANIME_WAIT:
    if ( FIELD_BMODEL_WaitCurrentAnime( fdaw->ctrl ) == TRUE
        && FIELD_BMODEL_CheckCurrentSE( fdaw->ctrl ) == FALSE )
    {
      *seq = SEQ_DOOROUT_PLAYER_STEP;
    }
    break;

  case SEQ_DOOROUT_PLAYER_STEP:
    EVENT_CAMERA_ACT_ResetCameraParameter( fieldmap );  // カメラの設定をデフォルトに戻す
    //自機出現、一歩移動アニメ
    MAPCHANGE_setPlayerVanish( fieldmap, FALSE );
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    *seq = SEQ_DOOROUT_CLOSEANIME_START;
    break;

  case SEQ_DOOROUT_CLOSEANIME_START:
    if (fdaw->ctrl == NULL)
    { /* エラーよけ、ドアがない場合 */ *seq = SEQ_DOOROUT_END;
    }
    else
    { //ドアを閉じるアニメ適用
      u16 seNo;
      FIELD_BMODEL_StartAnime( fdaw->ctrl, BMANM_INDEX_DOOR_CLOSE );
      if( FIELD_BMODEL_GetCurrentSENo( fdaw->ctrl, &seNo) )
      {
        PMSND_PlaySE( seNo );
      }
      *seq = SEQ_DOOROUT_CLOSEANIME_WAIT;
    }
    break;

  case SEQ_DOOROUT_CLOSEANIME_WAIT:
    if ( FIELD_BMODEL_WaitCurrentAnime( fdaw->ctrl ) == TRUE
        && FIELD_BMODEL_CheckCurrentSE( fdaw->ctrl ) == FALSE )
    {
      *seq = SEQ_DOOROUT_END;
    }
    break;

  case SEQ_DOOROUT_END:
    FIELD_BMODEL_Delete( fdaw->ctrl );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief ドアからでてくる一連の演出イベント
 * @param gsys
 * @param fieldmap
 * @param loc
 * @return  GMEVENT
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorOutAnime( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, BOOL cam_anm_flag )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL,ExitEvent_DoorOut, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  fdaw = GMEVENT_GetEventWork( event );
  fdaw->gsys = gsys;
  //fdaw->loc_req = * loc;
  getPlayerFrontPos(fieldmap, &fdaw->pos);
  fdaw->cam_anm_flag = cam_anm_flag;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorIn(GMEVENT * event, int *seq, void * work)
{
  enum {
    SEQ_DOORIN_OPENANIME_START = 0,
    SEQ_DOORIN_CAMERA_ACT,
    SEQ_DOORIN_OPENANIME_WAIT,
    SEQ_DOORIN_PLAYER_ONESTEP,
    SEQ_DOORIN_FADEOUT,
    SEQ_DOORIN_END,
  };

	FIELD_DOOR_ANIME_WORK * fdaw = work;
	GAMESYS_WORK  * gsys = fdaw->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fieldmap);
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);

  switch (*seq)
  { 
  case SEQ_DOORIN_OPENANIME_START:
    fdaw->ctrl = FIELD_BMODEL_Create_Search( bmodel_man, BM_SEARCH_ID_DOOR, &fdaw->pos );
    if (fdaw->ctrl)
    {
      u16 seNo;
      FIELD_BMODEL_StartAnime( fdaw->ctrl, BMANM_INDEX_DOOR_OPEN );
      if( FIELD_BMODEL_GetCurrentSENo( fdaw->ctrl, &seNo) )
      {
        PMSND_PlaySE( seNo );
      }
    }
    *seq = SEQ_DOORIN_CAMERA_ACT;
    break;

  case SEQ_DOORIN_CAMERA_ACT: 
    if( fdaw->cam_anm_flag )
    {
      EVENT_CAMERA_ACT_CallDoorInEvent( event, gsys, fieldmap ); 
    }
    if (fdaw->ctrl == NULL)
    { /* エラーよけ、ドアがない場合 */
      *seq = SEQ_DOORIN_PLAYER_ONESTEP;
    } 
    else
    {
      *seq = SEQ_DOORIN_OPENANIME_WAIT;
    }
    break;

  case SEQ_DOORIN_OPENANIME_WAIT:
    if ( FIELD_BMODEL_WaitCurrentAnime( fdaw->ctrl ) == TRUE
        && FIELD_BMODEL_CheckCurrentSE( fdaw->ctrl ) == FALSE )
    {
      *seq = SEQ_DOORIN_PLAYER_ONESTEP;
    }
    break;

  case SEQ_DOORIN_PLAYER_ONESTEP:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    *seq = SEQ_DOORIN_FADEOUT;
    break;

  case SEQ_DOORIN_FADEOUT:
    { // BGM変更リクエスト
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      PLAYER_WORK* player = GAMEDATA_GetPlayerWork( gamedata, 0 );
      PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
      FIELD_SOUND_ChangePlayZoneBGM( fsnd, gamedata, form, fdaw->loc_req.zone_id );
    }
    { // フェードアウト
      GMEVENT* fade_event;
      FIELD_STATUS* fstatus; 
      fstatus = GAMEDATA_GetFieldStatus( gamedata );
      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
      { // 輝度フェード
        fade_event = EVENT_FieldFadeOut_Black(gsys, fieldmap, FIELD_FADE_WAIT);
      }
      else
      { // クロスフェード
        fade_event = EVENT_FieldFadeOut_Cross(gsys, fieldmap);
      }
      GMEVENT_CallEvent( event, fade_event );
    }
    *seq = SEQ_DOORIN_END;
    break;

  case SEQ_DOORIN_END:
    if( fdaw->cam_anm_flag )
    {
      EVENT_CAMERA_ACT_ResetCameraParameter( fieldmap );  // カメラの設定をデフォルトに戻す
    }
    FIELD_BMODEL_Delete( fdaw->ctrl );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief ドアに入る一連の演出イベント
 * @param gsys
 * @param fieldmap
 * @param loc
 * @return  GMEVENT
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorInAnime
( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, const LOCATION * loc, BOOL cam_anm_flag )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL, ExitEvent_DoorIn, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  fdaw = GMEVENT_GetEventWork( event );
  fdaw->gsys = gsys;
  fdaw->loc_req = * loc;
  getPlayerFrontPos(fieldmap, &fdaw->pos);
  fdaw->ctrl = NULL;
  fdaw->cam_anm_flag = cam_anm_flag;

  return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
  u16 dir;

  dir = MMDL_GetDirDisp( fmmdl );

  FIELD_PLAYER_GetDirPos( player, dir, pos );
#if 0

  FIELD_PLAYER_GetPos(player, pos);
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
  }
	switch( dir ) {
	case DIR_UP:		pos->z -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_DOWN:	pos->z += FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_LEFT:	pos->x -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_RIGHT:	pos->x += FIELD_CONST_GRID_FX32_SIZE; break;
	default:
                  GF_ASSERT(0);
  }
#endif
}


