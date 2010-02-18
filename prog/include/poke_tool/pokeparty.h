//============================================================================================
/**
 * @file	pokeparty.h
 * @brief	ポケモンパーティ操作
 * @author	tamada	GAME FREAK Inc.
 * @date	2005.10.13
 *
 * ポケモンパーティーとは自分や対戦相手の手持ちなど、複数のポケモンをひとかたまりとして
 * 扱いたい場合に使用する。
 * 手持ちにポケモンを加えたり、あるいは手持ちから外したり、位置を入れ替えたりなどの操作は
 * POKEPARTYを対象として行う。
 */
//============================================================================================

#ifndef	__POKEPARTY_H__
#define	__POKEPARTY_H__

#include "savedata/save_control.h"	//SAVEDATA参照のため
#include "poke_tool.h"

//============================================================================================
//============================================================================================
//pokeparty.c[22]でサイズが異なるとコンパイラエラーがでるようにしている
#define POKEPARTY_WORK_SIZE (1332)

//----------------------------------------------------------
/**
 * @param	ポケモンパーティ型定義
 */
//----------------------------------------------------------
typedef struct _POKEPARTY POKEPARTY;

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int PokeParty_GetWorkSize(void);
extern POKEPARTY * PokeParty_AllocPartyWork(HEAPID heapID);
extern void PokeParty_InitWork(POKEPARTY * party);

//----------------------------------------------------------
//	POKEPARTY操作のための関数
//----------------------------------------------------------
extern void PokeParty_Init(POKEPARTY * party, int max);
extern POKEMON_PARAM * PokeParty_GetMemberPointer(const POKEPARTY * party, int pos);
extern BOOL PokeParty_Add(POKEPARTY * party, const POKEMON_PARAM * poke);
extern BOOL PokeParty_Delete(POKEPARTY * party, int pos);
extern BOOL PokeParty_ExchangePosition(POKEPARTY * party, int pos1, int pos2, HEAPID heapID );
extern void PokeParty_Copy(const POKEPARTY * src, POKEPARTY * dst);
extern BOOL PokeParty_PokemonCheck(const POKEPARTY * ppt, int mons_no);
extern int	PokeParty_GetPokeCountMax(const POKEPARTY * party);
extern int PokeParty_GetPokeCount(const POKEPARTY* party);
extern int PokeParty_GetPokeCountNotEgg(const POKEPARTY* party);
extern int PokeParty_GetPokeCountBattleEnable(const POKEPARTY* party);
extern int PokeParty_GetPokeCountOnlyEgg(const POKEPARTY* party);
extern int PokeParty_GetPokeCountOnlyDameEgg(const POKEPARTY* party);
extern int PokeParty_GetMemberTopIdxBattleEnable( const POKEPARTY * party );
extern int PokeParty_GetMemberTopIdxNotEgg( const POKEPARTY * party );
extern void PokeParty_SetMemberData( POKEPARTY* party, int pos, POKEMON_PARAM* pp );
extern int PokeParty_GetBattlePokeNum(const POKEPARTY * ppt);

//ステータス画面でのポケモンの向き
extern BOOL PokeParty_GetStatusViewIsFront( POKEPARTY* party, int pos );
extern void PokeParty_SetStatusViewIsFront( POKEPARTY* party, int pos , const BOOL isFront);

extern void Debug_PokeParty_MakeParty(POKEPARTY * party);
//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern POKEPARTY * SaveData_GetTemotiPokemon(SAVE_CONTROL_WORK * sv);

#ifdef CREATE_INDEX
extern void *Index_Get_PokeCount_Offset(POKEPARTY *pt);
#endif

#endif	/* __POKEPARTY_H__ */

