//============================================================================================
/**
 * @file  pokewood_system.c
 * @brief
 * @author  mori GAMEFREAK inc.
 * @date  2011.07.11
 */
//============================================================================================

#include <gflib.h>

#include "pokewood_system.h"

#include "system/main.h"
#include "pokewood_system_local.h"
//#include "wbt_tool.h"

#include "poke_tool/poketype_def.h"
#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE

#include "fld_btl_inst_tool.h"
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT


//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	POKEWOOD_SYSTEMワーク確保
 *
 * @param   heapID		
 *
 * @retval  PKWDSYS *		
 */
//=============================================================================================
PKWDSYS * PKWDSYS_Create( HEAPID heapID )
{
  int i;
  PKWDSYS * pkwdsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWDSYS) );

  pkwdsys->heapID = heapID;

  return pkwdsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	POKEWOOD_SYSTEMワーク解放
 *
 * @param   pkwdsys		
 */
//=============================================================================================
void PKWDSYS_Delete( PKWDSYS * pkwdsys )
{
  GFL_HEAP_FreeMemory( pkwdsys );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	ポケモンリストからの結果を取得
 *
 * @param   pkwdsys		
 * @param   plist_data		
 */
//=============================================================================================
void PKWDSYS_SetPokelistResult( PKWDSYS * pkwdsys, PLIST_DATA * plist_data )
{
  GFL_STD_MemCopy( plist_data->in_num, pkwdsys->pokelist_select_num, 6 );
  pkwdsys->pokelist_result_select = plist_data->ret_sel;
  pkwdsys->pokelist_return_mode = plist_data->ret_mode;

  if( pkwdsys->pokelist_return_mode != PL_RET_NORMAL ||
      pkwdsys->pokelist_result_select == PL_SEL_POS_EXIT ||
      pkwdsys->pokelist_result_select == PL_SEL_POS_EXIT2 )
  {
    *pkwdsys->scr_answer_work = FALSE;
  }
  else
  {
    *pkwdsys->scr_answer_work = TRUE;
  }
#ifdef  PM_DEBUG
  {
    int i;
    TAMADA_Printf("Select Mons:");
    for (i = 0; i < 6; i ++ )
    {
      TAMADA_Printf("%02d ", pkwdsys->pokelist_select_num[i] );
    }
    TAMADA_Printf("\n ret_sel(%d) ret_mode(%d) script_return(%d)\n",
        pkwdsys->pokelist_result_select, pkwdsys->pokelist_return_mode, *pkwdsys->scr_answer_work );
  }
#endif
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetupPokeListData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PLIST_DATA * plist_data )
{
  int type;
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gamedata );

  PLIST_DATA_Init( plist_data, gamedata, PL_MODE_BATTLE_SUBWAY, party );

  PokeRegulation_LoadData( REG_WBT_SINGLE, &pkwdsys->reg );
  plist_data->reg = (void *)&pkwdsys->reg;
  plist_data->type = PL_TYPE_SINGLE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetupPokeStatusData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus )
{
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gamedata );
  GFL_STD_MemClear32( pstatus, sizeof(PSTATUS_DATA) );
  pstatus->ppd = GAMEDATA_GetMyPokemon( gamedata );
  pstatus->ppt = PST_PP_TYPE_POKEPARTY;
  pstatus->max = PokeParty_GetPokeCount( pstatus->ppd );
  pstatus->mode = PST_MODE_NORMAL;
  pstatus->page = PPT_INFO;
  pstatus->game_data = gamedata;

  pstatus->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetScriptAnswerWork( PKWDSYS * pkwdsys, u16 * scr_answer_work )
{
  pkwdsys->scr_answer_work = scr_answer_work;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 * PKWDSYS_GetScriptAnswerWork( PKWDSYS * pkwdsys )
{
  GF_ASSERT( pkwdsys->scr_answer_work );
  return pkwdsys->scr_answer_work;
}



#if 0

//------------------------------------------------------------------
/**
 * @brief 参戦するポケモンパーティを生成する
 * @param pkwdsys
 * @param gamedata
 *
 * @return  POKEPARTY 生成したポケモンパーティ
 *
 * @note    呼び出し元で責任を持ってメモリを開放する必要がある
 */
//------------------------------------------------------------------
POKEPARTY * PKWDSYS_CreateBattleParty( PKWDSYS * pkwdsys, GAMEDATA * gamedata )
{
  POKEPARTY * entry_party;
  POKEPARTY * base_party;
  POKEMON_PARAM * entry_pp;
  u32 cnt, max;
  u8 revise_level;

  entry_party = GFL_HEAP_AllocMemory( pkwdsys->heapID, PokeParty_GetWorkSize() );
  PokeParty_Init( entry_party, TEMOTI_POKEMAX );
  base_party = GAMEDATA_GetMyPokemon( gamedata );
  entry_pp = GFL_HEAP_AllocMemoryLo( pkwdsys->heapID, POKETOOL_GetWorkSize() );
  PP_Clear( entry_pp );
  max = WBTTOOL_GetPartyMaxByRule( pkwdsys->rule );
#ifdef DEBUG_WEAK_PARTY
  max = 1;
#endif
#ifdef  DEBUG_STRONG_PARTY
  max = PokeParty_GetPokeCount( base_party );
#endif
  revise_level = PKWDSYS_GetRevisePokeLevel( pkwdsys );
#ifdef  DEBUG_STRONG_PARTY
  revise_level = 100;
#endif
  for ( cnt = 0; cnt < max; cnt ++ )
  {
    POKEMON_PARAM * base_pp;
    u32 pos = pkwdsys->pokelist_select_num[ cnt ];
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
  GFL_HEAP_FreeMemory( entry_pp );

  return entry_party;
}
#endif