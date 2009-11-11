//======================================================================
/**
 *
 * @file	event_debug_menu.c
 * @brief	フィールドデバッグメニューの制御イベント
 * @author	kagaya
 * @date	2008.11.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"

#include "field/field_msgbg.h"

#include "message.naix"
#include "msg/msg_d_field.h"

#include "fieldmap.h"
#include "field/zonedata.h"
#include "field_comm/field_comm_main.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"

#include "field_comm/field_comm_main.h"
#include "field_comm/field_comm_debug.h"
#include "net_app/irc_match/ircbattlemenu.h"
#include "event_ircbattle.h"
#include "event_wificlub.h"
#include "field_subscreen.h"
#include "sound/pm_sndsys.h"

#include "font/font.naix"

#include  "field/weather_no.h"
#include  "weather.h"
#include  "msg/msg_d_tomoya.h"

#include "field_debug.h"

#include "field_event_check.h"
#include "event_debug_item.h" //EVENT_DebugItemMake
#include "savedata/box_savedata.h"  //デバッグアイテム生成用
#include  "item/itemsym.h"  //ITEM_DATA_MAX
#include  "item/item.h"  //ITEM_CheckEnable
#include "app/townmap.h"
#include "net_app/worldtrade.h"
#include "../ui/debug/ui_template.h"
#include "savedata/shortcut.h"

#include "field_sound.h"

#include "eventwork.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "field/field_comm/intrude_field.h" //PALACE_MAP_LEN

//======================================================================
//	define
//======================================================================
#define DEBUG_BGFRAME_MENU (GFL_BG_FRAME1_M)	//使用フレーム
#define DEBUG_BGFRAME_MSG  (GFL_BG_FRAME2_M)	//使用フレーム
#define DEBUG_MENU_PANO	(14)
#define DEBUG_FONT_PANO	(15)

#define D_MENU_CHARSIZE_X (12)		//メニュー横幅
#define D_MENU_CHARSIZE_Y (16)		//メニュー縦幅

#define LINER_CAM_KEY_WAIT    (30)
#define LINER_CAM_KEY_COUNT    (15)

//--------------------------------------------------------------
///	DMENURET
//--------------------------------------------------------------
typedef enum
{
	DMENURET_CONTINUE,
	DMENURET_FINISH,
	DMENURET_NEXTEVENT,
}DMENURET;

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	DEBUG_MENU_EVENT_WORK
//--------------------------------------------------------------
typedef struct _TAG_DEBUG_MENU_EVENT_WORK DEBUG_MENU_EVENT_WORK;

//--------------------------------------------------------------
///	メニュー呼び出し関数
///	BOOL TRUE=イベント継続 FALSE==デバッグメニューイベント終了
//--------------------------------------------------------------
typedef BOOL (* DEBUG_MENU_CALLPROC)(DEBUG_MENU_EVENT_WORK*);

//--------------------------------------------------------------
///	DEBUG_MENU_LISTDATA
//--------------------------------------------------------------
typedef struct
{
	u16 charsize_x;
	u16 charsize_y;
	u32 max;
	const FLDMENUFUNC_LIST *list;
}DEBUG_MENU_LISTDATA;

//--------------------------------------------------------------
///	DEBUG_MENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_DEBUG_MENU_EVENT_WORK
{
	HEAPID heapID;
	GMEVENT *gmEvent;
	GAMESYS_WORK *gmSys;
  GAMEDATA * gdata;
	FIELDMAP_WORK * fieldWork;
	
	DEBUG_MENU_CALLPROC call_proc;
	GFL_MSGDATA *msgData;
	FLDMENUFUNC *menuFunc;
};

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef void MAKE_LIST_FUNC(GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID );

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef u16 GET_MAX_FUNC(FIELDMAP_WORK * fieldmap );

//--------------------------------------------------------------
/**
 * @brief   メニュー初期化構造体
 *
 * @note
 * 可変メニューの場合、makeListFunc/getMaxFuncも定義する必要がある。
 * 使用方法についてはどこでもジャンプなどを参照のこと。
 */
//--------------------------------------------------------------
typedef struct {
  u16 msg_arc_id;     ///<引用するメッセージアーカイブの指定
  u16 max;            ///<項目最大数
  const FLDMENUFUNC_LIST * menulist;  ///<参照するメニューデータ（生成する場合はNULL)
  const FLDMENUFUNC_HEADER * menuH;   ///<メニュー表示指定データ
  u8 px, py, sx, sy;
  MAKE_LIST_FUNC * makeListFunc;      ///<メニュー生成関数へのポインタ（固定メニューの場合はNULL)
  GET_MAX_FUNC * getMaxFunc;          ///<項目最大数取得関数へのポインタ（固定メニューの場合はNULL)
}DEBUG_MENU_INITIALIZER;

//======================================================================
//	proto
//======================================================================
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_GridCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_GridScaleSwitch( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_GridScaleControl( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_OpenStartComm( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenStartInvasion( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk );

static debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk );
static debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk );
static debugMenuCallProc_MusicalSelect( DEBUG_MENU_EVENT_WORK *wk );

static void DEBUG_SetMenuWorkZoneIDNameAll(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID );
static u16 DEBUG_GetZoneIDNameMax( FIELDMAP_WORK * fieldmap );


static BOOL debugMenuCallProc_ControlCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlTarget( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_CameraList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_ChangePlayerSex( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_WifiGts( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_GDS( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk );

//======================================================================
//  デバッグメニューリスト
//======================================================================
//--------------------------------------------------------------
///	デバッグメニューリスト
///	データを追加する事でメニューの項目も増えます。
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_DebugMenuList[] =
{
	{	DEBUG_FIELD_STR38, debugMenuCallProc_DebugSkyJump },
	{ DEBUG_FIELD_STR17, debugMenuCallProc_FieldPosData },
	{ DEBUG_FIELD_STR43, debugMenuCallProc_Jump },
	{ DEBUG_FIELD_STR02, debugMenuCallProc_ControlCamera },
	{ DEBUG_FIELD_STR20, debugMenuCallProc_ControlTarget },
	{ DEBUG_FIELD_STR03, debugMenuCallProc_GridScaleSwitch },
	{ DEBUG_FIELD_STR04, debugMenuCallProc_GridScaleControl },
	{ DEBUG_FIELD_STR05, debugMenuCallProc_MapZoneSelect },
	{ DEBUG_FIELD_STR06, debugMenuCallProc_MapSeasonSelect},
	{ DEBUG_FIELD_STR07, debugMenuCallProc_CameraList },
	{ DEBUG_FIELD_STR13, debugMenuCallProc_MMdlList },
	{ DEBUG_FIELD_C_CHOICE00, debugMenuCallProc_OpenCommDebugMenu },
	{ DEBUG_FIELD_STR19, debugMenuCallProc_OpenClubMenu },
	{ DEBUG_FIELD_STR22, debugMenuCallProc_ControlRtcList },
	{ DEBUG_FIELD_STR15, debugMenuCallProc_ControlLight },
	{ DEBUG_FIELD_STR16, debugMenuCallProc_WeatherList },
  { DEBUG_FIELD_STR_SUBSCRN, debugMenuCallProc_SubscreenSelect },
  { DEBUG_FIELD_STR21 , debugMenuCallProc_MusicalSelect },
  { DEBUG_FIELD_STR31, debugMenuCallProc_Naminori },
  { DEBUG_FIELD_STR41, debugMenuCallProc_DebugMakePoke },
  { DEBUG_FIELD_STR32, debugMenuCallProc_DebugItem },
  { DEBUG_FIELD_STR37, debugMenuCallProc_BoxMax },
  { DEBUG_FIELD_STR39, debugMenuCallProc_MyItemMax },
  { DEBUG_FIELD_STR36, debugMenuCallProc_ControlFog },
	{ DEBUG_FIELD_STR40, debugMenuCallProc_ChangePlayerSex },
	{	DEBUG_FIELD_STR42, debugMenuCallProc_WifiGts },
	{	DEBUG_FIELD_STR48, debugMenuCallProc_GDS },
	{	DEBUG_FIELD_STR44, debugMenuCallProc_UITemplate },
  { DEBUG_FIELD_STR45, debugMenuCallProc_Kairiki },
  { DEBUG_FIELD_STR46, debugMenuCallProc_ControlLinerCamera },
	{	DEBUG_FIELD_STR47, debugMenu_ControlShortCut },
};


//--------------------------------------------------------------
///	メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_DebugMenuListHeader =
{
	1,		//リスト項目数
	9,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	2,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};	

//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_DebugMenuList),
  DATA_DebugMenuList,
  &DATA_DebugMenuListHeader, 
  1, 1, D_MENU_CHARSIZE_X, D_MENU_CHARSIZE_Y,
  NULL,
  NULL,
};

//======================================================================
//	イベント：フィールドデバッグメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグメニューイベント起動
 * @param	gsys	GAMESYS_WORK
 * @param	fieldWork	FIELDMAP_WORK
 * @param	heapID	HEAPID
 * @param	page_id	デバッグメニューページ
 * @retval	GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu(
	GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
	HEAPID heapID, u16 page_id )
{
	DEBUG_MENU_EVENT_WORK * dmew;
	GMEVENT * event;
	
	event = GMEVENT_Create(
		gsys, NULL, DebugMenuEvent, sizeof(DEBUG_MENU_EVENT_WORK));
	
	dmew = GMEVENT_GetEventWork(event);
	MI_CpuClear8( dmew, sizeof(DEBUG_MENU_EVENT_WORK) );
	
	dmew->gmSys = gsys;
	dmew->gmEvent = event;
  dmew->gdata = GAMESYSTEM_GetGameData( gsys );
	dmew->fieldWork = fieldWork;
	dmew->heapID = heapID;
  
	return event;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static FLDMENUFUNC * DebugMenuInit(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init )
{
  GAMESYS_WORK * gsys;
  FLDMENUFUNC * ret;
	GFL_MSGDATA *msgData;
  FLDMSGBG *msgBG;
  FLDMENUFUNC_LISTDATA *listdata;
  FLDMENUFUNC_HEADER menuH;
  u16 max;
  
  gsys = FIELDMAP_GetGameSysWork( fieldmap );
  msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
  msgData = FLDMSGBG_CreateMSGDATA( msgBG, init->msg_arc_id );

  if (init->getMaxFunc) {
    max = init->getMaxFunc( fieldmap );
  } else {
    max = init->max;
  }
  if (init->makeListFunc) {
    listdata = FLDMENUFUNC_CreateListData( max, heapID );
    init->makeListFunc( gsys, listdata, heapID );
  } else {
    listdata = FLDMENUFUNC_CreateMakeListData( init->menulist, max, msgData, heapID );
  }

  menuH = *(init->menuH);
  {
    u8 sy = (max * 2 < init->sy) ? max * 2 : init->sy;
    FLDMENUFUNC_InputHeaderListSize( &menuH, max, init->px, init->py, init->sx, sy );
  }
  ret = FLDMENUFUNC_AddMenu( msgBG, &menuH, listdata );
  GFL_MSG_Delete( msgData );
  return ret;
}

//--------------------------------------------------------------
/**
 * イベント：フィールドデバッグメニュー
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk )
{
	DEBUG_MENU_EVENT_WORK *work = wk;
	
	switch (*seq) {
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugMenuData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}else if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				(*seq)++;
			}else{							//決定
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

//======================================================================
//	デバッグメニュー呼び出し
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用カメラ
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_GridCamera( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_Camera( fieldWork );
	return( FALSE );
}
#endif

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール切り替え
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_GridScaleSwitch( DEBUG_MENU_EVENT_WORK *wk )
{
#if 0
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleChange( fieldWork );
#endif
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　グリッド用スケール調節
 * @param	wk	DEBUG_MENU_EVENT_WORK *
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_GridScaleControl( DEBUG_MENU_EVENT_WORK *wk )
{
#if 0
	DEBUG_MENU_EVENT_WORK *d_menu = wk;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	HEAPID DebugHeapID = d_menu->heapID;
	
	DEBUG_FldGridProc_ScaleControl( fieldWork );
#endif
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　通信デバッグ子メニュー
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk )
{
	GMEVENT *event = wk->gmEvent;
	const HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	GAMESYS_WORK	*gameSys	= wk->gmSys;
	FIELD_COMM_DEBUG_WORK *work;
	
#if 0 //通信システム変更の為、Fix 2009.09.03(木)
	GMEVENT_Change( event,
		FIELD_COMM_DEBUG_CommDebugMenu, FIELD_COMM_DEBUG_GetWorkSize() );
	work = GMEVENT_GetEventWork( event );
	FIELD_COMM_DEBUG_InitWork( heapID , gameSys , fieldWork , event , work );
#else
  GF_ASSERT(0);
#endif

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * WiFiClubメニュー呼びだし
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk )
{
	GMEVENT *event = wk->gmEvent;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	GAMESYS_WORK	*gameSys	= wk->gmSys;

  EVENT_WiFiClubChange(gameSys, fieldWork, event);
	return( TRUE );
}

//======================================================================
//	デバッグメニュー どこでもジャンプ
//======================================================================
//--------------------------------------------------------------
///	DEBUG_ZONESEL_EVENT_WORK どこでもジャンプ処理用ワーク
//--------------------------------------------------------------
typedef struct
{
	int seq_no;
	HEAPID heapID;
	GAMESYS_WORK *gmSys;
	GMEVENT *gmEvent;
  GAMEDATA * gdata;
	FIELDMAP_WORK *fieldWork;
	GFL_MSGDATA *msgData;
	FLDMENUFUNC *menuFunc;
}DEBUG_ZONESEL_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static void initDZEW(DEBUG_ZONESEL_EVENT_WORK * dzew,
    GAMESYS_WORK * gsys, GMEVENT * event, HEAPID heapID)
{
	MI_CpuClear8( dzew, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
	
	dzew->gmSys = gsys;
	dzew->gmEvent = event;
	dzew->heapID = heapID;
	dzew->fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
}
//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneSelectEvent(
		GMEVENT *event, int *seq, void *work );

///	どこでもジャンプ　メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_ZoneSel =
{
	1,		//リスト項目数
	9,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
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
/**
 * デバッグメニュー呼び出し　どこでもジャンプ
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_ZONESEL_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuZoneSelectEvent, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：どこでもジャンプ
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneSelectEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_ZONESEL_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugZoneSelectData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}
			
      {
        GMEVENT * mapchange_event;
        mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( work->gmSys, work->fieldWork, ret );
        GMEVENT_ChangeEvent( work->gmEvent, mapchange_event );
      }
		}
		break;
	}

	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	デバッグメニュー ジャンプ
//======================================================================
//--------------------------------------------------------------
///	proto
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
  
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko) | defined(DEBUG_ONLY_FOR_iwao_kazumasa)
  ZONE_ID_C04R0201,
  ZONE_ID_C04R0202,
#endif
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　ジャンプ
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_ZONESEL_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuZoneJump, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
  initDZEW( work, gsys, event, heapID );

	return( TRUE );
}

static void DEBUG_SetMenuWorkZoneID_SelectZone(
		GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID )
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
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneJump(GMEVENT *event, int *seq, void *wk )
{
	DEBUG_ZONESEL_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugMenuZoneJumpData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}
			
      {
        GMEVENT * mapchange_event;
        if(ret == ZONE_ID_UNION){
          mapchange_event = EVENT_ChangeMapToUnion(work->gmSys, work->fieldWork);
        }
        else if(ret == ZONE_ID_PALACE01){
          VecFx32 pos = {PALACE_MAP_LEN/2, 0, 408*FX32_ONE};
          mapchange_event = DEBUG_EVENT_ChangeMapPos(work->gmSys, work->fieldWork, ret, &pos, 0);
        }
        else{
          mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( work->gmSys, work->fieldWork, ret );
        }
        GMEVENT_ChangeEvent( work->gmEvent, mapchange_event );
      }
		}
		break;
	}

	return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//	デバッグメニュー　四季ジャンプ
//======================================================================
//--------------------------------------------------------------
//	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSeasonSelectEvent(
		GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk )
{
	HEAPID heapID = wk->heapID;
	GMEVENT *event = wk->gmEvent;
	GAMESYS_WORK *gsys = wk->gmSys;
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( wk->gdata );
	ZONEID zone_id = PLAYERWORK_getZoneID( player );
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_ZONESEL_EVENT_WORK *work;
	
		GMEVENT_Change( event,
			debugMenuSeasonSelectEvent, sizeof(DEBUG_ZONESEL_EVENT_WORK) );
		work = GMEVENT_GetEventWork( event );
    initDZEW( work, gsys, event, heapID );

		return( TRUE );
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

/*
typedef struct {
  u16 msg_arc_id;
  u16 max;
  FLDMENUFUNC_LIST * menulist;
  const FLDMENUFUNC_HEADER * menuH;
  u8 px, py, sx, sy;
}DEBUG_MENU_INITIALIZER;
*/
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
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSeasonSelectEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_ZONESEL_EVENT_WORK *work = wk;

	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugSeasonJumpMenuData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}else{
				GMEVENT *event;
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

				GAMEDATA_SetSeasonID(gdata, ret);
        if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_GRID )
        {
          event = DEBUG_EVENT_ChangeMapPos(
            work->gmSys, work->fieldWork, zone_id, pos, dir );
        }
        else
        {
          event = DEBUG_EVENT_ChangeMapRailLocation(
            work->gmSys, work->fieldWork, zone_id, location, dir );
        }
				GMEVENT_ChangeEvent( work->gmEvent, event );
				OS_Printf( "x = %xH, z = %xH\n", pos->x, pos->z );
			}
		}
		break;
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
typedef struct {  
	HEAPID heapID;
	GAMESYS_WORK *gmSys;
	GMEVENT *gmEvent;
	FIELDMAP_WORK *fieldWork;
	FLDMENUFUNC *menuFunc;
  FIELD_SUBSCREEN_WORK * subscreen;
}DEBUG_MENU_EVENT_SUBSCRN_SELECT_WORK, DMESSWORK;

//--------------------------------------------------------------
//	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
		GMEVENT *event, int *seq, void *wk );
static void setupTouchCameraSubscreen(DMESSWORK * dmess);
static void setupSoundViewerSubscreen(DMESSWORK * dmess);
static void setupNormalSubscreen(DMESSWORK * dmess);
static void setupTopMenuSubscreen(DMESSWORK * dmess);
static void setupUnionSubscreen(DMESSWORK * dmess);
static void setupDebugLightSubscreen(DMESSWORK * dmess);


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk )
{
	HEAPID heapID = wk->heapID;
	GMEVENT *event = wk->gmEvent;
	GAMESYS_WORK *gsys = wk->gmSys;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DMESSWORK *work;
	
		GMEVENT_Change( event,
			debugMenuSubscreenSelectEvent, sizeof(DMESSWORK) );
		work = GMEVENT_GetEventWork( event );
		MI_CpuClear8( work, sizeof(DMESSWORK) );
	
		work->gmSys = gsys;
		work->gmEvent = event;
		work->heapID = heapID;
		work->fieldWork = fieldWork;
    work->subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
		return( TRUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_SubcreenMenuList[FIELD_SUBSCREEN_MODE_MAX] =
{
	{ DEBUG_FIELD_STR_SUBSCRN01, (void*)setupTouchCameraSubscreen },
	{ DEBUG_FIELD_STR_SUBSCRN02, (void*)setupSoundViewerSubscreen },
	{ DEBUG_FIELD_STR_SUBSCRN03, (void*)setupNormalSubscreen },
	{ DEBUG_FIELD_STR_SUBSCRN04, (void*)setupTopMenuSubscreen },
	{ DEBUG_FIELD_STR_SUBSCRN01, (void*)setupDebugLightSubscreen },
	{ DEBUG_FIELD_STR_SUBSCRN05, (void*)setupUnionSubscreen },
};

static const DEBUG_MENU_INITIALIZER DebugSubscreenSelectData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SubcreenMenuList),
  DATA_SubcreenMenuList,
  &DATA_DebugMenuList_ZoneSel, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：した画面ジャンプ
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DMESSWORK *work = wk;

	switch( *seq )
  {
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugSubscreenSelectData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}else{
        typedef void (* CHANGE_FUNC)(DMESSWORK*);
        CHANGE_FUNC func = (CHANGE_FUNC)ret;
        func(work);
        //FIELD_SUBSCREEN_ChangeForce(FIELDMAP_GetFieldSubscreenWork(work->fieldWork), ret);
				return( GMEVENT_RES_FINISH );
			}
		}
		break;
	}
	
	return GMEVENT_RES_CONTINUE ;
}

//--------------------------------------------------------------
/**
 * @brief サブスクリーンのカメラ操作に同期させる
 *
 * @todo  カメラをバインドしてから、どのように切り離すか？を確定させる
 */
//--------------------------------------------------------------
static void setupTouchCameraSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
  { 
    void * inner_work;
    FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(dmess->fieldWork);
    inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(dmess->subscreen);
    FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS);
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupSoundViewerSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_SOUNDVIEWER);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupNormalSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_NORMAL);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupTopMenuSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_TOPMENU);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupUnionSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_UNION);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupDebugLightSubscreen(DMESSWORK * dmess)
{ 
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_LIGHT);
}


//--------------------------------------------------------------
//  ミュージカル系
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(musical);
#include "musical/musical_local.h"
#include "musical/musical_define.h"
#include "musical/musical_system.h"
#include "musical/musical_dressup_sys.h"
#include "musical/musical_stage_sys.h"
#include "poke_tool/poke_tool.h"  //ドレスアップ仮データ用
#include "poke_tool/monsno_def.h" //ドレスアップ仮データ用
#include "event_fieldmap_control.h"
typedef struct {  
	HEAPID heapID;
	GAMESYS_WORK *gmSys;
	GMEVENT *gmEvent;
	GMEVENT *newEvent;
	FIELDMAP_WORK *fieldWork;
	FLDMENUFUNC *menuFunc;
	
	POKEMON_PARAM *pokePara;
	MUSICAL_POKE_PARAM *musPoke;
	MUSICAL_INIT_WORK *musInitWork;
	DRESSUP_INIT_WORK *dupInitWork;
	u8  menuRet;
}DEBUG_MENU_EVENT_MUSICAL_SELECT_WORK, DEB_MENU_MUS_WORK;
static GMEVENT_RESULT debugMenuMusicalSelectEvent( GMEVENT *event, int *seq, void *wk );

static void setupMusicalDressup(DEB_MENU_MUS_WORK * work);
static void setupMusicarShowPart(DEB_MENU_MUS_WORK * work);
static void setupMusicarAll(DEB_MENU_MUS_WORK * work);

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　ミュージカル呼び出し
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static debugMenuCallProc_MusicalSelect( DEBUG_MENU_EVENT_WORK *wk )
{
	HEAPID heapID = wk->heapID;
	GMEVENT *event = wk->gmEvent;
	GAMESYS_WORK *gsys = wk->gmSys;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEB_MENU_MUS_WORK *work;
	
		GMEVENT_Change( event,
			debugMenuMusicalSelectEvent, sizeof(DEB_MENU_MUS_WORK) );
		work = GMEVENT_GetEventWork( event );
		MI_CpuClear8( work, sizeof(DEB_MENU_MUS_WORK) );
	
		work->gmSys = gsys;
		work->gmEvent = event;
		work->heapID = heapID;
		work->fieldWork = fieldWork;
		work->musInitWork = NULL;
		work->dupInitWork = NULL;
		work->musPoke = NULL;
		work->pokePara = NULL;
		return( TRUE );
}

//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_MusicalMenuList[3] =
{
	{ DEBUG_FIELD_STR_MUSICAL3, (void*)setupMusicarAll },
	{ DEBUG_FIELD_STR_MUSICAL1, (void*)setupMusicalDressup },
	{ DEBUG_FIELD_STR_MUSICAL2, (void*)setupMusicarShowPart },
};

static const DEBUG_MENU_INITIALIZER DebugMusicalSelectData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_MusicalMenuList),
  DATA_MusicalMenuList,
  &DATA_DebugMenuList_ZoneSel, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：ミュージカルデバッグメニュー
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMusicalSelectEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEB_MENU_MUS_WORK *work = wk;

	switch( *seq )
  {
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugMusicalSelectData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );

			if( ret == FLDMENUFUNC_CANCEL ){	//キャンセル
				return( GMEVENT_RES_FINISH );
			}else{
        typedef void (* CHANGE_FUNC)(DEB_MENU_MUS_WORK*);
        CHANGE_FUNC func = (CHANGE_FUNC)ret;
        func(work);
        {
          GAMEDATA *gdata = GAMESYSTEM_GetGameData( work->gmSys );
          FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
          FIELD_SOUND_PushBGM( fsnd );
        }
        GMEVENT_CallEvent(work->gmEvent, work->newEvent);
    		(*seq)++;
    		return( GMEVENT_RES_CONTINUE );
			}
		}
		break;
  case 2:
    if( work->musInitWork != NULL )
    {
      GFL_HEAP_FreeMemory( work->musInitWork->pokePara );
      GFL_HEAP_FreeMemory( work->musInitWork );
      work->musInitWork = NULL;
    }
    if( work->dupInitWork != NULL )
    {
      MUSICAL_DRESSUP_DeleteInitWork( work->dupInitWork );
      work->dupInitWork = NULL;
    }
    if( work->musPoke != NULL )
    {
      GFL_HEAP_FreeMemory( work->musPoke );
      work->musPoke = NULL;
    }
    if( work->pokePara != NULL )
    {
      GFL_HEAP_FreeMemory( work->pokePara );
      work->pokePara = NULL;
    }
    
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( work->gmSys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PopBGM( fsnd );
    }  
    PMSND_FadeInBGM(60);
		return( GMEVENT_RES_FINISH );
    
    break;
	}
	
	return GMEVENT_RES_CONTINUE ;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupMusicalDressup(DEB_MENU_MUS_WORK * work)
{
  work->pokePara = PP_Create( MONSNO_PIKUSII , 20 , PTL_SETUP_POW_AUTO , GFL_HEAPID_APP|HEAPDIR_MASK );
  work->musPoke = MUSICAL_SYSTEM_InitMusPoke( work->pokePara , GFL_HEAPID_APP|HEAPDIR_MASK );
  work->dupInitWork = MUSICAL_DRESSUP_CreateInitWork( GFL_HEAPID_APP|HEAPDIR_MASK , work->musPoke , SaveControl_GetPointer() );

  work->newEvent = EVENT_FieldSubProc(work->gmSys, work->fieldWork,
        FS_OVERLAY_ID(musical), &DressUp_ProcData, work->dupInitWork );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupMusicarShowPart(DEB_MENU_MUS_WORK * work)
{ 
  work->newEvent = EVENT_FieldSubProc(work->gmSys, work->fieldWork,
        FS_OVERLAY_ID(musical), &MusicalStage_ProcData, NULL );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupMusicarAll(DEB_MENU_MUS_WORK * work)
{
	work->musInitWork = GFL_HEAP_AllocMemory( HEAPID_PROC , sizeof(MUSICAL_INIT_WORK));
	work->musInitWork->saveCtrl = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(work->gmSys) ); 
	work->musInitWork->pokePara = PP_Create( MONSNO_PIKUSII , 20 , PTL_SETUP_POW_AUTO , HEAPID_PROC );
	work->musInitWork->isComm = FALSE;
	work->musInitWork->isDebug = FALSE;
	work->musInitWork->gameComm = GAMESYSTEM_GetGameCommSysPtr(work->gmSys);
  work->newEvent = EVENT_FieldSubProc(work->gmSys, work->fieldWork,
        NO_OVERLAY_ID, &Musical_ProcData, work->musInitWork );
}

//======================================================================
//	デバッグ用ZoneID文字列変換
//======================================================================
//--------------------------------------------------------------
//	文字コード
//--------------------------------------------------------------
#define ASCII_EOM (0x00)
#define ASCII_0 (0x30)
#define ASCII_9 (0x39)
#define ASCII_A (0x41)
#define ASCII_Z (0x5a)
#define ASCII_UL (0x5f)

//半角
#define UTF16H_0 (0x0030)
#define UTF16H_9 (0x0039)
#define UTF16H_A (0x0041)
#define UTF16H_Z (0x005a)
#define UTF16H_UL (0x005f)

//全角
#define UTF16_0 (0xff10)
#define UTF16_9 (0xff19)
#define UTF16_A (0xff21)
#define UTF16_Z (0xff3a)

//--------------------------------------------------------------
/**
 * ASCII->UTF-16
 * @param	str	ASCII 半角英数字
 * @retval	u16 UTF-16 str
 */
//--------------------------------------------------------------
static u16 DEBUG_ASCIICODE_UTF16( u8 code )
{
	if( code == ASCII_EOM ){
		return( GFL_STR_GetEOMCode() );
	}
	
  if( code == ASCII_UL ){
    return( UTF16H_UL );
  }
  
	if( code >= ASCII_0 && code <= ASCII_9 ){
		code -= ASCII_0;
		return( UTF16H_0 + code );
	}
	
	if( code >= ASCII_A && code <= ASCII_Z ){
		code -= ASCII_A;
		return( UTF16H_A + code );
	}
  
  //未対応文字
#if 0	
	GF_ASSERT( 0 );
#else
  OS_Printf( "未対応文字 CODE=%d\n", code );
	return( GFL_STR_GetEOMCode() );
#endif
}

//--------------------------------------------------------------
/**
 * ZONE_ID->STRCODE
 * @param	heapID	文字列バッファ確保用ヒープID
 * @param	zoneID	文字列を取得したいzoneID
 * @retval	u16*	zoneID文字列が格納されたu16*(開放が必要
 */
//--------------------------------------------------------------
static u16 * DEBUG_GetZoneNameUTF16( u32 heapID, u32 zoneID )
{
	int i;
	u16 utf16,utf16_eom;
	u16 *pStrBuf;
	char name8[64];
	
	pStrBuf = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16)*64 );
	ZONEDATA_DEBUG_GetZoneName( name8, zoneID );
	utf16_eom = GFL_STR_GetEOMCode();
//	OS_Printf( "変換 %s\n", name8 );
	
	for( i = 0; i < 64; i++ ){
		utf16 = DEBUG_ASCIICODE_UTF16( name8[i] );
		pStrBuf[i] = utf16;
		
		if( utf16 == utf16_eom ){
			return( pStrBuf );
		}
	}
	
	pStrBuf[i-1] = utf16_eom;	//文字数オーバー
	GF_ASSERT( 0 );
	return( pStrBuf );
}

//--------------------------------------------------------------
/**
 * Zone ID Name -> BMP_MENULIST_DATA 
 * @param	heapID	テンポラリ文字列確保用heapID
 * @param	zoneID	文字列を取得したいzoneID
 * @param	strBuf	文字列格納先STRBUF
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetSTRBUF_ZoneIDName(
		u32 heapID, u32 zoneID, STRBUF *strBuf )
{
	u16 *str = DEBUG_GetZoneNameUTF16( heapID, zoneID );
	GFL_STR_SetStringCode( strBuf, str );
	GFL_HEAP_FreeMemory( str );
}

//--------------------------------------------------------------
/**
 * 何処でもジャンプ用BMP_MENULIST_DATAセット
 * @param	list	セット先BMP_MENULIST_DATA
 * @param	heapID	文字列バッファ確保用HEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkZoneIDNameAll(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID )
{
	int id,max = ZONEDATA_GetZoneIDMax();
	STRBUF *strBuf1 = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *strBuf2 = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
	GFL_MSGDATA * pMsgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_place_name_dat );
	
	for( id = 0; id < max; id++ ){
    u16 str_id = ZONEDATA_GetPlaceNameID( id );
    GFL_STR_ClearBuffer( strBuf1 );
	  GFL_MSG_GetString( pMsgData,	str_id, strBuf1 );	// 地名文字列を取得
		GFL_STR_ClearBuffer( strBuf2 );
		DEBUG_SetSTRBUF_ZoneIDName( heapID, id, strBuf2 );
    //GFL_STR_AddCode( strBuf1, DEBUG_ASCIICODE_UTF16('+') );
    GFL_STR_AddString( strBuf2, strBuf1 );
		FLDMENUFUNC_AddStringListData( list, strBuf2, id, heapID );
	}
	
	GFL_MSG_Delete( pMsgData );
	GFL_HEAP_FreeMemory( strBuf1 );
	GFL_HEAP_FreeMemory( strBuf2 );
}

//--------------------------------------------------------------
/**
 * @brief 何処でもジャンプ用リスト最大値取得関数
 * @param fieldmap
 * @return  マップ最大数
 */
//--------------------------------------------------------------
static u16 DEBUG_GetZoneIDNameMax( FIELDMAP_WORK * fieldmap )
{
  return ZONEDATA_GetZoneIDMax();
}


//======================================================================
//	デバッグメニュー　カメラ操作
//======================================================================
#define CM_RT_SPEED (FX32_ONE/8)
#define CM_HEIGHT_MV (FX32_ONE*2)
#define CM_NEARFAR_MV (FX32_ONE)

//--------------------------------------------------------------
///	DEBUG_CTLCAMERA_WORK カメラ操作ワーク1
//--------------------------------------------------------------
typedef struct
{
	int vanish;
	GAMESYS_WORK *gsys;
	GMEVENT *event;
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
	FLDMSGBG *pMsgBG;
	FLDMSGWIN *pMsgWin;
	STRBUF *pStrBuf;
}DEBUG_CTLCAMERA_WORK;

//--------------------------------------------------------------
///	DEBUG_CTL_LINERCAMERA_WORK 線形カメラ操作ワーク1
//--------------------------------------------------------------
typedef struct
{
	int vanish;
	GAMESYS_WORK *gsys;
	GMEVENT *event;
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
	FLDMSGBG *pMsgBG;
	FLDMSGWIN *pMsgWin;
	STRBUF *pStrBuf;
  u16 Wait;
  u16 Count;
}DEBUG_CTL_LINERCAMERA_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlCamera(
		GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT debugMenuControlLinerCamera(
		GMEVENT *event, int *seq, void *wk );
static void DampCameraInfo(FIELD_CAMERA * cam);
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal);

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　カメラ操作
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlCamera( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_CTLCAMERA_WORK *work;
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	
	GMEVENT_Change( event, debugMenuControlCamera, sizeof(DEBUG_CTLCAMERA_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CTLCAMERA_WORK) );
	
	work->gsys = gsys;
	work->event = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;

  {

    FIELD_SUBSCREEN_WORK * subscreen;
    

    // カメラ操作は下画面で行う
    subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldWork);
    FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
    { 
      void * inner_work;
      FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);
      inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(subscreen);
      FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS);
    }
  }
  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    
    if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_NOGRID )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
  }
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　カメラ操作
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlTarget( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_CTLCAMERA_WORK *work;
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	
	GMEVENT_Change( event, debugMenuControlCamera, sizeof(DEBUG_CTLCAMERA_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CTLCAMERA_WORK) );
	
	work->gsys = gsys;
	work->event = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;

  {
    FIELD_SUBSCREEN_WORK * subscreen;

    // カメラ操作は下画面で行う
    subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldWork);
    FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
    { 
      void * inner_work;
      FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);
      inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(subscreen);
      FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_TARGET_POS);
    }
  }

  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    
    if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_NOGRID )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
  }
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：カメラ操作
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlCamera(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_CTLCAMERA_WORK *work = wk;
  FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);


  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    FIELD_CAMERA_DEBUG_ReleaseSubScreen( cam );

    // レールカメラ反映の再開
    {
      FLDNOGRID_MAPPER* mapper;
      
      if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_NOGRID )
      {
        mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
        FLDNOGRID_MAPPER_SetRailCameraActive( mapper, TRUE );
      }
    }
	  return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	デバッグメニュー　テストカメラリスト
//======================================================================
//--------------------------------------------------------------
///	DEBUG_TESTCAMERALIST_EVENT_WORK
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
}DEBUG_TESTCAMERALIST_EVENT_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuTestCameraListEvent(
		GMEVENT *event, int *seq, void *work );

///カメラリスト最大
#define TESTCAMERALISTMAX (4)

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_TestCameraList =
{
	1,		//リスト項目数
	4,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

///テストカメラメニューリスト
static const FLDMENUFUNC_LIST DATA_TestCameraMenuList[TESTCAMERALISTMAX] =
{
	{ DEBUG_FIELD_STR09, (void*)0 },
	{ DEBUG_FIELD_STR10, (void*)1 },
	{ DEBUG_FIELD_STR11, (void*)2 },
	{ DEBUG_FIELD_STR08, (void*)3 },
};

static const DEBUG_MENU_INITIALIZER DebugCameraMenuListData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_TestCameraMenuList),
  DATA_TestCameraMenuList,
  &DATA_DebugMenuList_TestCameraList, //流用
  1, 1, 8, 7,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　テストカメラリスト
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_CameraList( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_TESTCAMERALIST_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuTestCameraListEvent, sizeof(DEBUG_TESTCAMERALIST_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_TESTCAMERALIST_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：テストカメラリスト
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuTestCameraListEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_TESTCAMERALIST_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugCameraMenuListData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );
			
			if( ret != FLDMENUFUNC_CANCEL ){	//決定
				static const u16 length[TESTCAMERALISTMAX] =
					{ 0x0090, 0x0078, 0x0080, 0x0078 };
				static const fx32 height[TESTCAMERALISTMAX] =
					{ 0xae000, 0xa0000, 0xab000, 0xd8000 };
				FIELD_CAMERA *camera =
					FIELDMAP_GetFieldCamera( work->fieldWork );
				VecFx32 vec0, vec1;
				fx32 cos, len;

				// XZ平面の距離とY方向の高さ情報から
				// angle角度　距離を求める
				//
				// FIELD_CAMERAシステムの処理をangleでの動作に変更したため
				// 修正
				len = FX_Mul( length[ret]<<FX32_SHIFT, length[ret]<<FX32_SHIFT ) + FX_Mul( height[ret], height[ret] );
				len = FX_Sqrt( len );

				VEC_Set( &vec0, 0,0,FX32_ONE );
				VEC_Set( &vec1, 0,height[ret],length[ret]<<FX32_SHIFT );
				VEC_Normalize( &vec0, &vec0 );
				VEC_Normalize( &vec1, &vec1 );
				cos = VEC_DotProduct( &vec0, &vec1 );
				
				FIELD_CAMERA_SetAnglePitch( camera, FX_AcosIdx( cos ) );
				FIELD_CAMERA_SetAngleLen( camera, len );
			}
			
			return( GMEVENT_RES_FINISH );
		}
		break;
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	デバッグメニュー　動作モデル一覧
//======================================================================
//--------------------------------------------------------------
///	DEBUG_MMDL_LIST_EVENT_WORK 動作モデルリスト処理用ワーク
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
	MMDLSYS *fldmmdlsys;

	u16 obj_code;
	u16 res_add;
	MMDL *fmmdl;
}DEBUG_MMDLLIST_EVENT_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
		GMEVENT *event, int *seq, void *wk );
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code );
static void DEBUG_SetMenuWorkMMdlList(
		GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID );

///	動作モデルリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_MMdlList =
{
	1,		//リスト項目数
	10,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

static const DEBUG_MENU_INITIALIZER DebugMMdlListData = {
  NARC_message_d_field_dat,
  OBJCODETOTAL,
  NULL,
  &DATA_DebugMenuList_MMdlList,
  1, 1, 11, 16,
  DEBUG_SetMenuWorkMMdlList,
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　動作モデル一覧
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_MMDLLIST_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuMMdlListEvent, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	
	{
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
		work->fldmmdlsys = GAMEDATA_GetMMdlSys( gdata );
	}

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：動作モデル一覧
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_MMDLLIST_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugMMdlListData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			if( ret == FLDMENUFUNC_CANCEL || ret == NONDRAW ){ //キャンセル
				FLDMENUFUNC_DeleteMenu( work->menuFunc );
				return( GMEVENT_RES_FINISH );
			}
			
			work->obj_code = ret;
#if 0 //要らない
			work->res_add = MMDL_BLACTCONT_AddOBJCodeRes(
					work->fldmmdlsys, work->obj_code );
#endif			
			{
				//VecFx32 pos;
				MMDL *jiki;
				MMDL_HEADER head = {
					0,	///<識別ID
					0,	///<表示するOBJコード
					MV_RND,	///<動作コード
					0,	///<イベントタイプ
					0,	///<イベントフラグ
					0,	///<イベントID
					0,	///<指定方向
					0,	///<指定パラメタ 0
					0,	///<指定パラメタ 1
					0,	///<指定パラメタ 2
					4,	///<X方向移動制限
					4,	///<Z方向移動制限
					0,	///<グリッドX
					0,	///<グリッドZ
					0,	///<Y値 fx32型
				};
        MMDL_HEADER_GRIDPOS *gridpos;

				
				jiki = MMDLSYS_SearchOBJID(
					work->fldmmdlsys, MMDL_ID_PLAYER );
				
				head.id = 250;
        MMDLHEADER_SetGridPos( &head, 
            MMDL_GetGridPosX( jiki ) + 2,
            MMDL_GetGridPosZ( jiki ),
            MMDL_GetVectorPosY( jiki ) );
				head.obj_code = work->obj_code;
				work->fmmdl = MMDLSYS_AddMMdl(
					work->fldmmdlsys, &head, 0 );
			}
			
			(*seq)++;
			break;
		case 2:
			{
				int key_trg = GFL_UI_KEY_GetTrg();
				MMDL_UpdateMoveProc( work->fmmdl );

				if( (key_trg & PAD_BUTTON_B) ){
					MMDL_Delete( work->fmmdl );
					
					if( work->res_add == TRUE ){
						MMDL_BLACTCONT_DeleteOBJCodeRes(
								work->fldmmdlsys, work->obj_code );
					}
					
					(*seq) = 1;
				}
			}
			break;
		}
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * 動作モデルOBJコード->STRCODE
 * @param	heapID	文字列バッファ確保用ヒープID
 * @param	code	文字列を取得したいOBJコード
 * @retval	u16*	文字列が格納されたu16*(開放が必要
 */
//--------------------------------------------------------------
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code )
{
	int i;
	u16 utf16,utf16_eom;
	u16 *pStrBuf;
	u8 *name8;
	
	pStrBuf = GFL_HEAP_AllocClearMemory( heapID,
			sizeof(u16)*DEBUG_OBJCODE_STR_LENGTH );
	name8 = DEBUG_MMDL_GetOBJCodeString( code, heapID );
	utf16_eom = GFL_STR_GetEOMCode();
//	OS_Printf( "変換 %s\n", name8 );
	
	for( i = 0; i < DEBUG_OBJCODE_STR_LENGTH; i++ ){
		utf16 = DEBUG_ASCIICODE_UTF16( name8[i] );
		pStrBuf[i] = utf16;
		if( utf16 == utf16_eom ){
			break;
		}
	}
	
	GFL_HEAP_FreeMemory( name8 );
	
	if( i >= DEBUG_OBJCODE_STR_LENGTH ){ //文字数オーバー
		GF_ASSERT( 0 );
		pStrBuf[DEBUG_OBJCODE_STR_LENGTH-1] = utf16_eom;
	}
	
	return( pStrBuf );
}

//--------------------------------------------------------------
/**
 * 動作モデルリスト用BMP_MENULIST_DATAセット
 * @param	list	セット先BMP_MENULIST_DATA
 * @param	heapID	文字列バッファ確保用HEAPID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkMMdlList(
		GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID )
{
	u16 *str;
	int id,max;
	STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_OBJCODE_STR_LENGTH, heapID );
	
	for( id = OBJCODESTART_BBD; id < OBJCODEEND_BBD; id++ ){
		GFL_STR_ClearBuffer( strBuf );
		str = DEBUG_GetOBJCodeStrBuf( heapID, id );
		GFL_STR_SetStringCode( strBuf, str );
		GFL_HEAP_FreeMemory( str );
		FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
	}
  
  for( id = OBJCODESTART_TPOKE; id < OBJCODEEND_TPOKE; id++ ){
		GFL_STR_ClearBuffer( strBuf );
		str = DEBUG_GetOBJCodeStrBuf( heapID, id );
		GFL_STR_SetStringCode( strBuf, str );
		GFL_HEAP_FreeMemory( str );
		FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
	}
  
	GFL_HEAP_FreeMemory( strBuf );
}

//======================================================================
//	デバッグメニュー　ライト操作
//======================================================================

//--------------------------------------------------------------
///	DEBUG_CTLIGHT_WORK ライト操作ワーク
//--------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK *gsys;
	GMEVENT *event;
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;

	GFL_BMPWIN* p_win;
}DEBUG_CTLLIGHT_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
		GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　カメラ操作
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_CTLLIGHT_WORK *work;
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	
	GMEVENT_Change( event, debugMenuControlLight, sizeof(DEBUG_CTLLIGHT_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CTLLIGHT_WORK) );
	
	work->gsys = gsys;
	work->event = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：カメラ操作
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_CTLLIGHT_WORK *work = wk;
	FIELD_LIGHT* p_light;

	// ライト取得
	p_light = FIELDMAP_GetFieldLight( work->fieldWork );
	
	switch( (*seq) ){
	case 0:
		// ライト管理開始
		FIELD_LIGHT_DEBUG_Init( p_light, work->heapID );

		// インフォーバーの非表示
		FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
		GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

		// ビットマップウィンドウ初期化
		{
			static const GFL_BG_BGCNT_HEADER header_sub3 = {
				0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
				GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
			};

			GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
			GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
			GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

			// パレット情報を転送
			GFL_ARC_UTIL_TransVramPalette(
				ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );
			
			// ビットマップウィンドウを作成
			work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
				0, 0, 32, 24,
				FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_F );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
			GFL_BMPWIN_MakeScreen( work->p_win );
			GFL_BMPWIN_TransVramCharacter( work->p_win );
			GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );
		}

		FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

		(*seq)++;
	case 1:
		// ライト管理メイン
		FIELD_LIGHT_DEBUG_Control( p_light );
		FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			(*seq)++;
		}
		break;
	case 2:
		// ライト管理終了
		FIELD_LIGHT_DEBUG_Exit( p_light );

		// ビットマップウィンドウ破棄
		{
			GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
			GFL_BMPWIN_Delete( work->p_win );
		}

		// インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//	デバッグメニュー　フォグ操作
//======================================================================

//--------------------------------------------------------------
///	DEBUG_CTLFOG_WORK フォグ操作ワーク
//--------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK *gsys;
	GMEVENT *event;
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;

	GFL_BMPWIN* p_win;

}DEBUG_CTLFOG_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
		GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_CTLFOG_WORK *work;
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	
	GMEVENT_Change( event, debugMenuControlFog, sizeof(DEBUG_CTLFOG_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CTLFOG_WORK) );
	
	work->gsys = gsys;
	work->event = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：ふぉぐ操作
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_CTLFOG_WORK *work = wk;
	FIELD_FOG_WORK* p_fog;

	// フォグ取得
	p_fog = FIELDMAP_GetFieldFog( work->fieldWork );
	
	switch( (*seq) ){
	case 0:

		// インフォーバーの非表示
		FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
		GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

		// ビットマップウィンドウ初期化
		{
			static const GFL_BG_BGCNT_HEADER header_sub3 = {
				0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
				GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
			};

			GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
			GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
			GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

			// パレット情報を転送
			GFL_ARC_UTIL_TransVramPalette(
				ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );
			
			// ビットマップウィンドウを作成
			work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
				0, 0, 32, 24,
				FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_F );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
			GFL_BMPWIN_MakeScreen( work->p_win );
			GFL_BMPWIN_TransVramCharacter( work->p_win );
			GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );
		}

		FIELD_FOG_DEBUG_Init( p_fog, work->heapID );


		(*seq)++;
	case 1:
		// フォグ管理メイン
		FIELD_FOG_DEBUG_Control( p_fog );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
		FIELD_FOG_DEBUG_PrintData( p_fog, work->p_win );
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			(*seq)++;
		}
		break;
	case 2:

		// ビットマップウィンドウ破棄
		{
			GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
			GFL_BMPWIN_Delete( work->p_win );
		}

		FIELD_FOG_DEBUG_Exit( p_fog );


		// インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//	デバッグメニュー　てんきリスト
//======================================================================
//--------------------------------------------------------------
///	DEBUG_WEATERLIST_EVENT_WORK
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
}DEBUG_WEATERLIST_EVENT_WORK;


//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
		GMEVENT *event, int *seq, void *work );

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_WeatherList =
{
	1,		//リスト項目数
	6,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

///てんきメニューリスト
static const FLDMENUFUNC_LIST DATA_WeatherMenuList[] =
{
	{ D_TOMOYA_WEATEHR00, (void*)WEATHER_NO_SUNNY },
	{ D_TOMOYA_WEATEHR01, (void*)WEATHER_NO_SNOW },
	{ D_TOMOYA_WEATEHR02, (void*)WEATHER_NO_RAIN },
	{ D_TOMOYA_WEATEHR03, (void*)WEATHER_NO_STORM },
	{ D_TOMOYA_WEATEHR05, (void*)WEATHER_NO_SPARK },
	{ D_TOMOYA_WEATEHR06, (void*)WEATHER_NO_SNOWSTORM },
	{ D_TOMOYA_WEATEHR07, (void*)WEATHER_NO_ARARE },
	{ D_TOMOYA_WEATEHR08, (void*)WEATHER_NO_MIRAGE },
};
#define DEBUG_WEATHERLIST_LIST_MAX	( NELEMS(DATA_WeatherMenuList) )

static const DEBUG_MENU_INITIALIZER DebugWeatherMenuListData = {
  NARC_message_d_tomoya_dat,
  NELEMS(DATA_WeatherMenuList),
  DATA_WeatherMenuList,
  &DATA_DebugMenuList_WeatherList,
  1, 1, 12, 11,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_WEATERLIST_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuWeatherListEvent, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：てんきリスト
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_WEATERLIST_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugWeatherMenuListData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			FLDMENUFUNC_DeleteMenu( work->menuFunc );
			
			if( ret != FLDMENUFUNC_CANCEL ){	//決定
				FIELD_WEATHER_Change( FIELDMAP_GetFieldWeather( work->fieldWork ), ret );
			}
			
			return( GMEVENT_RES_FINISH );
		}
		break;
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//	デバッグメニュー　位置情報
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド位置情報
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk )
{
	FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( wk->fieldWork );
	FIELD_DEBUG_SetPosPrint( debug );
	return( FALSE );
}

//======================================================================
///	デバックメニュー　時間
//======================================================================
//--------------------------------------------------------------
///	DEBUG_CONTROL_TIME_LIST_EVENT_WORK
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
}DEBUG_CONTROL_TIME_LIST_EVENT_WORK;

//--------------------------------------------------------------
///	proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlTimeListEvent(
		GMEVENT *event, int *seq, void *work );


//--------------------------------------------------------------
///	data
//--------------------------------------------------------------
/// 時間管理タイプ数
enum{
  CONT_TIME_TYPE_NORMAL,
  CONT_TIME_TYPE_8HOUR,
  CONT_TIME_TYPE_12HOUR,
  CONT_TIME_TYPE_18HOUR,
  CONT_TIME_TYPE_22HOUR,
  CONT_TIME_TYPE_2HOUR,
  CONT_TIME_TYPE_600RATE,
  CONT_TIME_TYPE_60RATE,

  CONT_TIME_TYPE_NUM,
} ;


///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_ContTimeList =
{
	1,		//リスト項目数
	CONT_TIME_TYPE_NUM,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

///テストカメラメニューリスト
static const FLDMENUFUNC_LIST DATA_ControlTimeMenuList[CONT_TIME_TYPE_NUM] =
{
	{ DEBUG_FIELD_STR30, (void*)CONT_TIME_TYPE_NORMAL,  },
	{ DEBUG_FIELD_STR23, (void*)CONT_TIME_TYPE_8HOUR,   },
	{ DEBUG_FIELD_STR24, (void*)CONT_TIME_TYPE_12HOUR,  },
	{ DEBUG_FIELD_STR25, (void*)CONT_TIME_TYPE_18HOUR,  },
	{ DEBUG_FIELD_STR26, (void*)CONT_TIME_TYPE_22HOUR,  },
	{ DEBUG_FIELD_STR27, (void*)CONT_TIME_TYPE_2HOUR,   },
	{ DEBUG_FIELD_STR28, (void*)CONT_TIME_TYPE_600RATE, },
	{ DEBUG_FIELD_STR29, (void*)CONT_TIME_TYPE_60RATE,  },
};

static const DEBUG_MENU_INITIALIZER DebugControlTimeMenuListData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_ControlTimeMenuList),
  DATA_ControlTimeMenuList,
  &DATA_DebugMenuList_ContTimeList,
  1, 1, 9, 13,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　テストカメラリスト
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	DEBUG_CONTROL_TIME_LIST_EVENT_WORK *work;
	
	GMEVENT_Change( event,
		debugMenuControlTimeListEvent, sizeof(DEBUG_CONTROL_TIME_LIST_EVENT_WORK) );
	
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CONTROL_TIME_LIST_EVENT_WORK) );
	
	work->gmSys = gsys;
	work->gmEvent = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：時間操作リスト
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlTimeListEvent(
		GMEVENT *event, int *seq, void *wk )
{
	DEBUG_CONTROL_TIME_LIST_EVENT_WORK *work = wk;
	
	switch( (*seq) ){
	case 0:
    work->menuFunc = DebugMenuInit( work->fieldWork, work->heapID,  &DebugControlTimeMenuListData );
		(*seq)++;
		break;
	case 1:
		{
			u32 ret;
			ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
			
			if( ret == FLDMENUFUNC_NULL ){	//操作無し
				break;
			}
			
			if( ret != FLDMENUFUNC_CANCEL ){	//決定
        switch( ret ){
        case CONT_TIME_TYPE_NORMAL:
          GFL_RTC_DEBUG_StopFakeTime();
          break;
        case CONT_TIME_TYPE_8HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 8,0 );
        case CONT_TIME_TYPE_12HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 12,0 );
          break;
        case CONT_TIME_TYPE_18HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 18,0 );
          break;
        case CONT_TIME_TYPE_22HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 22,0 );
          break;
        case CONT_TIME_TYPE_2HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 2,0 );
          break;
        case CONT_TIME_TYPE_600RATE:
          GFL_RTC_DEBUG_StartFakeTime( 600 );
          break;
        case CONT_TIME_TYPE_60RATE:
          GFL_RTC_DEBUG_StartFakeTime( 60 );
          break;
        default:
          break;
        }
			}else{

			  FLDMENUFUNC_DeleteMenu( work->menuFunc );
			
			
        // オワリ
  			return( GMEVENT_RES_FINISH );
      }
		}
		break;
	}
	
	return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　波乗り
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りイベント呼び出し
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk )
{
  if( FIELDMAP_GetMapControlType(wk->fieldWork) != FLDMAP_CTRLTYPE_GRID ){
    return( FALSE );
  }
  
  FIELD_EVENT_ChangeNaminoriStart( wk->gmEvent, wk->gmSys, wk->fieldWork );
  return( TRUE );
}

//======================================================================
//  デバッグメニュー ポケモン作成
//======================================================================
#include "debug/debug_makepoke.h"
FS_EXTERN_OVERLAY(debug_makepoke);
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
///	デバッグポケモン作成用ワーク	
//=====================================
typedef struct 
{
  HEAPID heapID;
	GAMESYS_WORK		*p_gamesys;
	GMEVENT					*p_event;
	FIELDMAP_WORK *p_field;
	PROCPARAM_DEBUG_MAKEPOKE p_mp_work;
	POKEMON_PARAM *pp;
} DEBUG_MAKEPOKE_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモン作成
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk		ワーク
 *
 *	@return	TRUEイベント継続	FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk )
{	
	GAMESYS_WORK	*p_gamesys	= p_wk->gmSys;
	GMEVENT				*p_event		= p_wk->gmEvent;
	FIELDMAP_WORK *p_field	= p_wk->fieldWork;
	HEAPID heapID = HEAPID_PROC;
	DEBUG_MAKEPOKE_EVENT_WORK	*p_mp_work;

	//イヴェント
	GMEVENT_Change( p_event, debugMenuMakePoke, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );
	p_mp_work = GMEVENT_GetEventWork( p_event );
	GFL_STD_MemClear( p_mp_work, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );
	
	//ワーク設定
	p_mp_work->p_gamesys	= p_gamesys;
	p_mp_work->p_event		= p_event;
	p_mp_work->p_field		= p_field;
	p_mp_work->heapID		  = heapID;
	p_mp_work->pp = PP_Create( 1,1,PTL_SETUP_ID_AUTO,p_mp_work->heapID );
	p_mp_work->p_mp_work.dst = p_mp_work->pp;

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグポケモン作成イベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_CALL_PROC,
		SEQ_PROC_END,
	};

	DEBUG_MAKEPOKE_EVENT_WORK	*p_wk	= p_wk_adrs;

	switch(*p_seq )
	{	
	case SEQ_CALL_PROC:
		GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, &p_wk->p_mp_work ) );
		*p_seq	= SEQ_PROC_END;
		break;

	case SEQ_PROC_END:
	  if( p_wk->pp != NULL )
	  {
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
      POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
      
      {
        u16 oyaName[6] = {L'で',L'ば',L'ぐ',L'ぽ',L'け',0xFFFF};
        PP_Put( p_wk->pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
        PP_Put( p_wk->pp , ID_PARA_oyasex , PTL_SEX_MALE );
      }
      //手持ちに空きがあれば入れる
      if( PokeParty_GetPokeCount( party ) < 6 )
      {
        PokeParty_Add( party , p_wk->pp );
      }
      
      GFL_HEAP_FreeMemory( p_wk->pp );
    }
		return GMEVENT_RES_FINISH;
		break;
	}

	return GMEVENT_RES_CONTINUE ;
}


//======================================================================
//  デバッグメニュー アイテム
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   デバッグアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk )
{
	GMEVENT *event = wk->gmEvent;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	GAMESYS_WORK	*gameSys	= wk->gmSys;

	EVENT_DebugItemMake(gameSys, fieldWork, event, wk->heapID);

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンボックスにポケモンでいっぱいにする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------

static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
	GAMESYS_WORK	*gameSys	= wk->gmSys;
  MYSTATUS *myStatus;
	POKEMON_PARAM *pp;
	POKEMON_PASO_PARAM	*ppp;
	const STRCODE *name;
	
  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );
  
	pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

	{
		int i,j;
		BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

		for(i=0;i < BOX_MAX_TRAY;i++){
			for(j=0;j < 30;j++){
//				int monsno =GFUser_GetPublicRand(MONSNO_END-1)+1;
        int monsno = i+10;
        monsno = GFUser_GetPublicRand(300);
				OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
				PP_Setup(pp,  monsno , 30, PTL_SETUP_ID_AUTO);

				//以下の親名設定は、090827現在PP_Putで設定できないので、無理やりPPPにするnagihashi
				ppp	= (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
				PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
				PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
        PPP_Put( ppp , ID_PARA_item , 13 );

				BOXDAT_PutPokemonBox(pBox, i, ppp);
			}
		}
	}
	
	GFL_HEAP_FreeMemory(pp);
#endif
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * @brief   バッグにアイテムを限界までいれる
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------

static BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk )
{
	u32	i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gmSys));

	for( i = 0; i < ITEM_DATA_MAX; i++ ){
    if( ITEM_CheckEnable( i ) ){
      MYITEM_AddItem(myitem , i, 2, GFL_HEAPID_APP );
    }
	}
	return( FALSE );
}



//======================================================================
//  デバッグメニュー そらを飛ぶ
//======================================================================
FS_EXTERN_OVERLAY(townmap);
//-------------------------------------
///	デバッグ空を飛ぶ用ワーク	
//=====================================
typedef struct 
{
	GAMESYS_WORK		*p_gamesys;
	GMEVENT					*p_event;
	FIELDMAP_WORK *p_field;
	TOWNMAP_PARAM		*p_param;
} DEBUG_SKYJUMP_EVENT_WORK;
//-------------------------------------
///		PROTOTYPE
//=====================================
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ空を飛ぶ
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk		ワーク
 *
 *	@return	TRUEイベント継続	FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk )
{	
	GAMESYS_WORK	*p_gamesys	= p_wk->gmSys;
	GMEVENT				*p_event		= p_wk->gmEvent;
	FIELDMAP_WORK *p_field	= p_wk->fieldWork;
	DEBUG_SKYJUMP_EVENT_WORK	*p_sky;

	//イヴェント
	GMEVENT_Change( p_event, debugMenuSkyJump, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );
	p_sky = GMEVENT_GetEventWork( p_event );
	GFL_STD_MemClear( p_sky, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );
	
	//ワーク設定
	p_sky->p_gamesys	= p_gamesys;
	p_sky->p_event		= p_event;
	p_sky->p_field		= p_field;

	//タウンマップワーク設定
	p_sky->p_param	= GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(TOWNMAP_PARAM) );
	GFL_STD_MemClear( p_sky->p_param, sizeof(TOWNMAP_PARAM) );
	p_sky->p_param->mode			= TOWNMAP_MODE_DEBUGSKY;
	{
		GAMEDATA		*p_gamedata;
		PLAYER_WORK *p_player;
		p_gamedata	= GAMESYSTEM_GetGameData( p_sky->p_gamesys );
		p_player		= GAMEDATA_GetMyPlayerWork( p_gamedata );
		p_sky->p_param->zoneID		= PLAYERWORK_getZoneID(p_player);
		p_sky->p_param->escapeID	= GAMEDATA_GetEscapeLocation( p_gamedata )->zone_id;
		p_sky->p_param->p_gamesys	= p_gamesys;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ空を飛ぶイベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_CALL_PROC,
		SEQ_PROC_END,
		SEQ_CHANGE_MAP,
		SEQ_EXIT,
	};

	DEBUG_SKYJUMP_EVENT_WORK	*p_wk	= p_wk_adrs;

	switch(*p_seq )
	{	
	case SEQ_CALL_PROC:
		GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(townmap), &TownMap_ProcData, p_wk->p_param ) );
		*p_seq	= SEQ_PROC_END;
		break;

	case SEQ_PROC_END:
		if( p_wk->p_param->select == TOWNMAP_SELECT_SKY )
		{	
			*p_seq	= SEQ_CHANGE_MAP;
		}
		else
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_CHANGE_MAP:
		GMEVENT_CallEvent( p_wk->p_event, DEBUG_EVENT_ChangeMapDefaultPosByWarp( p_wk->p_gamesys, p_wk->p_field, p_wk->p_param->zoneID ) );
		*p_seq	= SEQ_EXIT;
		break;

	case SEQ_EXIT:
		if( p_wk->p_param )
		{	
			GFL_HEAP_FreeMemory(p_wk->p_param);
		}
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  デバッグメニュー　自機男女切り替え
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー　自機男女切り替え
 * @param wk DEBUG_MENU_EVENT_WORK
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ChangePlayerSex( DEBUG_MENU_EVENT_WORK *wk )
{
  int sex;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldMap = wk->fieldWork;
  u16 zone_id = FIELDMAP_GetZoneID( fieldMap );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( wk->gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( wk->gdata );
  MYSTATUS *s_mystatus = SaveData_GetMyStatus( save );
  
  sex = MyStatus_GetMySex( mystatus );
  sex ^= 1;
  MyStatus_SetMySex( mystatus, sex );
  MyStatus_SetMySex( s_mystatus, sex );
  
  {
    GMEVENT * mapchange_event;
    mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldMap, zone_id );
    GMEVENT_ChangeEvent( event, mapchange_event );
  }
  return( TRUE );
}

//======================================================================
//  デバッグメニュー　GTS画面へ
//======================================================================
FS_EXTERN_OVERLAY(worldtrade);
#include "net/dwc_rapcommon.h"
//-------------------------------------
///	デバッグGTS用ワーク	
//=====================================
typedef struct 
{
	GAMESYS_WORK				*p_gamesys;
	GMEVENT							*p_event;
	FIELDMAP_WORK			*p_field;
	EVENT_WIFICLUB_WORK	wifi;
	WORLDTRADE_PARAM		gts;
} DEBUG_WIFIGTS_EVENT_WORK;
static GMEVENT_RESULT debugMenuWifiGts( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *	@brief	GTS画面へいくイベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WifiGts( DEBUG_MENU_EVENT_WORK *p_wk )
{	
	GAMESYS_WORK	*p_gamesys	= p_wk->gmSys;
	GMEVENT				*p_event		= p_wk->gmEvent;
	FIELDMAP_WORK *p_field	= p_wk->fieldWork;
	GAMEDATA *p_gamedata			= GAMESYSTEM_GetGameData(p_gamesys);
	DEBUG_WIFIGTS_EVENT_WORK	*p_gts;

	if( WifiList_CheckMyGSID( SaveData_GetWifiListData(SaveControl_GetPointer())) )
	{	
		//イヴェント
		GMEVENT_Change( p_event, debugMenuWifiGts, sizeof(DEBUG_WIFIGTS_EVENT_WORK) );
		p_gts = GMEVENT_GetEventWork( p_event );
		GFL_STD_MemClear( p_gts, sizeof(DEBUG_WIFIGTS_EVENT_WORK) );


		//ワーク設定
		p_gts->p_gamesys	= p_gamesys;
		p_gts->p_event		= p_event;
		p_gts->p_field		= p_field;

		//WiFiClubワーク設定
		p_gts->wifi.event			=	p_event;
		p_gts->wifi.gsys			= p_gamesys;
		p_gts->wifi.fieldmap	= p_field;
		p_gts->wifi.ctrl			= SaveControl_GetPointer();
		p_gts->wifi.pWork			= NULL;
		p_gts->wifi.isEndProc	= TRUE;
		p_gts->wifi.selectType	= 0;
	
		//GTSワーク設定
		p_gts->gts.savedata					= GAMEDATA_GetSaveControlWork(p_gamedata );
		p_gts->gts.worldtrade_data	= SaveData_GetWorldTradeData(p_gts->gts.savedata);
		p_gts->gts.systemdata				= SaveData_GetSystemData(p_gts->gts.savedata);
		p_gts->gts.myparty					= SaveData_GetTemotiPokemon(p_gts->gts.savedata);
		p_gts->gts.mybox	          = GAMEDATA_GetBoxManager(p_gamedata);
		p_gts->gts.zukanwork				= NULL;
		p_gts->gts.wifilist					= SaveData_GetWifiListData(p_gts->gts.savedata);
		p_gts->gts.wifihistory			= SaveData_GetWifiHistory(p_gts->gts.savedata);
		p_gts->gts.mystatus					= SaveData_GetMyStatus(p_gts->gts.savedata);
		p_gts->gts.config						= SaveData_GetConfig(p_gts->gts.savedata);
		p_gts->gts.record						= SaveData_GetRecord(p_gts->gts.savedata);
		p_gts->gts.myitem						= NULL;
		
		p_gts->gts.zukanmode				= 0;
		p_gts->gts.profileId				= WifiList_GetMyGSID( p_gts->gts.wifilist );
		p_gts->gts.contestflag			= FALSE;
		p_gts->gts.connect					= 0;	
	
		OS_Printf( "GTS Start\n" );
	}
	else
	{	
		OS_Printf( "GameSpyIDが不正なので、GTSを開始しなかった\n" );
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグGTS接続用イベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuWifiGts( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_CALL_WIFI,
		SEQ_PROC_END,
		SEQ_CALL_GTS,
		SEQ_EXIT,
	};

	DEBUG_WIFIGTS_EVENT_WORK	*p_wk	= p_wk_adrs;

	switch(*p_seq )
	{	
	case SEQ_CALL_WIFI:
		*p_seq	= SEQ_PROC_END;
		break;

	case SEQ_PROC_END:
		*p_seq	= SEQ_CALL_GTS;
		break;

	case SEQ_CALL_GTS:
		 GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(worldtrade), &WorldTrade_ProcData, &p_wk->gts ) );
		*p_seq	= SEQ_EXIT;
		break;

	case SEQ_EXIT:
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  デバッグメニュー　GDS接続
//======================================================================
#include "net_app/gds_main.h"
FS_EXTERN_OVERLAY(gds_comm);
//-------------------------------------
///	デバッグGDS用ワーク	
//=====================================
typedef struct 
{
	GAMESYS_WORK				*gsys;
	FIELDMAP_WORK			*fieldWork;
} DEBUG_GDS_EVENT_WORK;
static GMEVENT_RESULT debugMenuGDS( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *	@brief	GTS画面へいくイベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_GDS( DEBUG_MENU_EVENT_WORK *p_wk )
{	
  GAMESYS_WORK *gsys = p_wk->gmSys;
  FIELDMAP_WORK *fieldWork = p_wk->fieldWork;
	GMEVENT				*p_event		= p_wk->gmEvent;
  DEBUG_GDS_EVENT_WORK *p_gds;
  
	if( WifiList_CheckMyGSID( SaveData_GetWifiListData(SaveControl_GetPointer())) )
	{	
		//イヴェント
		GMEVENT_Change( p_event, debugMenuGDS, sizeof(DEBUG_GDS_EVENT_WORK) );
		p_gds = GMEVENT_GetEventWork( p_event );
		p_gds->gsys = gsys;
		p_gds->fieldWork = fieldWork;
	}
	else
	{	
		OS_Printf( "GameSpyIDが不正なので、GTSを開始しなかった\n" );
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグGTS接続用イベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuGDS( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	DEBUG_GDS_EVENT_WORK	*p_gds	= p_wk_adrs;

	switch(*p_seq )
	{	
	case 0:
    {
    	GDSPROC_PARAM *gds_param;
      
      gds_param = GFL_HEAP_AllocClearMemory(HEAPID_PROC, sizeof(GDSPROC_PARAM));
      gds_param->savedata = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(p_gds->gsys));
      gds_param->connect = 0;
      gds_param->gds_mode = 0;
  		
  		GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_gds->gsys, p_gds->fieldWork,
          FS_OVERLAY_ID(gds_comm), &GdsMainProcData, gds_param ) );
  		(*p_seq)++;
    }
		break;
  
  case 1:
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
}

//======================================================================
//  デバッグメニュー UIテンプレートへ
//======================================================================
FS_EXTERN_OVERLAY(ui_debug);
//-------------------------------------
///	デバッグUIテンプレート用ワーク	
//=====================================
typedef struct 
{
	GAMESYS_WORK				*p_gamesys;
	GMEVENT							*p_event;
	FIELDMAP_WORK				*p_field;
	UI_TEMPLATE_PARAM		param;
} DEBUG_UITEMPLATE_EVENT_WORK;
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *	@brief	UIテンプレート接続
 *
 *	@param	DEBUG_MENU_EVENT_WORK *p_wk		ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk )
{	
	GAMESYS_WORK	*p_gamesys	= p_wk->gmSys;
	GMEVENT				*p_event		= p_wk->gmEvent;
	FIELDMAP_WORK *p_field		= p_wk->fieldWork;
	DEBUG_UITEMPLATE_EVENT_WORK	*p_param;

	//イヴェント
	GMEVENT_Change( p_event, debugMenuUITemplate, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );
	p_param = GMEVENT_GetEventWork( p_event );
	GFL_STD_MemClear( p_param, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );

	p_param->p_gamesys	= p_gamesys;
	p_param->param.gamesys	= p_gamesys;
	p_param->p_event		= p_event;
	p_param->p_field		= p_field;

	OS_Printf( "UIテンプレート Start\n" );

	return TRUE;

}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグGTS接続用イベント
 *
 *	@param	GMEVENT *event	GMEVENT
 *	@param	*seq						シーケンス
 *	@param	*work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_CALL_PROC,
		SEQ_EXIT,
	};

	DEBUG_UITEMPLATE_EVENT_WORK	*p_wk	= p_wk_adrs;

	switch(*p_seq )
	{	
	case SEQ_CALL_PROC:
		 	GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(ui_debug), &UITemplateProcData, &p_wk->param ) );
		*p_seq	= SEQ_EXIT;
		break;

	case SEQ_EXIT:
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  怪力
//======================================================================
//--------------------------------------------------------------
/**
 * 怪力実行
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk )
{
	GAMESYS_WORK *gsys = wk->gmSys;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( wk->gdata );
  EVENTWORK_SetEventFlag( evwork, SYS_FLAG_KAIRIKI );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　線形カメラ操作
 * @param	wk	DEBUG_MENU_EVENT_WORK*
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk )
{
	DEBUG_CTL_LINERCAMERA_WORK *work;
	GAMESYS_WORK *gsys = wk->gmSys;
	GMEVENT *event = wk->gmEvent;
	HEAPID heapID = wk->heapID;
	FIELDMAP_WORK *fieldWork = wk->fieldWork;
	
	GMEVENT_Change( event, debugMenuControlLinerCamera, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );
	work = GMEVENT_GetEventWork( event );
	MI_CpuClear8( work, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );
	
	work->gsys = gsys;
	work->event = event;
	work->heapID = heapID;
	work->fieldWork = fieldWork;

  {

    FIELD_SUBSCREEN_WORK * subscreen;
    

    // カメラ操作は下画面で行う
    subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldWork);
    FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
    { 
      void * inner_work;
      FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);
      inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(subscreen);
      FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS);

      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(cam);
    }
  }
  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    
    if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_NOGRID )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
  }

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event work
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLinerCamera(
		GMEVENT *event, int *seq, void *wk )
{
  VecFx32 add = {0,0,0};
  BOOL shift = FALSE;

	DEBUG_CTL_LINERCAMERA_WORK *work = wk;
  FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);


  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    FIELD_CAMERA_DEBUG_ReleaseSubScreen( cam );

    // レールカメラ反映の再開
    {
      FLDNOGRID_MAPPER* mapper;
      
      if( FIELDMAP_GetMapControlType( work->fieldWork ) == FLDMAP_CTRLTYPE_NOGRID )
      {
        mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
        FLDNOGRID_MAPPER_SetRailCameraActive( mapper, TRUE );
      }
    }
    //バインド復帰
    FIELD_CAMERA_BindDefaultTarget(cam);
	  return( GMEVENT_RES_FINISH );
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG ){
    DampCameraInfo(cam);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){             //奥
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){             //手前
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){             //左
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){             //右
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){             //上
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.y);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){             //下
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.y);
  }else{
    work->Wait = 0;
    work->Count = 0;
  }
  if(shift){
    VecFx32 pos;
    FIELD_CAMERA_GetTargetPos( cam, &pos );
    VEC_Add( &pos, &add, &pos );
    FIELD_CAMERA_SetTargetPos( cam, &pos );
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作キー入力
 * @param	work      カメラワーク
 * @param	inAddval  気入力結果の際の加算値
 * @param	outVal    格納バッファ
 * @retval	BOOL    加算したらTRUE
 */
//--------------------------------------------------------------
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal)
{
  if (work->Wait >= LINER_CAM_KEY_WAIT){
    *outVal = inAddVal;
    return TRUE;
  }else{
    work->Wait++;
    if (work->Count>0){
      work->Count--;
    }else{
      *outVal = inAddVal;
      work->Count = LINER_CAM_KEY_COUNT;
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作 情報ダンプ
 * @param	cam   フィールドカメラポインタ
 * @retval	none
 */
//--------------------------------------------------------------
static void DampCameraInfo(FIELD_CAMERA * cam)
{
  u16 pitch;
  u16 yaw;
  fx32 len;
  VecFx32 target;
  u16 fovy;
  VecFx32 shift;

  OS_Printf("--DUMP_FIELD_CAMERA_PARAM--\n");

  pitch = FIELD_CAMERA_GetAnglePitch(cam);
  yaw = FIELD_CAMERA_GetAngleYaw(cam);
  len = FIELD_CAMERA_GetAngleLen(cam);
  fovy = FIELD_CAMERA_GetFovy(cam);
  FIELD_CAMERA_GetTargetOffset( cam, &shift );
  FIELD_CAMERA_GetTargetPos( cam, &target );

  OS_Printf("%d,%d,%d,%d,%d,%d\n", pitch, yaw, len, target.x, target.y, target.z );
#ifdef DEBUG_ONLY_FOR_saitou
  OS_Printf("%d,%d,%d,%d\n", fovy, shift.x, shift.y, shift.z );
#endif
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　Yボタン登録操作
 * @param	wk	DEBUG_MENU_EVENT_WORK*	ワーク
 * @retval	BOOL	TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk )
{	
	int i;
	GAMESYS_WORK *gsys = wk->gmSys;
	SAVE_CONTROL_WORK *p_sv	= GAMEDATA_GetSaveControlWork(wk->gdata);
	SHORTCUT *p_shortcut_sv	= SaveData_GetShortCut( p_sv );
	for( i = 0; i < SHORTCUT_ID_MAX; i++ )
	{	
		SHORTCUT_SetRegister( p_shortcut_sv, i, TRUE );
	}
	
	return FALSE;
}
