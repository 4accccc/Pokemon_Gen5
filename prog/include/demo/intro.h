//============================================================================
/**
 *
 *	@file		intro.h
 *	@brief  3Dデモ再生アプリ
 *	@author	genya hosaka
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"

#include "savedata/save_control.h" // for 
#include "savedata/save_control_intr.h" // for INTR_SAVE_CONTROL

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern const GFL_PROC_DATA IntroProcData;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//@TODO cdatと公開ヘッダの連動は美しくない。
// src/demo/intro/intro_cmd_data.cdat と対応
typedef enum
{ 
  INTRO_SCENE_ID_INIT = 0,
  INTRO_SCENE_ID_00,
  INTRO_SCENE_ID_01,
  INTRO_SCENE_ID_02,
  INTRO_SCENE_ID_03,
  INTRO_SCENE_ID_04,
  INTRO_SCENE_ID_05,
  INTRO_SCENE_ID_05_RETAKE_YESNO,
  INTRO_SCENE_ID_06,
  INTRO_SCENE_ID_07,
  INTRO_SCENE_ID_MAX,
} INTRO_SCENE_ID;

//--------------------------------------------------------------
///	
//==============================================================
typedef enum {
  INTRO_RETCODE_NORMAL = 0, ///< 通常
  INTRO_RETCODE_RETAKE,     ///< やり直し
} INTRO_RETCODE;

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//-------------------------------------
///	PROCに渡す引数
//=====================================
typedef struct {	
  // [IN]
  SAVE_CONTROL_WORK*  save_ctrl;
  INTRO_SCENE_ID      scene_id;
  INTR_SAVE_CONTROL*  intr_save;
  // [OUT]
  INTRO_RETCODE       retcode;
} INTRO_PARAM;

FS_EXTERN_OVERLAY(intro);

extern const GFL_PROC_DATA ProcData_Intro;




