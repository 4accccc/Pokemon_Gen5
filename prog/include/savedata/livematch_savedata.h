//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livematch_btlscore_data.h
 *	@brief  ライブマッチ戦績セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2010.03.07
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
///	自分の情報
//=====================================
typedef enum
{
  LIVEMATCH_MYDATA_PARAM_BTLCNT,
  LIVEMATCH_MYDATA_PARAM_WIN,
  LIVEMATCH_MYDATA_PARAM_LOSE,
  LIVEMATCH_MYDATA_PARAM_DUMMY, //パディング

  LIVEMATCH_MYDATA_PARAM_MAX
} LIVEMATCH_MYDATA_PARAM;

//-------------------------------------
///	対戦相手の情報
//=====================================
typedef enum
{
  LIVEMATCH_FOEDATA_PARAM_REST_POKE,
  LIVEMATCH_FOEDATA_PARAM_REST_HP,

  LIVEMATCH_FOEDATA_PARAM_MAX
} LIVEMATCH_FOEDATA_PARAM;

//-------------------------------------
///	対戦相手の記録最大
//=====================================
#define LIVEMATCH_FOEDATA_MAX   (10)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブデータ不完全型
//=====================================
typedef struct _LIVEMATCH_DATA LIVEMATCH_DATA;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int LIVEMATCH_GetWorkSize( void );
extern void LIVEMATCH_Init( LIVEMATCH_DATA *p_wk );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const LIVEMATCH_DATA * SaveData_GetLiveMatchConst( const SAVE_CONTROL_WORK * cp_sv );
extern LIVEMATCH_DATA * SaveData_GetLiveMatch( SAVE_CONTROL_WORK * p_sv );

//----------------------------------------------------------
//取得、操作する関数
//----------------------------------------------------------
//自分に関係するデータ
//取得
extern void LIVEMATCH_DATA_GetMyMacAddr( const LIVEMATCH_DATA *cp_wk, u8* p_mac_addr );
extern int LIVEMATCH_DATA_GetMyParam( const LIVEMATCH_DATA *cp_wk, LIVEMATCH_MYDATA_PARAM param );
//設定
//extern void LIVEMATCH_DATA_SetMyMacAddr( const LIVEMATCH_DATA *cp_wk, const u8* cp_mac_addr );
extern void LIVEMATCH_DATA_SetMyParam( LIVEMATCH_DATA *p_wk, LIVEMATCH_MYDATA_PARAM param, int data );
extern void LIVEMATCH_DATA_AddMyParam( LIVEMATCH_DATA *p_wk, LIVEMATCH_MYDATA_PARAM param, int data );

//相手に関係するデータ
//取得
extern void LIVEMATCH_DATA_GetFoeMacAddr( const LIVEMATCH_DATA *cp_wk, u32 index, u8* p_mac_addr );
extern int LIVEMATCH_DATA_GetFoeParam( const LIVEMATCH_DATA *cp_wk, u32 index, LIVEMATCH_FOEDATA_PARAM param );
//設定
extern void LIVEMATCH_DATA_SetFoeMacAddr( LIVEMATCH_DATA *p_wk, u32 index, const u8* cp_mac_addr );
extern void LIVEMATCH_DATA_SetFoeParam( LIVEMATCH_DATA *p_wk, u32 index, LIVEMATCH_FOEDATA_PARAM param, int data );

