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
#include "../../../resource/fldmapdata/land_data_patch/land_data_patch_def.h"
//=============================================================================
/**
 *					マップデータ
*/
//=============================================================================
//-------------------------------------
///場所とゾーンID
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
 *	@brief  住民の座標  (施設位置からのオフセット)
 *
 *	@param	u16 shop_3dmd  バッファ番号
 *	@param	*p_x  X座標
 *	@param	*p_x  Y座標
 *	@param	*p_dir  方向
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetResidentGridPosOfs( u16 shop_3dmd, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  static const struct
  {
    u16 x;
    u16 z;
    u32 dir;
  }sc_resident_pos[]  =
  {
		//LAND_DATA_PATCH_JA_CAFE_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_CAFE_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_CAFE_02_R
    { 2, 4, DIR_LEFT },
		//LAND_DATA_PATCH_JA_CAFE_02_L
    { 3, 2, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_CAFE_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_CAFE_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_LOT_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_LOT_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_LOT_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_LOT_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_LOT_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_LOT_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_PARLOR_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_PARLOR_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_PARLOR_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_PARLOR_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_PARLOR_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_PARLOR_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_MARKET_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_MARKET_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_MARKET_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_MARKET_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_MARKET_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_MARKET_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_FLORIST_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_FLORIST_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_FLORIST_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_FLORIST_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_FLORIST_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_FLORIST_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_NURSERY_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_NURSERY_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_NURSERY_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_NURSERY_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_NURSERY_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_NURSERY_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_EXERCICE_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_EXERCICE_02_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_EXERCICE_03_R
    { 1, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_EXERCICE_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_EXERCICE_02_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_EXERCICE_03_L
    { 4, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_ANTIQUE_01_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_ANTIQUE_02_R
    { 2, 3, DIR_LEFT },
		//LAND_DATA_PATCH_JA_ANTIQUE_03_R
    { 1, 4, DIR_LEFT },
		//LAND_DATA_PATCH_JA_ANTIQUE_01_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_ANTIQUE_02_L
    { 3, 3, DIR_RIGHT },
		//LAND_DATA_PATCH_JA_ANTIQUE_03_L
    { 4, 2, DIR_RIGHT },
  };
  GF_ASSERT( LAND_DATA_PATCH_MAX == NELEMS(sc_resident_pos) );

  GF_ASSERT( shop_3dmd < NELEMS(sc_resident_pos) );

  *p_x    = sc_resident_pos[shop_3dmd].x;
  *p_z    = sc_resident_pos[shop_3dmd].z;
  *p_dir  = sc_resident_pos[shop_3dmd].dir;
}


//----------------------------------------------------------------------------
/**
 *	@brief  看板の座標  (施設位置からのオフセット)
 *
 *	@param	u16 shop_3dmd  バッファ番号
 *	@param	*p_x  X座標
 *	@param	*p_x  Y座標
 *	@param	*p_dir  方向
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetBoardGridPosOfs( u16 shop_3dmd, u16 *p_x, u16 *p_z )
{
  static const struct
  {
    u16 x;
    u16 z;
  }sc_board_pos[]  =
  {
		//LAND_DATA_PATCH_JA_CAFE_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_CAFE_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_CAFE_02_R
    { 0, 3 },
		//LAND_DATA_PATCH_JA_CAFE_02_L
    { 5, 1 },
		//LAND_DATA_PATCH_JA_CAFE_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_CAFE_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_LOT_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_LOT_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_LOT_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_LOT_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_LOT_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_LOT_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_PARLOR_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_MARKET_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_MARKET_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_MARKET_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_MARKET_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_MARKET_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_MARKET_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_FLORIST_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_NURSERY_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_03_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_EXERCICE_03_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_ANTIQUE_01_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_ANTIQUE_02_R
    { 0, 2 },
		//LAND_DATA_PATCH_JA_ANTIQUE_03_R
    { 0, 3 },
		//LAND_DATA_PATCH_JA_ANTIQUE_01_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_ANTIQUE_02_L
    { 5, 2 },
		//LAND_DATA_PATCH_JA_ANTIQUE_03_L
    { 5, 1 },
  };

  GF_ASSERT( LAND_DATA_PATCH_MAX == NELEMS(sc_board_pos) );

  GF_ASSERT( shop_3dmd < NELEMS(sc_board_pos) );

  *p_x    = sc_board_pos[shop_3dmd].x;
  *p_z    = sc_board_pos[shop_3dmd].z;
}

//----------------------------------------------------------------------------
/**
 *	@brief  秘書の位置
 *
 *	@param	u16 kind  秘書の種類
 *	@param	*p_x    X座標
 *	@param	*p_z    Z座標
 *	@param	*p_dir  向き
 */
//-----------------------------------------------------------------------------
void RESORT_MAP_GetSecretaryGridPos( u16 kind, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  static const struct
  {
    u16 x;
    u16 z;
    u32 dir;
  }sc_secretary_pos[]  =
  {
    //設定
    {
      13,
      5,
      DIR_DOWN,
    },
    //移動
    {
      13,
      7,
      DIR_UP,
    },
    //ポケセン
    {
      11,
      11,
      DIR_LEFT,
    },
    //パソコン
    {
      10,
      10,
      DIR_DOWN,
    },
  };

  GF_ASSERT( kind < NELEMS(sc_secretary_pos) );

  *p_x    = sc_secretary_pos[kind].x;
  *p_z    = sc_secretary_pos[kind].z;
  *p_dir  = sc_secretary_pos[kind].dir;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物がいる場所
 *
 *	@param	u16 peopleID  人物
 *
 *	@return 場所
 */
//-----------------------------------------------------------------------------
RESORT_PLACEID RESORT_MAP_GetPeoplePlace( u16 peopleID )
{
  static const RESORT_PLACEID sc_place[]  =
  {
    RESORT_PLACEID_FIELD,//RESORT_PEOPLE_TYPE_RESIDENT (0) //住民
    RESORT_PLACEID_FIELD,//RESORT_PEOPLE_TYPE_PROXY (1) //代理人
    RESORT_PLACEID_FIELD,//RESORT_PEOPLE_TYPE_TOURIST (2) //旅行者
    RESORT_PLACEID_RECEPTION//RESORT_PEOPLE_TYPE_SECRETARY (3) //秘書
  };
  GF_ASSERT( peopleID < NELEMS(sc_place) );

  return sc_place[ peopleID ];
}
