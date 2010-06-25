//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		pm_weather.c
 *	@brief  天気管理　移動ポケモン　ゾーン　カレンダー
 *	@author	tomoya takahashi
 *	@date		2010.01.29
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "system/ds_system.h"

#include "gamesystem/pm_weather.h"
#include "gamesystem/pm_season.h"

#include "field/move_pokemon.h"
#include "field/calender.h"

#include "field/palace_weather.cdat"
#include "field/zonedata.h"



#include "field/eventwork.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"


#include "field/field_comm/intrude_work.h"

#include "field/areadata.h"
#include "system/main.h"

#ifdef PM_DEBUG

#include "debug/debug_flg.h"

#endif // PM_DEBUG

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
static BOOL CheckGameDataHaveRTCSeasonID( const GAMEDATA* p_gamedata );
static u16 PM_WEATHER_GetPalaceWeather( const GAMESYS_WORK* cp_gamesystem, const GAMEDATA* cp_data, int zone_id );
static u16 PM_WEATHER_GetEventWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id );
static u16 PM_WEATHER_GetBirthDayWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id );
static inline BOOL PM_WEATHER_IsMovePokeWeather( u32 weather_no );

//----------------------------------------------------------------------------
/**
 *	@brief  現在の指定ZONEの天気を取得する
 *
 *	@param	p_gamesystem データ
 *	@param	zone_id ゾーンID
 *
 *	@return ゾーンの天気
 */
//-----------------------------------------------------------------------------
u8 PM_WEATHER_GetZoneWeatherNo( GAMESYS_WORK* p_gamesystem, int zone_id )
{
#if 0
  GAMEDATA* p_data  = GAMESYSTEM_GetGameData( p_gamesystem );
  CALENDER* calender = GAMEDATA_GetCalender( p_data );
  u16       weather  = MP_CheckMovePokeWeather( p_data, zone_id );

  // 移動ポケモンチェック
  if( weather != WEATHER_NO_NONE )
  {
    return weather;
  }

  // パレスチェック
  weather = PM_WEATHER_GetPalaceWeather( p_gamesystem, p_data, zone_id );
  if( weather != WEATHER_NO_NONE )
  {
    return weather;
  }

  // イベントによる、天気書き換え
  weather = PM_WEATHER_GetEventWeather( p_gamesystem, p_data, zone_id );
  if( weather != WEATHER_NO_NONE )
  {
    return weather;
  }

  // 誕生日による天気書き換え
  weather = PM_WEATHER_GetBirthDayWeather( p_gamesystem, p_data, zone_id );
  if( weather != WEATHER_NO_NONE )
  {
    return weather;
  }

  if( CheckGameDataHaveRTCSeasonID( p_data ) ) {
    // カレンダーを参照して天気を取得する.
    // (カレンダーに登録されていないゾーンの天気は, ゾーンテーブルに従う.)
    return CALENDER_GetWeather_today( calender, zone_id );
  }
  else {
    // ゲームデータの季節とRTCの季節に整合性がない場合, ゾーンテーブルに従う
    return ZONEDATA_GetWeatherID( zone_id );
  } 
#endif

  // 体験版では, 季節ごとに天気を固定する
  {
    GAMEDATA* p_gamedata = GAMESYSTEM_GetGameData( p_gamesystem );
    u8 season_id  = GAMEDATA_GetSeasonID( p_gamedata );
    u8 weather_no;
    BOOL indoor;

    // 屋内 or 屋外?
    {
      u16       area_id   = ZONEDATA_GetAreaID( zone_id );
      AREADATA* area_data = AREADATA_Create( HEAPID_PROC, area_id, season_id ); 
      indoor = ( AREADATA_GetInnerOuterSwitch(area_data) == 0 );
      AREADATA_Delete( area_data );
    }

    if( indoor ) {
      weather_no = WEATHER_NO_SUNNY; // 屋内なら晴れ
    }
    else { 
      switch( season_id ) {
      default: GF_ASSERT(0);
      case PMSEASON_SPRING: weather_no = WEATHER_NO_SUNNY; break;
      case PMSEASON_SUMMER: weather_no = WEATHER_NO_RAIN; break;
      case PMSEASON_AUTUMN: weather_no = WEATHER_NO_SUNNY; break;
      case PMSEASON_WINTER: weather_no = WEATHER_NO_SNOW; break;
      }
    } 

    return weather_no;
  }
}

//----------------------------------------------------------------------------
/**
 * @brief ゲーム内季節とRTCから算出した季節が等しいかどうか？
 *
 * @param p_gamesystem
 */
//----------------------------------------------------------------------------
static BOOL CheckGameDataHaveRTCSeasonID( const GAMEDATA* p_gamedata )
{
  u8 gamedata_season = GAMEDATA_GetSeasonID( p_gamedata );
  u8 rtc_season = PMSEASON_GetRealTimeSeasonID();

  if( gamedata_season == rtc_season ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンチェンジでの天気アップデート
 *
 *	@param	p_gamesystem ゲームデータ
 */
//-----------------------------------------------------------------------------
void PM_WEATHER_UpdateZoneChangeWeatherNo( GAMESYS_WORK* p_gamesystem, int zone_id )
{
  GAMEDATA* p_data  = GAMESYSTEM_GetGameData( p_gamesystem );
  u16       weather  = PM_WEATHER_GetZoneWeatherNo( p_gamesystem, zone_id );
  GAMEDATA_SetWeatherNo( p_data, weather );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブ復帰時の天気アップデート
 *
 *	@param	p_data 
 */
//-----------------------------------------------------------------------------
void PM_WEATHER_UpdateSaveLoadWeatherNo( GAMEDATA* p_data, int zone_id )
{
  u16 weather  = MP_CheckMovePokeWeather( p_data, zone_id );
  u16 last_weather = GAMEDATA_GetWeatherNo( p_data );

  // 移動ポケモン優先
  if( weather != WEATHER_NO_NONE )
  {
    GAMEDATA_SetWeatherNo( p_data, weather );
  }else{

    // 移動ポケモンの動作後で、ライカミカザカミ天気の場合には、ZoneTableの天気にする。
    // 移動ポケモンの動作前の場合には、イベントでライカミカザカミにしていることがあるので
    // クリアすることはしない。
    //
    // 移動ポケモン　動作後にライカミ、カザカミ天気にしている場合が発生すると、
    // このクリア処理が悪さをする。　注意！！
    if( PM_WEATHER_IsMovePokeWeather( last_weather ) ){
      if( MP_CheckMovePokeValid( p_data ) == TRUE ){

        // 天気クリア   デフォルトの天気を使用する。
        GAMEDATA_SetWeatherNo( p_data, ZONEDATA_GetWeatherID( zone_id ) );
      }
    }
  }
}





//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  パレスの天気を取得
 *
 *	@param	p_data      データ
 *	@param	zone_id     ゾーンID
 *
 *	@return 天気
 */
//-----------------------------------------------------------------------------
static u16 PM_WEATHER_GetPalaceWeather( const GAMESYS_WORK* cp_gamesystem, const GAMEDATA* cp_data, int zone_id )
{
  EVENTWORK * p_evwork = GAMEDATA_GetEventWork( (GAMEDATA*)cp_data );
  GAME_COMM_SYS_PTR p_gamecomm = GAMESYSTEM_GetGameCommSysPtr( (GAMESYS_WORK*)cp_gamesystem );
  int i;
  
  // パレスなら、パレス用
  if( ZONEDATA_IsPalaceField( zone_id ) )
  {
    if( p_gamecomm ){
      for( i=0; i<NELEMS(sc_PALACE_WEATHER_DATA); i++ )
      {
        if( sc_PALACE_WEATHER_DATA[i].zone_id == zone_id )
        {
          if( EVENTWORK_CheckEventFlag( p_evwork, sc_PALACE_WEATHER_DATA[i].sys_flag ) == FALSE )
          {
            if( Intrude_GetRomVersion( p_gamecomm ) == VERSION_WHITE ){
              return WEATHER_NO_PALACE_WHITE_MIST_HIGH;
            }else{
              return WEATHER_NO_PALACE_BLACK_MIST_HIGH;
            }
          }
          else
          {
            if( Intrude_GetRomVersion( p_gamecomm ) == VERSION_WHITE ){
              return WEATHER_NO_PALACE_WHITE_MIST;
            }else{
              return WEATHER_NO_PALACE_BLACK_MIST;
            }
          }
        }
      }
    }
  }

#ifdef PM_DEBUG
  //パレス侵入テクスチャON
  if( DEBUG_FLG_GetFlg(DEBUG_FLG_PalaceTexture) == TRUE )
  {
    //パレス到着ON
    if( DEBUG_FLG_GetFlg(DEBUG_FLG_PalaceArrive) == TRUE )
    {
      if( GET_VERSION() == VERSION_WHITE ){
        return WEATHER_NO_PALACE_WHITE_MIST;
      }else{
        return WEATHER_NO_PALACE_BLACK_MIST;
      }
    }
    else
    {
      if( GET_VERSION() == VERSION_WHITE ){
        return WEATHER_NO_PALACE_WHITE_MIST_HIGH;
      }else{
        return WEATHER_NO_PALACE_BLACK_MIST_HIGH;
      }
    }
  }
#endif // PM_DEBUG

  return WEATHER_NO_NONE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  イベントの天気を取得
 *
 *	@param	p_gamesystem    ゲームシステム
 *	@param	p_data          データ
 *	@param	zone_id         ゾーンID
 *
 *	@return イベント起動：天気
 */
//-----------------------------------------------------------------------------
static u16 PM_WEATHER_GetEventWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id )
{
	EVENTWORK * p_ev;
	p_ev = GAMEDATA_GetEventWork( p_data );

  // R07  移動ポケモン
  if( zone_id == ZONE_ID_R07 )
  {
    if( EVENTWORK_CheckEventFlag( p_ev, SYS_FLAG_WEATHER_R07 ) )
    {
      // BLACK = カザカミ
      // WHITE = ライカミ
      if( GetVersion() == VERSION_BLACK ){
        return WEATHER_NO_KAZAKAMI;
      }
      return WEATHER_NO_RAIKAMI;
    }
  }

  return WEATHER_NO_NONE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  誕生日の天気を取得
 *
 *	@param	p_gamesystem  ゲームシステム
 *	@param	p_data        ゲームデータ
 *	@param	zone_id       ゾーンID
 *
 *	@return 誕生日：天気
 */
//-----------------------------------------------------------------------------
static u16 PM_WEATHER_GetBirthDayWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id )
{
  RTCDate data;
  u8 month;
  u8 day;
  
  if( ZONEDATA_IsBirthDayWeatherChange( zone_id ) ){
    // 今日の日にちを取得
    GFL_RTC_GetDate( &data );

    // 誕生日を取得
    DS_SYSTEM_GetBirthDay( &month, &day );

    // 日にちと誕生日が一致したら、晴れにする
    if( (data.month == month) && (data.day == day) )
    {
      return WEATHER_NO_SUNNY;
    }
  }
  return WEATHER_NO_NONE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  移動ポケモン天気かチェックする
 *
 *	@param	weather_no  天気ナンバー
 *
 *	@retval TRUE    移動ポケモン天気
 *	@retval FALSE   通常天気
 */
//-----------------------------------------------------------------------------
static inline BOOL PM_WEATHER_IsMovePokeWeather( u32 weather_no )
{
  if( (weather_no == WEATHER_NO_RAIKAMI) || (weather_no == WEATHER_NO_KAZAKAMI) ){
    return TRUE;
  }
  return FALSE;
}

