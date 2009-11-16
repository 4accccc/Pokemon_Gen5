////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ISSゾーンシステム
 * @file iss_zone_sys.c
 * @author obata
 * @date 2009.11.06
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/playerwork.h"
#include "iss_zone_sys.h"
#include "sound/pm_sndsys.h"    // for PMSND_ChangeBGMVolume
#include "gamesystem/game_data.h"
#include "field/zonedata.h"
#include "arc/arc_def.h"  // for ARCID_ISS_ZONE


//==========================================================================================
// ■ゾーンごとのデータ
//==========================================================================================
typedef struct
{
  u16 zoneID;        // ゾーンID
  u16 openTrackBit;  // 開放トラックビット
  u16 closeTrackBit; // 閉鎖トラックビット

} ZONE_DATA;


//==========================================================================================
// ■システムワーク
//==========================================================================================
struct _ISS_ZONE_SYS
{
  u8      zoneDataNum;  // 保持データ数
  ZONE_DATA* zoneData;  // データ配列
  u8   currentDataIdx;  // 参照データのインデックス
};


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void LoadZoneData( ISS_ZONE_SYS* sys, HEAPID heap_id );
static u8 SearchZoneData( const ISS_ZONE_SYS* sys, u16 zone_id );


//==========================================================================================
// ■作成・破棄
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ISSゾーンシステムを作成する
 * 
 * @param heap_id 使用するヒープID
 *
 * @return 作成したシステム
 */
//------------------------------------------------------------------------------------------
ISS_ZONE_SYS* ISS_ZONE_SYS_Create( HEAPID heap_id )
{
  ISS_ZONE_SYS* sys;

  // 生成
  sys = GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_ZONE_SYS) );
  sys->zoneDataNum = 0;
  sys->zoneData    = NULL;

  // データ読み込み
  LoadZoneData( sys, heap_id );
  return sys;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ISSゾーンシステムを破棄する
 *
 * @param sys 破棄するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_Delete( ISS_ZONE_SYS* sys )
{ 
  // ゾーンデータ
  if( sys->zoneData )
  {
    GFL_HEAP_FreeMemory( sys->zoneData );
  }
  // システム本体
  GFL_HEAP_FreeMemory( sys );
}


//==========================================================================================
// ■動作
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param sys          通知対象のシステム
 * @param next_zone_id 新しいゾーンID
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_ZoneChange( ISS_ZONE_SYS* sys, u16 next_zone_id )
{
} 


//==========================================================================================
// ■非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーンデータを読み込む
 *
 * @param sys     読み込んだデータを保持するシステム
 * @param heap_id 使用するヒープID
 */
//------------------------------------------------------------------------------------------
static void LoadZoneData( ISS_ZONE_SYS* sys, HEAPID heap_id )
{
  int data_idx;
  int num;
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ISS_ZONE, heap_id );

  // データ数を取得
  num = GFL_ARC_GetDataFileCntByHandle( handle );
  sys->zoneDataNum = num;

  // バッファを確保
  sys->zoneData = GFL_HEAP_AllocMemory( heap_id, sizeof(ZONE_DATA) * num );

  // データを読み込む
  for( data_idx=0; data_idx<num; data_idx++ )
  {
    GFL_ARC_LoadDataOfsByHandle( 
        handle, data_idx, 0, sizeof(ZONE_DATA), &sys->zoneData[data_idx] );
  }
  GFL_ARC_CloseDataHandle( handle );

  // DEBUG:
  OBATA_Printf( "ISS-Z: load zone data\n" );
  for( data_idx=0; data_idx<num; data_idx++ )
  {
    int j;
    OBATA_Printf( "- zoneData[%d]\n", data_idx );
    OBATA_Printf( "-- zoneID = %d\n", sys->zoneData[data_idx].zoneID ); 
    OBATA_Printf( "-- openTrack = " );
    for( j=0; j<16; j++ )
    {
      if( sys->zoneData[data_idx].openTrackBit & (1 << (15-j)) ) OBATA_Printf( "1" );
      else                                                       OBATA_Printf( "0" );
    }
  }
} 
