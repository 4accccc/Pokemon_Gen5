//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		townmap_util.h
 *	@brief  タウンマップ処理共通ルーチン
 *	@author	Toru=Nagihashi
 *	@date		2010.02.06
 *	@note   タウンマップで行っている主人公の位置サーチなどを
 *	        他の場所でも使うので外部化
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>
#include "app/townmap_util.h"
#include "field/zonedata.h"
#include "src/field/evt_lock.h"
#include "app/townmap_data.h"
#include "savedata/wifihistory.h"
#include "fieldmap/zone_id.h"
#include "app/townmap_data_sys.h"
#include "field/animal_trail.h"
#include "field/animal_trail_data.h"
#include "savedata/key_system_save.h"

#include "../resource/fldmapdata/flagwork/work_define.h"
#include "field/script_def.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  タウンマップで使用するため、そのゾーンIDがフィールド上の
 *	        どこのゾーンIDなのか、どこからきたゾーンIDなのか返す。
 *	        （例えば、ZONEID_C01GYM0101はC01のジムなので
 *	        ZONEID_C01が返ってきます）
 *	        -> タウンマップでもゾーングループを使うようになったので
 *	        ゾーングループが返ってきます。100407
 *	        ユニオン、アジト船、けもの道については個別の分岐があります。110902
 *	        白の樹洞、黒の摩天楼も個別の分岐があります。120124
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ  (エスケープロケーション取得用)
 *	@param  now_zoneID
 *
 *	@return 元になるZONEID
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_UTIL_GetRootZoneID( const GAMEDATA* cp_gamedata, u16 now_zoneID )
{
  ZONEID zoneID = ZONEDATA_GetGroupID( now_zoneID );

  if( zoneID == ZONE_ID_UNION )
  {
    return ZONEDATA_GetGroupID( GAMEDATA_GetSpecialLocation( cp_gamedata )->zone_id );
  }
  else if( zoneID == ZONE_ID_D27R0101 )
  {
    //アジト船だったら
    EVENTWORK *p_ev = GAMEDATA_GetEventWork((GAMEDATA *)cp_gamedata);
    u16 *p_value = EVENTWORK_GetEventWorkAdrs( p_ev, WK_SYS_HIDE_SHIP_PLACE );
    switch( *p_value )
    {
    default:
      GF_ASSERT(0);
    case SCR_HIDE_SHIP_PLACE_C05:
      return ZONE_ID_D05;
    case SCR_HIDE_SHIP_PLACE_W21:
      return ZONE_ID_W21;
    case SCR_HIDE_SHIP_PLACE_D10:
      return ZONE_ID_D10;
    case SCR_HIDE_SHIP_PLACE_W17:
      return ZONE_ID_D27R0101;
    }
  }
  else if( zoneID == ANIMAL_TRAIL_ZONE_ID )
  {
    ETC_SAVE_WORK *p_etc = SaveData_GetEtc( GAMEDATA_GetSaveControlWork((GAMEDATA *)cp_gamedata) );
    u8 trailID = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( p_etc );
    zoneID = ANIMAL_TRAIL_DATA_GetExitData( trailID, ANIMAL_TRAIL_DATA_PARAM_ZONEID );

    return ZONEDATA_GetGroupID( zoneID );
  }
  else if( zoneID == ZONE_ID_D31R0101 ) //ZONEDATA_IsInfinityDungeon関数に無限ダンジョンのゾーングループが入ってないので検知できない
  {
    KEY_SYSTEM_SAVE *p_key = SAVE_CONTROL_GetKeySystem( GAMEDATA_GetSaveControlWork((GAMEDATA *)cp_gamedata) );
    KEY_SYSTEM_WFBC wfbc_type = KEY_SYSTEM_SAVE_GetWfbc( p_key );

    #if PM_VERSION == VERSION_B2
      if( wfbc_type == KEY_SYSTEM_WFBC_SELF )
      {
        return ZONE_ID_BC10;
      }
      else
      {
        return ZONE_ID_WC10;
      }
    #else   // PM_VERSION == VERSION_B2
      if( wfbc_type == KEY_SYSTEM_WFBC_SELF )
      {
        return ZONE_ID_WC10;
      }
      else
      {
        return ZONE_ID_BC10;
      }
    #endif  // PM_VERSION == VERSION_B2
  }
  else
  {
    u32 ret  =  TOWNMAP_REPLACE_DATA_GetReplace( zoneID );
    if( ret != TOWNMAP_DATA_ERROR )
    { 
      zoneID  = ret;
    }

    return zoneID;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タウンマップデータのフラグをチェック
 *
 *	@param	const GAMEDATA* cp_gamedata
 *	@param	flag   TOWNMAP_DATA_PARAM_ARRIVE_FLAGやTOWNMAP_DATA_PARAM_HIDE_FLAGで取得した値
 *
 *	@return TRUEでフラグがたっている  FALSEでフラグが立っていない
 */
//-----------------------------------------------------------------------------
BOOL TOWNMAP_UTIL_CheckFlag( GAMEDATA* p_gamedata, u16 flag )
{ 
  EVENTWORK	* p_ev		    = GAMEDATA_GetEventWork( p_gamedata );
  MISC      * p_misc      = GAMEDATA_GetMiscWork( p_gamedata );
  MYSTATUS  * p_mystatus  = GAMEDATA_GetMyStatus( p_gamedata );
  WIFI_HISTORY *p_wifi    = SaveData_GetWifiHistory( GAMEDATA_GetSaveControlWork(p_gamedata) );

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  { 
    return TRUE;
  }
#endif //PM_DEBUG

  switch( flag )
  { 
  case TOWNMAP_USER_FLAG_LIBERTY_GARDEN_TICKET:
    //ビクティイベントフラグ　リバティガーデン島へのチケットを持っているか？
    return EVTLOCK_CheckEvtLock( p_misc, EVT_LOCK_NO_VICTYTICKET, p_mystatus );

  case TOWNMAP_USER_FLAG_OVERSEAS_TRADE:
    //国連へは世界交換したら地図上に発見
    return ( 1 <= WIFIHISTORY_GetMyCountryCountEx( p_wifi, p_mystatus, FALSE ) );

  case TOWNMAP_USER_FLAG_LAST_PLASMA:
    //プラズマフリゲートは最終段階に入っているか
    return ( SCR_HIDE_SHIP_PLACE_W17 == *EVENTWORK_GetEventWorkAdrs( p_ev, WK_SYS_HIDE_SHIP_PLACE ));

  default:
    //さもなくばイベントフラグ
    return EVENTWORK_CheckEventFlag( p_ev, flag );
  }
}
