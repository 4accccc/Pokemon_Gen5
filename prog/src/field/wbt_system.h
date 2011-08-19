//============================================================================================
/**
 * @file  wbt_system.h
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.13
 *
 * @todo  ダウンロードデータのサイズ、項目確認（肩書の長さなど）
 */
//============================================================================================
#pragma once

#include "field/wbt.h"

#include "battle/battle.h"

#include "poke_tool/poke_regulation.h"  //REGULATION

#include "app/pokelist.h" //PLIST_DATA
#include "app/p_status.h" //PSTATUS_DATA


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _WBTTRAINER WBTTRAINER;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _WBTRESULT WBTRESULT;

typedef struct _WBT_MATCH_CONDITION WBT_MATCH_CONDITION;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
#define BUFLEN_TOURNAMENT_NAME  ( (16 * GLOBAL_MSGLEN) + BUFLEN_EOM_SIZE )


//============================================================================================
//============================================================================================
extern WBTSYS * WBTSYS_Create( HEAPID heapID );
extern void WBTSYS_Delete( WBTSYS * wbtsys );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_SetWBTCup( WBTSYS * wbtsys, WBTCUP wbtcup );
extern WBTCUP WBTSYS_GetWBTCup( const WBTSYS * wbtsys );

extern void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule );
extern WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys );

extern void WBTSYS_SetPokeType( WBTSYS * wbtsys, PokeType poketype );
extern PokeType WBTSYS_GetPokeType( const WBTSYS * wbtsys );

extern WBTRECEPT WBTSYS_GetReceptionID( const WBTSYS * wbtsys );
extern void WBTSYS_SetReceptionID( WBTSYS * wbtsys, WBTRECEPT reception_id );

extern void WBTSYS_SetStage( WBTSYS * wbtsys, WBTSTAGE stage );
extern WBTSTAGE WBTSYS_GetStage( const WBTSYS * wbtsys );

extern u64 WBTSYS_GetRandomSeed( const WBTSYS * wbtsys );
//------------------------------------------------------------------
//------------------------------------------------------------------
extern u32 WBTSYS_GetRegulationID( WBTSYS * wbtsys );
extern const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys );

extern void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no );
extern u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys );

extern void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work );
extern u16 * WBTSYS_GetScriptAnswerWork( WBTSYS * wbtsys );
//============================================================================================
//============================================================================================
#ifdef  PM_DEBUG
extern void WBTTRAINER_DEBUG_dump( const WBTTRAINER * wbt_tr );
extern void WBTSYS_DEBUG_dumpTrainer( const WBTSYS * wbtsys );
extern void WBTSYS_DEBUG_PrintRegulation( const REGULATION * cp_data );
#endif
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_MakeMatch( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const u16 * tr_list );

extern const WBTTRAINER * WBTSYS_GetTrainer( const WBTSYS * wbtsys, u32 pos );

extern const WBTTRAINER * WBTSYS_GetPlayerTrainer( const WBTSYS * wbtsys );

extern u8 WBTTRAINER_GetID( const WBTTRAINER * wbt_tr );
extern u8 WBTTRAINER_GetPriority( const WBTTRAINER * wbt_tr );
extern u16 WBTTRAINER_GetBattleTrainerType( const WBTTRAINER * wbt_tr );
extern WBT_TRTYPE WBTTRAINER_GetWBTTrainerType( const WBTTRAINER * wbt_tr );
extern void WBTTRAINER_CopyName( const WBTTRAINER * wbt_tr, STRBUF * buf );
extern void WBTTRAINER_CopyTitle( const WBTTRAINER * wbt_tr, STRBUF * buf );
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_GetOpponentName( const WBTSYS * wbtsys, STRBUF * strbuf );
extern u8 WBTSYS_GetOpponentSex( const WBTSYS * wbtsys );
extern u16 WBTSYS_GetOpponentMMdlObjCode( const WBTSYS * wbtsys );
extern u16 WBTSYS_GetOpponentTrainerType( const WBTSYS * wbtsys );
extern void WBTSYS_GetOpponentBGM( const WBTSYS * wbtsys, u16 *btl_bgm, u16 *win_bgm );
extern u8 WBTSYS_GetRewardCount( const WBTSYS * wbtsys );

extern void WBTSYS_GetTournamentName( const WBTSYS * wbtsys, WBTCUP wbtcup, STRBUF * strbuf );
extern void WBTSYS_GetTrainerMsg( const WBTSYS * wbtsys, u32 type, STRBUF * strbuf );

extern BOOL WBTSYS_GetDownLoadCount( const WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_LoadDLData( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_UnloadDLData( WBTSYS * wbtsys );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern u16 WBTSYS_GetRomTrainerMMdlID( const WBTSYS * wbtsys, u16 tr_id );
extern WBT_TRPRI WBTSYS_GetRomTrainerPriority( const WBTSYS * wbtsys, u16 tr_id );

//------------------------------------------------------------------
//  in wbt_calc_result.c
//------------------------------------------------------------------
extern u32 WBTSYS_GetOpponentPos( const WBTSYS * wbtsys );
extern const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys );
extern void WBTSYS_CalcResult( WBTSYS * wbtsys, GFL_STD_RandContext * context );

//------------------------------------------------------------------
//  in wbt_makematch.c
//------------------------------------------------------------------
extern void WBTSYS_MakeEntryList( GAMEDATA * gamedata, const WBT_MATCH_CONDITION * cond, u16 * tr_list );
extern void WBTSYS_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );

extern void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, const PLIST_DATA * plist_data );
extern u8 WBTSYS_GetSurvivalCount( const WBTSYS * wbtsys, WBTSTAGE stage );

extern POKEPARTY * WBTSYS_CreateBattleParty( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys );
extern void WBTSYS_DeleteBattleSetupParam( WBTSYS * wbtsys, BATTLE_SETUP_PARAM * bsp );

extern void WBTSYS_ReflectBattleResult(
    WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp );

