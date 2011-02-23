//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_resort.c
 *	@brief  レゾナンスリゾート用デバッグメニュー
 *	@author		Toru=Nagihashi
 *	@data		2011.02.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "fieldmap.h"
#include "gamesystem/game_beacon.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"

#include "item/itemsym.h"
#include "event_debug_menu_resort.h"
#include "savedata/resonance_resort_sv.h"

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuListEvent(GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuCallProc_SetRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_SetProxy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearRecvBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearProxyBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllProx( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList[] =
{
  { DEBUG_FIELD_RESORT_01, debugMenuCallProc_SetRecv, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_02, debugMenuCallProc_SetProxy, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_03, debugMenuCallProc_ClearRecvBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_04, debugMenuCallProc_ClearProxyBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_05, debugMenuCallProc_AllRecv, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_06, debugMenuCallProc_AllProx, GFL_OVERLAY_BLANK_ID},
};

static const DEBUG_MENU_INITIALIZER DebugSubMenuData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubMenuList),
  DATA_SubMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：レゾナンスリゾート
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_Resort( GAMESYS_WORK *gsys, void *wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  DEBUG_MENU_EVENT_WORK   *work;
  GMEVENT *event;

  event = GMEVENT_Create( gsys, debug_work->gmEvent,
    debugMenuListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *work  = *debug_work;
  work->gmEvent = event;
  work->call_proc = NULL;

  work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugSubMenuData );

  return event;
}

//--------------------------------------------------------------
/**
 * イベント：リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else if( ret != FLDMENUFUNC_CANCEL ){  //決定
        work->call_proc = (DEBUG_MENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;

  case 2:
    {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( work->call_proc != NULL ){
        if( work->call_proc(work) == TRUE ){
          return( GMEVENT_RES_CONTINUE );
        }
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}
static BOOL debugMenuCallProc_SetRecv( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
  if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv ) )
  {
    p_recv_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
    if( p_recv_buff )
    {
      RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv );
    }
  }
  RESORT_RECV_DATA_Exit( p_recv );
  return FALSE;
}
static BOOL debugMenuCallProc_SetProxy( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PROXY_BUFF *p_proxy_buff = RESONANCE_RESORT_SV_GetProxyBuff( p_resort );

  RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_PROXY_DATA_SetCommData( p_proxy, 0 );
  RESORT_PROXY_BUFF_Add( p_proxy_buff, p_proxy );
  RESORT_PROXY_DATA_Exit( p_proxy );
  return FALSE;
}
static BOOL debugMenuCallProc_ClearRecvBuff( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );

  RESORT_RECV_BUFF_InitInstance( p_recv_buff, RESONANCE_RESORT_SV_RECV_BUFF_NUM );
  return FALSE;
}
static BOOL debugMenuCallProc_ClearProxyBuff( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PROXY_BUFF *p_proxy_buff = RESONANCE_RESORT_SV_GetProxyBuff( p_resort );

  RESORT_PROXY_BUFF_InitInstance( p_proxy_buff, RESONANCE_RESORT_SV_PROXY_BUFF_NUM );
  return FALSE;
}
static BOOL debugMenuCallProc_AllRecv( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while( TRUE )
  {
    RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
    if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv ) )
    {
      p_recv_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
      if( p_recv_buff )
      {
        if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv ) )
        {
          break;
        }
      }
    }
  }
  RESORT_RECV_DATA_Exit( p_recv );
  return FALSE;
}
static BOOL debugMenuCallProc_AllProx( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PROXY_BUFF *p_proxy_buff = RESONANCE_RESORT_SV_GetProxyBuff( p_resort );

  RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while(TRUE)
  {
    RESORT_PROXY_DATA_SetCommData( p_proxy, 0 );
    if( !RESORT_PROXY_BUFF_Add( p_proxy_buff, p_proxy ) )
    {
      break;
    }
  }
  RESORT_PROXY_DATA_Exit( p_proxy );
  return FALSE;
}
