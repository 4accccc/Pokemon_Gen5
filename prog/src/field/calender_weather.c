#include "calender_weather.h"
#include "arc/arc_def.h"
#include "arc/calender.naix"
#include "field/weather_no.h"


//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 1つのゾーンに関する, 1年間の天気データ
 */
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  u8  zoneID;   // ゾーンID
  u16 dateNum;  // 登録日付数
  u8* month;    // 月配列
  u8* day;      // 日配列
  u8* weather;  // 天気番号配列
}
WEATHER_DATA;


//========================================================================================
/**
 * @breif 非公開関数のプロトタイプ宣言
 */
//========================================================================================

// 指定した日付の天気を取得する
static u8 GetWeather( WEATHER_DATA* p_data, const RTCDate* p_date );


//========================================================================================
/**
 * @breif 非公開関数の定義
 */
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 指定した日付の天気を取得する
 *
 * @param p_data 天気データ
 * @param p_date 指定日
 *
 * @return 指定日の天気
 */
//----------------------------------------------------------------------------------------
static u8 GetWeather( WEATHER_DATA* p_data, const RTCDate* p_date )
{
  int i;
  int index = 0;

  // 指定日が含まれる場所を検索
  for( i=1; i<p_data->dateNum; i++ )
  {
    // 指定日を発見
    if( ( p_data->month[i] == p_date->month ) && ( p_data->day[i] == p_date->day ) )
    {
      index = i;
      break;
    }
    // 指定日より先の日付を発見
    if( ( p_date->month < p_data->month[i] ) ||
        ( ( p_date->month == p_data->month[i] ) && ( p_date->day < p_data->day[i] ) ) )
    {
      index = i - 1;
      break;
    }
  }

  // 指定日の天気を返す
  return p_data->weather[ index ];
}


//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief システムワーク
 */
//////////////////////////////////////////////////////////////////////////////////////////
struct _CALENDER_WEATHER_INFO
{
  u16           zoneNum;  // 登録ゾーン数
  WEATHER_DATA* data;     // 各ゾーンの天気データ
};


//========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//========================================================================================

// 指定した2バイトをリトルエンディアンとして解釈し, 値を取得する
static u16 GetU16( u8* data, int pos );

// データを読み込む
static void LoadData( CALENDER_WEATHER_INFO* p_info, HEAPID heap_id );


//========================================================================================
/**
 * @brief 公開関数の定義
 */
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief カレンダー天気データを作成する
 *
 * @param heap_id 使用するヒープID
 *
 * @return 作成したカレンダー天気データ
 */
//----------------------------------------------------------------------------------------
CALENDER_WEATHER_INFO* CALENDER_WEATHER_INFO_Create( HEAPID heap_id )
{
  CALENDER_WEATHER_INFO* p_info;

  // ワークを作成
  p_info = (CALENDER_WEATHER_INFO*)GFL_HEAP_AllocMemory( heap_id, sizeof( CALENDER_WEATHER_INFO ) );

  // データを読み込む
  LoadData( p_info, heap_id );

  // 作成したデータを返す
  return p_info;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 指定日・指定ゾーンの天気を取得する
 *
 * @param p_info  カレンダー天気データ
 * @param p_date  日付
 * @param zone_id ゾーンID
 *
 * @return 指定日の指定ゾーンの天気
 */
//----------------------------------------------------------------------------------------
u16 CALENDER_WEATHER_INFO_GetWeather( CALENDER_WEATHER_INFO* p_info, const RTCDate* p_date, u16 zone_id )
{
  int i;
  u16 weather_no = WEATHER_NO_NONE;

  // 指定ゾーンを検索
  for( i=0; i<p_info->zoneNum; i++ )
  { 
    // 発見
    if( p_info->data[i].zoneID == zone_id )
    {
      weather_no = (u16)GetWeather( &p_info->data[i], p_date );
      break;
    }
  }

  // DEBUG:
  OBATA_Printf( "%2d/%2d : zone_id = %d : weather no = %d\n", p_date->month, p_date->day, zone_id, weather_no );

  // 指定ゾーンが登録されていなければ, 無効値を返す
  return weather_no;
}

//----------------------------------------------------------------------------------------
/**
 * @brief カレンダー天気データを破棄する
 *
 * @param p_info 破棄するデータ
 */ 
//----------------------------------------------------------------------------------------
void CALENDER_WEATHER_INFO_Delete( CALENDER_WEATHER_INFO* p_info )
{
  int i;

  // 全ゾーンの天気データを破棄
  for( i=0; i<p_info->zoneNum; i++ )
  {
    GFL_HEAP_FreeMemory( p_info->data[i].month ); 
    GFL_HEAP_FreeMemory( p_info->data[i].day );
    GFL_HEAP_FreeMemory( p_info->data[i].weather );
  } 

  // 本体を破棄
  GFL_HEAP_FreeMemory( p_info->data );
  GFL_HEAP_FreeMemory( p_info );
}


//==================================================================================
/**
 * @brief 非公開関数の定義
 */
//==================================================================================

//---------------------------------------------------------------------------- 
/**
 * @brief data + pos の位置から始まる2バイトを, 
 *        リトルエンディアンで並んだu16のデータとして解釈し値を取得する
 *
 * @param data データ開始位置
 * @param pos  開始位置オフセット
 *
 * @return u16
 */
//---------------------------------------------------------------------------- 
static u16 GetU16( u8* data, int pos )
{
	u16 lower = (u16)( data[ pos ] );
	u16 upper = (u16)( data[ pos + 1 ] );
	u16 value = ( upper << 8 ) | lower;

	return value;
}

//---------------------------------------------------------------------------------
/**
 * @brief データを読み込む
 *
 * @param p_info  読み込んだデータの格納先
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------------
static void LoadData( CALENDER_WEATHER_INFO* p_info, HEAPID heap_id )
{
  int i, j;
  u8* src;
  int pos = 0; 

  // データ読み込み
  src = (u8*)GFL_ARC_LoadDataAlloc( ARCID_CALENDER, NARC_calender_weather_bin, heap_id ); 

  // 登録ゾーン数を取得
  p_info->zoneNum = GetU16( src, pos );

  // バッファ確保
  p_info->data = (WEATHER_DATA*)GFL_HEAP_AllocMemory( heap_id, sizeof( WEATHER_DATA ) * p_info->zoneNum );

  // 各ゾーンの天気データを取得
  for( i=0; i<p_info->zoneNum; i++ )
  {
    // ゾーンIDを取得
    pos = 2 + 4 * i;
    p_info->data[i].zoneID = GetU16( src, pos );
    pos += 2;

    // データ先頭位置を取得
    pos = GetU16( src, pos );

    // 登録日付数を取得
    p_info->data[i].dateNum = GetU16( src, pos );
    pos += 2;

    // バッファを確保
    p_info->data[i].month   = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->data[i].dateNum );
    p_info->data[i].day     = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->data[i].dateNum );
    p_info->data[i].weather = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->data[i].dateNum );

    // 登録されている全日付と, その天気を取得
    for( j=0; j<p_info->data[i].dateNum; j++ )
    {
      p_info->data[i].month[j]   = src[ pos++ ];
      p_info->data[i].day[j]     = src[ pos++ ];
      p_info->data[i].weather[j] = src[ pos++ ];
    }
  }

  // 後始末
  GFL_HEAP_FreeMemory( src ); 


  // DEBUG:
  OBATA_Printf( "zoneNum = %d\n", p_info->zoneNum );
  for( i=0; i<p_info->zoneNum; i++ )
  {
    OBATA_Printf( "---------------------------\n" );
    OBATA_Printf( "zoneID = %d, dateNum = %d\n", p_info->data[i].zoneID, p_info->data[i].dateNum );
    OBATA_Printf( "---------------------------\n" );
    for( j=0; j<p_info->data[i].dateNum; j++ )
    {
      OBATA_Printf( "%2d/%2d = %d\n", p_info->data[i].month[j], p_info->data[i].day[j], p_info->data[i].weather[j] );
    }
  }
}
