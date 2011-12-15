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

#include "system/main.h"

#include "wbt_system_local.h"
#include "wbt_tool.h"

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE


#include "message.naix"    //NARC_message_

#include "savedata/save_tbl.h"
#include "savedata/save_control.h"
#include "savedata/wbt_ex_save.h"
#include "savedata/wbt_download.h"

#include "sound/swan_sound_data.sadl"

#ifdef  PM_DEBUG
//#define DEBUG_WEAK_PARTY  //定義すると手持ちが激しく弱くなる
//#define DEBUG_STRONG_PARTY  //定義すると手持ちが激しく強くなる
#endif

//============================================================================================
//============================================================================================
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr, HEAPID heapID );
static void WBTTRAINER_Release( WBTTRAINER * wbt_tr );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSYS * WBTSYS_Create( HEAPID heapID, BOOL isTutorial )
{
  int i;
  WBTSYS * wbtsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTSYS) );

  wbtsys->heapID = heapID;
  wbtsys->isTutorial = isTutorial;
  wbtsys->wbtcup = WBTCUP_NULL;
  wbtsys->rule = WBTRULE_NULL;
  wbtsys->stage = WBTSTAGE_RECEPTION;
  {
    u64 seed64;
    seed64 = GFL_STD_MtRand( 0 );
    wbtsys->seed64 = (seed64 << 32) + GFL_STD_MtRand( 0 );
    GFL_STD_RandInit( &wbtsys->rand_context, wbtsys->seed64 );
  }
  wbtsys->audience_type = GFUser_GetPublicRand( WBT_AUDIENCE_TYPE_MAX );

  for (i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER_Init( &wbtsys->wbt_trainer[ i ], heapID );
  }
  //POKEPARTY保存領域を作成
  wbtsys->my_party = PokeParty_AllocPartyWork( wbtsys->heapID );
  wbtsys->npc_party = PokeParty_AllocPartyWork( wbtsys->heapID );
  wbtsys->sp_party = PokeParty_AllocPartyWork( wbtsys->heapID );

  //ROMトレーナーデータを読み込む
  wbtsys->rom_trdata = WBTTOOL_CreateRomTrainerData( wbtsys->heapID );

  return wbtsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_Delete( WBTSYS * wbtsys )
{
  int i;

  WBTTOOL_DeleteRomTrainerData( (void*)wbtsys->rom_trdata );

  GFL_HEAP_FreeMemory( wbtsys->sp_party );
  GFL_HEAP_FreeMemory( wbtsys->my_party );
  GFL_HEAP_FreeMemory( wbtsys->npc_party );
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER_Release( &wbtsys->wbt_trainer[ i ] );
  }
  GFL_HEAP_FreeMemory( wbtsys );
}

//============================================================================================
//
//
//    WBTSYSメンバー  アクセサー
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetWBTCup( WBTSYS * wbtsys, WBTCUP wbtcup )
{
  wbtsys->wbtcup = wbtcup;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTCUP WBTSYS_GetWBTCup( const WBTSYS * wbtsys )
{
  return wbtsys->wbtcup;
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
void WBTSYS_SetReceptionID( WBTSYS * wbtsys, WBTRECEPT reception_id )
{
  wbtsys->reception_id = reception_id;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRECEPT WBTSYS_GetReceptionID( const WBTSYS * wbtsys )
{
  return wbtsys->reception_id;
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
void WBTSYS_SetMixInfoWinPointer( WBTSYS * wbtsys, WBT_MIXWIN * wmw )
{
  wbtsys->MixInfoWinWork = wmw;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBT_MIXWIN * WBTSYS_GetMixInfoWinPointer( WBTSYS * wbtsys )
{
  return wbtsys->MixInfoWinWork;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetRentalType( WBTSYS * wbtsys, WBTRENT_TYPE rental_type )
{
  wbtsys->rental_type = rental_type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRENT_TYPE WBTSYS_GetRentalType( const WBTSYS * wbtsys )
{
  return wbtsys->rental_type;
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
 * @brief 自分のトレーナーデータを取得する
 * @param wbtsys
 * @return  WBTTRAINER  自機データをセットしたWBTTRAINERへのポインタ
 */
//------------------------------------------------------------------
const WBTTRAINER * WBTSYS_GetPlayerTrainer( const WBTSYS * wbtsys )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    if (wbtsys->wbt_trainer[i].wbt_tr_type == WBT_TRTYPE_PLAYER )
    {
      return &wbtsys->wbt_trainer[i];
    }
  }
  GF_ASSERT( 0 );
  return &wbtsys->wbt_trainer[0];
}

//------------------------------------------------------------------
/**
 * @brief トーナメント名を取得する
 */
//------------------------------------------------------------------
void WBTSYS_GetTournamentName( const WBTSYS * wbtsys, WBTCUP wbtcup, STRBUF * strbuf )
{
  if ( wbtcup == WBTCUP_DOWNLOAD )
  {
    GF_ASSERT( wbtsys->dldata_loaded );
    GFL_STR_SetStringCodeOrderLength( strbuf, WBTDL_DATA_GetTournamentName( &wbtsys->wbtdl_data ),
        WBTDL_TOURNAMENT_NAME_LEN );
  }
  else
  {
    WBTTOOL_GetTournamentName( wbtsys->heapID, wbtcup, strbuf );
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
POKEPARTY * WBTSYS_GetMyParty( WBTSYS * wbtsys )
{
  return wbtsys->my_party;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
POKEPARTY * WBTSYS_GetEnemyParty( WBTSYS * wbtsys )
{
  return wbtsys->npc_party;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTSYS_GetPartyMax( WBTSYS * wbtsys )
{
  return WBTTOOL_GetPartyMaxByRule( wbtsys->rule );
}


//============================================================================================
//
//    WBTTRAINER関連
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief WBTTRAINER生成
 *
 * 内部でメモリ確保するため、使用終了時には必ずWBTTRAINER_Releaseを呼ぶこと
 */
//------------------------------------------------------------------
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr, HEAPID heapID )
{
  wbt_tr->wbt_tr_type = WBT_TRTYPE_ROM;
  wbt_tr->sex         = PM_MALE;
  wbt_tr->pri         = 1;
  wbt_tr->has_ace     = FALSE;
  wbt_tr->poketype    = POKETYPE_NORMAL;
  wbt_tr->tr_id       = 0;
  wbt_tr->pow_rnd     = 0;
  wbt_tr->dl_tr_id    = 0;
  wbt_tr->mmdl_id     = SWHERO;
  wbt_tr->btl_tr_type = TRTYPE_HERO;
  wbt_tr->ai_bit      = WBT_AI_BIT_BASIC;
  wbt_tr->name        = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
  wbt_tr->title       = GFL_STR_CreateBuffer( WBTDL_TRAINER_TITLE_LEN, heapID );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Release( WBTTRAINER * wbt_tr )
{
  GFL_STR_DeleteBuffer( wbt_tr->name );
  GFL_STR_DeleteBuffer( wbt_tr->title );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTRAINER_GetPriority( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->pri;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTRAINER_GetPowerRnd( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->pow_rnd;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTRAINER_GetDLID( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->dl_tr_id;
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

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTTRAINER_CopyName( const WBTTRAINER * wbt_tr, STRBUF * buf )
{
  GFL_STR_CopyBuffer( buf, wbt_tr->name );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTTRAINER_CopyTitle( const WBTTRAINER * wbt_tr, STRBUF * buf )
{
  GFL_STR_CopyBuffer( buf, wbt_tr->title );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL WBTTRAINER_HasAce( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->has_ace;
}

//============================================================================================
//
//
//
//============================================================================================
#ifdef  PM_DEBUG
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTTRAINER_DEBUG_dump( const WBTTRAINER * wbt_tr )
{
  TAMADA_Printf("  wbt_tr_type(%d)tr_id(%03d)sex(%d)pri(%d)poketype(%2d)",
      wbt_tr->wbt_tr_type, wbt_tr->tr_id, wbt_tr->sex, wbt_tr->pri, wbt_tr->poketype );
  TAMADA_Printf("powrnd(%d)has_ace(%d)", wbt_tr->pow_rnd, wbt_tr->has_ace);
  TAMADA_Printf("mmdl_id(%04x)btl_tr_type(%03d)\n",
       wbt_tr->mmdl_id, wbt_tr->btl_tr_type );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_DEBUG_dumpTrainer( const WBTSYS * wbtsys )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, i );
    TAMADA_Printf("WBTTRAINER:POS=%02d", i );
    WBTTRAINER_DEBUG_dump( wbt_tr );
  }
}
#endif


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 参戦するポケモンパーティを生成する
 * @param wbtsys
 * @param gamedata
 *
 */
//------------------------------------------------------------------
void WBTSYS_SetupMyParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * entry_party;
  POKEPARTY * base_party;
  POKEMON_PARAM * entry_pp;
  u32 cnt, max;

  entry_party = PokeParty_AllocPartyWork( GFL_HEAP_LOWID(wbtsys->heapID) );
  PokeParty_InitWork( entry_party );
  base_party = WBTSYS_GetUsePokeParty( wbtsys, gamedata );
  entry_pp = GFL_HEAP_AllocMemoryLo( wbtsys->heapID, POKETOOL_GetWorkSize() );
  PP_Clear( entry_pp );
#if   defined(DEBUG_STRONG_PARTY)
  max = PokeParty_GetPokeCount( base_party );
#elif defined(DEBUG_WEAK_PARTY)
  max = 1;
#else
  max = WBTSYS_GetPartyMax( wbtsys );
#endif
  for ( cnt = 0; cnt < max; cnt ++ )
  {
    POKEMON_PARAM * base_pp;
    u32 pos = wbtsys->pokelist_select_num[ cnt ];
    if ( pos == 0 )
    {
      GF_ASSERT_MSG( 0, "参戦するポケモンが正しくありません!!\n" );
      pos = 1;
    }
    pos -= 1; //1 origin --> 0 origin
    base_pp = PokeParty_GetMemberPointer( base_party, pos );
    POKETOOL_CopyPPtoPP( base_pp, entry_pp );
#ifdef DEBUG_WEAK_PARTY
    PP_Put( entry_pp, ID_PARA_hp, 1 );  //負けやすくする！
#endif
    PokeParty_Add( entry_party, entry_pp );
  }
  PokeParty_Copy( entry_party, wbtsys->my_party );
  PokeParty_Copy( entry_party, wbtsys->sp_party );  //コピーを作成
  GFL_HEAP_FreeMemory( entry_party );
  GFL_HEAP_FreeMemory( entry_pp );
}

//============================================================================================
//
//    レギュレーション関連
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レギュレーション指定IDの取得
 */
//------------------------------------------------------------------
u32 WBTSYS_GetRegulationID( WBTSYS * wbtsys )
{
  static const u8 regAllTable[] = {
    REG_WBT_SINGLE, REG_WBT_DOUBLE, REG_WBT_TRIPLE, REG_WBT_ROTATION,
  };
  static const u8 regSubwayTable[] = {
    REG_WBT_SUBWAY_SINGLE, REG_WBT_SUBWAY_DOUBLE, REG_WBT_SUBWAY_TRIPLE, REG_WBT_SUBWAY_ROTATION,
  };
  const u8 * tbl;

  if ( wbtsys->wbtcup == WBTCUP_HODOMOE_EVENT )
  {
    return REG_WBT_EVENT_SINGLE;
  }

  switch ( WBTTOOL_GetRegulationID( wbtsys->wbtcup ) )
  {
  default:
  case WBTREG_DOWNLOAD:
    GF_ASSERT( 0 ); //ありえないはずなので、未対応
  case WBTREG_ALL:
    tbl = regAllTable;
    break;
  case WBTREG_SUBWAY:
  case WBTREG_POKETYPE:
    tbl = regSubwayTable;
    break;
  }
  GF_ASSERT( wbtsys->rule != WBTRULE_NULL && wbtsys->rule < WBTRULE_MAX );
  return tbl[ wbtsys->rule ];
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline void WBTDL_DATA_DecodeRegulation( const WBTDL_DATA * wbtdl_data, REGULATION * reg )
{
  const WBTDL_RULE * wbtdl_rule = &(wbtdl_data->wbtdl_rule);
  int i;
  GFL_STD_MemClear( reg, sizeof(REGULATION) );
  reg->NUM_LO       = wbtdl_rule->num_lo;
  reg->NUM_HI       = wbtdl_rule->num_hi;
  reg->LEVEL        = wbtdl_rule->level;
  reg->LEVEL_RANGE  = wbtdl_rule->level_range;
  reg->LEVEL_TOTAL  = wbtdl_rule->level_total;
  reg->BOTH_POKE    = wbtdl_rule->both_poke;
  reg->BOTH_ITEM    = wbtdl_rule->both_item;
  for ( i = 0; i < REG_POKENUM_MAX_BYTE; i ++ )
  {
    reg->VETO_POKE_BIT[i] = wbtdl_rule->veto_poke_bit[i];
  }
  for ( i = 0; i < REG_ITEMNUM_MAX_BYTE; i ++ )
  {
    reg->VETO_ITEM[i]     = wbtdl_rule->veto_item_bit[i];
  }
  reg->BATTLE_TYPE  = wbtdl_rule->btl_rule;
}

//------------------------------------------------------------------
/**
 * @brief レギュレーションの取得
 */
//------------------------------------------------------------------
const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys )
{
  if (wbtsys->wbtcup == WBTCUP_DOWNLOAD )
  {
    GF_ASSERT( wbtsys->dldata_loaded );
    WBTDL_DATA_DecodeRegulation( &wbtsys->wbtdl_data, &wbtsys->wbtreg.base_reg );
  }
  else
  {
    u32 reg_type;
    reg_type = WBTSYS_GetRegulationID( wbtsys );

    PokeRegulation_LoadData( reg_type, &wbtsys->wbtreg.base_reg );
    if ( wbtsys->wbtcup == WBTCUP_HODOMOE_EVENT || wbtsys->wbtcup == WBTCUP_HODOMOE )
    {
      wbtsys->wbtreg.base_reg.LEVEL = 25;
    }
  }

#ifdef PM_DEBUG
  //WBTSYS_DEBUG_PrintRegulation( &wbtsys->wbtreg.base_reg );
#endif
  return &wbtsys->wbtreg.base_reg;
}

//------------------------------------------------------------------
/**
 * @brief 拡張レギュレーションの取得
 */
//------------------------------------------------------------------
const REGULATIONEX * WBTSYS_GetRegulationEx( WBTSYS * wbtsys )
{
  u8 poketype = POKETYPE_NULL;
  u16 waza_no = 0;
  if ( wbtsys->wbtcup == WBTCUP_POKETYPE )
  { //タイプトーナメントの場合はタイプ指定
    poketype = wbtsys->poketype;
  }
  else if ( wbtsys->wbtcup == WBTCUP_DOWNLOAD )
  {
    poketype = wbtsys->wbtdl_data.wbtdl_rule.need_poketype;
    waza_no = wbtsys->wbtdl_data.wbtdl_rule.veto_waza_no;
  }
  REGULATIONEX_Init( &wbtsys->wbtreg.ex_reg, poketype, waza_no );
#ifdef PM_DEBUG
  WBTSYS_DEBUG_PrintRegulationEx( &wbtsys->wbtreg.ex_reg );
#endif
  return &wbtsys->wbtreg.ex_reg;
}

#ifdef PM_DEBUG
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_DEBUG_PrintRegulationEx( const REGULATIONEX * reg_ex )
{
  OS_TPrintf("REGULATIONEX:");
  OS_TPrintf("poketype = %2d, waza_no = %3d\n", reg_ex->poketype, reg_ex->waza_no );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_DEBUG_PrintRegulation( const REGULATION * cp_data )
{
  OS_TPrintf( "reg size =%d\n", sizeof(REGULATION));
  OS_TPrintf( "cupno    =%d\n", cp_data->cupNo);
  OS_TPrintf( "rule_num =%d\n", cp_data->ruleNo); ///< 大会ルール番号
  OS_TPrintf( "num_lo   =%d\n", cp_data->NUM_LO); ///<    #参加数下限
  OS_TPrintf( "num_hi   =%d\n", cp_data->NUM_HI); ///<    #参加数上限
  OS_TPrintf( "level    =%d\n", cp_data->LEVEL);  ///<    #参加レベル
  OS_TPrintf( "lv_range =%d\n", cp_data->LEVEL_RANGE); ///<    #レベル範囲
  OS_TPrintf( "lv_total =%d\n", cp_data->LEVEL_TOTAL); ///<    #レベル合計
  OS_TPrintf( "both_poke=%d\n", cp_data->BOTH_POKE); ///<    #同じポケモン
  OS_TPrintf( "both_item=%d\n", cp_data->BOTH_ITEM);  ///<    #同じどうぐ             60
  OS_TPrintf( "veto_poke=0x%x\n", cp_data->VETO_POKE_BIT[REG_POKENUM_MAX_BYTE]);  ///<    #参加禁止ポケモン 142
  OS_TPrintf( "veto_item=0x%d\n", cp_data->VETO_ITEM[REG_ITEMNUM_MAX_BYTE]); ///<    #持ち込み禁止道具 218
  OS_TPrintf( "must_poke=%d\n", cp_data->MUST_POKE);   ///<    #必須ポケモン
  OS_TPrintf( "must_form=%d\n", cp_data->MUST_POKE_FORM);   ///<    #必須ポケモンフォルム
  OS_TPrintf( "shooter  =%d\n", cp_data->SHOOTER);    ///<    #シューター
  OS_TPrintf( "time_vs  =%d\n", cp_data->TIME_VS);     ///<    #対戦時間
  OS_TPrintf( "time_cmd =%d\n", cp_data->TIME_COMMAND); ///<    #入力時間
  OS_TPrintf( "nick_view=%d\n", cp_data->NICKNAME); ///<    #ニックネーム表示
  OS_TPrintf( "camera   =%d\n", cp_data->CAMERA);  ///<     #カメラモード
  OS_TPrintf( "show_poke=%d\n", cp_data->SHOW_POKE);  ///<    #ポケモン見せ合い
  OS_TPrintf( "show_time=%d\n", cp_data->SHOW_POKE_TIME);  ///<ポケモン見せ合い時間
  OS_TPrintf( "btl_type =%d\n", cp_data->BATTLE_TYPE);    ///< バトルタイプ  230
}
#endif

