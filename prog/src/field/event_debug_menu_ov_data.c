//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_data.c
 *	@brief  デバッグメニューデータ作成系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
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
#include  "item/itemsym.h"  //ITEM_DATA_MAX
#include  "item/item.h"  //ITEM_CheckEnable
#include "app/name_input.h"
#include "savedata/irc_compatible_savedata.h"
#include "waza_tool\wazano_def.h"
#include "savedata/battle_box_save.h"

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "event_debug_item.h" //EVENT_DebugItemMake
#include "event_debug_menu_mystery_card.h"
#include "event_debug_make_poke.h"
#include "event_debug_rewrite_poke.h"
#include "savedata/box_savedata.h"  //デバッグアイテム生成用
#include "event_debug_menu_make_egg.h"  // for DEBUG_EVENT_FLDMENU_MakeEgg
#include "event_debug_sodateya.h" // for DEBUG_EVENT_DebugMenuSodateya

//外部公開
#include "event_debug_menu_ov_data.h"

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
//======================================================================
//  デバッグメニュー ポケモン作成
//======================================================================

//----------------------------------------------------------------------------
/**
 *  @brief  ポケモン作成
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID( debug_menu_makepoke ), DEBUG_EVENT_DebugMenu_MakePoke, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return( TRUE );
}
//======================================================================
//  デバッグメニュー ポケモン書き換え
//======================================================================

//----------------------------------------------------------------------------
/**
 *  @brief  ポケモン書き換え
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugReWritePoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID( debug_menu_rewrite_poke ), DEBUG_EVENT_DebugMenu_ReWritePoke, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return( TRUE );
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
BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugItemMake, NULL );
  //child_event = EVENT_DebugItemMake(gameSys, wk->heapID);
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}
//--------------------------------------------------------------
/**
 * @brief   デバッグ隠されアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_DebugSecretItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugSecretItemMake, NULL );
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}
//--------------------------------------------------------------
/**
 * @brief   デバッグPDWアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_DebugPDWItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugPDWItemMake, NULL );
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}
//--------------------------------------------------------------
/**
 * @brief   ポケモンボックスにポケモンでいっぱいにする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  MYSTATUS *myStatus;
  POKEMON_PARAM *pp;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;

  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

  {
    int i,j,k=BOX_MAX_TRAY;
    int monsno=1;
    BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

    BOXDAT_AddTrayMax( pBox );
    BOXDAT_AddTrayMax( pBox );
    for(i=0;i < k;i++){
      for(j=0;j < 30;j++){
        OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
        PP_Setup(pp,  monsno , 100, MyStatus_GetID( myStatus ));

        ppp = (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
        PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
        PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

        BOXDAT_PutPokemonBox(pBox, i, ppp);

        monsno++;
        if( monsno > MONSNO_END ){
          monsno = 1;
        }
      }
    }
  }

  GFL_HEAP_FreeMemory(pp);
#endif
  return( FALSE );
}
//--------------------------------------------------------------
/**
 * @brief   ポケモンボックス１つ分をポケモンでいっぱいにする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_1BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  MYSTATUS *myStatus;
  POKEMON_PARAM *pp;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;

  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

  {
    int i,j,k=1;
    int monsno=1;
    BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

    BOXDAT_AddTrayMax( pBox );
    BOXDAT_AddTrayMax( pBox );
    for(i=0;i < k;i++){
      for(j=0;j < 30;j++){
        OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
        PP_Setup(pp,  monsno , 100, MyStatus_GetID( myStatus ));

        ppp = (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
        PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
        PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
        BOXDAT_PutPokemonBox(pBox, i, ppp);

        monsno++;
        if( monsno > MONSNO_END ){
          monsno = 1;
        }
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
BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk )
{
  u32 i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gmSys));

  for( i = 0; i <= ITEM_DATA_MAX; i++ ){
    if( ITEM_CheckEnable( i ) ){
      MYITEM_AddItem(myitem , i, 999, GFL_HEAPID_APP );
    }
  }
  return( FALSE );
}
//----------------------------------------------------------------------------
/**
 *  @brief タマゴさくせい
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_MakeEgg( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_make_egg ), DEBUG_EVENT_FLDMENU_MakeEgg, wk->fieldWork );

  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief 育て屋
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_Sodateya( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_sodateya ), DEBUG_EVENT_DebugMenuSodateya, wk->fieldWork );

  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}
//======================================================================
//  デバッグメニュー 国連データ作成
//======================================================================
#include "debug/debug_make_undata.h"
FS_EXTERN_OVERLAY(debug_make_undata);
static GMEVENT_RESULT debugMenuMakeUNData( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// デバッグ国連データ作成用ワーク
//=====================================
typedef struct
{
  HEAPID HeapID;
  GAMESYS_WORK    *gsys;
  GMEVENT         *Event;
  FIELDMAP_WORK *FieldWork;
  PROCPARAM_DEBUG_MAKE_UNDATA p_work;
} DEBUG_MAKE_UNDATA_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *  @brief  国連データ作成
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugMakeUNData( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *gsys  = p_wk->gmSys;
  GMEVENT       *event    = p_wk->gmEvent;
  FIELDMAP_WORK *fieldWork  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_MAKE_UNDATA_EVENT_WORK *evt_work;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));

  //イベントチェンジ
  GMEVENT_Change( event, debugMenuMakeUNData, sizeof(DEBUG_MAKE_UNDATA_EVENT_WORK) );
  evt_work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( evt_work, sizeof(DEBUG_MAKE_UNDATA_EVENT_WORK) );

  //ワーク設定
  evt_work->gsys = gsys;
  evt_work->Event = event;
  evt_work->FieldWork = fieldWork;
  evt_work->HeapID = heapID;
  evt_work->p_work.wh = SaveData_GetWifiHistory(pSave);

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ国連データ作成イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakeUNData( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_MAKE_UNDATA_EVENT_WORK *evt_work = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( evt_work->Event, EVENT_FieldSubProc( evt_work->gsys, evt_work->FieldWork,
          FS_OVERLAY_ID(debug_make_undata), &ProcData_DebugMakeUNData, &evt_work->p_work ) );
    *p_seq  = SEQ_PROC_END;
    break;
  case SEQ_PROC_END:
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}
//======================================================================
//  デバッグメニュー　不思議データ作成
//======================================================================

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　不思議データ作成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_MakeMysteryCardList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_mystery_card ), DEBUG_EVENT_DebugMenu_MakeMysteryCard, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE;
}
//--------------------------------------------------------------
/**
 * @brief   不正チェックを通るポケモンをバトルBOXへセットする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_SetBtlBox( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gameSys));
  MYSTATUS *myStatus;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;
  POKEPARTY *party = PokeParty_AllocPartyWork( HEAPID_FIELDMAP );
  BATTLE_BOX_SAVE* btlbox = BATTLE_BOX_SAVE_GetBattleBoxSave( sv );
  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );
  BATTLE_BOX_SAVE_InitWork(btlbox);



  {
    int i,j;

    static const u16 monsno_tbl1[] =
    {
      MONSNO_KORATTA,
      MONSNO_OTATI,
      MONSNO_POTIENA,
      MONSNO_BIPPA,
      MONSNO_PHI,
      MONSNO_ZIGUZAGUMA,
    };
    static const u16 waza_tbl1[] =
    {
      WAZANO_TAIATARI,
      WAZANO_HIKKAKU,
      WAZANO_TAIATARI,
      WAZANO_TAIATARI,
      WAZANO_HATAKU,
      WAZANO_TAIATARI,
    };
    static const u16 monsno_tbl2[] =
    {
      MONSNO_KAIRIKII,
      MONSNO_OKORIZARU,
      MONSNO_GOUKAZARU,
      MONSNO_HARITEYAMA,
      MONSNO_HERAKUROSU,
      MONSNO_RUKARIO,
    };
    static const u16 waza_tbl2[] =
    {
      WAZANO_BAKURETUPANTI,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
    };
    u16 poke_name[6] =
      L"デバグポケ";

    const u16 *p_monsno;
    const u16 *p_waza;
    u32 pow = 0;

    poke_name[ 5 ]  = 0xFFFF;


    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
    {
      p_monsno  = monsno_tbl1;
      p_waza    =waza_tbl1;
      pow       =0;
    }
    else
    {
      p_monsno  = monsno_tbl2;
      p_waza    =waza_tbl2;
      pow       =252;
    }


    for(i=0;i < PokeParty_GetPokeCountMax(party);i++){
      POKEMON_PARAM *p_temp = PP_Create( p_monsno[ i ], 100, 0, HEAPID_PROC );
      PP_Put( p_temp, ID_PARA_waza1, p_waza[ i ] );
      PP_Put( p_temp, ID_PARA_waza2, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_waza3, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_waza4, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_oyaname_raw, (u32)poke_name );
      PP_Put( p_temp, ID_PARA_pow_exp, pow );

      PP_Renew( p_temp );
      PokeParty_Add( party, p_temp );

      GFL_HEAP_FreeMemory( p_temp );
    }
    BATTLE_BOX_SAVE_SetPokeParty( btlbox, party );

  }

  GFL_HEAP_FreeMemory(party);
#endif
  return( FALSE );
}
//======================================================================
//  デバッグメニュー 主人公名前設定
//======================================================================
FS_EXTERN_OVERLAY(namein);
//-------------------------------------
/// デバッグ主人公名変更
//=====================================
typedef struct
{
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  NAMEIN_PARAM   *p_param;
  STRBUF          *p_default_str;
} DEBUG_CHANGENAME_EVENT_WORK;
//-------------------------------------
///   PROTOTYPE
//=====================================
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//--------------------------------------------------------------
/**
 * @brief   主人公名前を再設定する
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_ChangeName( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_CHANGENAME_EVENT_WORK  *p_ev_wk;
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  MYSTATUS  *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
  MISC      *p_misc     = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(p_gamedata) );

  //イヴェント
  GMEVENT_Change( p_event, debugEvnetChangeName, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;

  //デフォルト名
  p_ev_wk->p_default_str  = MyStatus_CreateNameString(p_mystatus, HEAPID_PROC);

  //名前入力ワーク設定
  p_ev_wk->p_param  = NAMEIN_AllocParam( HEAPID_PROC, NAMEIN_MYNAME, MyStatus_GetMySex(p_mystatus), 0, NAMEIN_PERSON_LENGTH, p_ev_wk->p_default_str, NULL );
  p_ev_wk->p_param->prof_flag  = NAMEIN_PROF_ALL_ENABLE;

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  主人公名前再設定イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
    SEQ_EXIT,
  };

  DEBUG_CHANGENAME_EVENT_WORK  *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
          FS_OVERLAY_ID(namein), &NameInputProcData, p_wk->p_param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( !NAMEIN_IsCancel( p_wk->p_param ) )
    {
      MYSTATUS      *p_mystatus  = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_wk->p_gamesys) );
      NAMEIN_CopyStr( p_wk->p_param, p_wk->p_default_str );
      MyStatus_SetMyNameFromString(p_mystatus, p_wk->p_default_str );
    }

    *p_seq  = SEQ_EXIT;
    break;


  case SEQ_EXIT:
    NAMEIN_FreeParam( p_wk->p_param );
    GFL_STR_DeleteBuffer( p_wk->p_default_str );

    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//--------------------------------------------------------------
//イベントポケモン作成
//--------------------------------------------------------------
#include "poke_tool/poke_memo.h"
#include "field/evt_lock.h" //ロックカプセルのイベントロックのため
static GMEVENT_RESULT debugMenuEventpokeCreate( GMEVENT *event, int *seq, void *wk );


static const DEBUG_MENU_LIST DATA_EventPokeCreate[] =
{
  { DEBUG_FIELD_EVEPOKE01, (void*)0, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE02, (void*)1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE03, (void*)2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE04, (void*)3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE05, (void*)4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE06, (void*)5, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE07, (void*)6, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE08, (void*)7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE09, (void*)8, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE10, (void*)9, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE11, (void*)10, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE12, (void*)11, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE13, (void*)100, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EVEPOKE14, (void*)101, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugEventPokeCreateMenu = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_EventPokeCreate),
  DATA_EventPokeCreate,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

typedef struct
{
  HEAPID heapId;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FLDMENUFUNC *menuFunc;
  FIELDMAP_WORK *fieldWork;
}DEBUG_EVENT_POKE_CREATE;

BOOL debugMenuCallProc_EventPokeCreate( DEBUG_MENU_EVENT_WORK * wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_EVENT_POKE_CREATE *work;

  GMEVENT_Change( event, debugMenuEventpokeCreate, sizeof(DEBUG_EVENT_POKE_CREATE) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_EVENT_POKE_CREATE) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->heapId = heapID;

  return TRUE;
}
static GMEVENT_RESULT debugMenuEventpokeCreate( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_EVENT_POKE_CREATE *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapId, &DebugEventPokeCreateMenu );
    (*seq)++;
    break;

  case 1:
    {
      const u32 ret = FLDMENUFUNC_ProcMenu(work->menuFunc);
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(work->gmSys);

      if( ret == FLDMENUFUNC_NULL )
      {
        return GMEVENT_RES_CONTINUE;
      }
      else
        if( ret == FLDMENUFUNC_CANCEL )
        {
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          return GMEVENT_RES_FINISH;
        }
        else
        {
          POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
          BOX_MANAGER *boxData = GAMEDATA_GetBoxManager(gmData);
          u64 id = 0xbb76c1c5;
          u64 rnd;
          if( ret < 100 )
          {
            POKEMON_PARAM *pp;
            //ポケモン作成
            switch( ret )
            {
            case 0: //10えいがセレビィ
              pp = PP_Create( MONSNO_SEREBHI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_SEREBIXI_BEFORE );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 1: //10えいがエンテイ
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_ENTEI , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_ENTEI , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 2: //10えいがライコウ
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_RAIKOU , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_RAIKOU , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 3: //10えいがスイクン
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_SUIKUN , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_SUIKUN , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 4: //10えいがセレビィ(後
              pp = PP_Create( MONSNO_SEREBHI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_SEREBIXI_AFTER );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 5: //10えいがエンテイ(後
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_ENTEI , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_ENTEI , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 6: //10えいがライコウ(後
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_RAIKOU , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_RAIKOU , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 7: //10えいがスイクン(後
              rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_SUIKUN , 0 , 0 , 0 , TRUE );
              pp = PP_CreateEx( MONSNO_SUIKUN , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
              PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 8: //メロディア
              pp = PP_Create( MONSNO_655 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 9: //ダルタニス
              pp = PP_Create( MONSNO_654 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 10: //インセクタ
              pp = PP_Create( MONSNO_656 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            case 11: //はいふシェイミ
              pp = PP_Create( MONSNO_SHEIMI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
              PP_Put( pp , ID_PARA_event_get_flag , TRUE );
              break;
            }
            {
              u16 oyaName[] = {L'イ',L'ベ',L'ン',L'ト',L'ぽ',L'け',0xFFFF};
              PP_Put( pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
              PP_Put( pp , ID_PARA_oyasex , PTL_SEX_MALE );
            }
            //手持ちに空きがあれば入れる
            if( PokeParty_GetPokeCount( party ) < 6 )
            {
              PokeParty_Add( party , pp );
            }
            else
            {
              BOXDAT_PutPokemon( boxData , PP_GetPPPPointer( pp ) );
            }
            GFL_HEAP_FreeMemory( pp );
          }
          else
          {
            switch( ret )
            {
            case 100://ロックカプセル
              {
                MYITEM_PTR myItem = GAMEDATA_GetMyItem( gmData );
                MYSTATUS *myStatus = GAMEDATA_GetMyStatus( gmData );
                MISC *miscData = GAMEDATA_GetMiscWork( gmData );

                EVTLOCK_SetEvtLock( miscData , EVT_LOCK_NO_LOCKCAPSULE , myStatus );
                MYITEM_AddItem( myItem , ITEM_ROKKUKAPUSERU , 1 , work->heapId );
              }
              break;
            case 101://ビクティチケット
              {
                MYITEM_PTR myItem = GAMEDATA_GetMyItem( gmData );
                MYSTATUS *myStatus = GAMEDATA_GetMyStatus( gmData );
                MISC *miscData = GAMEDATA_GetMiscWork( gmData );

                EVTLOCK_SetEvtLock( miscData , EVT_LOCK_NO_VICTYTICKET , myStatus );
                MYITEM_AddItem( myItem , ITEM_RIBATHITIKETTO , 1 , work->heapId );
              }
              break;


            }
          }
        }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}
//======================================================================================
//  シンボルポケモン作成
//======================================================================================
#include "event_debug_menu_symbol.h"
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　シンボルポケモン作成リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_SymbolPokeList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * new_event;
  new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_symbol ), EVENT_DEBUG_SymbolPokeList, wk );
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );

  return TRUE;
}
//--------------------------------------------------------------
//ミュージカル関係デバッグ
//--------------------------------------------------------------
#include "savedata/musical_save.h"
#include "musical/musical_debug.h"
FS_EXTERN_OVERLAY( gds_debug );
static GMEVENT_RESULT debugMenuMusical( GMEVENT *event, int *seq, void *wk );


static const DEBUG_MENU_LIST DATA_Musical[] =
{
  { DEBUG_FIELD_MUSICAL_01, (void*)0, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_MUSICAL_02, (void*)1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_MUSICAL_03, (void*)2, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugMusicalMenu =
{
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_Musical),
  DATA_Musical,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

typedef struct
{
  HEAPID heapId;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FLDMENUFUNC *menuFunc;
  FIELDMAP_WORK *fieldWork;
}DEBUG_MUSICAL_CREATE;

BOOL debugMenuCallProc_Musical( DEBUG_MENU_EVENT_WORK * wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_MUSICAL_CREATE *work;

  GMEVENT_Change( event, debugMenuMusical, sizeof(DEBUG_MUSICAL_CREATE) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MUSICAL_CREATE) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->heapId = heapID;

  return TRUE;
}

static GMEVENT_RESULT debugMenuMusical( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MUSICAL_CREATE *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapId, &DebugMusicalMenu );
    (*seq)++;
    break;

  case 1:
    {
      const u32 ret = FLDMENUFUNC_ProcMenu(work->menuFunc);
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(work->gmSys);
      MUSICAL_SAVE *musSave = GAMEDATA_GetMusicalSavePtr(gmData);
      if( ret == FLDMENUFUNC_NULL )
      {
        return GMEVENT_RES_CONTINUE;
      }
      else
        if( ret == FLDMENUFUNC_CANCEL )
        {
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          return GMEVENT_RES_FINISH;
        }
        else
        {
          switch( ret )
          {
          case 0:
            {
              u16 i;
              for( i=0;i<MUSICAL_ITEM_MAX_REAL;i++ )
              {
                MUSICAL_SAVE_AddItem( musSave , i );
              }
            }
            break;
          case 1:
            MUSICAL_SAVE_InitWork( musSave );
            break;
          case 2:
            {
              MUSICAL_SHOT_DATA *shotData = MUSICAL_SAVE_GetMusicalShotData( musSave );

              GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_debug) );
              if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
              {
                POKEPARTY* party = GAMEDATA_GetMyPokemon( gmData );
                POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , 0 );
                const u32 monsno = PP_Get( pp , ID_PARA_monsno , NULL );
                MUSICAL_DEBUG_CreateDummyData( shotData , monsno , work->heapId );
              }
              else
              {
                //一番重いのはペンドラー
                MUSICAL_DEBUG_CreateDummyData( shotData , 545 , work->heapId );
              }
              GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_debug));
            }
            break;
          }
        }
    }
  }

  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー ミュージカルショット作成
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
FS_EXTERN_OVERLAY( gds_debug );
BOOL debugMenu_CreateMusicalShotData( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  MUSICAL_SAVE *musSave   = MUSICAL_SAVE_GetMusicalSave( p_sv );

  GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_debug) );

  MUSICAL_DEBUG_CreateDummyData( MUSICAL_SAVE_GetMusicalShotData( musSave ), MONSNO_PIKATYUU , HEAPID_PROC );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_debug));

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィーリングデータを１つ追加
 *
 *	@param	p_sv_ctrl セーブコントロール
 */
//-----------------------------------------------------------------------------
static void FEELINGCHECK_AddRankData( SAVE_CONTROL_WORK *p_sv_ctrl )
{	
  IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( p_sv_ctrl );
  {	
    STRCODE	str[128];
    u16	strlen;
    u16 idx;
    static const struct
    {	
      u16 * p_str;
      u8  str_len;
      u8  sex;
      u8  month;
      u8  day;
      u32 ID;
      u16 mons_no;
    } scp_debug_rank_data[]	=
    {	
      {	
        L"いちたろう",
        5,
        1,
        10,
        22,
        0x573,
        1,
      },
      {	
        L"じろう",
        3,
        1,
        1,
        27,
        0x785,
        2,
      },
      {	
        L"さぶろう",
        4,
        1,
        4,
        8,
        0x123,
        3,
      },
      {	
        L"しろう",
        3,
        1,
        7,
        14,
        0x987,
        4,
      },
      {	
        L"ごろう",
        3,
        0,
        2,
        15,
        0x782,
        5,
      },
      {	
        L"ろくろう",
        4,
        0,
        8,
        2,
        0x191,
        6,
      },
      {	
        L"ななお",
        3,
        0,
        1,
        7,
        0x232,
        7,
      },
      {	
        L"はちべえ",
        4,
        0,
        11,
        27,
        0x595,
        8,
      },
      {	
        L"くろう",
        3,
        0,
        10,
        18,
        0x999,
        9,
      }
    };
    u16 *p_str;

    idx	= GFUser_GetPublicRand( NELEMS(scp_debug_rank_data) );

    p_str	= scp_debug_rank_data[idx].p_str;

    strlen	= scp_debug_rank_data[idx].str_len;
    GFL_STD_MemCopy(p_str, str, strlen*2);
    str[strlen]	= GFL_STR_GetEOMCode();

    //セーブする
    IRC_COMPATIBLE_SV_AddRanking( p_sv, str,
        GFUser_GetPublicRand( 101 ), scp_debug_rank_data[idx].sex, scp_debug_rank_data[idx].month, scp_debug_rank_data[idx].day, scp_debug_rank_data[idx].ID,
        scp_debug_rank_data[idx].mons_no, 0, PTL_SEX_MALE, 0 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk ワーク
 *	@retval TRUE続行  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenu_CreateFeelingData( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  FEELINGCHECK_AddRankData( p_sv );

  return FALSE;
}
