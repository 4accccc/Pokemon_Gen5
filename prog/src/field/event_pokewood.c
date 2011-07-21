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
#include "app/pokelist.h"
//#include "app/p_status.h"
//#include "app/leader_board.h"
#include "poke_tool/poke_memo.h"
#include "battle/battle.h"
#include "app/pkwd_count.h"
#include "app/pkwd_book.h"
#include "app/pkwd_selectscreen.h"
#include "field_sound_system.h"
#include "message.naix"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"
#include "tr_tool/trtype_def.h"

#include "savedata/save_tbl.h"
#include "savedata/misc.h"

#include "fieldmap.h"

#include "battle/pokewood.h"  //POKEWOOD_SCENEARIO_MAX
#ifndef POKEWOOD_SCENEARIO_MAX
#define POKEWOOD_SCENEARIO_MAX POKEWOOD_SENARIO_MAX
#endif

#include "field/pokewood.h"
#include "pokewood_system.h"
#include "event_pokewood.h"
//#include "event_debug_menu_ov_pokewood.h"

#include "system/net_err.h"

#include "waza_tool/wazano_def.h"
#include "item/itemsym.h"
#include "savedata/pokewood_rec.h"
#include "msg/msg_pokewood_name.h"

#include "pokewood_party.naix"

#include "field_fade.h"

//定義した値がずれないようにしている
SDK_COMPILER_ASSERT( PKWD_REC_POS_00 == LOADDATA_POKEWOOD0 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_01 == LOADDATA_POKEWOOD1 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_02 == LOADDATA_POKEWOOD2 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_03 == LOADDATA_POKEWOOD3 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_MAX <= LOADDATA_POKEWOOD_MAX );

//======================================================================
//  define
//======================================================================
// pokewood
enum{
  PKWDS_REC_POKE = 0,   // 撮影用ポケモン
  PKWDS_PLAY_POKE,      // 上映用ポケモン
  PKWDS_LEVEL,          // レベル
  PKWDS_FORM,           // フォルム
  PKWDS_SEX,            // 性別
  PKWDS_MIND,           // 性格
  PKWDS_STATE,          // 特性
  PKWDS_GOODS,          // 道具
  PKWDS_SKILL_1,        // 技1
  PKWDS_SKILL_2,        // 技2
  PKWDS_SKILL_3,        // 技3
  PKWDS_SKILL_4,        // 技4
  PKWDS_HP_FIX,         // HP固体値
  PKWDS_HP_EFF,         // HP努力値
  PKWDS_ATK_FIX,        // 攻撃固体値
  PKWDS_ATK_EFF,        // 攻撃努力値
  PKWDS_DEF_FIX,        // 防御固体値
  PKWDS_DEF_EFF,        // 防御努力値
  PKWDS_SPD_FIX,        // 早さ固体値
  PKWDS_SPD_EFF,        // 早さ努力値
  PKWDS_SPATK_FIX,      // 特攻固体値
  PKWDS_SPATK_EFF,      // 特攻努力値
  PKWDS_SPDEF_FIX,      // 特防固体値
  PKWDS_SPDEF_EFF,      // 特防努力値
  //
  PKWDS_PARAM_MAX,
};

// ----------------------------------------
// ポケウッド用ポケモンデータ（cast用）
// ----------------------------------------

typedef struct{
  s16 rec_monsno, show_monsno;  // 撮影用ポケモン/上映用ポケモン
  s16 level;                    // レベル
  s16 form;                     // フォルム
  s16 sex;                      // 性別
  s16 seikaku;                  // 性格
  s16 tokusei;                  // 特性
  s16 item;                     // 道具
  s16 waza[4];                  // 技４つ
  s16 hp_rnd;                   // HP固体値
  s16 hp_eff;                   // HP努力値
  s16 pow_rnd;                  // 攻撃固体値
  s16 pow_eff;                  // 攻撃努力値
  s16 def_rnd;                  // 防御固体値
  s16 def_eff;                  // 防御努力値
  s16 spd_rnd;                  // 早さ固体値
  s16 spd_eff;                  // 早さ努力値
  s16 spepow_rnd;                // 特攻固体値
  s16 spepow_eff;                // 特攻努力値
  s16 spedef_rnd;                // 特防固体値
  s16 spedef_eff;                // 特防努力値
} POKEWOOD_POKE_DATA;

// ------------------------------------------------------------
// コンバータから出力されるポケウッド用データ全体
// ------------------------------------------------------------
typedef struct{
  s16                party_num[2];  // 敵ポケモン数、自分ポケモン数の順
  POKEWOOD_POKE_DATA data[12];      // 格納数によって変化するため、必ず11まで参照できるとは限らない
} POKEWOOD_PARTIES_DATA;


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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const int trainerData[][2]={
  { TRTYPE_SCIENTISTW, TRTYPE_SCIENTISTW,   },     // 研究員
  { TRTYPE_LEADER7,    TRTYPE_PKWD_VILLAIN, },     // ハチクマン
  { TRTYPE_SCIENTISTW, TRTYPE_SCIENTISTW,   },     // 研究員
};

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//static void setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const POKEWOOD_MONS_DATA * mons_data, u32 max )
static void setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const s32 scenario_id, const s32 mode )
{
  //
  int i,j;
  u32 id = MyStatus_GetID( mystat );
  u8 sex = MyStatus_GetMySex( mystat );
  s16 *adr;
  s16 maxnum;
  s16 off;
  s16 aoff;

  POKEPARTY * add_party = PokeParty_AllocPartyWork( heapID );
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  POKEWOOD_PARTIES_DATA *ppd;

  // 読み込み
  adr = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD_PARTY, NARC_pokewood_party_pkwd_party01_bin+scenario_id, heapID );
  ppd = (POKEWOOD_PARTIES_DATA*)adr;
  // 最大数
  maxnum = ppd->party_num[mode];
  PokeParty_Init( add_party, maxnum );
  //
  // オフセット
  // (敵の場合はオフセットに注意)
  off = 0;
  if(mode == 1){
    off = ppd->party_num[0];
  }
  //
  // 情報をセット
  for ( i = 0; i < maxnum; i ++ )
  {
    POKEWOOD_POKE_DATA *poke = &(ppd->data[off+i]);
    u64 rnd;
    OS_Printf("%d: monsno=%d, form=%d, sex=%d, tokusei=%d item=%d\n", i, poke->rec_monsno, poke->form, poke->sex, 
                                                              poke->tokusei, poke->item);

    //
    PP_Clear( pp );
    rnd = POKETOOL_CalcPersonalRandEx( id, poke->rec_monsno, poke->form, poke->sex, poke->tokusei, 0 );
    PP_SetupEx( pp, poke->rec_monsno, poke->level, id, PTL_SETUP_POW_AUTO, rnd );
    POKE_MEMO_SetTrainerMemoPP( pp , POKE_MEMO_SET_CAPTURE , mystat, MAPNAME_T1KANOKO , heapID );
    for(j=0;j<4;j++){
      PP_SetWazaPos( pp, poke->waza[j], j );
    }
    PP_Put( pp, ID_PARA_seikaku,   poke->seikaku );
    PP_Put( pp, ID_PARA_item,      poke->item );
    PP_Put( pp, ID_PARA_hp_rnd,    poke->hp_rnd );
    PP_Put( pp, ID_PARA_hp_exp,    poke->hp_eff );
    PP_Put( pp, ID_PARA_pow_rnd,   poke->pow_rnd );
    PP_Put( pp, ID_PARA_pow_exp,   poke->pow_eff );
    PP_Put( pp, ID_PARA_def_rnd,   poke->def_rnd );
    PP_Put( pp, ID_PARA_def_exp,   poke->def_eff );
    PP_Put( pp, ID_PARA_agi_rnd,   poke->spd_rnd );
    PP_Put( pp, ID_PARA_agi_exp,   poke->spd_eff );
    PP_Put( pp, ID_PARA_spepow_rnd,poke->spepow_rnd );
    PP_Put( pp, ID_PARA_spepow_exp,poke->spepow_eff );
    PP_Put( pp, ID_PARA_spedef_rnd,poke->spedef_rnd );
    PP_Put( pp, ID_PARA_spedef_exp,poke->spedef_eff );
    PP_Renew( pp );
    PokeParty_Add( add_party, pp );
  }
  // 破棄
  GFL_HEAP_FreeMemory( adr );
  
  PokeParty_Copy( add_party, party );
  GFL_HEAP_FreeMemory( pp );
  GFL_HEAP_FreeMemory( add_party );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void POKEWOOD_EVENT_setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const s32 scenario_id, const s32 mode )
{
	setupPokemon( mystat, party, heapID, scenario_id, mode );
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
  PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );

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
//  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, enemyData[senario_no], enemyDataNum[senario_no] );
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, senario_no, 1 );

  //プレイヤーセット
  { 
    //MyStatus
    bsp->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gamedata );
    
    // プレイヤーパーティーの登録
//    setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, myData[senario_no], myDataNum[senario_no] );
    if(PKWDSYS_GetJoinMypoke(*pkwdsys)==TRUE){
      // 手持ちから出す
      PKWDSYS_CreateBattleParty( *pkwdsys, gamedata, bsp->party[BTL_CLIENT_PLAYER] );
    }else{
      // レンタルポケモン
      setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, senario_no, 0 );
    }
  }

  GFL_HEAP_FreeMemory( pp );
  bsp->PokewoodMode = BTL_POKEWOOD_REC;
  bsp->pokewoodSenario = senario_no; //仮
//  bsp->DebugFlagBit = 1<<BTL_DEBUGFLAG_SKIP_BTLIN;

  OS_Printf("create BSP pokewood senario = %d\n", bsp->pokewoodSenario);

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
    OS_Printf(" load pokewood player sex=%d\n ", MyStatus_GetMySex(bsp->playerStatus[0]));

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
    // 指定シナリオのプレイ済みフラグを立てる
    {
      MISC *misc = GAMEDATA_GetMiscWork(psew->gamedata);
      MISC_SetPokewoodPlayedFlag( misc, psew->bsp->pokewoodSenario);  // シナリオ撮影済みフラグを立てる
    }

    /**
     *  BATTLE_SETUP_PARAM の録画データセーブ開始
     */
    OS_Printf(" load pokewood player sex=%d\n ", MyStatus_GetMySex(psew->bsp->playerStatus[0]));
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
  param->sex      = MyStatus_GetMySex( bsp->playerStatus[0] );
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


enum{
  PW_SHOW_SEQ_START=0,
  PW_SHOW_SEQ_ENCOUNT_EFFECT,
  PW_SHOW_SEQ_FIELDMAP_EXIT,
  PW_SHOW_SEQ_TITLE_APP,
  PW_SHOW_SEQ_TITLE_APP_END,
  PW_SHOW_SEQ_CALL_BATTLE,
  PW_SHOW_SEQ_FREE_BSP,
  PW_SHOW_SEQ_FIELD_BGM_PLAY,
  PW_SHOW_SEQ_FIELD_OPEN,
  PW_SHOW_SEQ_FIELD_BGM_WAIT,
  PW_SHOW_SEQ_FADEIN,
  PW_SHOW_SEQ_END,
};
//-----------------------------------------------------------------------------
// ポケウッド撮影イベント
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodShowBattleEvent( GMEVENT * event, int *seq, void * work )
{
  PBEW * pbew = (PBEW*)work;
  PREW * prew = pbew->prew;

  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case PW_SHOW_SEQ_START:
    // BATTLE_SETUP_PARAM生成
    pbew->bp = createPokewoodBSP( prew, gsys, fieldmap, prew->heapID );
    // プレイヤーボリュームをフェードアウト
    FIELD_SOUND_ChangePlayerVolume( fsnd, 0, 20 );  
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, pbew->bp->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case PW_SHOW_SEQ_ENCOUNT_EFFECT:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, prew->EncEffNo);
    (*seq)++;
    break;

  // フィールドマップ終了
  case PW_SHOW_SEQ_FIELDMAP_EXIT:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;
  // 撮影ならバトルへ、上映なら事前アプリの後バトルへ
  case PW_SHOW_SEQ_TITLE_APP:
    _init_pkwd_count_param( &prew->pkwd_count_param, pbew->bp );
    GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_announce), &PkwdCountProcData, &prew->pkwd_count_param );
    (*seq)++;
    break;

  // タイトルアプリ終了待ち
  case PW_SHOW_SEQ_TITLE_APP_END:
    _delete_pkwd_count_param( &prew->pkwd_count_param );
    (*seq)++;
    break;
  // バトル呼び出し
  case PW_SHOW_SEQ_CALL_BATTLE:
    // BGM切り替え
    FIELD_SOUND_ChangePlayerVolume( fsnd, MAX_VOLUME, 1 );  
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, pbew->bp );
    (*seq)++;
    break;

  // 呼び出しパラメーター解放
  case PW_SHOW_SEQ_FREE_BSP:
    deletePokewoodBSP( pbew->bp );
    (*seq) ++;
    break;

  // フィールドBGMを復帰
  case PW_SHOW_SEQ_FIELD_BGM_PLAY:
    GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle( gsys ) );
    (*seq) ++;
    break;

  case PW_SHOW_SEQ_FIELD_OPEN: 
    //勝ち負け判定
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;

  // BGMの復帰待ち
  case PW_SHOW_SEQ_FIELD_BGM_WAIT:
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( gsys ) );
    (*seq) ++;
    break;

  // フェードイン処理
  case PW_SHOW_SEQ_FADEIN:
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    (*seq) ++;
    break;

  case PW_SHOW_SEQ_END:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

enum{
  PW_REC_SEQ_START=0,
  PW_REC_SEQ_ENCOUNT_EFFECT,
  PW_REC_SEQ_FIELDMAP_EXIT,
  PW_REC_SEQ_CALL_BATTLE,
  PW_REC_SEQ_SAVE,
  PW_REC_SEQ_FREE_BSP,
  PW_REC_SEQ_FIELD_BGM_PLAY,
  PW_REC_SEQ_FIELD_OPEN,
  PW_REC_SEQ_FIELD_BGM_WAIT,
  PW_REC_SEQ_FADEIN,
  PW_REC_SEQ_END,
};

//-----------------------------------------------------------------------------
// ポケウッド上映イベント
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodRecBattleEvent( GMEVENT * event, int *seq, void * work )
{
  PBEW * pbew = (PBEW*)work;
  PREW * prew = pbew->prew;

  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case PW_REC_SEQ_START:
    // BATTLE_SETUP_PARAM生成
    pbew->bp = createPokewoodBSP( prew, gsys, fieldmap, prew->heapID );
    // プレイヤーボリュームをフェードアウト
    FIELD_SOUND_ChangePlayerVolume( fsnd, 0, 20 );  
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, pbew->bp->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case PW_REC_SEQ_ENCOUNT_EFFECT:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, prew->EncEffNo);
    (*seq)++;
    break;

  // フィールドマップ終了
  case PW_REC_SEQ_FIELDMAP_EXIT:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;
  // バトル呼び出し
  case PW_REC_SEQ_CALL_BATTLE:
    // BGM切り替え
    FIELD_SOUND_ChangePlayerVolume( fsnd, MAX_VOLUME, 1 );  
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, pbew->bp );
    (*seq)++;
    break;
  // 撮影時はセーブ
  case PW_REC_SEQ_SAVE:
    if(pbew->bp->result!=BTL_RESULT_RUN)
    { // 逃げてなかったらセーブする
      u8 bufID = prew->rec_id;
      GMEVENT * new_event = createPokewoodSaveEvent( gsys, bufID, pbew->bp, GFL_HEAPID_APP );
      GMEVENT_CallEvent( event, new_event );
    }
    (*seq)++;
    break;

  // バトル呼び出しパラメータを解放
  case PW_REC_SEQ_FREE_BSP:
    deletePokewoodBSP( pbew->bp );
    //侵入システムにフィールド中であることを伝える
    (*seq) ++;
    break;

  case PW_REC_SEQ_FIELD_BGM_PLAY:
    // フィールドBGMを復帰
    GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle( gsys ) );
    (*seq) ++;
    break;

  case PW_REC_SEQ_FIELD_OPEN: 
    //勝ち負け判定
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;

  // BGMの復帰待ち
  case PW_REC_SEQ_FIELD_BGM_WAIT:
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( gsys ) );
    (*seq) ++;
    break;

  case PW_REC_SEQ_FADEIN:
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    (*seq) ++;
    break;

  case PW_REC_SEQ_END:
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
  GMEVENT * event;
  PBEW * pbew;

  // 撮影・上映で別イベントを起動
  if(  prew->PokewoodMode==BTL_POKEWOOD_REC ){
    event = GMEVENT_Create( gsys, NULL, pokewoodRecBattleEvent,  sizeof(PBEW) );
  }else{
    event = GMEVENT_Create( gsys, NULL, pokewoodShowBattleEvent, sizeof(PBEW) );
  }
  pbew = GMEVENT_GetEventWork( event );
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
GMEVENT * POKEWOOD_EVENT_Rec( GAMESYS_WORK *gsys, void * param )
{
  GMEVENT * event;
  PREW * prew;
  const PKWD_REC_EVENT_PARAM * rec_param = (const PKWD_REC_EVENT_PARAM *)param;
  //POKEWOOD_WORK * pokewood_work = (POKEWOOD_WORK*)parent_work;

  GF_ASSERT( rec_param->rec_pos < PKWD_REC_POS_MAX );
  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys      = gsys;
  prew->rec_id    = rec_param->rec_pos; // 録画NO
  prew->SenarioNo = rec_param->scenario_no;
  prew->heapID    = HEAPID_PROC;
  OS_Printf(" POKEWOOD EVENT rec_id = %x, senario = %d\n", rec_param->rec_pos, rec_param->scenario_no );
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
  prew->rec_id = rec_id; //録画NO
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

//============================================================================================
//
//    PKWD_BOOK画面呼び出し
//
//============================================================================================
//-------------------------------------
///   PKWD_BOOK_PARAM定義
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  PKWD_BOOK_PARAM    book_param;
} PKWD_BOOK_EVENT_WORK;

static GMEVENT_RESULT eventPkwdBook( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PKWD_BOOK呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_BOOK_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_BOOK_PARAM *param = (PKWD_BOOK_PARAM *)parent_work;
  PKWD_BOOK_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdBook, sizeof(PKWD_BOOK_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_BOOK_EVENT_WORK) );

  p_param->gsys      = gsys;
  p_param->fieldmap  = fieldmap;
  p_param->book_param = *param;

  OS_Printf( "PKWD BOOK Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdBook呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdBook( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_BOOK_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_book ), &PkwdBookProcData, &p_wk->book_param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    OS_Printf( "p_wk = %08x\n", p_wk );
    if ( p_wk->book_param.result != NULL && *(p_wk->book_param.result) == PKWD_BOOK_NOSELECT )
    {
      *(p_wk->book_param.result) = POKEWOOD_SCENEARIO_MAX;
    }else{
      GAMEDATA * gamedata = GAMESYSTEM_GetGameData( p_wk->gsys );
      PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );
      PKWDSYS_SetScenarioNo( *pkwdsys, *(p_wk->book_param.result) );
    }
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//============================================================================================
//
//    ポケウッド上映選択画面呼び出し
//
//============================================================================================
//-------------------------------------
///   PKWD_SELECT_SCREEN_EVENT_PARAM定義
//=====================================
typedef struct
{
  GAMESYS_WORK       *gsys;
  FIELDMAP_WORK      *fieldmap;
  PKWD_SELECT_SCREEN_PARAM    param;
} PKWD_SELECT_SCREEN_EVENT_WORK;

static GMEVENT_RESULT eventPkwdSelectScreen( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PKWD_BOOK呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_SELECT_SCREEN_EVENT_Call( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_SELECT_SCREEN_PARAM *param = (PKWD_SELECT_SCREEN_PARAM *)parent_work;
  PKWD_SELECT_SCREEN_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イベント作成
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdSelectScreen, sizeof(PKWD_SELECT_SCREEN_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_SELECT_SCREEN_EVENT_WORK) );

  p_param->gsys       = gsys;
  p_param->fieldmap   = fieldmap;
  p_param->param      = *param;

  OS_Printf( "PKWD SELECT START Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdSelectScreen呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdSelectScreen( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_SELECT_SCREEN_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_selectscreen ), &PkwdSelectScreenProcData, &p_wk->param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    OS_Printf( "p_wk = %08x\n", p_wk );
    if ( p_wk->param.result != NULL && *(p_wk->param.result) == -1 )
    {
      *(p_wk->param.result) = POKEWOOD_SAVEDATA_MAX;
    }else{
      GAMEDATA * gamedata = GAMESYSTEM_GetGameData( p_wk->gsys );
      PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );
      PKWDSYS_SetPlayNo( *pkwdsys, *(p_wk->param.result) );
    }
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//============================================================================================
//  PKWD　イベント　ポケモンリスト
//============================================================================================
//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  PKWDSYS * pkwdsys;
  
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
    PKWDSYS_SetPokelistResult( work->pkwdsys, &work->list_data );
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
GMEVENT * PKWD_EVENT_SetSelectPokeList( GAMESYS_WORK * gsys, void * parent_work )
{
  GMEVENT   *event;
  GAMEDATA  *gamedata;
  PKWDSYS   *pkwdsys;
  POKEPARTY *pp;
  FIELDMAP_WORK        *fieldmap;
  EVENT_WORK_POKE_LIST *work;
  
  pkwdsys   = (PKWDSYS *)parent_work;
  gamedata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  
  event = GMEVENT_Create( gsys, NULL,
      ev_SelectPokeList, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->pkwdsys = pkwdsys;
  
  PKWDSYS_SetupPokeListData( pkwdsys, gamedata, &work->list_data );
  
  PKWDSYS_SetupPokeStatusData( pkwdsys, gamedata, &work->status_data );
   
  return( event );
}
