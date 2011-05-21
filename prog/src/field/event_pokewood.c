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
//    ポケウッド呼び出し
//
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  //POKEWOOD_WORK * pokewood_work;
  u8 PokewoodMode;
  int parent_value;
  HEAPID heapID;

  u16 EncEffNo;
  u32 musicDefault;
  u32 musicWin;
}POKEWOOD_REPLAY_EVENT_WORK, PREW;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  BATTLE_SETUP_PARAM * bp;
  PREW * prew;
}POKEWOOD_BATTLE_EVENT_WORK, PBEW;


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

//============================================================================================
//============================================================================================
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
/**
 * @brief ポケウッド録画用セットアップ
 * @todo  録画への対応
 * @todo  シナリオへの対応
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodRecBattleParam(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  u16 tr_id = TRID_SCIENTISTW_01; //TRID_NULL;
  BATTLE_SETUP_PARAM * bsp = BATTLE_PARAM_Create( heapID );

  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );

  int i;

  BattleRec_Init( heapID );
  {
    BTL_FIELD_SITUATION sit;
    BTL_FIELD_SITUATION_Init( &sit );
    BTL_SETUP_Single_Trainer( bsp, gamedata, &sit, TRID_NULL, heapID );
    BTL_SETUP_AllocRecBuffer( bsp, heapID );
  }
  TT_EncountTrainerPersonalDataMake( tr_id, bsp->tr_data[BTL_CLIENT_ENEMY1], heapID );
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, enemyData, NELEMS(enemyData) );

  //プレイヤーセット
  { 
    //MyStatus
    bsp->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gamedata );
    
    // プレイヤーパーティーの登録
    setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, myData, NELEMS(myData) );
  }

  GFL_HEAP_FreeMemory( pp );
  bsp->PokewoodMode = BTL_POKEWOOD_REC;
  bsp->pokewoodSenario = 0; //仮

  TAMADA_Printf(" POKEWOOD BSP %08x\n", bsp );
  return bsp;
}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生用セットアップ
 * @todo  正式録画データへの対応
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodShowBattleParam(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, u8 replay_id )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  LOAD_RESULT result;

  BattleRec_Load( sv, heapID, &result, replay_id );
  if ( result == LOAD_RESULT_OK )
  {

    BATTLE_SETUP_PARAM * bsp;
    bsp = BATTLE_PARAM_Create( heapID );
    BATTLE_PARAM_Init( bsp );
    BTL_SETUP_InitForRecordPlay( bsp, gamedata, heapID );
    BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
    BattleRec_RestoreSetupParam( bsp, heapID );
    BattleRec_UnloadToolModule();
    bsp->PokewoodMode = BTL_POKEWOOD_SHOW;
    bsp->pokewoodSenario = 0; //仮
    return bsp;
  }
  else
  { //仮：再生データがない場合への対応
    return createPokewoodRecBattleParam( gsys, fieldmap, heapID );
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodBSP(
    const PREW * prew, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID )
{
  BATTLE_SETUP_PARAM * bsp;
  switch ( prew->PokewoodMode )
  {
  case BTL_POKEWOOD_NONE:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case BTL_POKEWOOD_REC:
    bsp = createPokewoodRecBattleParam( gsys, fieldmap, heapID );
    break;
  case BTL_POKEWOOD_SHOW:
    bsp = createPokewoodShowBattleParam( gsys, fieldmap, heapID, prew->parent_value );
    break;
  }
  bsp->musicDefault = prew->musicDefault;
  bsp->musicWin = prew->musicWin;
  bsp->badgeCount = 8; //無理やりいうことをきかせる
  return bsp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void deletePokewoodBSP( BATTLE_SETUP_PARAM * bsp )
{
  if ( bsp->PokewoodMode == BTL_POKEWOOD_SHOW )
  {
    GFL_HEAP_FreeMemory( bsp->playerStatus[ BTL_CLIENT_PLAYER ] );  //プレイヤーのMySatusは開放されないので
    BTL_SETUP_QuitForRecordPlay( bsp );
  }
  if ( BattleRec_DataExistCheck() )
  {
    BattleRec_Exit();
  }
  BATTLE_PARAM_Delete( bsp );
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  u8 bufID;
  BATTLE_SETUP_PARAM * bsp;
  GAMEDATA * gamedata;
  HEAPID heapID;

  u16 saveSeq0;
  u16 saveSeq1;

}POKEWOOD_SAVE_EVENT_WORK, PSEW;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodSaveEvent( GMEVENT * event, int * seq, void * work )
{
  PSEW * psew = (PSEW *)work;
  u8 bufID = psew->bufID;

  switch ( *seq )
  {
  case 0:
    /**
     *  BATTLE_SETUP_PARAM の録画データセーブ開始
     */
    BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
    BattleRec_StoreSetupParam( psew->bsp );
    psew->saveSeq0 = 0;
    psew->saveSeq1 = 0;
    OS_Printf("ポケウッドセーブ開始\n");
    (*seq) ++;
    break;

  case 1:
    /**
     *  BATTLE_SETUP_PARAM の録画データセーブ終了待ち
     */
    {
      SAVE_RESULT result;
      result = BattleRec_Save(
          psew->gamedata, psew->heapID, 0, 0, bufID, &psew->saveSeq0, &psew->saveSeq1);
      switch( result ){
      case SAVE_RESULT_OK:
        {
          u32* dataPtr = (u32*)BattleRec_GetOperationBufferPtr();
          OS_Printf("録画データ正常にセーブ完了 (bufID=%d, dataPtr=%p, dump=%08x)\n", bufID, dataPtr, *dataPtr);
        }
        (*seq)++;
        break;
      case SAVE_RESULT_NG:
        {
          u32* dataPtr = (u32*)BattleRec_GetOperationBufferPtr();
          OS_Printf("録画データセーブ失敗 ... (bufID=%d, dataPtr=%p, dump=%08x)\n", bufID, dataPtr, *dataPtr);
        }
        (*seq)++;
        break;

      default:
        break;
      }
    }
    break;

  case 2:
    BattleRec_UnloadToolModule();
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT * createPokewoodSaveEvent(
    GAMESYS_WORK * gsys, u8 bufID, BATTLE_SETUP_PARAM * bsp, HEAPID heapID )
{
  GMEVENT * event;
  PSEW * psew;
  event = GMEVENT_Create( gsys, NULL, pokewoodSaveEvent, sizeof( PSEW ) );
  psew = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( psew, sizeof(PSEW) );
  psew->bufID = bufID;
  psew->bsp = bsp;
  psew->heapID = heapID;
  psew->gamedata = GAMESYSTEM_GetGameData( gsys );
  return event;
}
//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodBattleEvent( GMEVENT * event, int *seq, void * work )
{
  PBEW * pbew = (PBEW*)work;
  PREW * prew = pbew->prew;

  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case 0:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, prew->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case 1:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, prew->EncEffNo);
    (*seq)++;
    break;

  // フィールドマップ終了
  case 2:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;

  case 3:
    pbew->bp = createPokewoodBSP( prew, gsys, fieldmap, prew->heapID );
    //侵入システムに戦闘中であることを伝える
    TAMADA_Printf(" BSP ADDRESS %08x\n", pbew->bp );
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, pbew->bp );
    (*seq)++;
    break;
    
  case 4:
    if ( prew->PokewoodMode == BTL_POKEWOOD_REC )
    { //とりあえず無理やりセーブしてみる
      u8 bufID = 0; //仮
      GMEVENT * new_event = createPokewoodSaveEvent( gsys, bufID, pbew->bp, GFL_HEAPID_APP );
      GMEVENT_CallEvent( event, new_event );
    }
    (*seq)++;
    break;

  case 5:
    deletePokewoodBSP( pbew->bp );
    //侵入システムにフィールド中であることを伝える
    (*seq) ++;
    break;

  case 6: 
    //戦闘結果反映処理
    (*seq) ++;
    break;

  case 7:
    // フィールドBGMを復帰
    GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle( gsys ) );
    (*seq) ++;
    break;

  case 8: 
    //勝ち負け判定
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;

  // BGMの復帰待ち
  case 9:
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( gsys ) );
    (*seq) ++;
    break;

  case 10:
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    (*seq) ++;
    break;

  case 11:
    //BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT * createPokewoodBattleEvent(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, PREW * prew )
{
  GMEVENT * event = GMEVENT_Create( gsys, NULL, pokewoodBattleEvent, sizeof(PBEW) );
  PBEW * pbew = GMEVENT_GetEventWork( event );
  pbew->prew = prew;
  return event;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodCallEvent( GMEVENT * event, int *seq, void * work )
{
  enum {
    PRE_SEQ_START = 0,
    PRE_SEQ_END,
  };
  PREW * prew = work;
  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = prew->heapID;

  switch ( *seq )
  {
  case PRE_SEQ_START:
    //仮：本来はシナリオなどから反映する
    if ( prew->PokewoodMode == BTL_POKEWOOD_SHOW )
    {
      prew->musicDefault = SEQ_BGM_EYE_09;
      prew->musicWin = SEQ_BGM_E_LAST;
    } else {
      prew->musicDefault = SEQ_BGM_VS_NORAPOKE;
      prew->musicWin = SEQ_BGM_WIN1;
    }
    {
      GMEVENT * call_event;
      call_event = createPokewoodBattleEvent( gsys, fieldmap, prew );
      GMEVENT_CallEvent( event, call_event );
    }
    *seq = PRE_SEQ_END;
    break;

  case PRE_SEQ_END:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//============================================================================================
//
//    外部公開関数
//
//============================================================================================
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

  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
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

  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  //prew->pokewood_work = pokewood_work;
  prew->heapID = HEAPID_PROC;
  prew->parent_value = (int)parent_work;
  OS_Printf(" POKEWOOD EVENT Parent Value = %x\n", parent_work );
  prew->PokewoodMode = BTL_POKEWOOD_SHOW;

  return event;
}



