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
#ifdef  PM_DEBUG

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
#include "msg/debug/msg_debug_infinity.h"
#include "pokewood_system.h"


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
#include "battle/pokewood.h"

//=============================================================================
//=============================================================================
#include "event_pokewood.h"

typedef struct _DEBUG_POKEWOOD_WORK DEBUG_POKEWOOD_WORK;

typedef BOOL(*POKEWOOD_FUNC)( DEBUG_POKEWOOD_WORK * );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct _DEBUG_POKEWOOD_WORK
{
  int seq_no;
  HEAPID heapID;
  FLDSYSWIN * win;
  GFL_MSGDATA * msgData;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  POKEWOOD_FUNC call_func;
  FLDMENUFUNC *menuFunc;
  u16         result;
  u16         sub_seq_no;
  u32         num;
  DEBUG_MENU_EVENT_WORK *d_work;
  u16 list_pos, cursor_pos;
  u8  ending; // シナリオフラグ制御のモード設定
};

//typedef DEBUG_MENU_EVENT_WORK POKEWOOD_FUNC_WORK;
typedef DEBUG_POKEWOOD_WORK POKEWOOD_FUNC_WORK;

static GMEVENT_RESULT debugMenuPokewoodEvent( GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_PokewoodRec1( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodRec2( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodRec3( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow1( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow2( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodShow3( POKEWOOD_FUNC_WORK * wk );
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
static BOOL debugMenu_OpenScenario9 (POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario10(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario11(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario12(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario13(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario14(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario15(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario16(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario17(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario18(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario19(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario20(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario21(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario22(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario23(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario24(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario25(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario26(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario27(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario28(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario29(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario30(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario31(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario32(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario33(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario34(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario35(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario36(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario37(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario38(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario39(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario40(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario41(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_OpenScenario_for_Graphic(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series01_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series02_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series03_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series04_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series05_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series06_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series07_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series08_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series09_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series10_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_series11_ready(POKEWOOD_FUNC_WORK *wk);
static BOOL debugMenu_ProfitRecordClear( POKEWOOD_FUNC_WORK *wk );
static BOOL debugMenu_scenario_full_open(POKEWOOD_FUNC_WORK *wk);

static BOOL debugMenu_CutinCheck(POKEWOOD_FUNC_WORK *wk);
static GMEVENT_RESULT debugMenuPokewoodBattleSetupEvent( GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuPokewoodCallBattleSetupEvent( DEBUG_POKEWOOD_WORK *wk );
static GMEVENT_RESULT debugMenuPokewoodScenarioFlagEvent( GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuCallPokewoodScenarioSuccessFlagEvent( DEBUG_POKEWOOD_WORK *wk );
static BOOL debugMenuCallPokewoodScenarioBadFlagEvent( DEBUG_POKEWOOD_WORK *wk );
static BOOL debugMenuCallPokewoodScenarioStrangeFlagEvent( DEBUG_POKEWOOD_WORK *wk );


#define NUMINPUT_NONE (0xFFFF)
static BOOL NumInputState( POKEWOOD_FUNC_WORK *work, u32 default_num, u32 min, u32 max );
static void DrawNum( POKEWOOD_FUNC_WORK *work );

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
//  { DEBUG_STRID_POKEWOOD_REC_02, (void*)debugMenuCallProc_PokewoodRec2, GFL_OVERLAY_BLANK_ID },  //
//  { DEBUG_STRID_POKEWOOD_SHOW_02, (void*)debugMenuCallProc_PokewoodShow2, GFL_OVERLAY_BLANK_ID }, //
//  { DEBUG_STRID_POKEWOOD_REC_01, (void*)debugMenuCallProc_PokewoodRec1, GFL_OVERLAY_BLANK_ID },  // 
//  { DEBUG_STRID_POKEWOOD_SHOW_01, (void*)debugMenuCallProc_PokewoodShow1, GFL_OVERLAY_BLANK_ID }, //
//  { DEBUG_STRID_POKEWOOD_REC_03, (void*)debugMenuCallProc_PokewoodRec3, GFL_OVERLAY_BLANK_ID },  // 
//  { DEBUG_STRID_POKEWOOD_SHOW_03, (void*)debugMenuCallProc_PokewoodShow3, GFL_OVERLAY_BLANK_ID }, //
  { DEBUG_STRID_POKEWOOD_BATTLE_SETUP, (void*)debugMenuPokewoodCallBattleSetupEvent, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_PKWD_LIST_SUCCESS, (void*)debugMenuCallPokewoodScenarioSuccessFlagEvent, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_PKWD_LIST_BAD, (void*)debugMenuCallPokewoodScenarioBadFlagEvent, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_PKWD_LIST_STRANGE, (void*)debugMenuCallPokewoodScenarioStrangeFlagEvent, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ANNOUNCE, (void*)debugMenuCallProc_PokewoodAnnounce, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_BOOK, (void*)debugMenuCallProc_PokewoodBook, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_SELECT_SCREEN, (void*)debugMenuCallProc_PokewoodSelect, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_BRIEFING, (void*)debugMenuCallProc_PokewoodBriefing, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ENDROLL, (void*)debugMenuCallProc_PokewoodEndroll, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_RECORD_MONITOR, (void*)debugMenuCallProc_PokewoodRecordMonitor, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ALL_OPEN, (void*)debugMenu_scenario_full_open, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN02, (void*)debugMenu_OpenScenario2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN03, (void*)debugMenu_OpenScenario3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN04, (void*)debugMenu_OpenScenario4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN05, (void*)debugMenu_OpenScenario5, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN06, (void*)debugMenu_OpenScenario6, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN07, (void*)debugMenu_OpenScenario7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN08, (void*)debugMenu_OpenScenario8, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN09, (void*)debugMenu_OpenScenario9, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN10, (void*)debugMenu_OpenScenario10, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN11, (void*)debugMenu_OpenScenario11, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN12, (void*)debugMenu_OpenScenario12, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN13, (void*)debugMenu_OpenScenario13, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN14, (void*)debugMenu_OpenScenario14, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN15, (void*)debugMenu_OpenScenario15, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN16, (void*)debugMenu_OpenScenario16, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN17, (void*)debugMenu_OpenScenario17, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN18, (void*)debugMenu_OpenScenario18, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN19, (void*)debugMenu_OpenScenario19, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN20, (void*)debugMenu_OpenScenario20, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN21, (void*)debugMenu_OpenScenario21, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN22, (void*)debugMenu_OpenScenario22, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN23, (void*)debugMenu_OpenScenario23, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN24, (void*)debugMenu_OpenScenario24, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN25, (void*)debugMenu_OpenScenario24, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN26, (void*)debugMenu_OpenScenario26, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN27, (void*)debugMenu_OpenScenario27, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN28, (void*)debugMenu_OpenScenario28, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN29, (void*)debugMenu_OpenScenario29, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN30, (void*)debugMenu_OpenScenario30, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN31, (void*)debugMenu_OpenScenario31, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN32, (void*)debugMenu_OpenScenario32, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN33, (void*)debugMenu_OpenScenario33, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN34, (void*)debugMenu_OpenScenario34, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN35, (void*)debugMenu_OpenScenario35, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN36, (void*)debugMenu_OpenScenario36, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN37, (void*)debugMenu_OpenScenario37, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN38, (void*)debugMenu_OpenScenario38, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN39, (void*)debugMenu_OpenScenario39, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN40, (void*)debugMenu_OpenScenario40, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN41, (void*)debugMenu_OpenScenario41, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_PKWD_RECORD_CLEAR, (void*)debugMenu_ProfitRecordClear, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_OPEN_GRAPHIC, (void*)debugMenu_OpenScenario_for_Graphic, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES01,        (void*)debugMenu_series01_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES02,        (void*)debugMenu_series02_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES03,        (void*)debugMenu_series03_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES04,        (void*)debugMenu_series04_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES05,        (void*)debugMenu_series05_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES06,        (void*)debugMenu_series06_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES07,        (void*)debugMenu_series07_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES08,        (void*)debugMenu_series08_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES09,        (void*)debugMenu_series09_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES10,        (void*)debugMenu_series10_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_SERIES11,        (void*)debugMenu_series11_ready, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_CUTIN,           (void*)debugMenu_CutinCheck, GFL_OVERLAY_BLANK_ID },
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

#include "pokeicon/pokeicon.h"
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
      OS_Printf("p senario%02d open=%d, played=%d, success=%d, strange=%d, bad=%d, announced=%d\n", i ,
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_OPEN, i ),     // シナリオオープンフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_PLAYED, i ),   // シナリオ撮影済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_SUCCESS, i ),  // シナリオ成功終了済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_STRANGE, i ),   // シナリオストレンジ終了済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_BAD, i ),   // シナリオストレンジ終了済みフラグ
                POKEWOOD_SAVEDATA_GetPokewoodFlag( ps,PWSV_TYPE_SENARIO_ANNOUNCED, i ) // シナリオオープン告知済みフラグ
                );
    }
    for(i=0;i<5;i++){
      OS_Printf("StarRank%d Announced=%d\n", i, POKEWOOD_SAVEDATA_GetStarRankAnnounced(ps,i));
    }
  }
  
  // ポケアイコンのテスト
  {
    int i;
    u32 mons,form,sex;
    POKEICON_16BIT  data[10];
    data[0] = POKEICON_PackResourceData( 1,0,0 );
    data[1] = POKEICON_PackResourceData( 1,0,1 );
    data[2] = POKEICON_PackResourceData( 493, 5, 2 );
    data[3] = POKEICON_PackResourceData( 493,17, 2 );
    data[4] = POKEICON_PackResourceData( 201, 8, 0 );
    data[5] = POKEICON_PackResourceData( 201,30, 0 );
    data[6] = POKEICON_PackResourceData( 900, 4, 1 );

    for(i=0;i<7;i++){
      POKEICON_UnpackResourceData( data[i], &mons, &form, &sex );
      OS_Printf( "icon mons=%2d, form=%2d, sex=%2d\n", mons, form, sex);
    }
  }
  // 看板の状態を取得
  {
    int i;
    POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(gsys) );
    for(i=0;i<4;i++){
      OS_Printf("kanban%d: poster=%d\n", i ,POKEWOOD_SAVEDATA_GetKanbanNo(ps, i));
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
        work->call_func = (POKEWOOD_FUNC)ret;
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        (*seq)++;
      }
    }
    break;

  case 2:
    if ( work->call_func( work ) == TRUE )
    {
      return GMEVENT_RES_CONTINUE;
    }
    else
    {
      return GMEVENT_RES_FINISH;
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
static BOOL callCutinProc( POKEWOOD_FUNC_WORK * wk, PKWD_REC_POS rec_pos, int scenario_no )
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
        FS_OVERLAY_ID(notwifi_event_pokewood), DEBUG_POKEWOOD_EVENT_CutinCheck, (void*)&rec_param );
    GMEVENT_CallEvent( event, next_event );

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
    
    for(i=0;i<POKEWOOD_PLAY_MAX;i++){
      if(POKEWOOD_SAVEDATA_GetPokewoodOpenFlag(ps,i)){
        count ++;
      }
      if(POKEWOOD_SAVEDATA_GetPokewoodOpenFlag(ps,i)){
      }
    }
    OS_Printf(" pokewood senario open num = %d\n ", 
              POKEWOOD_SAVEDATA_GetFlagTotal( ps, PWSV_TYPE_SENARIO_OPEN ));

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

  pkwd_param->mode = PKWD_SELECT_MODE_DEL;
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
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);
	PKWD_BRIEFING_PARAM  *param = PKWD_BRIEFING_PARAM_Create( HEAPID_PROC, wk->gmSys, 0 );

  param->audienceNum = 10;
  param->scenario_no = 0;
  
	new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BRIEFING_EVENT_Call, (void *)param );

  PKWD_BRIEFING_PARAM_Delete( param );

	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Briefing Start\n" );

	return TRUE;
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	エンドロール画面デバッグ呼び出し
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
  param->scenario_no = 11;

  PMSND_PlayBGM( SEQ_BGM_PW_SCENARIO_01_L );
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
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_ANNOUNCED, no );

}

static void _set_scenario_done( POKEWOOD_FUNC_WORK *wk, int no )
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);
  
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, no ); //シナリオオープン
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_PLAYED, no );
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_THEATERGO, no );
  POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_ANNOUNCED, no );
  
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

static BOOL debugMenu_OpenScenario2 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  1 );  return FALSE;}
static BOOL debugMenu_OpenScenario3 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  2 );  return FALSE;}
static BOOL debugMenu_OpenScenario4 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  3 );  return FALSE;}
static BOOL debugMenu_OpenScenario5 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  4 );  return FALSE;}
static BOOL debugMenu_OpenScenario6 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  5 );  return FALSE;}
static BOOL debugMenu_OpenScenario7 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  6 );  return FALSE;}
static BOOL debugMenu_OpenScenario8 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  7 );  return FALSE;}
static BOOL debugMenu_OpenScenario9 (POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  8 );  return FALSE;}
static BOOL debugMenu_OpenScenario10(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk,  9 );  return FALSE;}
static BOOL debugMenu_OpenScenario11(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 10 );  return FALSE;}
static BOOL debugMenu_OpenScenario12(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 11 );  return FALSE;}
static BOOL debugMenu_OpenScenario13(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 12 );  return FALSE;}
static BOOL debugMenu_OpenScenario14(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 13 );  return FALSE;}
static BOOL debugMenu_OpenScenario15(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 14 );  return FALSE;}
static BOOL debugMenu_OpenScenario16(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 15 );  return FALSE;}
static BOOL debugMenu_OpenScenario17(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 16 );  return FALSE;}
static BOOL debugMenu_OpenScenario18(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 17 );  return FALSE;}
static BOOL debugMenu_OpenScenario19(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 18 );  return FALSE;}
static BOOL debugMenu_OpenScenario20(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 19 );  return FALSE;}
static BOOL debugMenu_OpenScenario21(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 20 );  return FALSE;}
static BOOL debugMenu_OpenScenario22(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 21 );  return FALSE;}
static BOOL debugMenu_OpenScenario23(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 22 );  return FALSE;}
static BOOL debugMenu_OpenScenario24(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 23 );  return FALSE;}
static BOOL debugMenu_OpenScenario25(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 24 );  return FALSE;}
static BOOL debugMenu_OpenScenario26(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 25 );  return FALSE;}
static BOOL debugMenu_OpenScenario27(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 26 );  return FALSE;}
static BOOL debugMenu_OpenScenario28(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 27 );  return FALSE;}
static BOOL debugMenu_OpenScenario29(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 28 );  return FALSE;}
static BOOL debugMenu_OpenScenario30(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 29 );  return FALSE;}
static BOOL debugMenu_OpenScenario31(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 30 );  return FALSE;}
static BOOL debugMenu_OpenScenario32(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 31 );  return FALSE;}
static BOOL debugMenu_OpenScenario33(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 32 );  return FALSE;}
static BOOL debugMenu_OpenScenario34(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 33 );  return FALSE;}
static BOOL debugMenu_OpenScenario35(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 34 );  return FALSE;}
static BOOL debugMenu_OpenScenario36(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 35 );  return FALSE;}
static BOOL debugMenu_OpenScenario37(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 36 );  return FALSE;}
static BOOL debugMenu_OpenScenario38(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 37 );  return FALSE;}
static BOOL debugMenu_OpenScenario39(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 38 );  return FALSE;}
static BOOL debugMenu_OpenScenario40(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 39 );  return FALSE;}
static BOOL debugMenu_OpenScenario41(POKEWOOD_FUNC_WORK *wk){  _open_flag( wk, 40 );  return FALSE;}

static BOOL debugMenu_OpenScenario_for_Graphic(POKEWOOD_FUNC_WORK *wk)
{
  _open_flag( wk, 35 );
  _open_flag( wk, 36 );
  _open_flag( wk, 37 );
  _open_flag( wk, 38 );
  _open_flag( wk, 39 );
  return FALSE;
}

static BOOL debugMenu_scenario_full_open( POKEWOOD_FUNC_WORK *wk )
{
  int i;
  for(i=0;i<40;i++){
    _set_scenario_done( wk, i );
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
// メダル用シリーズコンプ直前状態を作る
//----------------------------------------------------------------------------------
static BOOL debugMenu_series01_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 0 );
  _set_scenario_done( wk, 1 );
  return FALSE;
}

static BOOL debugMenu_series02_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 3 );
  _set_scenario_done( wk, 4 );
  _set_scenario_done( wk, 5 );
  return FALSE;
}
static BOOL debugMenu_series03_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 7 );
  _set_scenario_done( wk, 8 );
  _set_scenario_done( wk, 9 );
  return FALSE;
}

static BOOL debugMenu_series04_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 11 );
  _set_scenario_done( wk, 12 );
  _set_scenario_done( wk, 13 );
  return FALSE;
}

static BOOL debugMenu_series05_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 15 );
  _set_scenario_done( wk, 16 );
  return FALSE;
}

static BOOL debugMenu_series06_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 18 );
  _set_scenario_done( wk, 19 );
  return FALSE;
}

static BOOL debugMenu_series07_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 21 );
  _set_scenario_done( wk, 22 );
  return FALSE;
}

static BOOL debugMenu_series08_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 24 );
  _set_scenario_done( wk, 25 );
  _set_scenario_done( wk, 26 );
  return FALSE;
}

static BOOL debugMenu_series09_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 28 );
  _set_scenario_done( wk, 29 );
  return FALSE;
}

static BOOL debugMenu_series10_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 31 );
  _set_scenario_done( wk, 32 );
  _set_scenario_done( wk, 33 );
  return FALSE;
}

static BOOL debugMenu_series11_ready(POKEWOOD_FUNC_WORK *wk)
{
  _set_scenario_done( wk, 35 );
  _set_scenario_done( wk, 36 );
  _set_scenario_done( wk, 37 );
  return FALSE;
}


static BOOL debugMenu_ProfitRecordClear( POKEWOOD_FUNC_WORK *wk )
{
  int i;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);

  POKEWOOD_BOXOFFICE_RECORD record = {
    -9999,
    {0,0,0,0,0,0,},
    0,0,0,0,
  };
  for(i=0;i<POKEWOOD_PLAY_MAX;i++){
    POKEWOOD_SAVEDATA_SetForceBoxOfficeData( ps, i, &record );
  }
  return FALSE;
}

static BOOL debugMenu_CutinCheck(POKEWOOD_FUNC_WORK *wk)
{
  enum
  {
    SEQ_INIT,
    SEQ_NUM,
    SEQ_CALL,
    SEQ_EXIT,
  };

  switch( wk->seq_no )
  {
  case SEQ_INIT:
    {
      static u16 s_dummy  = 0;
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
      PKWDSYS   ** pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
      GF_ASSERT( *pkwdsys_work == NULL );
      *pkwdsys_work = PKWDSYS_Create( HEAPID_PROC );

      PKWDSYS_SetScriptAnswerWork( *pkwdsys_work, &s_dummy );
    }
    wk->seq_no++;
    break;
  case SEQ_NUM:
    {
      u32 num = NumInputState( wk, 0, 0, POKEWOOD_SENARIO_MAX );
      if( num != NUMINPUT_NONE )
      {
        wk->seq_no++;
      }
    }
    break;
  case SEQ_CALL:
    callCutinProc( wk, PKWD_REC_POS_01, wk->num );
    wk->seq_no++;
    break;
  case SEQ_EXIT:
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);
      PKWDSYS   ** pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
      GF_ASSERT( *pkwdsys_work != NULL );
      PKWDSYS_Delete( *pkwdsys_work );
      *pkwdsys_work = NULL;
    }
    wk->seq_no = 0;
    return FALSE;
  }


  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  数値入力ステート
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL NumInputState( POKEWOOD_FUNC_WORK *work, u32 default_num, u32 min, u32 max )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
  };

  switch( work->sub_seq_no )
  {
  case SEQ_INIT:
    {
      FLDMSGBG * msgBG;
      work->num = default_num;

      work->msgData = DEBUGFLDMENU_CreateMSGDATA(work->fieldWork, NARC_debug_message_debug_infinity_dat );
      msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->win = FLDSYSWIN_AddEx( msgBG, work->msgData , 1, 1, 30, 2 );
      DrawNum( work );
      work->sub_seq_no++;
    }
    break;
  case SEQ_MAIN:
    {
      u32 num = work->num;

      //動作
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT
          || GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
      {
        work->num--;
      }
      if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT
          || GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
      {
        work->num++;
      }

      //最大最小値補正
      if( work->num < min )
      {
        work->num = max;
      }
      else if( work->num > max )
      {
        work->num = min;
      }

      //値が変更したときだけ描画
      if( num != work->num )
      {
        DrawNum( work );
      }
      
      // 終了判定
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        FLDSYSWIN_ClearWindow( work->win );
        FLDSYSWIN_Delete( work->win );
        GFL_MSG_Delete( work->msgData );

        work->sub_seq_no = 0;
        return work->num;
      }
      break;
    }
  }
  return NUMINPUT_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  数値入力
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawNum( POKEWOOD_FUNC_WORK *work )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;

  str = GFL_MSG_CreateString( work->msgData, DEBUG_STR_INFINITY_NUM );
  wset = WORDSET_Create( work->heapID );
  exp  = GFL_STR_CreateBuffer( 128, work->heapID );
  WORDSET_RegisterNumber( wset, 0, work->num, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wset, exp, str );

  msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
  FLDSYSWIN_ClearWindow( work->win );
  FLDSYSWIN_PrintStrBuf( work->win, 0, 0, exp );

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// バトル用デバッグフラグ設定BMPLIST呼び出し
//----------------------------------------------------------------------------------
static BOOL debugMenuPokewoodCallBattleSetupEvent( DEBUG_POKEWOOD_WORK *wk )
{
  GAMESYS_WORK *gsys       = wk->gmSys;
  GMEVENT *event           = wk->gmEvent;
  HEAPID heapID            = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_POKEWOOD_WORK *work;

  GMEVENT_Change( event, debugMenuPokewoodBattleSetupEvent, sizeof(DEBUG_POKEWOOD_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_POKEWOOD_WORK) );

  work->gmSys     = gsys;
  work->gmEvent   = event;
  work->heapID    = heapID;
  work->fieldWork = fieldWork;

  return TRUE;
}


//-----------------------------------------------------------------------------
///リスト メニューヘッダー
//-----------------------------------------------------------------------------
static const FLDMENUFUNC_HEADER DebugPokewoodBattleSetupMenuHeader =
{
  1,    //リスト項目数
  7,    //表示最大項目数
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

#define DEBUG_POKEWOOD_BATTLE_SETUP_NUM  ( 8 )   /// 項目数
static void makePokewoodBattleSetupList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static u16 getPokewoodBattleSetupList( GAMESYS_WORK* gsys, void* cb_work );

//-----------------------------------------------------------------------------
/// デバッグメニュー初期化パラメータ
//-----------------------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuPokewoodBattleSetupInitializer = {
  NARC_debug_message_debug_pokewood_dat,
  0,
  NULL,
  &DebugPokewoodBattleSetupMenuHeader,
  1, 1, 14, 14,
  makePokewoodBattleSetupList,
  getPokewoodBattleSetupList,
};

//----------------------------------------------------------------------------------
/**
 * @brief	バトル用デバッグ設定を行うBMPLISTイベント
 *
 * @param   event	
 * @param   seq		
 * @param   wk		
 *
 * @retval  static		
 */
//----------------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuPokewoodBattleSetupEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_POKEWOOD_WORK *work = wk;
  u32 ret;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID, 
                                        &DebugMenuPokewoodBattleSetupInitializer,
                                        work->list_pos, work->cursor_pos );
    (*seq)++;
    break;

  case 1:

    ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
    switch( ret ){
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;
    default:
      // Aボタンを押すと項目反転
      {
        int flag = POKEWOOD_SAVEDATA_GetBattleDebugFlag(ret);
        POKEWOOD_SAVEDATA_SetBattleDebugFlag(ret, flag^1);
      }
      (*seq)++;
    }
    break;

  case 2:
    // メニュー再描画
    FLDMENUFUNC_GetListCursor( work->menuFunc, &work->list_pos, &work->cursor_pos );
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    (*seq) = 0;
    break;
  }

  return GMEVENT_RES_CONTINUE;
  
}


//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト生成
 *
 *	@param	GAMESYS_WORK * gsys
 *	@param	*list
 *	@param	heapID
 *	@param	msgData
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void makePokewoodBattleSetupList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int i;
  STRBUF *strBuf1 = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG          = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgData    = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_debug_pokewood_dat );
  
  // 「シナリオ？？：○」or「シナリオ？？：×」を追加
  for( i = 0; i < DEBUG_POKEWOOD_BATTLE_SETUP_NUM; i++ ){
    GFL_STR_ClearBuffer( strBuf1 );

    // デバッグ項目反映の場合は赤文字、反映していない時は白文字
    if(POKEWOOD_SAVEDATA_GetBattleDebugFlag(i)){
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_BATTLE_SETUP_PARAM_1_0+i, strBuf1 );
    }else{
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_BATTLE_SETUP_PARAM_0_0+i, strBuf1 );
    }
    FLDMENUFUNC_AddStringListData( list, strBuf1, i, heapID );
  }

  GFL_MSG_Delete( pMsgData );
  GFL_HEAP_FreeMemory( strBuf1 );

}

//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト項目数を返す
 *
 *	@param	GAMESYS_WORK* gsys
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 getPokewoodBattleSetupList( GAMESYS_WORK* gsys, void* cb_work )
{
  return DEBUG_POKEWOOD_BATTLE_SETUP_NUM;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// ポケウッドシナリオ達成状況変更用BMPLIST呼び出し
//----------------------------------------------------------------------------------
static DEBUG_POKEWOOD_WORK *_init_debugScenarioFlagMenu( DEBUG_POKEWOOD_WORK *wk );


//----------------------------------------------------------------------------------
/**
 * @brief	シナリオ成功フラグ制御BMPLIST呼び出し
 */
//----------------------------------------------------------------------------------
static BOOL debugMenuCallPokewoodScenarioSuccessFlagEvent( DEBUG_POKEWOOD_WORK *wk )
{
  DEBUG_POKEWOOD_WORK *work = _init_debugScenarioFlagMenu( wk );

  work->ending    = 0;    // 成功ENDリスト開始

  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	シナリオ失敗フラグ制御BMPLIST呼び出し
 */
//----------------------------------------------------------------------------------
static BOOL debugMenuCallPokewoodScenarioBadFlagEvent( DEBUG_POKEWOOD_WORK *wk )
{
  DEBUG_POKEWOOD_WORK *work = _init_debugScenarioFlagMenu( wk );

  work->ending    = 1;    // 成功ENDリスト開始

  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	シナリオストレンジフラグ制御BMPLIST呼び出し
 */
//----------------------------------------------------------------------------------
static BOOL debugMenuCallPokewoodScenarioStrangeFlagEvent( DEBUG_POKEWOOD_WORK *wk )
{
  DEBUG_POKEWOOD_WORK *work = _init_debugScenarioFlagMenu( wk );

  work->ending    = 2;    // 成功ENDリスト開始

  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief	シナリオフラグ制御BMPLIST呼び出しサブ
 *
 * @param   wk		
 *
 * @retval  DEBUG_POKEWOOD_WORK *		
 */
//----------------------------------------------------------------------------------
static DEBUG_POKEWOOD_WORK *_init_debugScenarioFlagMenu( DEBUG_POKEWOOD_WORK *wk )
{
  GAMESYS_WORK *gsys       = wk->gmSys;
  GMEVENT *event           = wk->gmEvent;
  HEAPID heapID            = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_POKEWOOD_WORK *work;

  GMEVENT_Change( event, debugMenuPokewoodScenarioFlagEvent, sizeof(DEBUG_POKEWOOD_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_POKEWOOD_WORK) );

  work->gmSys     = gsys;
  work->gmEvent   = event;
  work->heapID    = heapID;
  work->fieldWork = fieldWork;

  return work;
}

//-----------------------------------------------------------------------------
///リスト メニューヘッダー
//-----------------------------------------------------------------------------
static const FLDMENUFUNC_HEADER DebugPokewoodScenarioFlagMenuHeader =
{
  1,    //リスト項目数
  10,   //表示最大項目数
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

// リスト起動モードで集計するフラグ指定を変えるためのテーブル
static const ending_tbl[]={
  PWSV_TYPE_SENARIO_SUCCESS,    ///< シナリオ成功終了済みフラグ
  PWSV_TYPE_SENARIO_BAD,        ///< バッドEND取得済みフラグ  
  PWSV_TYPE_SENARIO_STRANGE,    ///< ストレンジEND取得済みフラグ
};

//-----------------------------------------------------------------------------
///メニューリスト
//-----------------------------------------------------------------------------

#define DEBUG_POKEWOOD_SCENARIO_FLAG_NUM  ( 40 )   /// 項目数
static void makePokewoodScenarioSuccessFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static void makePokewoodScenarioBadFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static void makePokewoodScenarioStrangeFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static u16 getPokewoodScenarioFlagListNum( GAMESYS_WORK* gsys, void* cb_work );

//-----------------------------------------------------------------------------
/// デバッグメニュー初期化パラメータ
//-----------------------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuPokewoodScenarioSuccessFlagInitializer = {
  NARC_debug_message_debug_pokewood_dat,
  0,
  NULL,
  &DebugPokewoodScenarioFlagMenuHeader,
  1, 1, 14, 17,
  makePokewoodScenarioSuccessFlagList,
  getPokewoodScenarioFlagListNum,
};
static const DEBUG_MENU_INITIALIZER DebugMenuPokewoodScenarioBadFlagInitializer = {
  NARC_debug_message_debug_pokewood_dat,
  0,
  NULL,
  &DebugPokewoodScenarioFlagMenuHeader,
  1, 1, 14, 17,
  makePokewoodScenarioBadFlagList,
  getPokewoodScenarioFlagListNum,
};
static const DEBUG_MENU_INITIALIZER DebugMenuPokewoodScenarioStrangeFlagInitializer = {
  NARC_debug_message_debug_pokewood_dat,
  0,
  NULL,
  &DebugPokewoodScenarioFlagMenuHeader,
  1, 1, 14, 17,
  makePokewoodScenarioStrangeFlagList,
  getPokewoodScenarioFlagListNum,
};

static const DEBUG_MENU_INITIALIZER *DebugMenuPokewoodScenarioFlagInitializer[]={
  &DebugMenuPokewoodScenarioSuccessFlagInitializer,
  &DebugMenuPokewoodScenarioBadFlagInitializer,
  &DebugMenuPokewoodScenarioStrangeFlagInitializer,
};

//----------------------------------------------------------------------------------
/**
 * @brief	バトル用デバッグ設定を行うBMPLISTイベント
 *
 * @param   event	
 * @param   seq		
 * @param   wk		
 *
 * @retval  static		
 */
//----------------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuPokewoodScenarioFlagEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_POKEWOOD_WORK *work = wk;
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(work->gmSys) );
  u32 ret;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID, 
                                        DebugMenuPokewoodScenarioFlagInitializer[work->ending],
                                        work->list_pos, work->cursor_pos );
    (*seq)++;
    break;

  case 1:

    ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
    switch( ret ){
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;
    case 40:  //「全て反転」
      {
        int i;
        for(i=0;i<40;i++){
          int flag = POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, ending_tbl[work->ending], i );
          POKEWOOD_SAVEDATA_ChangePokewoodFlag( ps, ending_tbl[work->ending], i, flag^1 );
        }
      }
      break;
    default:
      // Aボタンを押すと項目反転
      {
        int flag = POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, ending_tbl[work->ending], ret );
        OS_Printf("ret:%d, flag:%d\n", ret,flag);
        POKEWOOD_SAVEDATA_ChangePokewoodFlag( ps, ending_tbl[work->ending], ret, flag^1 );
      }
      (*seq)++;
    }
    break;

  case 2:
    // メニュー再描画
    FLDMENUFUNC_GetListCursor( work->menuFunc, &work->list_pos, &work->cursor_pos );
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    (*seq) = 0;
    break;
  }

  return GMEVENT_RES_CONTINUE;
  
}


//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト生成
 *
 *	@param	GAMESYS_WORK * gsys
 *	@param	*list
 *	@param	heapID
 *	@param	msgData
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void makePokewoodScenarioSuccessFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int i;
  STRBUF *strBuf1   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *strBuf2   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *expandBuf = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG          = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgData    = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_debug_pokewood_dat );
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(gsys) );
  WORDSET * wset;

  wset = WORDSET_Create( heapID );

   GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_SCENARIO01, strBuf1 );

  for( i = 0; i < DEBUG_POKEWOOD_SCENARIO_FLAG_NUM; i++ ){
    // デバッグ項目反映の場合は赤文字、反映していない時は白文字
    if(POKEWOOD_SAVEDATA_GetPokewoodFlag(ps, PWSV_TYPE_SENARIO_SUCCESS, i)){
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_MARU, strBuf2 );
    }else{
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_BATU, strBuf2 );
    }
    WORDSET_RegisterNumber( wset, 0, i+1, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU );
    WORDSET_RegisterWord( wset, 1, strBuf2, 0, 0, 0 );
    WORDSET_ExpandStr( wset, expandBuf, strBuf1 );
    FLDMENUFUNC_AddStringListData( list, expandBuf, i, heapID );
  }

  // 「全て反転」を追加
  GFL_MSG_GetString( pMsgData,DEBUG_STRID_PKWD_FLAG_REVERSE, strBuf1 );
  FLDMENUFUNC_AddStringListData( list, strBuf1, 40, heapID );

  WORDSET_Delete( wset );
  
  GFL_MSG_Delete( pMsgData );
  GFL_HEAP_FreeMemory( expandBuf );
  GFL_HEAP_FreeMemory( strBuf2   );
  GFL_HEAP_FreeMemory( strBuf1   );

}
//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト生成
 *
 *	@param	GAMESYS_WORK * gsys
 *	@param	*list
 *	@param	heapID
 *	@param	msgData
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void makePokewoodScenarioBadFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int i;
  STRBUF *strBuf1   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *strBuf2   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *expandBuf = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG          = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgData    = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_debug_pokewood_dat );
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(gsys) );
  WORDSET * wset;

  wset = WORDSET_Create( heapID );

   GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_SCENARIO01, strBuf1 );

  for( i = 0; i < DEBUG_POKEWOOD_SCENARIO_FLAG_NUM; i++ ){
    // デバッグ項目反映の場合は赤文字、反映していない時は白文字
    if(POKEWOOD_SAVEDATA_GetPokewoodFlag(ps, PWSV_TYPE_SENARIO_BAD, i)){
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_MARU, strBuf2 );
    }else{
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_BATU, strBuf2 );
    }
    WORDSET_RegisterNumber( wset, 0, i, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU );
    WORDSET_RegisterWord( wset, 1, strBuf2, 0, 0, 0 );
    WORDSET_ExpandStr( wset, expandBuf, strBuf1 );
    FLDMENUFUNC_AddStringListData( list, expandBuf, i, heapID );
  }

  // 「全て反転」を追加
  GFL_MSG_GetString( pMsgData,DEBUG_STRID_PKWD_FLAG_REVERSE, strBuf1 );
  FLDMENUFUNC_AddStringListData( list, strBuf1, 40, heapID );

  WORDSET_Delete( wset );
  
  GFL_MSG_Delete( pMsgData );
  GFL_HEAP_FreeMemory( expandBuf );
  GFL_HEAP_FreeMemory( strBuf2   );
  GFL_HEAP_FreeMemory( strBuf1   );

}
//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト生成
 *
 *	@param	GAMESYS_WORK * gsys
 *	@param	*list
 *	@param	heapID
 *	@param	msgData
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void makePokewoodScenarioStrangeFlagList( GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int i;
  STRBUF *strBuf1   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *strBuf2   = GFL_STR_CreateBuffer( 64, heapID );
  STRBUF *expandBuf = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG          = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgData    = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_debug_pokewood_dat );
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata( GAMESYSTEM_GetGameData(gsys) );
  WORDSET * wset;

  wset = WORDSET_Create( heapID );

   GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_SCENARIO01, strBuf1 );

  for( i = 0; i < DEBUG_POKEWOOD_SCENARIO_FLAG_NUM; i++ ){
    // デバッグ項目反映の場合は赤文字、反映していない時は白文字
    if(POKEWOOD_SAVEDATA_GetPokewoodFlag(ps, PWSV_TYPE_SENARIO_STRANGE, i)){
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_MARU, strBuf2 );
    }else{
      GFL_MSG_GetString( pMsgData, DEBUG_STRID_PKWD_FLAG_BATU, strBuf2 );
    }
    WORDSET_RegisterNumber( wset, 0, i, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU );
    WORDSET_RegisterWord( wset, 1, strBuf2, 0, 0, 0 );
    WORDSET_ExpandStr( wset, expandBuf, strBuf1 );
    FLDMENUFUNC_AddStringListData( list, expandBuf, i, heapID );
  }

  // 「全て反転」を追加
  GFL_MSG_GetString( pMsgData,DEBUG_STRID_PKWD_FLAG_REVERSE, strBuf1 );
  FLDMENUFUNC_AddStringListData( list, strBuf1, 40, heapID );

  WORDSET_Delete( wset );
  
  GFL_MSG_Delete( pMsgData );
  GFL_HEAP_FreeMemory( expandBuf );
  GFL_HEAP_FreeMemory( strBuf2   );
  GFL_HEAP_FreeMemory( strBuf1   );

}

//-----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化 リスト項目数を返す
 *
 *	@param	GAMESYS_WORK* gsys
 *	@param	cb_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 getPokewoodScenarioFlagListNum( GAMESYS_WORK* gsys, void* cb_work )
{
  return DEBUG_POKEWOOD_SCENARIO_FLAG_NUM+1;
}


#endif  //  PM_DEBUG

