//============================================================================================
/**
 * @file	poke_regulation.h
 * @brief	レギュレーション検査用プログラム
 * @author	k.ohno
 * @date	2006.5.25
 */
//============================================================================================
#pragma once

#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "savedata/regulation.h"

#include "regulation_def.h"


// PokeRegulationMatchPartialPokePartyとPokeRegulationMatchFullPokeParty の戻り値
enum{
    POKE_REG_OK,               // 正常
    POKE_REG_TOTAL_LV_FAILED,  // トータルのLVがオーバーしている
    POKE_REG_BOTH_POKE,        // 同じポケモンがいる
    POKE_REG_BOTH_ITEM,        // 同じアイテムがある
//--------------------------------------------------------
    POKE_REG_NUM_FAILED,       // 必要なポケモンの人数が満たされていない
    POKE_REG_ILLEGAL_POKE,     // 条件に合わないポケモンがいた
    POKE_REG_NUM_FAILED2,       // 必要なポケモンの人数が満たされていない2
    POKE_REG_NO_MASTPOKE,
} POKE_REG_RETURN_ENUM;

// ポケモンがレギュレーションに適合しているかどうか調べる  
extern BOOL PokeRegulationCheckPokePara(const REGULATION* pReg, POKEMON_PARAM* pp);
// ポケパーティがレギュレーションに完全適合しているかどうか調べる バトル最終チェック用
extern int PokeRegulationMatchFullPokeParty(const REGULATION* pReg, POKEPARTY * party, u8* sel);
// ポケパーティ中にレギュレーションに適合している
//  パーティーが組めるかどうか調べる 適応外のポケモンがいても大丈夫  受け付け用
extern int PokeRegulationMatchPartialPokeParty(const REGULATION* pReg, POKEPARTY * party);

// regulation.narcにあるデータを読み込む
extern const REGULATION* PokeRegulation_LoadDataAlloc(int regulation_data_no, HEAPID heapid);
