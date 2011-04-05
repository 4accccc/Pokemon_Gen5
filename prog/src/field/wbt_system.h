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

#include "app/pokelist.h" //PLIST_DATA
#include "app/p_status.h" //PSTATUS_DATA

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
#define BUFLEN_TOURNAMENT_NAME  ( (16 * GLOBAL_MSGLEN) + BUFLEN_EOM_SIZE )
//------------------------------------------------------------------
///トーナメントの種類
//------------------------------------------------------------------
typedef enum {
  WBTTYPE_NULL,
  WBTTYPE_ALL,
  WBTTYPE_CHAMPION,
  WBTTYPE_POKETYPE,
  WBTTYPE_DOWNLOAD,
}WBTTYPE;

typedef enum {
  WBTRULE_NULL,
  WBTRULE_SINGLE,
  WBTRULE_DOUBLE,
  WBTRULE_TRIPLE,
  WBTRULE_ROTATION,
}WBTRULE;

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
  WBT_DOWNLOAD_NUMBER = 3,
};
//============================================================================================
//============================================================================================
extern WBTSYS * WBTSYS_Create( HEAPID heapID );
extern void WBTSYS_Delete( WBTSYS * wbtsys );

extern void WBTSYS_SetType( WBTSYS * wbtsys, WBTTYPE type );
extern WBTTYPE WBTSYS_GetType( const WBTSYS * wbtsys );
extern void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule );
extern WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys );
extern const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys );
extern void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no );
extern u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys );

extern void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work );
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTTOOL_SetTournamentName( WBTTYPE type, STRBUF * strbuf );
extern void WBTTOOL_SetRuleName( WBTRULE rule, STRBUF * strbuf );
extern void WBTTOOL_SetEnemyTrainerName( WBTSYS * wbtsys, STRBUF * strbuf );
extern u8 WBTTOOL_GetEnemyTrainerSex( WBTSYS * wbtsys );
extern u16 WBTTOOL_GetEnemyTrainerMMdlCode( WBTSYS * wbtsys );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );

extern void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, PLIST_DATA * plist_data );
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param );

extern void WBTLIST_PARAM_Setup_Debug( WBTLIST_PARAM * wbtlist_param, WBTLIST_STAGE stage, GAMESYS_WORK * gsys );

