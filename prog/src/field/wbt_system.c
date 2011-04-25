//============================================================================================
/**
 * @file  wbt_system.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.15
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
//#include "app/wbt_list.h"

#include "system/main.h"

#include "wbt_system_local.h"
#include "wbt_tool.h"

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE

#include "savedata/bsubway_savedata.h"  //bsubway_battle_data.hの内部定義のため
#include "battle/bsubway_battle_data.h" //BSUBWAY_PARTNER_DATA
#include "fld_btl_inst_tool.h"
#include "message.naix"    //NARC_message_wbt_trname_dat
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT

#include "sound/swan_sound_data.sadl"
#ifdef  PM_DEBUG
//#define DEBUG_WEAK_PARTY  //定義すると手持ちが激しく弱くなる
//#define DEBUG_STRONG_PARTY  //定義すると手持ちが激しく強くなる
#endif

//============================================================================================
//============================================================================================
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr );
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetRomTrainer( WBTTRAINER * wbt_tr, u8 tr_id );
static void WBTTRAINER_SetWBPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata );

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSYS * WBTSYS_Create( HEAPID heapID )
{
  int i;
  WBTSYS * wbtsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTSYS) );

  wbtsys->heapID = heapID;
  wbtsys->type = WBTTYPE_NULL;
  wbtsys->rule = WBTRULE_NULL;
  wbtsys->stage = WBTSTAGE_RECEPTION;
  {
    u64 seed64;
    seed64 = GFL_STD_MtRand( 0 );
    wbtsys->seed64 = (seed64 << 32) + GFL_STD_MtRand( 0 );
    GFL_STD_RandInit( &wbtsys->rand_context, wbtsys->seed64 );
  }

  for (i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER_Init( &wbtsys->wbt_trainer[ i ] );
  }
  return wbtsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_Delete( WBTSYS * wbtsys )
{
  GFL_HEAP_FreeMemory( wbtsys );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief トレーナーID配列から実トレーナーデータをセットする
 */
//------------------------------------------------------------------
void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const u16 * tr_list )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER * wbt_tr = &wbtsys->wbt_trainer[ i ];
    switch ( tr_list[i] )
    {
    case WBT_TRCODE_PLAYER:
      WBTTRAINER_SetPlayer( wbt_tr, gamedata );
      break;
    case WBT_TRCODE_WBHERO:
      WBTTRAINER_SetWBPlayer( wbt_tr, gamedata );
      break;
    default:
      WBTTRAINER_SetRomTrainer( wbt_tr, tr_list[i] );
      break;
    }
  }
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetType( WBTSYS * wbtsys, WBTTYPE type )
{
  wbtsys->type = type;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTTYPE WBTSYS_GetType( const WBTSYS * wbtsys )
{
  return wbtsys->type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule )
{
  wbtsys->rule = rule;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys )
{
  return wbtsys->rule;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetPokeType( WBTSYS * wbtsys, PokeType poketype )
{
  wbtsys->poketype = poketype;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PokeType WBTSYS_GetPokeType( const WBTSYS * wbtsys )
{
  return wbtsys->poketype;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetStage( WBTSYS * wbtsys, WBTSTAGE stage )
{
  wbtsys->stage = stage;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSTAGE WBTSYS_GetStage( const WBTSYS * wbtsys )
{
  return wbtsys->stage;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u64 WBTSYS_GetRandomSeed( const WBTSYS * wbtsys )
{
  return wbtsys->seed64;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no )
{
  GF_ASSERT( no <= WBT_DOWNLOAD_NUM );
  wbtsys->select_dl_data = no;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys )
{
  return wbtsys->select_dl_data;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work )
{
  wbtsys->scr_answer_work = scr_answer_work;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 * WBTSYS_GetScriptAnswerWork( WBTSYS * wbtsys )
{
  GF_ASSERT( wbtsys->scr_answer_work );
  return wbtsys->scr_answer_work;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTSYS_SetTrainer( WBTSYS * wbtsys, const WBTTRAINER * wbt_tr, u32 pos )
{
  GF_ASSERT( pos < WBT_TRAINER_NUM );
  wbtsys->wbt_trainer[ pos ] = *wbt_tr;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const WBTTRAINER * WBTSYS_GetTrainer( const WBTSYS * wbtsys, u32 pos )
{
  GF_ASSERT( pos < WBT_TRAINER_NUM );
  return &wbtsys->wbt_trainer[ pos ];
}

//------------------------------------------------------------------
/**
 * @brief 参加ポケモンの補正レベルを取得する
 * @param wbtsys
 * @retval  0       補正しない
 * @retval  その他  補正するレベル値
 *
 * @todo  ダウンロードデータへの対応
 */
//------------------------------------------------------------------
static u8 WBTSYS_GetRevisePokeLevel( const WBTSYS * wbtsys )
{
  return 50;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr )
{
  wbt_tr->wbt_tr_type = WBT_TRTYPE_ROM;
  wbt_tr->sex = PM_MALE;
  wbt_tr->pri = 1;
  wbt_tr->tr_id = 0;
  wbt_tr->poketype = POKETYPE_NORMAL;
  wbt_tr->mmdl_id = SWHERO;
  wbt_tr->btl_tr_type = TRTYPE_HERO;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Set( WBTTRAINER * wbt_tr,
    WBT_TRTYPE type, u8 tr_id, u8 sex, u8 pri, u8 poketype, u16 mmdl_id, u16 btl_tr_type )
{
  wbt_tr->wbt_tr_type = type;
  wbt_tr->tr_id = tr_id;
  wbt_tr->sex = sex;
  wbt_tr->pri = pri;
  wbt_tr->poketype = poketype;
  wbt_tr->mmdl_id = mmdl_id;
  wbt_tr->btl_tr_type = btl_tr_type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_SetRomTrainer( WBTTRAINER * wbt_tr, u8 tr_id )
{
  WBT_TRDATA tr_rom_data;
  WBTTOOL_GetTrRomData( HEAPID_FIELDMAP, tr_id, &tr_rom_data );
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_ROM,
      tr_id, tr_rom_data.sex, tr_rom_data.priority, tr_rom_data.type,
      tr_rom_data.mmdl_id, tr_rom_data.tr_type );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

  u16 mmdl_id = SWHERO;
  u16 btl_tr_type = TRTYPE_HERO;
  u8 sex = MyStatus_GetMySex( mystat );
  u8 pri = 3;
  if ( WBTSAVE_GetFlag( wbtsave, WBTSAVE_FLAG_WIN_CHAMPION_TOURNAMENT ) == TRUE )
  {
    pri = 4;
  }
  if ( sex == PM_FEMALE )
  {
    mmdl_id = SWHEROINE;
    btl_tr_type = TRTYPE_HEROINE;
  }
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_PLAYER,
      0, sex, pri, POKETYPE_NORMAL, mmdl_id, btl_tr_type );
}

//------------------------------------------------------------------
/**
 * @todo  データ引継ぎ仕様に対応していない
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetWBPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  u8 tr_id;
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );

  tr_id = WBT_TRCODE_WBHERO_START; //トウヤ
  if ( MyStatus_GetMySex( mystat ) != PM_MALE )
  {
    tr_id = WBT_TRCODE_WBHERO_START + 1;  //トウコ
  }
  WBTTRAINER_SetRomTrainer( wbt_tr, tr_id );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTRAINER_GetPriority( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->pri;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTRAINER_GetID( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->tr_id;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 WBTTRAINER_GetBattleTrainerType( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->btl_tr_type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
WBT_TRTYPE WBTTRAINER_GetWBTTrainerType( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->wbt_tr_type;
}

//============================================================================================
//
//
//
//============================================================================================
#ifdef  PM_DEBUG
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_DEBUG_dumpTrainer( const WBTSYS * wbtsys )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, i );
    TAMADA_Printf("WBTTRAINER:POS=%02d", i );
    TAMADA_Printf("  wbt_tr_type(%d)tr_id(%03d)sex(%d)pri(%d)poketype(%2d)",
        wbt_tr->wbt_tr_type, wbt_tr->tr_id, wbt_tr->sex, wbt_tr->pri, wbt_tr->poketype );
    TAMADA_Printf("mmdl_id(%04x)btl_tr_type(%03d)\n",
         wbt_tr->mmdl_id, wbt_tr->btl_tr_type );
  }
}
#endif
//------------------------------------------------------------------
/**
 * @brief 対戦組み合わせの確定
 */
//------------------------------------------------------------------
void WBTSYS_MakeMatch( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  int i;
  u16 tr_list[ WBT_TRAINER_NUM ];
  WBT_MATCH_CONDITION cond;

  GFL_STD_MemClear( &cond, sizeof(WBT_MATCH_CONDITION) );
  cond.heapID = wbtsys->heapID;
  cond.type = wbtsys->type;
  cond.poketype = wbtsys->poketype;
  cond.level = WBTTOOL_GetAllTournamentEntryLevel( gamedata );
  cond.leader_num = WBTTOOL_GetAllTournamentLeaderRate( gamedata, &wbtsys->rand_context );
  cond.champ_entry = WBTTOOL_GetStatus( gamedata, WBT_COND_ID_ENTRY_CHAMP_IN_ALL );
  cond.seed64 = wbtsys->seed64;

  WBTTOOL_MakeEntryList( &cond, tr_list );

  WBTSYS_SetupTrainer( wbtsys, gamedata, tr_list );
#ifdef  PM_DEBUG
  WBTSYS_DEBUG_dumpTrainer( wbtsys );
  TAMADA_Printf("\n");
#endif

  WBTTOOL_SortTrainer( wbtsys->wbt_trainer , wbtsys->seed64, wbtsys->heapID );
#ifdef  PM_DEBUG
  WBTSYS_DEBUG_dumpTrainer( wbtsys );
#endif
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナーデータを取得
 */
//------------------------------------------------------------------
const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );
  return wbt_tr;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の性別を取得する
 */
//------------------------------------------------------------------
u8 WBTSYS_GetOpponentSex( WBTSYS * wbtsys )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );
  return wbt_tr->sex;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の動作モデル表示コードを取得する
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentMMdlObjCode( WBTSYS * wbtsys )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );
  return wbt_tr->mmdl_id;
}
//------------------------------------------------------------------
/**
 * @brief 対戦相手の名前を取得する
 * @todo  ダウンロードデータへの対応
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentName( WBTSYS * wbtsys, STRBUF * strbuf )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );

  if ( wbtsys->type == WBTTYPE_DOWNLOAD 
      || wbtsys->type == WBTTYPE_NULL )
  {
    GF_ASSERT( 0 );
  }
  else
  { 
    WBTTOOL_SetRomTrainerName( wbtsys->heapID, wbt_tr->tr_id, strbuf );
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys )
{
  const REGULATION * reg;
  int reg_type;
  switch ( wbtsys->rule )
  {
  case WBTRULE_SINGLE:
    reg_type = REG_WBT_SINGLE;
    break;
  case WBTRULE_DOUBLE:
    reg_type = REG_WBT_DOUBLE;
    break;
  case WBTRULE_TRIPLE:
    reg_type = REG_WBT_TRIPLE;
    break;
  case WBTRULE_ROTATION:
    reg_type = REG_WBT_ROTATION;
    break;
  default:
    GF_ASSERT( 0 );
  }
  PokeRegulation_LoadData( reg_type, &wbtsys->wbtreg.base_reg );
  return &wbtsys->wbtreg.base_reg;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, PLIST_DATA * plist_data )
{
  GFL_STD_MemCopy( plist_data->in_num, wbtsys->pokelist_select_num, 6 );
  wbtsys->pokelist_result_select = plist_data->ret_sel;
  wbtsys->pokelist_return_mode = plist_data->ret_mode;

  if( wbtsys->pokelist_return_mode != PL_RET_NORMAL ||
      wbtsys->pokelist_result_select == PL_SEL_POS_EXIT ||
      wbtsys->pokelist_result_select == PL_SEL_POS_EXIT2 )
  {
    *wbtsys->scr_answer_work = FALSE;
  }
  else
  {
    *wbtsys->scr_answer_work = TRUE;
  }
#ifdef  PM_DEBUG
  {
    int i;
    TAMADA_Printf("Select Mons:");
    for (i = 0; i < 6; i ++ )
    {
      TAMADA_Printf("%02d ", wbtsys->pokelist_select_num[i] );
    }
    TAMADA_Printf("\n ret_sel(%d) ret_mode(%d) script_return(%d)\n",
        wbtsys->pokelist_result_select, wbtsys->pokelist_return_mode, *wbtsys->scr_answer_work );
  }
#endif
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static POKEPARTY * getUsePokeParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * pp;
  if ( wbtsys->use_btlbox == FALSE )
  {
    pp = GAMEDATA_GetMyPokemon( gamedata );
  }
  else
  {
    pp = wbtsys->btl_box_party;
  }
  GF_ASSERT( pp );
  return pp;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data )
{
  int type;

  GFL_STD_MemClear32( plist_data, sizeof(PLIST_DATA) );

  plist_data->pp = getUsePokeParty( wbtsys, gamedata );

  switch ( wbtsys->rule )
  {
  default:
    GF_ASSERT(0);
    /* FALL THROUGH */

  case WBTRULE_SINGLE:
    type = PL_TYPE_SINGLE;
    break;
  case WBTRULE_DOUBLE:
    type = PL_TYPE_DOUBLE;
    break;
  case WBTRULE_TRIPLE:
    type = PL_TYPE_TRIPLE;
    break;
  case WBTRULE_ROTATION:
    type = PL_TYPE_ROTATION;
    break;
  }
  plist_data->reg = (void *)WBTSYS_GetRegulation( wbtsys );
  plist_data->mode = PL_MODE_BATTLE_SUBWAY;
  plist_data->type = type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus )
{
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gamedata );
  GFL_STD_MemClear32( pstatus, sizeof(PSTATUS_DATA) );
  pstatus->ppd = getUsePokeParty( wbtsys, gamedata );
  pstatus->ppt = PST_PP_TYPE_POKEPARTY;
  pstatus->max = PokeParty_GetPokeCount( pstatus->ppd );
  pstatus->mode = PST_MODE_NORMAL;
  pstatus->page = PPT_INFO;
  pstatus->game_data = gamedata;

  pstatus->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
}

//------------------------------------------------------------------
/**
 * @brief 参戦するポケモンパーティを生成する
 * @param wbtsys
 * @param gamedata
 *
 * @return  POKEPARTY 生成したポケモンパーティ
 *
 * @note    呼び出し元で責任を持ってメモリを開放する必要がある
 */
//------------------------------------------------------------------
POKEPARTY * WBTSYS_CreateBattleParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * entry_party;
  POKEPARTY * base_party;
  POKEMON_PARAM * entry_pp;
  u32 cnt, max;
  u8 revise_level;

  entry_party = GFL_HEAP_AllocMemory( wbtsys->heapID, PokeParty_GetWorkSize() );
  PokeParty_Init( entry_party, TEMOTI_POKEMAX );
  base_party = getUsePokeParty( wbtsys, gamedata );
  entry_pp = GFL_HEAP_AllocMemoryLo( wbtsys->heapID, POKETOOL_GetWorkSize() );
  PP_Clear( entry_pp );
  max = WBTTOOL_GetPartyMaxByRule( wbtsys->rule );
#ifdef DEBUG_WEAK_PARTY
  max = 1;
#endif
#ifdef  DEBUG_STRONG_PARTY
  max = PokeParty_GetPokeCount( base_party );
#endif
  revise_level = WBTSYS_GetRevisePokeLevel( wbtsys );
#ifdef  DEBUG_STRONG_PARTY
  revise_level = 100;
#endif
  for ( cnt = 0; cnt < max; cnt ++ )
  {
    POKEMON_PARAM * base_pp;
    u32 pos = wbtsys->pokelist_select_num[ cnt ];
    GF_ASSERT( pos );
    pos -= 1; //1 origin --> 0 origin
    base_pp = PokeParty_GetMemberPointer( base_party, pos );
    POKETOOL_CopyPPtoPP( base_pp, entry_pp );
    if ( revise_level != 0 && PP_Get( entry_pp, ID_PARA_level, NULL ) != revise_level )
    { //必要な場合はレベルを補正する
      POKETOOL_MakeLevelRevise( entry_pp, revise_level );
    }
#ifdef DEBUG_WEAK_PARTY
    PP_Put( entry_pp, ID_PARA_hp, 1 );  //負けやすくする！
#endif
    PokeParty_Add( entry_party, entry_pp );
  }
  GFL_HEAP_FreeMemory( entry_pp );

  return entry_party;
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const u8 pri2pow_table[5] = {
  0x10, 0x12, 0x14, 0x1a, 0x1f
};

//-----------------------------------------------------------------------------
/**
 * @param tr_data
 * @param wbt_tr
 * @param cnt
 * @param inHeapID
 * @todo  バトルサブウェイは４体しかポケモンを選択できない。
 */
//-----------------------------------------------------------------------------
static BOOL makeRomTrainerData( BSUBWAY_PARTNER_DATA * tr_data,
    const WBTTRAINER * wbt_tr,
    u8 cnt, HEAPID inHeapID)
{
  BOOL      ret = 0;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  u8 pow_rnd;
  u8 tr_pri = WBTTRAINER_GetPriority(wbt_tr);
  u8 tr_no = WBTTRAINER_GetID(wbt_tr);

  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(tr_data,
      ARCID_WBT_TD ,tr_no, NARC_message_wbt_trname_dat ,inHeapID);
  // パワー乱数を決定する
  pow_rnd= pri2pow_table[ tr_pri - 1 ];
  tr_data->bt_trd.tr_type = WBTTRAINER_GetBattleTrainerType( wbt_tr );
  //tr_data->bt_trd.player_id = tr_no + BSW_TR_ARCDATANO_ORG; //1 origin
  tr_data->bt_trd.player_id = tr_no + TRID_OFFSET_WBT;

  //ポケモンデータをセット
  ret = FBI_TOOL_SetBSWayPokemonParam(
      trd, tr_no, &tr_data->btpwd[0], cnt,
      ARCID_BSW_PD, NULL, NULL, NULL, NULL ,pow_rnd ,inHeapID);
  GFL_HEAP_FreeMemory( trd );
  return ret;
}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAM生成
 */
//------------------------------------------------------------------
BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys )
{
  BATTLE_SETUP_PARAM * bp;
  BSUBWAY_PARTNER_DATA * tr_data;
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  POKEPARTY *party;
  WBTRULE wbt_rule = WBTSYS_GetRule( wbtsys );
  BSWAY_PLAYMODE mode = WBTTOOL_WBTRuleToBtlRule( wbt_rule );
  int num = WBTTOOL_GetPartyMaxByRule( wbt_rule );

  tr_data = GFL_HEAP_AllocMemoryLo( HEAPID_PROC, sizeof(BSUBWAY_PARTNER_DATA) );
  party = WBTSYS_CreateBattleParty( wbtsys, GAMESYSTEM_GetGameData( gsys ) );
  makeRomTrainerData( tr_data, wbt_tr, num, HEAPID_PROC );
  bp = FBI_TOOL_WBT_CreateBattleParam( gsys, party, mode, tr_data, num  );

  /* 仮：α版用BGM設定 */
  if ( wbtsys->stage == WBTSTAGE_FINAL )
  {
    bp->musicDefault = SEQ_BGM_VS_WCS;
    bp->musicWin = SEQ_BGM_WIN5;
  } else {
    bp->musicDefault = SEQ_BGM_VS_SUBWAY_TRAINER;
    bp->musicWin = SEQ_BGM_WIN2;
  }

  GFL_HEAP_FreeMemory( party );
  GFL_HEAP_FreeMemory( tr_data );
  return bp;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static u32 countDeadStatus( const BtlPokeStatCode * party_state )
{
  int i;
  u32 count = 0;
  for ( i = 0; i < TEMOTI_POKEMAX; i ++ )
  {
    if ( party_state[i] == BTL_POKESTATE_DEAD )
    {
      count ++;
    }
  }
  return count;
}
//------------------------------------------------------------------
/**
 * @brief WBT戦闘結果の反映
 * @param wbtsys
 * @param gamedata
 */
//------------------------------------------------------------------
void WBTSYS_ReflectBattleResult( WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp )
{
  u16 * ret_wk = WBTSYS_GetScriptAnswerWork( wbtsys );
  u32 result = GAMEDATA_GetLastBattleResult( gamedata ); 
  u32 my_count    = countDeadStatus( bsp->party_state[ BTL_CLIENT_PLAYER ] );
  u32 enemy_count = countDeadStatus( bsp->party_state[ BTL_CLIENT_ENEMY1 ] );

  TAMADA_Printf("my_count = %d, enemy_count = %d\n", my_count, enemy_count );
  switch ( result )
  {
  default:
    GF_ASSERT_MSG( 0, "想定外のBtlResult(%d)\n", result );
  case BTL_RESULT_RUN:  //自分が逃げた
    result = BTL_RESULT_LOSE;
    break;
  case BTL_RESULT_LOSE:
  case BTL_RESULT_WIN:
  case BTL_RESULT_DRAW:
    break;
  }
  *ret_wk = result;
}



