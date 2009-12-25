////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ISSゾーンシステム
 * @file iss_zone_sys.c
 * @author obata
 * @date 2009.11.06
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_zone_sys.h"
#include "sound/pm_sndsys.h"  // for PMSND_ChangeBGMVolume
#include "arc/arc_def.h"      // for ARCID_ISS_ZONE


//==========================================================================================
// ■定数
//==========================================================================================
#define TRACK_NUM          (16)             // トラック数
#define MAX_TRACK_NO       (TRACK_NUM - 1)  // トラック番号の最大値
#define INVALID_DATA_INDEX (0xff)           // 参照データなし
#define MAX_VOLUME         (127)            // ボリューム最大値


//==========================================================================================
// ■ゾーンごとのデータ
//==========================================================================================
typedef struct
{
  u16 zoneID;        // ゾーンID
  u16 openTrackBit;  // 開放トラックビット
  u16 closeTrackBit; // 閉鎖トラックビット
  u16 fadeFrame;     // フェード時間

} ZONE_DATA;


//==========================================================================================
// ■システムワーク
//==========================================================================================
struct _ISS_ZONE_SYS
{
  BOOL boot;  // 起動しているかどうか

  u8      zoneDataNum;  // 保持データ数
  ZONE_DATA* zoneData;  // データ配列
  u8   currentDataIdx;  // 参照データのインデックス

  u16 fadeInTrackBit;   // フェードインさせるトラックビット
  u16 fadeOutTrackBit;  // フェードアウトさせるトラックビット
  u16 fadeFrame;        // フェード時間
  u16 fadeCount;        // フェードカウンタ
};


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void LoadZoneData( ISS_ZONE_SYS* sys, HEAPID heap_id );
static void BootSystem( ISS_ZONE_SYS* sys, u16 zone_id );
static void StopSystem( ISS_ZONE_SYS* sys );
static void ChangeZoneData( ISS_ZONE_SYS* sys, u16 zone_id );
static u8 SearchZoneData( const ISS_ZONE_SYS* sys, u16 zone_id );
static void UpdateFade( ISS_ZONE_SYS* sys );


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
  sys->boot            = FALSE;
  sys->zoneDataNum     = 0;
  sys->zoneData        = NULL;
  sys->currentDataIdx  = INVALID_DATA_INDEX;
  sys->fadeInTrackBit  = 0;
  sys->fadeOutTrackBit = 0;
  sys->fadeFrame       = 0;
  sys->fadeCount       = 0;

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
 * @brief システム・メイン動作
 *
 * @param sys 動かすシステム
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_Update( ISS_ZONE_SYS* sys )
{
  // 起動していない
  if( !sys->boot ){ return; }

  // フェード状態を更新
  UpdateFade( sys );
}


//==========================================================================================
// ■制御
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
  // 起動していない
  if( !sys->boot ){ return; }

  // 参照するデータを変更
  ChangeZoneData( sys, next_zone_id );
} 

//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys     起動するシステム
 * @param zone_id 起動時のゾーンID
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_On( ISS_ZONE_SYS* sys, u16 zone_id )
{
  BootSystem( sys, zone_id );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys     停止するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_ZONE_SYS_Off( ISS_ZONE_SYS* sys )
{
  StopSystem( sys );
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
    OBATA_Printf( "-- openTrack  = " );
    for( j=0; j<16; j++ )
    {
      if( sys->zoneData[data_idx].openTrackBit & (1 << (MAX_TRACK_NO-j)) ) 
        OBATA_Printf( "■" );
      else                                                       
        OBATA_Printf( "□" );
    }
    OBATA_Printf( "\n" );
    OBATA_Printf( "-- closeTrack = " );
    for( j=0; j<16; j++ )
    {
      if( sys->zoneData[data_idx].closeTrackBit & (1 << (MAX_TRACK_NO-j)) ) 
        OBATA_Printf( "■" );
      else                                                       
        OBATA_Printf( "□" );
    }
    OBATA_Printf( "\n" );
    OBATA_Printf( "-- fadeFrame = %d\n", sys->zoneData[data_idx].fadeFrame ); 
  }
} 

//------------------------------------------------------------------------------------------
/**
 * @brief システム起動処理
 *
 * @param sys     起動するシステム
 * @param zone_id 初期ゾーンID
 */
//------------------------------------------------------------------------------------------
static void BootSystem( ISS_ZONE_SYS* sys, u16 zone_id )
{
  u8 idx;

  // すでに起動している
  if( sys->boot ){ return; }

  // ゾーンデータを検索
  idx = SearchZoneData( sys, zone_id );

  // 指定ゾーンのデータを持っていない
  if( idx == INVALID_DATA_INDEX ){ return; }

  // 初期状態を設定
  sys->boot            = TRUE;
  sys->currentDataIdx  = idx;
  sys->fadeInTrackBit  = sys->zoneData[idx].openTrackBit;
  sys->fadeOutTrackBit = sys->zoneData[idx].closeTrackBit;
  sys->fadeFrame       = 1;   // 起動時はフェード無しで反映させる
  sys->fadeCount       = 0;

  // DEBUG:
  OBATA_Printf( "ISS-Z: boot\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システム停止処理
 *
 * @param sys 停止するシステム
 */
//------------------------------------------------------------------------------------------
static void StopSystem( ISS_ZONE_SYS* sys )
{
  // すでに停止している
  if( !sys->boot ){ return; }

  // 停止
  sys->boot = FALSE;

  // DEBUG:
  OBATA_Printf( "ISS-Z: stop\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 参照データを変更する
 *
 * @param sys     変更するシステム
 * @param zone_id 変更先データのゾーンID
 */
//------------------------------------------------------------------------------------------
static void ChangeZoneData( ISS_ZONE_SYS* sys, u16 zone_id )
{
  u8 now;
  u8 next;
  ZONE_DATA* data;

  // 参照するゾーンデータを検索
  now  = sys->currentDataIdx;
  next = SearchZoneData( sys, zone_id ); 
  data = sys->zoneData;

  // 新たに参照するデータが存在しない
  if( next == INVALID_DATA_INDEX ){ return; }

  // フェード状態を更新
  sys->fadeInTrackBit  = data[now].closeTrackBit & data[next].openTrackBit;
  sys->fadeOutTrackBit = data[now].openTrackBit  & data[next].closeTrackBit;
  sys->fadeFrame       = data[next].fadeFrame;
  sys->fadeCount       = 0;

  // 参照先データのインデックスを更新
  sys->currentDataIdx = next;

  // DEBUG:
  OBATA_Printf( "ISS-Z: change zone data\n" );
  OBATA_Printf( "- now = %d\n", now );
  OBATA_Printf( "- next = %d\n", next );
}

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーンIDをキーにして, データのインデックスを検索する
 *
 * @param sys     検索対象システム
 * @param zone_id 検索データのゾーンID
 *
 * @return 指定したゾーンIDを持つデータのインデックス
 *         存在しない場合は INVALID_DATA_INDEX を返す
 */
//------------------------------------------------------------------------------------------
static u8 SearchZoneData( const ISS_ZONE_SYS* sys, u16 zone_id )
{
  u8 idx;

  for( idx=0; idx<sys->zoneDataNum; idx++ )
  {
    // 発見
    if( sys->zoneData[idx].zoneID == zone_id )
    {
      return idx;
    }
  }

  // 存在しない
  OBATA_Printf( "ISS-Z: don't have data (zoneID = %d)\n", zone_id );
  return INVALID_DATA_INDEX;  
}

//------------------------------------------------------------------------------------------
/**
 * @brief フェード状態を更新する
 *
 * @param sys 更新するシステム
 */
//------------------------------------------------------------------------------------------
static void UpdateFade( ISS_ZONE_SYS* sys )
{
  int vol;

  // すでに完了している
  if( sys->fadeFrame <= sys->fadeCount ){ return; }

  // カウンタ更新
  sys->fadeCount++;

  // フェードイン
  if( sys->fadeInTrackBit )
  {
    vol = MAX_VOLUME * ((float)sys->fadeCount / (float)sys->fadeFrame);
    PMSND_ChangeBGMVolume( sys->fadeInTrackBit, vol );
    // DEBUG:
    OBATA_Printf( "ISS-Z: update fade in ==> %d\n", vol );
  }
  // フェードアウト
  if( sys->fadeOutTrackBit )
  {
    vol = MAX_VOLUME * ( 1.0f - ((float)sys->fadeCount / (float)sys->fadeFrame) );
    PMSND_ChangeBGMVolume( sys->fadeOutTrackBit, vol );
    // DEBUG:
    OBATA_Printf( "ISS-Z: update fade out ==> %d\n", vol );
  } 
}
