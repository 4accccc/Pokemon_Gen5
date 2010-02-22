//============================================================================================
/**
 * @file  field_diving_data.c
 * @brief ダイビング接続先チェック用データとそのアクセス関数
 * @date  2010.02.21
 * @author  tamada GAME FREAK inc.
 */
//============================================================================================

#include <gflib.h>

#include "field_diving_data.h"

#include "fieldmap.h"
#include "field_player.h"
#include "fldmmdl.h"

#include "arc/fieldmap/zone_id.h"
//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct {
  u16 check_zone_id;
  u16 grid_x, grid_z;
  u16 connect_zone_id;
}DIVING_SPOT_TABLE;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "../../../resource/fldmapdata/diving_spot/diving_spot.cdat"

#define DIVING_SPOT_MAX NELEMS(DivingSpotTable)

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brief ダイビングした先のゾーンIDを取得する
 * @param zone_id   現在のゾーン
 * @param x     探すX位置（グリッド単位）
 * @param z     探すZ位置（グリッド単位）
 * @return  u16 zone_id ZONE_ID_MAXの時、見つからなかった
 */
//-----------------------------------------------------------------------------
static u16 DIVINGSPOT_GetZoneID( u16 zone_id, u16 x, u16 z )
{
  int i;
  const DIVING_SPOT_TABLE * tbl = DivingSpotTable;
  for ( i = 0; i < DIVING_SPOT_MAX; i++, tbl++ )
  {
    if (tbl->check_zone_id == zone_id
        && tbl->grid_x <= x && x < tbl->grid_x + 3
        && tbl->grid_z <= z && z < tbl->grid_z + 3 )
    {
      return tbl->connect_zone_id;
    }
  }
  return ZONE_ID_MAX;
}

//-----------------------------------------------------------------------------
/**
 * @brief ダイビングできるかどうかの判定
 * @param zone_id   現在のゾーン
 * @param x     探すX位置（グリッド単位）
 * @param z     探すZ位置（グリッド単位）
 * @return  BOOL  TRUEのとき、ダイビング可能
 */
//-----------------------------------------------------------------------------
static BOOL DIVINGSPOT_SearchData( u16 zone_id, u16 x, u16 z )
{
  return DIVINGSPOT_GetZoneID( zone_id, x, z) != ZONE_ID_MAX;
}

//-----------------------------------------------------------------------------
/**
 * @brief ダイビング判定処理
 * @param fieldmap  FIELDMAP_WORKへのポインタ
 * @param zone_id   遷移先マップIDを受け取るためのポインタ
 * @return  BOOL  TRUEのとき、ダイビング可能
 */
//-----------------------------------------------------------------------------
BOOL DIVINGSPOT_Check( FIELDMAP_WORK * fieldmap, u16 * zone_id )
{
  u16 now_zone_id = FIELDMAP_GetZoneID( fieldmap );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  u16 x = MMDL_GetGridPosX( mmdl );
  u16 z = MMDL_GetGridPosZ( mmdl );
  *zone_id = DIVINGSPOT_GetZoneID( now_zone_id, x, z );
  if ( *zone_id != ZONE_ID_MAX ) return TRUE;
  //万が一の不具合対処
  *zone_id = now_zone_id;
  return FALSE;
}

