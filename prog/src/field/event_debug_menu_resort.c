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
#include "msg/debug/msg_debug_resort2.h"

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
#include "resonance_resort/resort_shop.h"

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
static BOOL debugMenuCallProc_DesignerSetup( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_PresenSetup( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_PresenRecvAdd( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList[] =
{
  { DEBUG_FIELD_RESORT_18, debugMenuCallProc_ChangeMap, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_38, debugMenuCallProc_DesignerSetup, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_39, debugMenuCallProc_PresenSetup, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_40, debugMenuCallProc_PresenRecvAdd, GFL_OVERLAY_BLANK_ID },
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
        RESORT_SECRETARY_DATA_SetRecv( p_dst, p_recv, 8+i, RESORT_MAP_GetSecretaryPlace( i ), i );
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
      RESORT_RESIDENT_DATA_SetRecv( p_dst, p_recv, i, RESORT_PLACEID_INSTITUTION_00+i, i );
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
static BOOL debugMenuCallProc_DesignerSetup( DEBUG_MENU_EVENT_WORK *p_wk )
{
  //まず住人を全部いれる
  debugMenuCallProc_AllResident( p_wk );

  //0,1,2をマーケットLv1,2,3,にする
  {
    SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
    RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
    RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
    int i;
    for( i = 0; i < 3; i++ )
    {
      RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident(p_all_people,i);
      RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, i );
      RESORT_RESIDENT_DATA_SetData( p_resident, RESORT_DATA_PARAM_WANT_JOB, RESORT_RESIDENT_TYPE_MARKET );
      RESORT_RESIDENT_DATA_JobChange( p_resident );
    }
  }

  return FALSE;
}
static void PresenSetup_SetRecv( RESORT_RECV_DATA *p_recv, GFL_MSGDATA *p_msg, u32 people_type, int cnt )
{
  typedef struct
  {
    u8  name;
    u8  phrase;
    u8  commID;
    u8  view;
    u8  shop_id;
    u8  shop_type;
    u8  placeID;
    u8  dummy;
  } PRESEN_DATA;

  static const PRESEN_DATA sc_tourist_data[]  =
  {
    //旅行者
    {
      RESORT_NAME_00,
      RESORT_PHRASE_00,
      RESORT_COMM_TYPE_BEACON,
      9,
    },
    {
      RESORT_NAME_01,
      RESORT_PHRASE_01,
      RESORT_COMM_TYPE_CGEAR_IRC,
      1,
    },
    {
      RESORT_NAME_02,
      RESORT_PHRASE_02,
      RESORT_COMM_TYPE_UNION,
      2,
    },
    {
      RESORT_NAME_03,
      RESORT_PHRASE_03,
      RESORT_COMM_TYPE_RNDMATCH,
      3,
    },
    {
      RESORT_NAME_04,
      RESORT_PHRASE_04,
      RESORT_COMM_TYPE_GTSNEGO,
      4,
    },
    {
      RESORT_NAME_05,
      RESORT_PHRASE_05,
      RESORT_COMM_TYPE_HYLINK,
      1,
    },
    {
      RESORT_NAME_06,
      RESORT_PHRASE_06,
      RESORT_COMM_TYPE_BEACON,
      12,
    },
    {
      RESORT_NAME_07,
      RESORT_PHRASE_07,
      RESORT_COMM_TYPE_CGEAR_IRC,
      7,
    },
    {
      RESORT_NAME_08,
      RESORT_PHRASE_08,
      RESORT_COMM_TYPE_UNION,
      0,
    },
    {
      RESORT_NAME_09,
      RESORT_PHRASE_09,
      RESORT_COMM_TYPE_RNDMATCH,
      1,
    },
    {
      RESORT_NAME_10,
      RESORT_PHRASE_10,
      RESORT_COMM_TYPE_GTSNEGO,
      14,
    },
    {
      RESORT_NAME_11,
      RESORT_PHRASE_11,
      RESORT_COMM_TYPE_HYLINK,
      3,
    },
    {
      RESORT_NAME_12,
      RESORT_PHRASE_12,
      RESORT_COMM_TYPE_BEACON,
      4,
    },
    {
      RESORT_NAME_13,
      RESORT_PHRASE_13,
      RESORT_COMM_TYPE_CGEAR_IRC,
      5,
    },
    {
      RESORT_NAME_14,
      RESORT_PHRASE_14,
      RESORT_COMM_TYPE_UNION,
      5,
    },
    {
      RESORT_NAME_15,
      RESORT_PHRASE_15,
      RESORT_COMM_TYPE_RNDMATCH,
      7,
    },
    //番兵
    {
      0
    },
  };

  static const PRESEN_DATA sc_secretary_data[]  =
  {
    //秘書
    {
      RESORT_NAME_16,
      RESORT_PHRASE_16,
      RESORT_COMM_TYPE_BEACON,
      0,
    },
    {
      RESORT_NAME_17,
      RESORT_PHRASE_17,
      RESORT_COMM_TYPE_BEACON,
      1,
    },
    {
      RESORT_NAME_18,
      RESORT_PHRASE_18,
      RESORT_COMM_TYPE_BEACON,
      6,
    },
    {
      RESORT_NAME_19,
      RESORT_PHRASE_19,
      RESORT_COMM_TYPE_BEACON,
      3,
    },
    {
      RESORT_NAME_20,
      RESORT_PHRASE_20,
      RESORT_COMM_TYPE_BEACON,
      4,
    },
  };

  static const PRESEN_DATA sc_resident_data[]  =
  {
    //住人
    {
      RESORT_NAME_21,
      RESORT_PHRASE_21,
      RESORT_COMM_TYPE_BEACON,
      1,
      RESORT_SHOP_ID_MARKET_00,
      RESORT_RESIDENT_TYPE_MARKET,
      RESORT_PLACEID_INSTITUTION_00,
    },
    {
      RESORT_NAME_22,
      RESORT_PHRASE_22,
      RESORT_COMM_TYPE_BEACON,
      2,
      RESORT_SHOP_ID_PARLOR_00,
      RESORT_RESIDENT_TYPE_PARLOR,
      RESORT_PLACEID_INSTITUTION_01,
    },
    {
      RESORT_NAME_23,
      RESORT_PHRASE_23,
      RESORT_COMM_TYPE_BEACON,
      4,
      RESORT_SHOP_ID_BREEDER_00,
      RESORT_RESIDENT_TYPE_BREEDER,
      RESORT_PLACEID_INSTITUTION_02,
    },
  };

  int i;

  const PRESEN_DATA * sc_presen_data;

  switch( people_type )
  {
  case RESORT_PEOPLE_TYPE_TOURIST:
    sc_presen_data = sc_tourist_data;
    GF_ASSERT( cnt < NELEMS(sc_tourist_data) );
    break;
  case RESORT_PEOPLE_TYPE_SECRETARY:
    sc_presen_data = sc_secretary_data;
    GF_ASSERT( cnt < NELEMS(sc_secretary_data) );
    break;
  case RESORT_PEOPLE_TYPE_RESIDENT:
    sc_presen_data = sc_resident_data;
    GF_ASSERT( cnt < NELEMS(sc_resident_data) );
    break;
  default:
    GF_ASSERT(0);
  }

  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_TRAINER_ID, people_type*1000 + cnt );
  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_TRAINERVIEW, sc_presen_data[cnt].view );
  if( sc_presen_data[cnt].view < 8 )
  {
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_SEX, PM_MALE );
  }
  else
  {
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_SEX, PM_FEMALE );
  }

  //名前
  {
    STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, sc_presen_data[cnt].name );
    const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(p_strbuf);
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_NAME, (u32)cp_strcode );
    GFL_STR_DeleteBuffer( p_strbuf );
  }

  //口癖
  {
    STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, sc_presen_data[cnt].phrase );
    const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(p_strbuf);
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_INTRODUCTION, (u32)cp_strcode );
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_EXT9_PHRASE, (u32)cp_strcode );
    GFL_STR_DeleteBuffer( p_strbuf );
  }

  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_EXTEND_BIT, 0x1FF );
  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_VER, PM_VERSION );
  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_LANG, PM_LANG );
  RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_COMM_ID, sc_presen_data[cnt].commID );

  //クチコミ
  for( i = RESORT_DATA_PARAM_EXT4_GOSSIP_00; i <= RESORT_DATA_PARAM_EXT4_GOSSIP_15; i++ )
  {
    RESORT_RECV_DATA_SetData( p_recv, i, GFUser_GetPublicRand(3) );
  }

  //オフレコ
  {
    u8  bit[12];
    for( i  = 0; i < NELEMS(bit); i++ )
    {
      bit[i]  = GFUser_GetPublicRand( 0xFF );
    }
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_EXT1_MEDAL_BIT, (u32)bit );
  }

  //お店
  if( RESORT_PLACEID_INSTITUTION_00 <= sc_presen_data[cnt].placeID 
    && sc_presen_data[cnt].placeID <= RESORT_PLACEID_INSTITUTION_07 )
  {

    RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_recv );
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, sc_presen_data[cnt].shop_id );
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_TYPE, sc_presen_data[cnt].shop_type );
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_WANT_JOB, sc_presen_data[cnt].shop_type );
    RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_PLACE_ID, sc_presen_data[cnt].placeID );
  }

  //受信日時
  {
    RTCDate date;
    RTCTime time;
    GFL_RTC_GetDateTime(&date, &time);
    RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_MEET_YEAR, date.year );
    RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_MEET_MONTH, date.month );
    RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_MEET_DAY, date.day );
    RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_MEET_HOUR, time.hour );
    RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_MEET_MIN, time.minute );
  }
}

static BOOL debugMenuCallProc_PresenSetup( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );

  GFL_MSGDATA *p_msg = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_DEBUG_MESSAGE,NARC_debug_message_debug_resort2_dat, GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  int i;

  //まずはデータクリア
  debugMenuCallProc_AllClear( p_wk );

  //お金を最大に
  {
    MISC *p_misc = SaveData_GetMisc(pSave);
    MISC_SetGold(p_misc, MY_GOLD_MAX);
  }

  //受信バッファに入れる
  debugMenuCallProc_PresenRecvAdd(p_wk);
 
  //秘書への追加
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
    PresenSetup_SetRecv( p_recv, p_msg, RESORT_PEOPLE_TYPE_SECRETARY, i );
    RESORT_SECRETARY_DATA_SetRecv( p_dst, p_recv, 16+i, RESORT_MAP_GetSecretaryPlace( i ), i );
  }

  //住人への追加
  for( i = 0; i < 3; i++ )
  {
    RESORT_RESIDENT_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
    PresenSetup_SetRecv( p_dst, p_msg, RESORT_PEOPLE_TYPE_RESIDENT, i );
  }

  //自分の街、自分のよびな
  {
    RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort );
    MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus(p_wk->gdata);

    STRBUF *p_myname = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(p_wk->heapID) );
    const STRCODE *cp_strcode;
    MyStatus_CopyNameString( p_mystatus, p_myname );

    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, RESORT_CITY_00 + GFUser_GetPublicRand(10) );

      GFL_STR_AddString( p_myname, p_strbuf );
      cp_strcode = GFL_STR_GetStringCodePointer(p_myname);
      RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_CITY_NAME, (u32)cp_strcode );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    MyStatus_CopyNameString( p_mystatus, p_myname );
    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, RESORT_AUTHER_00 + GFUser_GetPublicRand(5) );
      GFL_STR_AddString( p_myname, p_strbuf );
      cp_strcode = GFL_STR_GetStringCodePointer(p_myname);
      RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_AUTHER_NAME, (u32)cp_strcode );
      GFL_STR_DeleteBuffer( p_strbuf );
    }
    {
      STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, RESORT_MYPHRASE_00 + GFUser_GetPublicRand(5) );
      RESORT_RECV_DATA *p_own = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );
      cp_strcode = GFL_STR_GetStringCodePointer(p_strbuf);
      RESORT_RECV_DATA_SetData( p_own, RESORT_DATA_PARAM_EXT9_PHRASE, (u32)cp_strcode );
      GFL_STR_DeleteBuffer( p_strbuf );
    }

    GFL_STR_DeleteBuffer( p_myname );
  }

  RESORT_RECV_DATA_Exit( p_recv );
  GFL_MSG_Delete( p_msg );

  return FALSE;
}
static BOOL debugMenuCallProc_PresenRecvAdd( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );
  RESORT_RECV_BUFF *p_recv_tmp_buff = NULL;
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );

  GFL_MSGDATA *p_msg = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_DEBUG_MESSAGE,NARC_debug_message_debug_resort2_dat, GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  int cnt = 0;
  //旅行者への追加
  while( TRUE )
  {
    PresenSetup_SetRecv( p_recv, p_msg, RESORT_PEOPLE_TYPE_TOURIST, cnt++ );
    NAGI_Printf( "cnt = %d\n", cnt );
    if( !RESORT_PEOPLE_ALL_DATA_MergeRecv( p_all_people, p_recv, RESORT_RECV_DATA_GetData(p_recv,RESORT_DATA_PARAM_COMM_ID,NULL) ) )
    {
      if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv, RESORT_RECV_DATA_GetData(p_recv,RESORT_DATA_PARAM_COMM_ID,NULL) ) )
      {
        p_recv_tmp_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
        if( !p_recv_tmp_buff )
        {
          break;
        }
        else 
        { 
          if( !RESORT_RECV_BUFF_AddRecv( p_recv_tmp_buff, p_recv, RESORT_RECV_DATA_GetData(p_recv,RESORT_DATA_PARAM_COMM_ID,NULL) ) )
          {
            break;
          }
        }
      }
    }
  }

  RESORT_RECV_DATA_Exit( p_recv );
  GFL_MSG_Delete( p_msg );

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

