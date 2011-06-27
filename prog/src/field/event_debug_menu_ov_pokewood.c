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


//=============================================================================
//=============================================================================
#include "event_pokewood.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef enum {
  DEBUG_SHOW_01 = LOADDATA_POKEWOOD0,
  DEBUG_SHOW_02 = LOADDATA_POKEWOOD1,
  DEBUG_SHOW_03 = LOADDATA_POKEWOOD3,
  DEBUG_SHOW_ROM,
}DEBUG_SHOW_TYPE;

typedef enum {
  DEBUG_REC_01 = LOADDATA_POKEWOOD0,
  DEBUG_REC_02 = LOADDATA_POKEWOOD1,
  DEBUG_REC_03 = LOADDATA_POKEWOOD3,

  DEBUG_REC_TYPE_NUM,
}DEBUG_REC_TYPE;

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
static BOOL debugMenuCallProc_PokewoodShowRom( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodAnnounce( POKEWOOD_FUNC_WORK * wk );
static BOOL debugMenuCallProc_PokewoodBook( POKEWOOD_FUNC_WORK * wk );

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
//  { DEBUG_STRID_POKEWOOD_SHOW_ROM, (void*)debugMenuCallProc_PokewoodShowRom, GFL_OVERLAY_BLANK_ID },
//  { DEBUG_STRID_POKEWOOD_SHOW_ROM, (void*)debugMenuCallProc_PokewoodShowRom, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_ANNOUNCE, (void*)debugMenuCallProc_PokewoodAnnounce, GFL_OVERLAY_BLANK_ID },
  { DEBUG_STRID_POKEWOOD_BOOK, (void*)debugMenuCallProc_PokewoodBook, GFL_OVERLAY_BLANK_ID },
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
static BOOL callShowProc( POKEWOOD_FUNC_WORK * wk, DEBUG_SHOW_TYPE type )
{
  LOAD_RESULT result;
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData( wk->gmSys ) );

//  if ( BattleRec_DataOccCheck( sv, HEAPID_FIELDMAP, &result, type ) == FALSE )
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
    int rec_id = type;
    next_event = GMEVENT_CreateOverlayEventCall( gameSys,
        FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)&rec_id );
    GMEVENT_ChangeEvent( event, next_event );

    return TRUE;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL callRecProc( POKEWOOD_FUNC_WORK * wk, DEBUG_REC_TYPE type, int senario_no )
{
  if ( type >= DEBUG_REC_TYPE_NUM )
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
    POKEWOOD_EVENT_PARAM param;
    param.type       = type;
    param.senario_no = senario_no;
    next_event = GMEVENT_CreateOverlayEventCall( gameSys,
        FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, (void*)&param );
    GMEVENT_ChangeEvent( event, next_event );

    return TRUE;
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec1( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, DEBUG_REC_01, 0 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec2( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, DEBUG_REC_02, 1 );
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodRec3( POKEWOOD_FUNC_WORK * wk )
{
  return callRecProc( wk, DEBUG_REC_03, 2 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow1( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_01 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow2( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_02 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShow3( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_03 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_PokewoodShowRom( POKEWOOD_FUNC_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_ROM );
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
	GMEVENT_ChangeEvent( event, new_event );

	OS_Printf( "Pkwd Book Start\n" );
	return TRUE;
}



//============================================================================================
//
//  PKWD COUNT画面関連
//
//============================================================================================
//------------------------------------------------------------------
///  PKWD_COUNT用パラメータOBJECT生成
//------------------------------------------------------------------
PKWD_COUNT_PARAM * PKWD_COUNT_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	PKWD_COUNT_PARAM 	*pkwd_param;
	
	pkwd_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_COUNT_PARAM) );
	// 初期化
	pkwd_param->title_no	  = 0;
	pkwd_param->scenario_no = 0;
	pkwd_param->message		  = 0;
	//
	return pkwd_param;
}

//------------------------------------------------------------------
///  PKWD COUNT用パラメータOBJECT削除
//------------------------------------------------------------------
void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( pkwd_param );
}

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief PKWD COUNT用パラメータのセットアップ（デバッグ用）
 *
 */
//------------------------------------------------------------------
void PKWD_COUNT_PARAM_Setup_Debug( GAMESYS_WORK * gsys, PKWD_COUNT_PARAM * pkwd_param )
{
	// 必要であれば、ここでパラメータのセットをするらしい
	
}
#endif

//============================================================================================
//
//  PKWD BOOK画面関連
//
//============================================================================================
//------------------------------------------------------------------
///  PKWD_BOOK用パラメータOBJECT生成
//------------------------------------------------------------------
PKWD_BOOK_PARAM * PKWD_BOOK_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	PKWD_BOOK_PARAM 	*pkwd_param;
	
	pkwd_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_BOOK_PARAM) );
	// 初期化
//	pkwd_param->title_no	  = 0;
//	pkwd_param->scenario_no = 0;
//	pkwd_param->message		  = 0;
	//
	return pkwd_param;
}

//------------------------------------------------------------------
///  PKWD BOOK用パラメータOBJECT削除
//------------------------------------------------------------------
void PKWD_BOOK_PARAM_Delete( PKWD_BOOK_PARAM * pkwd_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( pkwd_param );
}

