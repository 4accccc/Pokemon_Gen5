//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_light_status.c
 *  @brief  バトルに引き継ぐフィールドライトカラー
 *  @author tomoya takahashi
 *	@date		2010.02.02
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc_def.h"
#include "arc/field_light.naix"
#include "arc/field_weather_light.naix"

#include "gamesystem/pm_season.h"

#include "field/field_light_status.h"
#include "field/weather_no.h"
#include "field/zonedata.h"
#include "field/areadata.h"

#include "field_light.h"
#include "field_zonefog.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static LIGHT_DATA* LIGHT_STATUS_LoadData( u32 arcID, u32 dataIdx, u32* p_num, HEAPID heapID );
static void LIGHT_STATUS_GetTimeColor( LIGHT_DATA* p_wk, u32 num, int time, FIELD_LIGHT_STATUS* p_color );


//----------------------------------------------------------------------------
/**
 *	@brief  ライト情報の取得
 *
 *	@param	zone_id       ゾーンID
 *	@param	time          時間
 *	@param	weather_id    天気ID
 *	@param	season        季節
 *	@param	p_status      状態パラメータ
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_STATUS_Get( u32 zone_id, int hour, int minute, int weather_id, int season, FIELD_LIGHT_STATUS* p_status, HEAPID heapID )
{
  u32 zone_light_idx;
  LIGHT_DATA* p_data = NULL;
  u32 data_num;
  int time;
  
  GF_ASSERT( weather_id < WEATHER_NO_NUM );
  GF_ASSERT( season < PMSEASON_TOTAL );

  // 秒に変換 
  time = (hour * 60 * 60) + (minute * 60);
  time /= 2;


  // 天気
  if( weather_id != WEATHER_NO_SUNNY )
  {
    // 天気ライトを取得
    p_data = LIGHT_STATUS_LoadData( ARCID_FIELD_WEATHER_LIGHT, FIELD_LIGHT_STATUS_GetWeatherLightDatIdx(weather_id), &data_num, GFL_HEAP_LOWID(heapID) );
  }

  // フォグZONE
  if( p_data == NULL )
  {
    zone_light_idx = ZONEDATA_GetLight( zone_id );
    if( zone_light_idx != FIELD_ZONEFOGLIGHT_DATA_NONE )
    {
      // ゾーン用ライトを取得
      p_data = LIGHT_STATUS_LoadData( ARCID_ZONELIGHT_TABLE, zone_light_idx, &data_num, GFL_HEAP_LOWID(heapID) );
    }
  }

  //　通常テーブル
  if( p_data == NULL )
  {
    AREADATA* p_area = AREADATA_Create( GFL_HEAP_LOWID(heapID), ZONEDATA_GetAreaID(zone_id), season );
      
    p_data = LIGHT_STATUS_LoadData( ARCID_FIELD_LIGHT, AREADATA_GetLightType(p_area), &data_num, GFL_HEAP_LOWID(heapID) );

    AREADATA_Delete( p_area );
  }

  // 時間の色を取得
  LIGHT_STATUS_GetTimeColor( p_data, data_num, time, p_status );

  GFL_HEAP_FreeMemory( p_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief  天気IDのライトデータインデックスを取得
 *
 *	@param	weather_id  天気
 *  
 *	@retval 天気IDのライトデータインデックス
 *	@retval FIELD_LIGHT_STATUS_WEATHER_LIGHT_DAT_NONE
 */
//-----------------------------------------------------------------------------
u16 FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( int weather_id )
{
  static const u32 sc_WEATHER_LIGHT[ WEATHER_NO_NUM ] = 
  {
    FIELD_LIGHT_STATUS_WEATHER_LIGHT_DAT_NONE,
    NARC_field_weather_light_light_show_dat,
    NARC_field_weather_light_light_rain_dat,
    NARC_field_weather_light_light_storm_dat,
    NARC_field_weather_light_light_rain_dat,
    NARC_field_weather_light_light_show_dat,
    NARC_field_weather_light_light_rain_dat,
    NARC_field_weather_light_light_raikami_dat,
    NARC_field_weather_light_light_kazakami_dat,
    FIELD_LIGHT_STATUS_WEATHER_LIGHT_DAT_NONE,
  };
  GF_ASSERT( weather_id < WEATHER_NO_NUM );
  return sc_WEATHER_LIGHT[ weather_id ];
}



//----------------------------------------------------------------------------
/**
 *	@brief  ライトデータ読み込み
 *
 *	@param	arcID       アーカイブデータ
 *	@param	dataIdx     データインデックス
 *	@param  p_num       データ数格納先
 *	@param	heapID      ヒープID
 *
 *	@retval ライトデータ
 */
//-----------------------------------------------------------------------------
static LIGHT_DATA* LIGHT_STATUS_LoadData( u32 arcID, u32 dataIdx, u32* p_num, HEAPID heapID )
{
  u32 size;
  int i, j;
  u32 data_num;
  LIGHT_DATA* p_buff;

  
  size = GFL_ARC_GetDataSize( arcID, dataIdx );
  data_num = size / sizeof(LIGHT_DATA);

  p_buff = GFL_ARC_LoadDataAlloc( arcID, dataIdx, heapID );


  // 方向ベクトルの値を一応単位ベクトルにする。
  for( i=0; i<data_num; i++ ){

    for( j=0; j<4; j++ ){
      VEC_Fx16Normalize( &p_buff[i].light_vec[j], &p_buff[i].light_vec[j] );
    }
  }

  return p_buff;
}

//----------------------------------------------------------------------------
/**
 *	@brief  時間の色を取得
 *e
 *	@param	p_wk    カラーデータ
 *	@param	num     データ数
 *	@param	time    今の時間
 *	@param	p_color 色格納先
 */
//-----------------------------------------------------------------------------
static void LIGHT_STATUS_GetTimeColor( LIGHT_DATA* p_wk, u32 num, int time, FIELD_LIGHT_STATUS* p_color )
{
  int i;
  int start_time;
  
  GF_ASSERT( num > 0 );

  
  start_time = 0;
  for( i=0; i<num; i++ )
  {
    if( (start_time <= time) && (p_wk[i].endtime > time) )
    {
      // この色を設定
      p_color->light = p_wk[i].light_color[0];
      p_color->diffuse = p_wk[i].diffuse;
      p_color->ambient = p_wk[i].ambient;
      p_color->specular = p_wk[i].specular;
      p_color->emission = p_wk[i].emission;
      return;
    }

    start_time = p_wk[i].endtime;
  }

  // 時間の色がみつからない
  GF_ASSERT(0);
}



