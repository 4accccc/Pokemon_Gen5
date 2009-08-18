/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フェードアウトイベントの作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////

#include "gamesystem/game_event.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/field_sound.h"      // FIELD_SOUND_GetFieldBGMNo
#include "field/location.h"

#include "event_entrance_out.h"

#include "event_fieldmap_control.h" // EVENT_FieldFadeOut
#include "field_door_anime.h"       // EVENT_FieldDoorInAnime
#include "event_fldmmdl_control.h"  // EVENT_PlayerOneStepAnime


//=======================================================================================
/**
 * @brief イベント・ワーク
 */
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*    gsys;
  GAMEDATA*        gdata;
  FIELD_MAIN_WORK* fieldmap;
  LOCATION         location;  // 遷移先指定
}
FIELD_FADE_WORK;


//=======================================================================================
/**
 * @breif 非公開関数のプロトタイプ宣言
 */
//======================================================================================= 
static void setNextBGM(GAMEDATA * gamedata, u16 zone_id);

// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeNone(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeDoor(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeStep(GMEVENT * event, int *seq, void * work);


//=======================================================================================
/**
 * @breif 公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif フェードアウト・イベントを作成する
 *
 * @param parent    親イベント
 * @param gsys      ゲームシステム
 * @param gdata     ゲームデータ
 * @param fieldmap  フィールドマップ
 * @param location  遷移先指定
 * @param exit_type 遷移タイプ指定
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_SwitchFieldFadeOut( GMEVENT* parent, 
                                   GAMESYS_WORK* gsys,
                                   GAMEDATA* gdata, 
                                   FIELD_MAIN_WORK* fieldmap, 
                                   LOCATION location, 
                                   EXIT_TYPE exit_type )
{
  GMEVENT* event;
  FIELD_FADE_WORK* work;

  // イベントテーブル
  const GMEVENT_FUNC fadeoutEventTable[] = 
  {
    EVENT_FadeOut_ExitTypeNone,   //EXIT_TYPE_NONE
    EVENT_FadeOut_ExitTypeNone,   //EXIT_TYPE_MAT
    EVENT_FadeOut_ExitTypeStep,   //EXIT_TYPE_STAIRS
    EVENT_FadeOut_ExitTypeDoor,   //EXIT_TYPE_DOOR
    EVENT_FadeOut_ExitTypeStep,   //EXIT_TYPE_WALL
  };

  // イベント作成
  event = GMEVENT_Create( gsys, parent, fadeoutEventTable[ exit_type ], sizeof( FIELD_FADE_WORK ) );

  // イベント・ワークを初期化
  work           = (FIELD_FADE_WORK*)GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->gdata    = gdata;
  work->fieldmap = fieldmap;
  work->location = location;

  // 作成したイベントを返す
  return event;
}


//=======================================================================================
/**
 * @breif 非公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief BGM切り替え
 */
//---------------------------------------------------------------------------------------
static void setNextBGM(GAMEDATA * gamedata, u16 zone_id)
{
  //取り急ぎ。常にフェードインで始まる
#if 0
  u16 trackBit = 0xfcff;	// track 9,10 OFF
  u16 nextBGM = ZONEDATA_GetBGMID(zone_id, GAMEDATA_GetSeasonID(gamedata));
//  PMSND_PlayNextBGM_EX(nextBGM, trackBit, 30, 0);
  PMSND_PlayBGM_EX(nextBGM, 0xfcff );
#else
  PLAYER_WORK *player = GAMEDATA_GetPlayerWork( gamedata, 0 );
  PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
  u32 no = FIELD_SOUND_GetFieldBGMNo( gamedata, form, zone_id );
  OS_Printf("NEXT BGM NO=%d\n",no);
  //FIELD_SOUND_PlayBGM( no );		
  FIELD_SOUND_PlayNextBGM( no );
#endif
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時のフェードアウト・イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeNone(GMEVENT * event, int *seq, void * work)
{
	FIELD_FADE_WORK* fade_work = work;
	GAMESYS_WORK*    gsys      = fade_work->gsys;
	FIELD_MAIN_WORK* fieldmap  = fade_work->fieldmap;
	GAMEDATA*        gamedata  = fade_work->gdata;

  switch ( *seq )
  {
  case 0:
    setNextBGM( gamedata, fade_work->location.zone_id );
		GMEVENT_CallEvent( event, EVENT_FieldFadeOut(gsys, fieldmap, 0) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時のフェードアウト・イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeDoor(GMEVENT * event, int *seq, void * work)
{
	FIELD_FADE_WORK* fade_work = work;
	GAMESYS_WORK*    gsys      = fade_work->gsys;
	FIELD_MAIN_WORK* fieldmap  = fade_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &fade_work->location ) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 階段のフェードアウト・イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FadeOut_ExitTypeStep(GMEVENT * event, int *seq, void * work)
{
	FIELD_FADE_WORK* fade_work = work;
	GAMESYS_WORK*    gsys      = fade_work->gsys;
	FIELD_MAIN_WORK* fieldmap  = fade_work->fieldmap;
	GAMEDATA*        gamedata  = fade_work->gdata;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    ++ *seq;
    break;
  case 1:
    setNextBGM( gamedata, fade_work->location.zone_id );
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gsys, fieldmap, 0));
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
