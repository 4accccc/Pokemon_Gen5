//======================================================================
/**
 * @file  fld_btl_inst_tool.h
 * @brief  施設バトル呼び出し等の共通部分
 *
 * @note  施設：institution
 * モジュール名：FBI_TOOL_
 */
//======================================================================
#pragma once

#include "app/pokelist.h"
#include "gamesystem/gamesystem.h"
#include "savedata/bsubway_savedata.h"    //for BSUBWAY_PARTNER_DATA
#include "battle/bsubway_battle_data.h" //for BSUBWAY_TRAINER_ROM_DATA

extern GMEVENT *FBI_TOOL_CreatePokeListEvt(
    GAMESYS_WORK *gsys,
    const PL_LIST_TYPE inType, const PL_MODE_TYPE inListMode, const int inReg,
    POKEPARTY *inTargetParty,
    u8 *outSelNoAry, PL_SELECT_POS *outResult, PL_RETURN_TYPE *outRetMode, POKEPARTY *outParty );

extern GMEVENT * FBI_TOOL_CreateTrainerBeforeMsgEvt(
    GAMESYS_WORK *gsys, BSUBWAY_PARTNER_DATA *tr_data, int tr_idx, u16 obj_id );
//↓以降サブウェイ・検定で共有
extern BATTLE_SETUP_PARAM * FBI_TOOL_CreateBattleParam(
    GAMESYS_WORK *gsys, const POKEPARTY *my_party, int inPlayMode,
    BSUBWAY_PARTNER_DATA *partner_data,
    BSUBWAY_PARTNER_DATA *ai_multi_data,
    const int inMemNum );

extern BOOL FBI_TOOL_MakeRomTrainerData(
    BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke, HEAPID inHeapID);

extern u16 FBI_TOOL_GetTrainerOBJCode( u32 inTrType );      

extern u32 FBI_TOOL_MakePokemonParam(
    BSUBWAY_POKEMON *pwd,
    u16 poke_no, u32 poke_id, u32 poke_rnd, u8 pow_rnd,
    u8 mem_idx, BOOL itemfix, HEAPID heapID );

extern BSUBWAY_TRAINER_ROM_DATA * FBI_TOOL_AllocTrainerRomData(
    BSUBWAY_PARTNER_DATA *tr_data, u16 zero_org_tr_no, HEAPID inHeapID );

extern BOOL FBI_TOOL_SetBSWayPokemonParam(
    BSUBWAY_TRAINER_ROM_DATA *trd,
    u16 tr_no, BSUBWAY_POKEMON *pwd, u8 cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM *poke, HEAPID heapID );

