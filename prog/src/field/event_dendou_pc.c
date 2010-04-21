///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 殿堂入り確認画面呼び出しイベント
 * @file  event_dendou_pc.c
 * @author obata
 * @date   2010.03.30
 */
/////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_dendou_pc.h"
#include "app/dendou_pc.h"
#include "event_fieldmap_control.h" // for EVENT_FieldSubProc
#include "system/main.h" // for HEAPID_PROC


//=====================================================================================
// ■index
//=====================================================================================
static GMEVENT_RESULT DendouCallEvent( GMEVENT* event, int* seq, void* wk );


//=====================================================================================
// ■イベントワーク
//===================================================================================== 
typedef struct {
	GAMESYS_WORK*  gameSystem;
	FIELDMAP_WORK* fieldmap;
  DENDOUPC_PARAM dendouParam;
  u16*           ret_wk; // 終了モードを格納するワーク
} EVENT_WORK; 


//=====================================================================================
// ■public func
//===================================================================================== 
//-------------------------------------------------------------------------------------
/**
 * @brief 殿堂入り確認画面呼び出しイベントを生成する
 *
 * @param gameSystem
 * @param call_mode 呼び出しモード
 * @param ret_wk    終了モードを格納するワーク
 */
//-------------------------------------------------------------------------------------
GMEVENT* EVENT_DendouCall( GAMESYS_WORK* gameSystem, u16 call_mode, u16* ret_wk )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベント生成
  event = GMEVENT_Create( 
      gameSystem, NULL, DendouCallEvent, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->ret_wk     = ret_wk;

  // プロックパラメータを設定
  work->dendouParam.gamedata = GAMESYSTEM_GetGameData( gameSystem );
  work->dendouParam.callMode = call_mode;

  // 作成したイベントを返す
  return event;
}

//=====================================================================================
// ■private func
//===================================================================================== 
//-------------------------------------------------------------------------------------
/**
 * @brief 殿堂入り確認画面呼び出しイベント
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT DendouCallEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = wk;

  switch( *seq ) {
  case 0:
    GMEVENT_CallEvent( event, 
        EVENT_FieldSubProc( work->gameSystem, work->fieldmap, 
          FS_OVERLAY_ID(dendou_pc), &DENDOUPC_ProcData, &work->dendouParam ) );
    ++(*seq);
    break;

  case 1: 
    // 終了モードを返す
    switch( work->dendouParam.retMode ) {
    case DENDOUPC_RET_NORMAL: *(work->ret_wk) = 0; break;
    case DENDOUPC_RET_CLOSE:  *(work->ret_wk) = 1; break;
    }
    OBATA_Printf( "ret_wk = %d\n", *(work->ret_wk) );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
