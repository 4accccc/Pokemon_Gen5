///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_city_unit.c
 * @brief  街ISSユニット
 * @author obata_toshihiro
 * @date   2009.11.02
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_city_unit.h"
#include "field/field_const.h" // for FX32_TO_GRID
#include "arc_def.h"  // for ARCID_ISS_CITY
#include "../../../resource/fldmapdata/zonetable/zone_id.h" 


//===========================================================================================
// ■定数
//===========================================================================================

// 1つの街ISSユニットが持つ音量空間の数
#define VOLUME_SPACE_NUM (4)



//===========================================================================================
// ■街ISSユニット構造体
//===========================================================================================
struct _ISS_C_UNIT
{
	// 配置場所のゾーンID
	u16 zoneID;
  u16 padding;

	// 座標[grid]
	int x;
	int y;
	int z;

	// 音量空間
  u8 volume[VOLUME_SPACE_NUM];
  u8 x_range[VOLUME_SPACE_NUM];
  u8 y_range[VOLUME_SPACE_NUM];
  u8 z_range[VOLUME_SPACE_NUM];
};

//-------------------------------------------------------------------------------------------
/**
 * @brief 指定した座標が空間内にあるかどうかを判定する
 *
 * @param unit  判定対象のユニット
 * @param index 判定対象の音量空間を指定
 * @param cx	  音量空間中心部のx座標
 * @param cy	  音量空間中心部のy座標
 * @param cz	  音量空間中心部のz座標
 * @param x		  主人公のx座標
 * @param y		  主人公のy座標
 * @param z		  主人公のz座標
 *
 * @return (x, y, z) が 指定した空間の内部にある場合 TRUE
 */
//-------------------------------------------------------------------------------------------
static BOOL IsCover( const ISS_C_UNIT* unit, int index, int cx, int cy, int cz, int x, int y, int z )
{
	int min_x, min_y, min_z;
	int max_x, max_y, max_z;

	// 空間の最小値・最大値を求める
	min_x = cx - unit->x_range[index];
	min_y = cy - unit->y_range[index];
	min_z = cz - unit->z_range[index]; 
	max_x = cx + unit->x_range[index];
	max_y = cy + unit->y_range[index];
	max_z = cz + unit->z_range[index];

	// 判定
	if( ( x < min_x ) | ( max_x < x ) |
		  ( y < min_y ) | ( max_y < y ) |
		  ( z < min_z ) | ( max_z < z ) )
  {
    return FALSE;
  } 
	return TRUE;
}


//=========================================================================================
// ■作成・破棄
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @breif ユニットを作成する
 *
 * @param heap_id 使用するヒープID
 * @param dat_id  読み込むユニットデータのアーカイブ内インデックス
 */
//-----------------------------------------------------------------------------------------
ISS_C_UNIT* ISS_C_UNIT_Create( HEAPID heap_id, u16 dat_id )
{
  ISS_C_UNIT* unit = NULL;

  // 読み込み
  unit = GFL_ARC_LoadDataAllocOfs( ARCID_ISS_CITY, dat_id, heap_id, 0, sizeof(ISS_C_UNIT) );

  // DEBUG:
  OBATA_Printf( "ISS-C-UNIT: Create\n" );
  ISS_C_UNIT_DebugPrint( unit );
  return unit; 
}

//-----------------------------------------------------------------------------------------
/**
 * @breif ユニットを破棄する
 *
 * @param unit 破棄するユニット
 */
//-----------------------------------------------------------------------------------------
void ISS_C_UNIT_Delete( ISS_C_UNIT* unit )
{
  GFL_HEAP_FreeMemory( unit );
}


//=========================================================================================
// ■取得
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief ゾーンIDを取得する
 *
 * @param unit 取得対象のユニット
 *
 * @return 指定したユニットのゾーンID
 */
//-----------------------------------------------------------------------------------------
u16 ISS_C_UNIT_GetZoneID( const ISS_C_UNIT* unit )
{
  return unit->zoneID;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 音量を取得する
 *
 * @param unit 操作対象のユニット
 * @param pos  主人公の座標[単位：ワールド座標]
 *
 * @return 指定した主人公座標における音量[0, 127]
 */
//-------------------------------------------------------------------------------------------
int ISS_C_UNIT_GetVolume( const ISS_C_UNIT* unit, const VecFx32* pos )
{
	int i;
	int volume = 0;
	int x, y, z;

	// グリッド座標を計算
  x = FX32_TO_GRID(pos->x);
  y = FX32_TO_GRID(pos->y);
  z = FX32_TO_GRID(pos->z);

	// 大きな空間から判定し, 属さない空間を見つけたら, 検索終了
	for( i=0; i<VOLUME_SPACE_NUM; i++ )
	{
		if( IsCover( unit, i, unit->x, unit->y, unit->z, x, y, z ) != TRUE )
		{
			break;
		}

		// 主人公が属する, 最も小さい空間に設定された音量を取得する
		volume = unit->volume[i];
	} 
	return volume;
}


//=========================================================================================
// ■デバッグ
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief ユニット情報を出力する
 *
 * @param unit 出力対象ユニット
 */
//-----------------------------------------------------------------------------------------
void ISS_C_UNIT_DebugPrint( const ISS_C_UNIT* unit )
{
  int i;
  OBATA_Printf( "- zoneID = %d\n", unit->zoneID );
  OBATA_Printf( "- pos = (%d, %d, %d)\n", unit->x, unit->y, unit->z );
  for( i=0; i<VOLUME_SPACE_NUM; i++ )
  {
    OBATA_Printf( "- volume, range = %d, (%d, %d, %d)\n",
        unit->volume[i], unit->x_range[i], unit->y_range[i], unit->z_range[i] );
  }
}
