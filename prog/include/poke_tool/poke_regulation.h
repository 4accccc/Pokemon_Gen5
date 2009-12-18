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

#include "print/wordset.h"


// PokeRegulationMatchPartialPokePartyとPokeRegulationMatchFullPokeParty の戻り値
typedef enum{
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
extern void PokeRegulation_LoadData(int regulation_data_no, REGULATION *reg);


//禁止フラグのビット
#define POKEFAILEDBIT_NUM         (0x01)     ///< 数が足りてない
#define POKEFAILEDBIT_LEVEL       (0x02)     ///< レベル違反のポケモンがいる
#define POKEFAILEDBIT_VETO_POKE     (0x04)   ///< 禁止ポケモン
#define POKEFAILEDBIT_BOTHPOKE    (0x08)    ///< 同じポケモン
#define POKEFAILEDBIT_BOTHITEM    (0x10)    ///< 同じアイテム
#define POKEFAILEDBIT_EGG         (0x20)    ///< たまご
#define POKEFAILEDBIT_VETO_ITEM         (0x40)   ///< 禁止アイテム
#define POKEFAILEDBIT_MAST_POKE (0x80)   ///< 必要ポケモンがいない

// ポケモンがレギュレーションに適合しているかどうか調べる  何処が悪かったかBITで返します
extern BOOL PokeRegulationCheckPokeParaLookAt(const REGULATION* pReg, POKEMON_PARAM* pp,u32* FailedBit);
// ポケパーティがレギュレーションに適合しているかどうか調べる 何処が悪かったかBITで返します
extern int PokeRegulationMatchLookAtPokeParty(const REGULATION* pReg, POKEPARTY * party, u32* FailedBit);



//==============================================================================
//  メッセージ取得
//==============================================================================
///レギュレーション項目表示カテゴリー
typedef enum{
  REGULATION_CATEGORY_NUM,        ///<ポケモンの数
  REGULATION_CATEGORY_LV,         ///<ポケモンのレベル
  REGULATION_CATEGORY_SP_POKE,    ///<特別なポケモン
  REGULATION_CATEGORY_SAMEPOKE,   ///<同じポケモン
  REGULATION_CATEGORY_SAMEITEM,   ///<同じ道具
  REGULATION_CATEGORY_SHOOTER,    ///<サポートシューター
  
  REGULATION_CATEGORY_MAX,
}REGULATION_CATEGORY;

///レギュレーション内容を一覧表示する場合のメッセージパッケージ
typedef struct{
  GFL_MSGDATA *msgdata;
  STRBUF *category[REGULATION_CATEGORY_MAX];     ///<カテゴリー名のメッセージID
  STRBUF *prerequisite[REGULATION_CATEGORY_MAX]; ///<カテゴリー毎の前提条件メッセージ
  u8 shooter_type;
  u8 padding[3];
}REGULATION_PRINT_MSG;

extern REGULATION_PRINT_MSG * PokeRegulation_CreatePrintMsg(
  const REGULATION* pReg, WORDSET *wordset, HEAPID heap_id, int shooter_type);
extern void PokeRegulation_DeletePrintMsg(REGULATION_PRINT_MSG *rpm);
