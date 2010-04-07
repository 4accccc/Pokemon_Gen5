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
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ  (エスケープロケーション取得用)
 *	@param  now_zoneID
 *
 *	@return 元になるZONEID
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_UTIL_GetRootZoneID( const GAMEDATA* cp_gamedata, u16 now_zoneID )
{
  PLAYER_WORK *p_player;

  return ZONEDATA_GetGroupID( now_zoneID );

#if 0 //old
  if( ZONEDATA_IsFieldMatrixID( now_zoneID ) )
  { 
    //フィールドにいる状態
    return now_zoneID;
  }
  else
  { 
    //フィールド以外にいる状態
    return GAMEDATA_GetEscapeLocation( cp_gamedata )->zone_id;
  }
#endif
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

  //まずはユーザーフラグかチェック
  if( flag == TOWNMAP_USER_FLAG_LIBERTY_GARDEN_TICKET )
  { 
    //ビクティイベントフラグ　リバティガーデン島へのチケットを持っているか？
    return EVTLOCK_CheckEvtLock( p_misc, EVT_LOCK_NO_VICTYTICKET, p_mystatus );
  }

  //さもなくばイベントフラグ
  return EVENTWORK_CheckEventFlag( p_ev, flag );
}
