//======================================================================
/**
 * @file	local_tvt_type_def.h
 * @brief	非通信テレビトランシーバー タイプ定義
 * @author	ariizumi
 * @data	09/12/12
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

typedef enum
{
  LTCT_PLAYER_M,  //主人公だった場合分岐させる
  LTCT_RIVAL,
  LTCT_SUPPORT,
  LTCT_DOCTOR_D,

  LTCT_PLAYER_F,  //主人公だった場合分岐させる
  
  LTCT_MAX,
  
  LTCT_NONE = 0xFF,
}LOCAL_TVT_CHARA_TYPE;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _LOCAL_TVT_WORK LOCAL_TVT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto



//======================================================================
//	スクリプト系
//======================================================================
#pragma mark [>script
typedef enum
{
  LCL_FINISH,
  LCL_DISP_MESSAGE,
  LCL_WAIT,

  LCL_MAX,
}LTVT_COMMAND_LIST;

typedef void (*LTVT_SCRIPT_INIT_FUNC)( LOCAL_TVT_WORK *work );
typedef const BOOL (*LTVT_SCRIPT_MAIN_FUNC)( LOCAL_TVT_WORK *work );
typedef struct
{
  LTVT_SCRIPT_INIT_FUNC initFunc;
  LTVT_SCRIPT_MAIN_FUNC mainFunc;
}LTVT_SCRIPT_COMMAND_TABLE;

