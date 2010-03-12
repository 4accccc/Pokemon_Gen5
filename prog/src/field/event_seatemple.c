//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_seatemple.c
 *	@brief  海底遺跡　イベント起動
 *	@author	tomoya takahashi 
 *	@date		2010.03.11
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "sound/pm_sndsys.h"

#include "event_seatemple.h"
#include "event_mapchange.h"

#include "savedata/situation.h"
#include "script.h"
#include "fieldmap.h"

#include "field_task_player_drawoffset.h"
#include "field_diving_data.h"  //DIVINGSPOT_GetZoneID

#include "../../../resource/fldmapdata/script/seatemple_scr_def.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	歩数起動スクリプトID
//=====================================
typedef struct
{
  u16 step;
  u16 scrid;
} STEP_DO_SCRIPT_ID;

static const STEP_DO_SCRIPT_ID sc_STEP_DO_SCRIPT_ID[] = {
  { 0, SCRID_STEP_000 },
  { 100, SCRID_STEP_100 },
  { 300, SCRID_STEP_300 },
  { 450, SCRID_STEP_450 },
  { 500, SCRID_STEP_500 },
};

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿　歩数イベント起動
 *
 *	@param	gsys        ゲームシステム
 *	@param	fieldmap    フィールドマップ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_SeaTemple_GetStepEvent( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* saveData = GAMEDATA_GetSaveControlWork(gdata);
  SITUATION* situation = SaveData_GetSituation( saveData );
  u16 count;
  int i;

  count = Situation_GetSeaTempleStepCount( situation );

  for( i=0; i<NELEMS(sc_STEP_DO_SCRIPT_ID); i++ ){

    if( sc_STEP_DO_SCRIPT_ID[i].step == count ){
      return SCRIPT_SetEventScript( gsys, sc_STEP_DO_SCRIPT_ID[i].scrid, NULL, FIELDMAP_GetHeapID( fieldmap ));

    }
  }

  return NULL;
}



//-----------------------------------------------------------------------------
/**
 *					Diving遷移イベント
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	Diving遷移ワーク
//=====================================
typedef struct {
  GAMESYS_WORK* p_gsys;
  FIELDMAP_WORK* p_fieldmap;

  s32 timing;
} EV_DIVING_WORK;

//-------------------------------------
///	自機もぐり演出
//=====================================
#define DIVE_HERO_Y        (-64<<FX32_SHIFT)
#define DIVE_HERO_FRAME    (40)

//-------------------------------------
///	SE再生タイミング
//=====================================
#define DIVE_SE_TIMING  ( 8 )

//----------------------------------------------------------------------------
/**
 *	@brief  Diving　下降イベント
 *
 *  @retval GMEVENT_RES_CONTINUE = 0, ///<イベント継続中
 *  @retval GMEVENT_RES_FINISH,       ///<イベント終了
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_DivingDown( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EV_DIVING_WORK* p_wk = p_work;
  

  switch( (*p_seq) ){

  // 自機を下げる
  case 0:
    {
      FIELD_TASK* p_move;
      FIELD_TASK_MAN* p_task_man = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      VecFx32 vec;
      
      VEC_Set( &vec, 0, DIVE_HERO_Y, 0 );

      p_move     = FIELD_TASK_TransDrawOffset( p_wk->p_fieldmap, DIVE_HERO_FRAME, &vec );
      FIELD_TASK_MAN_AddTask( p_task_man, p_move, NULL );

      p_wk->timing = DIVE_SE_TIMING;
      
      (*p_seq)++;
    }
    break;

  // タイミングをみて、SE再生
  case 1:
    p_wk->timing --;
    if( p_wk->timing <= 0 ){
      PMSND_PlaySE( SEQ_SE_FLD_123 );
      (*p_seq)++;
    }
    break;

  case 2:
    if( PMSND_CheckPlaySE_byPlayerID( PMSND_GetSE_DefaultPlayerID(SEQ_SE_FLD_123) ) == FALSE )
    {
      // マップジャンプ
      u16 connect_zone_id;
      BOOL result;
      GMEVENT * p_sub_event;
      result = DIVINGSPOT_Check( p_wk->p_fieldmap, &connect_zone_id );
      GF_ASSERT( result );
      p_sub_event = DEBUG_EVENT_ChangeMapDefaultPos( p_wk->p_gsys, 
          p_wk->p_fieldmap, connect_zone_id );
      GMEVENT_CallEvent(p_event, p_sub_event);
      (*p_seq)++;
    }
    break;

  // 完了待ち
  case 3:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  Diving　上昇イベント
 *
 *  @retval GMEVENT_RES_CONTINUE = 0, ///<イベント継続中
 *  @retval GMEVENT_RES_FINISH,       ///<イベント終了
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_DivingUp( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EV_DIVING_WORK* p_wk = p_work;
  

  switch( (*p_seq) ){

  case 0:
    // マップジャンプ
    {
      GMEVENT* p_sub_event;
      p_sub_event = EVENT_ChangeMapBySeaTemple( p_wk->p_gsys );

      GMEVENT_CallEvent(p_event, p_sub_event);
    }
    (*p_seq)++;
    break;

  // 完了待ち
  case 1:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  Diving　下降イベント
 *
 *	@param	gsys        ゲームシステム
 *	@param	fieldmap    フィールドマップ
 *
 *	@return 下降イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_SeaTemple_GetDivingDownEvent( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT* p_event;
  EV_DIVING_WORK* p_wk;
  
  p_event = GMEVENT_Create( gsys, NULL, EVENT_DivingDown, sizeof(EV_DIVING_WORK) );


  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_gsys      = gsys;
  p_wk->p_fieldmap  = fieldmap;

  return p_event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  Diving　上昇　イベント
 *
 *	@param	gsys          ゲームシステム
 *	@param	fieldmap      フィールドマップ
 *
 *	@return 上昇イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_SeaTemple_GetDivingUpEvent( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT* p_event;
  EV_DIVING_WORK* p_wk;
  
  p_event = GMEVENT_Create( gsys, NULL, EVENT_DivingUp, sizeof(EV_DIVING_WORK) );


  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_gsys      = gsys;
  p_wk->p_fieldmap  = fieldmap;

  return p_event;
}



