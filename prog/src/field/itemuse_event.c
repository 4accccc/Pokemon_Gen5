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
GMEVENT * EVENT_CycleUse(GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, CycleEvent, sizeof(CYCLEUSE_STRUCT));
	CYCLEUSE_STRUCT * pCy = GMEVENT_GetEventWork(event);
	pCy->gameSys = gsys;
	return event;
}

