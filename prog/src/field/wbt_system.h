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

#include "app/wbt_list.h" //WBTLIST_PARAM
#include "app/win_record.h"		// WIN_RECORD_PARAM

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
///WBTトーナメントの種類
//------------------------------------------------------------------
typedef enum {
  WBTTYPE_NULL,
  WBTTYPE_ALL,
  WBTTYPE_CHAMPION,
  WBTTYPE_POKETYPE,
  WBTTYPE_DOWNLOAD,
}WBTTYPE;

//------------------------------------------------------------------
///WBT対戦ルール
//------------------------------------------------------------------
typedef enum {
  WBTRULE_NULL,
  WBTRULE_SINGLE,
  WBTRULE_DOUBLE,
  WBTRULE_TRIPLE,
  WBTRULE_ROTATION,
}WBTRULE;

//------------------------------------------------------------------
///WBTの状態
//------------------------------------------------------------------
typedef enum {
  WBTSTAGE_RECEPTION = 0,  ///<WBT状態：受付
  WBTSTAGE_ENTRY,          ///<WBT状態：エントリー
  WBTSTAGE_1ST,            ///<WBT状態：1回戦
  WBTSTAGE_2ND,            ///<WBT状態：2回戦
  WBTSTAGE_FINAL,          ///<WBT状態：決勝戦
  WBTSTAGE_WIN,            ///<WBT状態：勝利
}WBTSTAGE;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBT_ALLTOUR_LEVEL_0 = 0,
  WBT_ALLTOUR_LEVEL_1,
  WBT_ALLTOUR_LEVEL_2,
  WBT_ALLTOUR_LEVEL_3,
}WBT_ALLTOURNAMENT_LEVEL;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBT_TRTYPE_ROM = 0, ///<ロムに存在するトレーナー
  WBT_TRTYPE_DL,      ///<ダウンロードデータのトレーナー
  WBT_TRTYPE_SAVE,    ///<WB（データ引継ぎ）のトレーナー
  WBT_TRTYPE_PLAYER,  ///<プレイヤー
}WBT_TRTYPE;
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_TRMSG_1ST_FIGHT_START = 0,
  WBT_TRMSG_2ND_FIGHT_START,
  WBT_TRMSG_3RD_FIGHT_START,
  WBT_TRMSG_FIGHT_LOSE,
  WBT_TRMSG_PLAYER_WIN,
  WBT_TRMSG_PLAYER_LOSE,
  WBT_TRMSG_ANNOUNCE,

  WBT_TRMSG_TYPE_NUM,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_DOWNLOAD_NUMBER = 3,
};
//============================================================================================
//============================================================================================
extern WBTSYS * WBTSYS_Create( HEAPID heapID );
extern void WBTSYS_Delete( WBTSYS * wbtsys );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_SetType( WBTSYS * wbtsys, WBTTYPE type );
extern WBTTYPE WBTSYS_GetType( const WBTSYS * wbtsys );

extern void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule );
extern WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys );

extern void WBTSYS_SetPokeType( WBTSYS * wbtsys, PokeType poketype );
extern PokeType WBTSYS_GetPokeType( const WBTSYS * wbtsys );

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
extern u16 WBTTRAINER_GetTrainerType( const WBTTRAINER * wbt_tr );
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
extern void WBTSYS_ReflectBattleResult( WBTSYS * wbtsys, GAMEDATA * gamedata );
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param );

extern void WBTLIST_PARAM_Setup( GAMESYS_WORK * gsys, WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param );
#ifdef  PM_DEBUG
extern void WBTLIST_PARAM_Setup_Debug(
    GAMESYS_WORK * gsys, WBTLIST_PARAM * wbtlist_param,
    WBTTYPE wbttype, WBTRULE rule,
    WBTSTAGE stage, const u16 * trid_table, int debug_value );
#endif

//
extern WIN_RECORD_PARAM * WIN_RECORD_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void WIN_RECORD_PARAM_Delete( WIN_RECORD_PARAM * wr_param );
#ifdef  PM_DEBUG
extern void WIN_RECORD_PARAM_Setup_Debug( GAMESYS_WORK * gsys, WIN_RECORD_PARAM * wr_param );
#endif


