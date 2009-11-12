//============================================================================================
/**
 * @file	  itemuse_proc.c
 * @brief	  フィールドに関連したアイテムの使用処理
 * @author	k.ohno
 * @date	  09.08.04
 */
//============================================================================================
#include "gflib.h"
#include "itemuse_event.h"
#include "field/fieldmap.h"
#include "event_mapchange.h"
#include "arc/fieldmap/zone_id.h"

//=============================================================================
/**
 *	共通
 */
//=============================================================================
//-------------------------------------
///	共通呼び出し用テーブル
//=====================================
static ItemUseEventFunc * const Event_ItemUse[ EVENT_ITEMUSE_CALL_MAX ]	=
{	
	&EVENT_CycleUse,
	&EVENT_PalaceJumpUse,
};


//----------------------------------------------------------------------------
/**
 *	@brief	共通呼び出し
 *
 *	@param	EVENT_ITEMUSE_CALL_TYPE type	呼び出すアイテム
 *	@param	*gsys				ゲームシステム
 *	@param	*field_wk		フィールド
 *
 *	@return	イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_FieldItemUse( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk )
{	
	GF_ASSERT( type < EVENT_ITEMUSE_CALL_MAX );
	return Event_ItemUse[ type ]( field_wk, gsys );
}


//=============================================================================
/**
 *	個別呼び出し
 */
//=============================================================================
typedef struct{
  GAMESYS_WORK *gameSys;
} CYCLEUSE_STRUCT;

//------------------------------------------------------------------------------
/**
 * @brief   自転車を使う
 * @retval  none
 */
//------------------------------------------------------------------------------

static GMEVENT_RESULT CycleEvent(GMEVENT * event, int * seq, void *work)
{
  CYCLEUSE_STRUCT* pCy = work;
  FIELDMAP_SetPlayerItemCycle( GAMESYSTEM_GetFieldMapWork( pCy->gameSys ) );
  return GMEVENT_RES_FINISH;

}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_CycleUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, CycleEvent, sizeof(CYCLEUSE_STRUCT));
	CYCLEUSE_STRUCT * pCy = GMEVENT_GetEventWork(event);
	pCy->gameSys = gsys;
	return event;
}


//------------------------------------------------------------------------------
/**
 * @brief   パレスにジャンプする
 * @retval  none
 */
//------------------------------------------------------------------------------


GMEVENT * EVENT_PalaceJumpUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys)
{
  VecFx32 pos;
  ZONEID jump_zone;
  
  if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
  	pos.x = 184 << FX32_SHIFT;
  	pos.y = 0;
  	pos.z = 184 << FX32_SHIFT;
  	jump_zone = ZONE_ID_UNION;   //こちらに飛ぶのはデバッグです
  }
  else{
  	pos.x = 760 << FX32_SHIFT;
  	pos.y = 0;
  	pos.z = 234 << FX32_SHIFT;
  	jump_zone = ZONE_ID_PALACE01;
  }
  return EVENT_ChangeMapPos(gsys, fieldWork, jump_zone, &pos, 0);
}


