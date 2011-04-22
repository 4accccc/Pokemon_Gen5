//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_binary.c
 *	@brief  レゾナンスリゾートバイナリデータ読み込み
 *	@author	Toru=Nagihashi
 *	@data		2011.02.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>

//resource
#include "arc_def.h"

//outer
#include "resort_binary.h"


#ifdef PM_DEBUG
#define DEBUG_DATA_PRINT
#endif //PM_DEBUG

//=============================================================================
/**
 *  バイナリ操作
 */
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY
{
  u32 param_max;
  u32 size;
  u16 *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ読み込み
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY * RESORT_BINARY_Init( ARCDATID datID, u32 param_max, HEAPID heapID )
{
  RESORT_BINARY *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, datID, FALSE, heapID, &p_wk->size );
  p_wk->param_max = param_max;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ破棄
 *
 *	@param	RESORT_BINARY *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_Exit( RESORT_BINARY *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	RESORT_BINARY *p_wk ワーク
 *	@param	index   インデックス
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_GetData( const RESORT_BINARY *cp_wk, u32 index, u32 param )
{
  u32 off = cp_wk->param_max * index + param;
  return cp_wk->p_data[off];
}
//----------------------------------------------------------------------------
/** 
 *	@brief  最大数を取得
 *
 *	@param	const RESORT_BINARY *cp_wk  ワーク
 *
 *	@return 最大数
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_GetMax( const RESORT_BINARY *cp_wk )
{
  return cp_wk->size / (cp_wk->param_max * sizeof(u16));
}

//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================

//-------------------------------------
/// PRIVATE
//=====================================
//-------------------------------------
/// PUBLIC	
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトIDを取得
 *
 *	@param	const RESORT_BINARY_PEOPLE_DATA *cp_wk　 ワーク
 *
 *	@return スクリプトID
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_PEOPLE_DATA_GetScrID( const RESORT_BINARY_PEOPLE_DATA *cp_wk )
{
  return cp_wk->param[ RESORT_BINARY_PEOPLE_PARAM_SCRID ];
}

//----------------------------------------------------------------------------
/**
 *	@brief　メッセージIDを取得
 *
 *	@param	const RESORT_BINARY_PEOPLE_DATA *cp_wk　ワーク
 *	@param	param 引数
 *
 *	@return　メッセージID
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_PEOPLE_DATA_GetMsgID( const RESORT_BINARY_PEOPLE_DATA *cp_wk, u16 param )
{
  u16 index = RESORT_BINARY_PEOPLE_PARAM_MSGID_00 + param;
  return cp_wk->param[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物タイプと種類からバイナリ取得
 *
 *	@param	const RESORT_BINARY_PEOPLE *cp_wkワーク
 *	@param	people_type　人物タイプ
 *	@param	kind 　種類
 *	@param  
 *
 *	@return　バイナリデータ
 */
//-----------------------------------------------------------------------------
const RESORT_BINARY_PEOPLE_DATA * RESORT_BINARY_SearchData( const RESORT_BINARY_PEOPLE *cp_wk, RESORT_PEOPLE_TYPE people_type, RESORT_KIND_TYPE kind )
{
  int i;
  const RESORT_BINARY_PEOPLE_DATA *cp_data;
  for( i = 0; i < RESORT_BINARY_PEOPLE_GetMax(cp_wk); i++ )
  {
    cp_data   = (const RESORT_BINARY_PEOPLE_DATA *)(cp_wk->p_data + cp_wk->param_max * i);
    if( RESORT_BINARY_GetData(cp_wk,i,RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE) == people_type
        && RESORT_BINARY_GetData(cp_wk,i,RESORT_BINARY_PEOPLE_PARAM_KIND) == kind )
    {
      return cp_data;
    }
  }

  GF_ASSERT_MSG(0, "people_type%d kind%d\n", people_type, kind);
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief　データ取得（メッセージから）
 *
 *	@param	const RESORT_BINARY_SPOT *cp_wk ワーク
 *	@param	msg メッセージ
 *	@param	p_index インデックス
 *
 *	@return   TRUE …あった FALSE …なかった
 */
//-----------------------------------------------------------------------------
BOOL RESORT_BINARY_SPOT_SearchIndex( const RESORT_BINARY_SPOT *cp_wk, u32 msg, u32 *p_index )
{
  for( (*p_index) = 0; (*p_index) < RESORT_BINARY_SPOT_GetMax(cp_wk); (*p_index)++ )
  {
    NAGI_Printf( "data %d msg %d \n", RESORT_BINARY_SPOT_GetData(cp_wk, *p_index, RESORT_BINARY_SPOT_PARAM_MSG_00), msg );
    if( RESORT_BINARY_SPOT_GetData(cp_wk, *p_index, RESORT_BINARY_SPOT_PARAM_MSG_00) == msg )
    {
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街レベルからインデックスを取得
 *
 *	@param	const RESORT_BINARY_CITY *cp_wk ワーク
 *	@param	lv  街レベル
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_CITY_GetIndexByCityLv( const RESORT_BINARY_CITY *cp_wk, u32 lv )
{
  int i;
  for( i = 0; i < RESORT_BINARY_CITY_GetMax(cp_wk); i++ )
  {
    u16 lv_l = RESORT_BINARY_CITY_GetData(cp_wk,i,RESORT_BINARY_CITY_PARAM_LV_LOW );
    u16 lv_h = RESORT_BINARY_CITY_GetData(cp_wk,i,RESORT_BINARY_CITY_PARAM_LV_HIGH );
    if( lv_l <= lv && lv <= lv_h )
    {
      return i;
    }
  }

  GF_ASSERT(0);
  return 0;
}
