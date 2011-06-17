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
#include "app/pkwd_count.h"
#include "field_sound_system.h"
#include "message.naix"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"
#include "tr_tool/trtype_def.h"

#include "savedata/save_tbl.h"

#include "fieldmap.h"

#include "event_pokewood.h"
//#include "event_debug_menu_ov_pokewood.h"

#include "system/net_err.h"

#include "waza_tool/wazano_def.h"
#include "item/itemsym.h"
#include "savedata/pokewood_rec.h"
#include "msg/msg_pokewood_name.h"

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
  u8 SenarioNo;
  int rec_id;
  HEAPID heapID;

  u16 EncEffNo;
  PKWD_COUNT_PARAM pkwd_count_param;
//  int musicDefault;
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
  u8  pow[12];
  u16 seikaku;
}POKEWOOD_MONS_DATA;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const POKEWOOD_MONS_DATA myData[2][3] = {
   {
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
  },
  {
    {
      15, MONSNO_RUKARIO,
      { WAZANO_HADOUDAN, WAZANO_REITOUPANTI, WAZANO_INFAITO, WAZANO_NULL },
      0,  //no item
      { 0,0, 31,255, 0,0, 0,0, 31,255, 0,0,},
      PTL_SEIKAKU_YANTYA,
    },
  }
};

static const int myDataNum[]={3,1};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const POKEWOOD_MONS_DATA enemyData[2][2] = {
  {
    {
      50, MONSNO_WASIBON,
      { WAZANO_TONBOGAERI, WAZANO_TUBAMEGAESI, WAZANO_TUTUKU, WAZANO_NULL },
      ITEM_KUROITEKKYUU,
    },
    {
      50, MONSNO_ufo,
      { WAZANO_DENGEKIHA, WAZANO_SAIKOWHEEBU, WAZANO_KOUSOKUIDOU, WAZANO_NULL },
      0,  //no item
    }
  },
  {
    {
      15, MONSNO_ZURUGGU,
      { WAZANO_TOBIHIZAGERI, WAZANO_NULL, WAZANO_NULL, WAZANO_NULL },
      0,
      { 0,0, 31,255, 0,0, 0,0, 0,0, 0,0,},
      PTL_SEIKAKU_HIKAEME,
    },
    {
      20, MONSNO_YAMIKARASU,
      { WAZANO_DAMASIUTI, WAZANO_EASURASSYU, WAZANO_NULL, WAZANO_NULL },
      0,  //no item
      { 0,0, 31,255, 0,0, 31,0, 31,255, 31,0, },
      PTL_SEIKAKU_OTTORI,
    }
  },
};

static const int enemyDataNum[]={2,2,};

#if 0
  ID_PARA_hp_rnd,     //HP乱数
  ID_PARA_hp_exp,     //HP努力値
  ID_PARA_pow_rnd,    //攻撃力乱数
  ID_PARA_pow_exp,    //攻撃力努力値
  ID_PARA_def_rnd,    //防御力乱数
  ID_PARA_def_exp,    //防御力努力値
  ID_PARA_agi_rnd,    //素早さ乱数
  ID_PARA_agi_exp,    //素早さ努力値
  ID_PARA_spepow_rnd, //特攻乱数
  ID_PARA_spepow_exp, //特攻努力値
  ID_PARA_spedef_exp, //特防努力値
  ID_PARA_spedef_rnd, //特防乱数
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const int trainerData[][2]={
  { TRTYPE_SCIENTISTW, TRTYPE_SCIENTISTW,   },     // 研究員
  { TRTYPE_LEADER7,    TRTYPE_PKWD_VILLAIN, },     // ハチクマン
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
    PP_Put( pp, ID_PARA_seikaku, mons_data->seikaku );
    PP_Put( pp, ID_PARA_item, mons_data->itemno );
    PP_Put( pp, ID_PARA_hp_rnd,    mons_data->pow[0] );
    PP_Put( pp, ID_PARA_hp_exp,    mons_data->pow[1] );
    PP_Put( pp, ID_PARA_pow_rnd,   mons_data->pow[2] );
    PP_Put( pp, ID_PARA_pow_exp,   mons_data->pow[3] );
    PP_Put( pp, ID_PARA_def_rnd,   mons_data->pow[4] );
    PP_Put( pp, ID_PARA_def_exp,   mons_data->pow[5] );
    PP_Put( pp, ID_PARA_agi_rnd,   mons_data->pow[6] );
    PP_Put( pp, ID_PARA_agi_exp,   mons_data->pow[7] );
    PP_Put( pp, ID_PARA_spepow_rnd,mons_data->pow[8] );
    PP_Put( pp, ID_PARA_spepow_exp,mons_data->pow[9] );
    PP_Put( pp, ID_PARA_spedef_rnd,mons_data->pow[10] );
    PP_Put( pp, ID_PARA_spedef_exp,mons_data->pow[11] );
    PP_Renew( pp );
    PokeParty_Add( add_party, pp );
  }
  PokeParty_Copy( add_party, party );
  GFL_HEAP_FreeMemory( pp );
  GFL_HEAP_FreeMemory( add_party );
}

#include  "tr_tool/tr_tool.h"


//----------------------------------------------------------------------------------
/**
 * @brief	トレーナーデータを格納する
 *
 * @param   senario_no	  シナリオＮＯ
 * @param   PokewoodType	BTL_POKEWOOD_REC か BTL_POKEWOOD_SHOW
 * @param   data		      トレーナーデータ格納先
 * @param   heapID		    HEAPID
 */
//----------------------------------------------------------------------------------
static void dataMakeEncountTrainer( int senario_no, int PokewoodType, BSP_TRAINER_DATA* data, HEAPID heapID )
{
  
  data->tr_id   = TRID_NULL;
  data->tr_type = trainerData[senario_no][BTL_POKEWOOD_REC-PokewoodType];
  data->ai_bit  = 0;
  {
    GFL_MSGDATA* man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pokewood_name_dat, heapID );
    if(PokewoodType==BTL_POKEWOOD_SHOW){
      GFL_MSG_GetString( man, pw_show_trainername_01+senario_no, data->name );
    }else{
      GFL_MSG_GetString( man, pw_rec_trainername_01+senario_no, data->name );
    }
    GFL_MSG_Delete(man);
  }
  PMSDAT_Clear( &data->win_word );
  PMSDAT_Clear( &data->lose_word );

}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド録画用セットアップ
 * @todo  録画への対応
 * @todo  シナリオへの対応
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodRecBattleParam(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, int senario_no )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
//  u16 tr_type = trainerData[senario_no]; //TRID_NULL;
  BATTLE_SETUP_PARAM * bsp = BATTLE_PARAM_Create( heapID );

  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );

  int i;

  POKEWOOD_REC_Init( heapID );
  {
    BTL_FIELD_SITUATION sit;
    BTL_FIELD_SITUATION_Init( &sit );
    BTL_SETUP_Single_Trainer( bsp, gamedata, &sit, TRID_NULL, heapID );
    BTL_SETUP_AllocRecBuffer( bsp, heapID );
  }
//  TT_EncountTrainerPersonalDataMake( tr_id, bsp->tr_data[BTL_CLIENT_ENEMY1], heapID );
  dataMakeEncountTrainer( senario_no, BTL_POKEWOOD_REC, bsp->tr_data[BTL_CLIENT_ENEMY1], heapID);
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, enemyData[senario_no], enemyDataNum[senario_no] );

  //プレイヤーセット
  { 
    //MyStatus
    bsp->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gamedata );
    
    // プレイヤーパーティーの登録
    setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, myData[senario_no], myDataNum[senario_no] );
  }

  GFL_HEAP_FreeMemory( pp );
  bsp->PokewoodMode = BTL_POKEWOOD_REC;
  bsp->pokewoodSenario = senario_no; //仮

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
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, u8 replay_id, int senario_no )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  LOAD_RESULT result;

  OS_Printf("load id=%d\n", replay_id);
  POKEWOOD_REC_Init( heapID );

  result = POKEWOOD_REC_Load( sv, heapID, replay_id );
  if ( result == LOAD_RESULT_OK )
  {
    BATTLE_SETUP_PARAM * bsp;
    bsp = BATTLE_PARAM_Create( heapID );
    BATTLE_PARAM_Init( bsp );
    BTL_SETUP_InitForRecordPlay( bsp, gamedata, heapID );
    BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
    BattleRec_RestoreSetupParam( bsp, BTLREC_RESOTRE_MODE_POKEWOOD, heapID );
    BattleRec_UnloadToolModule();
//    bsp->PokewoodMode = BTL_POKEWOOD_SHOW;  //RestoreSetupParamが入れてくれる
//    bsp->pokewoodSenario = senario_no; 
    return bsp;
  }
  else
  { //仮：再生データがない場合への対応
    return createPokewoodRecBattleParam( gsys, fieldmap, heapID, senario_no );
  }
}

// BGMのテーブル（TODO:外部化）
static const showBgmTable[][2]={
  { SEQ_BGM_EYE_09,     SEQ_BGM_E_LAST, },
  { SEQ_BGM_E_HANDSOME, SEQ_BGM_WIN2,   },
};



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
    bsp = createPokewoodRecBattleParam( gsys, fieldmap, heapID, prew->SenarioNo );
    bsp->musicDefault = SEQ_BGM_VS_NORAPOKE;
    bsp->musicWin     = SEQ_BGM_WIN1;
    break;
  case BTL_POKEWOOD_SHOW:
    bsp = createPokewoodShowBattleParam( gsys, fieldmap, heapID, prew->rec_id, prew->SenarioNo );
    bsp->musicDefault = showBgmTable[bsp->pokewoodSenario][0];
    bsp->musicWin     = showBgmTable[bsp->pokewoodSenario][1];
    break;
  }
  bsp->badgeCount   = 8; //無理やりいうことをきかせる
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
  POKEWOOD_REC_Exit();
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
    // BATTLE_SETUP_PARAMを上映用のトレーナーデータに書き換える
    dataMakeEncountTrainer( psew->bsp->pokewoodSenario, BTL_POKEWOOD_SHOW, psew->bsp->tr_data[BTL_CLIENT_ENEMY1], psew->heapID );
    POKEWOOD_REC_ConvertRecToShow( psew->bsp, psew->heapID );
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
      result = POKEWOOD_REC_Save(psew->gamedata, psew->heapID, bufID, &psew->saveSeq0 );
      switch( result ){
      case SAVE_RESULT_OK:
        OS_Printf("save result=%d\n", result);
        (*seq)++;
        break;
      case SAVE_RESULT_NG:
        OS_Printf("save result=%d\n", result);
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

static void _init_pkwd_count_param( PKWD_COUNT_PARAM *param, BATTLE_SETUP_PARAM *bsp )
{
  STRBUF *str       = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_PROC );
  POKEMON_PARAM *pp = GFL_HEAP_AllocMemory( HEAPID_PROC, POKETOOL_GetWorkSize() );

  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  
  // 録画データから手持ちポケモンの名前を取得する
  POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_POKE0, pp );
  PP_Get( pp, ID_PARA_nickname, str );
  param->message = str;
  BattleRec_UnloadToolModule();
  GFL_HEAP_FreeMemory( pp );
  
  // 現在はシナリオNOとタイトル画像指定番号を同じにする
  param->title_no = bsp->pokewoodSenario;
}

static void _delete_pkwd_count_param( PKWD_COUNT_PARAM *param )
{
  // 取得しポケモン名文字列の解放
  GFL_STR_DeleteBuffer( param->message );
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
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case 0:
    // BATTLE_SETUP_PARAM生成
    pbew->bp = createPokewoodBSP( prew, gsys, fieldmap, prew->heapID );
    // プレイヤーボリュームをフェードアウト
    FIELD_SOUND_ChangePlayerVolume( fsnd, 0, 20 );  
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, pbew->bp->musicDefault));
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
  // 撮影ならバトルへ、上映なら事前アプリの後バトルへ
  case 3:
//    pbew->bp = createPokewoodBSP( prew, gsys, fieldmap, prew->heapID );
    //侵入システムに戦闘中であることを伝える
    TAMADA_Printf(" BSP ADDRESS %08x\n", pbew->bp );
    if ( prew->PokewoodMode == BTL_POKEWOOD_REC ){
      (*seq)=102;
    }else{
#ifdef PM_DEBUG
      if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
        (*seq)=102;   //直接バトルへ
      }else{
        (*seq)=100;   //上映前アプリを実行してからバトルへ
      }
#else
      (*seq)=100;   //上映前アプリを実行してからバトルへ
#endif
    }
    break;
  // 事前アプリ
  case 100:
//    prew->pkwd_count_param.gamesys = gsys;
    _init_pkwd_count_param( &prew->pkwd_count_param, pbew->bp );
    GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_announce), &PkwdCountProcData, &prew->pkwd_count_param );
    (*seq) = 101;
    break;
  case 101:
    _delete_pkwd_count_param( &prew->pkwd_count_param );
    (*seq) = 102;
    break;
  // バトル呼び出し
  case 102:
    // BGM切り替え
    FIELD_SOUND_ChangePlayerVolume( fsnd, MAX_VOLUME, 1 );  

    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, pbew->bp );
    (*seq) = 4;
    break;
  // 撮影時はセーブ
  case 4:
    if ( prew->PokewoodMode == BTL_POKEWOOD_REC )
    { //とりあえず無理やりセーブしてみる
      u8 bufID = prew->rec_id;
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

#include "app/pkwd_count.h"



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
     //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    //仮：本来はシナリオなどから反映する
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
  POKEWOOD_EVENT_PARAM *param = (POKEWOOD_EVENT_PARAM*)parent_work;
  //POKEWOOD_WORK * pokewood_work = (POKEWOOD_WORK*)parent_work;

  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  prew->rec_id    = param->type; // 録画NO
  prew->SenarioNo = param->senario_no;
  prew->heapID    = HEAPID_PROC;
  OS_Printf(" POKEWOOD EVENT rec_id = %x, senario = %d\n", param->type, param->senario_no );
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
  int  rec_id = *(int*)parent_work;

  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  prew->heapID = HEAPID_PROC;
  prew->rec_id = rec_id;
  OS_Printf(" POKEWOOD EVENT Parent Value = %x\n", rec_id);
  prew->PokewoodMode = BTL_POKEWOOD_SHOW;

  return event;
}




//============================================================================================
//
//    PKWD_COUNT画面呼び出し
//
//============================================================================================
extern void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param );
//-------------------------------------
///   PKWD_COUNT_PARAM定義
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  PKWD_COUNT_PARAM    pkwd_param;
} PKWD_COUNT_EVENT_WORK;

static GMEVENT_RESULT eventPkwdCount( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PKWD_COUNT呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_COUNT_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_COUNT_PARAM *param = (PKWD_COUNT_PARAM *)parent_work;
  PKWD_COUNT_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdCount, sizeof(PKWD_COUNT_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_COUNT_EVENT_WORK) );

  p_param->gsys      = gsys;
  p_param->fieldmap  = fieldmap;
  p_param->pkwd_param.scenario_no = param->scenario_no;

  // デバッグメニューからもらった引数としてのPOKEWOOD_EVENT_PARAMは解放
  PKWD_COUNT_PARAM_Delete( param );

  OS_Printf( "PKWD COUNT Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  Pkwd呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdCount( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_COUNT_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_announce ), &PkwdCountProcData, &p_wk->pkwd_param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    PKWD_COUNT_PARAM_Delete( &p_wk->pkwd_param );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

