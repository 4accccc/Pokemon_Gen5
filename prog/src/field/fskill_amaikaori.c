/*
 *  @file   fskill_amaikaori.c
 *  @brief  フィールド技：「あまいかおり」
 *  @author Miyuki Iwasawa
 *  @date   09.10.27
 */

#include <gflib.h>
#include "system/gfl_use.h"

#include "system/main.h"  //HEAPID_PROC
#include "fieldmap.h"
#include "weather.h"

#include "field_skill.h"
#include "eventwork.h"

#include "script.h"
#include "fskill_amaikaori.h"
#include "encount_data.h"
#include "field_encount.h"
#include "fldmmdl.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_AMAIKAORI_ENC_FAILED

typedef enum{
 SEQ_WEATHER_CHECK,
 SEQ_ENCOUNT_CHECK,
 SEQ_ENCOUNT_FAILED,
 SEQ_WEATHER_ERR,
 SEQ_END,
}AMAIKAORI_SEQ;

typedef struct _AMAIKAORI_WORK{
  u8  poke_pos;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK *fieldWork;
}AMAIKAORI_WORK;

static GMEVENT_RESULT FSkillAmaikaoriEvent(GMEVENT * event, int * seq, void *work);
static BOOL amaikaori_WeatherCheck( AMAIKAORI_WORK* wk );

//------------------------------------------------------------------
/*
 *  @brief  あまいかおりイベント起動
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSkillAmaikaori( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 poke_pos )
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, FSkillAmaikaoriEvent, sizeof(AMAIKAORI_WORK));
	AMAIKAORI_WORK * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(AMAIKAORI_WORK));
  wk->poke_pos = poke_pos;
  wk->gsys = gsys;
  wk->fieldWork = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
	return event;
}

static GMEVENT_RESULT FSkillAmaikaoriEvent(GMEVENT * event, int * seq, void *work)
{
	AMAIKAORI_WORK * wk = work;
	
  switch (*seq) {
	case SEQ_WEATHER_CHECK:
    if( amaikaori_WeatherCheck( wk ) == FALSE){
      *seq = SEQ_WEATHER_ERR;
      break;
    }
    //エフェクト起動
    *seq = SEQ_ENCOUNT_CHECK;
    break;
  case SEQ_ENCOUNT_CHECK:
    //エンカウントチェック
    {
      GMEVENT* enc_ev = FIELD_ENCOUNT_CheckEncount( FIELDMAP_GetEncount( wk->fieldWork ), ENC_TYPE_FORCE );
      if( enc_ev == NULL ){  //エンカウト失敗
        *seq = SEQ_ENCOUNT_FAILED;
        break;
      }
      GMEVENT_CallEvent( event, enc_ev );
    }
		(*seq) = SEQ_END;
		break;
  case SEQ_ENCOUNT_FAILED:
    SCRIPT_CallScript( event,SCRID_FLD_EV_AMAIKAORI_ENC_FAILED, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;
  case SEQ_WEATHER_ERR:
    SCRIPT_CallScript( event,SCRID_FLD_EV_AMAIKAORI_WEATHER_ERROR, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;
	case SEQ_END:
    MMDLSYS_ClearPauseMoveProc( FIELDMAP_GetMMdlSys( wk->fieldWork ) );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

////////////////////////////////////////////////
// parts
////////////////////////////////////////////////
static BOOL amaikaori_WeatherCheck( AMAIKAORI_WORK* wk )
{
  FIELD_WEATHER* fld_weather = FIELDMAP_GetFieldWeather( wk->fieldWork );
  u32 weather = FIELD_WEATHER_GetWeatherNo( fld_weather );

  if( weather == WEATHER_NO_SUNNY ){
    return TRUE;
  }
  return FALSE;
}

