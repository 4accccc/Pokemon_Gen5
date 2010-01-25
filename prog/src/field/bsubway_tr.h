#pragma once
#include "battle/battle.h"

extern u16 BSUBWAY_GetTrainerOBJCode( u8 tr_type );

extern BATTLE_SETUP_PARAM * BSUBWAY_SCRWORK_CreateBattleParam(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );

extern u16 BSUBWAY_SCRWORK_GetTrainerNo(
    BSUBWAY_SCRWORK* wk, u16 stage, u8 round, int play_mode );

extern BOOL BSUBWAY_SCRWORK_MakeRomTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no,u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID);

extern void BSUBWAY_SCRWORK_MakePartnerRomData(
    BSUBWAY_SCRWORK *wk, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no,BOOL itemfix,
    const BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID);

