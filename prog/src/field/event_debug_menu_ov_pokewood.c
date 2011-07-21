//=============================================================================
/**
 *
 *	@file		src\field\event_debug_menu_ov_pokewood.c
 *	@brief  デバッグメニューポケウッド系オーバーレイ領域
 *	@author	tamada GAMEFREAK inc.
 *	@date		2011.04.28
 *
 */
//=============================================================================
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
//#include "system/bmp_winframe.h"
#include "savedata/misc.h"
#include "font/font.naix"
#include "sound/pm_sndsys.h"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "event_mapchange.h"

//#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_debug_pokewood.h"


//アプリ
#include "savedata/battle_rec.h"
#include "savedata/pokewood_rec.h"
#include "app/pkwd_count.h"
#include "app/pkwd_book.h"

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 

#include "event_debug_local.h"
#include "event_debug_menu_ov_pokewood.h"

#include "pokewood_setup.h"

//=============================================================================
//=============================================================================
#include "event_pokewood.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
}DEBUG_POKEWOOD_WORK;

//typedef DEBUG_MENU_EVENT_WORK POKEWOOD_FUNC_WORK;
typedef DEBUG_POKEWOOD_WORK POKEWOOD_FUNC_WORK;
typedef BOOL(*POKEWOOD_FUNC)( POKEWOOD_FUNC_WORK * );

static GMEVENT_RESULT debugMenuPokewoodEvent( GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_PokewoodRec1( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodRec2( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodRec3( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow1( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow2( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow3( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow4( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodAnnounce( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodBook( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodSelect( POKEWOOD_FUNC_WORK * wk );

//-----------------------------------------------------------------------------
///リスト メニューヘッダー
//-----------------------------------------------------------------------------
static const FLDMENUFUNC_HEADER DebugPokewoodMenuHeader =
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

//-----------------------------------------------------------------------------
///メニューリスト
//-----------------------------------------------------------------------------
static const DEBUG_MENU_LIST DebugPokewoodMenuList[] =
{
  { DEBUG_STRID_POKEWOOD_REC_02, (void*)debugMenuCallProc_PokewoodRec2, GFL_OVERLAY_BLANK_ID },  //
  { DEBUG_STRID_POKEWOOD_SHOW_02, (void*)debugMenuCallProc_PokewoodShow2, GFL_OVERLAY_BLANK_ID }, //
  { DEBUG_STRID_POKEWOOD_REC_01, (void*)debugMenuCallProc_PokewoodRec1, GFL_OVERLAY_BLANK_ID },  // 
  { DEBUG_STRID_POKEWOOD_SHOW_01, (void*)debugMenuCallProc_PokewoodShow1, GFL_OVERLAY_BLANK_ID }, //
  { DEBUG_STRID_POKEWOOD_REC_03, (void*)debugMenuCallProc_PokewoodRec3, GFL_OVERLAY_BLANK_ID },  // 
  { DEBUG_STRID_POKEWOOD_SHOW_03, (void*)debugMenuCallProc_PokewoodShow3, GFL_OVERLAY_BLANK_ID }, //
//  { DEBUG_STRID_POKEWOOD_SHOW_ROM, (void*)debugMenuCallProc_PokewoodShow4, GFL_OVERLAY_BLANK_ID },
//  { DEBUG_STRID_POKEWOOD_SHOW_ROM, (void*)debugMenuCallProc_PokewoodShow4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ANNOUNCE, (void*)debugMenuCallProc_PokewoodAnnounce, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_BOOK, (void*)debugMenuCallProc_PokewoodBook, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_SELECT_SCREEN, (void*)debugMenuCallProc_PokewoodSelect, GFL_OVERLAY_BLANK_ID },
};

//-----------------------------------------------------------------------------
/// デバッグメニュー初期化パラメータ
//-----------------------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuPokewoodInitializer = {
  NARC_debug_message_debug_pokewood_dat,
  NELEMS(DebugPokewoodMenuList),
  DebugPokewoodMenuList,
  &DebugPokewoodMenuHeader,
  1, 1, 16, 16,
  NULL,
  NULL
};

//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_Pokewood( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_POKEWOOD_WORK *work;

  u16 zone_id = FIELDMAP_GetZoneID( fieldWork );
  if (
      ( zone_id != ZONE_ID_C15 ) && ( zone_id != ZONE_ID_C15R0101 ) && ( zone_id != ZONE_ID_C15R0201 )
     )
  {
    //オーバーレイがないものを呼び出さないように使用箇所を制限
    OS_Printf("デバッグメニューPOKEWOODはC15以外では呼べません\n");
    return FALSE;
  }

  GMEVENT_Change( event, debugMenuPokewoodEvent, sizeof(DEBUG_POKEWOOD_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_POKEWOOD_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * イベント：ポケウッド用デバッグメニュー
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuPokewoodEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_POKEWOOD_WORK *work = wk;
  u32 ret;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuPokewoodInitializer );
    (*seq)++;
    break;

  case 1:

    ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
    switch( ret )
    {
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    default:
      {
        BOOL result;
        POKEWOOD_FUNC call_func = (POKEWOOD_FUNC)ret;
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        if ( call_func( work ) == TRUE )
        {
          return GMEVENT_RES_CONTINUE;
        }
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}



//=============================================================================
//=============================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL callShowProc( POKEWOOD_FUNC_WORK * wk, PKWD_REC_POS rec_pos )
{
  LOAD_RESULT result;
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData( wk->gmSys ) );

//  if ( BattleRec_DataOccCheck( sv, HEAPID_FIELDMAP, &result, rec_pos ) == FALSE )
//  { //対応する録画データが存在しないときにはなにもせずに戻る
//    PMSND_PlaySE( SEQ_SE_BEEP );
//    return FALSE;
//  }
//  else
  {
    GMEVENT *event = wk->gmEvent;
    FIELDMAP_WORK *fieldWork = wk->fieldWork;
    GAMESYS_WORK  *gameSys  = wk->gmSys;
    GMEVENT *next_event;
    int rec_id = rec_pos;
    next_event = GMEVENT_CreateOverlayEventCall( gameSys,
        FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)&rec_id );
    GMEVENT_ChangeEvent( event, next_event );

    return TRUE;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL callRecProc( POKEWOOD_FUNC_WORK * wk, PKWD_REC_POS rec_pos, int scenario_no )
{
  if ( rec_pos >= PKWD_REC_POS_MAX )
  { //対応するシナリオデータが存在しないときにはなにもせずに戻る
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }
  else
  {
    GMEVENT *event = wk->gmEvent;
    FIELDMAP_WORK *fieldWork = wk->fieldWork;
    GAMESYS_WORK  *gameSys  = wk->gmSys;
    GMEVENT *next_event;
    PKWD_REC_EVENT_PARAM rec_param;
    rec_param.rec_pos       = rec_pos;
    rec_param.scenario_no = scenario_no;
    next_event = GMEVENT_CreateOverlayEventCall( gameSys,
        FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, (void*)&rec_param );
    GMEVENT_ChangeEvent( event, next_event );

    return TRUE;
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec1( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, PKWD_REC_POS_00, 0 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec2( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, PKWD_REC_POS_01, 1 );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec3( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, PKWD_REC_POS_02, 2 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow1( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, PKWD_REC_POS_00 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow2( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, PKWD_REC_POS_01 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow3( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, PKWD_REC_POS_02 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow4( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, PKWD_REC_POS_03 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用ポケウッド上映前アプリ呼び出し
 *
 *  @param  POKEWOOD_FUNC_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodAnnounce( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_COUNT_PARAM 	*pkwd_param = PKWD_COUNT_PARAM_Create( HEAPID_PROC, wk->gmSys, NULL );

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_COUNT_EVENT_Show, (void *)pkwd_param );
	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Count Start\n" );
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用ポケウッド台本アプリ呼び出し
 *
 *  @param  POKEWOOD_FUNC_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodBook( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_BOOK_PARAM 	*pkwd_param = PKWD_BOOK_PARAM_Create( HEAPID_PROC, wk->gmSys, NULL );

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BOOK_EVENT_Show, (void *)pkwd_param );

  PKWD_BOOK_PARAM_Delete( pkwd_param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Book Start\n" );

  {
    int i,count=0,newcount=0;
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
    MISC *misc = GAMEDATA_GetMiscWork(gamedata);
    
    for(i=0;i<POKEWOOD_SENARIO_MAX;i++){
      if(MISC_GetPokewoodOpenFlag(misc,i)){
        count ++;
      }
      if(MISC_GetPokewoodOpenFlag(misc,i)){
      }
    }
    OS_Printf(" pokewood senario open num = %d\n ", count);
/*
    MISC_SetPokewoodOpenFlag(misc,4);
    MISC_SetPokewoodOpenFlag(misc,5);
    MISC_SetPokewoodOpenFlag(misc,6);
    MISC_SetPokewoodOpenFlag(misc,7);
    MISC_SetPokewoodOpenFlag(misc,8);
    MISC_SetPokewoodOpenFlag(misc,9);
    MISC_SetPokewoodOpenFlag(misc,11);
    MISC_SetPokewoodOpenFlag(misc,12);
    MISC_SetPokewoodOpenFlag(misc,13);
    MISC_SetPokewoodOpenFlag(misc,14);
    MISC_SetPokewoodOpenFlag(misc,15);
    MISC_SetPokewoodOpenFlag(misc,16);
    MISC_SetPokewoodOpenFlag(misc,11);
    MISC_SetPokewoodOpenFlag(misc,17);
    MISC_SetPokewoodOpenFlag(misc,18);
    MISC_SetPokewoodOpenFlag(misc,19);
    MISC_SetPokewoodOpenFlag(misc,20);
    MISC_SetPokewoodOpenFlag(misc,31);
    MISC_SetPokewoodOpenFlag(misc,32);
    MISC_SetPokewoodOpenFlag(misc,33);
    MISC_SetPokewoodOpenFlag(misc,34);
    MISC_SetPokewoodOpenFlag(misc,35);
    MISC_SetPokewoodOpenFlag(misc,40);
*/
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      newcount = MISC_GetPokewoodPlayedFlag(misc,50);
    }
  }
  
	return TRUE;
}


//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用ポケウッド上映選択アプリ呼び出し
 *
 *  @param  POKEWOOD_FUNC_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodSelect( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_SELECT_SCREEN_PARAM  *pkwd_param = PKWD_SELECT_SCREEN_PARAM_Create( HEAPID_PROC, wk->gmSys, NULL );

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_SELECT_SCREEN_EVENT_Call, (void *)pkwd_param );

  PKWD_SELECT_SCREEN_PARAM_Delete( pkwd_param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Select Screen Start\n" );
  
	return TRUE;
}


