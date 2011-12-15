//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_sys.c
 *	@brief  リゾートシステム
 *	@author	Toru=Nagihashi
 *	@date		2011.07.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "gamesystem/game_data.h"

#include "resort_sys.h"
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
//-------------------------------------
///	リゾートシステム
//=====================================
struct _RESORT_SYSTEM
{
  RESORT_ALL_DATA_PARAM  param;
  RESORT_BINARY_SHOP    *p_binary_shop;
  RESORT_BINARY_SPOT    *p_binary_spot;
  RESORT_BINARY_CITY    *p_binary_city;
  RESORT_BINARY_INSTITUTION *p_binary_insti;
  RESORT_BINARY_PRESENT *p_binary_present;
  RESORT_BINARY_PRESET  *p_binary_preset;
  RESORT_BINARY_NPC     *p_binary_npc;

  RESORT_ABSTRUCT_MANAGER *p_abstruct;

  RESORT_DATA_MANAGER *p_manager;

  RESORT_WORK *p_work;
};

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================

//=============================================================================
/**
 *					データ
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  システム初期化
 *
 *	@param	const RESORT_SYSTEM_INIT_PARAM *cp_param  初期化構造体
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
RESORT_SYSTEM * RESORT_SYSTEM_Init( const RESORT_ALL_DATA_PARAM *cp_param, RESORT_WORK *p_work, HEAPID heapID )
{
  RESORT_SYSTEM *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_SYSTEM) );
  p_wk->param = *cp_param;
  p_wk->p_work  = p_work;

  //バイナリ情報
  p_wk->p_binary_shop = RESORT_BINARY_SHOP_Init( heapID );
  p_wk->p_binary_spot = RESORT_BINARY_SPOT_Init( heapID );
  p_wk->p_binary_city = RESORT_BINARY_CITY_Init( heapID );
  p_wk->p_binary_insti = RESORT_BINARY_INSTITUTION_Init( heapID );
  p_wk->p_binary_present  = RESORT_BINARY_PRESENT_Init( heapID );
  p_wk->p_binary_preset   = RESORT_BINARY_PRESET_Init( heapID );
  p_wk->p_binary_npc        = RESORT_BINARY_NPC_Init( heapID );

  //データ作成
  p_wk->p_abstruct = RESORT_ABSTRUCT_MANAGER_Init( cp_param, heapID );

  //データマネージャー（なくすかも）
  p_wk->p_manager = RESORT_DATA_MANAGER_Init( p_wk->p_abstruct, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  システム破棄
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_SYSTEM_Exit( RESORT_SYSTEM *p_wk )
{
  //データマネージャー破棄
  RESORT_DATA_MANAGER_Exit( p_wk->p_manager );

  //データ破棄
  RESORT_ABSTRUCT_MANAGER_Exit( p_wk->p_abstruct );

  //バイナリ破棄
  RESORT_BINARY_NPC_Exit( p_wk->p_binary_npc );
  RESORT_BINARY_PRESET_Exit( p_wk->p_binary_preset );
  RESORT_BINARY_PRESENT_Exit( p_wk->p_binary_present );
  RESORT_BINARY_INSTITUTION_Exit( p_wk->p_binary_insti );
  RESORT_BINARY_CITY_Exit( p_wk->p_binary_city );
  RESORT_BINARY_SPOT_Exit( p_wk->p_binary_spot );
  RESORT_BINARY_SHOP_Exit( p_wk->p_binary_shop );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  お店データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return お店データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SHOP * RESORT_SYSTEM_GetBinaryShop( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_shop;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットデータ取得
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return スポットデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SPOT * RESORT_SYSTEM_GetBinarySpot( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_spot;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return 街データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_CITY * RESORT_SYSTEM_GetBinaryCity( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_city;
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return 施設データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_INSTITUTION * RESORT_SYSTEM_GetBinaryInstitution( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_insti;
}
//----------------------------------------------------------------------------
/**
 *	@brief  おみやげデータ
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return おみやげデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_PRESENT * RESORT_SYSTEM_GetBinaryPresent( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_present;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ROM内人物プリセットデータ
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return ROM内人物プリセットデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_PRESET * RESORT_SYSTEM_GetBinaryPreset( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_preset;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ROM内NPC情報データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return ROM内NPC情報データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_NPC * RESORT_SYSTEM_GetBinaryNpc( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_binary_npc;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ抽象マネージャー取得
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return データ抽象マネージャー
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_MANAGER * RESORT_SYSTEM_GetAbstructManager( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_abstruct;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データマネージャー
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return データマネージャー
 */
//-----------------------------------------------------------------------------
RESORT_DATA_MANAGER * RESORT_SYSTEM_GetDataManager( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_manager;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return 人物データ
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_ALL_DATA * RESORT_SYSTEM_GetPeopleAllData( RESORT_SYSTEM *p_wk )
{
  return p_wk->param.p_all_people;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return 受信バッファ
 */
//-----------------------------------------------------------------------------
RESORT_RECV_BUFF * RESORT_SYSTEM_GetRecvBuff( RESORT_SYSTEM *p_wk )
{
  return p_wk->param.p_recv_sv_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief  一時受信バッファ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return 一時受信バッファ
 */
//-----------------------------------------------------------------------------
RESORT_RECV_BUFF * RESORT_SYSTEN_GetRecvTmpBuff( RESORT_SYSTEM *p_wk )
{
  return p_wk->param.p_recv_tmp_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief  一時代理バッファ取得
 *
 *	@param	RESORT_SYSTEM *p_wk ワーク
 *
 *	@return 代理バッファ
 */
//-----------------------------------------------------------------------------
RESORT_PROXY_BUFF * RESORT_SYSTEM_GetProxyBuff( RESORT_SYSTEM *p_wk )
{
  return p_wk->param.p_proxy_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ取得
 *
 *	@param	RESORT_SYSTEM *p_wk   ワーク
 *
 *	@return 街データ
 */
//-----------------------------------------------------------------------------
RESORT_CITY_DATA * RESORT_SYSTEM_GetCityData( RESORT_SYSTEM *p_wk )
{
  return p_wk->param.p_city_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワーク
 *
 *	@param	RESORT_SYSTEM *p_wk システム
 *
 *	@return 常駐ワーク
 */
//-----------------------------------------------------------------------------
RESORT_WORK * RESORT_SYSTEM_GetWork( RESORT_SYSTEM *p_wk )
{
  return p_wk->p_work;
}
