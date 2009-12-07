///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセンPC関連イベント
 * @file   event_pokecen_pc.c
 * @author obata
 * @date   2009.12.07
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "event_pokecen_pc.h"
#include "gamesystem/game_event.h"
#include "sound/pm_sndsys.h"


//=========================================================================================
// ■イベントワーク
//=========================================================================================
typedef struct
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;

} EVENT_WORK;


//=========================================================================================
// ■PC起動イベント
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief PC起動イベント処理
 */
//-----------------------------------------------------------------------------------------
static GMEVENT_RESULT PcOnEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // アニメ, SE再生
  case 0:
    PMSND_PlaySE( SEQ_SE_PC_ON );
    ++(*seq);
    break;
  // アニメ, SE待ち
  case 1:
    ++(*seq);
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC起動イベントを作成する
 *
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
GMEVENT* EVENT_PokecenPcOn( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, NULL, PcOnEvent, sizeof(EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  return event;
} 


//=========================================================================================
// ■PC停止イベント
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief PC停止イベント処理
 */
//-----------------------------------------------------------------------------------------
static GMEVENT_RESULT PcOffEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // アニメ, SE再生
  case 0:
    PMSND_PlaySE( SEQ_SE_PC_LOGOFF );
    ++(*seq);
    break;
  // アニメ, SE待ち
  case 1:
    ++(*seq);
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC停止イベントを作成する
 *
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
GMEVENT* EVENT_PokecenPcOff( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, NULL, PcOffEvent, sizeof(EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  return event;
} 
