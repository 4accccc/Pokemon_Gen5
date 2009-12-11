/**
 *  @file   event_fishing.c
 *  @brief  釣りイベント
 *  @author Miyuki Iwasawa
 *  @datge  09.12.04
 */

#include <gflib.h>
#include "system/gfl_use.h"

#include "system/main.h"  //HEAPID_PROC
#include "fieldmap.h"
#include "weather.h"

#include "field_skill.h"
#include "eventwork.h"

#include "script.h"
#include "encount_data.h"
#include "field_encount.h"
#include "fldmmdl.h"

#include "event_fishing.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_AMAIKAORI_ENC_FAILED

typedef enum{
 SEQ_ENCOUNT_CHECK,
 SEQ_ENCOUNT_FAILED,
 SEQ_END,
}AMAIKAORI_SEQ;

typedef struct _FISHING_WORK{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK *fieldWork;
  
  FIELD_PLAYER *fplayer;
  MMDL* player_mmdl;
  MMDL_GRIDPOS  pos;

  VecFx32 player_pos;
  VecFx32 target_pos;
}FISHING_WORK;

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work);

//------------------------------------------------------------------
/*
 *  @brief  釣りイベント起動
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldFishing( FIELDMAP_WORK* fieldmap, GAMESYS_WORK* gsys )
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, FieldFishingEvent, sizeof(FISHING_WORK));
	FISHING_WORK * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(FISHING_WORK));
  wk->gsys = gsys;
  wk->fieldWork = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  
  wk->fplayer = FIELDMAP_GetFieldPlayer( fieldmap );
  wk->player_mmdl = FIELD_PLAYER_GetMMdl( wk->fplayer );

  FIELD_PLAYER_GetFrontGridPos( wk->fplayer, &wk->pos.gx, &wk->pos.gy, &wk->pos.gz );
  FIELD_PLAYER_GetPos( wk->fplayer, &wk->player_pos);

  {
    FLDMAPPER_GRIDINFODATA gridData;
    const FLDMAPPER* g3dMapper = (const FLDMAPPER*)FIELDMAP_GetFieldG3Dmapper( fieldmap ); 
    wk->target_pos.x = GRID_SIZE_FX32(wk->pos.gx);
    wk->target_pos.y = wk->player_pos.y;
    wk->target_pos.z = GRID_SIZE_FX32(wk->pos.gz);
    if( FLDMAPPER_GetGridData( g3dMapper, &wk->target_pos, &gridData) == FALSE){
      return NULL;
    }
    wk->target_pos.y = gridData.height;
  }

  return event;
}

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work)
{
	FISHING_WORK * wk = work;
	
  switch (*seq) {
  case SEQ_ENCOUNT_CHECK:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( wk->fieldWork ) );

    //エンカウントチェック
    {
      GMEVENT* enc_ev = FIELD_ENCOUNT_CheckFishingEncount( FIELDMAP_GetEncount( wk->fieldWork ), ENC_TYPE_NORMAL );
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
	case SEQ_END:
    MMDLSYS_ClearPauseMoveProc( FIELDMAP_GetMMdlSys( wk->fieldWork ) );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}


