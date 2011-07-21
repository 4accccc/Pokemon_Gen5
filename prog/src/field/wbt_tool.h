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

  WBT_TRCODE_NUM = 115,
  WBT_TRCODE_NUM_CHAMP = 6,
  WBT_TRCODE_BATTLE_MOB_NUM = 10,

  WBT_TRCODE_CHAMP_START = 14,
  WBT_TRCODE_OTHER_LEADER_START = WBT_TRCODE_CHAMP_START + WBT_TRCODE_NUM_CHAMP,
  WBT_TRCODE_MOB_START = 53,
  WBT_TRCODE_WBHERO_START = 63,

  WBT_TRCODE_MOB_LAST = 114,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBTREG_ALL = 0,
  WBTREG_SUBWAY,
  WBTREG_POKETYPE,
  WBTREG_WCS,
  WBTREG_DOWNLOAD,
}WBTREG_ID;

//-----------------------------------------------------------------------------
/**
 * @brief WBTにまつわる判定処理
 */
//-----------------------------------------------------------------------------
typedef enum {
  WBT_COND_ID_ENABLE_DOWNLOAD_TOURNAMENT,   ///<ダウンロードトーナメント開催可能か？
}WBT_COND_ID;

//-----------------------------------------------------------------------------
//  トーナメント出場者の決定に必要な引数定義
//-----------------------------------------------------------------------------
typedef struct {
  HEAPID heapID;

  WBTCUP wbtcup;
  //WBTRule rule;
  
  PokeType poketype;              ///<タイプトーナメント用：タイプ指定

  u64 seed64;
}WBT_MATCH_CONDITION;


//============================================================================================
//  WBT用ツール関数群
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTTOOL_SetTournamentName( WBTCUP wbtcup, STRBUF * strbuf );
extern void WBTTOOL_SetRuleName( WBTRULE rule, STRBUF * strbuf );
extern void WBTTOOL_SetRomTrainerName( HEAPID heapID, u8 tr_id, STRBUF * strbuf );
extern u8 WBTTOOL_GetPartyPokeType( POKEPARTY * party, u64 seed );
extern void WBTTOOL_GetTrRomData( HEAPID heapID, u8 tr_id, WBT_TRDATA * tr_data );

extern void WBTTOOL_MakeEntryList( const WBT_MATCH_CONDITION * cond, u16 * tr_list );
extern void WBTTOOL_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID );
extern u8 WBTTOOL_GetPartyMaxByRule( WBTRULE rule );
extern BtlRule WBTTOOL_WBTRuleToBtlRule( WBTRULE wbt_rule );
extern WBTWIN_COUNT WBTTOOL_WBTCupToWinCount( WBTCUP wbtcup, u8 poketype );
extern WBTCUP WBTTOOL_WBTWinCountToCup( WBTWIN_COUNT counter_id );
extern WBTREG_ID WBTTOOL_GetRegulationID( WBTCUP wbtcup );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern BOOL WBTTOOL_CheckCupEnable( GAMEDATA * gamedata, WBTCUP wbtcup );
extern BOOL WBTTOOL_CheckReceptionID( WBTCUP wbtcup, WBTRECEPT reception_id );
extern BOOL WBTTOOL_GetStatus( GAMEDATA * gamedata, WBT_COND_ID cond_id );



