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
#include "field_bmanime_tool.h"

#include "event_mapchange.h"        //MAPCHANGE_setPlayerVanish
#include "event_fieldmap_control.h" //EVENT_FieldFadeOut
#include "event_fldmmdl_control.h"  //EVENT_PlayerOneStepAnime
#include "event_entrance_effect.h"

#include "event_camera_act.h" 
#include "field_bgm_control.h"


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

  //以下はシーケンス動作中にセットされる
  //FIELD_BMODEL * entry;
  //G3DMAPOBJST * obj;
  BMANIME_CONTROL_WORK * ctrl;
}FIELD_DOOR_ANIME_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorOut(GMEVENT * event, int *seq, void * work)
{
  enum {
    SEQ_DOOROUT_INIT = 0,
    SEQ_DOOROUT_OPENANIME_START,
    SEQ_DOOROUT_CAMERA_ACT,
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
    EVENT_CAMERA_ACT_PrepareForDoorOut( fieldmap );
    //自機を消す
    MAPCHANGE_setPlayerVanish( fieldmap, TRUE );
		GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, fieldmap, 0));
    *seq = SEQ_DOOROUT_OPENANIME_START;
    break;

  case SEQ_DOOROUT_OPENANIME_START:
    fdaw->ctrl = BMANIME_CTRL_Create( bmodel_man, &fdaw->pos );
    if (fdaw->ctrl)
    {
      BMANIME_CTRL_SetAnime( fdaw->ctrl, ANM_INDEX_DOOR_OPEN );
    }
    *seq = SEQ_DOOROUT_CAMERA_ACT;
    break;

  case SEQ_DOOROUT_CAMERA_ACT:
    EVENT_CAMERA_ACT_CallDoorOutEvent( event, gsys, fieldmap );
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
    if ( BMANIME_CTRL_WaitAnime( fdaw->ctrl ) == TRUE )
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
    { /* エラーよけ、ドアがない場合 */
      *seq = SEQ_DOOROUT_END;
    }
    else
    { //ドアを閉じるアニメ適用
      BMANIME_CTRL_SetAnime( fdaw->ctrl, ANM_INDEX_DOOR_CLOSE );
      *seq = SEQ_DOOROUT_CLOSEANIME_WAIT;
    }
    break;

  case SEQ_DOOROUT_CLOSEANIME_WAIT:
    if (BMANIME_CTRL_WaitAnime( fdaw->ctrl ) == TRUE )
    {
      *seq = SEQ_DOOROUT_END;
    }
    break;

  case SEQ_DOOROUT_END:
    BMANIME_CTRL_Delete( fdaw->ctrl );
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
GMEVENT * EVENT_FieldDoorOutAnime( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL,ExitEvent_DoorOut, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  fdaw = GMEVENT_GetEventWork( event );
  fdaw->gsys = gsys;
  //fdaw->loc_req = * loc;
  getPlayerFrontPos(fieldmap, &fdaw->pos);

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
    fdaw->ctrl = BMANIME_CTRL_Create( bmodel_man, &fdaw->pos );
    if (fdaw->ctrl)
    {
      BMANIME_CTRL_SetAnime( fdaw->ctrl, ANM_INDEX_DOOR_OPEN );
    }
    *seq = SEQ_DOORIN_CAMERA_ACT;
    break;

  case SEQ_DOORIN_CAMERA_ACT: 
    EVENT_CAMERA_ACT_CallDoorInEvent( event, gsys, fieldmap ); 
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
    if ( BMANIME_CTRL_WaitAnime( fdaw->ctrl ) == TRUE)
    {
      *seq = SEQ_DOORIN_PLAYER_ONESTEP;
    }
    break;

  case SEQ_DOORIN_PLAYER_ONESTEP:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    *seq = SEQ_DOORIN_FADEOUT;
    break;

  case SEQ_DOORIN_FADEOUT:
    FIELD_BGM_CONTROL_FadeOut( gamedata, fdaw->loc_req.zone_id, 30 );
    GMEVENT_CallEvent( event, EVENT_FieldFadeOut(gsys, fieldmap, 0) );
    *seq = SEQ_DOORIN_END;
    break;

  case SEQ_DOORIN_END:
    EVENT_CAMERA_ACT_ResetCameraParameter( fieldmap );  // カメラの設定をデフォルトに戻す
    BMANIME_CTRL_Delete( fdaw->ctrl );
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
( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, const LOCATION * loc )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL, ExitEvent_DoorIn, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  fdaw = GMEVENT_GetEventWork( event );
  fdaw->gsys = gsys;
  fdaw->loc_req = * loc;
  getPlayerFrontPos(fieldmap, &fdaw->pos);
  fdaw->ctrl = NULL;

  return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  u16 dir;

  FIELD_PLAYER_GetPos(player, pos);
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
    dir = MMDL_GetDirDisp( fmmdl );
  }
	switch( dir ) {
	case DIR_UP:		pos->z -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_DOWN:	pos->z += FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_LEFT:	pos->x -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_RIGHT:	pos->x += FIELD_CONST_GRID_FX32_SIZE; break;
	default:
                  GF_ASSERT(0);
  }
}


