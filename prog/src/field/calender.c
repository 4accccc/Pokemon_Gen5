////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  カレンダー
 * @file   calender.h
 * @author obata
 * @date   2010.01.29
 */
////////////////////////////////////////////////////////////////////////////////
#include "calender.h"
#include "field/zonedata.h"
#include "ev_time.h"
#include "arc/arc_def.h"
#include "arc/calender.naix"


//==============================================================================
// ■定数
//==============================================================================
#define WEATHER_DATA_NONE (0xffff)  // データが登録されていない


//==============================================================================
// ■インデックス データ フォーマット
//==============================================================================
typedef struct
{
  u16 zoneID;     // ゾーンID
  u16 dataIndex;  // 先頭データのインデックス

} INDEX_DATA;


//==============================================================================
// ■カレンダーワーク
//==============================================================================
struct _CALENDER
{
  HEAPID     heapID;
  GAMEDATA*  gameData;
  ARCHANDLE* arcHandle;  // アーカイブハンドル
};


//==============================================================================
// ■非公開関数
//============================================================================== 
// カレンダーワーク
void InitCalenderWork( CALENDER* calender );
void OpenArcHandle   ( CALENDER* calender );
void CloseArcHandle  ( CALENDER* calender ); 
// 天気データ
WEATHER_NO GetWeatherNo_today( const CALENDER* calender, u16 zoneID );
WEATHER_NO GetWeatherNo      ( const CALENDER* calender, u16 zoneID, u8 month, u8 day ); 
// 辞書データ
u16 GetAnnualWeatherDataIndex ( const CALENDER* calender, u16 zoneID );
u16 GetDateIndex              ( u8 month, u8 day );


//==============================================================================
// ■公開関数
//============================================================================== 

//------------------------------------------------------------------------------
/**
 * @brief カレンダーを作成する
 *
 * @praram gameData
 * @param  heapID
 */
//------------------------------------------------------------------------------
CALENDER* CALENDER_Create( GAMEDATA* gameData, HEAPID heapID )
{
  CALENDER* calender;

  // 生成
  calender = GFL_HEAP_AllocMemory( heapID, sizeof(CALENDER) );

  // 初期化
  InitCalenderWork( calender );
  calender->heapID   = heapID;
  calender->gameData = gameData;
  OpenArcHandle( calender );

  return calender;
}

//------------------------------------------------------------------------------
/**
 * @brief カレンダーを破棄する
 *
 * @param calender
 */
//------------------------------------------------------------------------------
void CALENDER_Delete( CALENDER* calender )
{
  CloseArcHandle( calender );
  GFL_HEAP_FreeMemory( calender );
}

//------------------------------------------------------------------------------
/**
 * @brief 天気を取得する
 *
 * @param calender
 * @parma zoneID  ゾーンID
 * @param month   月[1, 12]
 * @param day     日[1, 31]
 *
 * @return 指定した 場所-日付 の天気
 */
//------------------------------------------------------------------------------
WEATHER_NO CALENDER_GetWeather( const CALENDER* calender, u16 zoneID, u8 month, u8 day )
{
  return GetWeatherNo( calender, zoneID, month, day );
}

//------------------------------------------------------------------------------
/**
 * @brief 今日の天気を取得する
 *
 * @param calender
 * @parma zoneID  ゾーンID
 *
 * @return 指定した場所の今日の天気
 */
//------------------------------------------------------------------------------
WEATHER_NO CALENDER_GetWeather_today( const CALENDER* calender, u16 zoneID )
{
  return GetWeatherNo_today( calender, zoneID );
}

//==============================================================================
// □カレンダーワーク
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief カレンダーワークを初期化する
 *
 * @param calender
 */
//------------------------------------------------------------------------------
void InitCalenderWork( CALENDER* calender )
{
  calender->heapID    = 0;
  calender->gameData  = NULL;
  calender->arcHandle = NULL;
}

//------------------------------------------------------------------------------
/**
 * @brief カレンダーデータのファイルハンドルを開く
 *
 * @param calender
 */
//------------------------------------------------------------------------------
void OpenArcHandle( CALENDER* calender )
{
  GF_ASSERT( calender );
  GF_ASSERT( calender->heapID );
  GF_ASSERT( calender->arcHandle == NULL );

  calender->arcHandle = GFL_ARC_OpenDataHandle( ARCID_CALENDER, calender->heapID );

  GF_ASSERT( calender->arcHandle );
}

//------------------------------------------------------------------------------
/**
 * @brief カレンダーデータのファイルハンドルを閉じる
 *
 * @param calender
 */
//------------------------------------------------------------------------------
void CloseArcHandle( CALENDER* calender )
{
  GF_ASSERT( calender );
  GF_ASSERT( calender->arcHandle );

  GFL_ARC_CloseDataHandle( calender->arcHandle );
  calender->arcHandle = NULL;
}


//==============================================================================
// □天気データ
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 今日の天気を取得する
 *
 * @param calender
 * @param zoneID   天気を取得する場所
 *
 * @return 指定した場所の今日の天気 ( WEATHER_NO_xxxx )
 */
//------------------------------------------------------------------------------
WEATHER_NO GetWeatherNo_today( const CALENDER* calender, u16 zoneID )
{
  int month, day;

  // 日付を取得
  month = EVTIME_GetMonth( calender->gameData );
  day   = EVTIME_GetDay( calender->gameData );

  // 今日の天気を返す
  return GetWeatherNo( calender, zoneID, month, day );
}

//------------------------------------------------------------------------------
/**
 * @brief 天気を取得する
 *
 * @param calender
 * @param zoneID   天気を取得する場所
 * @param month    天気を取得する月[1, 12]
 * @param day      天気を取得する日[1, 31]
 *
 * @return 指定した場所/日付の天気 ( WEATHER_NO_xxxx )
 */
//------------------------------------------------------------------------------
WEATHER_NO GetWeatherNo( const CALENDER* calender, u16 zoneID, u8 month, u8 day )
{
  u8 weather;
  u16 annualIndex, dateIndex;

  // 年間データのインデックス取得
  annualIndex = GetAnnualWeatherDataIndex( calender, zoneID );

  // 登録されていない
  if( annualIndex == WEATHER_DATA_NONE )
  {
    // ゾーンテーブルを参照
    return ZONEDATA_GetWeatherID( zoneID );
  }

  // 指定日の天気データのインデックス
  dateIndex = annualIndex + GetDateIndex( month, day );

  // 天気を取得
  GFL_ARC_LoadDataOfsByHandle( calender->arcHandle, NARC_calender_weather_data_bin, 
                               dateIndex, sizeof(weather), &weather );
  return weather;
}


//==============================================================================
// □辞書データ
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 年間データのインデックスを取得する
 *
 * @param calender
 * @param zoneID   ゾーンID
 *
 * @return 指定したゾーンの年間天気データのインデックス
 *         ( 天気データ中の, 指定ゾーン1/1の天気データの場所 )
 *
 *         指定したゾーンがテーブルに登録されていない場合 WEATHER_DATA_NONE
 */
//------------------------------------------------------------------------------
u16 GetAnnualWeatherDataIndex( const CALENDER* calender, u16 zoneID )
{
  int i;
  u16 index;
  u16 tableSize;
  INDEX_DATA* table;

  // テーブルサイズ取得
  GFL_ARC_LoadDataOfsByHandle( calender->arcHandle, NARC_calender_weather_index_bin, 
                               0, sizeof(tableSize), &tableSize );
  
  // テーブル作成
  table = GFL_HEAP_AllocMemory( calender->heapID, sizeof(INDEX_DATA) * tableSize );
  GFL_ARC_LoadDataOfsByHandle( calender->arcHandle, NARC_calender_weather_index_bin,
                               sizeof(tableSize), sizeof(INDEX_DATA) * tableSize, table );

  // 検索
  index = WEATHER_DATA_NONE;
  for( i=0; i < tableSize; i++ )
  {
    // 発見
    if( table[i].zoneID == zoneID )
    {
      index = table[i].dataIndex; 
      break;
    }
  }

  // テーブル破棄
  GFL_HEAP_FreeMemory( table );

  return index;
}

//------------------------------------------------------------------------------
/**
 * @brief 月間データのインデックスを取得する
 *
 * @param month    月[1, 12]
 * @param day      日[1, 31]
 *
 * @return 指定した日付の年間インデックス ( 何日目か )
 */
//------------------------------------------------------------------------------
u16 GetDateIndex( u8 month, u8 day )
{
  int i;
  u16 index;

  // 月ごとの日数
  const u8 dayOfMonth[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // 引数チェック
  GF_ASSERT( (month != 0) && (month <= 12) );
  GF_ASSERT( (day   != 0) && (day   <= dayOfMonth[month]) );

  // 何日目か
  index = 0;
  for( i=0; i<month; i++ ){ index += dayOfMonth[i]; }  // 前月までの日数
  index += (day - 1);  // 指定日

  return index;
}
