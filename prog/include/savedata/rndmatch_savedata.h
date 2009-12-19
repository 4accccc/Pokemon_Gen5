//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		rndmatch_savedata.h
 *	@brief	ランダムバトルマッチ用セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2009.12.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	取得できるもの
//=====================================
typedef enum
{
  RNDMATCH_PARAM_IDX_RATE,
  RNDMATCH_PARAM_IDX_WIN,
  RNDMATCH_PARAM_IDX_LOSE,
  
  RNDMATCH_PARAM_IDX_MAX
} RNDMATCH_PARAM_IDX;

//-------------------------------------
///	種類
//=====================================
typedef enum
{
  RNDMATCH_TYPE_FREE_SINGLE,
  RNDMATCH_TYPE_FREE_DOUBLE,
  RNDMATCH_TYPE_FREE_TRIPLE,
  RNDMATCH_TYPE_FREE_ROTATE,
  RNDMATCH_TYPE_FREE_SHOOTER,
  RNDMATCH_TYPE_RATE_SINGLE,
  RNDMATCH_TYPE_RATE_DOUBLE,
  RNDMATCH_TYPE_RATE_TRIPLE,
  RNDMATCH_TYPE_RATE_ROTATE,
  RNDMATCH_TYPE_RATE_SHOOTER,

  RNDMATCH_TYPE_MAX
} RNDMATCH_TYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブデータ不完全型
//=====================================
typedef struct _RNDMATCH_DATA RNDMATCH_DATA;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int RNDMATCH_GetWorkSize( void );
extern void RNDMATCH_Init( RNDMATCH_DATA *p_wk );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const RNDMATCH_DATA * SaveData_GetRndMatchConst( const SAVE_CONTROL_WORK * cp_sv );
extern RNDMATCH_DATA * SaveData_GetRndMatch( SAVE_CONTROL_WORK * p_sv );

//----------------------------------------------------------
//取得、操作する関数
//----------------------------------------------------------
extern u16 RNDMATCH_GetParam( const RNDMATCH_DATA *cp_wk, RNDMATCH_TYPE type, RNDMATCH_PARAM_IDX idx );
extern void RNDMATCH_SetParam( RNDMATCH_DATA *p_wk, RNDMATCH_TYPE type, RNDMATCH_PARAM_IDX idx, u16 num );
