//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\field_goout_effect_data.h
 *	@brief  フィールド室外表現のデータ操作
 *	@author		Toru=Nagihashi
 *	@date		2011.02.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/pm_season.h"
#include "field_goout_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	室外表現が行われるタイムゾーン
//=====================================
typedef enum
{
  FIELD_GOOUT_TIMEZONE_MORNING,
  FIELD_GOOUT_TIMEZONE_EVENING,
  FIELD_GOOUT_TIMEZONE_NIGHT,

  FIELD_GOOUT_TIMEZONE_MAX
}FIELD_GOOUT_TIMEZONE;

//-------------------------------------
///	素材データ
//=====================================
typedef enum
{
  FIELD_GOOUT_RESOURCE_IMD,
  FIELD_GOOUT_RESOURCE_ICA,
  FIELD_GOOUT_RESOURCE_IMA,
  FIELD_GOOUT_RESOURCE_ITA,
  FIELD_GOOUT_RESOURCE_ITP,
  FIELD_GOOUT_RESOURCE_MAX,
} FIELD_GOOUT_RESOURCE;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//室外表現組み合わせID
typedef u16 GOOUT_COMBI_ID;
//室外表現エフェクトID
typedef u16 GOOUT_EFFECT_ID;

//-------------------------------------
///	室外表現データ
//=====================================
typedef struct _FIELD_GOOUT_DATA  FIELD_GOOUT_DATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern FIELD_GOOUT_DATA *FIELD_GOOUT_DATA_Init( HEAPID heapID );
extern void FIELD_GOOUT_DATA_Exit( FIELD_GOOUT_DATA *p_wk );

extern ZONEID FIELD_GOOUT_DATA_GetZoneID( const FIELD_GOOUT_DATA *cp_wk, u32 index );
extern GOOUT_COMBI_ID FIELD_GOOUT_DATA_GetCombiID( const FIELD_GOOUT_DATA *cp_wk, u32 index, u8 season, FIELD_GOOUT_TIMEZONE timezone );
extern u32 FIELD_GOOUT_DATA_GetIndexMax( const FIELD_GOOUT_DATA *cp_wk );
extern GOOUT_COMBI_ID FIELD_GOOUT_DATA_GetCombiIDByZoneID( const FIELD_GOOUT_DATA *cp_wk, ZONEID zoneID, u8 season, FIELD_GOOUT_TIMEZONE timezone );

extern GOOUT_EFFECT_ID FIELD_GOOUT_DATA_GetEffectID( const FIELD_GOOUT_DATA *cp_wk, GOOUT_COMBI_ID combiID, u32 index );
extern u8 FIELD_GOOUT_DATA_GetEffectNum( const FIELD_GOOUT_DATA *cp_wk, GOOUT_COMBI_ID combiID );
extern ARCDATID FIELD_GOOUT_DATA_GetResource( const FIELD_GOOUT_DATA *cp_wk, GOOUT_EFFECT_ID effectID, FIELD_GOOUT_RESOURCE res );
