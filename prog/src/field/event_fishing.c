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
#include "effect_encount.h"
#include "fldmmdl.h"
#include "fldeff_gyoe.h"

#include "event_fishing.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_AMAIKAORI_ENC_FAILED

typedef enum{
 SEQ_FISHING_START,
 SEQ_START_WAIT,
 SEQ_HIT_WAIT,
 SEQ_HIT,
 SEQ_HIT_SUCCESS,
 SEQ_ENCOUNT_FAILED,
 SEQ_TOO_EARLY,
 SEQ_TOO_SLOW,
 SEQ_END,
}AMAIKAORI_SEQ;

enum{
  CODE_NONE,
  CODE_HIT,
  CODE_END,
};

#define TIME_START_WAIT (30)
#define TIME_ENCOUNT_FAILED_WAIT (30*4)
#define TIME_HIT_WAIT_MIN (30)
#define TIME_HIT_WAIT_MAX (120-TIME_HIT_WAIT_MIN)
#define TIME_HIT_COUNT (30)

typedef struct _FISHING_WORK{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK *fieldWork;
  FIELD_ENCOUNT* fld_enc;
  const FLDMAPPER* g3dMapper;
  MMDLSYS* mmdl_sys;

  FIELD_PLAYER *fplayer;
  MMDL* player_mmdl;
  MMDL_GRIDPOS  pos;

  FLDEFF_TASK*  task_gyoe;

  VecFx32 player_pos;
  VecFx32 target_pos;

  u32     enc_type;
  u32     time;
  u32     count;
  GMEVENT* enc_event;
}FISHING_WORK;

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work);
static BOOL sub_TimeWait( FISHING_WORK* wk, u32 time );
static int sub_TimeKeyWait( FISHING_WORK* wk, u32 time );
static void sub_SetGyoeAnime( FISHING_WORK* wk );
static void sub_DelGyoeAnime( FISHING_WORK* wk );

/*
 *  @brief  釣りができるポジションかチェック
 */
BOOL FieldFishingCheckPos( GAMEDATA* gdata, FIELDMAP_WORK* fieldmap, VecFx32* outPos )
{
  u8 dir;
  VecFx32 pos;
  MMDL* mmdl;
  FIELD_PLAYER* player;
  FLDMAPPER_GRIDINFODATA gridData;

  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );

  player = FIELDMAP_GetFieldPlayer( fieldmap );
  mmdl = FIELD_PLAYER_GetMMdl( player );
  dir = MMDL_GetDirDisp( mmdl );

  FIELD_PLAYER_GetDirPos( player, dir, &pos );
  if( FLDMAPPER_GetGridData( mapper, &pos, &gridData) == FALSE){
    return FALSE;
  }
  //岸チェック
  if( MAPATTR_VALUE_CheckShore( MAPATTR_GetAttrValue( gridData.attr ) ) == TRUE ){ //岸
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 ); //もう一歩先
    FIELD_PLAYER_GetDirPos( player, dir, &pos );
  }
  //アトリビュートチェック
  if( MAPATTR_VALUE_CheckWaterTypeSeason( MAPATTR_GetAttrValue( gridData.attr ),GAMEDATA_GetSeasonID( gdata ))){
    return FALSE;
  }
  outPos->x = pos.x;
  outPos->y = gridData.height;
  outPos->z = pos.x;

  return TRUE;
}


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
  wk->fld_enc = FIELDMAP_GetEncount( wk->fieldWork );

  wk->g3dMapper = (const FLDMAPPER*)FIELDMAP_GetFieldG3Dmapper( wk->fieldWork ); 
  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldWork );
  wk->fplayer = FIELDMAP_GetFieldPlayer( fieldmap );
  wk->player_mmdl = FIELD_PLAYER_GetMMdl( wk->fplayer );

  FIELD_PLAYER_GetPos( wk->fplayer, &wk->player_pos);

  //ポジションチェック
  if( FieldFishingCheckPos( wk->gdata, wk->fieldWork, &wk->target_pos ) == FALSE){
    return NULL;
  }
  wk->pos.gx = SIZE_GRID_FX32(wk->target_pos.x);
  wk->pos.gy = SIZE_GRID_FX32(wk->target_pos.y);
  wk->pos.gz = SIZE_GRID_FX32(wk->target_pos.z);

  //エフェクトエンカウントチェック
  if( EFFECT_ENC_CheckEffectPos( wk->fld_enc, &wk->pos )){
    wk->enc_event = FIELD_ENCOUNT_CheckFishingEncount( wk->fld_enc, ENC_TYPE_EFFECT );
  }else{
    wk->enc_event = FIELD_ENCOUNT_CheckFishingEncount( wk->fld_enc, ENC_TYPE_NORMAL );
  }
  return event;
}

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work)
{
  int ret;
	FISHING_WORK * wk = work;
	
  switch (*seq) {
  case SEQ_FISHING_START:
    MMDLSYS_PauseMoveProc( wk->mmdl_sys );

    //エンカウントチェック
    wk->enc_event = FIELD_ENCOUNT_CheckFishingEncount( wk->fld_enc, wk->enc_type );
    //フォルムチェンジ
    FIELD_PLAYER_ChangeDrawForm( wk->fplayer, PLAYER_DRAW_FORM_FISHING );
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_START );
    (*seq)++;
    break;

  case SEQ_START_WAIT:
    if( !sub_TimeWait(wk,TIME_START_WAIT )){
      break;
    }
    if( wk->enc_event == NULL ){  //エンカウト失敗
      *seq = SEQ_ENCOUNT_FAILED;
      break;
    }
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_ON );
    wk->time = GFUser_GetPublicRand0(TIME_HIT_WAIT_MAX)+TIME_HIT_WAIT_MIN;
    (*seq)++;
    break;
  
  //当たり待ち
  case SEQ_HIT_WAIT:
    ret = sub_TimeKeyWait(wk, wk->time );
    if( ret == CODE_HIT ){
      *seq = SEQ_TOO_EARLY;
    }else if( ret == CODE_END ){
      sub_SetGyoeAnime( wk );
      (*seq)++;
    }
    break;

  //釣り上げチェック
  case SEQ_HIT:
    ret = sub_TimeKeyWait(wk, TIME_HIT_COUNT );
    if( ret == CODE_NONE){
      break;
    }
    sub_DelGyoeAnime( wk );
    if( ret == CODE_HIT){
      MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_HIT );
      SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_SUCCESS, NULL, NULL, HEAPID_FIELDMAP );
      *seq = SEQ_HIT_SUCCESS;
    }else if( ret == CODE_END){
      *seq = SEQ_TOO_SLOW;
    }
    break;

  case SEQ_HIT_SUCCESS:
    GMEVENT_CallEvent( event, wk->enc_event );
		(*seq) = SEQ_END;
		break;

  //エンカウント失敗
  case SEQ_ENCOUNT_FAILED:
    if( !sub_TimeWait(wk,TIME_ENCOUNT_FAILED_WAIT )){
      break;
    }
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_END );
    SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_FAILED_ENCOUNT, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;

  //釣り上げ失敗
  case SEQ_TOO_EARLY:
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_END );
    SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_FAILED_TOO_EARLY, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;
  //釣り上げ失敗
  case SEQ_TOO_SLOW:
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_END );
    SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_FAILED_TOO_SLOW, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;

	case SEQ_END:
    FIELD_PLAYER_ChangeDrawForm( wk->fplayer, PLAYER_DRAW_FORM_NORMAL );
    MMDLSYS_ClearPauseMoveProc( wk->mmdl_sys );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

static BOOL sub_TimeWait( FISHING_WORK* wk, u32 time )
{
  if(wk->count++ < time){
    return FALSE;
  }
  wk->count = 0;
  return TRUE;
}

static int sub_TimeKeyWait( FISHING_WORK* wk, u32 time )
{
  if(wk->count++ < time){
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      wk->count = 0;
      return CODE_HIT;
    }
  }else{
    wk->count = 0;
    return CODE_END;
  }
  return CODE_NONE;
}

/*
 *  @brief  ビックリマークアニメ
 */
static void sub_SetGyoeAnime( FISHING_WORK* wk )
{
  FLDEFF_CTRL *fectrl =  FIELDMAP_GetFldEffCtrl( wk->fieldWork );
  wk->task_gyoe = FLDEFF_GYOE_SetMMdl(
      fectrl, wk->player_mmdl, FLDEFF_GYOETYPE_GYOE, TRUE );
}

/*
 *  @brief  ビックリマークアニメ破棄
 */
static void sub_DelGyoeAnime( FISHING_WORK* wk )
{
  FLDEFF_TASK_CallDelete( wk->task_gyoe );
}

