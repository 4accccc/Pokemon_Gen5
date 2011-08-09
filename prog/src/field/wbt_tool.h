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
  WBT_TRCODE_DL1    = 0xfff1,
  WBT_TRCODE_DL2    = 0xfff2,
  WBT_TRCODE_DL3    = 0xfff3,
  WBT_TRCODE_DL4    = 0xfff4,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBTREG_ALL = 0,
  WBTREG_SUBWAY,
  WBTREG_POKETYPE,
  WBTREG_WCS,
  WBTREG_DOWNLOAD,

  WBTREG_MAX,
}WBTREG_ID;

//------------------------------------------------------------------
/// WBT用BTDデータの指定
//------------------------------------------------------------------
typedef enum {
  WBTBTDATA_1,  ///<クリア前用データ
  WBTBTDATA_2,  ///<クリア後用データ
  WBTBTDATA_3,  ///<最強用データ

  WBTBTDATA_MAX,
}WBTBTDATA;

typedef enum {
  WBTBTD_ARCOFS_TR_ACE = 0,
  WBTBTD_ARCOFS_PM_ACE,
  WBTBTD_ARCOFS_TR_NORMAL,
  WBTBTD_ARCOFS_PM_NORMAL,
}WBTBTD_ARCOFS;


//============================================================================================
//  WBT用ツール関数群
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTTOOL_GetRomTrainerName( HEAPID heapID, u8 wbt_tr_id, STRBUF * strbuf );
extern void WBTTOOL_GetRomTrainerTypeName( HEAPID heapID, u8 wbt_tr_id, STRBUF * strbuf );
extern void WBTTOOL_GetTrRomData( u8 tr_id, WBT_TRDATA * tr_data );
extern u16 WBTTOOL_GetBWRomTrainerID( u8 sex );
extern u16 WBTTOOL_GetRivalTrainerID( void );

extern u8 WBTTOOL_GetPartyPokeType( POKEPARTY * party, u64 seed );

//------------------------------------------------------------------
//  ルール関連データ
//------------------------------------------------------------------
extern void WBTTOOL_GetRuleName( WBTRULE rule, STRBUF * strbuf );
extern u8 WBTTOOL_GetPartyMaxByRule( WBTRULE rule );
extern BtlRule WBTTOOL_WBTRuleToBtlRule( WBTRULE wbt_rule );
extern WBTRULE WBTTOOL_BtlRuleToWBTRule( BtlRule btl_rule );

//------------------------------------------------------------------
//  トーナメント関連データ
//------------------------------------------------------------------
extern void WBTTOOL_GetTournamentName( WBTCUP wbtcup, STRBUF * strbuf );
extern WBTWIN_COUNT WBTTOOL_WBTCupToWinCount( WBTCUP wbtcup, u8 poketype );
extern WBTCUP WBTTOOL_WBTWinCountToCup( WBTWIN_COUNT counter_id );
extern WBTREG_ID WBTTOOL_GetRegulationID( WBTCUP wbtcup );
extern u32 WBTTOOL_GetRomArcID( WBTCUP wbtcup );
extern u32 WBTTOOL_GetRewardCount( WBTCUP wbtcup, WBTRULE wbtrule );

extern BOOL WBTTOOL_CheckCupEnable( GAMEDATA * gamedata, WBTCUP wbtcup );
extern BOOL WBTTOOL_CheckCupUnlock( GAMEDATA * gamedata, WBTCUP now_cup, WBTCUP check_cup );
extern BOOL WBTTOOL_CheckReceptionID( WBTCUP wbtcup, WBTRECEPT reception_id );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern u32 WBTTOOL_GetDLTrtype( u8 wbt_dl_tr_type );


