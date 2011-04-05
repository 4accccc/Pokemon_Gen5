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

//#include "poke_tool/regulation_def.h"
//#include "wbt_trlist_struct.h"  //WBT_TRDATA

//#include "arc_def.h"
//#include "arc/wbt/wbt_trlist.naix"
//#include "arc/fieldmap/fldmmdl_mdlres.naix"

//#include "msg/msg_wbt_tournament.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

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
static const u16 sample_trid[WBT_TRAINER_NUM] = {
  12,//アロエ
  53,//モブ
  11,//デント
  0xffff,//プレイヤー
  55,//モブ
  56,//モブ
  18,//シロナ
  13,//ハチク
};

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_Setup_Debug( WBTSYS * wbtsys, GAMEDATA * gamedata, WBTSTAGE stage )
{
  int i;
  wbtsys->type = WBTTYPE_ALL;
  wbtsys->rule = WBTRULE_SINGLE;
  wbtsys->stage = stage;

  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER * wbt_tr = &wbtsys->wbt_trainer[ i ];
    if ( sample_trid[i] == 0xffff )
    {
      WBTTRAINER_SetPlayer( wbt_tr, gamedata );
    } else {
      WBTTRAINER_SetRomTrainer( wbt_tr, sample_trid[i] );
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
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_Set( WBTTRAINER * wbt_tr, WBT_TRTYPE type, u8 tr_id, u8 sex, u8 pri )
{
  wbt_tr->wbt_type = type;
  wbt_tr->tr_id = tr_id;
  wbt_tr->sex = sex;
  wbt_tr->pri = pri;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_SetRomTrainer( WBTTRAINER * wbt_tr, u8 tr_id )
{
  WBT_TRDATA tr_rom_data;
  WBT_TRDATA_Copy( HEAPID_FIELDMAP, tr_id, &tr_rom_data );
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_ROM, tr_id, tr_rom_data.sex, tr_rom_data.priority );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

  u8 sex = MyStatus_GetMySex( mystat );
  u8 pri = 3;
  if ( WBTSAVE_GetFlag( wbtsave, WBTSAVE_FLAG_WIN_CHAMPION_TOURNAMENT ) == TRUE )
  {
    pri = 4;
  }
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_PLAYER, 0, sex, pri );
}

//============================================================================================
//
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @todo  ダウンロードデータへの対応（引数がWBTSYSになる）
 */
//------------------------------------------------------------------
void WBTTOOL_SetTournamentName( WBTTYPE type, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;

  switch ( type )
  {
  default:
  case WBTTYPE_DOWNLOAD:
    //仮：ダウンロードタイトル名には未対応
  case WBTTYPE_NULL:
    GF_ASSERT(0);
    /* FALL THROUGH */
  case WBTTYPE_ALL:
    msg_id = msg_wbtsys_title_01;
    break;
  case WBTTYPE_CHAMPION:
    msg_id = msg_wbtsys_title_03;
    break;
  case WBTTYPE_POKETYPE:
    msg_id = msg_wbtsys_title_02;
    break;
  }
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//------------------------------------------------------------------
/**
 * @brief WBTルール名の取得
 */
//------------------------------------------------------------------
void WBTTOOL_SetRuleName( WBTRULE rule, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;
  switch ( rule )
  {
  case WBTRULE_NULL:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case WBTRULE_SINGLE:
    msg_id = msg_wbtsys_rule_single;
    break;
  case WBTRULE_DOUBLE:
    msg_id = msg_wbtsys_rule_double;
    break;
  case WBTRULE_TRIPLE:
    msg_id = msg_wbtsys_rule_triple;
    break;
  case WBTRULE_ROTATION:
    msg_id = msg_wbtsys_rule_rotation;
    break;
  }
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTTOOL_GetEnemyTrainerSex( WBTSYS * wbtsys )
{
  return PM_MALE; //仮
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 WBTTOOL_GetEnemyTrainerMMdlCode( WBTSYS * wbtsys )
{
  return LEADER7; //仮
}
//------------------------------------------------------------------
/**
 * @todo  ダウンロードデータへの対応
 */
//------------------------------------------------------------------
void WBTTOOL_SetEnemyTrainerName( WBTSYS * wbtsys, STRBUF * strbuf )
{
  if ( wbtsys->type == WBTTYPE_DOWNLOAD 
      || wbtsys->type == WBTTYPE_NULL )
  {
  }
  else
  {
    HEAPID heapID = wbtsys->heapID;
    GFL_MSGDATA     *man;
    u16 msg_id = 13; //仮・本当は進行状況から判断する
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_trname_dat, heapID );
    if( man )
    {
      GFL_MSG_GetString( man, msg_id, strbuf );
      GFL_MSG_Delete( man );
    }
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



