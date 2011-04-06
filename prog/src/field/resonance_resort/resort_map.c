//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_map.c
 *	@brief  レゾナンスリゾートのマップ処理
 *	@author	Toru=Nagihashi
 *	@data		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "resort_map.h"
//=============================================================================
/**
 *					マップデータ
*/
//=============================================================================
//-------------------------------------
///場所とゾーンID　//TODO
//=====================================
static const ZONEID sc_placeIDToZoneID[RESORT_PLACEID_MAX] =
{
  0,                          //RESORT_PLACEID_NONE,              //無効値
  RESORT_ZONE_ID_FIELD,       //RESORT_PLACEID_FIELD,             //フィールド
  RESORT_ZONE_ID_RECEPTION,   //RESORT_PLACEID_RECEPTION,         //迎賓館
  RESORT_ZONE_ID_EMBASSY,     //RESORT_PLACEID_EMBASSY,           //大使館
  RESORT_ZONE_ID_CONDOMINIUM_00,//RESORT_PLACEID_CONDOMINIUM_00,    //コンドミニアム
  RESORT_ZONE_ID_CONDOMINIUM_01,//RESORT_PLACEID_CONDOMINIUM_01,    //コンドミニアム
  RESORT_ZONE_ID_CONDOMINIUM_02,//RESORT_PLACEID_CONDOMINIUM_02,    //コンドミニアム
  RESORT_ZONE_ID_CONDOMINIUM_03,//RESORT_PLACEID_CONDOMINIUM_03,    //コンドミニアム
  RESORT_ZONE_ID_CONDOMINIUM_04,//RESORT_PLACEID_CONDOMINIUM_04,    //コンドミニアム
  RESORT_ZONE_ID_CONDOMINIUM_05,//RESORT_PLACEID_CONDOMINIUM_05,    //コンドミニアム
  RESORT_ZONE_ID_INSTITUTION_00,//RESORT_PLACEID_INSTITUTION_00,    //施設
  RESORT_ZONE_ID_INSTITUTION_01,//RESORT_PLACEID_INSTITUTION_01,    //施設
  RESORT_ZONE_ID_INSTITUTION_02,//RESORT_PLACEID_INSTITUTION_02,    //施設
  RESORT_ZONE_ID_INSTITUTION_03,//RESORT_PLACEID_INSTITUTION_03,    //施設
  RESORT_ZONE_ID_INSTITUTION_04,//RESORT_PLACEID_INSTITUTION_04,    //施設
  RESORT_ZONE_ID_INSTITUTION_05,//RESORT_PLACEID_INSTITUTION_05,    //施設
  RESORT_ZONE_ID_INSTITUTION_06,//RESORT_PLACEID_INSTITUTION_06,    //施設
  RESORT_ZONE_ID_INSTITUTION_07,//RESORT_PLACEID_INSTITUTION_07,    //施設
};


//----------------------------------------------------------------------------
/**
 *	@brief  場所IDからゾーンIDへ変換
 *
 *	@param	RESORT_PLACEID placeID  場所ID
 *
 *	@return ゾーンID
 */
//-----------------------------------------------------------------------------
ZONEID RESORT_MAP_PlaceIDToZoneID( RESORT_PLACEID placeID )
{
  return sc_placeIDToZoneID[ placeID ];
}

//----------------------------------------------------------------------------
/**
 *	@brief　ゾーンIDから場所IDへの変換
 *	          この関数はZONEIDに重複がないときにしか扱えないので禁止
 *
 *	@param	ZONEID zoneID 　ゾーンID
 *
 *	@return 場所ID
 */
//-----------------------------------------------------------------------------
/*
RESORT_PLACEID RESORT_MAP_ZoneIDToPlaceID( ZONEID zoneID )
{
  int i;
  for( i = 0; i < RESORT_PLACEID_MAX; i++ )
  {
    if( sc_placeIDToZoneID[i] == zoneID )
    {
      return i;
    }
  }
  GF_ASSERT(0);
  return 0;
}
*/
//----------------------------------------------------------------------------
/**
 *	@brief 場所による、インデックスごとの座標を取得
 *
 *	@param	RESORT_PLACEID placeID 場所ID
 *	@param  u16 index インデックス（主にpeopleID）
 *	@param	*p_x　Xグリッド座標
 *	@param	*p_z  Zグリッド座標
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetPlaceGridPosByIndex( RESORT_PLACEID placeID, u16 index, u16 *p_x, u16 *p_z )
{
  switch( placeID )
  {
  case RESORT_PLACEID_RECEPTION:
    *p_x  = 2 + (index%5)*2;
    *p_z  = 3 + (index/5)*2;
    break;
  case RESORT_PLACEID_FIELD:
    {
      static const struct
      {
        s16 x;
        s16 z;
      }sc_pos_data[]  =
      {
        {3,5},
        {16,5},
        {28,5},
        {3,18},
        {28,18},
        {3,31},
        {16,31},
        {28,31},
        //デバッグ用秘書位置 TODO
        {13,10},
        {13,13},
        {13,16},
        {19,10},
        {19,13},
        {19,16},
      };
      *p_x  = sc_pos_data[index].x;
      *p_z  = sc_pos_data[index].z;
    }
    break;
    //TODO
  default:
    GF_ASSERT(0);
  }
}
