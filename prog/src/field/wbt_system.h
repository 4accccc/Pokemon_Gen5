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

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
#define BUFLEN_TOURNAMENT_NAME  ( (16 * GLOBAL_MSGLEN) + BUFLEN_EOM_SIZE )

//------------------------------------------------------------------
/**
 * @brief オールトーナメント参戦トレーナーのレベル
 */
//------------------------------------------------------------------
typedef enum {
  WBT_ALLTOUR_LEVEL0 = 0,
  WBT_ALLTOUR_LEVEL1,
  WBT_ALLTOUR_LEVEL2,
  WBT_ALLTOUR_LEVEL3,
}WBT_ALLTOUR_LEVEL;

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
extern const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys );
extern void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no );
extern u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys );

extern void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work );
extern u16 * WBTSYS_GetScriptAnswerWork( WBTSYS * wbtsys );
//============================================================================================
//============================================================================================
#ifdef  PM_DEBUG
extern void WBTSYS_DEBUG_dumpTrainer( const WBTSYS * wbtsys );
#endif
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_MakeMatch( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_CalcResult( WBTSYS * wbtsys, GFL_STD_RandContext * context );
extern void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const u16 * tr_list );

extern const WBTTRAINER * WBTSYS_GetTrainer( const WBTSYS * wbtsys, u32 pos );
extern u32 WBTSYS_GetOpponentPos( const WBTSYS * wbtsys );

extern const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys );

extern u8 WBTTRAINER_GetID( const WBTTRAINER * wbt_tr );
extern u8 WBTTRAINER_GetPriority( const WBTTRAINER * wbt_tr );
extern u16 WBTTRAINER_GetBattleTrainerType( const WBTTRAINER * wbt_tr );
extern WBT_TRTYPE WBTTRAINER_GetWBTTrainerType( const WBTTRAINER * wbt_tr );
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_GetOpponentName( WBTSYS * wbtsys, STRBUF * strbuf );
extern u8 WBTSYS_GetOpponentSex( WBTSYS * wbtsys );
extern u16 WBTSYS_GetOpponentMMdlObjCode( WBTSYS * wbtsys );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );

extern void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, PLIST_DATA * plist_data );

extern POKEPARTY * WBTSYS_CreateBattleParty( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys );

extern void WBTSYS_ReflectBattleResult(
    WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp );

