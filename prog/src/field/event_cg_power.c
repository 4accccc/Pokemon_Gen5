//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_cg_power.c
 *	@brief  CGEAR　電源　イベント
 *	@author	tomoya takahashi
 *	@date		2010.04.10
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "fieldmap.h"
#include "field_subscreen.h"

#include "event_cg_power.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CGEAR　電源　ONOFF　管理　処理
//=====================================
enum {
  SEQ_INIT,
  SEQ_MAIN,
  SEQ_EXIT,
} ;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CGEAR　電源　イベント
//=====================================
typedef struct {
  FIELDMAP_WORK* p_fieldmap;
  FIELD_SUBSCREEN_WORK* p_subscreen;
} CGEAR_POWER_EVENT_WK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static GMEVENT_RESULT CGearPowerEvent( GMEVENT* p_event, int* p_seq, void* p_work );

//----------------------------------------------------------------------------
/**
 *	@brief  CGEAR 電源イベント
 *
 *	@param	gsys  ゲームシステム
 *  
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_CGearPower( GAMESYS_WORK *gsys )
{
  GMEVENT* p_event = GMEVENT_Create( gsys, NULL, CGearPowerEvent, sizeof(CGEAR_POWER_EVENT_WK));
  CGEAR_POWER_EVENT_WK* p_wk =GMEVENT_GetEventWork(p_event);

  p_wk->p_fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  p_wk->p_subscreen = FIELDMAP_GetFieldSubscreenWork( p_wk->p_fieldmap );
  
  return p_event;
}





//-----------------------------------------------------------------------------
/**
 *      private
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  Cギア　電源　イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT CGearPowerEvent( GMEVENT* p_event, int* p_seq, void* p_work )
{
  CGEAR_POWER_EVENT_WK* p_wk = p_work;

  switch( *p_seq ){
  case SEQ_INIT:
    FIELD_SUBSCREEN_Change(p_wk->p_subscreen, FIELD_SUBSCREEN_CGEAR_ONOFF);
    (*p_seq) ++;
    break;

  case SEQ_MAIN:
    {
      FIELD_SUBSCREEN_ACTION action;

      action = FIELD_SUBSCREEN_GetAction( p_wk->p_subscreen );
      if( action == FIELD_SUBSCREEN_ACTION_CGEAR_POWER_EXIT ){
        FIELD_SUBSCREEN_ResetAction( p_wk->p_subscreen );
        (*p_seq) ++;
      }
    }
    break;

  case SEQ_EXIT:
	  FIELD_SUBSCREEN_Change( p_wk->p_subscreen, FIELD_SUBSCREEN_NORMAL );
		return GMEVENT_RES_FINISH;

  default:
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

