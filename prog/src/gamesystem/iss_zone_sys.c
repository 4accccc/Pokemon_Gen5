////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 
 * @brief ISSゾーンシステム
 * @file iss_zone_sys.c
 * @author obata
 * @date 2009.11.06
 *
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
  u16 zoneID;         // ゾーンID
  u16 padding;
  u8 initVolume[16];  // 各トラックの初期音量
} ZONE_DATA;


//==========================================================================================
// ■システムワーク
//==========================================================================================
struct _ISS_ZONE_SYS
{
  BOOL     active;  // 起動しているかどうか
  BOOL   initFlag;  // 音量初期化実行フラグ
  u16  initZoneID;  // 初期化実行対象のゾーンID

  GAMEDATA*     gdata;  // 監視対象のゲームデータ
  PLAYER_WORK* player;  // 監視対象の自機

  u8         zoneDataNum;  // 保持データ数
  ZONE_DATA*    zoneData;  // データ配列
};


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void InitISSZoneSystem(
    ISS_ZONE_SYS* sys, GAMEDATA* gdata, PLAYER_WORK* player, HEAPID heap_id );
static void LoadZoneData( ISS_ZONE_SYS* sys, HEAPID heap_id );
static void InitBGMVolume( ISS_ZONE_SYS* sys );
static void CheckSystemDown( ISS_ZONE_SYS* sys );
static void SystemOn( ISS_ZONE_SYS* sys );
static void SystemOff( ISS_ZONE_SYS* sys );


//==========================================================================================
// ■作成・破棄
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ISSゾーンシステムを作成する
 * 
 * @param gdata   監視対象のゲームデータ
 * @param player  監視対象の自機
 * @param heap_id 使用するヒープID
 *
 * @return 作成したシステム
 */
//------------------------------------------------------------------------------------------
ISS_ZONE_SYS* ISS_ZONE_SYS_Create( GAMEDATA* gdata, PLAYER_WORK* player, HEAPID heap_id )
{
  ISS_ZONE_SYS* sys;

  // 生成
  sys = GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_ZONE_SYS) );

  // 初期化
  InitISSZoneSystem( sys, gdata, player, heap_id );
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
 * @brief メイン動作
 *
 * @param sys 動かすシステム
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_Update( ISS_ZONE_SYS* sys )
{ 
  // 起動してない
  if( sys->active != TRUE ) return;

  // BGMボリュームの初期化
  InitBGMVolume( sys );

  // システム停止チェック
  CheckSystemDown( sys );
}

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
  // 初期化を登録
  sys->initFlag   = TRUE;
  sys->initZoneID = next_zone_id;

  // システム起動
  SystemOn( sys );
} 


//==========================================================================================
// ■非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief システムワークを初期化する
 *
 * @param sys     初期化するシステムワーク
 * @param gdata   監視対象のゲームデータ
 * @param player  監視対象の自機
 * @param heap_id 使用するヒープID
 */
//------------------------------------------------------------------------------------------
static void InitISSZoneSystem( 
    ISS_ZONE_SYS* sys, GAMEDATA* gdata, PLAYER_WORK* player, HEAPID heap_id )
{
  sys->active      = FALSE;
  sys->initFlag    = FALSE;
  sys->initZoneID  = 0;
  sys->gdata       = gdata;
  sys->player      = player;
  sys->zoneDataNum = 0;
  sys->zoneData    = NULL;

  // ゾーンデータを読み込む
  LoadZoneData( sys, heap_id );
}

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
  int i;
  int num;
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ISS_ZONE, heap_id );

  // データ数を取得
  num = GFL_ARC_GetDataFileCntByHandle( handle );
  sys->zoneDataNum = num;

  // バッファを確保
  sys->zoneData = GFL_HEAP_AllocMemory( heap_id, sizeof(ZONE_DATA) * num );

  // データを読み込む
  for( i=0; i<num; i++ )
  {
    GFL_ARC_LoadDataOfsByHandle( handle, i, 0, sizeof(ZONE_DATA), &sys->zoneData[i] );
  }
  GFL_ARC_CloseDataHandle( handle );

  // DEBUG:
  OBATA_Printf( "ISS-Z: load zone data\n" );
  for( i=0; i<num; i++ )
  {
    int j;
    OBATA_Printf( "- zoneData[%d]\n", i );
    OBATA_Printf( "-- zoneID = %d\n", sys->zoneData[i].zoneID ); 
    for( j=0; j<16; j++ )
    {
      OBATA_Printf( "-- track[%d].volume = %d\n", j, sys->zoneData[i].initVolume[j] );
    }
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief BGMのボリュームを初期化する
 *        ※ゾーンデータの内, 音量が0のトラックのみを反映させる。
 *        @todo 必要に応じて変更。
 *
 * @param 初期化を実行するシステム
 */
//------------------------------------------------------------------------------------------ 
static void InitBGMVolume( ISS_ZONE_SYS* sys )
{
  int i;
  u16 zone_id = PLAYERWORK_getZoneID( sys->player );

  // 初期化が登録されていない
  if( sys->initFlag != TRUE )
  {
    return;
  }
  // 自機が指定ゾーンにいない
  if( sys->initZoneID != zone_id )
  {
    return;
  }
  // 指定ゾーンのBGMが再生されていない
  {
    u8 season = GAMEDATA_GetSeasonID( sys->gdata );
    u16 def = ZONEDATA_GetBGMID( sys->initZoneID, season );
    u16 now = PMSND_GetBGMsoundNo();
    if( now != def )
    {
      return;
    }
  }

  // 所持データ内を検索
  for( i=0; i<sys->zoneDataNum; i++ )
  {
    // 指定ゾーンのデータを発見
    if( sys->zoneData[i].zoneID == zone_id )
    {
      int track;
      u16 track_bit = 0;
      // 音量0のトラックを検出
      for( track=0; track<16; track++ )
      {
        if( sys->zoneData[track].initVolume == 0 )
        {
          track_bit |= (1 << track);
        }
      }
      PMSND_ChangeBGMVolume( track_bit, 0 );  // 音量を変更
      // DEBUG:
      OBATA_Printf( "ISS-Z: init BGM volume\n" );
      break;
    }
  }
  sys->initFlag = FALSE;
}

//------------------------------------------------------------------------------------------ 
/**
 * @brief システム停止チェック
 *
 * @param sys チェックを行うシステム
 */
//------------------------------------------------------------------------------------------ 
static void CheckSystemDown( ISS_ZONE_SYS* sys )
{
  // すべての仕事がなくなったら停止
  if( sys->initFlag != TRUE )
  {
    SystemOff( sys );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//------------------------------------------------------------------------------------------
static void SystemOn( ISS_ZONE_SYS* sys )
{
  // すでに起動済み
  if( sys->active ) return;

  // 起動
  sys->active = TRUE;

  // DEBUG:
  OBATA_Printf( "ISS-Z: On\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param sys 停止させるシステム
 */
//------------------------------------------------------------------------------------------
static void SystemOff( ISS_ZONE_SYS* sys )
{
  // すでに停止済み
  if( sys->active != TRUE ) return;

  // 停止
  sys->active = FALSE;

  // DEBUG:
  OBATA_Printf( "ISS-Z: Off\n" );
}
