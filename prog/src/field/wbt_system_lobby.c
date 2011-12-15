//============================================================================================
/**
 * @file  wbt_system_lobby.c
 * @brief   WBTシステム：ロビー内専用処理
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



//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  ///WBT AI全部入り定義
  WBT_AI_BIT_ALL  = WBT_AI_BIT_BASIC | WBT_AI_BIT_STRONG | WBT_AI_BIT_EXPERT | WBT_AI_BIT_DOUBLE,
};

//============================================================================================
//============================================================================================
static void WBTTRAINER_SetPlayer( WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetRomTrainer(
    const WBTSYS * wbtsys, HEAPID heapID, WBTTRAINER * wbt_tr, WBTROM_TRID tr_id );
static void WBTTRAINER_SetDLTrainer( WBTTRAINER * wbt_tr,
    const WBTDL_CHARACTER * dl_char, u32 dl_tr_id, u8 pri );
static void WBTTRAINER_SetDLHybridTrainer( WBTTRAINER * wbt_tr,
    const WBTSYS * wbtsys, WBTROM_TRID tr_id,
    const WBTDL_CHARACTER * dl_char, u32 dl_tr_id, u8 pri );

//============================================================================================
//
//  対戦データ生成処理関連
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：基本
 */
//------------------------------------------------------------------
static void WBTTRAINER_Set( WBTTRAINER * wbt_tr,
    WBT_TRTYPE type, u8 tr_id, u8 sex, u8 pri, u8 pow_rnd,
    u8 dl_tr_id,
    BOOL has_ace, PokeType poketype, u16 mmdl_id, u16 btl_tr_type, u8 ai_bit )
{
  GF_ASSERT( type < WBT_TRTYPE_MAX );
  GF_ASSERT( sex == PM_MALE || sex == PM_FEMALE );
  GF_ASSERT( pri < WBT_TRPRI_MAX );
  GF_ASSERT( pow_rnd < 0x20 );
  GF_ASSERT( dl_tr_id < WBTDL_CHAR_MAX );
  GF_ASSERT( poketype <= POKETYPE_MAX );
  GF_ASSERT( btl_tr_type < TRTYPE_MAX );

  wbt_tr->wbt_tr_type = type;
  wbt_tr->tr_id       = tr_id;
  wbt_tr->sex         = sex;
  wbt_tr->pri         = pri;
  wbt_tr->pow_rnd     = pow_rnd;
  wbt_tr->dl_tr_id    = dl_tr_id;
  wbt_tr->has_ace     = has_ace;
  wbt_tr->poketype    = poketype;
  wbt_tr->mmdl_id     = mmdl_id;
  wbt_tr->btl_tr_type = btl_tr_type;
  wbt_tr->ai_bit      = ai_bit;
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
      tr_id, tr_rom_data.sex, tr_rom_data.priority, tr_rom_data.power_rnd,
      0, has_ace, tr_rom_data.type,
      tr_rom_data.mmdl_id, tr_rom_data.tr_type, WBT_AI_BIT_ALL );
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
  u8 pri = WBT_TRPRI_LEADER;
  if ( WBTSAVE_GetWinCount( wbtsave, WBTWIN_COUNT_CHAMPION ) != 0 )
  {
    pri = WBT_TRPRI_CHAMP;
  }
  if ( sex == PM_FEMALE )
  {
    mmdl_id = SWHEROINE;
    btl_tr_type = TRTYPE_HEROINE;
  }
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_PLAYER,
      0, sex, pri, 0, 0, FALSE, POKETYPE_NORMAL, mmdl_id, btl_tr_type, WBT_AI_BIT_BASIC );
  MyStatus_CopyNameString( mystat, wbt_tr->name );
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
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_DL, 0, sex,
      pri, 0, dl_tr_id, FALSE, POKETYPE_NORMAL, mmdl_id, trtype, dl_char->ai_bit );

  GFL_STR_SetStringCodeOrderLength( wbt_tr->name, dl_char->name, SAVELEN_TRAINER_NAME );
  GFL_STR_SetStringCodeOrderLength( wbt_tr->title, dl_char->title, WBTDL_TRAINER_TITLE_LEN );
}

//------------------------------------------------------------------
/**
 * @brief WBTTRAINER設定：ダウンロードデータ+ROMデータから
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetDLHybridTrainer( WBTTRAINER * wbt_tr,
    const WBTSYS * wbtsys, WBTROM_TRID tr_id,
    const WBTDL_CHARACTER * dl_char, u32 dl_tr_id, u8 pri )
{
  WBT_TRDATA tr_rom_data;
  BOOL has_ace;

  GF_ASSERT( dl_tr_id < WBTDL_CHAR_MAX );

  WBTTOOL_GetRomTrainerData( wbtsys->rom_trdata, tr_id, &tr_rom_data );
  has_ace = tr_rom_data.ace_poke_id != 0 ? TRUE: FALSE;
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_DLHYBRID,
      tr_id, tr_rom_data.sex, tr_rom_data.priority, tr_rom_data.power_rnd,
      dl_tr_id, FALSE, tr_rom_data.type,
      tr_rom_data.mmdl_id, tr_rom_data.tr_type, WBT_AI_BIT_ALL );
  WBTTOOL_GetRomTrainerName( wbtsys->heapID, tr_id, wbt_tr->name );
}


//------------------------------------------------------------------
/**
 * @brief トレーナーID配列から実トレーナーデータをセットする
 *
 * @param[in,out] wbtsys
 * @param[in]     gamedata
 * @param[in]     tr_list
 */
//------------------------------------------------------------------
void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const WBT_TRCODE * tr_list )
{
  int i, count;
  for ( i = 0, count = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTTRAINER * wbt_tr = &wbtsys->wbt_trainer[ i ];
    switch ( tr_list[i].type )
    {
    case WBT_TRCODE_TYPE_PLAYER:
      WBTTRAINER_SetPlayer( wbt_tr, gamedata );
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->tr_id, wbt_tr->title );
      break;

    case WBT_TRCODE_TYPE_DL1:
    case WBT_TRCODE_TYPE_DL2:
    case WBT_TRCODE_TYPE_DL3:
    case WBT_TRCODE_TYPE_DL4:
      {
        const WBTDL_CHARACTER * dl_char;
        // DLタイプは連番になっている
        u32 dl_tr_id = tr_list[i].type - WBT_TRCODE_TYPE_DL1;
        dl_char = WBTDL_DATA_GetCharacter( &(wbtsys->wbtdl_data), dl_tr_id );
        if ( tr_list[i].tr_id == 0 )
        {
          WBTTRAINER_SetDLTrainer( wbt_tr, dl_char, dl_tr_id, 3 );
        } else {
          WBTROM_TRID tr_id = tr_list[i].tr_id - 1; //　1 org --> 0 org に変換する
          WBTTRAINER_SetDLHybridTrainer( wbt_tr, wbtsys, tr_id, dl_char, dl_tr_id, 3 );
          WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, tr_id, wbt_tr->title );
        }
      }
      break;
    default:
      WBTTRAINER_SetRomTrainer( wbtsys, wbtsys->heapID, wbt_tr, tr_list[i].tr_id );
      if ( tr_list[i].tr_id == WBTROM_TRID_RIVAL )
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

//------------------------------------------------------------------
/**
 * @brief 対戦組み合わせの確定
 */
//------------------------------------------------------------------
void WBTSYS_MakeMatch( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  int i;
  WBT_TRCODE tr_list[ WBT_TRAINER_NUM ];
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
//============================================================================================
//============================================================================================
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
//
//
//  ダウンロードデータ読み込み関連
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 指定IDのダウンロードデータをチェック
 * @param[in] gamedata
 * @param[in] dl_id
 * @return  BOOL  TRUEのとき有効なデータ、FALSEのとき無効
 */
//------------------------------------------------------------------
static BOOL checkWBTDownLoad( GAMEDATA * gamedata, WBT_DLID dl_id )
{
  SAVE_CONTROL_WORK* save;
  WBTEXSAVE * save_p;
  SAVE_EXTRA_ID save_id;

  LOAD_RESULT load_res;
  BOOL result = FALSE;

  save_id = SAVE_EXTRA_ID_WBT_00 + dl_id - WBT_DLID_START;

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
/**
 * @brief 選択可能なダウンロードデータをカウントする
 * @param[in] wbtsys
 * @param[in] gamedata
 * @return  u32
 */
//------------------------------------------------------------------
u32 WBTSYS_GetDownLoadCount( const WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  int count = 0;
  WBT_DLID dl_id;
  for ( dl_id = WBT_DLID_START; dl_id < WBT_DLID_START + WBT_DOWNLOAD_NUM; dl_id ++ )
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

  if ( wbtsys->select_dl_data == WBT_DLID_NULL )
  {
    WBTSYS_UnloadDLData( wbtsys );
    return;
  }

  save_id = SAVE_EXTRA_ID_WBT_00 + wbtsys->select_dl_data - WBT_DLID_START;

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
//
//
//  ポケモン選択画面関連
//
//
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
/**
 * @brief 使用する自分のPOKEPARTYを取得する
 */
//------------------------------------------------------------------
POKEPARTY * WBTSYS_GetUsePokeParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * party;
  if ( wbtsys->wbtcup == WBTCUP_RENTAL || wbtsys->wbtcup == WBTCUP_RENTALMASTER )
  {
    party = wbtsys->sp_party;
  }
  else
  {
    if ( wbtsys->use_btlbox == FALSE )
    {
      party = GAMEDATA_GetMyPokemon( gamedata );
    }
    else
    {
      party = wbtsys->btl_box_party;
    }
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

  PLIST_DATA_Init( plist_data, gamedata, PL_MODE_BATTLE_SUBWAY,
      WBTSYS_GetUsePokeParty( wbtsys, gamedata ) );

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
/**
 * @brief ポケモンステータス呼び出しパラメータの設定
 */
//------------------------------------------------------------------
void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus )
{
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gamedata );
  GFL_STD_MemClear32( pstatus, sizeof(PSTATUS_DATA) );
  pstatus->ppd = WBTSYS_GetUsePokeParty( wbtsys, gamedata );
  pstatus->ppt = PST_PP_TYPE_POKEPARTY;
  pstatus->max = PokeParty_GetPokeCount( pstatus->ppd );
  pstatus->mode = PST_MODE_NORMAL;
  pstatus->page = PPT_INFO;
  pstatus->game_data = gamedata;

  pstatus->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
}


