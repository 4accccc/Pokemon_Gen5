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
extern void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );

extern void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, PLIST_DATA * plist_data );
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys );
extern void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param );

extern void WBTLIST_PARAM_Setup_Debug( WBTLIST_PARAM * wbtlist_param, WBTLIST_STAGE stage, HEAPID heapID );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief WBTでセーブする要素
 */
//------------------------------------------------------------------
typedef struct {
  ///チャンピオントーナメントを1日1回しか出来ないように制御するためのフラグ
  BOOL ChampionTournamentFlag;
  ///チャンピオントーナメントに参加できるフラグ（条件クリア後？）
  BOOL EnableChampionTournamentFlag;

  ///優勝回数カウント
  u16 WinAllTournamentCount;
  u16 WinTypeTournamentCount[POKETYPE_MAX];
}WBT_SAVE;

