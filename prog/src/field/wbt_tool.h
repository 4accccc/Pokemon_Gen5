//============================================================================================
/**
 * @file  wbt_tool.h
 * @brief   WBT用ツール関数群
 * @date  2011.04.08
 * @author  tamada@gamefreak.co.jp
 *
 * WBTSYSの状態に依存しない関数をここに集めている
 */
//============================================================================================
#pragma once

#include "field/wbt.h"
#include "wbt_system.h"


#include "wbt_trlist_struct.h"  //WBT_TRDATA

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
enum {
  WBT_TRCODE_PLAYER = 0xffff,
  WBT_TRCODE_WBHERO = 0xfffe,
};

//-----------------------------------------------------------------------------
//  トーナメント出場者の決定に必要な引数定義
//-----------------------------------------------------------------------------
typedef struct {
  HEAPID heapID;

  WBTTYPE type;
  //WBTRule rule;
  
  WBT_ALLTOURNAMENT_LEVEL level;
  PokeType poketype;

  u64 seed64;
}WBT_MATCH_CONDITION;


//============================================================================================
//  WBT用ツール関数群
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTTOOL_SetTournamentName( WBTTYPE type, STRBUF * strbuf );
extern void WBTTOOL_SetRuleName( WBTRULE rule, STRBUF * strbuf );
extern void WBTTOOL_SetRomTrainerName( HEAPID heapID, u8 tr_id, STRBUF * strbuf );
extern u8 WBTTOOL_GetPartyPokeType( POKEPARTY * party, u64 seed );
extern void WBTTOOL_GetTrRomData( HEAPID heapID, u8 tr_id, WBT_TRDATA * tr_data );

extern void WBTTOOL_MakeEntryList( const WBT_MATCH_CONDITION * cond, u16 * tr_list );
extern void WBTTOOL_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID );
extern u8 WBTTOOL_GetPartyMaxByRule( WBTRULE rule );
extern BtlRule WBTTOOL_WBTRuleToBtlRule( WBTRULE wbt_rule );

