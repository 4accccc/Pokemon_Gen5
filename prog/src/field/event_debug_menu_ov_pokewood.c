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
#include "savedata/pokewood_savedata.h"
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
#include "app/pkwd_record_monitor.h"

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
  u16         result;
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
static BOOL debugMenuCallProc_PokewoodBriefing( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodEndroll( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodRecordMonitor( POKEWOOD_FUNC_WORK * wk );

static BOOL debugMenu_OpenScenario2 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario3 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario4 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario5 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario6 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario7 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario8 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario10(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario11(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario12(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario13(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario14(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario15(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario16(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario18(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario19(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario20(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario22(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario23(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario24(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario26(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario27(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario28(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario29(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario30(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario31(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario32(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario34(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario35(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario36(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario37(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario38(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario39(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario40(POKEWOOD_FUNC_WORK *wk);


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
  { DEBUG_STRID_POKEWOOD_BRIEFING, (void*)debugMenuCallProc_PokewoodBriefing, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ENDROLL, (void*)debugMenuCallProc_PokewoodEndroll, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_RECORD_MONITOR, (void*)debugMenuCallProc_PokewoodRecordMonitor, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN02, (void*)debugMenu_OpenScenario2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN03, (void*)debugMenu_OpenScenario3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN04, (void*)debugMenu_OpenScenario4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN05, (void*)debugMenu_OpenScenario5, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN06, (void*)debugMenu_OpenScenario6, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN07, (void*)debugMenu_OpenScenario7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN08, (void*)debugMenu_OpenScenario8, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN10, (void*)debugMenu_OpenScenario10, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN11, (void*)debugMenu_OpenScenario11, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN12, (void*)debugMenu_OpenScenario12, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN13, (void*)debugMenu_OpenScenario13, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN14, (void*)debugMenu_OpenScenario14, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN15, (void*)debugMenu_OpenScenario15, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN16, (void*)debugMenu_OpenScenario16, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN18, (void*)debugMenu_OpenScenario18, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN19, (void*)debugMenu_OpenScenario19, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN20, (void*)debugMenu_OpenScenario20, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN22, (void*)debugMenu_OpenScenario22, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN23, (void*)debugMenu_OpenScenario23, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN24, (void*)debugMenu_OpenScenario24, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN26, (void*)debugMenu_OpenScenario26, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN27, (void*)debugMenu_OpenScenario27, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN28, (void*)debugMenu_OpenScenario28, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN29, (void*)debugMenu_OpenScenario29, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN30, (void*)debugMenu_OpenScenario30, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN31, (void*)debugMenu_OpenScenario31, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN32, (void*)debugMenu_OpenScenario32, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN34, (void*)debugMenu_OpenScenario34, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN35, (void*)debugMenu_OpenScenario35, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN36, (void*)debugMenu_OpenScenario36, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN37, (void*)debugMenu_OpenScenario37, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN38, (void*)debugMenu_OpenScenario38, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN39, (void*)debugMenu_OpenScenario39, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN40, (void*)debugMenu_OpenScenario40, GFL_OVERLAY_BLANK_ID },
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
      ( zone_id != ZONE_ID_C15 ) && ( zone_id != ZONE_ID_C15R0101 )
      && ( zone_id != ZONE_ID_C15R0201 ) && (zone_id != ZONE_ID_C15R0301 )
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

  // ポケウッド状況のデバッグプリント
  {
    int i;
    POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(gsys) );
    for(i=0;i<40;i++){
      OS_Printf("p senario%02d open=%d, played=%d, success=%d, announced=%d\n", i ,
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_OPEN, i ),     // シナリオオープンフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_PLAYED, i ),   // シナリオ撮影済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_SUCCESS, i ),  // シナリオ成功終了済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_ANNOUNCED, i ) // シナリオオープン告知済みフラグ
                );
    }
    for(i=0;i<5;i++){
      OS_Printf("StarRank%d Announced=%d\n", i, POKEWOOD_SAVEDATA_GetStarRankAnnounced(ps,i));
    }
  }
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
    POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);
    
    for(i=0;i<POKEWOOD_SENARIO_MAX;i++){
      if(POKEWOOD_SAVEDATA_GetPokewoodOpenFlag(ps,i)){
        count ++;
      }
      if(POKEWOOD_SAVEDATA_GetPokewoodOpenFlag(ps,i)){
      }
    }
    OS_Printf(" pokewood senario open num = %d\n ", POKEWOOD_SAVEDATA_GetSenarioOpenNum(ps));
/*
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,4);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,5);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,6);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,7);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,8);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,9);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,11);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,12);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,13);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,14);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,15);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,16);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,11);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,17);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,18);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,19);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,20);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,31);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,32);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,33);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,34);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,35);
    POKEWOOD_SAVEDATA_SetPokewoodOpenFlag(ps,40);
*/
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      newcount = POKEWOOD_SAVEDATA_GetPokewoodPlayedFlag(ps,50);
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
	PKWD_SELECT_SCREEN_PARAM  *pkwd_param;
  
  pkwd_param = PKWD_SELECT_SCREEN_PARAM_Create( HEAPID_PROC, wk->gmSys, PKWD_SELECT_MODE_NORMAL, &wk->result );

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_SELECT_SCREEN_EVENT_Call, (void *)pkwd_param );

  PKWD_SELECT_SCREEN_PARAM_Delete( pkwd_param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Select Screen Start\n" );
  
	return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	あらすじ画面デバッグ呼び出し
 *
 * @param   wk		
 *
 * @retval  BOOL		
 */
//----------------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodBriefing( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_BRIEFING_PARAM  *param = PKWD_BRIEFING_PARAM_Create( HEAPID_PROC, wk->gmSys, 0 );

  param->audienceNum = 10;

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BRIEFING_EVENT_Call, (void *)param );

  PKWD_BRIEFING_PARAM_Delete( param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Briefing Start\n" );
  
	return TRUE;
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	あらすじ画面デバッグ呼び出し
 *
 * @param   wk		
 *
 * @retval  BOOL		
 */
//----------------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodEndroll( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_BRIEFING_PARAM  *param = PKWD_BRIEFING_PARAM_Create( HEAPID_PROC, wk->gmSys, 0 );

  param->audienceNum = 10;
  param->scenario_no = 1;

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_ENDROLL_EVENT_Call, (void *)param );

  PKWD_BRIEFING_PARAM_Delete( param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Briefing Start\n" );
  
	return TRUE;
  
}

static void _open_flag( POKEWOOD_FUNC_WORK *wk, int no )
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);
  
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, no );
}

//----------------------------------------------------------------------------
/**
 *	@brief  記録モニターアプリ呼び出し
 *
 *	@param	POKEWOOD_FUNC_WORK * wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRecordMonitor( POKEWOOD_FUNC_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
	GMEVENT *new_event;
	PKWD_RECORD_MONITOR_PARAM *param = PKWD_RECORD_MONITOR_PARAM_Create( HEAPID_PROC, wk->gmSys );

	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_RECORD_MONITOR_EVENT_Call, (void *)param );

  PKWD_RECORD_MONITOR_PARAM_Delete( param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd RecordMonitro Start\n" );
  
	return TRUE;
  
}

//----------------------------------------------------------------------------------
// シナリオオープンデバッグ用の関数
//----------------------------------------------------------------------------------

static BOOL debugMenu_OpenScenario2 ( POKEWOOD_FUNC_WORK *wk )
{
  _open_flag( wk, 2 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario3 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 3 );
  return FALSE;
}

static BOOL debugMenu_OpenScenario4 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 4 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario5 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 5 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario6 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 6 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario7 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 7 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario8 (POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 8 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario10(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 10 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario11(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 11 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario12(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 12 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario13(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 13 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario14(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 14 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario15(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 15 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario16(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 16 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario18(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 18 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario19(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 19 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario20(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 20 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario22(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 22 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario23(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 23 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario24(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 24 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario26(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 26 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario27(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 27 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario28(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 28 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario29(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 29 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario30(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 30 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario31(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 31 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario32(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 32 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario34(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 34 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario35(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 35 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario36(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 36 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario37(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 37 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario38(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 38 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario39(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 39 );
  return FALSE;
}
static BOOL debugMenu_OpenScenario40(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 40 );
  return FALSE;
}
