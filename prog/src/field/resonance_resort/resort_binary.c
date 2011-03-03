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
#include "resort_binary.h"

//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	データ
//=====================================
typedef enum
{
  RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE,
  RESORT_BINARY_PEOPLE_PARAM_KIND,
  RESORT_BINARY_PEOPLE_PARAM_PERSONALITY,
  RESORT_BINARY_PEOPLE_PARAM_VIEW,
  RESORT_BINARY_PEOPLE_PARAM_SEX,
  RESORT_BINARY_PEOPLE_PARAM_SCRID,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_00,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_01,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_02,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_03,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_04,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_05,
  RESORT_BINARY_PEOPLE_PARAM_MAX,
}RESORT_BINARY_PEOPLE_PARAM;

//-------------------------------------
///	人物バイナリの内容
//=====================================
struct _RESORT_BINARY_PEOPLE_DATA
{
  u16 param[RESORT_BINARY_PEOPLE_PARAM_MAX];
} ;


//-------------------------------------
///	人物バイナリデータワーク
//=====================================
struct _RESORT_BINARY_PEOPLE
{
  u32   size;
  RESORT_BINARY_PEOPLE_DATA  *p_data;
};

//-------------------------------------
/// PRIVATE
//=====================================
//-------------------------------------
/// PUBLIC	
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ読み込み
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@returnデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_PEOPLE * RESORT_BINARY_PEOPLE_Init( HEAPID heapID )
{
  RESORT_BINARY_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_PEOPLE) );

  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_people_dat, FALSE, heapID, &p_wk->size );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ破棄
 *
 *	@param	RESORT_BINARY_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_PEOPLE_Exit( RESORT_BINARY_PEOPLE *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}

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
  for( i = 0; i < cp_wk->size/sizeof(RESORT_BINARY_PEOPLE_DATA); i++ )
  {
    cp_data   = &cp_wk->p_data[i];
    if( cp_data->param[RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE] == people_type
        && cp_data->param[RESORT_BINARY_PEOPLE_PARAM_KIND] == kind)
    {
      return cp_data;
    }
  }

  GF_ASSERT(0);
  return NULL;
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
  u32 service_size;
  RESORT_BINARY_SERVICE_DATA  *p_service;

  u32 shop_size;
  RESORT_BINARY_SHOP_DATA *p_shop;
};

//-------------------------------------
///	お店のデータ
//=====================================
typedef enum 
{
  RESORT_BINARY_SHOP_PARAM_SHOP,
  RESORT_BINARY_SHOP_PARAM_TYPE,
  RESORT_BINARY_SHOP_PARAM_LV,
  RESORT_BINARY_SHOP_PARAM_NAME,
  RESORT_BINARY_SHOP_PARAM_LIST_01,
  RESORT_BINARY_SHOP_PARAM_LIST_02,
  RESORT_BINARY_SHOP_PARAM_LIST_03,
  RESORT_BINARY_SHOP_PARAM_LIST_04,
  RESORT_BINARY_SHOP_PARAM_LIST_05,
  RESORT_BINARY_SHOP_PARAM_LIST_06,
  RESORT_BINARY_SHOP_PARAM_MAX,
} RESORT_BINARY_SHOP_PARAM;

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

  GF_ASSERT(0);

  return NULL;
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
u16 RESORT_BINARY_SHOP_DATA_GetName( const RESORT_BINARY_SHOP_DATA *cp_wk )
{
  return cp_wk->param[ RESORT_BINARY_SHOP_PARAM_NAME ];
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
