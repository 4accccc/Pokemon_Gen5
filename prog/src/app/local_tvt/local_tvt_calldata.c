//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_calldata.c
 *	@brief  呼び出しデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.01.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "field/calender.h"
#include "field/zonedata.h"
#include "battle/battle_bg_def.h"
#include "gamesystem/game_data.h"

//アーカイブ
#include "arc_def.h"
#include "local_tvt_call.naix"

//外部
#include "local_tvt_calldata.h"

//デバッグ
#include "debug/debug_flg.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
//#define DEBUG_CALL_FLAG_ALL_ON
//#define DEBUG_SELECT_FLAG_ALL_ON
#endif //PM_DEBUG

//バトルBGに追加があったらワーニングを出す
#if BATTLE_BG_TYPE_MAX != (19)
//#warning add battle bg
#endif

//天気に追加があったらワーニングを出す
#if WEATHER_NO_NUM != (15)
#warning add weather
#endif
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	呼び出しデータバイナリ
//=====================================
typedef struct {
  u16 person;
  u16 state;
  u16 place;
  u16 zoneid;
  u16 call;
  u16 greet_msgid;
  u16 talk_msgid;
  u16 bye_msgid;
  u16 sp_msgid;
  u16 dummy;
} LOCAL_TVT_BINARY;

//-------------------------------------
///	呼び出しデータワーク
//=====================================
struct _LOCAL_TVT_CALLDATA
{
  GAMEDATA            *p_gamedata;
  LOCAL_TVT_BINARY    *p_data;
  CALENDER            *p_calender;
  PERSON_STATE        *p_person_state;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static const LOCAL_TVT_BINARY * Local_Tvt_Calldata_GetData( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );

//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し情報データ  初期化
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID                ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CALLDATA * LOCAL_TVT_CALLDATA_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  LOCAL_TVT_CALLDATA  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_CALLDATA ) );
  p_wk->p_gamedata  = p_gamedata;
  p_wk->p_calender  = GAMEDATA_GetCalender( p_gamedata );
  p_wk->p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
  p_wk->p_data      = GFL_ARC_UTIL_Load( ARCID_LOCAL_TVT_CALLDATA, NARC_local_tvt_call_local_tvt_call_dat, FALSE, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し情報データ  破棄
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLDATA_Exit( LOCAL_TVT_CALLDATA *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  背景取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 背景
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_BACK LOCAL_TVT_CALLDATA_GetBack( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  
  switch( cp_data->place )
  {
  case LOCAL_TVT_PLACE_ROOM:
    return LOCAL_TVT_BACK_ROOM;

  case LOCAL_TVT_PLACE_DUNGEON:
    return LOCAL_TVT_BACK_DUNGEON;

  case LOCAL_TVT_PLACE_FIELD:
    return LOCAL_TVT_CALLDATA_GetBackByZoneID( cp_wk, cp_data->zoneid );

  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  天気取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 天気
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_WEATHER LOCAL_TVT_CALLDATA_GetWeather( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  return LOCAL_TVT_CALLDATA_GetWeatherByZoneID( cp_wk, cp_data->zoneid );
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字ID取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 文字ID
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_CALLDATA_GetGreetingMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  return cp_data->greet_msgid;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字ID取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 文字ID
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_CALLDATA_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  return cp_data->talk_msgid;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字ID取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 文字ID
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_CALLDATA_GetFarewellMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  return cp_data->bye_msgid;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字ID取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 文字ID
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_CALLDATA_GetSpecialMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
  return cp_data->sp_msgid;
}
//----------------------------------------------------------------------------
/**
 *	@brief  電話フラグ取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 電話タイプ
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CALLDATA_IsCall( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
#ifdef DEBUG_CALL_FLAG_ALL_ON
  return TRUE;
#endif
  //隠しキャラのみ特別分岐
  if( person == LOCAL_TVT_PERSON_SECRET )
  {
    BOOL is_once  = PERSON_STATE_GetSecretOnceDayCallFlag( cp_wk->p_person_state );
    BOOL is_date  = (PERSON_STATE_GetSecretDateEvent( cp_wk->p_person_state ) == PERSON_STATE_SECRET_DATE_NONE);
    BOOL is_data  = PERSON_STATE_GetLocalTvtCall( cp_wk->p_person_state, LOCAL_TVT_PERSON_SECRET );
    BOOL is_rnd = GAMEDATA_GetLiveCasterSecretCallFlag( cp_wk->p_gamedata );
#ifdef PM_DEBUG
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_LTvtSecretNotRnd ) )
  {
    is_rnd = TRUE;
  }
#endif
    //かけられる条件
    //・デートを約束していない
    //・１日１回だけ
    //・設定データでかけてよい状態になっている
    //・こちらからかけるときは乱数でOKになっている
    if( is_date && !is_once && is_data && is_rnd )
    {
      return TRUE;
    }
    
    return FALSE;    
  }
  else
  {
    return PERSON_STATE_GetLocalTvtCall( cp_wk->p_person_state, person );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択フラグ取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	person  人物 LOCAL_TVT_PERSON_xxxxxx
 *
 *	@return 選択タイプ
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CALLDATA_IsSelect( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  const LOCAL_TVT_BINARY *cp_data = Local_Tvt_Calldata_GetData( cp_wk, person );
#ifdef DEBUG_SELECT_FLAG_ALL_ON
  return TRUE;
#endif
  return cp_data->call != 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンIDから背景を取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	ZONEID zoneID ゾーンID
 *
 *	@return 背景
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_BACK LOCAL_TVT_CALLDATA_GetBackByZoneID( const LOCAL_TVT_CALLDATA *cp_wk, ZONEID zoneID )
{
  switch( ZONEDATA_GetBattleBGType( zoneID ) )
  {
  case BATTLE_BG_TYPE_GRASS:         ///< 草むら
  case BATTLE_BG_TYPE_GRASS_SEASON:  ///< 草むら(四季有り)
  case BATTLE_BG_TYPE_FOREST:        ///< 森
    return LOCAL_TVT_BACK_PLAIN;
  case BATTLE_BG_TYPE_CITY:          ///< 街
  case BATTLE_BG_TYPE_CITY_SEASON:   ///< 街(四季有り)
    return LOCAL_TVT_BACK_CITY;
  case BATTLE_BG_TYPE_CAVE:          ///< 洞窟
  case BATTLE_BG_TYPE_CAVE_DARK:     ///< 洞窟(暗い)
    return LOCAL_TVT_BACK_DUNGEON;
  case BATTLE_BG_TYPE_MOUNTAIN:      ///< 山
    return LOCAL_TVT_BACK_MAUNTAIN;
  case BATTLE_BG_TYPE_SEA:           ///< 海
    return LOCAL_TVT_BACK_SEA;
  case BATTLE_BG_TYPE_ROOM:          ///< 室内
  case BATTLE_BG_TYPE_BIGFOUR_GHOST:   ///< 四天王部屋(ゴースト)
  case BATTLE_BG_TYPE_BIGFOUR_KAKUTOU: ///< 四天王部屋(格闘)
  case BATTLE_BG_TYPE_BIGFOUR_EVIL:    ///< 四天王部屋(悪)
  case BATTLE_BG_TYPE_BIGFOUR_ESPER:   ///< 四天王部屋(エスパー)
  case BATTLE_BG_TYPE_N_CASTLE:        ///< Nの城(謁見の間)
  case BATTLE_BG_TYPE_CHAMPIOPN:       ///< チャンピオン部屋
  case BATTLE_BG_TYPE_RYUURASEN7:      ///< リュウラセンの塔7F
    return LOCAL_TVT_BACK_ROOM;
  case BATTLE_BG_TYPE_SAND:          ///< 砂漠
    return LOCAL_TVT_BACK_DESERT;
  default:
    GF_ASSERT(0);
    return LOCAL_TVT_BACK_PLAIN;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンIDから天気取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *	@param	zoneID  ゾーンID
 *
 *	@return 天気
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_WEATHER LOCAL_TVT_CALLDATA_GetWeatherByZoneID( const LOCAL_TVT_CALLDATA *cp_wk, ZONEID zoneID )
{
  WEATHER_NO  weather = CALENDER_GetWeather_today( cp_wk->p_calender, zoneID );

  switch( weather )
  {
  case WEATHER_NO_SUNNY:
  case WEATHER_NO_MIST:
  case WEATHER_NO_PALACE_WHITE_MIST:
  case WEATHER_NO_PALACE_BLACK_MIST:
  case WEATHER_NO_STORM:
  case WEATHER_NO_STORMHIGH:
  case WEATHER_NO_PALACE_WHITE_MIST_HIGH:
  case WEATHER_NO_PALACE_BLACK_MIST_HIGH:
    return LOCAL_TVT_WEATHER_SUNNY;
  case WEATHER_NO_SNOW:
  case WEATHER_NO_ARARE:
  case WEATHER_NO_DIAMONDDUST:
    return LOCAL_TVT_WEATHER_SNOW;
  case WEATHER_NO_RAIN: 
  case WEATHER_NO_RAIKAMI:
  case WEATHER_NO_KAZAKAMI:
    return LOCAL_TVT_WEATHER_RAIN;
  case WEATHER_NO_SNOWSTORM:
    return LOCAL_TVT_WEATHER_SAND;
  default:
    GF_ASSERT(0);
    return LOCAL_TVT_WEATHER_SUNNY;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの顔を隠しているかチェック
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk   ワーク
 *
 *	@return TRUE  顔を隠す  FALSE顔を隠さない
 */
//-----------------------------------------------------------------------------
u16 LOCAL_TVT_CALLDATA_GetState( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  return PERSON_STATE_GetState( cp_wk->p_person_state, person );
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの性別を取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *
 *	@return 性別を取得
 */
//-----------------------------------------------------------------------------
u32 LOCAL_TVT_CALLDATA_GetSecretSex( const LOCAL_TVT_CALLDATA *cp_wk )
{
  u32 sex = MyStatus_GetMySex( GAMEDATA_GetMyStatus(cp_wk->p_gamedata) );

  //主人公の反対の性別
  if( sex == PTL_SEX_MALE )
  {
    return PTL_SEX_FEMALE;
  }
  else
  {
    return PTL_SEX_MALE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャラの状態を加算
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk  ワーク
 *	@param	person  キャラ
 *	@param  max     最大値
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLDATA_AddState( LOCAL_TVT_CALLDATA *p_wk, u16 person, u16 max )
{
  u16 state = PERSON_STATE_GetState( p_wk->p_person_state, person );
  if( state < max )
  {
    state++;
    PERSON_STATE_SetState( p_wk->p_person_state, person, state );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの不機嫌フラグを取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk ワーク
 *
 *	@return TRUEで不機嫌  FALSEで普通
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CALLDATA_GetSecretDispleased( const LOCAL_TVT_CALLDATA *cp_wk )
{
  return PERSON_STATE_GetSecretDispleased( cp_wk->p_person_state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  隠しキャラの不機嫌フラグ設定
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk  ワーク
 *	@param	enable TRUEで不機嫌  FALSEで普通
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLDATA_SetSecretDispleased( LOCAL_TVT_CALLDATA *p_wk, BOOL enable )
{
  PERSON_STATE_SetSecretDispleased( p_wk->p_person_state, enable );
}
//----------------------------------------------------------------------------
/**
 *	@brief  約束設定
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk  ワーク
 *	@param	type  約束タイプ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLDATA_SetSecretDateEvent( LOCAL_TVT_CALLDATA *p_wk, PERSON_STATE_SECRET_DATE type )
{
  RTCDate date;
  GFL_RTC_GetDate( &date );
  PERSON_STATE_SetSecretDateEvent( p_wk->p_person_state, type, &date );

}

//----------------------------------------------------------------------------
/**
 *	@brief  １季節１回デートフラグを取得
 *
 *	@param	const LOCAL_TVT_CALLDATA *cp_wk   ワーク
 *
 *	@return TRUE定期イベントが可能  FALSE不可能
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CALLDATA_GetSecretSeasonDate( const LOCAL_TVT_CALLDATA *cp_wk )
{
  return PERSON_STATE_GetSeasonDateFlag( cp_wk->p_person_state );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物と状態に対応したデータを取得
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk  ワーク
 *	@param	person                    人物
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static const LOCAL_TVT_BINARY * Local_Tvt_Calldata_GetData( const LOCAL_TVT_CALLDATA *cp_wk, u16 person )
{
  int i;

  LOCAL_TVT_BINARY  *p_data = NULL;
  u16 state = PERSON_STATE_GetState( cp_wk->p_person_state, person );

  for( i = 0; i < LOCAL_TVT_CALLDATA_MAX; i++ )
  {
    p_data  = &cp_wk->p_data[i];
    if( p_data->person == person
        && p_data->state == state )
    {
      return p_data;
    }
  }
  GF_ASSERT( 0 );
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  １日１回電話フラグON
 *
 *	@param	LOCAL_TVT_CALLDATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLDATA_SetSecretOnceCall( LOCAL_TVT_CALLDATA *p_wk )
{
  PERSON_STATE_SetSecretOnceDayCallFlag( p_wk->p_person_state, TRUE );
}

