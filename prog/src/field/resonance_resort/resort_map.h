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
  RESORT_PLACEID_FIELD,             //フィールド
  RESORT_PLACEID_RECEPTION,         //迎賓館
  RESORT_PLACEID_CONDOMINIUM_00,    //コンドミニアム
  RESORT_PLACEID_CONDOMINIUM_01,    //コンドミニアム
  RESORT_PLACEID_CONDOMINIUM_02,    //コンドミニアム
  RESORT_PLACEID_CONDOMINIUM_03,    //コンドミニアム
  RESORT_PLACEID_CONDOMINIUM_04,    //コンドミニアム
  RESORT_PLACEID_CONDOMINIUM_05,    //コンドミニアム
  RESORT_PLACEID_INSTITUTION_00,    //施設
  RESORT_PLACEID_INSTITUTION_01,    //施設
  RESORT_PLACEID_INSTITUTION_02,    //施設
  RESORT_PLACEID_INSTITUTION_03,    //施設
  RESORT_PLACEID_INSTITUTION_04,    //施設
  RESORT_PLACEID_INSTITUTION_05,    //施設
  RESORT_PLACEID_INSTITUTION_06,    //施設
  RESORT_PLACEID_INSTITUTION_07,    //施設
  RESORT_PLACEID_MAX,
}RESORT_PLACEID;

extern ZONEID RESORT_MAP_PlaceIDToZoneID( RESORT_PLACEID placeID );
extern RESORT_PLACEID RESORT_MAP_ZoneIDToPlaceID( ZONEID zoneID );
extern void RESORT_MAP_GetPlaceGridPosByIndex( RESORT_PLACEID placeID, u16 index, u16 *p_x, u16 *p_z );
