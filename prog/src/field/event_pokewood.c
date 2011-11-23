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
#include "app/pkwd_briefing.h"
#include "app/pkwd_record_monitor.h"
#include "field_sound_system.h"
#include "message.naix"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"
#include "tr_tool/trtype_def.h"

#include "savedata/save_tbl.h"
#include "savedata/pokewood_savedata.h"

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
#include "scrcmd_pokewood.h"

#include "waza_tool/wazano_def.h"
#include "item/itemsym.h"
#include "savedata/pokewood_rec.h"
#include "msg/msg_pokewood_name.h"
#include "../battle/pokewood/pokewood_senario.h"
#include "pkwd_scenario_open.h"
#include "pokewood_setup.h"
#include "poke_tool/pokewood_party.h"

#include "pokewood_party.naix"
#include "gamesystem/game_beacon.h"

#include "field_fade.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"


#ifdef PM_DEBUG
static BOOL s_pokewood_cutin_check_flag = FALSE;
#endif  

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static void dataMakePokewoodShowEnenyTrainer( BATTLE_SETUP_PARAM * bsp, HEAPID heapID );
static void saveDataConvertRecToShow( BATTLE_SETUP_PARAM* bsp, HEAPID heapID );

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
  PKWD_COUNT_PARAM    pkwd_count_param;
  PKWD_BRIEFING_PARAM pkwd_briefing_param;
  PKWD_BRIEFING_PARAM pkwd_endroll_param;
  STRBUF              *TrainerTypeName;
//  int musicDefault;
}POKEWOOD_REPLAY_EVENT_WORK, PREW;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  BATTLE_SETUP_PARAM * bp;
  PREW * prew;
}POKEWOOD_BATTLE_EVENT_WORK, PBEW;


//-----------------------------------------------------------------------------
// define
//-----------------------------------------------------------------------------

/// チュートリアル再生を行わせるための録画ID
#define POKEWOOD_TUTORIAL_MOVIE_NO  ( 8 )   

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//定義した値がずれないようにしている
SDK_COMPILER_ASSERT( PKWD_REC_POS_00 == LOADDATA_POKEWOOD0 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_01 == LOADDATA_POKEWOOD1 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_02 == LOADDATA_POKEWOOD2 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_03 == LOADDATA_POKEWOOD3 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_04 == LOADDATA_POKEWOOD4 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_05 == LOADDATA_POKEWOOD5 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_06 == LOADDATA_POKEWOOD6 );
SDK_COMPILER_ASSERT( PKWD_REC_POS_07 == LOADDATA_POKEWOOD7 );
//SDK_COMPILER_ASSERT( PKWD_REC_POS_MAX == LOADDATA_POKEWOOD_MAX );

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
 * @brief	ポケウッドモードと性別から敵トレーナー名IDのスタート位置を帰す
 *
 * @param   pokewood		BTL_POKEWOOD_REC or BTL_POKEWOOD_SHOW
 * @param   sex		      PM_MALE or PM_FEMALE
 *
 * @retval  int		      pw_rec_trainername_01等( pokewood_name.mstxtのID )
 */
//----------------------------------------------------------------------------------
static int _get_trainername_id( int pokewood, int sex )
{
  if(      pokewood==BTL_POKEWOOD_REC  && sex==PM_MALE   ){
      return pw_rec_trainername_01;
  }else if(pokewood==BTL_POKEWOOD_REC  && sex==PM_FEMALE ){
    return pw_rec_trainername_f_01;
  }else if(pokewood==BTL_POKEWOOD_SHOW && sex==PM_MALE   ){
    return pw_show_trainername_01;
  }else if(pokewood==BTL_POKEWOOD_SHOW && sex==PM_FEMALE ){
    return pw_show_trainername_f_01;
  }
  GF_ASSERT_MSG( 0,"ポケウッドモードか性別指定がおかしい pokewood=%d, sex=%d\n", pokewood, sex );

  return pw_rec_trainername_01;
}
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
static void dataMakeEncountTrainer( int senario_no, int PokewoodType, BSP_TRAINER_DATA* data, const MYSTATUS *my, HEAPID heapID )
{
  POKEWOOD_SENARIO *senario = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD, senario_no, GetHeapLowID(heapID));
  int sex = MyStatus_GetMySex(my);

  data->tr_id   = TRID_NULL;
  data->ai_bit  = 0;

  //=====================================================================
  // 敵トレーナーの設定
  //=====================================================================

  // 撮影と上映、自分が男か女かで敵トレーナーの見た目が変わる
  if(PokewoodType==BTL_POKEWOOD_REC){
    data->tr_type = senario->trtype[sex*2];
  }else if(PokewoodType==BTL_POKEWOOD_SHOW){
    data->tr_type = senario->trtype[sex*2+1];
  }

  OS_Printf("===================================================================\n");
  OS_Printf("senario_no = %d enemy tr_type = %d\n", senario_no, data->tr_type);
  OS_Printf("===================================================================\n");

  // トレーナー名を格納
  {
    GFL_MSGDATA* man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pokewood_name_dat, heapID );
    int trainername_id = _get_trainername_id( PokewoodType, sex );
    GFL_MSG_GetString( man, trainername_id+senario_no, data->name );
    GFL_MSG_Delete(man);
  }
  PMSDAT_Clear( &data->win_word );
  PMSDAT_Clear( &data->lose_word );

  GFL_HEAP_FreeMemory( senario );
}


static void dataMakeMyTrainer( int senario_no, BSP_TRAINER_DATA *data, const MYSTATUS *my, HEAPID heapID )
{
  POKEWOOD_SENARIO *senario = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD, senario_no, GetHeapLowID(heapID));
  int sex = MyStatus_GetMySex(my);

  //=====================================================================
  // 自分トレーナーの設定
  //=====================================================================
  data->tr_id  = 1;
  data->ai_bit = 0;
  if(sex==PM_MALE){
    data->tr_type = senario->mytype[1];
  }else if(sex==PM_FEMALE){
    data->tr_type = senario->mytype[1]+1;
  }
  GFL_HEAP_FreeMemory( senario );

}

// スターランクで入る観客数[基本数,乱数範囲]   [5,5]は 5+rand()%5 の意味
static const audience_tbl[][2]={ {5,5}, {7,6}, {10,9}, {16,7}, {22,1} };

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド録画用セットアップ
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodRecBattleParam( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, int senario_no )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  BATTLE_SETUP_PARAM * bsp = BATTLE_PARAM_Create( heapID );
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
  const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );
  PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );

  int i;

  //POKEWOOD_REC_Init( heapID );
  {
    BTL_FIELD_SITUATION sit;
    BTL_FIELD_SITUATION_Init( &sit );
    BTL_SETUP_Single_Trainer( bsp, gamedata, &sit, TRID_NULL, heapID );
    BTL_SETUP_AllocRecBuffer( bsp, heapID );
  }
  dataMakeEncountTrainer( senario_no, BTL_POKEWOOD_REC, bsp->tr_data[BTL_CLIENT_ENEMY1], mystatus, heapID);
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, senario_no, 1 );

  //プレイヤーセット
  { 
    //MyStatus
    bsp->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gamedata );
    
    // プレイヤーパーティーの登録
    if(PKWDSYS_GetJoinMypoke(*pkwdsys)==TRUE){
      // 手持ちから出す
      PKWDSYS_CreateBattleParty( *pkwdsys, gamedata, bsp->party[BTL_CLIENT_PLAYER] );
      bsp->pokewoodRentalFlag = FALSE;
    }else{
      // レンタルポケモン
      setupPokemon( mystatus, bsp->party[BTL_CLIENT_PLAYER], heapID, senario_no, 0 );
      bsp->pokewoodRentalFlag = TRUE;
    }
  }

  GFL_HEAP_FreeMemory( pp );
  bsp->PokewoodMode = BTL_POKEWOOD_REC;
  bsp->pokewoodSenario = senario_no; //仮
//  bsp->DebugFlagBit = 1<<BTL_DEBUGFLAG_SKIP_BTLIN;

  // 観客数を設定
  {
    int star_rank = POKEWOOD_SAVEDATA_CalcStarPoint2StarRank(POKEWOOD_SAVEDATA_GetStarPoint(ps));
    bsp->audience_num     = audience_tbl[star_rank][0] + GFUser_GetPublicRand(audience_tbl[star_rank][1]);
    bsp->player_star_rank = star_rank;
  }
  OS_Printf("create BSP pokewood senario = %d\n", bsp->pokewoodSenario);

  TAMADA_Printf(" POKEWOOD BSP %08x\n", bsp );

#ifdef PM_DEBUG
  if( s_pokewood_cutin_check_flag )
  {
    s_pokewood_cutin_check_flag = FALSE;

    bsp->DebugFlagBit |= 1<<BTL_DEBUGFLAG_POKEWOOD_CUTIN;

    bsp->PokewoodMode = BTL_POKEWOOD_SHOW;
  }
#endif

  return bsp;
}


//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生用敵トレーナー・ポケモン情報セットアップ
 */
//-----------------------------------------------------------------------------
static void _setup_enemyData( GAMEDATA *gamedata, BATTLE_SETUP_PARAM *bsp, int senario_no, HEAPID heapID )
{
  const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );

  //敵ポケモンをROMから読みだす
  setupPokemon( mystatus, bsp->party[BTL_CLIENT_ENEMY1], heapID, senario_no, 1 );
  saveDataConvertRecToShow( bsp, heapID );

  //敵トレーナーもROMから読み出す
  dataMakePokewoodShowEnenyTrainer( bsp, heapID );
  
}
//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生用セットアップSUB(通常上映時のセットアップ）
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM *_createNormalShowBSP(
                           GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, u8 replay_id)
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  LOAD_RESULT result;
  int senario_no=0;

  OS_Printf("load id=%d\n", replay_id);
  //POKEWOOD_REC_Init( heapID );

  result = POKEWOOD_REC_Load( sv, heapID, replay_id );
  if ( result == LOAD_RESULT_OK )
  {
    BATTLE_SETUP_PARAM * bsp;
    bsp = BATTLE_PARAM_Create( heapID );
    BATTLE_PARAM_Init( bsp );
    BTL_SETUP_InitForRecordPlay( bsp, gamedata, heapID );
    BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
    senario_no  = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_SENARIO, NULL );
    BattleRec_RestoreSetupParam( bsp, BTLREC_RESOTRE_MODE_POKEWOOD, heapID );
    BattleRec_UnloadToolModule();
    OS_Printf(" load pokewood player sex=%d\n ", MyStatus_GetMySex(bsp->playerStatus[0]));
  
    // 敵トレーナー・ポケモン情報を格納
    _setup_enemyData( gamedata, bsp, senario_no, heapID );

    return bsp;
  }


  GF_ASSERT_MSG( 0,"再生データが読み込めていないのに上映しようとしている!!!\n" );
  return NULL;
}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッドチュートリアルムービー再生用セットアップSUB
 *        外部セーブデータを読み込まずに、ROM内のデータを再生します。
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM *_createTutorialShowBSP(
                           GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  LOAD_RESULT result;
  BATTLE_SETUP_PARAM * bsp;

  result = POKEWOOD_REC_Load_ROM( sv, heapID, 0 );

  // ROMから読み込むので、ロード結果は必ずLOAD_RESULT_OKになる
  bsp = BATTLE_PARAM_Create( heapID );
  BATTLE_PARAM_Init( bsp );
  BTL_SETUP_InitForRecordPlay( bsp, gamedata, heapID );
  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  BattleRec_RestoreSetupParam( bsp, BTLREC_RESOTRE_MODE_POKEWOOD, heapID );
  BattleRec_UnloadToolModule();

  // 敵トレーナー・ポケモン情報を格納
  _setup_enemyData( gamedata, bsp, bsp->pokewoodSenario, heapID );
  OS_Printf("tutorial senario_no = %d\n", bsp->pokewoodSenario);

  return bsp;

}
//-----------------------------------------------------------------------------
/**
 * @brief ポケウッド再生用セットアップ
 *        この関数でscenario_noが代入されます
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodShowBattleParam( 
                            GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, u8 replay_id )
{
  BATTLE_SETUP_PARAM *bsp;
  // チュートリアル再生用のBSP作成
  if(replay_id==POKEWOOD_TUTORIAL_MOVIE_NO){
    return _createTutorialShowBSP( gsys, fieldmap, heapID);
  }
  
  // 通常時BSP作成
  bsp = _createNormalShowBSP( gsys, fieldmap, heapID, replay_id );

  // すれちがいログで「ポケウッドで「●●●」を上映中」と表示
  GAMEBEACON_Set_PokewoodWatchMovie( bsp->pokewoodSenario );

  return bsp;

}

//-----------------------------------------------------------------------------
/**
 * @brief ポケウッドチュートリアルムービー再生用セットアップ
 *        外部セーブデータを読み込まずに、ROM内のデータを再生します。
 */
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodTutorialShowBattleParam(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, HEAPID heapID, u8 replay_id, int senario_no )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  LOAD_RESULT result;
  BATTLE_SETUP_PARAM * bsp;

  result = POKEWOOD_REC_Load_ROM( sv, heapID, replay_id );

  // ROMから読み込むので、ロード結果は必ずLOAD_RESULT_OKになる
  bsp = BATTLE_PARAM_Create( heapID );
  BATTLE_PARAM_Init( bsp );
  BTL_SETUP_InitForRecordPlay( bsp, gamedata, heapID );
  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  BattleRec_RestoreSetupParam( bsp, BTLREC_RESOTRE_MODE_POKEWOOD, heapID );
  BattleRec_UnloadToolModule();

  return bsp;
}


#include "msg/msg_pokewood_senario01_m.h"

//----------------------------------------------------------------------------------
/**
 * @brief	トレーナータイプの文字列をbspをセットする
 *
 * @param   prew    
 * @param   bsp     
 * @param   no      
 * @param   pw_type 
 * @param   heapID  
 */
//----------------------------------------------------------------------------------
static void setTrainerTypeName( PREW *prew, BATTLE_SETUP_PARAM *bsp, int no, int pw_type, HEAPID heapID )
{
  int sex    = MyStatus_GetMySex( bsp->playerStatus[BTL_CLIENT_PLAYER] );
  u8 sex_ofs = (sex == PM_MALE? 1: 0);
  int msgid  = NARC_message_pokewood_senario01_f_dat+no*2+sex_ofs;
  GFL_MSGDATA *msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msgid, heapID );
  OS_Printf("senario_no=%d, sex_ofs=%d, msgid=%d\n", no, sex_ofs, msgid);


  if(pw_type==BTL_POKEWOOD_REC){
    prew->TrainerTypeName = GFL_MSG_CreateString( msg, pw_trname_rec );
  }else if(pw_type==BTL_POKEWOOD_SHOW){
    prew->TrainerTypeName = GFL_MSG_CreateString( msg, pw_trname_show );
  }
  BATTLE_PARAM_SetTrainerTypeName( bsp, BTL_CLIENT_ENEMY1, prew->TrainerTypeName );

  GFL_MSG_Delete(msg);

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BATTLE_SETUP_PARAM * createPokewoodBSP(
    PREW * prew, GAMESYS_WORK * gsys, GAMEDATA *gamedata, FIELDMAP_WORK * fieldmap, HEAPID heapID )
{
  BATTLE_SETUP_PARAM * bsp;
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);


  switch ( prew->PokewoodMode )
  {
  case BTL_POKEWOOD_NONE:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case BTL_POKEWOOD_REC:
    bsp = createPokewoodRecBattleParam( gsys, fieldmap, heapID, prew->SenarioNo );
    bsp->musicDefault = SEQ_BGM_PW_FILMING;
    bsp->musicWin     = SEQ_BGM_WIN1;
    setTrainerTypeName( prew, bsp, prew->SenarioNo, BTL_POKEWOOD_REC, heapID );
    break;
  case BTL_POKEWOOD_SHOW:
    // 録画データ読み込み、BGM設定、敵トレーナータイプ名設定
    {
      POKEWOOD_SENARIO *senario;
      bsp = createPokewoodShowBattleParam( gsys, fieldmap, heapID, prew->rec_id );
      senario = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD, bsp->pokewoodSenario, GetHeapLowID(heapID));
      bsp->musicDefault = senario->battle_bgm;
      bsp->musicWin     = senario->win_bgm;
      bsp->musicLose    = senario->lose_bgm;
      bsp->musicStrange = senario->strange_bgm;
      setTrainerTypeName( prew, bsp, bsp->pokewoodSenario, BTL_POKEWOOD_SHOW, heapID );
      GFL_HEAP_FreeMemory( senario );
    }
    
    // 上映キャンセル可能にするか?
    if(bsp->pokewoodSenario<POKEWOOD_PLAY_MAX){   //チュートリアル上映は常にキャンセル禁止
      // この録画データは一度上映しているか？
      if(POKEWOOD_SAVEDATA_GetMovieShowed( ps, prew->rec_id )==0){
        OS_Printf("上映してない id=%d\n", prew->rec_id);
        bsp->PokewoodShowNoCancelFlag = TRUE;
      }else{
        OS_Printf("上映済み id=%d\n", prew->rec_id);
      }
    }
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
  //POKEWOOD_REC_Exit();
  BATTLE_PARAM_Delete( bsp );
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brief 録画データを上映用に書き直す
 */
//-----------------------------------------------------------------------------
static void rewritePokewoodRec( GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp, HEAPID heapID )
{
  PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );

  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意

  //敵トレーナー情報はROMから読み出しセーブしないことにしたので、
  //ここの処理はまるごといらない
#if 0
  // BATTLE_SETUP_PARAMを上映用のトレーナーデータに書き換える
  dataMakeEncountTrainer( bsp->pokewoodSenario, BTL_POKEWOOD_SHOW, bsp->tr_data[BTL_CLIENT_ENEMY1], 
                          bsp->playerStatus[0], heapID );
  bsp->tr_data[BTL_CLIENT_ENEMY1]->tr_id   = 1;
#endif
  bsp->tr_data[BTL_CLIENT_PLAYER] = BSP_TRAINER_DATA_Create( heapID );
  dataMakeMyTrainer( bsp->pokewoodSenario, bsp->tr_data[BTL_CLIENT_PLAYER], bsp->playerStatus[0], heapID );
  
  BattleRec_StoreSetupParam( bsp );
//  BattleRec_SetPokewoodRentalFlag( PKWDSYS_GetJoinMypoke(*pkwdsys)^1 );   //レンタルポケモンフラグを反映させる
  BattleRec_UnloadToolModule();
}

//----------------------------------------------------------------------------------
/**
 * @brief	上映前アプリパラメータ作成
 *
 * @param   param		
 * @param   bsp		
 */
//----------------------------------------------------------------------------------
static void _init_pkwd_count_param( PKWD_COUNT_PARAM *param, BATTLE_SETUP_PARAM *bsp )
{
  STRBUF *str       = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_PROC );
  POKEMON_PARAM *pp = GFL_HEAP_AllocMemory( HEAPID_PROC, POKETOOL_GetWorkSize() );

  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  
  // 録画データから手持ちポケモンの名前を取得する
  POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_POKE0, pp );
  {
    int monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    GFL_MSGDATA *monsname_msg =  GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_monsname_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
    GFL_MSG_GetString( monsname_msg, monsno, str );
    GFL_MSG_Delete( monsname_msg );
  }
  param->message = str;
  BattleRec_UnloadToolModule();
  GFL_HEAP_FreeMemory( pp );
  
  // 現在はシナリオNOとタイトル画像指定番号を同じにする
//  param->title_no = bsp->pokewoodSenario;
  param->scenario_no = bsp->pokewoodSenario;
  param->sex      = MyStatus_GetMySex( bsp->playerStatus[0] );

  OS_Printf("レンタルポケモンか？=%d\n", POKEWOOD_REC_GetData(POKEWOOD_REC_PARAM_RENTAL_FLAG,NULL));
}

//----------------------------------------------------------------------------------
/**
 * @brief	上映前アプリ後始末
 *
 * @param   param		
 */
//----------------------------------------------------------------------------------
static void _delete_pkwd_count_param( PKWD_COUNT_PARAM *param )
{
  // 取得しポケモン名文字列の解放
  GFL_STR_DeleteBuffer( param->message );
}

//----------------------------------------------------------------------------------
/**
 * @brief	あらすじアプリパラメータ作成
 *
 * @param   param		
 * @param   bsp		
 */
//----------------------------------------------------------------------------------
static void _init_pkwd_briefing_param( PKWD_BRIEFING_PARAM *param, BATTLE_SETUP_PARAM *bsp, MYSTATUS *mystatus, GAMEDATA *gamedata )
{
  param->scenario_no  = bsp->pokewoodSenario;
  param->audienceNum  = bsp->audience_num;
  param->mystatus     = mystatus;
  param->bsp          = bsp;
}

//----------------------------------------------------------------------------------
/**
 * @brief	エンドロールアプリパラメータ作成
 *
 * @param   param		
 * @param   bsp		
 * @param   mystatus		
 * @param   gamedata		
 */
//----------------------------------------------------------------------------------
static void _init_pkwd_endroll_param( PKWD_BRIEFING_PARAM *param, BATTLE_SETUP_PARAM *bsp, MYSTATUS *mystatus, GAMEDATA *gamedata )
{
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
	
	// 初期化
  param->sv          = sv;
  param->gameData    = gamedata;
  param->mystatus    = mystatus;
  param->audienceNum = bsp->audience_num;
  param->audienceBit = bsp->audience_bit;
  param->bsp         = bsp;
  param->scenario_no = bsp->pokewoodSenario;
}

//----------------------------------------------------------------------------------
/**
 * @brief	上映前アプリ後始末
 *
 * @param   param		
 */
//----------------------------------------------------------------------------------
static void _delete_pkwd_briefing_param( PKWD_BRIEFING_PARAM *param )
{
  // 現状なにもなし
  return;
}

//============================================================================================
//============================================================================================


enum{
  PW_SHOW_SEQ_START=0,
  PW_SHOW_SEQ_ENCOUNT_EFFECT,
  PW_SHOW_SEQ_FIELDMAP_EXIT,
  PW_SHOW_SEQ_TITLE_APP,
  PW_SHOW_SEQ_TITLE_APP_END,
  PW_SHOW_SEQ_BRIEFING_APP,
  PW_SHOW_SEQ_BRIEFING_APP_END,
  PW_SHOW_SEQ_CALL_BATTLE,
  PW_SHOW_SEQ_CALL_ENDROLL,
  PW_SHOW_SEQ_FIELD_BGM_PLAY,
  PW_SHOW_SEQ_FIELD_OPEN,
  PW_SHOW_SEQ_FIELD_BGM_WAIT,
  PW_SHOW_SEQ_FADEIN,
  PW_SHOW_SEQ_FREE_BSP,
  PW_SHOW_SEQ_END,
};


FS_EXTERN_OVERLAY( scrcmd_pokewood );
//-----------------------------------------------------------------------------
// ポケウッド上映イベント
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodShowBattleEvent( GMEVENT * event, int *seq, void * work )
{
  PBEW * pbew = (PBEW*)work;
  PREW * prew = pbew->prew;

  GAMESYS_WORK * gsys = prew->gsys;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );
  MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);

  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case PW_SHOW_SEQ_START:
    // BATTLE_SETUP_PARAM生成
    pbew->bp = createPokewoodBSP( prew, gsys, gamedata, fieldmap, prew->heapID );
    // プレイヤーボリュームをフェードアウト
    FIELD_SOUND_ChangePlayerVolume( fsnd, 0, 20 );  
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, pbew->bp->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case PW_SHOW_SEQ_ENCOUNT_EFFECT:
    //GMEVENT_CallEvent(event, EVENT_FieldFadeOut_Black(gsys,fieldmap,FIELD_FADE_WAIT) );
    //ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, prew->EncEffNo);
    (*seq)++;
    break;

  // フィールドマップ終了
  case PW_SHOW_SEQ_FIELDMAP_EXIT:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;
  // 事前アプリ実行
  case PW_SHOW_SEQ_TITLE_APP:
#ifdef PM_DEBUG
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
      (*seq) = PW_SHOW_SEQ_CALL_BATTLE;
    }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
      (*seq) = PW_SHOW_SEQ_BRIEFING_APP;
    }else{
      _init_pkwd_count_param( &prew->pkwd_count_param, pbew->bp );
      GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_announce), &PkwdCountProcData, &prew->pkwd_count_param );
      (*seq)++;
    }
#else
    _init_pkwd_count_param( &prew->pkwd_count_param, pbew->bp );
    GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_announce), &PkwdCountProcData, &prew->pkwd_count_param );
    (*seq)++;
#endif
    break;
  // タイトルアプリ終了待ち
  case PW_SHOW_SEQ_TITLE_APP_END:
    _delete_pkwd_count_param( &prew->pkwd_count_param );
    (*seq)++;
    break;
  // あらすじアプリ実行
  case PW_SHOW_SEQ_BRIEFING_APP:
    _init_pkwd_briefing_param( &prew->pkwd_briefing_param, pbew->bp, mystatus, gamedata );
    GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_briefing), &PkwdBriefingProcData, &prew->pkwd_briefing_param );
    (*seq)++;
    break;
  // あらすじアプリ終了待ち
  case PW_SHOW_SEQ_BRIEFING_APP_END:
    pbew->bp->audience_bit = prew->pkwd_briefing_param.audienceBit; // あらすじで設置した観客配置を引き継ぐ
    _delete_pkwd_briefing_param( &prew->pkwd_briefing_param );
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
  // エンドロール呼び出し
  case PW_SHOW_SEQ_CALL_ENDROLL:
    OS_Printf("キャンセルした？ = %d\n", pbew->bp->recPlayCancel);
    // 上映をキャンセルしていなかったらエンドロール
    if(pbew->bp->recPlayCancel==FALSE){
      _init_pkwd_endroll_param( &prew->pkwd_endroll_param, pbew->bp, mystatus, gamedata );
      GMEVENT_CallProc( event, FS_OVERLAY_ID(pkwd_briefing), &PkwdEndrollProcData, &prew->pkwd_endroll_param );
    }
    (*seq)++;
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
    //GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    (*seq) ++;
    break;

  // 呼び出しパラメーター解放
  case PW_SHOW_SEQ_FREE_BSP:
    // バトルで使用したトレーナータイプ名バッファを削除
    GFL_STR_DeleteBuffer( prew->TrainerTypeName );

    // チュートリアル上映でなければ評価・キャンセル計算
    if(pbew->bp->pokewoodSenario<POKEWOOD_PLAY_MAX)
    { //途中でキャンセルしてないかをスクリプトに返す
      PKWDSYS * pkwdsys = *(GAMEDATA_GetPKWDSysPointer( gamedata ) );
      u16 * ret_wk = PKWDSYS_GetScriptAnswerWork( pkwdsys );
      if ( ret_wk != NULL )
      {
        *ret_wk = ( pbew->bp->result != BTL_RESULT_RUN );
        if(*ret_wk){
          PKWDSYS_CheckEnding( pkwdsys, gamedata, pbew->bp->pokewoodSenario, pbew->bp->pokewoodResult );
          // スターランク計算
          PKWDSYS_CalcStarPoint( gamedata, pkwdsys, pbew->bp->pokewoodResult, prew->rec_id );
          // 観劇フラグを立てる
          POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_THEATERGO, 
                                             pbew->bp->pokewoodSenario );
          // シナリオオープンチェック
          PKWD_OpenCheckScenario( ps, gamedata, pbew->bp->pokewoodSenario, 
                                  pbew->bp->pokewoodResult, pbew->bp->pokewoodProfit );
        }
      }
    }
    deletePokewoodBSP( pbew->bp );
    (*seq) ++;
    break;

  case PW_SHOW_SEQ_END:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
// ポケウッド撮影イベント
//-----------------------------------------------------------------------------
static GMEVENT_RESULT pokewoodRecBattleEvent( GMEVENT * event, int *seq, void * work )
{
  enum{
    PW_REC_SEQ_START=0,
    PW_REC_SEQ_ENCOUNT_EFFECT,
    PW_REC_SEQ_FIELDMAP_EXIT,
    PW_REC_SEQ_CALL_BATTLE,
    PW_REC_SEQ_SAVE,
    PW_REC_SEQ_FIELD_BGM_PLAY,
    PW_REC_SEQ_FIELD_OPEN,
    PW_REC_SEQ_FIELD_BGM_WAIT,
    PW_REC_SEQ_FADEIN,
    PW_REC_SEQ_FREE_BSP,
    PW_REC_SEQ_END,
  };

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
    pbew->bp = createPokewoodBSP( prew, gsys, gamedata, fieldmap, prew->heapID );
    // プレイヤーボリュームをフェードアウト
    FIELD_SOUND_ChangePlayerVolume( fsnd, 0, 20 );  
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, pbew->bp->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case PW_REC_SEQ_ENCOUNT_EFFECT:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, ENCEFFID_POKEWOOD);
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
  // セーブ
  case PW_REC_SEQ_SAVE:
    if(pbew->bp->result!=BTL_RESULT_RUN)
    { // 逃げてなかったらセーブする
      rewritePokewoodRec( gamedata, pbew->bp, prew->heapID );
    }
    (*seq)++;
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

  // バトル呼び出しパラメータを解放
  case PW_REC_SEQ_FREE_BSP:
    // バトルで使用したトレーナータイプ名バッファを削除
    GFL_STR_DeleteBuffer( prew->TrainerTypeName );
    { //途中でキャンセルしてないかをスクリプトに返す
      PKWDSYS * pkwdsys = *(GAMEDATA_GetPKWDSysPointer( gamedata ) );
      u16 * ret_wk = PKWDSYS_GetScriptAnswerWork( pkwdsys );
      if ( ret_wk != NULL )
      {
        *ret_wk = ( pbew->bp->result != BTL_RESULT_RUN );
      }
      // 興行収入データの作成
      PKWDSYS_MakeBoxofficeRecord( pkwdsys, pbew->bp, pbew->bp->pokewoodSenario );
    }
    deletePokewoodBSP( pbew->bp );
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
static GMEVENT * createPokewoodBattleEvent( GAMESYS_WORK * gsys, PREW * prew )
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
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  PKWDSYS * pkwdsys = *(GAMEDATA_GetPKWDSysPointer( gamedata ) );
  HEAPID heapID = prew->heapID;

  switch ( *seq )
  {
  case PRE_SEQ_START:
    POKEWOOD_REC_Init( prew->heapID );
     //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    //仮：本来はシナリオなどから反映する
    {
      GMEVENT * call_event;
      call_event = createPokewoodBattleEvent( gsys, prew );
      GMEVENT_CallEvent( event, call_event );
    }
    *seq = PRE_SEQ_END;
    break;

  case PRE_SEQ_END:
    //加工済みの録画データをPOKEWOODシステムに取っておく
    POKEWOOD_REC_Backup( PKWDSYS_GetBackupArea( pkwdsys ) );
    POKEWOOD_REC_Exit();
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
 * @brief ポケウッド撮影イベントの生成
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

  //GF_ASSERT( rec_param->rec_pos < PKWD_REC_POS_MAX );
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


//=============================================================================================
/**
 * @brief	チュートリアル上映
 *
 * @param   gsys		
 * @param   parent_work		
 *
 * @retval  GMEVENT *		
 */
//=============================================================================================
GMEVENT *POKEWOOD_EVENT_TutorialShow( GAMESYS_WORK *gsys, void * parent_work )
{
  GMEVENT * event;
  PREW * prew;

  event = GMEVENT_Create( gsys, NULL, pokewoodCallEvent, sizeof(PREW) );
  prew = GMEVENT_GetEventWork( event );
  prew->gsys = gsys;
  prew->heapID = HEAPID_PROC;
  prew->rec_id = POKEWOOD_TUTORIAL_MOVIE_NO; // チュートリアルムービー指定
  OS_Printf(" チュートリアルムービー再生開始 POKEWOOD EVENT Parent Value = %x\n", prew->rec_id);
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
 *  @brief  PKWD_SELECT_SCREEN呼び出し
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
    if ( p_wk->param.result != NULL && *(p_wk->param.result) == POKEWOOD_SAVEDATA_MAX )
    {
      *(p_wk->param.result) = POKEWOOD_SAVEDATA_MAX;
    }else{
      GAMEDATA * gamedata = GAMESYSTEM_GetGameData( p_wk->gsys );
//      PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gamedata );
//      PKWDSYS_SetPlayNo( *pkwdsys, *(p_wk->param.result) );
    }
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//============================================================================================
//
//    ポケウッドあらすじ画面呼び出し
//
//============================================================================================
//-------------------------------------
///   PKWD_BRIEFING_EVENT_PARAM定義
//=====================================
typedef struct
{
  GAMESYS_WORK       *gsys;
  FIELDMAP_WORK      *fieldmap;
  PKWD_BRIEFING_PARAM    param;
} PKWD_BRIEFING_EVENT_WORK;

static GMEVENT_RESULT eventPkwdBriefing( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdBriefing呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_BRIEFING_EVENT_Call( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_BRIEFING_PARAM *param = (PKWD_BRIEFING_PARAM *)parent_work;
  PKWD_BRIEFING_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イベント作成
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdBriefing, sizeof(PKWD_BRIEFING_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_BRIEFING_EVENT_WORK) );

  p_param->gsys       = gsys;
  p_param->fieldmap   = fieldmap;
  p_param->param      = *param;

  OS_Printf( "PKWD BRIEFING Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdBriefing呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdBriefing( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_BRIEFING_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_briefing ), &PkwdBriefingProcData, &p_wk->param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    OS_Printf( "Pkwd Briefing End\n");
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

static GMEVENT_RESULT eventPkwdEndroll( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdEndroll呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_ENDROLL_EVENT_Call( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_BRIEFING_PARAM *param = (PKWD_BRIEFING_PARAM *)parent_work;
  PKWD_BRIEFING_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イベント作成
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdEndroll, sizeof(PKWD_BRIEFING_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_BRIEFING_EVENT_WORK) );

  p_param->gsys       = gsys;
  p_param->fieldmap   = fieldmap;
  p_param->param      = *param;

  OS_Printf( "PKWD ENDROLL Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdEndroll呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdEndroll( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_BRIEFING_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_briefing ), &PkwdEndrollProcData, &p_wk->param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    OS_Printf( "Pkwd Endroll End\n");
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

//--------------------------------------------------------------
// シナリオオープン条件構造体
//--------------------------------------------------------------
static void _debug_open_print( POKEWOOD_SAVEDATA *ps, int no )
{
#ifdef PM_DEBUG
  if(POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, no )){
    OS_Printf("==========シナリオ%dがオープン===========\n",no);
  }
#endif
}

typedef struct{
  u8 open_provision;  // オープン条件
  u8 target;          // 条件で参照される値
} SCENARIO_PROVISION;

static const SCENARIO_PROVISION ScenarioProvisionTbl[]={
  { PW_OPEN_PROVISION_BEGINING,    },   // シナリオ1
  { PW_OPEN_PROVISION_DUAL,      0 },   // シナリオ2
  { PW_OPEN_PROVISION_SUCCESS,   1 },   // シナリオ3
  { PW_OPEN_PROVISION_PROFIT,      },   // シナリオ4
  { PW_OPEN_PROVISION_SUCCESS,   3 },   // シナリオ5
  { PW_OPEN_PROVISION_SUCCESS,   4 },   // シナリオ6
  { PW_OPEN_PROVISION_SUCCESS,   5 },   // シナリオ7
  { PW_OPEN_PROVISION_TUTORIAL,    },   // シナリオ8
  { PW_OPEN_PROVISION_SUCCESS,   7 },   // シナリオ9
  { PW_OPEN_PROVISION_SUCCESS,   8 },   // シナリオ10
  { PW_OPEN_PROVISION_SUCCESS,   9 },   // シナリオ11
  { PW_OPEN_PROVISION_GAMECLEAR,   },   // シナリオ12
  { PW_OPEN_PROVISION_SUCCESS,  11 },   // シナリオ13
  { PW_OPEN_PROVISION_SUCCESS,  12 },   // シナリオ14
  { PW_OPEN_PROVISION_SUCCESS,  13 },   // シナリオ15
  { PW_OPEN_PROVISION_TUTORIAL,    },   // シナリオ16
  { PW_OPEN_PROVISION_SUCCESS,  15 },   // シナリオ17
  { PW_OPEN_PROVISION_SUCCESS,  16 },   // シナリオ18
  { PW_OPEN_PROVISION_GAMECLEAR,17 },   // シナリオ19
  { PW_OPEN_PROVISION_SUCCESS,  18 },   // シナリオ20
  { PW_OPEN_PROVISION_SUCCESS,  19 },   // シナリオ21
  { PW_OPEN_PROVISION_TUTORIAL,    },   // シナリオ22
  { PW_OPEN_PROVISION_SUCCESS,  21 },   // シナリオ23
  { PW_OPEN_PROVISION_SUCCESS,  22 },   // シナリオ24
  { PW_OPEN_PROVISION_STARRANK,  2 },   // シナリオ25
  { PW_OPEN_PROVISION_SUCCESS,  24 },   // シナリオ26
  { PW_OPEN_PROVISION_SUCCESS,  25 },   // シナリオ27
  { PW_OPEN_PROVISION_SUCCESS,  26 },   // シナリオ28
  { PW_OPEN_PROVISION_TUTORIAL, 27 },   // シナリオ29
  { PW_OPEN_PROVISION_SUCCESS,  28 },   // シナリオ30
  { PW_OPEN_PROVISION_SUCCESS,  29 },   // シナリオ31
  { PW_OPEN_PROVISION_STARRANK,  3 },   // シナリオ32
  { PW_OPEN_PROVISION_SUCCESS,     },   // シナリオ33
  { PW_OPEN_PROVISION_SUCCESS,  31 },   // シナリオ34
  { PW_OPEN_PROVISION_SUCCESS,  32 },   // シナリオ35
  { PW_OPEN_PROVISION_GAMECLEAR,   },   // シナリオ36
  { PW_OPEN_PROVISION_SUCCESS,  34 },   // シナリオ37
  { PW_OPEN_PROVISION_SUCCESS,  35 },   // シナリオ38
  { PW_OPEN_PROVISION_SUCCESS,  36 },   // シナリオ39
  { PW_OPEN_PROVISION_ALLGOOD,     },   // シナリオ40
};

/// 最終シナリオがオープンするために必要な成功END取得数
#define PW_LAST_SENARIO_OPEN_NUM  ( 39 )  
//=============================================================================================
/**
 * @brief	シナリオが新しくオープンするかチェックする(関数内でオープンも行う）
 *
 * @param   ps		          POKEWOOD_SAVEDATA
 * @param   gamedata	      GAMEDATA
 * @param   scenario_no		  シナリオNO
 * @param   pokewoodResult	撮影終了時の状態
 * @param   profit		      興行収益
 */
//=============================================================================================
void PKWD_OpenCheckScenario( POKEWOOD_SAVEDATA *ps, GAMEDATA *gamedata, int scenario_no, int result, int profit )
{
  int i;
  OS_Printf("x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-\n");
  OS_Printf("scenario=%d, result=%d, profit=%d\n", scenario_no, result, profit );
  OS_Printf("x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-\n");
  
  
  // i番目のシナリオがまだオープンしていないのであればチェック
  for(i=0;i<POKEWOOD_PLAY_MAX;i++){

    if( POKEWOOD_SAVEDATA_GetPokewoodFlag(ps, PWSV_TYPE_SENARIO_OPEN, i)==0){
      switch(ScenarioProvisionTbl[i].open_provision){
      // 指定のシナリオを成功エンド
      case PW_OPEN_PROVISION_SUCCESS:
        if( scenario_no==ScenarioProvisionTbl[i].target && result==1 ){
          POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
          _debug_open_print( ps, i );
        }
        break;
      // 指定のスターランクに到達した
      case PW_OPEN_PROVISION_STARRANK:
        {
          int point = POKEWOOD_SAVEDATA_GetStarPoint( ps );
          int rank  = POKEWOOD_SAVEDATA_CalcStarPoint2StarRank(point);
          if( rank >= ScenarioProvisionTbl[i].target ){
            POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
            _debug_open_print( ps, i );
          }
        }
        break;

      // 指定の興行収入以上の成績を残した
      case PW_OPEN_PROVISION_PROFIT:
        if(profit>=POKEWOOD_PROVISION_PROFIT){
            POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
            _debug_open_print( ps, i );
        }
        break;
      // ゲームクリア
      case PW_OPEN_PROVISION_GAMECLEAR:
        {
          EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
          if(EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR)){
            POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
            _debug_open_print( ps, i );
          }
        }
        break;

      // 条件が２つある
      case PW_OPEN_PROVISION_DUAL:
        // 指定シナリオをグッドENDにした事がある上で
        if( POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, 
            PWSV_TYPE_SENARIO_SUCCESS, ScenarioProvisionTbl[i].target)){
          // 殿堂入りしている
          EVENTWORK *evwork = GAMEDATA_GetEventWork( gamedata );
          if(EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR)){
            POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
            _debug_open_print( ps, i );
          }
        }
        break;
      // ラストシナリオ以外で全てGOODエンドを取得している
      case PW_OPEN_PROVISION_ALLGOOD:
        {
          int j,num=0;
          // 全てのシナリオの成功END状況を取得する
          for(j=0;j<PW_LAST_SENARIO_OPEN_NUM;j++){
            num += POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, PWSV_TYPE_SENARIO_SUCCESS, j );
          }
          if(num==PW_LAST_SENARIO_OPEN_NUM){
            POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
            _debug_open_print( ps, i );
          }
        }
        break;
        
      // チュートリアルを終了している
      case PW_OPEN_PROVISION_TUTORIAL:
        if(POKEWOOD_SAVEDATA_GetPokewoodFlag(ps, PWSV_TYPE_SENARIO_THEATERGO, 0)){
          POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_OPEN, i );
          _debug_open_print( ps, i );
        }
        break;
      }
    }
  }
}

//=============================================================================================
/**
 * @brief	各シナリオがオープンされる条件を返す
 *
 * @param   scenario_no		
 *
 * @retval  int		オープン条件（pkwd_scenario_open.hに記述）
 */
//=============================================================================================
int  PKWD_GetScenarioOpenProvision( int scenario_no )
{
  return ScenarioProvisionTbl[scenario_no].open_provision;
}

//============================================================================================
//
//    PKWD_RECORD_MONITOR画面呼び出し
//
//============================================================================================
//-------------------------------------
///   PKWD_RECORD_MONITOR_PARAM定義
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK       *fieldmap;
  PKWD_RECORD_MONITOR_PARAM    param;
} PKWD_RECORD_MONITOR_EVENT_WORK;

static GMEVENT_RESULT eventPkwdRecordMonitor( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  PKWD_RECORD_MONITOR呼び出し
 *
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
GMEVENT * PKWD_RECORD_MONITOR_EVENT_Call( GAMESYS_WORK * gsys, void * parent_work )
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PKWD_RECORD_MONITOR_PARAM *param = (PKWD_RECORD_MONITOR_PARAM *)parent_work;
  PKWD_RECORD_MONITOR_EVENT_WORK *p_param;
  GMEVENT * new_event;

  //イヴェント
  new_event = GMEVENT_Create( gsys, NULL, eventPkwdRecordMonitor, sizeof(PKWD_RECORD_MONITOR_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( p_param, sizeof(PKWD_RECORD_MONITOR_EVENT_WORK) );

  p_param->gsys      = gsys;
  p_param->fieldmap  = fieldmap;
  p_param->param = *param;

  OS_Printf( "PKWD RECORD MONITOR Start\n" );

  return new_event;

}

//----------------------------------------------------------------------------
/**
 *  @brief  PkwdRecordMonitor呼び出し用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT eventPkwdRecordMonitor( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  PKWD_RECORD_MONITOR_EVENT_WORK *p_wk = p_wk_adrs;
  GMEVENT * call_event;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    call_event = EVENT_FieldSubProc( p_wk->gsys, p_wk->fieldmap,
        FS_OVERLAY_ID( pkwd_record_monitor ), &PkwdRecordMonitorProcData, &p_wk->param );
    GMEVENT_CallEvent( p_event, call_event );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    OS_Printf( "p_wk = %08x\n", p_wk );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}


//----------------------------------------------------------------------------
/**
 *	@brief  再生用に敵トレーナーをセットする
 *
 *	@param	BATTLE_SETUP_PARAM * bsp  バトルセットパラム
 *	@param	heapID 
 *
 *	@note   (bspのpokewoodSenarioとplayerStatus[BTL_CLIENT_PLAYER]が設定されていること)
 */
//-----------------------------------------------------------------------------
static void dataMakePokewoodShowEnenyTrainer( BATTLE_SETUP_PARAM * bsp, HEAPID heapID )
{
  // BATTLE_SETUP_PARAMを上映用のトレーナーデータに書き換える
  dataMakeEncountTrainer( bsp->pokewoodSenario, BTL_POKEWOOD_SHOW, bsp->tr_data[BTL_CLIENT_ENEMY1], 
                          bsp->playerStatus[BTL_CLIENT_PLAYER], heapID );
  bsp->tr_data[BTL_CLIENT_ENEMY1]->tr_id   = 1;
}


//----------------------------------------------------------------------------
/**
 *	@brief ポケウッドの撮影データを上映データに変換
 */
//-----------------------------------------------------------------------------
static void saveDataConvertRecToShow( BATTLE_SETUP_PARAM* bsp, HEAPID heapID )
{
  enum
  {
    POKEWOOD_CONV_DATA_POKE_NUM = 6,
    POKEWOOD_DATA_ENEMY_START = 6,
  };
  GFL_MSGDATA *msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                         NARC_message_pokewood_name_dat, 
                                         GFL_HEAP_LOWID(heapID) );
  POKEWOOD_PARTIES_DATA *ppd = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD_PARTY, 
                               NARC_pokewood_party_pkwd_party01_bin+bsp->pokewoodSenario, 
                               GFL_HEAP_LOWID(heapID) );

  //敵ポケモンの設定
  //置換するものがあったら変える
  {
    int i,j;

    POKEPARTY *p_party = bsp->party[BTL_CLIENT_ENEMY1];
    for( i = 0; i < PokeParty_GetPokeCount(p_party); i++ )
    {
      POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer(p_party, i );
      if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) == 0 )
      {
        continue;
      }
      // MONSNOを書き換え（主にボスポケモン）
      {
        POKEWOOD_POKE_DATA *poke = &ppd->data[ppd->party_num[0]+i];
        // 生成したポケモンパラメータが撮影ポケモンが一致していたら上映ポケモンに変更する
        if( poke->rec_monsno == PP_Get( p_pp, ID_PARA_monsno, NULL ) )
        {
          OS_Printf("%dを%dに書き換え\n",PP_Get( p_pp, ID_PARA_monsno, NULL ), poke->show_monsno);
          PP_Put( p_pp, ID_PARA_monsno, poke->show_monsno );
          PP_Renew( p_pp );

          // ボスポケモンの場合はニックネームを代入する
          if(poke->show_monsno>=MONSNO_ufo && poke->show_monsno<=MONSNO_KARATEOU){
            STRBUF *strbuf = GFL_MSG_CreateString( msgdata, pw_boss_poke_name_652+(poke->show_monsno-MONSNO_ufo));
            PP_Put( p_pp, ID_PARA_nickname, (u32)strbuf );
            GFL_STR_DeleteBuffer(strbuf);
          }
        }
      }
    }
  }
  GFL_HEAP_FreeMemory( ppd );
  GFL_MSG_Delete( msgdata );

}


#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  カットインチェック用バトルを開始
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	* param 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_POKEWOOD_EVENT_CutinCheck( GAMESYS_WORK *gsys, void * param )
{
  s_pokewood_cutin_check_flag  = TRUE;
  return POKEWOOD_EVENT_Rec( gsys, param );
}
#endif 
