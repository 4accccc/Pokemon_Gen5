//============================================================================================
/**
 * @file    event_gpower.c
 * @brief	  イベント：Gパワー関連イベント
 * @author  iwasawa	
 * @date	  2009.02.17
 */
//============================================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "./event_fieldmap_control.h"
#include "./event_gpower.h"

#include "sound/pm_sndsys.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "../../../resource/fldmapdata/script/gpower_scr_def.h" //SCRID_～


static GMEVENT_RESULT EVENT_GPowerUseMain(GMEVENT * event, int *  seq, void * work);

/*
 *  @brief  Gパワー発動イベント
 *
 *  @param  g_power 発動するGパワーID
 *  @param  mine    自分のGパワーかどうか？
 */
GMEVENT* EVENT_GPowerEffectStart(GAMESYS_WORK * gsys, GPOWER_ID g_power, BOOL mine )
{
  GMEVENT* event;

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_START, NULL, HEAPID_FIELDMAP );

  {
    POWER_CONV_DATA * p_data = GPOWER_PowerData_LoadAlloc( GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    GPOWER_TYPE type = GPOWER_ID_to_Type( p_data, g_power );
    GPOWER_Set_OccurID( g_power, p_data, mine );
    GAMEBEACON_Set_GPower( g_power );
 
    SCRIPT_SetScriptWorkParam( SCRIPT_GetScriptWorkFromEvent( event ), g_power, type, mine, 0 );
    
    GPOWER_PowerData_Unload( p_data );
  }
  return event;
}

/*
 *  @brief  Gパワー効果終了イベント
 */
GMEVENT* EVENT_GPowerEffectEnd( GAMESYS_WORK * gsys )
{
  GMEVENT* event;

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_END, NULL, HEAPID_FIELDMAP );
  return event;
}

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_GPowerUseMain(GMEVENT * event, int *  seq, void * work)
{
//  GAMESYS_WORK * gsys = bdw->gsys;
//  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
//  FIELDMAP_WORK * fwork = GAMESYSTEM_GetFieldMapWork(gsys);

//  GMEVENT_CallEvent(event, fade_event);
  switch (*seq) {
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


