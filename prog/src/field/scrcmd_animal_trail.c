//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_animal_trail.c
 *	@brief  獣道用スクリプト
 *	@author	Masayuki Onoue
 *	@date		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_animal_trail.h"
#include "fieldmap/zone_id.h"
#include "event_mapchange.h"
#include "event_field_fade.h"
#include "field/field_const.h"

#include "../../../resource/animal_trail/animal_trail_data.cdat"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define WARP_POS_X  (15)
#define WARP_POS_Z  (27)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  FIELDMAP_WORK         *p_field_work;
}MAP_CHAGE_EVENT_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );


//----------------------------------------------------------------------------
/**
 *	@brief  けものみちへ移動コマンド
 *
 *	@param	core
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWarpAnimalTrail( VMHANDLE* core, void* wk )
{
	GMEVENT       *p_event = NULL;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  u16 trail_id  = SCRCMD_GetVMWorkValue( core, p_wk );
  MAP_CHAGE_EVENT_WORK *p_ev_wk;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_wk, p_event );
	
  // イベントのパラメータ設定
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  p_ev_wk->p_gsys = p_gsys;
  p_ev_wk->p_field_work = GAMESYSTEM_GetFieldMapWork( p_gsys );

  return VMCMD_RESULT_SUSPEND;
}


//----------------------------------------------------------------------------
/**
 *	@brief  マップチェンジイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_CHAGE_EVENT_WORK *p_wk = wk;

  switch( *seq )
  {
  case 0:
    {
      // フェードアウト
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 1:
    {
      // マップ移動
      VecFx32 warp_pos;       // 遷移先でのマップ位置
      SetGridPos( &warp_pos ,WARP_POS_X ,0 ,WARP_POS_Z);
      GMEVENT_CallEvent(event,
          EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work,ZONE_ID_D34R0101 ,&warp_pos,DIR_UP));
      (*seq)++;
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }
 
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
      
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標をFx32に変換してセットする
 *
 *	@param	*vec 
 *	@param	x     Xグリッド座標
 *	@param	y     Yグリッド座標
 *	@param	z     Xグリッド座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z )
{
  vec->x = (x * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->y = (y * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->z = (z * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
}

