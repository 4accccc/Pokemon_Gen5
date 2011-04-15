//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_binary_st.h
 *	@brief  レゾナンスリゾートバイナリデータ 常駐
 *	@author	Toru=Nagihashi
 *	@date		2011.02.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "field/resonance_resort_data.h"

//=============================================================================
/**
 *					レゾナンスリゾートお店バイナリデータ
*/
//=============================================================================
#define RESORT_BINARY_SHOP_LIST_MAX (6)

//-------------------------------------
///	お店バイナリデータワーク
//=====================================
typedef struct _RESORT_BINARY_SHOP RESORT_BINARY_SHOP;

//-------------------------------------
///	お店バイナリデータ 各お店分のワーク
//=====================================
typedef struct _RESORT_BINARY_SHOP_DATA RESORT_BINARY_SHOP_DATA;

//-------------------------------------
///	サービスバイナリデータ サービス情報のワーク
//=====================================
typedef struct _RESORT_BINARY_SERVICE_DATA RESORT_BINARY_SERVICE_DATA;

//-------------------------------------
/// PUBLIC	
//=====================================
//バイナリ読み込み、破棄
extern RESORT_BINARY_SHOP * RESORT_BINARY_SHOP_Init( HEAPID heapID );
extern void RESORT_BINARY_SHOP_Exit( RESORT_BINARY_SHOP *p_wk );


extern u32 RESORT_BINARY_SHOP_SearchShopID( const RESORT_BINARY_SHOP *cp_wk, const RESORT_RESIDENT_DATA *cp_data );
extern const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_SearchDataByData( const RESORT_BINARY_SHOP *cp_wk, RESORT_RESIDENT_DATA *p_data );
extern u32 RESORT_BINARY_SHOP_DATA_GetMax( const RESORT_BINARY_SHOP *cp_wk );

//-------------------------------------
///	お店のデータ
//=====================================
typedef enum 
{
  RESORT_BINARY_SHOP_PARAM_SHOP,
  RESORT_BINARY_SHOP_PARAM_TYPE,
  RESORT_BINARY_SHOP_PARAM_LV,
  RESORT_BINARY_SHOP_PARAM_NEXT_EXP,
  RESORT_BINARY_SHOP_PARAM_NAME,
  RESORT_BINARY_SHOP_PARAM_OUT_MODEL,
  RESORT_BINARY_SHOP_PARAM_IN_3DMD,
  RESORT_BINARY_SHOP_PARAM_IN_TEX,
  RESORT_BINARY_SHOP_PARAM_LIST_01,
  RESORT_BINARY_SHOP_PARAM_LIST_02,
  RESORT_BINARY_SHOP_PARAM_LIST_03,
  RESORT_BINARY_SHOP_PARAM_LIST_04,
  RESORT_BINARY_SHOP_PARAM_LIST_05,
  RESORT_BINARY_SHOP_PARAM_LIST_06,
  RESORT_BINARY_SHOP_PARAM_MAX,
} RESORT_BINARY_SHOP_PARAM;
extern u16 RESORT_BINARY_SHOP_DATA_GetData( const RESORT_BINARY_SHOP_DATA *cp_wk, RESORT_BINARY_SHOP_PARAM param );

extern const RESORT_BINARY_SERVICE_DATA * RESORT_BINARY_SHOP_DATA_GetList( const RESORT_BINARY_SHOP *cp_wk, const RESORT_BINARY_SHOP_DATA *cp_shop, u16 list );
extern const u32 RESORT_BINARY_SHOP_DATA_GetListMax( const RESORT_BINARY_SHOP *cp_wk, const RESORT_BINARY_SHOP_DATA *cp_shop );

typedef enum
{
  RESORT_BINARY_SERVICE_PARAM_NAME_FLAG,
  RESORT_BINARY_SERVICE_PARAM_NAME,
  RESORT_BINARY_SERVICE_PARAM_INFO,
  RESORT_BINARY_SERVICE_PARAM_PRICE,
  RESORT_BINARY_SERVICE_PARAM_SERVICE_ID,
  RESORT_BINARY_SERVICE_PARAM_PARAM_1,
  RESORT_BINARY_SERVICE_PARAM_PARAM_2,
  RESORT_BINARY_SERVICE_PARAM_EFFECT,
  RESORT_BINARY_SERVICE_PARAM_MAX,
}RESORT_BINARY_SERVICE_PARAM;
extern u16 RESORT_BINARY_SERVICE_DATA_GetData( const RESORT_BINARY_SERVICE_DATA *cp_wk, RESORT_BINARY_SERVICE_PARAM param );


//=============================================================================
/**
 *					施設データ
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _RESORT_BINARY_INSTITUTION RESORT_BINARY_INSTITUTION;

//-------------------------------------
///	取得データ
//=====================================
typedef enum
{
  RESORT_BINARY_INSTITUTION_PARAM_NUMBER,
  RESORT_BINARY_INSTITUTION_PARAM_GRID_X,
  RESORT_BINARY_INSTITUTION_PARAM_GRID_Z,
  RESORT_BINARY_INSTITUTION_PARAM_DOOR_ID,
  RESORT_BINARY_INSTITUTION_PARAM_MAX,
}RESORT_BINARY_INSTITUTION_PARAM;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_BINARY_INSTITUTION * RESORT_BINARY_INSTITUTION_Init( HEAPID heapID );
extern void RESORT_BINARY_INSTITUTION_Exit( RESORT_BINARY_INSTITUTION *p_wk );
extern u16 RESORT_BINARY_INSTITUTION_GetData( const RESORT_BINARY_INSTITUTION *cp_wk, u32 index, RESORT_BINARY_INSTITUTION_PARAM param );
