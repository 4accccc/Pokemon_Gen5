/**
 *	@file		event_cgearget.c
 *	@brief  CGEARゲットイベント
 *	@author	k.ohno
 *	@date		2010.02.22
 */

#include <gflib.h>
#include "event_cgearget.h"
#include "../fieldmap.h"
#include "../field_subscreen.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	シーケンス
//=====================================
typedef enum {
  SEQ_CGEAR_START,
  SEQ_CGEAR_LOOP,
  SEQ_END,
} _SEQ_CGEAR_STEP;



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

typedef struct{
  int end;
   GAMESYS_WORK* gsys ;
} CGEARGET_EVENT_WORK;


static void _endCallback(void *wk)
{
  CGEARGET_EVENT_WORK* work=wk;

  work->end=TRUE;

}


//--------------------------------------------------------------
/**
 * WiFiBsubway  イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CGEARGET_Proc( GMEVENT *event, int *seq, void *wk )
{
  CGEARGET_EVENT_WORK* work = wk;
  GAMESYS_WORK* gsys = work->gsys;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_SUBSCREEN_WORK* pSubWork = FIELDMAP_GetFieldSubscreenWork(fieldmap);
  
  switch( *seq )
  {
  case SEQ_CGEAR_START:
    FIELD_SUBSCREEN_CgearFirst(pSubWork,
                               FIELD_SUBSCREEN_CGEARFIRST,
                               &_endCallback,
                               (void*)work);
    (*seq)++;
    break;

  case SEQ_CGEAR_LOOP:
    if(work->end){
      (*seq)++;
    }
    break;
  case SEQ_END:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}


//----------------------------------------------------------------------------
/**
 *	@brief  CGEARイベント 生成
 *	@param	gsys    ゲームシステム
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * CGEARGET_EVENT_Start( GAMESYS_WORK *gsys )
{
  GMEVENT * p_event;
  GAMEDATA * p_gdata;
  CGEARGET_EVENT_WORK * p_wk;

  p_gdata = GAMESYSTEM_GetGameData( gsys );
  p_event = GMEVENT_Create( gsys, NULL,  CGEARGET_Proc, sizeof(CGEARGET_EVENT_WORK) );

  p_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear(p_wk ,sizeof(CGEARGET_EVENT_WORK));
  p_wk->gsys = gsys;

  return p_event;
}

#if PM_DEBUG
//------------------------------------------------------------------
/*
 *  @brief  イベントチェンジ
 */
//------------------------------------------------------------------
void CGEARGET_EVENT_Change(GAMESYS_WORK * gsys,GMEVENT * event)
{
  CGEARGET_EVENT_WORK * p_wk;

  GMEVENT_Change( event, CGEARGET_Proc, sizeof(CGEARGET_EVENT_WORK) );
  p_wk = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear(p_wk ,sizeof(CGEARGET_EVENT_WORK));
  p_wk->gsys = gsys;

}
#endif
