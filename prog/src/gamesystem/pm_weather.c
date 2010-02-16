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

#include "gamesystem/pm_weather.h"

#include "field/move_pokemon.h"
#include "field/calender.h"

#include "field/palace_weather.cdat"
#include "field/zonedata.h"

#include "field/field_comm/intrude_work.h"

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
static u16 PM_WEATHER_GetPalaceWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id );

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


  // カレンダーを参照して天気を取得する.
  // (カレンダーに登録されていないゾーンの天気は, ゾーンテーブルに従う.)
	return CALENDER_GetWeather_today( calender, zone_id );
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

  // 移動ポケモン優先
  if( weather != WEATHER_NO_NONE )
  {
    GAMEDATA_SetWeatherNo( p_data, weather );
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
static u16 PM_WEATHER_GetPalaceWeather( GAMESYS_WORK* p_gamesystem, GAMEDATA* p_data, int zone_id )
{
  EVENTWORK * p_evwork = GAMEDATA_GetEventWork( p_data );
  GAME_COMM_SYS_PTR p_gamecomm = GAMESYSTEM_GetGameCommSysPtr( p_gamesystem );
  int i;
  
  // パレスなら、パレス用
  if( ZONEDATA_IsPalaceField( zone_id ) )
  {
     for( i=0; i<NELEMS(sc_PALACE_WEATHER_DATA); i++ )
     {
       if( sc_PALACE_WEATHER_DATA[i].zone_id == zone_id )
       {
         if( EVENTWORK_CheckEventFlag( p_evwork, sc_PALACE_WEATHER_DATA[i].sys_flag ) == FALSE )
         {
           if( p_gamecomm ){
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

  return WEATHER_NO_NONE;
}



