//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_zonefog.h
 *	@brief	ゾーン用フォグ・ライト情報
 *	@author	tomoya takahashi
 *	@date		2009.07.09
 *
 *	モジュール名：FIELD_ZONEFOGLIGHT
 *
 *	他のモジュールとの関係
 *			FIELD_WEATHER
 *			WEATHERがSUNNY関数内で、FOG情報を反映しています。
 *			(weather_sunny.c)
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "arc_def.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define FIELD_ZONEFOGLIGHT_DATA_NONE	(0xfffffff)
#define FIELD_ZONEFOGLIGHT_ARC_LIGHT	( ARCID_ZONELIGHT_TABLE )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIELD_ZONEFOGLIGHTシステム
//=====================================
typedef struct _FIELD_ZONEFOGLIGHT	FIELD_ZONEFOGLIGHT;


//-------------------------------------
///	参照FOGデータ
//=====================================
typedef struct {
  u16 zone_id;
  u16 data_id;
} ZONE_FOG_DATA;

//-------------------------------------
///	参照LIGHTデータ
//=====================================
typedef struct {
  u16 zone_id;
  u16 data_id;
} ZONE_LIGHT_DATA;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern FIELD_ZONEFOGLIGHT* FIELD_ZONEFOGLIGHT_Create( HEAPID heapID );
extern void FIELD_ZONEFOGLIGHT_Delete( FIELD_ZONEFOGLIGHT* p_sys );

extern void FIELD_ZONEFOGLIGHT_Load( FIELD_ZONEFOGLIGHT* p_sys, u32 fogno, u32 lightno, HEAPID heapID );
extern void FIELD_ZONEFOGLIGHT_Clear( FIELD_ZONEFOGLIGHT* p_sys );

extern BOOL FIELD_ZONEFOGLIGHT_IsFogData( const FIELD_ZONEFOGLIGHT* cp_sys );
extern BOOL FIELD_ZONEFOGLIGHT_IsLightData( const FIELD_ZONEFOGLIGHT* cp_sys );
extern s32 FIELD_ZONEFOGLIGHT_GetOffset( const FIELD_ZONEFOGLIGHT* cp_sys );
extern u32 FIELD_ZONEFOGLIGHT_GetSlope( const FIELD_ZONEFOGLIGHT* cp_sys );
extern u32 FIELD_ZONEFOGLIGHT_GetLightIndex( const FIELD_ZONEFOGLIGHT* cp_sys );

#ifdef _cplusplus
}	// extern "C"{
#endif



