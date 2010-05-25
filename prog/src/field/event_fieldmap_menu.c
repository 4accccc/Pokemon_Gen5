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

#include "system/wipe.h"
#include "gamesystem/game_comm.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "field/field_msgbg.h"
#include "field/event_fieldmap_menu.h"
#include "field/fieldmap.h"
#include "field/map_attr.h"
#include "field/itemuse_event.h"
#include "field/zonedata.h"
#include "field/event_field_proclink.h"
#include "field/field_skill.h"
#include "field/event_subscreen.h"

//======================================================================
//  define
//======================================================================

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
  FMENUSTATE_EXIT_MENU,
  FMENUSTATE_USE_SKILL,

  FMENUSTATE_CALL_PROC,
  FMENUSTATE_RETURN_PROC,

  FMENUSTATE_USE_ITEM,
  FMENUSTATE_RETURN_ITEM,
  
}FMENU_STATE;

//======================================================================
//  typedef struct
//======================================================================
/// FMENU_EVENT_WORK
typedef struct _TAG_FMENU_EVENT_WORK FMENU_EVENT_WORK;

//--------------------------------------------------------------
/// FMENU_EVENT_WORK
//--------------------------------------------------------------
struct _TAG_FMENU_EVENT_WORK
{
  HEAPID heapID;
  GMEVENT *gmEvent;
  GAMESYS_WORK *gmSys;
  FIELDMAP_WORK *fieldWork;
  FIELD_MENU_ITEM_TYPE  item_type;
  FMENU_STATE state;
  
  FIELD_SUBSCREEN_MODE return_subscreen_mode;

  EVENT_PROCLINK_PARAM  link; //プロセスリンク情報
  
};

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT FldMapMenuEvent( GMEVENT *event, int *seq, void *wk );
static EVENT_PROCLINK_CALL_TYPE FldMapGetProclinkToMenuItem( FIELD_MENU_ITEM_TYPE type );

//コールバック
static BOOL FldMapMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
static BOOL FldMapMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
static BOOL FldMapMenu_InitBefore_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
//======================================================================
//  イベント：フィールドマップメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップメニューイベント起動
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param heapID  HEAPID
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_FieldMapMenu(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heapID )
{
  FMENU_EVENT_WORK *mwk;
  GMEVENT * event;
  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  
  event = GMEVENT_Create( gsys, NULL, FldMapMenuEvent, sizeof(FMENU_EVENT_WORK));
  
  mwk = GMEVENT_GetEventWork(event);

  GFL_STD_MemClear( mwk, sizeof(FMENU_EVENT_WORK) );
  
  mwk->gmSys = gsys;
  mwk->gmEvent = event;
  mwk->fieldWork = fieldWork;
  mwk->heapID = heapID;
  mwk->state = FMENUSTATE_INIT;

  mwk->link.gsys      = gsys;
  mwk->link.field_wk  = fieldWork;
  mwk->link.event     = event;
  mwk->link.open_func   = FldMapMenu_Open_Callback;
  mwk->link.close_func  = FldMapMenu_Close_Callback;
  mwk->link.init_before_func  = FldMapMenu_InitBefore_Callback;
  mwk->link.wk_adrs   = mwk;
  mwk->link.data      = EVENT_PROCLINK_DATA_NONE;

  switch(FIELD_SUBSCREEN_GetMode(subscreen)){
  case FIELD_SUBSCREEN_INTRUDE:
    mwk->return_subscreen_mode = FIELD_SUBSCREEN_INTRUDE;
    break;
  case FIELD_SUBSCREEN_BEACON_VIEW:
    mwk->return_subscreen_mode = FIELD_SUBSCREEN_BEACON_VIEW;
    break;
  default:
    mwk->return_subscreen_mode = 
      FIELD_SUBSCREEN_CGearCheck( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_NORMAL );
  }

  // Xボタンで開始しているが、タッチ開始時のようにカーソルは出さない
  GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );

  return event;
}

//--------------------------------------------------------------
/**
 * ユニオンルーム用フィールドマップメニューイベント起動
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param heapID  HEAPID
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_UnionMapMenu(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heapID )
{
  FMENU_EVENT_WORK *mwk;
  GMEVENT * event;
  
  event = EVENT_FieldMapMenu(gsys, fieldWork, heapID);
  
  mwk = GMEVENT_GetEventWork(event);
  if(ZONEDATA_IsColosseum(FIELDMAP_GetZoneID(fieldWork)) == TRUE){
    mwk->return_subscreen_mode = FIELD_SUBSCREEN_NOGEAR;
  }
  else{
    mwk->return_subscreen_mode = FIELD_SUBSCREEN_UNION;
  }

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
  switch (mwk->state) 
  {
    //-------------------------------------
    ///   メニュー制御
    //=====================================
  case FMENUSTATE_INIT:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) ) == TRUE )
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      MMDLSYS_PauseMoveProc( fldMdlSys );
      GAMEDATA_SetSubScreenType( gameData , FMIT_POKEMON );
      FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
      mwk->state = FMENUSTATE_MAIN;
    }
    break;
  case FMENUSTATE_MAIN:
    {
      const FIELD_SUBSCREEN_ACTION action = FIELD_SUBSCREEN_GetAction(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork));
      if( action == FIELD_SUBSCREEN_ACTION_TOPMENU_DECIDE )
      {
        //何か項目が決定された
        mwk->state = FMENUSTATE_DECIDE_ITEM;
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      }
      else
      if( action == FIELD_SUBSCREEN_ACTION_TOPMENU_EXIT )
      {
        //キャンセル
        mwk->state = FMENUSTATE_EXIT_MENU;
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      }
/*      else
      if( action == FIELD_SUBSCREEN_ACTION_DEBUG_PALACEJUMP )
      {
        //キャンセル+パレスジャンプ
        mwk->state = FMENUSTATE_EXIT_MENU;
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      }*/
    }
    break;
  case FMENUSTATE_DECIDE_ITEM:
    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      const FIELD_MENU_ITEM_TYPE type = FIELD_SUBSCREEN_GetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );
      GAMEDATA_SetSubScreenType( gameData , type );

      mwk->item_type  = FIELD_SUBSCREEN_GetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork) );

      mwk->link.call  = FldMapGetProclinkToMenuItem( type );
      mwk->state      = FMENUSTATE_CALL_PROC;
     }
    break;
    
  case FMENUSTATE_EXIT_MENU:
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
      MMDLSYS_ClearPauseMoveProc( fldMdlSys );
      //Exitの場合はCallbackの中で戻されるので行わない
      if( mwk->link.result != EVENT_PROCLINK_RESULT_EXIT
       && mwk->link.result != EVENT_PROCLINK_RESULT_DOWSINGMACHINE ) // DOWSINGMACHINEの場合もCallbackの中で変えられるので行わない
      { 
//        FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), mwk->return_subscreen_mode);
        GMEVENT_ChangeEvent( event, EVENT_ChangeSubScreen( GMEVENT_GetGameSysWork( event ), mwk->fieldWork, mwk->return_subscreen_mode ));
        return( GMEVENT_RES_CONTINUE );
      }
      return( GMEVENT_RES_FINISH );
    }
    break;

    //-------------------------------------
    ///   フィールド技使用
    //=====================================
  case FMENUSTATE_USE_SKILL:
    {
      FLDSKILL_CHECK_WORK skillCheckWork;
      FLDSKILL_USE_HEADER skillUseWork;
      GMEVENT *event;

      //フィールド復帰
      {
        MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
        MMDLSYS_ClearPauseMoveProc( fldMdlSys );
      }

      //技使用
      FLDSKILL_InitCheckWork( GAMESYSTEM_GetFieldMapWork(mwk->gmSys), &skillCheckWork ); //再初期化
      FLDSKILL_InitUseHeader( &skillUseWork , mwk->link.select_poke, mwk->link.select_param, mwk->link.zoneID);

      event = FLDSKILL_UseSkill( mwk->link.select_param, &skillUseWork , &skillCheckWork );
      if( event == NULL ){  //起動エラーチェック
        return( GMEVENT_RES_FINISH ); //なにもしないで終わる
      }
      GMEVENT_ChangeEvent(mwk->gmEvent , event);
    }
    break;

    //-------------------------------------
    ///   バッグ、メニューなどアプリへ飛ぶ
    //=====================================
  case FMENUSTATE_CALL_PROC:
    GMEVENT_CallEvent( event, EVENT_ProcLink( &mwk->link, mwk->heapID ) );
    mwk->state = FMENUSTATE_RETURN_PROC;
    break;

  case FMENUSTATE_RETURN_PROC:
    if( mwk->link.result == EVENT_PROCLINK_RESULT_RETURN )
    {
      mwk->state  = FMENUSTATE_MAIN;
    }
    else if( mwk->link.result == EVENT_PROCLINK_RESULT_EXIT )
    { 
      mwk->state  = FMENUSTATE_EXIT_MENU;
    }
    else if( mwk->link.result == EVENT_PROCLINK_RESULT_ITEM )
    { 
      mwk->state  = FMENUSTATE_USE_ITEM;
    }
    else if( mwk->link.result == EVENT_PROCLINK_RESULT_SKILL )
    { 
      mwk->state  = FMENUSTATE_USE_SKILL;
    }
    else if( mwk->link.result == EVENT_PROCLINK_RESULT_DOWSINGMACHINE )
    {
      mwk->state  = FMENUSTATE_EXIT_MENU;
    }
    break;

    //-------------------------------------
    ///   アイテム使用
    //=====================================
  case FMENUSTATE_USE_ITEM:
    { 
      GMEVENT *p_item_event;
      //フィールド復帰
      {
        MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( mwk->fieldWork );
        MMDLSYS_ClearPauseMoveProc( fldMdlSys );
      }

      //ITEMの場合はCallbackの中で戻されるので行わない
      //FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), mwk->return_subscreen_mode);

      //アイテムコール
      //GFL_OVERLAY_Load( FS_OVERLAY_ID(itemuse) );
      p_item_event  = EVENT_FieldItemUse( mwk->link.select_param, mwk->gmSys, mwk->fieldWork );
      GMEVENT_CallEvent(event, p_item_event );
      mwk->state = FMENUSTATE_RETURN_ITEM;
    }
    break;

  case FMENUSTATE_RETURN_ITEM:
    //GFL_OVERLAY_Unload( FS_OVERLAY_ID(itemuse) );
    return (GMEVENT_RES_FINISH);
  }

  return( GMEVENT_RES_CONTINUE );

}
//----------------------------------------------------------------------------
/**
 *  @brief  FIELD_MENU_ITEM_TYPEからEVENT_PROCLINK_CALL_TYPEへの変換
 *
 *  @param  FIELD_MENU_ITEM_TYPE type   メニューアイテム
 *
 *  @return プロックリンク
 */
//-----------------------------------------------------------------------------
static EVENT_PROCLINK_CALL_TYPE FldMapGetProclinkToMenuItem( FIELD_MENU_ITEM_TYPE type )
{ 
  static const int proclink_to_menu_item_tbl[ FMIT_MAX ]  =
  { 
    -1,//  FMIT_EXIT,
    EVENT_PROCLINK_CALL_POKELIST,//  FMIT_POKEMON,
    EVENT_PROCLINK_CALL_ZUKAN,//  FMIT_ZUKAN,
    EVENT_PROCLINK_CALL_BAG,//  FMIT_ITEMMENU,
    EVENT_PROCLINK_CALL_TRAINERCARD,//  FMIT_TRAINERCARD,
    EVENT_PROCLINK_CALL_REPORT,//  FMIT_REPORT,
    EVENT_PROCLINK_CALL_CONFIG,//  FMIT_CONFING,
    -1,//  FMIT_NONE,
  };

  GF_ASSERT( type < FMIT_MAX );
  GF_ASSERT( proclink_to_menu_item_tbl[ type ] != -1 );
  return proclink_to_menu_item_tbl[ type ];
}

//=============================================================================
/**
 *    CALLBACK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  プロセスリンクに渡すコールバック  メニューが開く動作のとき実行される
 *
 *  @param  const EVENT_PROCLINK_PARAM *param 引数
 *  @param  *wk_adrs                          自分のワーク
 */
//-----------------------------------------------------------------------------
static BOOL FldMapMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{ 
  FMENU_EVENT_WORK *mwk = wk_adrs;

  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork);

  //!!!　※この関数内でのFIELD_SUBSCREEN_ChangeForce関数の使用禁止!!!

  //以前ここでFIELD_SUBSCREEN_ChangeForceでサブスクリーンモードを変更をしていましたが、
  //そうするとfieldmapのinitで生成したあとにここで再度初期化をおこない無駄なsyncがかかるため、
  //FldMapMenu_InitBefore_Callback内でgamedataのSubscreenModeに設定するようにしました
  //そうすれば、fieldmapの初期化時にgamedataのSubscreenModeを見て1回だけ初期化します　nagihashi

  //フィールドに戻る場合
  if( param->result == EVENT_PROCLINK_RESULT_RETURN )
  { 
    //メニューに戻る場合
    FIELD_SUBSCREEN_SetTopMenuItemNo( subscreen, mwk->item_type );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  プロセスリンクに渡すコールバック  メニューが閉じる動作のとき実行される
 *
 *  @param  const EVENT_PROCLINK_PARAM *param 引数
 *  @param  *wk_adrs                          自分のワーク
 */
//-----------------------------------------------------------------------------
static BOOL FldMapMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{ 
  FMENU_EVENT_WORK *mwk = wk_adrs;
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  プロセスリンクに渡すコールバック  fieldmapの初期化前に実行される
 *
 *  @param  const EVENT_PROCLINK_PARAM *param 引数
 *  @param  *wk_adrs                          自分のワーク
 */
//-----------------------------------------------------------------------------
static BOOL FldMapMenu_InitBefore_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{ 
  FMENU_EVENT_WORK *mwk = wk_adrs;
  GAMEDATA  *gamedata = GAMESYSTEM_GetGameData( mwk->gmSys );

  switch( param->result )
  {
  case EVENT_PROCLINK_RESULT_DOWSINGMACHINE:
    GAMEDATA_SetSubScreenMode( gamedata, FIELD_SUBSCREEN_DOWSING );
    break;

  case EVENT_PROCLINK_RESULT_RETURN:
    GAMEDATA_SetSubScreenMode( gamedata, FIELD_SUBSCREEN_TOPMENU );
    break;

  case EVENT_PROCLINK_RESULT_EXIT:
    /* fallthrough */
  case EVENT_PROCLINK_RESULT_ITEM:
    /* fallthrough */
  case EVENT_PROCLINK_RESULT_SKILL:
    GAMEDATA_SetSubScreenMode( gamedata, mwk->return_subscreen_mode );
  }

  return TRUE;
}
