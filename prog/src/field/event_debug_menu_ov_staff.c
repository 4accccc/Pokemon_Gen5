//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_staff.c
 *	@brief  デバッグメニュースタッフ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef  PM_DEBUG

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
#include "system/medaldata.h" // fro MEDALDATA

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
#include "debug_wfbc.h"
#include "../../../resource/fldmapdata/script/pasokon_def.h"  //SCRID_PC
#include "savedata/shortcut.h"
#include "event_gtsnego.h"
#include "waza_tool/wazano_def.h" // for WAZANO_xxxx
#include "event_poke_status.h" // for EVENT_CreatePokeSelectWazaOboe

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "event_gts.h"
#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"
#include "net_app\wifi_country.h"
#include "savedata\medalbox.h"
#include "msg/debug/msg_debug_medal.h"

//外部公開
#include "event_debug_menu_ov_staff.h"

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
 * パソコン
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_PCScript( DEBUG_MENU_EVENT_WORK *wk )
{
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_PC, NULL, HEAPID_FIELDMAP );
  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　Yボタン登録操作
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  SHORTCUT *p_shortcut_sv = SaveData_GetShortCut( p_sv );
  for( i = 0; i < SHORTCUT_ID_MAX; i++ )
  {
    SHORTCUT_SetRegister( p_shortcut_sv, i, TRUE );
  }

  return FALSE;
}
//--------------------------------------------------------------
/**
 * GTSNegoメニュー呼びだし
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_OpenGTSNegoMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT *next_event;

  next_event = GMEVENT_CreateOverlayEventCall( gameSys, 
      FS_OVERLAY_ID(event_gtsnego), EVENT_GTSNegoChangeDebug, fieldWork );
  GMEVENT_ChangeEvent( event, next_event );
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 着信音
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_RingTone( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GAMESYS_WORK * gsys = p_wk->gmSys;
  GAMEDATA* gamedata  = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  if ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE3 ) == FALSE ) {
    FSND_RequestTVTRingTone( fsnd );
  } else {
    FSND_StopTVTRingTone( fsnd );
  }
  return FALSE;
}
//-----------------------------------------------------------------------------
/**
 *    WFBC自動チェック
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// イベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK* p_gsys;
  FIELDMAP_WORK* p_fieldmap;
} EV_DEBUGMENU_WFBC;


static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk );
//----------------------------------------------------------------------------
/**
 *  @brief  WFBCオートデバック
 */
//-----------------------------------------------------------------------------
BOOL debugMenu_WfbcDebug( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  EV_DEBUGMENU_WFBC *work;

  GMEVENT_Change( event, EV_debugMenu_WfbcDebug, sizeof(EV_DEBUGMENU_WFBC) );

  work = GMEVENT_GetEventWork( event );

  work->p_gsys = gsys;
  work->p_fieldmap = fieldWork;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  自動動作
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk )
{
  EV_DEBUGMENU_WFBC *work = wk;


  switch( *seq ){
  case 0:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_menu_wfbc_check) );
#ifdef DEBUG_ONLY_FOR_tomoya_takahashi
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
      GMEVENT_CallEvent( event, DEBUG_WFBC_PeopleCheck( work->p_gsys ) );
    }
    else
#endif //DEBUG_ONLY_FOR_tomoya_takahashi
    {
      GMEVENT_CallEvent( event, DEBUG_WFBC_View( work->p_gsys ) );
    }
    (*seq)++;
    break;

  case 1:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_menu_wfbc_check) );
    return ( GMEVENT_RES_FINISH );
  }

  return ( GMEVENT_RES_CONTINUE );
}


//-----------------------------------------------------------------------------
/// private
//-----------------------------------------------------------------------------
typedef struct {
  HEAPID        heap_id;
  GAMESYS_WORK  *gsys;
  FIELDMAP_WORK *p_fieldmap;

  u16     retDecide;
  u16     retPos;
} DEBUG_POKELIST_WORK;

// ポケリスト呼び出し
static GMEVENT_RESULT debugPokeList( GMEVENT *event, int *seq, void *wk );
static void checkSkillTeach( GAMESYS_WORK *gsys, u8 *oboeBit );

static void ichi_coutry_chekNG(u32 country_code, u32 local_code, u32 language)
{
    BOOL ret;
    ret = WIFI_COUNTRY_CheckNG( country_code, local_code, language );
    OS_Printf("ret %d ", ret);
    OS_Printf("coutry %d ", country_code);
    OS_Printf("local %d ", local_code);
    OS_Printf("language %d\n", language);
    return;
}

BOOL debugMenuCallProc_DebugTest_Ichi( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldMap = wk->fieldWork;
  u16 zone_id = FIELDMAP_GetZoneID( fieldMap );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( wk->gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( wk->gdata );
  MYSTATUS *s_mystatus = SaveData_GetMyStatus( save );

  // 技教えイベント
  {
    DEBUG_POKELIST_WORK *work;
    GMEVENT_Change( event, debugPokeList, sizeof(DEBUG_POKELIST_WORK) );
    
    work = GMEVENT_GetEventWork( event );
    GFL_STD_MemClear( work, sizeof(DEBUG_POKELIST_WORK) );

    work->heap_id = heapID;
    work->gsys = gsys;
    work->p_fieldmap = fieldMap;
  }

  // BUGFIX_COUNTRY_NGCHECK_20100806
  // 自分でデータをいろいろ送って関数の単体テストをする
  {
    u32 country_code;
    u32 local_code;
    u32 language;

    country_code  = MyStatus_GetMyNation(mystatus);
    local_code    = MyStatus_GetMyArea(mystatus);
    language      = MyStatus_GetRegionCode(mystatus);

    ichi_coutry_chekNG( 0, 0, language );
    ichi_coutry_chekNG( country_code, local_code, language );
    ichi_coutry_chekNG( country_code, local_code, LANG_ENGLISH );
    ichi_coutry_chekNG( country_code, 0, language );
    ichi_coutry_chekNG( 138, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 137, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 186, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 185, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 138, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 1, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 2, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 51, LANG_ENGLISH );
    ichi_coutry_chekNG( 220, 52, LANG_ENGLISH );
    ichi_coutry_chekNG( 227, 0, LANG_ENGLISH );
    ichi_coutry_chekNG( 228, 0, LANG_ENGLISH );
  }

  // 技教えチェック
  {
    int i;

    ICHI_Printf("ふしぎだね\n");
    ICHI_Printf("赤\n");
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 1, 0, i, 0 ));
    ICHI_Printf("\n青\n");
    // 青
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 1, 0, i, 1 ));
    ICHI_Printf("\n黄\n");
    // 黄
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 1, 0, i, 2 ));
    ICHI_Printf("\n緑\n");
    // 緑
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 1, 0, i, 3 ));

    ICHI_Printf("つたーじゃ\n");
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 495, 0, i, 0 ));
    ICHI_Printf("\n青\n");
    // 青
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 495, 0, i, 1 ));
    ICHI_Printf("\n黄\n");
    // 黄
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 495, 0, i, 2 ));
    ICHI_Printf("\n緑\n");
    // 緑
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 495, 0, i, 3 ));

    ICHI_Printf("ぽかぶ\n");
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 498, 0, i, 0 ));
    ICHI_Printf("\n青\n");
    // 青
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 498, 0, i, 1 ));
    ICHI_Printf("\n黄\n");
    // 黄
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 498, 0, i, 2 ));
    ICHI_Printf("\n緑\n");
    // 緑
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 498, 0, i, 3 ));

    ICHI_Printf("みじゅまる\n");
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 501, 0, i, 0 ));
    ICHI_Printf("\n青\n");
    // 青
    for( i=0; i<15; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 501, 0, i, 1 ));
    ICHI_Printf("\n黄\n");
    // 黄
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 501, 0, i, 2 ));
    ICHI_Printf("\n緑\n");
    // 緑
    for( i=0; i<10; i++ )
      ICHI_Printf( "%d_",POKETOOL_CheckWazaOshieSW( 501, 0, i, 3 ));
  }
  
  
  return( TRUE );

}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグポケリスト呼び出し
 *
 *	@param	GMEVENT *event  イベント
 *	@param	*seq    シーケンス
 *	@param	*wk     ワーク
 *	@return
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugPokeList( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_POKELIST_WORK *work = wk;

  switch( *seq ){
  case 0:
    {
      GMEVENT *call_event;
      EVENT_POKESEL_PARAM_WOBOE prm;
      prm.retDecide = &work->retDecide;
      prm.retPos    = &work->retPos;
      //prm.learnBit  = 0x33; // 適当、本当はチェックが必要
      checkSkillTeach( work->gsys, &prm.learnBit );
      prm.wazano    = WAZANO_GIHUTOPASU;
      call_event = GMEVENT_CreateOverlayEventCall( work->gsys,
          FS_OVERLAY_ID( notwifi_event_poke_status ), EVENT_CreatePokeSelectWazaOboe, (void*)&prm );
      GMEVENT_CallEvent( event, call_event );
    }
    (*seq)++;
    break;
  case 1:
    OS_Printf("retDecide %d\t", work->retDecide);
    OS_Printf("retPos %d\n", work->retPos);

    {
      u8 bit;
      checkSkillTeach( work->gsys, &bit );
      if( bit & (1<<work->retPos))
      {
        OS_Printf("選択したポケモンは覚えれる\n");
      }else{
        OS_Printf("選択したポケモンは覚えられない\n");
      }
    }

    (*seq)++;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  手持ちに技教えが可能か判定
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	*oboeBit    ビット単位でどのポケモンに教えられるか？を返す
 */
//-----------------------------------------------------------------------------
static void checkSkillTeach( GAMESYS_WORK *gsys, u8 *oboeBit )
{
  GAMEDATA *    gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY*    party = GAMEDATA_GetMyPokemon( gdata );
  int           max = PokeParty_GetPokeCount( party );

  int pos;
  int i;

  *oboeBit = 0;
  
  for ( pos = 0; pos < max; pos++ )
  {
    int waza_no = 0;
    int color = 0;
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, pos );
    if( PP_CheckWazaOshieSW( pp, waza_no, color) == TRUE )
    {
      *oboeBit |= ( 1 << pos );
    }
  }
  OS_Printf("覚えビット %x\n", *oboeBit);
}

//----------------------------------------------------------------------------
/**
 *	@brief  尾上用テスト
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugTest_Onoue( DEBUG_MENU_EVENT_WORK *wk )
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

  // 性別をひっくり返す
  sex = MyStatus_GetMySex( mystatus );
  sex ^= 1;
  MyStatus_SetMySex( mystatus, sex );
  MyStatus_SetMySex( s_mystatus, sex );

  // ユニオン見た目もひっくり返す
  MyStatus_SetTrainerView( mystatus,   sex*MYSTATUS_UNIONVIEW_MAX );
  MyStatus_SetTrainerView( s_mystatus, sex*MYSTATUS_UNIONVIEW_MAX );

  {
    GMEVENT * mapchange_event;
    mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldMap, zone_id );
    GMEVENT_ChangeEvent( event, mapchange_event );
  }

  return( TRUE );

}

#endif  //  PM_DEBUG

