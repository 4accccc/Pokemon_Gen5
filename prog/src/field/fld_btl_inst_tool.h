//======================================================================
/**
 * @file  fld_btl_inst_tool.h
 * @brief  施設バトル呼び出し等の共通部分
 *
 * @note  施設：institution
 * モジュール名：
 */
//======================================================================
#pragma once

#include "app/pokelist.h"
#include "gamesystem/gamesystem.h"
#include "savedata/bsubway_savedata.h"    //for BSUBWAY_PARTNER_DATA
//#include "battle/bsubway_battle_data.h"

extern GMEVENT *FBI_TOOL_CreatePokeListEvt(
    GAMESYS_WORK *gsys,
    const PL_LIST_TYPE inType, const int inReg, POKEPARTY *pp,
    u8 *outSelNoAry, PL_SELECT_POS *outResult, PL_RETURN_TYPE *outRetMode );

extern BOOL FBI_TOOL_MakeRomTrainerData(
    BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke, HEAPID inHeapID);


extern BATTLE_SETUP_PARAM * FBI_TOOL_CreateBattleParam(
    GAMESYS_WORK *gsys, const POKEPARTY *my_party, int inPlayMode,
    BSUBWAY_PARTNER_DATA *partner_data, const int inMemNum );

extern u16 FBI_TOOL_GetTrainerOBJCode( u32 inTrType );
/**
extern u32 FBI_TOOL_MakePokemonParam(
    BSUBWAY_POKEMON *pwd,
    u16 poke_no, u32 poke_id, u32 poke_rnd, u8 pow_rnd,
    u8 mem_idx, BOOL itemfix, HEAPID heapID );
extern void FBI_TOOL_GetPokemonRomData( BSUBWAY_POKEMON_ROM_DATA *prd,int index );
*/
