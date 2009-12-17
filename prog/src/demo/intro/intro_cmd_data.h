//============================================================================
/**
 *
 *	@file		intro_cmd_data.h
 *	@brief  シーンデータへのアクセサ
 *	@author	hosaka genya
 *	@data		2009.12.11
 *
 */
//============================================================================
#pragma once

#include "intro_cmd_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
typedef enum
{ 
  INTRO_SCENE_ID_INIT = 0,
  INTRO_SCENE_ID_00,
  INTRO_SCENE_ID_01,
  INTRO_SCENE_ID_02,
  INTRO_SCENE_ID_03,
  INTRO_SCENE_ID_04,
  INTRO_SCENE_ID_05,
  INTRO_SCENE_ID_06,
  INTRO_SCENE_ID_07,
  INTRO_SCENE_ID_MAX,
} INTRO_SCENE_ID;

#define INTRO_CMD_PARAM_MAX (4)

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

typedef enum
{ 
  CMD_READ_END = ( FALSE ), ///< 次のコマンドを同時に読み込みまない(デフォルト値)
  CMD_READ_NEXT = ( TRUE ), ///< 次のコマンドも同時に読み込む
} CMD_READ;

//--------------------------------------------------------------
///	
//==============================================================
typedef struct {
  INTRO_CMD_TYPE type;
  int param[ INTRO_CMD_PARAM_MAX ];
  u32 read_next : 1; ///< CMD_READ : CMD_READ_NEXT なら次のコマンドを並列実行
} INTRO_CMD_DATA;

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================
extern const INTRO_CMD_DATA* Intro_DATA_GetCmdData( INTRO_SCENE_ID scene_id );

extern u16 Intro_DATA_GetSceneMax( void );


