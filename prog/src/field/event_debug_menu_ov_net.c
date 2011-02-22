//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_net.c
 *	@brief  デバッグメニュー通信系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"
#include "field/field_comm/mission_types.h"
#include "field/field_comm/intrude_mission.h" //MISSION_LIST_Create

//アプリ
#include "event_geonet.h"
#include "savedata/wifihistory.h"
#include "event_ircbattle.h"
#include "event_wificlub.h"
#include "event_gtsnego.h"
#include "event_debug_menu_wifinote.h"  // for DEBUG_
#include "event_debug_wifimatch.h"

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "event_debug_livecomm.h"
#include "event_debug_linkfest.h"
#include "event_debug_beacon.h"
#include "event_debug_menu_intrude.h"   // for DEBUG_EVENT_FLDMENU_Intrude
#include "event_debug_menu_resort.h"

//外部公開
#include "event_debug_menu_ov_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * WiFiClubメニュー呼びだし
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  GMEVENT *next_event;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  EV_WIFICLUB_PARAM param;

  param.fieldmap = fieldWork;
  param.bFieldEnd = TRUE;


  next_event = GMEVENT_CreateOverlayEventCall( gameSys, 
      FS_OVERLAY_ID(event_wificlub), 
      EVENT_CallWiFiClub, &param );

  GMEVENT_ChangeEvent( event, next_event );

  return( TRUE );
}
//======================================================================
//  デバッグメニュー　Gパワー
//======================================================================
#include "gamesystem/g_power.h"
#include "msg/debug/msg_d_matsu.h"
//--------------------------------------------------------------
/// DEBUG_GPOWER_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;

  POWER_CONV_DATA *powerdata;
}DEBUG_GPOWER_EVENT_WORK;


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuGPowerListEvent(
    GMEVENT *event, int *seq, void *work );

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_GPowerList =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  16,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};
static void DEBUG_SetMenuWorkGPower(GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list,
    HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static u16 DEBUG_GetGPowerMax( GAMESYS_WORK* gsys, void* cb_work );

static const DEBUG_MENU_INITIALIZER DebugGPowerMenuListData = {
  NARC_message_power_dat,
  NULL,
  NULL,
  &DATA_DebugMenuList_GPowerList,
  1, 1, 24, 19,
  DEBUG_SetMenuWorkGPower,
  DEBUG_GetGPowerMax,
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_GPowerList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_GPOWER_EVENT_WORK *work;

  GMEVENT_Change( event,
      debugMenuGPowerListEvent, sizeof(DEBUG_GPOWER_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_GPOWER_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：Gパワーリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuGPowerListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_GPOWER_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_InitArcID(
        work->fieldWork, work->heapID,  &DebugGPowerMenuListData, ARCID_MESSAGE );
    work->powerdata = GPOWER_PowerData_LoadAlloc(work->heapID);
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      if(ret == FLDMENUFUNC_CANCEL){
        (*seq)++;
        break;
      }

      if(ret < GPOWER_ID_MAX){
        GPOWER_Set_OccurID(ret, work->powerdata, FALSE);
      }
      else{
        GPOWER_Clear_AllPower();
      }
    }
    break;
  case 2:
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    GPOWER_PowerData_Unload(work->powerdata);
    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * Gパワー用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkGPower(GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list,
    HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int id;
  STRBUF *strBuf = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgDataDMatsu = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_d_matsu_dat );

  for( id = 0; id < GPOWER_ID_MAX; id++ ){
    GFL_MSG_GetString( msgData,  id, strBuf );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }
  GFL_MSG_GetString(pMsgDataDMatsu, DM_MSG_POWER_RESET, strBuf);
  FLDMENUFUNC_AddStringListData( list, strBuf, GPOWER_ID_MAX, heapID );

  GFL_MSG_Delete( pMsgDataDMatsu );
  GFL_HEAP_FreeMemory( strBuf );
}

//--------------------------------------------------------------
/**
 * @brief Gパワー用リスト最大値取得関数
 * @param fieldmap
 * @return  マップ最大数
 */
//--------------------------------------------------------------
static u16 DEBUG_GetGPowerMax( GAMESYS_WORK* gsys, void* cb_work )
{
  return GPOWER_ID_MAX + 1; // +1 = 「全てリセット」
}

//----------------------------------------------------------------------------
/**
 *  @brief  ビーコンでの友達コード配信
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_BeaconFriendCode( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * new_event;
  new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_beacon ), EVENT_DebugBeacon, wk->fieldWork );
  //new_event = EVENT_DebugBeacon( wk->gmSys, wk->fieldWork );
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  サケサーバーデバッグモードへ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return TRUEでイベンﾄ継続
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugSake( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* set_event;
  EV_DEBUG_WIFIMATCH param;

  param.fieldmap = p_wk->fieldWork;

  set_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
      FS_OVERLAY_ID(event_debug_wifimatch), EVENT_DEBUG_CallWifiMatch, &param );

  GMEVENT_ChangeEvent( p_wk->gmEvent, set_event );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief   デバッグメニュー：ジオネット呼び出し
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_Geonet( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_Geonet( p_wk->gmSys ) );
  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー ジオネット情報削除（データズレ対策）
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_ClearWifiHistory( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys      = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  WIFI_HISTORY *wifiHistory = SaveData_GetWifiHistory(p_sv);
  MYSTATUS *mystatus        = GAMEDATA_GetMyStatus( wk->gdata );

  WIFIHISTORY_Init( wifiHistory );
  MyStatus_SetMyNationArea( mystatus, 0, 0 );
  OS_Printf("mystatus adr=%08x\n", (u32)mystatus);

  return FALSE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　ライブ通信デバッグ 
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_LiveComm( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( d_livecomm ), DEBUG_EVENT_LiveCommFromField, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　リンクフェスティバルデバッグ 
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_LinkFestival( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( d_linkfest ), DEBUG_EVENT_LinkFestFromField, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　ミッションリスト再作成
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_MissionReset( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMEDATA *gmData = GAMESYSTEM_GetGameData(wk->gmSys);
  OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gmData);
  MISSION_LIST_Create( occupy );
  return FALSE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　ハイリンク
 */
//--------------------------------------------------------------
BOOL debugMenu_Intrude( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT* event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_menu_intrude ), DEBUG_EVENT_FLDMENU_Intrude, wk->fieldWork );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　友達手帳書き込み
 */
//--------------------------------------------------------------
BOOL debugMenu_WriteWifiNote( DEBUG_MENU_EVENT_WORK *wk )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_menu_wifinote) );
  DEBUG_EntryWifiNote( wk->gmSys );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_menu_wifinote) );
  return FALSE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー　レゾナンスリゾート
 */
//--------------------------------------------------------------
BOOL debugMenu_Resort( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT *event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_resort ), DEBUG_EVENT_DebugMenu_Resort, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}
