//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		battlematch_savedata.h
 *	@brief  バトル大会用セーブデータ
 *	        （もともとは別々だったが、セーブの256アライメントのため共通にした）
 *	@author	Toru=Nagihashi
 *	@date		2010.03.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "save_control.h"

#include "rndmatch_savedata.h"
#include "livematch_savedata.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブデータ不完全型
//=====================================
typedef struct _BATTLEMATCH_DATA BATTLEMATCH_DATA;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================

//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int BATTLEMATCH_GetWorkSize( void );
extern void BATTLEMATCH_Init( BATTLEMATCH_DATA *p_wk );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const BATTLEMATCH_DATA * SaveData_GetBattleMatchConst( const SAVE_CONTROL_WORK * cp_sv );
extern BATTLEMATCH_DATA * SaveData_GetBattleMatch( SAVE_CONTROL_WORK * p_sv );

//----------------------------------------------------------
//大会データからの取得
//----------------------------------------------------------
extern const RNDMATCH_DATA * BATTLEMATCH_GetRndMatchConst( const BATTLEMATCH_DATA * cp_sv );
extern RNDMATCH_DATA * BATTLEMATCH_GetRndMatch( BATTLEMATCH_DATA * p_sv );
extern const LIVEMATCH_DATA * BATTLEMATCH_GetLiveMatchConst( const BATTLEMATCH_DATA * cp_sv );
extern LIVEMATCH_DATA * BATTLEMATCH_GetLiveMatch( BATTLEMATCH_DATA * p_sv );


//----------------------------------------------------------
//電源切断対策
//----------------------------------------------------------
extern void BATTLEMATCH_StartPowerOffCheck( BATTLEMATCH_DATA * p_sv );
extern void BATTLEMATCH_EndPowerOffCheck( BATTLEMATCH_DATA * p_sv );

extern BOOL BATTLEMATCH_IsPowerOff( const BATTLEMATCH_DATA * cp_sv );
extern BOOL BATTLEMATCH_IsSameDS( const BATTLEMATCH_DATA * cp_sv );
extern BOOL BATTLEMATCH_IsEndPenaltyTime( const BATTLEMATCH_DATA * cp_sv );
extern void BATTLEMATCH_SetProgTime( BATTLEMATCH_DATA * p_sv );

extern void BATTLEMATCH_SetRndmatch( BATTLEMATCH_DATA * p_sv, BOOL is_on );
extern BOOL BATTLEMATCH_GetRndmatch( const BATTLEMATCH_DATA * cp_sv );
