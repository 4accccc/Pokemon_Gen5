//======================================================================
/**
 *
 * @file  event_fieldmap_menu.c
 * @brief フィールドマップメニューの制御イベント
 * @author  kagaya
 * @date  2008.11.13
 *
 * モジュール名：FIELD_MENU
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"


#include "message.naix"
#include "msg/msg_fldmapmenu.h"
#include "test/easy_pokelist.h"

#include "system/wipe.h"
#include "gamesystem/game_comm.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "field/field_msgbg.h"
#include "field/event_fieldmap_menu.h"
#include "field/fieldmap.h"
#include "field/map_attr.h"

#include "event_fieldmap_control.h" //EVENT_FieldSubProc
#include "app/config_panel.h"   //ConfigPanelProcData
#include "app/trainer_card.h"   //TrainerCardSysProcData
#include "app/bag/itemmenu_local.h" //ItemMenuProcData
#include "app/pokelist.h"   //PokeList_ProcData・PLIST_DATA

extern const GFL_PROC_DATA DebugAriizumiMainProcData;
extern const GFL_PROC_DATA TrainerCardProcData;

//======================================================================
//  define
//======================================================================
#define BGFRAME_MENU  (GFL_BG_FRAME1_M) //使用フレーム
#define MENU_PANO (14)            //メニューパレットNo
#define FONT_PANO (15)            //フォントパレットNo
#define MENU_CHARSIZE_X (8)         //メニュー横幅
#define MENU_CHARSIZE_Y (16)        //メニュー縦幅

//--------------------------------------------------------------
/// FMENURET
//--------------------------------------------------------------
typedef enum
{
  FMENURET_CONTINUE,
  FMENURET_FINISH,
  FMENURET_NEXTEVENT,
}FMENURET;

typedef enum
{
  FMENUSTATE_INIT,
  FMENUSTATE_MAIN,
  FMENUSTATE_DECIDE_ITEM, //項目が決定された
  FMENUSTATE_WAIT_RETURN,
  FMENUSTATE_RETURN_MENU,
  FMENUSTATE_EXIT_MENU,
}FMENU_STATE;

//======================================================================
//  typedef struct
//======================================================================
/// FMENU_EVENT_WORK
typedef struct _TAG_FMENU_EVENT_WORK FMENU_EVENT_WORK;

//--------------------------------------------------------------
/// メニュー呼び出し関数
/// BOOL TRUE=イベント継続 FALSE==フィールドマップメニューイベント終了
//--------------------------------------------------------------
typedef BOOL (* FMENU_CALLPROC)(FMENU_EVENT_WORK*);

//--------------------------------------------------------------
/// FMENU_LISTDATA
//--------------------------------------------------------------
typedef struct
{
  u16 charsize_x;
  u16 charsize_y;
  u32 max;
  const FLDMENUFUNC_LIST *list;
}FMENU_LISTDATA;

//--------------------------------------------------------------
/// FMENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_FMENU_EVENT_WORK
{
  HEAPID heapID;
  GMEVENT *gmEvent;
  GAMESYS_WORK *gmSys;
  FIELD_MAIN_WORK *fieldWork;
  
  FIELD_MENU_ITEM_TYPE befType;
  
  int menu_num;
  FMENU_CALLPROC call_proc;
  
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  
  void *sub_proc_parent;
};

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT FldMapMenuEvent( GMEVENT *event, int *seq, void *wk );

static BOOL FMenuCallProc_Zukan( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_PokeStatus( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Bag( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_MyTrainerCard( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Report( FMENU_EVENT_WORK *mwk );
static BOOL FMenuCallProc_Config( FMENU_EVENT_WORK *mwk );

static GMEVENT * createFMenuMsgWinEvent(
  GAMESYS_WORK *gsys, u32 heapID, u32 strID, FLDMSGBG *msgBG );
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk );

static GMEVENT * createFMenuReportEvent(
  GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, u32 heapID, FLDMSGBG *msgBG );
static GMEVENT_RESULT FMenuReportEvent( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/// フィールドマップメニューリスト
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_FldMapMenuList[] =
{
  { FLDMAPMENU_STR01, FMenuCallProc_Zukan },
  { FLDMAPMENU_STR02, FMenuCallProc_PokeStatus },
  { FLDMAPMENU_STR03, FMenuCallProc_Bag },
  { FLDMAPMENU_STR04, FMenuCallProc_MyTrainerCard },
  { FLDMAPMENU_STR05, FMenuCallProc_Report },
  { FLDMAPMENU_STR06, FMenuCallProc_Config },
  { FLDMAPMENU_STR07, NULL },
};

//--------------------------------------------------------------
/// フィールドマップメニューリストテーブル
//--------------------------------------------------------------
static const FMENU_LISTDATA DATA_FldMapMenuListTbl[] =
{
  {
    MENU_CHARSIZE_X,
    MENU_CHARSIZE_Y,
    NELEMS(DATA_FldMapMenuList),
    DATA_FldMapMenuList
  },
};

//メニュー最大数
#define MENULISTTBL_MAX (NELEMS(DATA_FldMapMenuListTbl))

//--------------------------------------------------------------
/// メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_FldMapMenuListHeader =
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
  6,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

//======================================================================
//  イベント：フィールドマップメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップメニューイベント起動
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELD_MAIN_WORK
 * @param heapID  HEAPID
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_FieldMapMenu(
  GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, HEAPID heapID )
{
  FMENU_EVENT_WORK *mwk;
  GMEVENT * event;
  event = GMEVENT_Create(
    gsys, NULL, FldMapMenuEvent, sizeof(FMENU_EVENT_WORK));
  
  mwk = GMEVENT_GetEventWork(event);
  MI_CpuClear8( mwk, sizeof(FMENU_EVENT_WORK) );
  
  mwk->gmSys = gsys;
  mwk->gmEvent = event;
  mwk->fieldWork = fieldWork;
  mwk->heapID = heapID;
  mwk->befType = FMIT_POKEMON;
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント：フィールドフィールドマップメニュー
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FldMapMenuEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_EVENT_WORK *mwk = wk;
  switch (*seq) 
  {
  case FMENUSTATE_INIT:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) ) == TRUE )
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      MMDLSYS_PauseMoveProc( fldMdlSys );
      GAMEDATA_SetSubScreenType( gameData , FMIT_POKEMON );
      FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
      (*seq) = FMENUSTATE_MAIN;
    }
    break;
  case FMENUSTATE_MAIN:
    {
      const FIELD_SUBSCREEN_ACTION action = FIELD_SUBSCREEN_GetAction(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork));
      if( action == FIELD_SUBSCREEN_ACTION_TOPMENU_DECIDE )
      {
        //何か項目が決定された
        (*seq) = FMENUSTATE_DECIDE_ITEM;
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      }
      else
      if( action == FIELD_SUBSCREEN_ACTION_TOPMENU_EXIT )
      {
        //キャンセル
        (*seq) = FMENUSTATE_EXIT_MENU;
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      }
    }
    break;
  case FMENUSTATE_DECIDE_ITEM:
    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      const FIELD_MENU_ITEM_TYPE type = FIELD_SUBSCREEN_GetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      mwk->befType = type;
      GAMEDATA_SetSubScreenType( gameData , type );
      
      switch( type )
      {
        case FMIT_POKEMON:
          mwk->call_proc = FMenuCallProc_PokeStatus;
          break;

        case FMIT_ZUKAN:
          mwk->call_proc = FMenuCallProc_Zukan;
          break;

        case FMIT_ITEMMENU:
          mwk->call_proc = FMenuCallProc_Bag;
          break;

        case FMIT_TRAINERCARD:
          mwk->call_proc = FMenuCallProc_MyTrainerCard;
          break;

        case FMIT_REPORT:
          mwk->call_proc = FMenuCallProc_Report;
          break;

        case FMIT_CONFING:
          mwk->call_proc = FMenuCallProc_Config;
          break;
      }
      if( mwk->call_proc != NULL ){
        GF_ASSERT(mwk->sub_proc_parent == NULL);
        if( mwk->call_proc(mwk) == TRUE ){
          mwk->call_proc = NULL;
          (*seq) = FMENUSTATE_WAIT_RETURN;
          return( GMEVENT_RES_CONTINUE );
        }
      }
      else
      {
        (*seq) = FMENUSTATE_EXIT_MENU;
      }
     }
    break;

  case FMENUSTATE_WAIT_RETURN:
    /* sub event 終了待ち */
    if(mwk->sub_proc_parent != NULL)
    {
      GFL_HEAP_FreeMemory(mwk->sub_proc_parent);
      mwk->sub_proc_parent = NULL;
    }
    (*seq) = FMENUSTATE_RETURN_MENU;
    break;
    
  case FMENUSTATE_RETURN_MENU:
    (*seq) = FMENUSTATE_MAIN;
    break;
  
  case FMENUSTATE_EXIT_MENU:
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
      MMDLSYS_ClearPauseMoveProc( fldMdlSys );
      FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_NORMAL);
    }
    return( GMEVENT_RES_FINISH );
    break;
  
  }
  return( GMEVENT_RES_CONTINUE );

#if 0
  FMENU_EVENT_WORK *mwk = wk;
  
  switch (*seq) {
  case 0:
    {
      FLDMSGBG *msgBG;
      FLDMENUFUNC_HEADER head;
      FLDMENUFUNC_LISTDATA *listdata;
      const FLDMENUFUNC_LIST *menulist; 
      const FMENU_LISTDATA *fmenu_listdata;
      
      msgBG = FIELDMAP_GetFldMsgBG( mwk->fieldWork );
      mwk->msgData = FLDMSGBG_CreateMSGDATA(
        msgBG, NARC_message_fldmapmenu_dat );
      
      fmenu_listdata = &DATA_FldMapMenuListTbl[mwk->menu_num];
      menulist = fmenu_listdata->list;
      
      listdata = FLDMENUFUNC_CreateMakeListData(
        menulist, fmenu_listdata->max, mwk->msgData, mwk->heapID );
      
      head = DATA_FldMapMenuListHeader;
      FLDMENUFUNC_InputHeaderListSize( &head, fmenu_listdata->max,
        23, 1, fmenu_listdata->charsize_x, fmenu_listdata->charsize_y );
      
      mwk->menuFunc = FLDMENUFUNC_AddMenu( msgBG, &head, listdata );
      GFL_MSG_Delete( mwk->msgData );
    }
    
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( mwk->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else{              //決定
        mwk->call_proc = (FMENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;
  case 2:
    {
      FLDMENUFUNC_DeleteMenu( mwk->menuFunc );
      
      if( mwk->call_proc != NULL ){
        GF_ASSERT(mwk->sub_proc_parent == NULL);
        if( mwk->call_proc(mwk) == TRUE ){
          mwk->call_proc = NULL;
          (*seq)++;
          return( GMEVENT_RES_CONTINUE );
        }
      }
      
      return( GMEVENT_RES_FINISH );
    }
    break;
  case 3:
    /* sub event 終了待ち */
    if(mwk->sub_proc_parent != NULL){
      GFL_HEAP_FreeMemory(mwk->sub_proc_parent);
      mwk->sub_proc_parent = NULL;
    }
    (*seq) = 0;
    break;
  }
  return( GMEVENT_RES_CONTINUE );
#endif
}

//======================================================================
//  フィールドマップメニュー呼び出し
//======================================================================
//--------------------------------------------------------------
/**
 * メニュー呼び出し 図鑑イベント
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Zukan( FMENU_EVENT_WORK *mwk )
{
  GMEVENT * subevent = createFMenuMsgWinEvent( mwk->gmSys, mwk->heapID,
    FLDMAPMENU_STR08, FIELDMAP_GetFldMsgBG(mwk->fieldWork) );
  GMEVENT_CallEvent(mwk->gmEvent, subevent);
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し ポケモンステータス
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_PokeStatus( FMENU_EVENT_WORK *mwk )
{
  //GMEVENT * subevent = createFMenuMsgWinEvent(mwk->gmSys, mwk->heapID, FLDMAPMENU_STR09);
  //GMEVENT_CallEvent(mwk->gmEvent, subevent);
  GMEVENT * newEvent;
  /*
  EASY_POKELIST_PARENT *epp;
  
  epp = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EASY_POKELIST_PARENT));
  epp->party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(mwk->gmSys));
  mwk->sub_proc_parent = epp;
  newEvent = EVENT_FieldSubProc(mwk->gmSys, mwk->fieldWork,
      FS_OVERLAY_ID(pokelist), &EasyPokeListData, epp);
  */
  
  PLIST_DATA *plistData;
  plistData = GFL_HEAP_AllocClearMemory(HEAPID_PROC, sizeof(PLIST_DATA));
  plistData->pp = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(mwk->gmSys));
  plistData->mode = PL_MODE_FIELD;

  mwk->sub_proc_parent = plistData;
  newEvent = EVENT_FieldSubProc(mwk->gmSys, mwk->fieldWork,
      FS_OVERLAY_ID(pokelist), &PokeList_ProcData, plistData);
    
  GMEVENT_CallEvent(mwk->gmEvent, newEvent);
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し バッグ
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(bag);

static BOOL FMenuCallProc_Bag( FMENU_EVENT_WORK *mwk )
{
  GMEVENT * newEvent;
  FIELD_ITEMMENU_WORK *epp;
	VecFx32 aPos;
	const FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer(mwk->fieldWork);
	FLDMAPPER *g3Dmapper = FIELDMAP_GetFieldG3Dmapper(mwk->fieldWork);
	GAMEDATA* pGameData = GAMESYSTEM_GetGameData(mwk->gmSys);
  
  epp = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(FIELD_ITEMMENU_WORK));
  epp->ctrl = SaveControl_GetPointer();
  epp->gsys = mwk->gmSys;
  epp->fieldmap = mwk->fieldWork;
	epp->heapID = GFL_HEAPID_APP;
	epp->mode = BAG_MODE_FIELD;
	FIELD_PLAYER_GetPos(fld_player, &aPos);
	epp->icwk.NowAttr = MAPATTR_GetAttribute(g3Dmapper, &aPos);
	FIELD_PLAYER_GetDirPos((FIELD_PLAYER*)fld_player, FIELD_PLAYER_GetDir(fld_player), &aPos);
	epp->icwk.FrontAttr = MAPATTR_GetAttribute(g3Dmapper, &aPos);
	epp->icwk.gsys = mwk->gmSys;
	//ゾーンＩＤ
	epp->icwk.zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(pGameData ));
	//連れ歩き
	epp->icwk.Companion = FALSE;
	epp->mystatus = GAMEDATA_GetMyStatus(pGameData);

  mwk->sub_proc_parent = epp;

	newEvent = EVENT_FieldSubProc(mwk->gmSys, mwk->fieldWork, FS_OVERLAY_ID(bag), &ItemMenuProcData, epp);
  epp->event = newEvent;

	GMEVENT_CallEvent(mwk->gmEvent, newEvent);

	return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し 自分
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_MyTrainerCard( FMENU_EVENT_WORK *mwk )
{
  //GMEVENT * subevent = createFMenuMsgWinEvent(mwk->gmSys, mwk->heapID, FLDMAPMENU_STR11);
  //GMEVENT_CallEvent(mwk->gmEvent, subevent);
  GMEVENT * newEvent;
  TRCARD_CALL_PARAM *callParam = TRAINERCASR_CreateCallParam_SelfData( 
                                    GAMESYSTEM_GetGameData( mwk->gmSys ), 
                                    HEAPID_PROC );

  newEvent = EVENT_FieldSubProc(mwk->gmSys, mwk->fieldWork,
      TRCARD_OVERLAY_ID, &TrCardSysProcData, callParam);
  GMEVENT_CallEvent(mwk->gmEvent, newEvent);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し レポート
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Report( FMENU_EVENT_WORK *mwk )
{
  GMEVENT * subevent = createFMenuReportEvent( mwk->gmSys, mwk->fieldWork, mwk->heapID,
      FIELDMAP_GetFldMsgBG(mwk->fieldWork) );
  GMEVENT_CallEvent(mwk->gmEvent, subevent);
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * メニュー呼び出し 設定
 * @param mwk FMENU_EVENT_WORK
 * @retval  BOOL  TRUE=イベント切り替え
 */
//--------------------------------------------------------------
static BOOL FMenuCallProc_Config( FMENU_EVENT_WORK *mwk )
{
  //GMEVENT * subevent = createFMenuMsgWinEvent(mwk->gmSys, mwk->heapID, FLDMAPMENU_STR13);
  //GMEVENT_CallEvent(mwk->gmEvent, subevent);
  GMEVENT * newEvent;
  GAMEDATA *gameData = GAMESYSTEM_GetGameData( mwk->gmSys );
  SAVE_CONTROL_WORK *saveControl = GAMEDATA_GetSaveControlWork( gameData );
  CONFIG *config = SaveData_GetConfig( saveControl );
  newEvent = EVENT_FieldSubProc(mwk->gmSys, mwk->fieldWork,
      FS_OVERLAY_ID(config_panel), &ConfigPanelProcData, config);
  GMEVENT_CallEvent(mwk->gmEvent, newEvent);
  return TRUE;
}

//======================================================================
//  メッセージウィンドウ表示イベント
//======================================================================
//--------------------------------------------------------------
/// FMENU_MSGWIN_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  u32 heapID;
  u32 strID;
  FLDMSGBG *msgBG;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
}FMENU_MSGWIN_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * createFMenuMsgWinEvent(
  GAMESYS_WORK *gsys, u32 heapID, u32 strID, FLDMSGBG *msgBG )
{
  GMEVENT * msgEvent;
  FMENU_MSGWIN_EVENT_WORK *work;
  
  msgEvent = GMEVENT_Create(
    gsys, NULL, FMenuMsgWinEvent, sizeof(FMENU_MSGWIN_EVENT_WORK));
  work = GMEVENT_GetEventWork( msgEvent );
  MI_CpuClear8( work, sizeof(FMENU_MSGWIN_EVENT_WORK) );
  work->msgBG = msgBG;
  work->heapID = heapID;
  work->strID = strID;
  return msgEvent;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_MSGWIN_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    work->msgData = FLDMSGBG_CreateMSGDATA(
      work->msgBG, NARC_message_fldmapmenu_dat );
    work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
    FLDMSGWIN_Print( work->msgWin, 0, 0, work->strID );
    GXS_SetMasterBrightness(-16);
    (*seq)++;
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );
    GXS_SetMasterBrightness(0);
    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
       
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );
    }
    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  レポート表示イベント
//======================================================================
//--------------------------------------------------------------
/// FMENU_REPORT_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  u32 heapID;
  FLDMSGBG *msgBG;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  GAMESYS_WORK *gsys;
  FIELD_MAIN_WORK *fieldWork;
}FMENU_REPORT_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * createFMenuReportEvent(GAMESYS_WORK *gsys, FIELD_MAIN_WORK *fieldWork, u32 heapID, FLDMSGBG *msgBG )
{
  GMEVENT * msgEvent;
  FMENU_REPORT_EVENT_WORK *work;

  msgEvent = GMEVENT_Create(
    gsys, NULL, FMenuReportEvent, sizeof(FMENU_REPORT_EVENT_WORK));
  work = GMEVENT_GetEventWork( msgEvent );
  MI_CpuClear8( work, sizeof(FMENU_REPORT_EVENT_WORK) );
  work->msgBG = msgBG;
  work->heapID = heapID;
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  return msgEvent;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuReportEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_REPORT_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    {
      GAME_COMM_SYS_PTR commSys = GAMESYSTEM_GetGameCommSysPtr( work->gsys );
      work->msgData = FLDMSGBG_CreateMSGDATA(
             work->msgBG, NARC_message_fldmapmenu_dat );
      
      work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
      
      if( GameCommSys_BootCheck(commSys) == GAME_COMM_NO_INVASION )
      {
        //通信中でレポートが書けない
        FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR16 );
        GXS_SetMasterBrightness(-16);
        (*seq) = 20;
      }
      else
      {
        FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR14 );
        //本来ならレポート用への切り替え？
        //FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
        GXS_SetMasterBrightness(-16);
        (*seq)++;
      }
    }
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    GAMEDATA_Save(GAMESYSTEM_GetGameData(GMEVENT_GetGameSysWork(event)));
    (*seq)++;
    break;
  case 3:
    FLDMSGWIN_Delete( work->msgWin );
    
    work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
    FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR15 );
    (*seq)++;
    break;
  case 4:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 5:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 6:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );
    //本来ならメニューへの切り替え？
    //FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
    GXS_SetMasterBrightness(0);
    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
       
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );
    }
    return( GMEVENT_RES_FINISH );
    break;
    
    //セーブできません
  case 20:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 21:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 22:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );
    GXS_SetMasterBrightness(0);
    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
      
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );
    }
    return( GMEVENT_RES_FINISH );
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}
