///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー画面呼び出しイベント
 * @file   event_research_radar.c
 * @author obata
 * @date   2010.02.02
 */
///////////////////////////////////////////////////////////////////////////////////////
#include "event_research_radar.h"
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "app/research_radar.h"
#include "event_field_fade.h"
#include "event_fieldmap_control.h"

FS_EXTERN_OVERLAY( research_radar );


//=====================================================================================
// ■イベントワーク
//=====================================================================================
typedef struct 
{
	GAMESYS_WORK*  gameSystem;
	FIELDMAP_WORK* fieldmap;

  RESEARCH_PARAM* researchParam;  // 調査レーダー呼び出しパラメータ

} EVENT_WORK;


//-------------------------------------------------------------------------------------
/**
 * @brief 調査レーダー画面呼び出しイベント
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT ResearchRadarCallEvent( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;

  // イベント シーケンス
  enum {
    SEQ_FADE_OUT,           // フェードアウト
    SEQ_FIELDMAP_CLOSE,     // フィールドマップ終了
    SEQ_CALL_RESEARCH_PROC, // 調査レーダー画面呼び出し
    SEQ_FIELDMAP_OPEN,      // フィールドマップ復帰
    SEQ_FADE_IN,            // フェードイン
    SEQ_END,                // イベント終了
  };

	switch( *seq ) 
  {
  // フェードアウト
	case SEQ_FADE_OUT: 
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeOut_Black( gameSystem, fieldmap,  FIELD_FADE_WAIT ) );
    *seq = SEQ_FIELDMAP_CLOSE;
		break;

  // フィールドマップ終了
	case SEQ_FIELDMAP_CLOSE: 
		GMEVENT_CallEvent( event, EVENT_FieldClose( gameSystem, fieldmap ) );
    *seq = SEQ_CALL_RESEARCH_PROC;
		break;

  // 調査レーダー画面呼び出し
  case SEQ_CALL_RESEARCH_PROC:
    GAMESYSTEM_CallProc( gameSystem, FS_OVERLAY_ID(research_radar), &ResearchRadarProcData, work->researchParam );
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
    GFL_HEAP_FreeMemory( work->researchParam );
		return GMEVENT_RES_FINISH; 
	} 
	return GMEVENT_RES_CONTINUE;
} 


//-------------------------------------------------------------------------------------
/**
 * @brief 調査レーダー画面呼び出しイベント 生成
 *
 * @param gameSystem
 * @param fieldmap
 */
//-------------------------------------------------------------------------------------
GMEVENT* EVENT_ResearchRadar( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
	GMEVENT* event;
	EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gameSystem, NULL, ResearchRadarCallEvent, sizeof(EVENT_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // 初期化
	work->gameSystem    = gameSystem;
	work->fieldmap      = fieldmap;
  work->researchParam = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(RESEARCH_PARAM) );

  // 調査レーダー呼び出しパラメータ設定
  work->researchParam->gameSystem = gameSystem;

	return event;
}
