//======================================================================
/**
 *
 * @file  event_debug_menu_jump.c
 * @brief フィールドデバッグメニュー：マップ遷移関連
 * @author  GAMEFREAK inc.
 * @date  2009.11.29
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "arc_def.h"


#include "message.naix"
#include "msg/msg_d_field.h"

#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"


#include "app/townmap.h"




#include "field/field_comm/intrude_field.h" //PALACE_MAP_LEN

#include "event_debug_local.h"

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode

static void DEBUG_SetMenuWorkZoneIDNameAll(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static u16 DEBUG_GetZoneIDNameMax( GAMESYS_WORK* gsys, void* cb_work );

//======================================================================
//  デバッグメニュー どこでもジャンプ
//======================================================================
//--------------------------------------------------------------
/// DEBUG_ZONESEL_EVENT_WORK どこでもジャンプ処理用ワーク
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GAMEDATA * gdata;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
}DEBUG_ZONESEL_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static void initDZEW(DEBUG_ZONESEL_EVENT_WORK * dzew,
    GAMESYS_WORK * gsys, HEAPID heapID)
{
	GFL_STD_MemClear( dzew, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
	
	dzew->gmSys = gsys;
	dzew->heapID = heapID;
	dzew->fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
}
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneSelectEvent(
    GMEVENT *event, int *seq, void *work );

/// どこでもジャンプ　メニューヘッダー
const FLDMENUFUNC_HEADER DATA_DebugMenuList_ZoneSel =
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
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugZoneSelectData = {
  NARC_message_d_field_dat,
  0,
  NULL,
  &DATA_DebugMenuList_ZoneSel,
  1, 1, 20, 16,
  DEBUG_SetMenuWorkZoneIDNameAll,
  DEBUG_GetZoneIDNameMax,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FLDMENU_JumpAllZone( GAMESYS_WORK * gsys, HEAPID heapID )
{
  GMEVENT * new_event = GMEVENT_Create(
      gsys, NULL, debugMenuZoneSelectEvent, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
  DEBUG_ZONESEL_EVENT_WORK * work = GMEVENT_GetEventWork( new_event );
  initDZEW( work, gsys, heapID );
  return new_event;
}

//--------------------------------------------------------------
/**
 * イベント：どこでもジャンプ
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneSelectEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZONESEL_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugZoneSelectData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      
      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }
      
      {
        GMEVENT * mapchange_event;
        mapchange_event = DEBUG_EVENT_QuickChangeMapDefaultPos( work->gmSys, work->fieldWork, ret );
        GMEVENT_ChangeEvent( event, mapchange_event );
      }
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー ジャンプ
//======================================================================
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneJump(GMEVENT *event, int *seq, void *wk );
static void DEBUG_SetSTRBUF_ZoneIDName(u32 heapID, u32 zoneID, STRBUF *strBuf );

//--------------------------------------------------------------
//  data
//--------------------------------------------------------------
///「ジャンプ」コマンドで選択可能なゾーンID
static const u16 JumpZoneID_ListTbl[] = {
  ZONE_ID_UNION,
  ZONE_ID_PALACE01,
  ZONE_ID_PALACE02,
  ZONE_ID_GT,
  ZONE_ID_D09,
  
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko) | defined(DEBUG_ONLY_FOR_iwao_kazumasa)
  ZONE_ID_C04R0201,
  ZONE_ID_C04R0202,
#endif
};

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FLDMENU_JumpEasy( GAMESYS_WORK * gsys, HEAPID heapID )
{
  GMEVENT * new_event;
  DEBUG_ZONESEL_EVENT_WORK *work;
  
  new_event = GMEVENT_Create( gsys, NULL, debugMenuZoneJump, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
  work = GMEVENT_GetEventWork( new_event );
  initDZEW( work, gsys, heapID );

  return new_event;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkZoneID_SelectZone(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int i;
  STRBUF *strBuf = GFL_STR_CreateBuffer( 64, heapID );
  
  for(i = 0; i < NELEMS(JumpZoneID_ListTbl); i++){
    GFL_STR_ClearBuffer( strBuf );
    DEBUG_SetSTRBUF_ZoneIDName( heapID, JumpZoneID_ListTbl[i], strBuf );
    FLDMENUFUNC_AddStringListData( list, strBuf, JumpZoneID_ListTbl[i], heapID );
  }
  
  GFL_HEAP_FreeMemory( strBuf );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuZoneJumpData = {
  NARC_message_d_field_dat,
  NELEMS(JumpZoneID_ListTbl),
  NULL,
  &DATA_DebugMenuList_ZoneSel, //流用
  1, 1, 11, 16,
  DEBUG_SetMenuWorkZoneID_SelectZone,
  NULL,
};

//--------------------------------------------------------------
/**
 * イベント：ジャンプ
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneJump(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZONESEL_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugMenuZoneJumpData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      
      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }
      
      {
        GMEVENT * mapchange_event;
        if(ret == ZONE_ID_UNION){
          mapchange_event = EVENT_ChangeMapToUnion(work->gmSys, work->fieldWork);
        }
        else if(ret == ZONE_ID_PALACE01){
          VecFx32 pos = {PALACE_MAP_LEN/2, 32*FX32_ONE, 488*FX32_ONE};
          mapchange_event = EVENT_ChangeMapPos(work->gmSys, work->fieldWork, ret, &pos, 0);
        }
        else{
          mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( work->gmSys, work->fieldWork, ret );
        }
        GMEVENT_ChangeEvent( event, mapchange_event );
      }
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　四季ジャンプ
//======================================================================
//--------------------------------------------------------------
//  proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSeasonSelectEvent(
    GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FLDMENU_ChangeSeason( GAMESYS_WORK * gsys, HEAPID heapID )
{
  GMEVENT * new_event = GMEVENT_Create(
      gsys, NULL, debugMenuSeasonSelectEvent, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
  DEBUG_ZONESEL_EVENT_WORK * work = GMEVENT_GetEventWork( new_event );
  initDZEW( work, gsys, heapID );
  return new_event;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
#include "gamesystem/pm_season.h"
static const FLDMENUFUNC_LIST DATA_SeasonMenuList[PMSEASON_TOTAL] =
{
  { DEBUG_FIELD_STR_SPRING, (void*)PMSEASON_SPRING },
  { DEBUG_FIELD_STR_SUMMER, (void*)PMSEASON_SUMMER },
  { DEBUG_FIELD_STR_AUTUMN, (void*)PMSEASON_AUTUMN },
  { DEBUG_FIELD_STR_WINTER, (void*)PMSEASON_WINTER },
};

static const DEBUG_MENU_INITIALIZER DebugSeasonJumpMenuData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SeasonMenuList),
  DATA_SeasonMenuList,
  &DATA_DebugMenuList_ZoneSel, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：四季ジャンプ
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSeasonSelectEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZONESEL_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSeasonJumpMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }else{
        GMEVENT *mapchange_event;
        GAMEDATA *gdata = GAMESYSTEM_GetGameData( work->gmSys );
        PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
        const VecFx32 *pos = PLAYERWORK_getPosition( player );
        const RAIL_LOCATION *location = PLAYERWORK_getRailPosition( player );
        u16 dir = PLAYERWORK_getDirection( player );
        ZONEID zone_id = PLAYERWORK_getZoneID(player);
        
        if( (dir>0x2000) && (dir<0x6000) ){
          dir = EXIT_DIR_LEFT;
        }else if( (dir >= 0x6000) && (dir <= 0xa000) ){
          dir = EXIT_DIR_DOWN;
        }else if( (dir > 0xa000) && (dir < 0xe000) ){
          dir = EXIT_DIR_RIGHT;
        }else{
          dir = EXIT_DIR_UP;
        }

        { //DSの保持しているRTCを直接触っている
          u16 now_season = GAMEDATA_GetSeasonID( gdata );
          RTCDate date;
          GFL_RTC_GetDate( &date );
          if ( now_season < ret ) {
            date.month += ret - now_season;
          } else {
            date.month += PMSEASON_TOTAL + ret - now_season;
          }
          if (date.month > 12 ) {
            date.month = 1;
            date.year += 1;
          }
          RTC_SetDate( &date );
        }
        //GAMEDATA_SetSeasonID(gdata, ret);
        if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_GRID )
        {
          mapchange_event = EVENT_ChangeMapPos(
            work->gmSys, work->fieldWork, zone_id, pos, dir );
        }
        else
        {
          mapchange_event = DEBUG_EVENT_ChangeMapRailLocation(
            work->gmSys, work->fieldWork, zone_id, location, dir );
        }
        GMEVENT_ChangeEvent( event, mapchange_event );
        OS_Printf( "x = %xH, z = %xH\n", pos->x, pos->z );
      }
    }
    break;
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * Zone ID Name -> BMP_MENULIST_DATA 
 * @param heapID  テンポラリ文字列確保用heapID
 * @param zoneID  文字列を取得したいzoneID
 * @param strBuf  文字列格納先STRBUF
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetSTRBUF_ZoneIDName(
    u32 heapID, u32 zoneID, STRBUF *strBuf )
{
  u16 *pStrCode;
  char name8[64];
  
  pStrCode = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16)*64 );

  ZONEDATA_DEBUG_GetZoneName( name8, zoneID );
  DEB_STR_CONV_SjisToStrcode( name8, pStrCode, 64 );
  GFL_STR_SetStringCode( strBuf, pStrCode );

  GFL_HEAP_FreeMemory( pStrCode );
}

static void setDebugZoneIDStr( const char * allName, u32 zoneID, STRBUF * strBuf )
{
  u16 pStrCode[64];
  
  //pStrCode = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16)*64 );

  DEB_STR_CONV_SjisToStrcode( &allName[ ZONEDATA_NAME_LENGTH * zoneID ], pStrCode, 64 );
  GFL_STR_SetStringCode( strBuf, pStrCode );

  //GFL_HEAP_FreeMemory( pStrCode );
}
//--------------------------------------------------------------
/**
 * 何処でもジャンプ用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkZoneIDNameAll(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int id,max = ZONEDATA_GetZoneIDMax();
  const char * nameAllBuf = ZONEDATA_GetAllZoneName( heapID );
  STRBUF *strBuf1 = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *strBuf2 = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_place_name_dat );
  
  for( id = 0; id < max; id++ ){
    u16 str_id = ZONEDATA_GetPlaceNameID( id );
    GFL_STR_ClearBuffer( strBuf1 );
    GFL_MSG_GetString( pMsgData,  str_id, strBuf1 );  // 地名文字列を取得
    GFL_STR_ClearBuffer( strBuf2 );
    setDebugZoneIDStr( nameAllBuf, id, strBuf2 );
 //   DEBUG_SetSTRBUF_ZoneIDName( heapID, id, strBuf2 );
    GFL_STR_AddString( strBuf2, strBuf1 );
    FLDMENUFUNC_AddStringListData( list, strBuf2, id, heapID );
  }
  
  GFL_MSG_Delete( pMsgData );
  GFL_HEAP_FreeMemory( strBuf1 );
  GFL_HEAP_FreeMemory( strBuf2 );
  GFL_HEAP_FreeMemory( (void*)nameAllBuf );
}

//--------------------------------------------------------------
/**
 * @brief 何処でもジャンプ用リスト最大値取得関数
 * @param fieldmap
 * @return  マップ最大数
 */
//--------------------------------------------------------------
static u16 DEBUG_GetZoneIDNameMax( GAMESYS_WORK* gsys, void* cb_work )
{
  return ZONEDATA_GetZoneIDMax();
}

