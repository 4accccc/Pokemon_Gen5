//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_binary.h
 *	@brief  レゾナンスリゾートバイナリデータ読み込み
 *	@author	Toru=Nagihashi
 *	@date		2011.02.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "field/resonance_resort_data.h"
//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	人物要素定義  値はコンバータから吐き出されるので型を付けてあげる
//=====================================
typedef s32 RESORT_PEOPLE_TYPE;
typedef s32 RESORT_PERSONALITY_TYPE;

typedef s32 RESORT_KIND_TYPE; //以下のタイプすべてが格納される
typedef s32 RESORT_RESIDENT_TYPE;
typedef s32 RESORT_PROXY_TYPE;
typedef s32 RESORT_TURIST_TYPE;
typedef s32 RESORT_SECRETARY_TYPE;
typedef s32 RESORT_VISITOR_TYPE;
typedef s32 RESORT_NPC_TYPE;

//-------------------------------------
///	人物バイナリデータワーク
//=====================================
typedef struct _RESORT_BINARY_PEOPLE RESORT_BINARY_PEOPLE;

//-------------------------------------
///	人物バイナリデータ 各人物分のワーク
//=====================================
typedef struct _RESORT_BINARY_PEOPLE_DATA RESORT_BINARY_PEOPLE_DATA;

//-------------------------------------
/// PUBLIC	
//=====================================
//バイナリ読み込み、破棄
extern RESORT_BINARY_PEOPLE * RESORT_BINARY_PEOPLE_Init( HEAPID heapID );
extern void RESORT_BINARY_PEOPLE_Exit( RESORT_BINARY_PEOPLE *p_wk );


extern const RESORT_BINARY_PEOPLE_DATA * RESORT_BINARY_SearchData( const RESORT_BINARY_PEOPLE *cp_wk, RESORT_PEOPLE_TYPE people_type, RESORT_KIND_TYPE kind );

//バイナリからデータ取得
extern u16 RESORT_BINARY_PEOPLE_DATA_GetScrID( const RESORT_BINARY_PEOPLE_DATA *cp_wk );
extern u16 RESORT_BINARY_PEOPLE_DATA_GetMsgID( const RESORT_BINARY_PEOPLE_DATA *cp_wk, u16 param );


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


extern const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_SearchData( const RESORT_BINARY_SHOP *cp_wk, u16 shop, u16 type, u16 lv );
extern const RESORT_BINARY_SHOP_DATA * RESORT_BINARY_SHOP_SearchDataByData( const RESORT_BINARY_SHOP *cp_wk, RESORT_RESIDENT_DATA *p_data );

//バイナリからデータ取得
extern u16 RESORT_BINARY_SHOP_DATA_GetName( const RESORT_BINARY_SHOP_DATA *cp_wk );

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
  RESORT_BINARY_SERVICE_PARAM_MAX,
}RESORT_BINARY_SERVICE_PARAM;
extern u16 RESORT_BINARY_SERVICE_DATA_GetData( const RESORT_BINARY_SERVICE_DATA *cp_wk, RESORT_BINARY_SERVICE_PARAM param );
