//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_fly.c
 *	@brief  飛行ジム
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.02.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// system
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl
#include "field_task_player_trans.h" // for FIELD_TASK_TransPlayer

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// script
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c06gym0101_def.h"  //for SCRID_～

// arc
#include "arc/fieldmap/gym_fly.naix"
#include "../../../resource/fldmapdata/gimmick/gym_fly/gym_fly_local_def.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_fly_sv.h"
#include "gym_fly_se_def.h"
#include "gym_fly.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_FLY_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_FLY_UNIT_IDX (0)
#define GYM_FLY_TMP_ASSIGN_ID  (1)

//-------------------------------------
///	風力計状態
//=====================================
typedef enum {
  ANEMOMETER_STATE_LOW,     // 低速
  ANEMOMETER_STATE_MIDDLE,  // 中速
  ANEMOMETER_STATE_HIGH,    // 高速
}ANEMOMETER_STATE;

// 中速に変わる時間
#define ANEMOMETER_MIDDLE_TIME  (2*30)    // 2秒
// 高速に変わる時間
#define ANEMOMETER_HIGH_TIME    (4*30)    // 4秒

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  u16 time;     // 時間計測用
  u8  state;    // 風速状態   ANEMOMETER_STATE_XXX
  u8  dummy;
}GYM_FLY_TMP;

//-------------------------------------
///	吹き飛ばしイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *fieldWork;   // フィールドマップ
  FIELD_PLAYER    *player;      // 自機
  MMDL            *mmdl;        // 自機の動作モデル
} BLOW_EVENT_WORK;


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================
// ANEMOMETER 風力計
static ANEMOMETER_STATE ANEMOMETER_GetState( const u16 c_time ); 
static void ANEMOMETER_ChangeState( FIELDMAP_WORK *fieldWork, const ANEMOMETER_STATE state );
static GMEVENT* ANEMOMETER_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT BlowEvent( GMEVENT* event, int* seq, void* wk );

//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief     セットアップ関数
 * @param	    fieldWork     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク確保
  GYM_FLY_TMP *p_tmp = GMK_TMP_WK_AllocWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_FLY_TMP));

  p_tmp->time   = 0;
  p_tmp->state  = ANEMOMETER_STATE_LOW;
}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   fieldWork     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_FLY_End(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_FLY_Move(FIELDMAP_WORK *fieldWork)
{
  GYM_FLY_TMP *p_tmp = GMK_TMP_WK_GetWork( fieldWork, GYM_FLY_TMP_ASSIGN_ID );

}


//----------------------------------------------------------------------------
/**
 *	@brief  時間計測・監視　イベント起動チェック
 *
 *	@param	GAMESYS_WORK *p_gamesys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_FLY_CheckEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_FLY_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  GMEVENT *event;

  if( p_tmp == NULL ){ return NULL; }

  // 時間経過
  p_tmp->time++;
  ICHI_Printf("*");
  
  {
    // 時間監視
    ANEMOMETER_STATE tmp_state = ANEMOMETER_GetState( p_tmp->time );

    // 風力計の状態が変更したら
    if( p_tmp->state != tmp_state )
    {
      ANEMOMETER_ChangeState( p_fieldmap, tmp_state );

      // 高速の時のみ吹き飛ばしイベント作成
      if( p_tmp->state == ANEMOMETER_STATE_HIGH )
      {
        event = ANEMOMETER_BLOW_CreateEvent( NULL, p_gamesys );
        return event;
      }
    }
  }

  return NULL;
}


//=============================================================================
/**
 *      static関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  時間を見て風力計の状態を返す
 *
 *	@param	const u16 c_time 
 *
 *	@return その時間の風力計状態 ANEMOMETER_STATE_XXX
 */
//-----------------------------------------------------------------------------
static ANEMOMETER_STATE ANEMOMETER_GetState( const u16 c_time )
{
  if( c_time >= ANEMOMETER_HIGH_TIME )
  {
    return ANEMOMETER_STATE_HIGH;
  }
  else if( c_time >= ANEMOMETER_MIDDLE_TIME )
  {
    return ANEMOMETER_STATE_MIDDLE;
  }
  else
  {
    return ANEMOMETER_STATE_LOW;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  風力計の状態変更
 *
 *	@param	FIELDMAP_WORK *fieldWork    フィールドワークポインタ
 *	@param	ANEMOMETER_STATE state      変更後の状態
 */
//-----------------------------------------------------------------------------
static void ANEMOMETER_ChangeState( FIELDMAP_WORK *fieldWork, const ANEMOMETER_STATE state )
{
  GYM_FLY_TMP *p_tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);

  p_tmp->state = state;
  
  // 状態別初期化
  switch( p_tmp->state ){
  case ANEMOMETER_STATE_LOW:
    ICHI_Printf("風力計状態：低速\n");
    break;
  case ANEMOMETER_STATE_MIDDLE:
    ICHI_Printf("風力計状態：中速\n");
    break;
  case ANEMOMETER_STATE_HIGH:
    p_tmp->time = 0;
    ICHI_Printf("風力計状態：高速\n");
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベント作成
 *
 *	@param	GMEVENT* parent
 *	@param	gsys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* ANEMOMETER_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*          event;
  BLOW_EVENT_WORK*  work;
  FIELD_PLAYER*     player;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // 生成
  event = GMEVENT_Create( gsys, parent, BlowEvent, sizeof(BLOW_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys              = gsys;
  work->fieldWork          = p_fieldmap;
  work->player            = player;
  work->mmdl              = FIELD_PLAYER_GetMMdl( player );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT BlowEvent( GMEVENT* event, int* seq, void* wk )
{
  BLOW_EVENT_WORK *p_wk = (BLOW_EVENT_WORK*)wk;

  // 処理シーケンス
  enum{
    SEQ_WAIT,   // グリッド調度待ち
    SEQ_START,   // イベント開始 
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
  };

  switch( *seq )
  {
  // グリッド調度待ち
  case SEQ_WAIT:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // ！マーク
      MMDL_SetAcmd( p_wk->mmdl, AC_MARK_GYOE ); 

      *seq = SEQ_START;
    }
    break;
  // イベント開始
  case SEQ_START:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // 足元チェック

      // 移動タスク登録
      {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        VecFx32 down_pos;
        {
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->fieldWork );
          FIELD_PLAYER_GetPos( player, &down_pos );
          down_pos.z += GRID_TO_FX32(5);
        }
        {
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->fieldWork );
          s16 gx, gy, gz;
          FIELD_PLAYER_GetGridPos( player, &gx, &gy, &gz );
          ICHI_Printf("自機Grid座標 %d:%d:%d\n", gx, gy, gz);
        }
        move = FIELD_TASK_TransPlayer( p_wk->fieldWork, 5*4, &down_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->fieldWork );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
      }

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->fieldWork );
        FIELD_PLAYER_CORE* core = FIELD_PLAYER_GetCoreWk( player );
        MMDL* mmdl = FIELD_PLAYER_CORE_GetMMdl( core );
        MMDL_OnStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      *seq = SEQ_MAIN;
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      // 自機の向きを変える
      {
        int key = GFL_UI_KEY_GetCont();
        if( key & PAD_KEY_UP )   { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_U_4F ); }
        else if( key & PAD_KEY_DOWN ) { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_D_4F ); }
        else if( key & PAD_KEY_LEFT ) { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_L_4F ); }
        else if( key & PAD_KEY_RIGHT ){ MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_R_4F ); }
        else
        {
          // キー入力がない場合は主人公の向きにその場歩き
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->fieldWork );
          u16 dir = FIELD_PLAYER_GetDir( player );
          dir %= 4;
          MMDL_SetAcmd( p_wk->mmdl, (AC_STAY_WALK_U_4F + dir) );
        }
      }

      // タスク終了チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->fieldWork );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {
          *seq = SEQ_EXIT;
        }
      }
    }
    break;

  // イベント終了
  case SEQ_EXIT:
    // 高さ取得禁止ステータスビットをOFFに戻す
    {
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->fieldWork );
      FIELD_PLAYER_CORE* core = FIELD_PLAYER_GetCoreWk( player );
      MMDL* mmdl = FIELD_PLAYER_CORE_GetMMdl( core );
      MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

