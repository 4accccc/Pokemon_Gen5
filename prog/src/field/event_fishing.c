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
#include "field/eventwork.h"
#include "script.h"
#include "fldmmdl.h"
#include "fldeff_gyoe.h"
#include "fldeff_fishing.h"

#include "gamesystem/btl_setup.h"
#include "encount_data.h"
#include "field_encount.h"
#include "effect_encount.h"
#include "event_battle.h"
#include "savedata/record.h"

#include "event_fishing.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_AMAIKAORI_ENC_FAILED

#include "enceffno_def.h"

typedef enum{
 SEQ_FISHING_START,
 SEQ_START_WAIT,
 SEQ_HIT_WAIT,
 SEQ_HIT,
 SEQ_ENCOUNT_FAILED,
 SEQ_TOO_EARLY,
 SEQ_TOO_SLOW,
 SEQ_END,
 SEQ_HIT_SUCCESS,
}AMAIKAORI_SEQ;

enum{
  CODE_NONE,
  CODE_HIT,
  CODE_END,
};

#define TIME_START_WAIT (15)
#define TIME_ENCOUNT_FAILED_WAIT (30*4)
#define TIME_HIT_WAIT_MIN (30)
#define TIME_HIT_WAIT_MAX (120-TIME_HIT_WAIT_MIN)
#define TIME_HIT_COUNT (30)

typedef struct _FISHING_WORK{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK *fieldWork;
  FIELD_ENCOUNT* fld_enc;
  RECORD* record;
  MMDLSYS* mmdl_sys;

  FIELD_PLAYER *fplayer;
  MMDL* player_mmdl;
  MMDL_GRIDPOS  pos;
  PLAYER_DRAW_FORM player_form;

  FLDEFF_TASK*  task_gyoe;
  FLDEFF_TASK*  task_lure;

  VecFx32 player_pos;
  VecFx32 target_pos;
  VecFx32 player_view_ofs;

  u8      dir;
  u8      enc_type;
  u16     enc_error;
  u32     time;
  u32     count;
  BATTLE_SETUP_PARAM* bsp;
}FISHING_WORK;

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work);
static BOOL sub_TimeWait( FISHING_WORK* wk, u32 time );
static int sub_TimeKeyWait( FISHING_WORK* wk, u32 time );
static void sub_SetGyoeAnime( FISHING_WORK* wk );
static void sub_DelGyoeAnime( FISHING_WORK* wk );
static void sub_SetLureAnime( FISHING_WORK* wk );
static void sub_DelLureAnime( FISHING_WORK* wk );

/*
 *  @brief  釣りができるポジションかチェック
 */
BOOL FieldFishingCheckPos( GAMEDATA* gdata, FIELDMAP_WORK* fieldmap, VecFx32* outPos )
{
  u8 dir;
  VecFx32 t_pos;
  MMDL* mmdl;
  FIELD_PLAYER* player;
  FLDMAPPER_GRIDINFODATA gridData;

  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );

  player = FIELDMAP_GetFieldPlayer( fieldmap );
  mmdl = FIELD_PLAYER_GetMMdl( player );
  dir = MMDL_GetDirDisp( mmdl );

  FIELD_PLAYER_GetDirPos( player, dir, &t_pos );

  //レールマップでは釣り不可
  if( FIELDMAP_GetBaseSystemType( fieldmap) == FLDMAP_BASESYS_RAIL ){
    return FALSE;
  }
#ifdef DEBUG_ONLY_FOR_iwasawa
  {
    GAMESYS_WORK * gsys = FIELDMAP_GetGameSysWork( fieldmap );
    PLAYER_WORK *player = GAMESYSTEM_GetMyPlayerWork( gsys );
	  const VecFx32 *pos = PLAYERWORK_getPosition( player );
    VecFx32 v_pos;
    MMDL_GetVectorPos( mmdl, &v_pos );

    OS_Printf(" p_pos( %08x, %08x, %08x ) mdl_pos( %08x, %08x, %08x ) vec_pos( %08x, %08x, %08x )\n",
        pos->x,pos->y,pos->z, t_pos.x, t_pos.y, t_pos.z, v_pos.x, v_pos.y, v_pos.z );  
  }
#endif

  if( FLDMAPPER_GetGridData( mapper, &t_pos, &gridData) == FALSE){
    return FALSE;
  }
  //岸チェック
  if( MAPATTR_VALUE_CheckShore( MAPATTR_GetAttrValue( gridData.attr ) ) == TRUE ){ //岸
    MMDL_TOOL_AddDirVector( dir, &t_pos, GRID_FX32 ); //もう一歩先
    if( FLDMAPPER_GetGridData( mapper, &t_pos, &gridData) == FALSE){
      return FALSE;
    }
  }
  //アトリビュートチェック
  if( !MAPATTR_VALUE_CheckWaterType(MAPATTR_GetAttrValue( gridData.attr ))){
    return FALSE;
  }
  if( outPos != NULL ){
    outPos->x = t_pos.x;
    outPos->y = gridData.height;
    outPos->z = t_pos.z;
  }

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
  wk->record = GAMEDATA_GetRecordPtr( wk->gdata);

  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldWork );
  wk->fplayer = FIELDMAP_GetFieldPlayer( fieldmap );
  wk->player_mmdl = FIELD_PLAYER_GetMMdl( wk->fplayer );
  wk->dir = MMDL_GetDirDisp( wk->player_mmdl );

  FIELD_PLAYER_GetPos( wk->fplayer, &wk->player_pos);
  wk->player_form = FIELD_PLAYER_GetDrawForm( wk->fplayer );

  //ポジションチェック
  if( FieldFishingCheckPos( wk->gdata, wk->fieldWork, &wk->target_pos ) == FALSE){
    wk->enc_error = TRUE;
  }
  wk->pos.gx = SIZE_GRID_FX32(wk->target_pos.x);
  wk->pos.gy = SIZE_GRID_FX32(wk->target_pos.y);
  wk->pos.gz = SIZE_GRID_FX32(wk->target_pos.z);

  //エフェクトエンカウントチェック
  if( EFFECT_ENC_CheckEffectPos( wk->fld_enc, &wk->pos )){
    wk->enc_type = ENC_TYPE_EFFECT;
    wk->bsp = FIELD_ENCOUNT_CheckFishingEncount( wk->fld_enc, ENC_TYPE_EFFECT );
  }else{
    wk->enc_type = ENC_TYPE_NORMAL;
    wk->bsp = FIELD_ENCOUNT_CheckFishingEncount( wk->fld_enc, ENC_TYPE_NORMAL );
  }

  IWASAWA_Printf("PlayerPos ( 0x%08x, 0x%08x, 0x%08x)\n", wk->player_pos.x, wk->player_pos.y, wk->player_pos.z );
  IWASAWA_Printf("TargetPos ( 0x%08x, 0x%08x, 0x%08x)\n", wk->target_pos.x, wk->target_pos.y, wk->target_pos.z );

  MMDLSYS_PauseMoveProc( wk->mmdl_sys );
  return event;
}

static GMEVENT_RESULT FieldFishingEvent(GMEVENT * event, int * seq, void *work)
{
  int ret;
	FISHING_WORK * wk = work;
	
  switch (*seq) {
  case SEQ_FISHING_START:
    if(wk->enc_error){
      *seq = SEQ_END;
      break;
    }
    
    //フォルムチェンジ
    FIELD_PLAYER_ChangeDrawForm( wk->fplayer, PLAYER_DRAW_FORM_FISHING );
    FIELD_PLAYER_ForceWaitVBlank( wk->fplayer );
    
    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_START );
    PMSND_PlaySE( SEQ_SE_FLD_19 );

    (*seq)++;
    break;

  case SEQ_START_WAIT:
    if( !sub_TimeWait(wk,TIME_START_WAIT )){
      break;
    }
    //ルアーセット
    sub_SetLureAnime(wk);

    if( wk->bsp == NULL ){  //エンカウト失敗
      *seq = SEQ_ENCOUNT_FAILED;
      break;
    }
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
      sub_DelLureAnime( wk );
      MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_HIT );
      SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_SUCCESS, NULL, NULL, HEAPID_FIELDMAP );
      *seq = SEQ_HIT_SUCCESS;
    }else if( ret == CODE_END){
      *seq = SEQ_TOO_SLOW;
    }
    break;
  //エンカウント失敗
  case SEQ_ENCOUNT_FAILED:
    ret = sub_TimeKeyWait(wk, TIME_ENCOUNT_FAILED_WAIT );
    if( ret != CODE_END ){
      if( ret == CODE_HIT ){
        *seq = SEQ_TOO_EARLY;
      }
      break;
    }
    //ブレイクスルー
  case SEQ_TOO_EARLY:
  case SEQ_TOO_SLOW:
    sub_DelLureAnime( wk );
    if(wk->bsp != NULL){
      if( wk->enc_type == ENC_TYPE_EFFECT){ //エフェクト破棄
        EFFECT_ENC_EffectDelete( wk->fld_enc );
      }
      BATTLE_PARAM_Delete(wk->bsp);
    }
    //つりそこなった回数  
    RECORD_Inc( wk->record, RECID_FISHING_FAILURE );

    MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_END );
    SCRIPT_CallScript( event,SCRID_FLD_EV_FISHING_FAILED_ENCOUNT+(*seq-SEQ_ENCOUNT_FAILED), NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;

	case SEQ_END:
    FIELD_PLAYER_ChangeDrawForm( wk->fplayer, wk->player_form );
    FIELD_PLAYER_ForceWaitVBlank( wk->fplayer ); //BTS5723 100616
		return GMEVENT_RES_FINISH;

  //エンカウントイベントへ移行
  case SEQ_HIT_SUCCESS:
    //釣り上げた回数  
    RECORD_Inc( wk->record, RECID_FISHING_SUCCESS );
    GMEVENT_ChangeEvent( event, EVENT_WildPokeBattle( wk->gsys, wk->fieldWork, wk->bsp, FALSE, ENC_TYPE_FISHING ) );
	  return GMEVENT_RES_CONTINUE;
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
  
  MMDL_SetDrawStatus( wk->player_mmdl, DRAW_STA_FISH_ON );
  wk->task_gyoe = FLDEFF_GYOE_SetMMdl(
      fectrl, wk->player_mmdl, FLDEFF_GYOETYPE_GYOE, TRUE );

  FLDEFF_FISHING_LURE_ChangeAnime( wk->task_lure, FISHING_LURE_ANM_HIT );
}

/*
 *  @brief  ビックリマークアニメ破棄
 */
static void sub_DelGyoeAnime( FISHING_WORK* wk )
{
  if(wk->task_gyoe != NULL){
    FLDEFF_TASK_CallDelete( wk->task_gyoe );
    wk->task_gyoe = NULL;
  }
}

/*
 *  @brief  ルアーアニメ
 */
static void sub_SetLureAnime( FISHING_WORK* wk )
{
  FLDEFF_CTRL *fectrl =  FIELDMAP_GetFldEffCtrl( wk->fieldWork );
  wk->task_lure = FLDEFF_FISHING_LURE_Set( fectrl, &wk->target_pos, wk->dir, ( wk->target_pos.y == wk->player_pos.y ) );
}

/*
 *  @brief  ビックリマークアニメ破棄
 */
static void sub_DelLureAnime( FISHING_WORK* wk )
{
  if(wk->task_lure != NULL){
    FLDEFF_TASK_CallDelete( wk->task_lure );
    wk->task_lure = NULL;
  }
}

