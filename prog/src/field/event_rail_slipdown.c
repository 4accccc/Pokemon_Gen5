//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_rail_slipdown.c
 *	@brief  チャンピオンロード専用　レールマップ　ずり落ちイベント
 *	@author	tomoya takahashi
 *	@date		2009.10.16
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fieldmap.h"
#include "event_rail_slipdown.h"

#include "arc/fieldmap/zone_id.h"
#include "field_nogrid_mapper.h"

#include "rail_slipdown.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ずり落ちイベントワーク
//=====================================
typedef struct {
  RAIL_SLIPDOWN_WORK* p_wk;
} EVENT_RAIL_SLIPDOWN_WORK;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// ずり落ち
static GMEVENT_RESULT EVENT_RailSlipDownMain(GMEVENT * p_event, int *  p_seq, void * p_work);




//----------------------------------------------------------------------------
/**
 *	@brief  チャンピオンマップ　レールマップ　ずり落ち処理
 *
 *	@param	gsys        ゲームシステム
 *	@param	fieldmap    フィールドマップ
 *
 *	@return 設定したイベント
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_RailSlipDown(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  GMEVENT *event;
  EVENT_RAIL_SLIPDOWN_WORK* p_slipdown;

  event = GMEVENT_Create(
    gsys, NULL, EVENT_RailSlipDownMain, sizeof(EVENT_RAIL_SLIPDOWN_WORK));

  // チャンピオンロードかチェック
  GF_ASSERT( FIELDMAP_GetZoneID( fieldmap ) == ZONE_ID_D09 );

  // レールマップかチェック
  GF_ASSERT( FIELDMAP_GetMapControlType( fieldmap ) == FLDMAP_CTRLTYPE_NOGRID );

  // ワークの初期化
  p_slipdown = GMEVENT_GetEventWork(event);


  // OVERLAY
	GFL_OVERLAY_Load( FS_OVERLAY_ID(field_d09_slipdown) );

  // 自機
  {
    FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fieldmap );
    MMDL* p_mmdl = FIELD_PLAYER_GetMMdl( p_player );
    p_slipdown->p_wk = RailSlipDown_Create( gsys, fieldmap, p_mmdl, TRUE );
  }

  
  return event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  チャンピオンマップ　レールマップ　ずり落ち処理
 *
 *	@param	gsys        ゲームシステム
 *	@param	fieldmap    フィールドマップ
 *	@param  mmdl        すべり降りさせるモデル
 *
 *	@return 設定したイベント
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_RailSlipDownObj(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, MMDL* mmdl)
{
  GMEVENT *event;
  EVENT_RAIL_SLIPDOWN_WORK* p_slipdown;

  event = GMEVENT_Create(
    gsys, NULL, EVENT_RailSlipDownMain, sizeof(EVENT_RAIL_SLIPDOWN_WORK));

  // チャンピオンロードかチェック
  GF_ASSERT( FIELDMAP_GetZoneID( fieldmap ) == ZONE_ID_D09 );

  // レールマップかチェック
  GF_ASSERT( FIELDMAP_GetMapControlType( fieldmap ) == FLDMAP_CTRLTYPE_NOGRID );

  // ワークの初期化
  p_slipdown = GMEVENT_GetEventWork(event);

  // OVERLAY
	GFL_OVERLAY_Load( FS_OVERLAY_ID(field_d09_slipdown) );

  p_slipdown->p_wk = RailSlipDown_Create( gsys, fieldmap, mmdl, FALSE );

  
  return event;
}




//-----------------------------------------------------------------------------
/**
 *      ｐｒｉｖａｔｅ関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  スリップイベント
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_RailSlipDownMain(GMEVENT * p_event, int *  p_seq, void * p_work)
{
  EVENT_RAIL_SLIPDOWN_WORK* p_slipdown;

  //ワークの初期化 
  p_slipdown = GMEVENT_GetEventWork(p_event);

  if( RailSlipDown_IsEnd( p_slipdown->p_wk ) ){

    RailSlipDown_Delete( p_slipdown->p_wk );

		GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_d09_slipdown) );
    return GMEVENT_RES_FINISH;
  }
  

  return GMEVENT_RES_CONTINUE;
}
