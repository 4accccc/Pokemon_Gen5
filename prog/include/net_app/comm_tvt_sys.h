//======================================================================
/**
 * @file	comm_tvt_sys
 * @brief	通信TVTシステム
 * @author	ariizumi
 * @data	09/12/16
 *
 * モジュール名：COMM_TVT
 */
//======================================================================
#pragma once

#include "gamesystem/game_data.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

FS_EXTERN_OVERLAY(comm_tvt);

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CTM_TEST,   //テスト用(changeOver
  CTM_PARENT, //親起動
  CTM_CHILD,  //子起動
  CTM_WIFI,   //Wifi起動
}COMM_TVT_INIT_MODE;


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  GAMEDATA *gameData;
  COMM_TVT_INIT_MODE mode;
  u8  macAddress[6];    //子起動のみ
}COMM_TVT_INIT_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern GFL_PROC_DATA COMM_TVT_ProcData;


