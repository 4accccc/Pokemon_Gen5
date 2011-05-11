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
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"

//アプリ
#include "savedata/battle_rec.h"

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
  DEBUG_SHOW_01 = 0,
  DEBUG_SHOW_02 = 1,
  DEBUG_SHOW_03 = 2,
  DEBUG_SHOW_ROM,
}DEBUG_SHOW_TYPE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL callShowProc( DEBUG_MENU_EVENT_WORK * wk, DEBUG_SHOW_TYPE type )
{
  LOAD_RESULT result;
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData( wk->gmSys ) );

  if ( BattleRec_DataOccCheck( sv, HEAPID_FIELDMAP, &result, type ) == FALSE )
  { //対応する録画データが存在しないときにはなにもせずに戻る
    PMSND_PlaySE( SEQ_SE_BEEP );
    return FALSE;
  }
  else
  {
    GMEVENT *event = wk->gmEvent;
    FIELDMAP_WORK *fieldWork = wk->fieldWork;
    GAMESYS_WORK  *gameSys  = wk->gmSys;
    GMEVENT *next_event;
    next_event = GMEVENT_CreateOverlayEventCall( gameSys,
        FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)type );
    GMEVENT_ChangeEvent( event, next_event );

    return TRUE;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_PokewoodRec( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT *next_event;
  next_event = GMEVENT_CreateOverlayEventCall( gameSys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, NULL );
  GMEVENT_ChangeEvent( event, next_event );

  return TRUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_PokewoodShow1( DEBUG_MENU_EVENT_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_01 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_PokewoodShow2( DEBUG_MENU_EVENT_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_02 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_PokewoodShow3( DEBUG_MENU_EVENT_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_03 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_PokewoodShowRom( DEBUG_MENU_EVENT_WORK * wk )
{
  return callShowProc( wk, DEBUG_SHOW_ROM );
}


