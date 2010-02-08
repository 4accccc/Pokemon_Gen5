///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  タマゴ孵化イベント
 * @file   event_egg_birth.c
 * @author obata
 * @date   2010.02.05
 */
///////////////////////////////////////////////////////////////////////////////////////
#include "event_egg_birth.h"
#include "demo/egg_demo.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "event_field_fade.h"
#include "event_fieldmap_control.h"


//=====================================================================================
// ■イベントワーク
//=====================================================================================
typedef struct 
{
	GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
	FIELDMAP_WORK* fieldmap;
  
  POKEMON_PARAM* egg;        // 孵化させるタマゴ
  EGG_DEMO_PARAM demoParam;  // 孵化デモ パラメータ

} EVENT_WORK;


//-------------------------------------------------------------------------------------
/**
 * @brief タマゴ孵化イベント
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT EggBirthEvent( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
  GAMEDATA*      gameData   = work->gameData;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;

  // イベント シーケンス
  enum {
    SEQ_EGG_CHECK,          // タマゴチェック
    SEQ_INIT_DEMO_PARAM,    // デモ準備
    SEQ_FADE_OUT,           // フェードアウト
    SEQ_FIELDMAP_CLOSE,     // フィールドマップ終了
    SEQ_CALL_DEMO,          // デモ画面呼び出し
    SEQ_FIELDMAP_OPEN,      // フィールドマップ復帰
    SEQ_FADE_IN,            // フェードイン
    SEQ_END,                // イベント終了
  };

	switch( *seq ) 
  {
  // タマゴチェック
  case SEQ_EGG_CHECK:
    // タマゴじゃない or 不正タマゴ
    if( ( PP_Get( work->egg, ID_PARA_tamago_flag,       NULL ) == FALSE ) ||
        ( PP_Get( work->egg, ID_PARA_fusei_tamago_flag, NULL ) == TRUE  ) )
    {
      *seq = SEQ_END;
    }
    else
    {
      *seq = SEQ_INIT_DEMO_PARAM;
    }
    break;

  // デモ準備
  case SEQ_INIT_DEMO_PARAM:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(egg_demo) );
    EGG_DEMO_InitParam( &(work->demoParam), gameData, work->egg );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(egg_demo) );
    *seq = SEQ_FADE_OUT;
    break;

  // フェードアウト
	case SEQ_FADE_OUT: 
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeOut_Black( gameSystem, fieldmap,  FIELD_FADE_WAIT ) );
    *seq = SEQ_FIELDMAP_CLOSE;
		break;

  // フィールドマップ終了
	case SEQ_FIELDMAP_CLOSE: 
		GMEVENT_CallEvent( event, EVENT_FieldClose( gameSystem, fieldmap ) );
    *seq = SEQ_CALL_DEMO;
		break;

  // デモ画面呼び出し
  case SEQ_CALL_DEMO:
    GAMESYSTEM_CallProc( gameSystem, FS_OVERLAY_ID(egg_demo), &EGG_DEMO_ProcData, &(work->demoParam) );
    *seq = SEQ_FIELDMAP_OPEN;
    break;

  // フィールドマップ復帰
	case SEQ_FIELDMAP_OPEN:
		GMEVENT_CallEvent( event, EVENT_FieldOpen( gameSystem ) );
    *seq = SEQ_FADE_IN;
		break;

  // フェードイン
	case SEQ_FADE_IN:
    GMEVENT_CallEvent(event, 
        EVENT_FieldFadeIn_Black( gameSystem, fieldmap, FIELD_FADE_WAIT ) );
    *seq = SEQ_END;
		break;

  // イベント終了
	case SEQ_END: 
		return GMEVENT_RES_FINISH; 
	} 
	return GMEVENT_RES_CONTINUE;
} 

//-------------------------------------------------------------------------------------
/**
 * @brief タマゴ孵化イベント 生成
 *
 * @param gameSystem
 * @param fieldmap
 * @param egg        孵化させるタマゴ
 */
//-------------------------------------------------------------------------------------
GMEVENT* EVENT_EggBirth( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, POKEMON_PARAM* egg )
{
	GMEVENT* event;
	EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gameSystem, NULL, EggBirthEvent, sizeof(EVENT_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // 初期化
	work->gameSystem = gameSystem;
	work->fieldmap   = fieldmap;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem );
  work->egg        = egg;

	return event;
}
