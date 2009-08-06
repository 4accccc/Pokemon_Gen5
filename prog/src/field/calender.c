//////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  カレンダー情報取得システム
 * @author obata
 * @date   2009.0805 
 *
 */
////////////////////////////////////////////////////////////////////////////// 
#include "calender.h" 


//=============================================================================
/**
 * @brief カレンダー・システム・ワーク
 */
//=============================================================================
struct _CALENDER
{
  CALENDER_ENCOUNT_INFO* pEncountInfo;  // 出現率番号データ
  CALENDER_HATCH_INFO*   pHatchInfo;    // 孵化変化番号データ
  CALENDER_WEATHER_INFO* pWeatherInfo;  // 天気データ
};


//=============================================================================
/**
 * @brief システムの作成・破棄
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief カレンダーシステムを作成する
 *
 * @param heap_id 使用するヒープID
 *
 * @return 作成したカレンダー
 */
//-----------------------------------------------------------------------------
CALENDER* CALENDER_Create( HEAPID heap_id )
{
  CALENDER* p_sys;

  // システムワーク作成
  p_sys = GFL_HEAP_AllocMemory( heap_id, sizeof( CALENDER ) );

  // 各種データを読み込む
  p_sys->pEncountInfo = CALENDER_ENCOUNT_INFO_Create( heap_id );
  p_sys->pHatchInfo   = CALENDER_HATCH_INFO_Create( heap_id );
  p_sys->pWeatherInfo = CALENDER_WEATHER_INFO_Create( heap_id );

  // 作成したカレンダーを返す
  return p_sys;
}

//-----------------------------------------------------------------------------
/**
 * @brief カレンダーシステムを破棄する
 *
 * @param p_sys 破棄するカレンダー
 */
//-----------------------------------------------------------------------------
void CALENDER_Delete( CALENDER* p_sys )
{
  // 各種データを破棄
  CALENDER_ENCOUNT_INFO_Delete( p_sys->pEncountInfo );
  CALENDER_HATCH_INFO_Delete( p_sys->pHatchInfo );
  CALENDER_WEATHER_INFO_Delete( p_sys->pWeatherInfo );

  // 本体を破棄
  GFL_HEAP_FreeMemory( p_sys );
}


//=============================================================================
/**
 * @brief カレンダーへの問い合わせ
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief 指定日における, 指定ゾーンの天気を取得する
 *
 * @param p_sys   カレンダー
 * @param p_date  日付指定
 * @param zone_id ゾーン指定
 *
 * @return 天気番号( WEATHER_NO_XXXX )
 *         指定ゾーンが登録されていない場合, 無効値 WEATHER_NO_NONE が返る
 */
//----------------------------------------------------------------------------- 
u16 CALENDER_GetWeather( CALENDER* p_sys, const RTCDate* p_date, u16 zone_id )
{
  return CALENDER_WEATHER_INFO_GetWeather( p_sys->pWeatherInfo, p_date, zone_id );
}

//-----------------------------------------------------------------------------
/**
 * @breif 指定日の出現率変化番号を取得する
 *
 * @param p_sys  カレンダー
 * @param p_date 日付指定
 *
 * @return 出現率変化番号( ENCOUNT_NO_x )
 */
//-----------------------------------------------------------------------------
u8 CALENDER_GetEncountNo( CALENDER* p_sys, const RTCDate* p_date )
{
  return CALENDER_ENCOUNT_INFO_GetEncountNo( p_sys->pEncountInfo, p_date );
}

//-----------------------------------------------------------------------------
/**
 * @brief 指定日の孵化変化番号を取得する
 *
 * @param p_sys  カレンダー
 * @param p_date 日付指定
 *
 * @return 孵化変化番号( HATCH_NO_x )
 */
//-----------------------------------------------------------------------------
u8 CALENDER_GetHatchNo( CALENDER* p_sys, const RTCDate* p_date )
{
  return CALENDER_HATCH_INFO_GetHatchNo( p_sys->pHatchInfo, p_date );
}
