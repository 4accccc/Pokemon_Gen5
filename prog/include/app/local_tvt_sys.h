//======================================================================
/**
 * @file	local_tvt_sys.h
 * @brief	非通信テレビトランシーバー メイン
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#pragma once

#include "gamesystem/game_data.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define LOCAL_TVT_MEMBER_MAX (4)
FS_EXTERN_OVERLAY(local_tvt);

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  LTM_2_MEMBER = 2,
  LTM_4_MEMBER = 4,
  
  LTM_MAX,
}LOCAL_TVT_MEMBER;

typedef enum
{
  LOCAL_TVT_MODE_SCRIPT,  //スクリプト制御モード
  LOCAL_TVT_MODE_APP,     //アプリケーションモード
}LOCAL_TVT_MODE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  LOCAL_TVT_MODE  mode;     //アプリ起動モード
  u16             scriptId; //スクリプト制御モード時につかうスクリプト番号
  GAMEDATA        *gameData;//ゲームデータ
}LOCAL_TVT_INIT_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern const GFL_PROC_DATA LocalTvt_ProcData;
