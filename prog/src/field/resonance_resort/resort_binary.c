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
#include "system/main.h"

//resource
#include "arc_def.h"

//debug
#include "resort_debug.h"

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
const RESORT_BINARY_PEOPLE_DATA * RESORT_BINARY_SearchData( const RESORT_BINARY_PEOPLE *cp_wk, RESORT_PEOPLE_TYPE people_type, RESORT_KIND_TYPE kind, u16 sex )
{
  int i;
  const RESORT_BINARY_PEOPLE_DATA *cp_data;
  for( i = 0; i < RESORT_BINARY_PEOPLE_GetMax(cp_wk); i++ )
  {
    cp_data   = RESORT_BINARY_GetStruct(cp_wk, i);
    if( RESORT_BINARY_GetData(cp_wk,i,RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE) == people_type
        && RESORT_BINARY_GetData(cp_wk,i,RESORT_BINARY_PEOPLE_PARAM_KIND) == kind
        && RESORT_BINARY_GetData(cp_wk,i,RESORT_BINARY_PEOPLE_PARAM_SEX) == sex
      )
    {
      return cp_data;
    }
  }

  GF_ASSERT_MSG(0, "people_type%d kind%d sex%d\n", people_type, kind, sex);
  return NULL;
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
//----------------------------------------------------------------------------
/**
 *	@brief  街の最大レベル
 *
 *	@param	const RESORT_BINARY_CITY *cp_wk   街データ
 *
 *	@return 最大LV
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_CITY_GetCityLvMax( const RESORT_BINARY_CITY *cp_wk )
{
  u32 max = RESORT_BINARY_CITY_GetMax( cp_wk );
  return RESORT_BINARY_CITY_GetData(cp_wk,max-1,RESORT_BINARY_CITY_PARAM_LV_HIGH );
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
  GF_ASSERT( p_wk != NULL );

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
u32 RESORT_BINARY_SHOP_SearchShopID( const RESORT_BINARY_SHOP *cp_wk, u16 kind, u16 lv, u16 type )
{
  int i;

  //先頭はNULLデータとしている
  for( i = 1; i < RESORT_BINARY_SHOP_DATA_GetMax(cp_wk); i ++ )
  {
    const RESORT_BINARY_SHOP_DATA *cp_data = RESORT_BINARY_GetStruct(cp_wk->p_shop, i);
    if( RESORT_BINARY_SHOP_DATA_GetData( cp_data, RESORT_BINARY_SHOP_PARAM_SHOP ) == kind 
        && RESORT_BINARY_SHOP_DATA_GetData( cp_data, RESORT_BINARY_SHOP_PARAM_TYPE ) == type
        && RESORT_BINARY_SHOP_DATA_GetData( cp_data, RESORT_BINARY_SHOP_PARAM_LV ) == lv
        )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0,"kind%dlv%dtype%d\n",kind,lv,type);
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

  GF_ASSERT(id < RESORT_BINARY_SHOP_DATA_GetMax(cp_wk));

  return RESORT_BINARY_GetStruct(cp_wk->p_shop, id);
}
//----------------------------------------------------------------------------
/**
 *	@brief  お店から個別店情報を取得
 *
 *	@param	const RESORT_BINARY_SHOP *cp_wk ワーク
 *	@param	index インデックス
 *
 *	@return お店データ
 */
//-----------------------------------------------------------------------------
const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_GetData( const RESORT_BINARY_SHOP *cp_wk,u16 index)
{
  GF_ASSERT(index < RESORT_BINARY_SHOP_DATA_GetMax(cp_wk));
  return RESORT_BINARY_GetStruct(cp_wk->p_shop, index);
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
  u16 service_tag = cp_shop->param[RESORT_BINARY_SHOP_PARAM_LIST_START+list];
  GF_ASSERT( list < RESORT_BINARY_SHOP_LIST_MAX );
  
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


//----------------------------------------------------------------------------
/**
 *	@brief  抽選
 *
 *	@param	const RESORT_BINARY_PRESENT *cp_wk  ワーク
 *	@param	rnd 乱数（RESORT_BINARY_PRESENT_PARAM_DENOMINATORの）
 *
 *	@return 抽選したインデックス  (RESORT_BINARY_PRESENT_PARAM_ITEM_MAXならなし)
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_PRESENT_DrawLotsIndex( const RESORT_BINARY_PRESENT *cp_wk, u32 listID, u32 rnd )
{
  int i;
  u32 percent = 0;


#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_ITEM_WORK_ON) )
  {
    return RESORT_DEBUG_WORK_Get(RESORT_DEBUG_WORK_FORCE_ITEM);
  }
#endif

  for( i = 0; i < RESORT_BINARY_PRESENT_PARAM_ITEM_MAX; i++ )
  {
    u32 item = RESORT_BINARY_PRESENT_GetData(cp_wk, listID, RESORT_BINARY_PRESENT_PARAM_ITEM_00+i*2);
    percent += RESORT_BINARY_PRESENT_GetData(cp_wk, listID, RESORT_BINARY_PRESENT_PARAM_PERCENT_00+i*2);
    if( rnd < percent )
    {
     GF_ASSERT_MSG( item != 0, "idx%d item%d percent%d rnd%d\n", i, item, percent, rnd );
      return i;
    }
  }
  NAGI_Printf( "list%d per%d\n", listID, percent );
  return RESORT_BINARY_PRESENT_PARAM_ITEM_MAX;
}
