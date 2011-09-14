//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort\resort_map.h
 *	@brief  レゾナンスリゾートのマップ処理
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "arc/fieldmap/zone_id.h"
#include "gamesystem/playerwork.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef enum
{
  RESORT_PLACEID_NONE,             //無効値
  RESORT_PLACEID_START,             //開始
  RESORT_PLACEID_FIELD  = RESORT_PLACEID_START,   //フィールド
  RESORT_PLACEID_RECEPTION,         //迎賓館
  RESORT_PLACEID_MAX,
}RESORT_PLACEID;

#define RESORT_ZONE_ID_FIELD      (ZONE_ID_C16)
#define RESORT_ZONE_ID_RECEPTION  (ZONE_ID_C16R0101)

extern ZONEID RESORT_MAP_PlaceIDToZoneID( RESORT_PLACEID placeID );
extern RESORT_PLACEID RESORT_MAP_ZoneIDToPlaceID( ZONEID zoneID );
extern void RESORT_MAP_GetPlaceGridPosByIndex( RESORT_PLACEID placeID, u16 people_type, u16 kind, u16 peopleID, u16 *p_x, u16 *p_z, u16 *p_dir );
extern void RESORT_MAP_GetResidentGridPosOfs( u16 shop_3dmd, u16 *p_x, u16 *p_z, u16 *p_dir );
extern void RESORT_MAP_GetBoardGridPosOfs( u16 shop_3dmd, u16 *p_x, u16 *p_z );
extern void RESORT_MAP_GetSecretaryGridPos( u16 kind, u16 *p_x, u16 *p_z, u16 *p_dir );
extern RESORT_PLACEID RESORT_MAP_GetPeoplePlace( u16 peopleID );
