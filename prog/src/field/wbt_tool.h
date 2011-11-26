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

#include <gflib.h>

#include "field/wbt.h"

#include "wbt_trlist_struct.h"  //WBT_TRDATA

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
enum {
  WBT_TRCODE_TYPE_PLAYER = 0xff,
  WBT_TRCODE_TYPE_WBHERO = 0xfe,
  WBT_TRCODE_TYPE_DL1    = 0xf1,
  WBT_TRCODE_TYPE_DL2    = 0xf2,
  WBT_TRCODE_TYPE_DL3    = 0xf3,
  WBT_TRCODE_TYPE_DL4    = 0xf4,
  WBT_TRCODE_TYPE_ROM    = 0x00,
};

enum {
  WBT_TRCODE_PLAYER = WBT_TRCODE_TYPE_PLAYER << 8,
  WBT_TRCODE_WBHERO = WBT_TRCODE_TYPE_WBHERO << 8,
  WBT_TRCODE_DL1    = WBT_TRCODE_TYPE_DL1 << 8,
  WBT_TRCODE_DL2    = WBT_TRCODE_TYPE_DL2 << 8,
  WBT_TRCODE_DL3    = WBT_TRCODE_TYPE_DL3 << 8,
  WBT_TRCODE_DL4    = WBT_TRCODE_TYPE_DL4 << 8,
};

//------------------------------------------------------------------
//  トーナメント毎のルールのタイプ指定
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

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBTBTD_ARCOFS_TR_ACE = 0,
  WBTBTD_ARCOFS_PM_ACE,
  WBTBTD_ARCOFS_TR_NORMAL,
  WBTBTD_ARCOFS_PM_NORMAL,
}WBTBTD_ARCOFS;

//------------------------------------------------------------------
/// トーナメント属性データ定義
//------------------------------------------------------------------
typedef struct {
  u16 title_id;       ///<トーナメント名のMsgID
  u8 win_count_id;    ///<WBTSAVE::WBTWIN_COUNT
  u8 reception_id;    ///<WBTRECEPT
  u8 reg_type;        ///<WBTREG_ID
  u8 btdata_id;       ///<WBTBTDATA

  u8 reward[4];       ///<ごほうびのBP数
}WBT_TOURNAMENT_DATA;


//============================================================================================
//  WBT用ツール関数群
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTTOOL_GetRomTrainerName( HEAPID heapID, WBTROM_TRID wbt_tr_id, STRBUF * strbuf );
extern void WBTTOOL_GetRomTrainerTypeName( HEAPID heapID, WBTROM_TRID wbt_tr_id, STRBUF * strbuf );

extern const WBTROM_TRDATA * WBTTOOL_CreateRomTrainerData( HEAPID heapID );
extern void WBTTOOL_DeleteRomTrainerData( WBTROM_TRDATA * rom_trdata );
extern u16 WBTTOOL_GetRomTrainerBtlTrType(
    const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id );
extern u16 WBTTOOL_GetRomTrainerMMdlID(
    const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id );
extern WBT_TRPRI WBTTOOL_GetRomTrainerPriority(
    const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id );
extern void WBTTOOL_GetRomTrainerData(
    const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id, WBT_TRDATA * tr_data );
extern BOOL WBTTOOL_IsVaridRomTrainerID( const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id );
extern WBTROM_TRID WBTTOOL_GetBWRomTrainerID( const WBTROM_TRDATA * rom_trdata, u8 sex );
extern WBTROM_TRID WBTTOOL_GetRivalTrainerID( const WBTROM_TRDATA * rom_trdata );

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
//  以下はフィールド常駐（wbt_system_st.c）に存在
//------------------------------------------------------------------
extern const WBT_TOURNAMENT_DATA * WBTTOOL_GetTournamentData( WBTCUP wbtcup );
extern void WBTTOOL_GetTournamentName( HEAPID heapID, WBTCUP wbtcup, STRBUF * strbuf );
extern const WBT_TOURNAMENT_DATA WBTTournamentData[ WBTCUP_MAX - 1 ];

//------------------------------------------------------------------
//  トーナメント関連データ
//------------------------------------------------------------------
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


