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
#include "resonance_resort_data.naix"

//outer
#include "resort_binary_st.h"


#ifdef PM_DEBUG
#define DEBUG_DATA_PRINT
#endif //PM_DEBUG

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
  u32 service_size;
  RESORT_BINARY_SERVICE_DATA  *p_service;

  u32 shop_size;
  RESORT_BINARY_SHOP_DATA *p_shop;
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

  p_wk->p_service = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_service_dat, FALSE, heapID, &p_wk->service_size );


  p_wk->p_shop = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_shop_dat, FALSE, heapID, &p_wk->shop_size );

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
  GFL_HEAP_FreeMemory(p_wk->p_shop);
  GFL_HEAP_FreeMemory(p_wk->p_service);
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
const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_SearchData( const RESORT_BINARY_SHOP *cp_wk, u16 shop, u16 type, u16 lv )
{
  int i;
  for( i = 0; i < (cp_wk->shop_size / sizeof(RESORT_BINARY_SHOP_DATA)); i++ )
  {
    const RESORT_BINARY_SHOP_DATA* cp_shop = &cp_wk->p_shop[i];
    if( cp_shop->param[ RESORT_BINARY_SHOP_PARAM_SHOP ] == shop
      &&  cp_shop->param[ RESORT_BINARY_SHOP_PARAM_TYPE ] == type
      &&  cp_shop->param[ RESORT_BINARY_SHOP_PARAM_LV ] == lv )
    {
      return cp_shop;
    }
  }

  GF_ASSERT_MSG(0, "shop%d type%d lv%d\n", shop, type, lv);

  return NULL;
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

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_data );
  u16 shop = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_TYPE, NULL );
  u16 type = 0; //TODO
  u16 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  return RESORT_BINARY_SHOP_SearchData( cp_wk, shop, type, lv );
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
extern const RESORT_BINARY_SERVICE_DATA * RESORT_BINARY_SHOP_DATA_GetList( const RESORT_BINARY_SHOP *cp_wk, const RESORT_BINARY_SHOP_DATA *cp_shop, u16 list )
{
  u16 service_tag = cp_shop->param[RESORT_BINARY_SHOP_PARAM_LIST_01+list];
  
  if( service_tag < cp_wk->service_size / sizeof(RESORT_BINARY_SERVICE_DATA) )
  {
    return &cp_wk->p_service[ service_tag ];
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


//=============================================================================
/**
 *					施設データ
*/
//=============================================================================
//-------------------------------------
///	バイナリの中身
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_INSTITUTION_PARAM_MAX ];
} RESORT_BINARY_INSTITUTION_DATA;

//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY_INSTITUTION
{
  u32 size;
  RESORT_BINARY_INSTITUTION_DATA *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ作成
 *
 *	@param	HEAPID heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
 RESORT_BINARY_INSTITUTION * RESORT_BINARY_INSTITUTION_Init( HEAPID heapID )
{
  RESORT_BINARY_INSTITUTION *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_INSTITUTION ) );

  p_wk->p_data = GFL_ARC_UTIL_LoadEx( ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_institution_dat, FALSE, heapID, &p_wk->size );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ破棄
 *
 *	@param	RESORT_BINARY_INSTITUTION *p_wk ワーク
 */
//-----------------------------------------------------------------------------
 void RESORT_BINARY_INSTITUTION_Exit( RESORT_BINARY_INSTITUTION *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データから取得
 *
 *	@param	const RESORT_BINARY_INSTITUTION *cp_wk  ワーク
 *	@param	index
 *	@param	param 取得する種類
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
 u16 RESORT_BINARY_INSTITUTION_GetData( const RESORT_BINARY_INSTITUTION *cp_wk, u32 index, RESORT_BINARY_INSTITUTION_PARAM param )
{
  GF_ASSERT( index < cp_wk->size / sizeof(RESORT_BINARY_INSTITUTION_DATA) );
  GF_ASSERT( param < RESORT_BINARY_INSTITUTION_PARAM_MAX );
  return cp_wk->p_data[ index ].param[ param ];
}

