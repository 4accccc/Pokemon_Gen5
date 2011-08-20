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


#include "fld_btl_inst_tool.h"  //FBI_TOOL_GetTrainerOBJCode
#include "message.naix"    //NARC_message_

#include "savedata/etc_save.h"    //ETC_SAVE_WORK

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
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetRomTrainer(
    const WBTSYS * wbtsys, HEAPID heapID, WBTTRAINER * wbt_tr, WBTROM_TRID tr_id );
static void WBTTRAINER_SetWBPlayer(
    const WBTSYS * wbtsys, HEAPID heapID, WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetDLTrainer( WBTTRAINER * wbt_tr,
    const WBTDL_CHARACTER * dl_char, u32 dl_tr_id, u8 pri );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSYS * WBTSYS_Create( HEAPID heapID )
{
  int i;
  WBTSYS * wbtsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTSYS) );

  wbtsys->heapID = heapID;
  wbtsys->wbtcup = WBTCUP_NULL;
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
    WBTTRAINER_Init( &wbtsys->wbt_trainer[ i ], heapID );
  }
  //POKEPARTY保存領域を作成
  wbtsys->my_party = PokeParty_AllocPartyWork( wbtsys->heapID );
  wbtsys->npc_party = PokeParty_AllocPartyWork( wbtsys->heapID );

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

  GFL_HEAP_FreeMemory( wbtsys->my_party );
  GFL_HEAP_FreeMemory( wbtsys->npc_party );
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER_Release( &wbtsys->wbt_trainer[ i ] );
  }
  GFL_HEAP_FreeMemory( wbtsys );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief トレーナーID配列から実トレーナーデータをセットする
 *
 * @param[in,out] wbtsys
 * @param[in]     gamedata
 * @param[in]     tr_list
 */
//------------------------------------------------------------------
void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const u16 * tr_list )
{
  int i, count;
  for ( i = 0, count = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER * wbt_tr = &wbtsys->wbt_trainer[ i ];
    switch ( tr_list[i] )
    {
    case WBT_TRCODE_PLAYER:
      WBTTRAINER_SetPlayer( wbt_tr, gamedata );
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->tr_id, wbt_tr->title );
      break;
    case WBT_TRCODE_WBHERO:
      WBTTRAINER_SetWBPlayer( wbtsys, wbtsys->heapID, wbt_tr, gamedata );
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->tr_id, wbt_tr->title );
      break;
    case WBT_TRCODE_DL1:
    case WBT_TRCODE_DL2:
    case WBT_TRCODE_DL3:
    case WBT_TRCODE_DL4:
      {
        const WBTDL_CHARACTER * dl_char;
        u32 dl_tr_id = tr_list[i] - WBT_TRCODE_DL1;
        dl_char = WBTDL_DATA_GetCharacter( &(wbtsys->wbtdl_data), dl_tr_id );
        WBTTRAINER_SetDLTrainer( wbt_tr, dl_char, dl_tr_id, 3 );
      }
      break;
    default:
      WBTTRAINER_SetRomTrainer( wbtsys, wbtsys->heapID, wbt_tr, tr_list[i] );
      if ( tr_list[i] == WBTTOOL_GetRivalTrainerID(wbtsys->rom_trdata) )
      { //ライバルの場合はセーブデータから名前を引っ張る
        SAVE_CONTROL_WORK * sv_ctrl_work = GAMEDATA_GetSaveControlWork( gamedata );
        ETC_SAVE_WORK * etcsave = SaveData_GetEtc( sv_ctrl_work );
        GFL_STR_SetStringCode( wbt_tr->name, EtcSave_GetRivalName( etcsave ) );
      }
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->tr_id, wbt_tr->title );
      break;
    }
    //ＤＬトーナメントの場合は優先度をＤＬデータのもので上書きする
    if ( wbtsys->wbtcup == WBTCUP_DOWNLOAD
        && wbt_tr->wbt_tr_type != WBT_TRTYPE_PLAYER )
    {
      const WBTDL_MATCH * wbtdl_match = WBTDL_DATA_GetMatchData( &wbtsys->wbtdl_data );
      wbt_tr->pri = wbtdl_match[count].pri;
      count ++;
    }
  }
}

//============================================================================================
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
    WBTTOOL_GetTournamentName( wbtcup, strbuf );
  }
}
//============================================================================================
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
  wbt_tr->sex = PM_MALE;
  wbt_tr->pri = 1;
  wbt_tr->tr_id = 0;
  wbt_tr->poketype = POKETYPE_NORMAL;
  wbt_tr->mmdl_id = SWHERO;
  wbt_tr->btl_tr_type = TRTYPE_HERO;
  wbt_tr->name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
  wbt_tr->title = GFL_STR_CreateBuffer( WBTDL_TRAINER_TITLE_LEN, heapID );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Release( WBTTRAINER * wbt_tr )
{
  GFL_STR_DeleteBuffer( wbt_tr->name );
  GFL_STR_DeleteBuffer( wbt_tr->title );
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：基本
 */
//------------------------------------------------------------------
static void WBTTRAINER_Set( WBTTRAINER * wbt_tr,
    WBT_TRTYPE type, u8 tr_id, u8 sex, u8 pri, BOOL has_ace, u8 poketype, u16 mmdl_id, u16 btl_tr_type )
{
  wbt_tr->wbt_tr_type = type;
  wbt_tr->tr_id = tr_id;
  wbt_tr->sex = sex;
  wbt_tr->pri = pri;
  wbt_tr->has_ace = has_ace;
  wbt_tr->poketype = poketype;
  wbt_tr->mmdl_id = mmdl_id;
  wbt_tr->btl_tr_type = btl_tr_type;
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：ROM上のトレーナー
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetRomTrainer(
    const WBTSYS * wbtsys, HEAPID heapID, WBTTRAINER * wbt_tr, WBTROM_TRID tr_id )
{
  WBT_TRDATA tr_rom_data;
  BOOL has_ace;

  WBTTOOL_GetRomTrainerData( wbtsys->rom_trdata, tr_id, &tr_rom_data );
  has_ace = tr_rom_data.ace_poke_id != 0 ? TRUE: FALSE;
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_ROM,
      tr_id, tr_rom_data.sex, tr_rom_data.priority, has_ace, tr_rom_data.type,
      tr_rom_data.mmdl_id, tr_rom_data.tr_type );
  WBTTOOL_GetRomTrainerName( heapID, tr_id, wbt_tr->name );
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：プレイヤー自身
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

  u16 mmdl_id = SWHERO;
  u16 btl_tr_type = TRTYPE_HERO;
  u8 sex = MyStatus_GetMySex( mystat );
  u8 pri = 3;
  if ( WBTSAVE_GetWinCount( wbtsave, WBTWIN_COUNT_CHAMPION ) != 0 )
  {
    pri = 4;
  }
  if ( sex == PM_FEMALE )
  {
    mmdl_id = SWHEROINE;
    btl_tr_type = TRTYPE_HEROINE;
  }
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_PLAYER,
      0, sex, pri, FALSE, POKETYPE_NORMAL, mmdl_id, btl_tr_type );
  MyStatus_CopyNameString( mystat, wbt_tr->name );
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：前作主人公
 * @todo  データ引継ぎ仕様に対応していない
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetWBPlayer(
    const WBTSYS * wbtsys, HEAPID heapID, WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  WBTROM_TRID tr_id;
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );

  tr_id = WBTTOOL_GetBWRomTrainerID( wbtsys->rom_trdata, MyStatus_GetMySex( mystat ) );
  WBTTRAINER_SetRomTrainer( wbtsys, heapID, wbt_tr, tr_id );
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：ダウンロードデータから
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetDLTrainer( WBTTRAINER * wbt_tr,
    const WBTDL_CHARACTER * dl_char, u32 dl_tr_id, u8 pri )
{
  u8 sex;
  u32 trtype, mmdl_id;

  GF_ASSERT( dl_tr_id < WBTDL_CHAR_MAX );
  trtype = WBTTOOL_GetDLTrtype( dl_char->trtype_id );
  mmdl_id = FBI_TOOL_GetTrainerOBJCode( trtype );
  sex = TT_TrainerTypeSexGet( trtype );
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_DL, dl_tr_id, sex,
      pri, FALSE, POKETYPE_NORMAL, mmdl_id, trtype );

  GFL_STR_SetStringCodeOrderLength( wbt_tr->name, dl_char->name, SAVELEN_TRAINER_NAME );
  GFL_STR_SetStringCodeOrderLength( wbt_tr->title, dl_char->title, WBTDL_TRAINER_TITLE_LEN );
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
  cond.wbtcup = wbtsys->wbtcup;
  cond.poketype = wbtsys->poketype;
  cond.rom_trainer_num = wbtsys->rom_trdata->num;
  cond.rom_trainer_data = wbtsys->rom_trdata->data;
  cond.seed64 = wbtsys->seed64;
  cond.wbtdl_match = NULL;
  if ( wbtsys->dldata_loaded )
  {
    cond.wbtdl_match = WBTDL_DATA_GetMatchData( &wbtsys->wbtdl_data );
  }

  WBTSYS_MakeEntryList( gamedata, &cond, tr_list );

  WBTSYS_SetupTrainer( wbtsys, gamedata, tr_list );
#ifdef  PM_DEBUG
  WBTSYS_DEBUG_dumpTrainer( wbtsys );
  TAMADA_Printf("\n");
#endif

  WBTSYS_SortTrainer( wbtsys->wbt_trainer, wbtsys->seed64, wbtsys->heapID );
#ifdef  PM_DEBUG
  WBTSYS_DEBUG_dumpTrainer( wbtsys );
#endif
}

//------------------------------------------------------------------
/**
 * @brief ごほうびのBP数取得
 * @param[in]   wbtsys
 * @return      u8
 */
//------------------------------------------------------------------
u8 WBTSYS_GetRewardCount( const WBTSYS * wbtsys )
{
  u8 count;
  int i;
  switch ( wbtsys->wbtcup )
  {
  case WBTCUP_CHAMPION:
  case WBTCUP_WORLD:
    for ( i = 0, count = 0; i < WBT_BATTLE_NUM; i ++ )
    {
      count += wbtsys->survival_count[ i ];
    }
    break;
  default:
    count = WBTTOOL_GetRewardCount( wbtsys->wbtcup, wbtsys->rule );
  }
  return count;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
static BOOL checkWBTDownLoad( GAMEDATA * gamedata, u32 dl_id )
{
  SAVE_CONTROL_WORK* save;
  WBTEXSAVE * save_p;
  SAVE_EXTRA_ID save_id;

  LOAD_RESULT load_res;
  BOOL result = FALSE;

  save_id = SAVE_EXTRA_ID_WBT_00 + dl_id - 1;

  save = GAMEDATA_GetSaveControlWork( gamedata );
  load_res = SaveControl_Extra_Load( save, save_id, HEAPID_PROC );
  if ( load_res == LOAD_RESULT_OK )
  {
    save_p = SaveControl_Extra_DataPtrGet( save, save_id, EXGMDATA_ID_WBT_00 );
    result = WBTEXSAVE_IsExists( save_p );
  }
  SaveControl_Extra_Unload( save, save_id ); // 外部データを解放
  return result;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL WBTSYS_GetDownLoadCount( const WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  int count, dl_id;
  for ( count = 0, dl_id = 1; dl_id <= WBT_DOWNLOAD_NUM; dl_id ++ )
  {
    if (checkWBTDownLoad( gamedata, dl_id ) == TRUE )
    {
      count ++;
    }
  }
  return count;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_UnloadDLData( WBTSYS * wbtsys )
{
  GFL_STD_MemClear32( &wbtsys->wbtdl_data, sizeof(WBTDL_DATA) );
  wbtsys->dldata_loaded = FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_LoadDLData( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  SAVE_CONTROL_WORK* save;
  WBTEXSAVE * save_p;
  SAVE_EXTRA_ID save_id;
  LOAD_RESULT load_res;

  BOOL result = FALSE;

  if ( wbtsys->select_dl_data == 0 )
  {
    WBTSYS_UnloadDLData( wbtsys );
    return;
  }

  save_id = SAVE_EXTRA_ID_WBT_00 + wbtsys->select_dl_data - 1;

  save = GAMEDATA_GetSaveControlWork( gamedata );
  load_res = SaveControl_Extra_Load( save, save_id, HEAPID_PROC );  //外部データを確保

  if ( load_res == LOAD_RESULT_OK )
  {
    save_p = SaveControl_Extra_DataPtrGet( save, save_id, EXGMDATA_ID_WBT_00 );
    result = WBTEXSAVE_IsExists( save_p );
  }
  if ( result )
  {
    GFL_STD_MemCopy32( save_p, &wbtsys->wbtdl_data, sizeof(WBTDL_DATA) );
    wbtsys->rule = WBTTOOL_BtlRuleToWBTRule( WBTDL_DATA_GetBtlRule( &wbtsys->wbtdl_data ) );
  }
  else
  {
    GFL_STD_MemClear32( &wbtsys->wbtdl_data, sizeof(WBTDL_DATA) );
  }
  SaveControl_Extra_Unload( save, save_id ); // 外部データを解放

  wbtsys->dldata_loaded = result;
}

//============================================================================================
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
  static const u8 regWCSTable[] = {
    REG_WBT_WCS_SINGLE, REG_WBT_WCS_DOUBLE, REG_WBT_WCS_TRIPLE, REG_WBT_WCS_ROTATION,
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
    GF_ASSERT( 0 ); //仮：未対応
  case WBTREG_ALL:
    tbl = regAllTable;
    break;
  case WBTREG_SUBWAY:
  case WBTREG_POKETYPE:
    tbl = regSubwayTable;
    break;
  case WBTREG_WCS:
    tbl = regWCSTable;
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
  return &wbtsys->wbtreg.ex_reg;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTSYS_GetPartyMax( WBTSYS * wbtsys )
{
  return WBTTOOL_GetPartyMaxByRule( wbtsys->rule );
}

#ifdef PM_DEBUG
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
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief ポケモンリスト選択結果の反映処理
 */
//------------------------------------------------------------------
void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, const PLIST_DATA * plist_data )
{
  GFL_STD_MemCopy( plist_data->in_num, wbtsys->pokelist_select_num, TEMOTI_POKEMAX );
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
    for (i = 0; i < TEMOTI_POKEMAX; i ++ )
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
  POKEPARTY * party;
  if ( wbtsys->use_btlbox == FALSE )
  {
    party = GAMEDATA_GetMyPokemon( gamedata );
  }
  else
  {
    party = wbtsys->btl_box_party;
  }
  GF_ASSERT( party );
  return party;
}

//------------------------------------------------------------------
/**
 * @brief ポケモンリスト呼び出しパラメータの設定
 */
//------------------------------------------------------------------
void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data )
{
  PL_LIST_TYPE list_type;

  PLIST_DATA_Init( plist_data, gamedata, PL_MODE_BATTLE_SUBWAY, getUsePokeParty( wbtsys, gamedata ) );

  switch ( wbtsys->rule )
  {
  default:
    GF_ASSERT(0);
    /* FALL THROUGH */

  case WBTRULE_SINGLE:
    list_type = PL_TYPE_SINGLE;
    break;
  case WBTRULE_DOUBLE:
    list_type = PL_TYPE_DOUBLE;
    break;
  case WBTRULE_TRIPLE:
    list_type = PL_TYPE_TRIPLE;
    break;
  case WBTRULE_ROTATION:
    list_type = PL_TYPE_ROTATION;
    break;
  }
  plist_data->reg = (void *)WBTSYS_GetRegulation( wbtsys );
  plist_data->regEx = (void *)WBTSYS_GetRegulationEx( wbtsys );
  plist_data->type = list_type;
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
 */
//------------------------------------------------------------------
void WBTSYS_SetupMyParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * entry_party;
  POKEPARTY * base_party;
  POKEMON_PARAM * entry_pp;
  u32 cnt, max;

  entry_party = wbtsys->my_party;
  PokeParty_InitWork( entry_party );
  base_party = getUsePokeParty( wbtsys, gamedata );
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
  GFL_HEAP_FreeMemory( entry_pp );
}


