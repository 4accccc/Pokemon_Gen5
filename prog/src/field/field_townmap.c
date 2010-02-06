//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_townmap.h
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
#include "field_townmap.h"
#include "field/zonedata.h"

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
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ
 *
 *	@return 元になるZONEID
 */
//-----------------------------------------------------------------------------
u16 FIELD_TOWNMAP_GetRootZoneID( const GAMEDATA *cp_gamedata )
{
  u16 zoneID;
  PLAYER_WORK *p_player;
#ifdef DEBUG_ONLY_FOR_toru_nagihashi
#warning( TODO:add zoneID check!! )
#endif //DEBUG_ONLY_FOR_toru_nagihashi
  
  p_player   = GAMEDATA_GetMyPlayerWork( (GAMEDATA*)cp_gamedata );
  zoneID  = PLAYERWORK_getZoneID(p_player);

  if( ZONEDATA_IsFieldMatrixID( zoneID ) )
  { 
    //フィールドにいる状態
    return zoneID;
  }
  else
  { 
    //フィールド以外にいる状態
    return GAMEDATA_GetEscapeLocation( cp_gamedata )->zone_id;
  }
}
