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
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuListEvent(GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuCallProc_SetRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_SetProxy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearRecvBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ClearProxyBuff( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllRecv( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllProxy( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_ChangeMap( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllSecretary( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllResident( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllClear( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_UpdateDay( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallDebugInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallCityNameInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallOwnerNameInput( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_CallOwnerPhraseInput( DEBUG_MENU_EVENT_WORK *p_wk );

static const DEBUG_MENU_LIST DATA_SubMenuList[] =
{
  { DEBUG_FIELD_RESORT_07, debugMenuCallProc_ChangeMap, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_01, debugMenuCallProc_SetRecv, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_02, debugMenuCallProc_SetProxy, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_03, debugMenuCallProc_ClearRecvBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_04, debugMenuCallProc_ClearProxyBuff, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_05, debugMenuCallProc_AllRecv, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_06, debugMenuCallProc_AllProxy, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_08, debugMenuCallProc_AllSecretary, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_09, debugMenuCallProc_AllResident, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_10, debugMenuCallProc_AllClear, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_11, debugMenuCallProc_UpdateDay, GFL_OVERLAY_BLANK_ID},
  { DEBUG_FIELD_RESORT_12, debugMenuCallProc_CallDebugInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_13, debugMenuCallProc_CallCityNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_14, debugMenuCallProc_CallOwnerNameInput, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_RESORT_15, debugMenuCallProc_CallOwnerPhraseInput, GFL_OVERLAY_BLANK_ID },
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
  RESORT_RECV_BUFF *p_recv_tmp_buff = NULL;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  while( TRUE )
  {
    RESORT_RECV_DATA_SetDebugData( p_recv, p_wk->gdata );
    if( !RESORT_RECV_BUFF_AddRecv( p_recv_buff, p_recv ) )
    {
      p_recv_tmp_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();
      if( !p_recv_tmp_buff )
      {
        break;
      }
      else 
      { 
        if( !RESORT_RECV_BUFF_AddRecv( p_recv_tmp_buff, p_recv ) )
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
static BOOL debugMenuCallProc_ChangeMap( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->gmSys,
                                                p_wk->fieldWork, ZONE_ID_C16 );
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
      RESORT_SECRETARY_DATA_SetRecv( p_dst, p_recv, 10+i, 1, i );
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

//-------------------------------------
/// デバッグ文字名変更
//=====================================
typedef enum
{
  CHANGENAME_TYPE_CITY,
  CHANGENAME_TYPE_AUTHER,
  CHANGENAME_TYPE_PHRASE,
}CHANGENAME_TYPE;
typedef struct
{
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  NAMEIN_PARAM   *p_param;
  STRBUF          *p_default_str;
  CHANGENAME_TYPE type;
} DEBUG_CHANGENAME_EVENT_WORK;
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
static BOOL debugMenuCallProc_CallCityNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_CHANGENAME_EVENT_WORK  *p_ev_wk;
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
  RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData(p_resort);
  STRCODE strcode[RESORT_CITY_NAME_LEN+EOM_SIZE];

  //イヴェント
  GMEVENT_Change( p_event, debugEvnetChangeName, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;
  p_ev_wk->type = CHANGENAME_TYPE_CITY;

  //デフォルト名
  p_ev_wk->p_default_str  = GFL_STR_CreateBuffer( RESORT_CITY_NAME_LEN+EOM_SIZE, HEAPID_PROC );
  RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_CITY_NAME, strcode );
  GFL_STR_SetStringCode(p_ev_wk->p_default_str,strcode);

  //名前入力ワーク設定
  p_ev_wk->p_param  = NAMEIN_AllocParam( HEAPID_PROC, NAMEIN_FREE_WORD, 0, 0, RESORT_CITY_NAME_LEN, p_ev_wk->p_default_str, NULL );
  p_ev_wk->p_param->prof_flag  = NAMEIN_PROF_ALL_ENABLE;

  return TRUE;
}
static BOOL debugMenuCallProc_CallOwnerNameInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_CHANGENAME_EVENT_WORK  *p_ev_wk;
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
  RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData(p_resort);
  STRCODE strcode[RESORT_MY_NAME_LEN+EOM_SIZE];

  //イヴェント
  GMEVENT_Change( p_event, debugEvnetChangeName, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;
  p_ev_wk->type = CHANGENAME_TYPE_AUTHER;

  //デフォルト名
  p_ev_wk->p_default_str  = GFL_STR_CreateBuffer( RESORT_MY_NAME_LEN+EOM_SIZE, HEAPID_PROC );
  RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_AUTHER_NAME, strcode );
  GFL_STR_SetStringCode(p_ev_wk->p_default_str,strcode);

  //名前入力ワーク設定
  p_ev_wk->p_param  = NAMEIN_AllocParam( HEAPID_PROC, NAMEIN_FREE_WORD, 0, 0, RESORT_MY_NAME_LEN, p_ev_wk->p_default_str, NULL );
  p_ev_wk->p_param->prof_flag  = NAMEIN_PROF_ALL_ENABLE;

  return TRUE;
}

static BOOL debugMenuCallProc_CallOwnerPhraseInput( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_CHANGENAME_EVENT_WORK  *p_ev_wk;
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort);
  RESORT_RECV_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
  STRCODE strcode[RESORT_MY_NAME_LEN+EOM_SIZE];

  //イヴェント
  GMEVENT_Change( p_event, debugEvnetChangeName, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;
  p_ev_wk->type = CHANGENAME_TYPE_PHRASE;

  //デフォルト名
  p_ev_wk->p_default_str  = GFL_STR_CreateBuffer( RESORT_MY_NAME_LEN+EOM_SIZE, HEAPID_PROC );
  RESORT_RECV_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT9_PHRASE, strcode );

  GFL_STR_SetStringCode(p_ev_wk->p_default_str,strcode);

  //名前入力ワーク設定
  p_ev_wk->p_param  = NAMEIN_AllocParam( HEAPID_PROC, NAMEIN_FREE_WORD, 0, 0, RESORT_MY_NAME_LEN, p_ev_wk->p_default_str, NULL );
  p_ev_wk->p_param->prof_flag  = NAMEIN_PROF_ALL_ENABLE;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  主人公名前再設定イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
    SEQ_EXIT,
  };

  DEBUG_CHANGENAME_EVENT_WORK  *p_wk = p_wk_adrs;
  const STRCODE *cp_strcode = NULL;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
          FS_OVERLAY_ID(namein), &NameInputProcData, p_wk->p_param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( !NAMEIN_IsCancel( p_wk->p_param ) )
    {
      GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
      RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
      RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData(p_resort);
      RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort);
      NAMEIN_CopyStr( p_wk->p_param, p_wk->p_default_str );
      cp_strcode = GFL_STR_GetStringCodePointer( p_wk->p_default_str );
      switch(p_wk->type)
      {
      case CHANGENAME_TYPE_CITY:
        RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_CITY_NAME,(u32)cp_strcode );
        break;
      case CHANGENAME_TYPE_AUTHER:
        RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_AUTHER_NAME,(u32)cp_strcode );
        break;
      case CHANGENAME_TYPE_PHRASE:
        {
          RESORT_RECV_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
          DEBUG_RESORT_DATA_SetPermission( TRUE );
          RESORT_RECV_DATA_SetData( p_data, RESORT_DATA_PARAM_EXT9_PHRASE, (u32)cp_strcode );
          DEBUG_RESORT_DATA_SetPermission( FALSE );
        }
        break;
      }
    }

    *p_seq  = SEQ_EXIT;
    break;


  case SEQ_EXIT:
    NAMEIN_FreeParam( p_wk->p_param );
    GFL_STR_DeleteBuffer( p_wk->p_default_str );

    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}
