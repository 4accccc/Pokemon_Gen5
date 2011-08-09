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
static void WBTTRAINER_SetRomTrainer( HEAPID heapID, WBTTRAINER * wbt_tr, u8 tr_id );
static void WBTTRAINER_SetWBPlayer( HEAPID heapID, WBTTRAINER * wbt_tr, GAMEDATA * gamedata );
static void WBTTRAINER_SetDLTrainer(
    WBTSYS * wbtsys, WBTTRAINER * wbt_tr, GAMEDATA * gamedata, u32 dl_tr_id, u8 pri );

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
  return wbtsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_Delete( WBTSYS * wbtsys )
{
  int i;
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
 * @todo  DLトーナメントの優先度反映を実装する
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
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->btl_tr_type, wbt_tr->title );
      break;
    case WBT_TRCODE_WBHERO:
      WBTTRAINER_SetWBPlayer( wbtsys->heapID, wbt_tr, gamedata );
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->btl_tr_type, wbt_tr->title );
      break;
    case WBT_TRCODE_DL1:
    case WBT_TRCODE_DL2:
    case WBT_TRCODE_DL3:
    case WBT_TRCODE_DL4:
      WBTTRAINER_SetDLTrainer( wbtsys, wbt_tr, gamedata, tr_list[i] - WBT_TRCODE_DL1, 3 );
      break;
    default:
      WBTTRAINER_SetRomTrainer( wbtsys->heapID, wbt_tr, tr_list[i] );
      if ( tr_list[i] == WBTTOOL_GetRivalTrainerID() )
      { //ライバルの場合はセーブデータから名前を引っ張る
        SAVE_CONTROL_WORK * sv_ctrl_work = GAMEDATA_GetSaveControlWork( gamedata );
        ETC_SAVE_WORK * etcsave = SaveData_GetEtc( sv_ctrl_work );
        GFL_STR_SetStringCode( wbt_tr->name, EtcSave_GetRivalName( etcsave ) );
      }
      WBTTOOL_GetRomTrainerTypeName( wbtsys->heapID, wbt_tr->tr_id, wbt_tr->title );
      break;
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
  return 100;
}

//------------------------------------------------------------------
/**
 * @brief トーナメント名を取得する
 */
//------------------------------------------------------------------
void WBTSYS_GetTournamentName( WBTSYS * wbtsys, WBTCUP wbtcup, STRBUF * strbuf )
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
static void WBTTRAINER_SetRomTrainer( HEAPID heapID, WBTTRAINER * wbt_tr, u8 tr_id )
{
  WBT_TRDATA tr_rom_data;
  WBTTOOL_GetTrRomData( tr_id, &tr_rom_data );
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_ROM,
      tr_id, tr_rom_data.sex, tr_rom_data.priority, tr_rom_data.type,
      tr_rom_data.mmdl_id, tr_rom_data.tr_type );
  WBTTOOL_GetRomTrainerName( heapID, tr_id, wbt_tr->name );
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
      0, sex, pri, POKETYPE_NORMAL, mmdl_id, btl_tr_type );
  MyStatus_CopyNameString( mystat, wbt_tr->name );
}

//------------------------------------------------------------------
/**
 * @todo  データ引継ぎ仕様に対応していない
 */
//------------------------------------------------------------------
static void WBTTRAINER_SetWBPlayer( HEAPID heapID, WBTTRAINER * wbt_tr, GAMEDATA * gamedata )
{
  u8 tr_id;
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );

  tr_id = WBTTOOL_GetBWRomTrainerID( MyStatus_GetMySex( mystat ) );
  WBTTRAINER_SetRomTrainer( heapID, wbt_tr, tr_id );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTTRAINER_SetDLTrainer(
    WBTSYS * wbtsys, WBTTRAINER * wbt_tr, GAMEDATA * gamedata, u32 dl_tr_id, u8 pri )
{
  u8 sex;
  u32 trtype, mmdl_id;
  const WBTDL_CHARACTER * dl_char;
  
  dl_char = WBTDL_DATA_GetCharacter( &(wbtsys->wbtdl_data), dl_tr_id );

  trtype = WBTTOOL_GetDLTrtype( dl_char->trtype_id );
  mmdl_id = FBI_TOOL_GetTrainerOBJCode( trtype );
  sex = TT_TrainerTypeSexGet( trtype );
  WBTTRAINER_Set( wbt_tr, WBT_TRTYPE_DL, dl_tr_id, sex,
      pri, POKETYPE_NORMAL, mmdl_id, trtype );

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
static BOOL WBTTRAINER_HasAce( const WBTTRAINER * wbt_tr )
{
  WBT_TRDATA tr_rom_data;
  if (wbt_tr->wbt_tr_type != WBT_TRTYPE_ROM ) return FALSE;
  WBTTOOL_GetTrRomData( wbt_tr->tr_id, &tr_rom_data );
  return ( tr_rom_data.ace_poke_id != 0 );
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
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->sex;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の動作モデル表示コードを取得する
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentMMdlObjCode( WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->mmdl_id;
}
//------------------------------------------------------------------
/**
 * @brief 対戦相手の名前を取得する
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentName( WBTSYS * wbtsys, STRBUF * strbuf )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  WBTTRAINER_CopyName( wbt_tr, strbuf );
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナータイプを取得する
 * @param wbtsys
 * @return  u16 tr_type(include/tr_tool/trtype_def.h参照）
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentTrainerType( WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->btl_tr_type;
}

//------------------------------------------------------------------
/**
 * @brief 戦闘BGMの取得
 * @param[in]   wbtsys
 * @param[out]  btl_bgm
 * @param[out]  win_bgm
 *
 * @todo  ダウンロードデータへの対応
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentBGM( WBTSYS * wbtsys, u16 *btl_bgm, u16 *win_bgm )
{
  switch ( wbtsys->wbtcup )
  {
  case WBTCUP_DOWNLOAD:
    *btl_bgm = SEQ_BGM_VS_WBT;
    *win_bgm = SEQ_BGM_WIN_WBT;
    //*btl_bgm = SEQ_BGM_VS_WCS;
    //*win_bgm = SEQ_BGM_WIN5;
    break;
  default:
    {
      const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
      WBT_TRDATA tr_rom_data;
      GF_ASSERT( wbt_tr->wbt_tr_type == WBT_TRTYPE_ROM );
      WBTTOOL_GetTrRomData( wbt_tr->tr_id, &tr_rom_data );
      *btl_bgm = tr_rom_data.btl_bgm_id;
      *win_bgm = tr_rom_data.win_bgm_id;
    }
    break;
  }
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
BOOL WBTSYS_GetDownLoadCount( WBTSYS * wbtsys, GAMEDATA * gamedata )
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
  GF_ASSERT( wbtsys->rule != WBTRULE_NULL && wbtsys->rule <= WBTRULE_MAX );
  return tbl[ wbtsys->rule - 1 ];
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
  }

#ifdef PM_DEBUG
  WBTSYS_DEBUG_PrintRegulation( &wbtsys->wbtreg.base_reg );
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
static u8 WBTSYS_GetPartyMax( WBTSYS * wbtsys )
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
  max = WBTSYS_GetPartyMax( wbtsys );
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
    if ( pos == 0 )
    {
      GF_ASSERT_MSG( 0, "参戦するポケモンが正しくありません!!\n" );
      pos = 1;
    }
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
  PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), entry_party );
  GFL_HEAP_FreeMemory( entry_pp );

  return entry_party;
}

//============================================================================================
//
//
//  ダウンロード手持ちデータの設定処理
//
//
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkDLPokemon( const WBTDL_POKEMON * dl_poke )
{
  if ( dl_poke->monsno == 0 || dl_poke->level == 0 ) return FALSE;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @param[out]  pp
 * @param[in]   dl_poke
 * @param[in]   tempHeapID
 */
//--------------------------------------------------------------
static void setDLPokemon( POKEMON_PARAM * pp, const WBTDL_POKEMON * dl_poke, HEAPID tempHeapID )
{
  int i;
  u64 personal_rnd;

  PP_Clear( pp );

  personal_rnd = ( dl_poke->rare_flag == TRUE ? PTL_SETUP_RND_RARE : PTL_SETUP_RND_AUTO );
  PP_SetupEx( pp, dl_poke->monsno, dl_poke->level, PTL_SETUP_ID_AUTO,
      (dl_poke->pow_rnd & 0x3FFFFFFF), personal_rnd );

  //form_no
  PP_Put( pp, ID_PARA_form_no, dl_poke->form_no );

  //装備アイテム設定
  PP_Put( pp, ID_PARA_item, dl_poke->item_no );
  //技設定
  for ( i = 0; i < PTL_WAZA_MAX; i ++ )
  {
    u8 point;
    u8 buf8;
    PP_Put( pp, ID_PARA_waza1 + i, dl_poke->waza_id[i] );
    buf8 = (dl_poke->pp_count >> (i*2))&0x03;
    PP_Put( pp, ID_PARA_pp_count1 + i, buf8);
    point = PP_Get( pp, ID_PARA_pp_max1 + i, NULL );
    PP_Put( pp, ID_PARA_pp1 + i, point );
  }
  //経験値設定
  PP_Put( pp, ID_PARA_hp_exp, dl_poke->hp_exp );
  PP_Put( pp, ID_PARA_pow_exp, dl_poke->pow_exp );
  PP_Put( pp, ID_PARA_def_exp, dl_poke->def_exp );
  PP_Put( pp, ID_PARA_agi_exp, dl_poke->agi_exp );
  PP_Put( pp, ID_PARA_spepow_exp, dl_poke->spepow_exp );
  PP_Put( pp, ID_PARA_spedef_exp, dl_poke->spedef_exp );

  //特性設定
  PP_Put( pp, ID_PARA_speabino, dl_poke->tokusei );
  //なつきど設定
  PP_Put( pp, ID_PARA_friend, dl_poke->friend_rate );

  //性格セット
  PP_SetSeikaku( pp, dl_poke->seikaku );

  { //ニックネームセット
    STRBUF * nickname;
    nickname = GFL_STR_CreateBuffer( MONS_NAME_SIZE + EOM_SIZE, tempHeapID );
    GFL_STR_SetStringCodeOrderLength( nickname, dl_poke->nickname, MONS_NAME_SIZE + EOM_SIZE );
    PP_Put( pp, ID_PARA_nickname, (u32)nickname );
    GFL_STR_DeleteBuffer( nickname );
  }
  //捕まえたボール
  PP_Put( pp, ID_PARA_get_ball, dl_poke->ball_id );

  PP_Renew( pp );
}

//--------------------------------------------------------------
/**
 * @param[out]  party
 * @param[in]   wbtdl_data
 * @param[in]   dl_tr_id
 * @param[in]   num
 * @param[in]   tempHeapID
 * 
 * @todo  データ異常の場合のエラー処理
 */
//--------------------------------------------------------------
static void setDLPokeParty( POKEPARTY * party,
    const WBTDL_DATA * wbtdl_data, u32 dl_tr_id, u32 num, HEAPID tempHeapID )
{
  int i;
  POKEMON_PARAM * entry_pp;

  PokeParty_Init( party, TEMOTI_POKEMAX );

  entry_pp = GFL_HEAP_AllocMemoryLo( tempHeapID, POKETOOL_GetWorkSize() );

  for ( i = 0; i < num; i ++ )
  {
    const WBTDL_POKEMON * dl_poke = WBTDL_DATA_GetPokemon( wbtdl_data, dl_tr_id, i );
    if ( checkDLPokemon( dl_poke ) )
    {
      setDLPokemon( entry_pp, &dl_poke[i], tempHeapID );
      PokeParty_Add( party, entry_pp );
    }
    else
    {
      //万が一、データ異常の場合の救済処理：ロムから引っ張る？
    }
  }
  GFL_HEAP_FreeMemory( entry_pp );
}

//============================================================================================
//============================================================================================
#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/work_define.h"  //WK_SYS_FIRST_POKETYPE
#include "../../../resource/fldmapdata/script/user_define.h"    //FIRST_POKETYPE_

//--------------------------------------------------------------
/**
 * @brief 最初の3体選択結果でトレーナーIDを振り分ける
 */
//--------------------------------------------------------------
static u32 getWBTRivalTrainerID( GAMESYS_WORK * gsys )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( GAMESYSTEM_GetGameData( gsys ) );
  u16 * first_choice = EVENTWORK_GetEventWorkAdrs( evwork, WK_SYS_FIRST_POKETYPE );
  switch ( *first_choice )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case FIRST_POKETYPE_KUSA:
    return TRID_SWRIVAL_07;
  case FIRST_POKETYPE_HONOO:
    return TRID_SWRIVAL_08;
  case FIRST_POKETYPE_MIZU:
    return TRID_SWRIVAL_09;
  }
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 GetRand( GFL_STD_RandContext *rand_context )
{
  if( rand_context == NULL )
  {
    return( GFUser_GetPublicRand(0xffffffff)/65535 );
  }
  else
  {
    return( GFL_STD_Rand( rand_context, 0xffffffff )/65535 );
  }
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void GetPokemonRomData(
    ARCID poke_arc_id,BSUBWAY_POKEMON_ROM_DATA *prd,int index)
{
//  index += BSW_PM_ARCDATANO_ORG;
  GFL_ARC_LoadData( (void*)prd, poke_arc_id, index );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  ROM_POKE_MAX = 10,

  NG_POKE_MAX = 10,
  NG_ITEM_MAX = 10,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  u16 ng_mons_no[ NG_POKE_MAX ];
  u16 ng_item_no[ NG_ITEM_MAX ];
}WBT_MONS_CONDITION;

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  u8 available;
  u16 index_no;
  BSUBWAY_POKEMON_ROM_DATA prd;
}SELECT_DATA;

//--------------------------------------------------------------
//--------------------------------------------------------------
static void initMonsCondition( WBT_MONS_CONDITION * cond )
{
  GFL_STD_MemClear( cond, sizeof(WBT_MONS_CONDITION) );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void addNGItem( WBT_MONS_CONDITION * cond, u16 item_no )
{
  int i;
  for ( i = 0; i < NG_ITEM_MAX; i ++ )
  {
    if (cond->ng_item_no[ i ] == 0 )
    {
      cond->ng_item_no[ i ] = item_no;
      return;
    }
  }
  GF_ASSERT(0);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void addNGMons( WBT_MONS_CONDITION * cond, u16 mons_no )
{
  int i;
  for ( i = 0; i < NG_POKE_MAX; i ++ )
  {
    if (cond->ng_mons_no[ i ] == 0 )
    {
      cond->ng_mons_no[ i ] = mons_no;
      return;
    }
  }
  GF_ASSERT(0);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkNGData( const WBT_MONS_CONDITION * cond, const SELECT_DATA * select_data )
{
  int i;
  //データ無し→チェックOKする
  if ( cond == NULL ) return TRUE;

  for ( i = 0; i < NG_POKE_MAX; i ++ )
  {
    u16 mons_no = select_data->prd.mons_no;
    if ( mons_no != 0 && mons_no == cond->ng_mons_no[i] ) return TRUE;
  }
  for ( i = 0; i < NG_POKE_MAX; i ++ )
  {
    u16 item_no = select_data->prd.item_no;
    if ( item_no != 0 && item_no == cond->ng_item_no[i] ) return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief WBTのロムトレーナーのポケモンを決める
 *
 * @param[in]     trd           BS形式トレーナーデータ
 * @param[out]    pwd           BSUBWAY_POKEMON構造体
 * @param[in]     cnt           トレーナーに持たせるポケモンの数
 * @param[in]     id            ポケモンに指定するID
 * @param[in]     poke_arc_id   ポケモンデータが格納されているアーカイブID
 * @param[in]     cond          ペアトレーナーポケモン (NULLだとチェックなし）
 * @param[in]     rand_context  乱数値ポインタ(NULL GFUser_GetPublicRandを使用
 * @param[in]     pow_rnd       ポケモンにセットするpow_rnd値
 * @param[in]     heapID        ヒープID
 *
 * (1)BSUBWAY_TRAINER_ROM_DATAを解析、必要なBSUBWAY_POKEMON_ROM_DATAを事前にselect_dataとして読み込む
 * (2)select_data中のNGのアイテム、ポケモンを含むデータを使用できないようにする
 * (3)select_dataの利用可能なものをランダムに選択、同じポケモン・アイテムのデータを使えないようにする
 * (4)　(3)を必要なだけ繰り返し、完了
 */
//--------------------------------------------------------------
static void setBSubwayPokemonParam(
    const BSUBWAY_TRAINER_ROM_DATA *trd,
     BSUBWAY_POKEMON *pwd, u8 cnt, u32 id,
    ARCID poke_arc_id, const WBT_MONS_CONDITION * cond,
    GFL_STD_RandContext *rand_context ,u8 pow_rnd, HEAPID heapID )
{
  int  i,j;
  u8  poke_index;
  int  set_count;
  u8 selected[6];

  BSUBWAY_POKEMON_ROM_DATA * prd_d;
  SELECT_DATA * select_data;

  select_data = GFL_HEAP_AllocMemoryLo( heapID, sizeof(SELECT_DATA) * ROM_POKE_MAX );
  GF_ASSERT( trd->use_poke_cnt < ROM_POKE_MAX );

  GFL_STD_MemClear( selected, 6 );

  for ( i = 0; i < ROM_POKE_MAX; i ++ )
  { 
    SELECT_DATA * rd = &select_data[i];
    rd->available = FALSE;

    if ( i < trd->use_poke_cnt )
    {
      rd->available = TRUE;
      rd->index_no = trd->use_poke_table[ i ];
      GetPokemonRomData( poke_arc_id, &rd->prd, rd->index_no );
      if ( checkNGData( cond, rd ) == TRUE )
      {
        rd->available = FALSE;
      }
      TAMADA_Printf("Index(%3d) monsno(%3d) itemno(%3d) =:%d\n",
          rd->index_no, rd->prd.mons_no, rd->prd.item_no, rd->available );
    }
  }
#ifdef  PM_DEBUG
  for ( j = 0, i = 0; i < ROM_POKE_MAX; i ++ )
  {
    SELECT_DATA * rd = &select_data[i];
    if (rd->available) j ++;
  }
  //抽選対象が必要な数に満たない場合Assert
  GF_ASSERT( j >= cnt );
#endif

  set_count=0;

  while(set_count!=cnt)
  {
    j = GetRand( rand_context ) % ( trd->use_poke_cnt - set_count );
    for ( poke_index = 0; poke_index < ROM_POKE_MAX; poke_index ++ )
    {
      if ( select_data[poke_index].available )
      {
        if ( j == 0 ) break;
        j --;
      }
    }
    GF_ASSERT( poke_index < ROM_POKE_MAX );

    prd_d = &select_data[ poke_index ].prd;

    for ( i = 0; i < ROM_POKE_MAX; i ++ )
    {
      if ( i == poke_index ) continue;
      if ( select_data[ i ].prd.mons_no == prd_d->mons_no )
      { //同一のポケモンを含むデータは使えなくなる
        select_data[ i ].available = FALSE;
      }
      if ( select_data[ i ].prd.item_no == prd_d->item_no )
      { //同一のアイテムを持つデータは使えなくなる
        select_data[ i ].available = FALSE;
      }
    }
    //選ばれたデータは次の抽選で使えない
    select_data[ poke_index ].available = FALSE;

    selected[ set_count ] = poke_index;
    set_count++;
  }

  for(i=0;i<set_count;i++)
  {
    u32 index_no = select_data[ selected[i] ].index_no;
    FBI_TOOL_MakePokemonParam(&(pwd[i]),
      poke_arc_id, index_no, id, 0, pow_rnd, i, FALSE, heapID);
  }

  GFL_HEAP_FreeMemory( select_data );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const u8 pri2pow_table[5] = {
  0x10, 0x12, 0x14, 0x1a, 0x1f
};

//-----------------------------------------------------------------------------
/**
 * @param bt_trd
 * @param btpwd
 * @param wbt_tr
 * @param cnt
 * @param inHeapID
 * @todo  バトルサブウェイは４体しかポケモンを選択できない。
 */
//-----------------------------------------------------------------------------
static void makeRomTrainerData(
    BSUBWAY_TRAINER * bt_trd,
    BSUBWAY_POKEMON * btpwd,
    WBTCUP wbtcup, const WBTTRAINER * wbt_tr,
    u8 cnt, HEAPID inHeapID)
{
  BSUBWAY_PARTNER_DATA * tr_data;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  u8 pow_rnd;
  u8 tr_pri   = WBTTRAINER_GetPriority(wbt_tr);
  u8 tr_no    = WBTTRAINER_GetID(wbt_tr);
  u32 arc_id  = WBTTOOL_GetRomArcID( wbtcup );
  u32 id;
  GFL_STD_RandContext * rand_context = NULL;
  WBT_MONS_CONDITION cond;

  initMonsCondition( &cond );
  
  // パワー乱数を決定する
  pow_rnd= pri2pow_table[ tr_pri - 1 ];
  id=(GetRand( rand_context )|(GetRand( rand_context )<<16));

  tr_data = GFL_HEAP_AllocMemoryLo( inHeapID, sizeof(BSUBWAY_PARTNER_DATA) );

  if (WBTTRAINER_HasAce(wbt_tr) )
  { //切り札ポケモンセット処理
    u16 index_no;
    trd = FBI_TOOL_AllocTrainerRomData(tr_data,
        arc_id + WBTBTD_ARCOFS_TR_ACE, tr_no, NARC_message_wbt_trname_dat, inHeapID);
    //切り札ポケモンは１体だけなので０番決めうちで取得
    index_no = trd->use_poke_table[0];
    FBI_TOOL_MakePokemonParam( btpwd,
        arc_id + WBTBTD_ARCOFS_PM_ACE, index_no, id, 0, pow_rnd, 0, FALSE, inHeapID );

    addNGMons( &cond, btpwd->mons_no );
    addNGItem( &cond, btpwd->item_no );
    btpwd ++;
    cnt --;
    GFL_HEAP_FreeMemory( trd );
  }

  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(tr_data,
      arc_id + WBTBTD_ARCOFS_TR_NORMAL ,tr_no, NARC_message_wbt_trname_dat ,inHeapID);
  *bt_trd = tr_data->bt_trd;
  bt_trd->tr_type = WBTTRAINER_GetBattleTrainerType( wbt_tr );
  bt_trd->player_id = tr_no + TRID_OFFSET_WBT;

  TAMADA_Printf(" WBT TrData(%d) id=%d use_poke_cnt = %d\n", arc_id, tr_no, trd->use_poke_cnt );
  //ポケモンデータをセット
  setBSubwayPokemonParam(
      trd, btpwd, cnt, id,
      arc_id + WBTBTD_ARCOFS_PM_NORMAL, &cond, NULL, pow_rnd, inHeapID);

  GFL_HEAP_FreeMemory( trd );
  GFL_HEAP_FreeMemory( tr_data );
}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAM生成
 * @todo  DOWNLOADデータへの対応
 */
//------------------------------------------------------------------
BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys )
{
  HEAPID tempHeapID = GFL_HEAP_LOWID( wbtsys->heapID );
  BATTLE_SETUP_PARAM * bp;

  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  int num = WBTSYS_GetPartyMax( wbtsys );

  {
    WBTRULE wbt_rule = WBTSYS_GetRule( wbtsys );
    BSWAY_PLAYMODE mode = WBTTOOL_WBTRuleToBtlRule( wbt_rule );
    POKEPARTY *party;
    party = WBTSYS_CreateBattleParty( wbtsys, GAMESYSTEM_GetGameData( gsys ) );
    bp = FBI_TOOL_WBT_CreateBattleParam( gsys, party, mode );
    GFL_HEAP_FreeMemory( party );
  }

  switch ( wbt_tr->wbt_tr_type )
  {
  case WBT_TRTYPE_ROM:
    {
      BSUBWAY_TRAINER * bt_trd;
      BSUBWAY_POKEMON * btpwd;
#ifdef PM_DEBUG
      u32 tempHeapSize = GFL_HEAP_GetHeapFreeSize( tempHeapID );
#endif

      bt_trd = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(BSUBWAY_TRAINER) );
      btpwd = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(BSUBWAY_POKEMON) * 6 );

      makeRomTrainerData( bt_trd, btpwd, WBTSYS_GetWBTCup( wbtsys ), wbt_tr, num, tempHeapID );
      FBI_TOOL_WBT_SetEnemyData( bp, bt_trd, btpwd, num, 50, tempHeapID );

      if ( wbt_tr->btl_tr_type == TRTYPE_SWRIVAL )
      {//ライバルの場合、ポケモンデータを上書き
        u32 tr_id = getWBTRivalTrainerID( gsys );
        TT_EncountTrainerPokeDataMake( tr_id, bp->party[BTL_CLIENT_ENEMY1], tempHeapID );
      }

      GFL_HEAP_FreeMemory( btpwd );
      GFL_HEAP_FreeMemory( bt_trd );

#ifdef PM_DEBUG
      GF_ASSERT( tempHeapSize == GFL_HEAP_GetHeapFreeSize( tempHeapID ) );
#endif
    }
    break;

  case WBT_TRTYPE_DL:
    {
      setDLPokeParty( bp->party[BTL_CLIENT_ENEMY1], &wbtsys->wbtdl_data, 
          wbt_tr->tr_id, WBTSYS_GetPartyMax(wbtsys), tempHeapID );
    }
    break;

  default:
  case WBT_TRTYPE_PLAYER:
    GF_ASSERT( 0 );
    break;
  }

  //レギュレーションにあわせてレベル調整
  PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), bp->party[BTL_CLIENT_ENEMY1] );
  //名前を書き換える
  WBTTRAINER_CopyName( wbt_tr, bp->tr_data[BTL_CLIENT_ENEMY1]->name );
  //肩書STRBUFにポインタをつなぐ
  BATTLE_PARAM_SetTrainerTypeName( bp, BTL_CLIENT_ENEMY1, wbt_tr->title );

  WBTSYS_GetOpponentBGM( wbtsys, &bp->musicDefault, &bp->musicWin );
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
  if ( result == BTL_RESULT_WIN )
  {
    //戦ったかどうかのフラグを反映
    const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
    if ( wbt_tr->tr_id < WBTSAVE_FLAG_MAX )
    {
      WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
      WBTSAVE_SetFlag( wbtsave, wbt_tr->tr_id );
    }
  }
  *ret_wk = result;
}



