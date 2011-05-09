//======================================================================
/**
 * @file  event_pokewood.c
 * @brief  ポケウッド　イベント
 * @date  2011.04.27
 * @author  tamada
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem\btl_setup.h"
#include "event_fieldmap_control.h"
#include "event_fldmmdl_control.h"
//#include "poke_tool/poke_regulation.h"
//#include "app/pokelist.h"
//#include "app/p_status.h"
//#include "app/leader_board.h"
#include "poke_tool/poke_memo.h"
#include "battle/battle.h"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"

#include "savedata/save_tbl.h"

#include "fieldmap.h"

#include "event_pokewood.h"

#include "system/net_err.h"

#include "waza_tool/wazano_def.h"
#include "item/itemsym.h"

#include "field_fade.h"
//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//============================================================================================
//
//    ポケウッド再生呼び出し
//
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  //POKEWOOD_WORK * pokewood_work;
  BATTLE_SETUP_PARAM * bp;
  u8 PokewoodMode;
  HEAPID heapID;
  MYSTATUS dmyMyStatus;
}POKEWOOD_REPLAY_EVENT_WORK, PREW;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  u8 level;
  u16 monsno;
  u16 wazano[4];
  u16 itemno;
}POKEWOOD_MONS_DATA;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const POKEWOOD_MONS_DATA myData[3] = {
  {
    50, MONSNO_RANKURUSU,
    { WAZANO_TORIKKU, WAZANO_MAMORU, WAZANO_SAIKOKINESISU, WAZANO_ZIKOSAISEI },
    0,  //no item
  },
  {
    50, MONSNO_BARUZIINA,
    { WAZANO_HUKITOBASI, WAZANO_MAMORU, WAZANO_TUBASADEUTU, WAZANO_NULL },
    0,  //no item
  },
  {
    50, MONSNO_DORYUUZU,
    { WAZANO_ZISIN, WAZANO_MAMORU, WAZANO_TUMETOGI, WAZANO_KOUSOKUSUPIN },
    0,  //no item
  },
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const POKEWOOD_MONS_DATA enemyData[2] = {
  {
    50, MONSNO_WASIBON,
    { WAZANO_TONBOGAERI, WAZANO_TUBAMEGAESI, WAZANO_NULL, WAZANO_NULL },
    ITEM_KUROITEKKYUU,
  },
  {
    50, MONSNO_ufo,
    { WAZANO_DENGEKIHA, WAZANO_SAIKOWHEEBU, WAZANO_KOUSOKUIDOU, WAZANO_NULL },
    0,  //no item
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const POKEWOOD_MONS_DATA * mons_data, u32 max )
{
  //
  int i;
  u32 id = MyStatus_GetID( mystat );
  u8 sex = MyStatus_GetMySex( mystat );

  POKEPARTY * add_party = PokeParty_AllocPartyWork( heapID );
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  GF_ASSERT( max <= TEMOTI_POKEMAX );
  PokeParty_Init( add_party, max );
  for ( i = 0; i < max; i ++, mons_data ++ )
  {
    PP_Clear( pp );
    PP_SetupEx( pp, mons_data->monsno, mons_data->level, id,
        PTL_SETUP_POW_AUTO, PTL_SETUP_RND_AUTO );
    POKE_MEMO_SetTrainerMemoPP( pp , POKE_MEMO_SET_CAPTURE , mystat, MAPNAME_T1KANOKO , heapID );
    PP_Put( pp, ID_PARA_oyasex, sex );
    PP_SetWazaPos( pp, mons_data->wazano[0], 0 );
    PP_SetWazaPos( pp, mons_data->wazano[1], 1 );
    PP_SetWazaPos( pp, mons_data->wazano[2], 2 );
    PP_SetWazaPos( pp, mons_data->wazano[3], 3 );
    PP_Put( pp, ID_PARA_item, mons_data->itemno );
    PokeParty_Add( add_party, pp );
  }
  PokeParty_Copy( add_party, party );
  GFL_HEAP_FreeMemory( pp );
  GFL_HEAP_FreeMemory( add_party );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodReplayBattleParam(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  u16 tr_id = TRID_SCIENTISTW_01; //TRID_NULL;
  BATTLE_SETUP_PARAM * bsp = BATTLE_PARAM_Create( heapID );

  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );

  int i;

  {
    BTL_FIELD_SITUATION sit;
    BTL_FIELD_SITUATION_Init( &sit );
    BTL_SETUP_Single_Trainer( bsp, gamedata, &sit, tr_id, heapID );
  }
  //TT_EncountTrainerPersonalDataMake( tr_id, bsp->tr_data[BTL_CLIENT_ENEMY1], heapID );
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, enemyData, NELEMS(enemyData) );

  //プレイヤーセット
  { 
    //MyStatus
    bsp->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gamedata );
    
    // プレイヤーパーティーの登録
    setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, myData, NELEMS(myData) );
  }

  GFL_HEAP_FreeMemory( pp );

  TAMADA_Printf(" POKEWOOD BSP %08x\n", bsp );
  return bsp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodBattleEvent( GMEVENT * event, int *seq, void * work )
{
  enum {
    PRE_SEQ_START = 0,
    PRE_SEQ_FADE_IN,
    PRE_SEQ_FADE_WAIT,
    PRE_SEQ_END,
  };
  PREW * prew = work;
  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = prew->heapID;

  switch ( *seq )
  {
  case PRE_SEQ_START:
    {
      GMEVENT * call_event;
      //MyStatus_Copy( GAMEDATA_GetMyStatus( gamedata ), &prew->dmyMyStatus );
      prew->bp = createPokewoodReplayBattleParam( gsys, fieldmap, heapID );
      prew->bp->badgeCount = 8; //無理やりいうことをきかせる
      prew->bp->PokewoodMode = prew->PokewoodMode;
      GF_ASSERT( prew->bp->PokewoodMode != BTL_POKEWOOD_NONE );
      prew->bp->pokewoodSenario = 0;
      call_event = EVENT_PokewoodBattle( gsys, fieldmap, prew->bp );
      GMEVENT_CallEvent( event, call_event );
      if ( prew->bp->PokewoodMode == BTL_POKEWOOD_SHOW )
      {
        prew->bp->musicDefault = SEQ_BGM_EYE_09;
        prew->bp->musicWin = SEQ_BGM_E_LAST;
      }
    }
    *seq = PRE_SEQ_FADE_IN;
    break;

  case PRE_SEQ_FADE_IN:
    FIELDMAP_CreateMapFadeReqTask( gsys, fieldmap, FMAP_FADE_IN, FIELD_FADE_BLACK, FIELD_FADE_WAIT );
    *seq = PRE_SEQ_FADE_WAIT;
    break;

  case PRE_SEQ_FADE_WAIT:
    if( FIELDMAP_CheckMapFadeReqFlg( fieldmap ) == FALSE )
    {
      *seq = PRE_SEQ_END;
    }
    break;

  case PRE_SEQ_END:
    BATTLE_PARAM_Delete( prew->bp );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生イベントの生成
 * @param gsys
 * @return  生成イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * POKEWOOD_EVENT_Rec( GAMESYS_WORK *gsys, void * parent_work )
{
  GMEVENT * event;
  PREW * prew;
  //POKEWOOD_WORK * pokewood_work = (POKEWOOD_WORK*)parent_work;

  event = GMEVENT_Create( gsys, NULL, pokewoodBattleEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  //prew->pokewood_work = pokewood_work;
  prew->heapID = HEAPID_PROC;
  OS_Printf(" POKEWOOD EVENT Parent Value = %x\n", parent_work );
  prew->PokewoodMode = BTL_POKEWOOD_REC;

  return event;
}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生イベントの生成
 * @param gsys
 * @return  生成イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * POKEWOOD_EVENT_Show( GAMESYS_WORK *gsys, void * parent_work )
{
  GMEVENT * event;
  PREW * prew;
  //POKEWOOD_WORK * pokewood_work = (POKEWOOD_WORK*)parent_work;

  event = GMEVENT_Create( gsys, NULL, pokewoodBattleEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  //prew->pokewood_work = pokewood_work;
  prew->heapID = HEAPID_PROC;
  OS_Printf(" POKEWOOD EVENT Parent Value = %x\n", parent_work );
  prew->PokewoodMode = BTL_POKEWOOD_SHOW;

  return event;
}


#if 0
//============================================================================================
//  WBT　イベント　ポケモンリスト
//============================================================================================
//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  WBTSYS * wbtsys;
  
  PSTATUS_DATA status_data;

  PLIST_DATA list_data;
  
}EVENT_WORK_POKE_LIST;

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_SelectPokeList( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_POKE_LIST *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event,
        EVENT_FieldFadeOut_Black(gsys,work->fieldmap,FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PokeSelect(
          gsys,work->fieldmap,&work->list_data,&work->status_data) );
    (*seq)++;
    break;
  case 3:
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(
          gsys,work->fieldmap, FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 5:
    WBTSYS_SetPokelistResult( work->wbtsys, &work->list_data );
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント　呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * WBT_EVENT_SetSelectPokeList( GAMESYS_WORK * gsys, void * parent_work )
{
  GMEVENT *event;
  GAMEDATA *gamedata;
  WBTSYS * wbtsys;
  POKEPARTY *pp;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;
  
  wbtsys   = (WBTSYS *)parent_work;
  gamedata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  
  event = GMEVENT_Create( gsys, NULL,
      ev_SelectPokeList, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->wbtsys = wbtsys;
  
  WBTSYS_SetupPokeListData( wbtsys, gamedata, &work->list_data );
  
  WBTSYS_SetupPokeStatusData( wbtsys, gamedata, &work->status_data );
   
  return( event );
}

//============================================================================================
//
//    WBTトーナメントリスト画面呼び出し
//
//============================================================================================
//-------------------------------------
///
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  WBTLIST_PARAM       *wbtlist_param;
  BOOL                call_fade_flag;
} WBTLIST_EVENT_WORK;

static GMEVENT_RESULT eventWBTList( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * WBT_EVENT_ShowTournamentList(
    GAMESYS_WORK * gsys, void * parent_work )
{ 
  WBTLIST_PARAM * wbtlist_param = (WBTLIST_PARAM *) parent_work;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  WBTLIST_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventWBTList, sizeof(WBTLIST_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(WBTLIST_EVENT_WORK) );

  p_param->gsys  = gsys;
  p_param->fieldmap    = fieldmap;
  p_param->wbtlist_param = wbtlist_param;
  p_param->call_fade_flag = wbtlist_param->call_fade_flag;

  OS_Printf( "WBT List Start\n" );

  return new_event;

}
//----------------------------------------------------------------------------
/**
 *  @brief  WBTリストアプリ呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventWBTList( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  WBTLIST_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    PMSND_PlayerSetVolume( SEPLAYER_SE2, 63 );
    if ( p_wk->call_fade_flag )
    {
      call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
          FS_OVERLAY_ID(wbtlist), &WBTListProcData, p_wk->wbtlist_param );
    } else {
      call_event = EVENT_FieldSubProcNoFade( p_wk->gsys, p_wk->fieldmap,
          FS_OVERLAY_ID(wbtlist), &WBTListProcData, p_wk->wbtlist_param );
    }
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    WBTLIST_PARAM_Delete( p_wk->wbtlist_param );
    PMSND_PlayerSetVolume( SEPLAYER_SE2, 127 );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}


#endif



