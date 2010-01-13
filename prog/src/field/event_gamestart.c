//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_gamestart.c
 *	@brief  イベント　ゲーム開始
 *	@author	tomoya takahashi
 *	@date		2010.01.12
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "event_gamestart.h"

#include "event_mapchange.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// OVERLAY FIELD_INIT
FS_EXTERN_OVERLAY(field_init);


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ゲーム終了待ちイベント
//=====================================
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
}GAME_END_WORK;
//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static GMEVENT_RESULT GameEndEvent(GMEVENT * event, int *seq, void *work);


//------------------------------------------------------------------
/**
 * @brief ゲーム開始イベント
 */
//------------------------------------------------------------------
GMEVENT * EVENT_GameStart(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work)
{
  // フィールド初期化用オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_init) );
  return EVENT_CallGameStart( gsys, game_init_work );
}


//------------------------------------------------------------------
/**
 * @brief デバッグ用：ゲーム終了
 */
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_GameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, GameEndEvent, sizeof(GAME_END_WORK));
  GAME_END_WORK * gew = GMEVENT_GetEventWork(event);

  // ゲーム終了イベント生成
  gew->gsys = gsys;
  gew->fieldmap = fieldmap;
  return event;
}


//--------------------------------------------------------------
/**
 * @brief 全滅時のマップ遷移処理（フィールド非生成時）
 */
//--------------------------------------------------------------
GMEVENT * EVENT_GameOver( GAMESYS_WORK * gsys )
{
  // フィールド初期化用オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_init) );
  return EVENT_CallGameOver( gsys );
}


//----------------------------------------------------------------------------
/**
 *	@brief  オーバーレイの読み込み
 */
//-----------------------------------------------------------------------------
void GAMESTART_OVERLAY_FIELD_INIT_Load( void )
{
  // フィールド初期化用オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_init) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  オーバーレイの破棄
 */
//-----------------------------------------------------------------------------
void GAMESTART_OVERLAY_FIELD_INIT_UnLoad( void )
{
  // フィールド初期化用オーバーレイ読み込み
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_init) );
}





//-----------------------------------------------------------------------------
/**
 *    private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド終了待ちイベント
 *
 *	@retval GMEVENT_RES_CONTINUE = 0, ///<イベント継続中
 *	@retval GMEVENT_RES_FINISH,       ///<イベント終了
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GameEndEvent(GMEVENT * event, int *seq, void *work)
{
  GAME_END_WORK * gew = work;
  
  switch (*seq) {
  case 0:
    //終了イベントをコール
    {
      GMEVENT* end_event;
      end_event = DEBUG_EVENT_CallGameEnd(gew->gsys, gew->fieldmap);
      GMEVENT_CallEvent(event, end_event);
    }
    (*seq)++;
    break;
  case 1:
    //　オーバーレイのアンロード
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_init) );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

