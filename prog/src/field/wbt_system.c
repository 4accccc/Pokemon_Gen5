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

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE


//============================================================================================
//============================================================================================
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr );
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetRomTrainer( WBTTRAINER * wbt_tr, u8 tr_id );

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
//------------------------------------------------------------------
void WBTSYS_SetupTrainer_Debug( WBTSYS * wbtsys, GAMEDATA * gamedata, const u16 * tr_tbl )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER * wbt_tr = &wbtsys->wbt_trainer[ i ];
    if ( tr_tbl[i] == 0xffff )
    {
      WBTTRAINER_SetPlayer( wbt_tr, gamedata );
    } else {
      WBTTRAINER_SetRomTrainer( wbt_tr, tr_tbl[i] );
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
  GF_ASSERT( no <= WBT_DOWNLOAD_NUMBER );
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

//============================================================================================
//============================================================================================
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
//------------------------------------------------------------------
static void WBTTRAINER_Init( WBTTRAINER * wbt_tr )
{
  wbt_tr->wbt_type = WBT_TRTYPE_ROM;
  wbt_tr->sex = PM_MALE;
  wbt_tr->pri = 1;
  wbt_tr->tr_id = 0;
  wbt_tr->poketype = POKETYPE_NORMAL;
  wbt_tr->mmdl_id = SWHERO;
  wbt_tr->tr_type = TRTYPE_HERO;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Set( WBTTRAINER * wbt_tr,
    WBT_TRTYPE type, u8 tr_id, u8 sex, u8 pri, u8 poketype, u16 mmdl_id, u16 tr_type )
{
  wbt_tr->wbt_type = type;
  wbt_tr->tr_id = tr_id;
  wbt_tr->sex = sex;
  wbt_tr->pri = pri;
  wbt_tr->poketype = poketype;
  wbt_tr->mmdl_id = mmdl_id;
  wbt_tr->tr_type = tr_type;
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
  u16 tr_type = TRTYPE_HERO;
  u8 sex = MyStatus_GetMySex( mystat );
  u8 pri = 3;
  if ( WBTSAVE_GetFlag( wbtsave, WBTSAVE_FLAG_WIN_CHAMPION_TOURNAMENT ) == TRUE )
  {
    pri = 4;
  }
  if ( sex == PM_FEMALE )
  {
    mmdl_id = SWHEROINE;
    tr_type = TRTYPE_HEROINE;
  }
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_PLAYER,
      0, sex, pri, POKETYPE_NORMAL, mmdl_id, tr_type );
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
u16 WBTTRAINER_GetTrainerType( const WBTTRAINER * wbt_tr )
{
  return wbt_tr->tr_type;
}


//============================================================================================
//
//
//
//============================================================================================
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
//------------------------------------------------------------------
POKEPARTY * WBTSYS_GetBattleParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  return getUsePokeParty( wbtsys, gamedata );
}



