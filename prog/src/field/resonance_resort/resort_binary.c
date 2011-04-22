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
//----------------------------------------------------------------------------
/**
 *	@brief  構造体開始アドレスを取得
 *
 *	@param	const RESORT_BINARY *cp_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return 構造体開始アドレス
 */
//-----------------------------------------------------------------------------
void * RESORT_BINARY_GetStruct( const RESORT_BINARY *cp_wk, u32 index )
{
  return cp_wk->p_data + cp_wk->param_max * index;
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
    cp_data   = RESORT_BINARY_GetStruct(cp_wk, i);
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

//=============================================================================
/**
 *					レゾナンスリゾートお店バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	お店バイナリデータワーク
//=====================================
struct _RESORT_BINARY_SHOP
{
  RESORT_BINARY *p_shop;
  RESORT_BINARY *p_service;
};

//-------------------------------------
///	お店バイナリデータ 各お店分のワーク
//=====================================
struct _RESORT_BINARY_SHOP_DATA
{
  u16 param[RESORT_BINARY_SHOP_PARAM_MAX];
};

//-------------------------------------
///	サービスバイナリデータ サービス情報のワーク
//=====================================
struct _RESORT_BINARY_SERVICE_DATA
{
  u16 param[RESORT_BINARY_SERVICE_PARAM_MAX];
};


//-------------------------------------
/// PUBLIC	
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  お店バイナリ読み込み
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return お店バイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SHOP * RESORT_BINARY_SHOP_Init( HEAPID heapID )
{
  RESORT_BINARY_SHOP *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_BINARY_SHOP) );

  p_wk->p_shop = RESORT_BINARY_Init( NARC_resonance_resort_data_resort_shop_dat, RESORT_BINARY_SHOP_PARAM_MAX,heapID);

  p_wk->p_service = RESORT_BINARY_Init( NARC_resonance_resort_data_resort_service_dat, RESORT_BINARY_SERVICE_PARAM_MAX, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  お店バイナリ開放
 *
 *	@param	RESORT_BINARY_SHOP *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_SHOP_Exit( RESORT_BINARY_SHOP *p_wk )
{
  RESORT_BINARY_Exit(p_wk->p_service);
  RESORT_BINARY_Exit(p_wk->p_shop);
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief　お店バイナリから個別の店情報を取得
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk ワーク
 *	@param	shop  お店
 *	@param	type  タイプ
 *	@param	lv    レベル
 *
 *	@return　個別の店情報
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_SHOP_SearchShopID( const RESORT_BINARY_SHOP *cp_wk, const RESORT_RESIDENT_DATA *cp_data )
{
  RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( (RESORT_RESIDENT_DATA *)cp_data ); 
  u16 shop  = RESORT_RESIDENT_DATA_GetData( cp_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
  u16 type  = 0; //TODO
  u16 lv    = RESORT_INSTITUTION_DATA_GetData(p_institution,RESORT_INSTITUTION_DATA_PARAM_LV );
  int i;

  for( i = 0; i < RESORT_BINARY_SHOP_DATA_GetMax(cp_wk); i++ )
  {
    const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_GetStruct(cp_wk->p_shop, i);
    if( cp_shop->param[ RESORT_BINARY_SHOP_PARAM_SHOP ] == shop
      &&  cp_shop->param[ RESORT_BINARY_SHOP_PARAM_TYPE ] == type
      &&  cp_shop->param[ RESORT_BINARY_SHOP_PARAM_LV ] == lv )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0, "shop%d type%d lv%d\n", shop, type, lv);

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief　お店バイナリから個別の店情報を取得（RESIDENT版）
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk ワーク
 *	@param	*p_data 住民データ
 *
 *	@return 個別の店情報
 */
//-----------------------------------------------------------------------------
const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_SearchDataByData( const RESORT_BINARY_SHOP *cp_wk, RESORT_RESIDENT_DATA *p_data )
{
  RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data );
  u16 id = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
  const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_GetStruct(cp_wk->p_shop, id);

  GF_ASSERT(id < RESORT_BINARY_SHOP_DATA_GetMax(cp_wk));

  return cp_shop;
}
//----------------------------------------------------------------------------
/**
 *	@brief  最大値を取得
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk ワーク
 *
 *	@return 最大値
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_SHOP_DATA_GetMax( const RESORT_BINARY_SHOP *cp_wk )
{
  return RESORT_BINARY_GetMax(cp_wk->p_shop);
}

//----------------------------------------------------------------------------
/**
 *	@brief　個別の店情報からお店名を取得
 *
 *	@param	const RESORT_BINARY_SHOP_DATA *cp_wk 　ワーク
 *
 *	@return お店名タグ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_SHOP_DATA_GetData( const RESORT_BINARY_SHOP_DATA *cp_wk, RESORT_BINARY_SHOP_PARAM param )
{
  return cp_wk->param[ param ];
}

//----------------------------------------------------------------------------
/**
 *	@brief　個別の店情報からサービス情報を取得
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk  ワーク
 *	@param	const RESORT_BINARY_SHOP_DATA *cp_shop  お店情報
 *	@param	list  何番のサービスか
 *
 *	@return サービス情報
 */
//-----------------------------------------------------------------------------
const RESORT_BINARY_SERVICE_DATA * RESORT_BINARY_SHOP_DATA_GetList( const RESORT_BINARY_SHOP *cp_wk, const RESORT_BINARY_SHOP_DATA *cp_shop, u16 list )
{
  u16 service_tag = cp_shop->param[RESORT_BINARY_SHOP_PARAM_LIST_01+list];
  
  if( service_tag < RESORT_BINARY_GetMax( cp_wk->p_service ) )
  {
    return RESORT_BINARY_GetStruct(cp_wk->p_service, service_tag);
  }
  else
  {
    return NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  サービスの個数を取得
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk   ワーク
 *	@param	RESORT_BINARY_SHOP_DATA *cp_shop  店データ
 *
 *	@return サービスの個数
 */
//-----------------------------------------------------------------------------
const u32 RESORT_BINARY_SHOP_DATA_GetListMax( const RESORT_BINARY_SHOP *cp_wk, const RESORT_BINARY_SHOP_DATA *cp_shop )
{
  int i;
  for( i = 0; i < RESORT_BINARY_SHOP_LIST_MAX; i++ )
  {
    const RESORT_BINARY_SERVICE_DATA *cp_data = RESORT_BINARY_SHOP_DATA_GetList( cp_wk, cp_shop, i );
    if( cp_data == NULL )
    {
      break;
    }
  }

  return i;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サービス情報からデータ取得
 *
 *	@param	const RESORT_BINARY_SERVICE_DATA *p_wk  ワーク
 *	@param	param   取得する種類
 *
 *	@return　データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_SERVICE_DATA_GetData( const RESORT_BINARY_SERVICE_DATA *cp_wk, RESORT_BINARY_SERVICE_PARAM param )
{
  GF_ASSERT( param < RESORT_BINARY_SERVICE_PARAM_MAX );
  return cp_wk->param[ param ];
}
