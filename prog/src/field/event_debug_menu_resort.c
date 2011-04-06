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
#include "event_mapchange.h"
#include "arc/fieldmap/zone_id.h"
#include "resonance_resort/resort_debug_input.h"
#include "app/name_input.h"
#include "event_fieldmap_control.h"
#include "event_name_input.h"
#include "resonance_resort/resort_map.h"

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuListEvent(GMEVENT *event, int *seq, void *wk );
static void DEBUG_EVENT_DebugMenu_CallSubMenu( DEBUG_MENU_EVENT_WORK*p_wk, const DEBUG_MENU_INITIALIZER *cp_menu_data );

//--------------------------------------------------------------
/// 大元
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ChangeMap( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Input( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_SetRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_SetProxy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearRecvBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearProxyBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllProxy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllSecretary( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllResident( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllClear( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_UpdateDay( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList[] =
{
  { DEBUG_FIELD_RESORT_18, debugMenuCallProc_ChangeMap, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_16, debugMenuCallProc_Input, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_01, debugMenuCallProc_SetRecv, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_05, debugMenuCallProc_AllRecv, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_02, debugMenuCallProc_SetProxy, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_06, debugMenuCallProc_AllProxy, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_03, debugMenuCallProc_ClearRecvBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_04, debugMenuCallProc_ClearProxyBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_08, debugMenuCallProc_AllSecretary, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_09, debugMenuCallProc_AllResident, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_10, debugMenuCallProc_AllClear, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_11, debugMenuCallProc_UpdateDay, GFL_OVERLAY_BLANK_ID},

};

static const DEBUG_MENU_INITIALIZER DebugSubMenuData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubMenuList),
  DATA_SubMenuList,
  &DATA_DebugMenuList_Default,
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/// 移動
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MoveField( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveReceotion( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveEmbassy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution0( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution1( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution2( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution3( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution4( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution5( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution6( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveInstitution7( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium0( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium1( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium2( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium3( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium4( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MoveCondominium5( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList_Move[] =
{
  { DEBUG_FIELD_RESORT_21, debugMenuCallProc_MoveField, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_22, debugMenuCallProc_MoveReceotion, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_23, debugMenuCallProc_MoveEmbassy, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_24, debugMenuCallProc_MoveInstitution0, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_25, debugMenuCallProc_MoveInstitution1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_26, debugMenuCallProc_MoveInstitution2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_27, debugMenuCallProc_MoveInstitution3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_28, debugMenuCallProc_MoveInstitution4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_29, debugMenuCallProc_MoveInstitution5, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_30, debugMenuCallProc_MoveInstitution6, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_31, debugMenuCallProc_MoveInstitution7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_32, debugMenuCallProc_MoveCondominium0, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_33, debugMenuCallProc_MoveCondominium1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_34, debugMenuCallProc_MoveCondominium2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_35, debugMenuCallProc_MoveCondominium3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_36, debugMenuCallProc_MoveCondominium4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_37, debugMenuCallProc_MoveCondominium5, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugSubMenuData_Move = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubMenuList_Move),
  DATA_SubMenuList_Move,
  &DATA_DebugMenuList_Default,
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/// 入力
//--------------------------------------------------------------
static BOOL debugMenuCallProc_CallDebugInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallCityNameInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallOwnerNameInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallOwnerPhraseInput( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList_Input[] =
{
  { DEBUG_FIELD_RESORT_12, debugMenuCallProc_CallDebugInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_13, debugMenuCallProc_CallCityNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_14, debugMenuCallProc_CallOwnerNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_15, debugMenuCallProc_CallOwnerPhraseInput, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugSubMenuData_Input = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubMenuList_Input),
  DATA_SubMenuList_Input,
  &DATA_DebugMenuList_Default,
  1, 1, 16, 17,
  NULL,
  NULL
};


//--------------------------------------------------------------
/// struct
//--------------------------------------------------------------
typedef struct
{
  DEBUG_MENU_EVENT_WORK ev_work;
  u16 list_pos;
  u16 cursor_pos;
  const DEBUG_MENU_INITIALIZER *cp_menu_data;
}DEBUG_MENU_SUB_EVENT_WORK;

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
  DEBUG_MENU_SUB_EVENT_WORK   *work;
  GMEVENT *event;

  event = GMEVENT_Create( gsys, debug_work->gmEvent,
    debugMenuListEvent, sizeof(DEBUG_MENU_SUB_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_SUB_EVENT_WORK) );

  work->ev_work  = *debug_work;
  work->ev_work.gmEvent = event;
  work->cp_menu_data = &DebugSubMenuData;

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
  DEBUG_MENU_SUB_EVENT_WORK *p_wk = wk;
  DEBUG_MENU_EVENT_WORK *work = &p_wk->ev_work;

  switch( (*seq) ){
  case 0:
    work->call_proc = NULL;
    work->menuFunc = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID, p_wk->cp_menu_data, p_wk->list_pos, p_wk->cursor_pos );
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
    FLDMENUFUNC_GetListCursor( work->menuFunc, &p_wk->list_pos, &p_wk->cursor_pos );
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    (*seq)++;
    break;

  case 3:
    {

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

//----------------------------------------------------------------------------
/**
 *	@brief  サブメニューを呼び出し
 *
 *	@param	DEBUG_MENU_EVENT_WORK*p_wk  ワーク
 *	@param	DEBUG_MENU_INITIALIZER *cp_menu_data  メニューデータ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static void DEBUG_EVENT_DebugMenu_CallSubMenu( DEBUG_MENU_EVENT_WORK*p_wk, const DEBUG_MENU_INITIALIZER *cp_menu_data )
{
  GMEVENT *event = GMEVENT_Create( p_wk->gmSys, p_wk->gmEvent,
    debugMenuListEvent, sizeof(DEBUG_MENU_SUB_EVENT_WORK) );

  {
    DEBUG_MENU_SUB_EVENT_WORK   *work = GMEVENT_GetEventWork( event );
    GFL_STD_MemClear( work, sizeof(DEBUG_MENU_SUB_EVENT_WORK) );
    work->ev_work = *p_wk;

    work->ev_work.gmEvent = event;
    work->cp_menu_data = cp_menu_data;
  }

  GMEVENT_CallEvent( p_wk->gmEvent, event );


  //親に戻ってきた時に親が最初から動くように
  {
    int *p_seq = GMEVENT_GetSequenceWork(p_wk->gmEvent);
    *p_seq = 0;
  }
}

static BOOL debugMenuCallProc_ChangeMap( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DEBUG_EVENT_DebugMenu_CallSubMenu( p_wk, &DebugSubMenuData_Move );
  return TRUE;
}
static BOOL debugMenuCallProc_Input( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DEBUG_EVENT_DebugMenu_CallSubMenu( p_wk, &DebugSubMenuData_Input );
  return TRUE;
}



static BOOL debugMenuCallProc_SetRecv( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
  if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv, RESORT_COMM_TYPE_BEACON ) )
  {
    p_recv_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
    if( p_recv_buff )
    {
      RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv, RESORT_COMM_TYPE_BEACON );
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
  RESORT_RECV_BUFF *p_recv_tmp_buff = NULL;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while( TRUE )
  {
    RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
    if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv, RESORT_COMM_TYPE_BEACON ) )
    {
      p_recv_tmp_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
      if( !p_recv_tmp_buff )
      {
        break;
      }
      else 
      { 
        if( !RESORT_RECV_BUFF_AddRecv( p_recv_tmp_buff, p_recv, RESORT_COMM_TYPE_BEACON ) )
        {
          break;
        }
      }
    }
  }
  RESORT_RECV_DATA_Exit( p_recv );
  return FALSE;
}
static BOOL debugMenuCallProc_AllProxy( DEBUG_MENU_EVENT_WORK *p_wk )
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
static BOOL debugMenuCallProc_MoveField( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_FIELD );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveReceotion( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_RECEPTION );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveEmbassy( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_EMBASSY );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution0( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_00 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution1( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_01 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution2( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_02 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution3( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_03 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution4( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_04 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution5( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_05 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution6( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_06 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveInstitution7( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_INSTITUTION_07 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium0( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_00 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium1( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_01 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium2( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_02 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium3( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_03 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium4( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_04 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}
static BOOL debugMenuCallProc_MoveCondominium5( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, RESORT_ZONE_ID_CONDOMINIUM_05 );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}

static BOOL debugMenuCallProc_AllSecretary( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
  int i;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );

  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
    if( RESORT_SECRETARY_DATA_IsEmpty( p_dst ) )
    {
      RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
      RESORT_SECRETARY_DATA_SetRecv( p_dst, p_recv, 8+i, 1, i );
    }
  }

  RESORT_RECV_DATA_Exit( p_recv );
  return FALSE;
}
static BOOL debugMenuCallProc_AllResident( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
  int i;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );

  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
    if( RESORT_RESIDENT_DATA_IsEmpty( p_dst ) )
    {
      RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
      RESORT_RESIDENT_DATA_SetRecv( p_dst, p_recv, i, 1, i );
    }
  }

  RESORT_RECV_DATA_Exit( p_recv );
  return FALSE;
}
static BOOL debugMenuCallProc_AllClear( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );

  //バッファクリア
  {
    RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );
    RESORT_RECV_BUFF_InitInstance( p_recv_buff, RESONANCE_RESORT_SV_RECV_BUFF_NUM );
  }
  {
    RESORT_PROXY_BUFF *p_proxy_buff = RESONANCE_RESORT_SV_GetProxyBuff( p_resort );
    RESORT_PROXY_BUFF_InitInstance( p_proxy_buff, RESONANCE_RESORT_SV_PROXY_BUFF_NUM );
  }
  //街データクリア
  {
    RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort );
    RESORT_CITY_DATA_InitInstance( p_city );
  }
  //人物データクリア
  {
    RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
    RESORT_PEOPLE_ALL_DATA_InitInstance( p_all_people );
  }
  return FALSE;
}
static BOOL debugMenuCallProc_UpdateDay( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  //街データ
  {
    RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort );
    RESORT_CITY_DATA_UpdateDay( p_city );
  }
  //人物データ
  {
    RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
    RESORT_PEOPLE_ALL_DATA_UpdateDay( p_all_people );
  }
  return FALSE;
}
static BOOL debugMenuCallProc_CallDebugInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT *event = EVENT_CallDebugInputResort( p_wk->gmSys, NULL );
  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}

static BOOL debugMenuCallProc_CallCityNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_CITY_NAME, p_wk->gmSys, NULL ) );

  return TRUE;
}
static BOOL debugMenuCallProc_CallOwnerNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_NAME, p_wk->gmSys, NULL ) );

  return TRUE;
}

static BOOL debugMenuCallProc_CallOwnerPhraseInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  //GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_PHASE, p_wk->gmSys, NULL ) );
  
  DEBUG_EVENT_DebugMenu_CallSubMenu( p_wk, &DebugSubMenuData );

  return TRUE;
}

