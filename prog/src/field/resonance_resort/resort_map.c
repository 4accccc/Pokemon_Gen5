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

        //これ以上なることはない
        {4,6},
        {6,7},
        {8,6},
        {10,7},
        {12,6},
        {14,7},
        {16,6},
        {18,7},
      };

      GF_ASSERT( peopleID < NELEMS(sc_pos_data) );
      *p_x  = sc_pos_data[peopleID].x;
      *p_z  = sc_pos_data[peopleID].z;
      *p_dir = DIR_DOWN;
    }
    break;
  case RESORT_PLACEID_FIELD:
    {
      *p_x  = 11+peopleID%10;
      *p_z  = 9+peopleID/10*2;
    }
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民の座標
 *
 *	@param	u16 buffer_index  バッファ番号
 *	@param	*p_x  X座標
 *	@param	*p_x  Y座標
 *	@param	*p_dir  方向
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetResidentGridPos( u16 buffer_index, u16 shop_rank, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  static const struct
  {
    u16 x;
    u16 z;
    u32 dir;
  }sc_resident_pos[]  =
  {
    {
      21,
      63,
      DIR_LEFT,
    },
    {
      9,
      56,
      DIR_RIGHT,
    },
    {
      21,
      49,
      DIR_LEFT,
    },
    {
      9,
      42,
      DIR_RIGHT,
    },
    {
      21,
      35,
      DIR_LEFT,
    },
    {
      9,
      28,
      DIR_RIGHT,
    },
    {
      21,
      21,
      DIR_LEFT,
    },
    {
      9,
      14,
      DIR_RIGHT,
    },
  };

  GF_ASSERT( buffer_index < NELEMS(sc_resident_pos) );

  *p_x    = sc_resident_pos[buffer_index].x;
  *p_z    = sc_resident_pos[buffer_index].z;
  *p_dir  = sc_resident_pos[buffer_index].dir;

  if( shop_rank == 3 )
  {
    if( *p_dir == DIR_RIGHT )
    {
      (*p_x)--;
    }
    else
    {
      (*p_x)++;
    }
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
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_WORK
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_SPACE
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_MOVE
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_CENTER
    RESORT_PLACEID_RECEPTION,//RESORT_SECRETARY_TYPE_PC
  };

  return sc_secretary_place[ type ];
}
