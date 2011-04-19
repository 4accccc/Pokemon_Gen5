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
#include "resort_people_def.h"
#include "field/field_dir.h"
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
 *	          この関数はZONEIDに重複がないときにしか扱えないのに注意！！
 *
 *	@param	ZONEID zoneID 　ゾーンID
 *
 *	@return 場所ID
 */
//-----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
/**
 *	@brief 場所による、インデックスごとの座標を取得
 *
 *	@param	RESORT_PLACEID placeID 場所ID
 *	@param  type　人物タイプ
 *	@param  u16 index インデックス（主にpeopleID）
 *	@param	*p_x　Xグリッド座標
 *	@param	*p_z  Zグリッド座標
 *	@param	*p_dir 方向
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetPlaceGridPosByIndex( RESORT_PLACEID placeID, u16 people_type, u16 kind, u16 peopleID, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  switch( placeID )
  {
  case RESORT_PLACEID_RECEPTION:
    if( people_type == RESORT_PEOPLE_TYPE_SECRETARY )
    {
      GF_ASSERT( kind == RESORT_SECRETARY_TYPE_SPACE );
      *p_x  = 13;
      *p_z  = 13;
      *p_dir = DIR_LEFT;
    }
    else
    {
      static const struct
      {
        s16 x;
        s16 z;
      }sc_pos_data[]  =
      {
        {4,2},
        {6,3},
        {8,2},
        {10,3},
        {12,2},
        {14,3},
        {16,2},
        {18,3},
        {4,4},
        {6,5},
        {8,4},
        {10,5},
        {12,4},
        {14,5},
        {16,4},
        {18,5},
      };

      GF_ASSERT( peopleID < NELEMS(sc_pos_data) );
      *p_x  = sc_pos_data[peopleID].x;
      *p_z  = sc_pos_data[peopleID].z;
      *p_dir = DIR_DOWN;
    }
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
      *p_x  = sc_pos_data[peopleID].x;
      *p_z  = sc_pos_data[peopleID].z;
    }
    break;
  case RESORT_PLACEID_INSTITUTION_00:
  case RESORT_PLACEID_INSTITUTION_01:
  case RESORT_PLACEID_INSTITUTION_02:
  case RESORT_PLACEID_INSTITUTION_03:
  case RESORT_PLACEID_INSTITUTION_04:
  case RESORT_PLACEID_INSTITUTION_05:
  case RESORT_PLACEID_INSTITUTION_06:
  case RESORT_PLACEID_INSTITUTION_07:
    if( people_type == RESORT_PEOPLE_TYPE_RESIDENT )
    {
      *p_x = 11;
      *p_z = 12;
      *p_dir  = DIR_LEFT;
    }
    else
    {
      *p_x = 3+peopleID;
      *p_z = 6;
    }
    break;
  case RESORT_PLACEID_EMBASSY:
    //迎賓館の秘書位置
    if( people_type == RESORT_PEOPLE_TYPE_SECRETARY )
    {
      switch( kind )
      {
      case RESORT_SECRETARY_TYPE_WORK:
        *p_x    = 11;
        *p_z    = 8;
        *p_dir  = DIR_DOWN;
        break;
      case RESORT_SECRETARY_TYPE_MOVE:
        *p_x    = 4;
        *p_z    = 13;
        *p_dir  = DIR_RIGHT;
        break;
      case RESORT_SECRETARY_TYPE_CENTER:
        *p_x    = 11;
        *p_z    = 10;
        *p_dir  = DIR_UP;
        break;
      case RESORT_SECRETARY_TYPE_PC:
        *p_x    = 10;
        *p_z    = 14;
        *p_dir  = DIR_DOWN;
        break;
      default:
        GF_ASSERT(0);
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書タイプ
 *
 *	@param	u16 type  秘書タイプ
 *
 *	@return 場所ID
 */
//-----------------------------------------------------------------------------
RESORT_PLACEID RESORT_MAP_GetSecretaryPlace( u16 type )
{
  static const RESORT_PLACEID sc_secretary_place[RESORT_SECRETARY_TYPE_MAX] =
  {
    RESORT_PLACEID_EMBASSY,//RESORT_SECRETARY_TYPE_WORK
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_SPACE
    RESORT_PLACEID_EMBASSY,//RESORT_SECRETARY_TYPE_MOVE
    RESORT_PLACEID_EMBASSY,//RESORT_SECRETARY_TYPE_CENTER
    RESORT_PLACEID_EMBASSY,//RESORT_SECRETARY_TYPE_PC
  };

  return sc_secretary_place[ type ];
}
