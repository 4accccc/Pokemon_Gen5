//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		prog\src\field\event_wbt_wifi.c
 *	@brief  WBTのWIFIイベント
 *	@author	Toru=Nagihashi
 *	@data		2011.08.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem\btl_setup.h"
#include "event_fieldmap_control.h"
#include "event_fldmmdl_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "app/leader_board.h"
#include "battle/battle.h"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"

#include "savedata/save_tbl.h"

#include "fieldmap.h"

#include "wbt_setup.h"
#include "wbt_system.h"

#include "event_wbt.h"

#include "system/net_err.h"

#include "app/win_record.h"
#include "app/t_download.h"
#include "app/pkwd_count.h"
#include "net_app/wbt_download_sys.h"

#include "event_wbt_wifi.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================


//============================================================================================
//
//    T_DOWNLOAD画面呼び出し
//
//============================================================================================
//-------------------------------------
///   T_DOWNLOAD定義
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  WBT_DOWNLOAD_SYS_PARAM *td_param;
} T_DOWNLOAD_EVENT_WORK;

static GMEVENT_RESULT eventTDownload( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  T_DOWNLOAD呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * T_DOWNLOAD_EVENT_ShowWifi( GAMESYS_WORK * gsys, void *p_adrs )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  T_DOWNLOAD_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventTDownload, sizeof(T_DOWNLOAD_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(T_DOWNLOAD_EVENT_WORK) );

  p_param->gsys			= gsys;
  p_param->fieldmap		= fieldmap;
  p_param->td_param 	= GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(WBT_DOWNLOAD_SYS_PARAM));
  {
    WBT_DOWNLOAD_SYS_PARAM *p_wbt_param = p_param->td_param;
    p_wbt_param->p_gamesys = gsys;
    p_wbt_param->p_gamedata = GAMESYSTEM_GetGameData(gsys);
  }

  OS_Printf( "T Download Start\n" );


  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  return new_event;

}
//----------------------------------------------------------------------------
/**
 *  @brief  T_DOWNLOAD呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventTDownload( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_COMM_CHECK,
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  T_DOWNLOAD_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_COMM_CHECK:
    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(p_wk->gsys)))
    {
      *p_seq  = SEQ_CALL_PROC;
    }
    break;
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID(wbt_download), &WBT_DOWNLOAD_SYS_ProcData, p_wk->td_param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    GFL_HEAP_FreeMemory( p_wk->td_param );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

