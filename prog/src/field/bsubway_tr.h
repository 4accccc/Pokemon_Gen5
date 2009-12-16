#pragma once
#include "battle/battle.h"

extern u16  BattleTowerTrainerNoGet(
    BSUBWAY_SCRWORK* wk,u8 stage,u8 round,int play_mode);
extern BOOL  RomBattleTowerTrainerDataMake(
    BSUBWAY_SCRWORK* wk,BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no,int cnt,
    u16 *set_poke_no,u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID);
extern u16 BtlTower_TrType2ObjCode(u8 tr_type);
extern void RomBattleTowerPartnerDataMake(
    BSUBWAY_SCRWORK* wk,BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no,BOOL itemfix,
    const BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID);
extern BATTLE_SETUP_PARAM * BtlTower_CreateBattleParam(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys );
