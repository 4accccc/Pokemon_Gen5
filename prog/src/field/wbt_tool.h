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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
enum {
  WBT_TRPRI_NULL = 0,
  WBT_TRPRI_WEAK_MOB = 1,
  WBT_TRPRI_MOB,
  WBT_TRPRI_LEADER,
  WBT_TRPRI_CHAMP,
  WBT_TRPRI_SPECIAL,
};

//-----------------------------------------------------------------------------
/**
 * @brief WBTにまつわる判定処理
 */
//-----------------------------------------------------------------------------
typedef enum {
  WBT_COND_ID_ENABLE_POKETYPE_TOURNAMENT,   ///<タイプトーナメント開催可能か？
  WBT_COND_ID_ENABLE_CHAMPION_TOURNAMENT,   ///<チャンピオントーナメント開催可能か？
  WBT_COND_ID_ENABLE_DOWNLOAD_TOURNAMENT,   ///<ダウンロードトーナメント開催可能か？
  WBT_COND_ID_ENTRY_CHAMP_IN_ALL,           ///<オールトーナメントにチャンピオン参戦するか？
}WBT_COND_ID;

//-----------------------------------------------------------------------------
//  トーナメント出場者の決定に必要な引数定義
//-----------------------------------------------------------------------------
typedef struct {
  HEAPID heapID;

  WBTTYPE type;
  //WBTRule rule;
  
  WBT_ALLTOUR_LEVEL level;
  PokeType poketype;              ///<タイプトーナメント用：タイプ指定
  u8 leader_num;                  ///<オールトーナメント用：リーダー人数
  BOOL champ_entry;               ///<オールトーナメント用：チャンピオンはエントリーするか？

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

//------------------------------------------------------------------
//------------------------------------------------------------------
extern BOOL WBTTOOL_GetStatus( GAMEDATA * gamedata, WBT_COND_ID cond_id );
extern u32 WBTTOOL_GetAllTournamentLeaderRate( GAMEDATA * gamedata, GFL_STD_RandContext * context );
extern WBT_ALLTOUR_LEVEL WBTTOOL_GetAllTournamentEntryLevel( GAMEDATA * gamedata );


