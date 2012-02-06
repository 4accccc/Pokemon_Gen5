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
#ifdef  PM_DEBUG

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "fieldmap.h"
#include "gamesystem/game_beacon.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "msg/debug/msg_debug_resort2.h"
#include "arc/script_message.naix"
#include "msg/script/msg_resort_scr.h"

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
#include "resonance_resort/resort_binary.h"
#include "resonance_resort/resort_shop.h"
#include "resonance_resort/resort_sys.h"
#include "resonance_resort/resort_field.h"
#include "resonance_resort/resort_people.h"
#include "resonance_resort/resort_data_manager.h"
#include "resonance_resort/resort_util.h"
#include "resonance_resort/resort_npc.h"
#include "resonance_resort/resort_binary.h"
#include "net_app/union/union_beacon_tool.h"
#include "resonance_resort/resort_debug_inline.h"
#include "script.h"
#include "event_debug_resort_input.h"

#include "../../../resource/fldmapdata/script/resort_scr_def.h"


extern void RESORT_FIELD_AppearFun( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID );
extern void RESORT_FIELD_AppearCeleb( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID );
extern void RESORT_RESIDENT_DATA_JobChange( RESORT_RESIDENT_DATA *p_wk, const RESORT_BINARY_SHOP *cp_shop );
extern void RESORT_RESIDENT_DATA_ConvertRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );
extern void RESORT_RESIDENT_DATA_ConvertProxy( RESORT_RESIDENT_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );

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
static BOOL debugMenuCallProc_DesignerSetup1( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Script( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AddFun( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AddCeleb( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList[] =
{
  //システム
  { DEBUG_FIELD_RESORT_45, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_16, debugMenuCallProc_Input, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_18, debugMenuCallProc_ChangeMap, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_44, debugMenuCallProc_Script, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_10, debugMenuCallProc_AllClear, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_11, debugMenuCallProc_UpdateDay, GFL_OVERLAY_BLANK_ID},

  //人物操作
  { DEBUG_FIELD_RESORT_48, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_01, debugMenuCallProc_SetRecv, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_05, debugMenuCallProc_AllRecv, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_02, debugMenuCallProc_SetProxy, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_06, debugMenuCallProc_AllProxy, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_03, debugMenuCallProc_ClearRecvBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_04, debugMenuCallProc_ClearProxyBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_08, debugMenuCallProc_AllSecretary, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_09, debugMenuCallProc_AllResident, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_50, debugMenuCallProc_AddFun, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_49, debugMenuCallProc_AddCeleb, GFL_OVERLAY_BLANK_ID},
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

static const DEBUG_MENU_LIST DATA_SubMenuList_Move[] =
{
  { DEBUG_FIELD_RESORT_21, debugMenuCallProc_MoveField, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_22, debugMenuCallProc_MoveReceotion, GFL_OVERLAY_BLANK_ID },
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
static BOOL debugMenuCallProc_CallOwnerThanksInput( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList_Input[] =
{
  { DEBUG_FIELD_RESORT_12, debugMenuCallProc_CallDebugInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_13, debugMenuCallProc_CallCityNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_14, debugMenuCallProc_CallOwnerNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_15, debugMenuCallProc_CallOwnerPhraseInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_43, debugMenuCallProc_CallOwnerThanksInput, GFL_OVERLAY_BLANK_ID },
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


typedef struct
{
  u32 seq;
  s32 num;
  GFL_MSGDATA * msgData;
  FLDSYSWIN * win;
  FIELDMAP_WORK *fieldWork;
  HEAPID heapID;
}NUM_INPUT;

#define NUMINPUT_NONE (0xFFFF)
static s32 NumInput_State( NUM_INPUT *p_wk, FIELDMAP_WORK *fieldWork, s32 default_num, s32 min, s32 max, HEAPID heapID);
static void NumInput_Draw( NUM_INPUT *p_wk );

static BOOL Debig_Menu_Resort_ChangeOutZone( DEBUG_MENU_EVENT_WORK*p_wk, u16 checkID, u16 nextID )
{
  if( FIELDMAP_GetZoneID( p_wk->fieldWork) == checkID )
  {
    GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
        p_wk->fieldWork, nextID );
    GMEVENT_ChangeEvent( p_wk->gmEvent, event );
    return TRUE;
  }

  return FALSE;
}

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

static struct 
{
  int seq;
  NUM_INPUT num_input;
} s_debug_wk  = {0};

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

  GFL_STD_MemClear( &s_debug_wk, sizeof(s_debug_wk) );

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
        s_debug_wk.seq = 0;
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

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  Debug_Resort_SetRecv( p_recv, p_wk->gdata );

  RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_recv, RESORT_MERGE_TYPE_RECV );

  RESORT_RECV_DATA_Exit( p_recv );

  return FALSE;
}

static void Debug_Resort_SetProxy( RESORT_PROXY_DATA *p_proxy, GAMEDATA *p_gamedata, HEAPID heapID )
{
  const MYSTATUS  *cp_mystatus = GAMEDATA_GetMyStatus(p_gamedata);
  MYSTATUS  *p_status = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), MYSTATUS_SAVE_SIZE );

  int pokemon = 1+GFL_STD_MtRand(151+1);
  RESORT_COMM_TYPE  comm_type = GFL_STD_MtRand(2) ? RESORT_COMM_TYPE_GTS: RESORT_COMM_TYPE_PDW;

  GFL_STD_MemCopy( cp_mystatus, p_status, MYSTATUS_SAVE_SIZE );

  //名前セット
  {
    GFL_MSGDATA* p_msg = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_DEBUG_MESSAGE,NARC_debug_message_debug_resort2_dat, GFL_HEAP_LOWID(heapID) );
    
    STRBUF *p_strbuf = GFL_MSG_CreateString(p_msg, RESORT_NAME_00+GFL_STD_MtRand(20) );
    MyStatus_SetMyNameFromString( p_status, p_strbuf );
    GFL_STR_DeleteBuffer( p_strbuf );
    GFL_MSG_Delete( p_msg );
  }

  //トレーナーID
  MyStatus_SetID( p_status, GFL_STD_MtRand(GFL_STD_RAND_MAX) );

  //性別
  MyStatus_SetMySex( p_status, GFL_STD_MtRand(2) );

  //見た目
  MyStatus_SetTrainerView( p_status, MyStatus_GetMySex(p_status)*8 + GFL_STD_MtRand(8) );

  RESORT_PROXY_DATA_SetCommData( p_proxy, p_status, pokemon, comm_type );

  GFL_HEAP_FreeMemory( p_status );
}

static BOOL debugMenuCallProc_SetProxy( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );

  RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  Debug_Resort_SetProxy( p_proxy, p_wk->gdata, p_wk->heapID );
  RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_proxy, RESORT_MERGE_TYPE_PROXY );

  RESORT_PROXY_DATA_Exit( p_proxy );
  return FALSE;
}
static BOOL debugMenuCallProc_ClearRecvBuff( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );

  RESORT_RECV_BUFF_InitInstance( p_recv_buff, RESONANCE_RESORT_SV_RECV_BUFF_NUM );

  return Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_FIELD, ZONE_ID_R04 );
}
static BOOL debugMenuCallProc_ClearProxyBuff( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PROXY_BUFF *p_proxy_buff = RESONANCE_RESORT_SV_GetProxyBuff( p_resort );

  RESORT_PROXY_BUFF_InitInstance( p_proxy_buff, RESONANCE_RESORT_SV_PROXY_BUFF_NUM );
  return Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_FIELD, ZONE_ID_R04 );
}
static BOOL debugMenuCallProc_AllRecv( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );
  RESORT_RECV_BUFF *p_recv_tmp_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while( TRUE )
  {
    Debug_Resort_SetRecv( p_recv, p_wk->gdata );
    RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_recv, RESORT_MERGE_TYPE_RECV );

    if( p_recv_tmp_buff )
    {
      if( RESORT_RECV_BUFF_GetCnt(p_recv_buff) >= RESONANCE_RESORT_SV_RECV_BUFF_NUM
        && RESORT_RECV_BUFF_GetCnt(p_recv_tmp_buff) >= RESONANCE_RESORT_BUFF_RECV_EX_NUM )
      {
        break;
      }
    }
    else
    {
      if( RESORT_RECV_BUFF_GetCnt(p_recv_buff) >= RESONANCE_RESORT_SV_RECV_BUFF_NUM )
      {
        break;
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
  RESORT_CITY_DATA  *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort );

  BOOL is_tutorial = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_IS_TUTORIAL, NULL );

  RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while(TRUE)
  {
    Debug_Resort_SetProxy( p_proxy, p_wk->gdata, p_wk->heapID );
    RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_proxy, RESORT_MERGE_TYPE_PROXY );
    if( (RESORT_PROXY_BUFF_GetCnt( p_proxy_buff ) >= RESONANCE_RESORT_SV_PROXY_BUFF_NUM && !is_tutorial)
      || (RESORT_PROXY_BUFF_GetCnt( p_proxy_buff ) >= 11 && is_tutorial)
        )
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
        Debug_Resort_SetRecv( p_recv, p_wk->gdata );
        RESORT_SECRETARY_DATA_ConvertRecv( p_dst, p_recv, 8+i, RESORT_MAP_GetPeoplePlace( RESORT_PEOPLE_TYPE_SECRETARY ), i );
    }
  }

  RESORT_RECV_DATA_Exit( p_recv );

  return Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_RECEPTION, RESORT_ZONE_ID_FIELD );
}
static BOOL debugMenuCallProc_AllResident( DEBUG_MENU_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
  int i;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_BINARY_SHOP *p_shop =  RESORT_BINARY_SHOP_Init( GFL_HEAP_LOWID(p_wk->heapID) );

  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
    if( RESORT_RESIDENT_DATA_IsEmpty( p_dst ) )
    {
      Debug_Resort_SetRecv( p_recv, p_wk->gdata );
  
      RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_WANT_JOB, 1+GFL_STD_MtRand(RESORT_SHOP_ID_MAX-1) );

      RESORT_RESIDENT_DATA_ConvertRecv( p_dst, p_recv, p_shop, 63-i, RESORT_PLACEID_FIELD );
    }
  }
  RESORT_BINARY_SHOP_Exit(p_shop);

  RESORT_RECV_DATA_Exit( p_recv );

  return Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_FIELD, ZONE_ID_R04 );
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


  if( Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_FIELD, ZONE_ID_R04 ) )
  {
    return TRUE;
  }
  if( Debig_Menu_Resort_ChangeOutZone( p_wk, RESORT_ZONE_ID_RECEPTION, ZONE_ID_R04 ) )
  {
    return TRUE;
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
static BOOL debugMenuCallProc_DesignerSetup1( DEBUG_MENU_EVENT_WORK *p_wk )
{
  RESORT_BINARY_SHOP *p_shop =  RESORT_BINARY_SHOP_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  //まず住人を全部いれる
  debugMenuCallProc_AllResident( p_wk );

  {
    SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
    RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
    RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
    int i;
    for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
    {
      RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident(p_all_people,i);
      RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, 2 );
      RESORT_RESIDENT_DATA_SetData( p_resident, RESORT_DATA_PARAM_WANT_JOB, RESORT_SHOP_ID_CAFE_02_B2  );
      RESORT_RESIDENT_DATA_JobChange( p_resident, p_shop );
    }
  }

  RESORT_BINARY_SHOP_Exit(p_shop);

  return FALSE;
}
static BOOL debugMenuCallProc_Script( DEBUG_MENU_EVENT_WORK *p_wk )
{
  if( FIELDMAP_GetZoneID( p_wk->fieldWork) == RESORT_ZONE_ID_FIELD )
  {
    SCRIPT_ChangeScript( p_wk->gmEvent, SCRID_RESERVE_RESORT_DEBUG, NULL, p_wk->heapID );
    return TRUE;
  }
  else
  {
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }
}
static BOOL debugMenuCallProc_AddFun( DEBUG_MENU_EVENT_WORK *p_wk )
{
  enum
  {
    SEQ_NUM_INPUT,
    SEQ_FUN_SET,
  };

  if( FIELDMAP_GetZoneID( p_wk->fieldWork) != RESORT_ZONE_ID_FIELD )
  {
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }

  switch( s_debug_wk.seq )
  {
  case SEQ_NUM_INPUT:
    if( NUMINPUT_NONE != NumInput_State( &s_debug_wk.num_input, p_wk->fieldWork, 7, 7, 54, p_wk->heapID) )
    {
      s_debug_wk.seq++;
    }
    break;
  case SEQ_FUN_SET:
    {
      SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
      RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
      RESORT_SYSTEM *p_sys = RESORT_FIELD_GetResortSys( p_wk->fieldWork );
      RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID ));

      RESORT_UTIL_SetProxy( p_sys, p_wk->gdata, p_proxy, RESORT_PROXY_TYPE_FUN, s_debug_wk.num_input.num-1, p_wk->heapID );
       RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_proxy, RESORT_MERGE_TYPE_PROXY );
      RESORT_PROXY_DATA_Exit(p_proxy);

      return FALSE;
    }
    break;
  }


  return TRUE;
}

static BOOL debugMenuCallProc_AddCeleb( DEBUG_MENU_EVENT_WORK *p_wk )
{
  enum
  {
    SEQ_NUM_INPUT,
    SEQ_SET,
  };

  if( FIELDMAP_GetZoneID( p_wk->fieldWork) != RESORT_ZONE_ID_FIELD )
  {
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }

  switch( s_debug_wk.seq )
  {
  case SEQ_NUM_INPUT:
    if( NUMINPUT_NONE != NumInput_State( &s_debug_wk.num_input, p_wk->fieldWork, 55, 55, 62, p_wk->heapID) )
    {
      s_debug_wk.seq++;
    }
    break;
  case SEQ_SET:
    {
      SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
      RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( pSave );
      RESORT_SYSTEM *p_sys = RESORT_FIELD_GetResortSys( p_wk->fieldWork );
      RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID ));

      RESORT_UTIL_SetProxy( p_sys, p_wk->gdata, p_proxy, RESORT_PROXY_TYPE_CELEB, s_debug_wk.num_input.num-1, p_wk->heapID );
       RESONANCE_RESORT_SV_SetData( p_resort, p_wk->gdata, p_proxy, RESORT_MERGE_TYPE_PROXY );
      RESORT_PROXY_DATA_Exit(p_proxy);

      return FALSE;
    }
    break;
  }


  return TRUE;
}

static BOOL debugMenuCallProc_CallDebugInput( DEBUG_MENU_EVENT_WORK *p_wk )
{

  GMEVENT *event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
      FS_OVERLAY_ID(event_debug_resort_input), EVENT_CallDebugInputResort, NULL );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );
  return TRUE;
}

static BOOL debugMenuCallProc_CallCityNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_CITY_NAME, p_wk->gmSys, NULL, TRUE ) );

  return TRUE;
}
static BOOL debugMenuCallProc_CallOwnerNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_NAME, p_wk->gmSys, NULL, TRUE ) );

  return TRUE;
}

static BOOL debugMenuCallProc_CallOwnerPhraseInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_PHASE, p_wk->gmSys, NULL, TRUE ) );

  return TRUE;
}

static BOOL debugMenuCallProc_CallOwnerThanksInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_THANKS, p_wk->gmSys, NULL, TRUE ) );

  return TRUE;
}


#include "msg/debug/msg_debug_infinity.h"
//----------------------------------------------------------------------------
/**
 *	@brief  数字表示
 *
 *	@param	NUM_INPUT *p_wk ワーク
 *	@param	*fieldWork  フィールド
 *	@param	default_num デフォルトのあ対
 *	@param	min 最小値
 *	@param	max 最大値
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
static s32 NumInput_State( NUM_INPUT *p_wk, FIELDMAP_WORK *fieldWork, s32 default_num, s32 min, s32 max, HEAPID heapID)
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
  };

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      FLDMSGBG * msgBG  = FIELDMAP_GetFldMsgBG( fieldWork );
      p_wk->num = default_num;
      p_wk->heapID   = heapID;
      p_wk->fieldWork = fieldWork;

      p_wk->msgData = DEBUGFLDMENU_CreateMSGDATA(fieldWork, NARC_debug_message_debug_infinity_dat );
      p_wk->win = FLDSYSWIN_AddEx( msgBG, p_wk->msgData , 1, 1, 30, 2 );
      NumInput_Draw( p_wk );
      p_wk->seq++;
    }
    break;
  case SEQ_MAIN:
    {
      u32 num = p_wk->num;

      //動作
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT
          || GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
      {
        p_wk->num--;
      }
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT
          || GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
      {
        p_wk->num++;
      }

      //最大最小値補正
      if( p_wk->num < min )
      {
        p_wk->num = max;
      }
      else if( p_wk->num > max )
      {
        p_wk->num = min;
      }

      //値が変更したときだけ描画
      if( num != p_wk->num )
      {
        NumInput_Draw( p_wk );
      }
      
      // 終了判定
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        FLDSYSWIN_ClearWindow( p_wk->win );
        FLDSYSWIN_Delete( p_wk->win );
        GFL_MSG_Delete( p_wk->msgData );

        p_wk->seq = 0;
        return p_wk->num;
      }
      break;
    }
  }
  return NUMINPUT_NONE;
}
static void NumInput_Draw( NUM_INPUT *p_wk )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;

  str = GFL_MSG_CreateString( p_wk->msgData, DEBUG_STR_INFINITY_NUM );
  wset = WORDSET_Create( p_wk->heapID );
  exp  = GFL_STR_CreateBuffer( 128, p_wk->heapID );
  WORDSET_RegisterNumber( wset, 0, p_wk->num, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wset, exp, str );

  msgBG = FIELDMAP_GetFldMsgBG( p_wk->fieldWork );
  FLDSYSWIN_ClearWindow( p_wk->win );
  FLDSYSWIN_PrintStrBuf( p_wk->win, 0, 0, exp );

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

#endif  //  PM_DEBUG

