////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  電光掲示板を見るイベント
 * @file   event_look_elboard.c
 * @author obata
 * @date   2009.11.25
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_manager.h"
#include "field_gimmick_gate.h"
#include "event_look_elboard.h"


//==========================================================================================
// ■定数
//==========================================================================================
// イベントシーケンス
enum
{
  SEQ_INIT,               // イベント初期化
  SEQ_WAIT_CAMERA_SET,    // カメラの移動完了待ち
  SEQ_KEY_WAIT,           // キー入力待ち
  SEQ_WAIT_CAMERA_RESET,  // カメラリセット完了待ち
  SEQ_EXIT,               // イベント終了
};


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap; 
  u16 cameraMoveFrame;  // カメラ移動に要するフレーム数

} EVENT_WORK;


//==========================================================================================
// ■イベント処理関数
//==========================================================================================
static GMEVENT_RESULT SeasonDisplay( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // イベント初期化
  case SEQ_INIT:
    // カメラ移動タスクを登録
    GATE_GIMMICK_Camera_LookElboard( work->fieldmap, work->cameraMoveFrame );
    ++(*seq);
    break;
  // カメラの移動完了待ち
  case SEQ_WAIT_CAMERA_SET:
    { // 全タスク処理終了で次のシーケンスへ
      FIELD_TASK_MAN* man = FIELDMAP_GetTaskManager( work->fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) ){ ++(*seq); }
    }
    break;
  // キー入力待ち
  case SEQ_KEY_WAIT:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( (trg & PAD_KEY_LEFT)  ||
          (trg & PAD_KEY_RIGHT) ||
          (trg & PAD_KEY_DOWN)  ||
          (trg & PAD_BUTTON_B) )
      {
        // リセットタスクを登録
        GATE_GIMMICK_Camera_Reset( work->fieldmap, work->cameraMoveFrame );
        ++(*seq);
      }
    }
    break;
  // カメラリセット完了待ち
  case SEQ_WAIT_CAMERA_RESET:
    { // 全タスク処理終了で次のシーケンスへ
      FIELD_TASK_MAN* man = FIELDMAP_GetTaskManager( work->fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) ){ ++(*seq); }
    }
    break;
  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//==========================================================================================
// ■外部公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を見るイベントを生成する
 *
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 * @param frame    カメラの移動に要するフレーム数
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_LookElboard( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u16 frame)
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, SeasonDisplay, sizeof(EVENT_WORK) ); 
  // イベントワーク初期化
  work                  = GMEVENT_GetEventWork( event );
  work->gsys            = gsys;
  work->fieldmap        = fieldmap;
  work->cameraMoveFrame = frame;
  return event;
} 
