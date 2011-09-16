//============================================================================================
/**
 * @file  wbt_system_lobby.c
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


