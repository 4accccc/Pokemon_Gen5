///////////////////////////////////////////////////////////////////////////
/**
 * @brief  ゲーム内マニュアルの呼び出しイベント
 * @file   event_game_manual.c
 * @author obata
 * @date   2010.04.27
 */
///////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h" 
#include "gamesystem/game_event.h" 
#include "system/main.h"
#include "app/manual.h"
#include "fieldmap.h"
#include "script_def.h"
#include "event_fieldmap_control.h"
#include "event_game_manual.h"


//=========================================================================
// ■イベントワーク
//=========================================================================
typedef struct {

  GAMESYS_WORK*  gameSystem;
  FIELDMAP_WORK* fieldmap;
  MANUAL_PARAM*  manualParam;
  u16*           ret_wk;  // 終了結果の格納先

} EVENT_WORK;


//=========================================================================
// ■prototype
//=========================================================================
static GMEVENT_RESULT GameManualEvent( GMEVENT* event, int* seq, void* wk );
static void ReturnManualResult( EVENT_WORK* work ); // ゲームマニュアルの終了結果を返す


//=========================================================================
// ■public functions
//=========================================================================

//-------------------------------------------------------------------------
/**
 * @brief ゲーム内マニュアルの呼び出しイベントを生成する
 *
 * @param gameSystem
 * @param ret_wk     終了結果を格納するワークへのポインタ
 *
 * @return ゲーム内マニュアル呼び出しイベント
 */
//-------------------------------------------------------------------------
GMEVENT* EVENT_GameManual( GAMESYS_WORK* gameSystem, u16* ret_wk )
{
  GMEVENT* event;
  EVENT_WORK* work;
  GAMEDATA* gameData;
  FIELDMAP_WORK* fieldmap;

  gameData = GAMESYSTEM_GetGameData( gameSystem );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, GameManualEvent, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldmap = fieldmap;

  // マニュアル呼び出し準備
  work->manualParam = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(MANUAL_PARAM) );
  work->manualParam->gamedata = gameData;

  return event;
}


//=========================================================================
// ■private functions
//=========================================================================

//------------------------------------------------------------------------- 
/**
 * @brief ゲーム内マニュアル呼び出しイベント
 *
 * @param event
 * @param seq
 * @param wk
 */
//-------------------------------------------------------------------------
static GMEVENT_RESULT GameManualEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = wk;

  switch( *seq ) {
  case 0:
    // サブプロセス呼び出し
    GMEVENT_CallEvent( event,
        EVENT_FieldSubProc( work->gameSystem, work->fieldmap, 
          FS_OVERLAY_ID(manual), &MANUAL_ProcData, work->manualParam ) );
    (*seq)++;
    break;

  case 1:
    // 終了結果を格納
    ReturnManualResult( work );
    // 後片付け
    GFL_HEAP_FreeMemory( work->manualParam );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-------------------------------------------------------------------------
/**
 * @brief ゲームマニュアルの終了結果を返す
 *
 * @param work
 */
//-------------------------------------------------------------------------
static void ReturnManualResult( EVENT_WORK* work )
{
  if( work->manualParam->result == MANUAL_RESULT_CLOSE ) {
    *(work->ret_wk) = SCR_MANUAL_RESULT_CLOSE;
  }
  else {
    *(work->ret_wk) = SCR_MANUAL_RESULT_RETURN;
  }
}
