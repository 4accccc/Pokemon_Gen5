//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_data_manager.h
 *	@brief  データマネージャー
 *	@author	Toru=Nagihashi
 *	@date		2011.02.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "field/resonance_resort_data.h"
#include "resort_map.h"
#include "resort_people_def.h"
#include "resort_binary.h"
//=============================================================================
/**
 *					データマネージャー
*/
//=============================================================================
//-------------------------------------
///	データタイプ
//=====================================
typedef enum
{
  RESORT_DATA_MANAGER_TYPE_RESIDENT,
  RESORT_DATA_MANAGER_TYPE_VISITOR,
  RESORT_DATA_MANAGER_TYPE_SECRETARY,
  RESORT_DATA_MANAGER_TYPE_RECV_SV_BUFF,
  RESORT_DATA_MANAGER_TYPE_RECV_TMP_BUFF,
  RESORT_DATA_MANAGER_TYPE_PROXY_BUFF,
  RESORT_DATA_MANAGER_TYPE_MAX,
  RESORT_DATA_MANAGER_TYPE_NULL = RESORT_DATA_MANAGER_TYPE_MAX,
} RESORT_DATA_MANAGER_TYPE;

#define RESORT_DATA_MANAGER_INDEX_NULL  (0xFF)

//１つの場所にいれる人物最大数
#define RESORT_DATA_MANAGER_MAX (32)

//-------------------------------------
///	データマネージャー
//=====================================
typedef struct _RESORT_DATA_MANAGER RESORT_DATA_MANAGER;

//-------------------------------------
///	初期化構造体
//=====================================
typedef struct
{
  RESORT_PEOPLE_ALL_DATA *p_all_people;
  RESORT_RECV_BUFF    *p_recv_sv_buff;
  RESORT_RECV_BUFF    *p_recv_tmp_buff;
  RESORT_PROXY_BUFF   *p_proxy_buff;
  RESORT_CITY_DATA    *p_city_data;
} RESORT_DATA_MANAGER_INIT_PARAM;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_DATA_MANAGER * RESORT_DATA_MANAGER_Init( const RESORT_DATA_MANAGER_INIT_PARAM *cp_param, HEAPID heapID );
extern void RESORT_DATA_MANAGER_Exit( RESORT_DATA_MANAGER *p_wk );

//データの作成、破棄、移動をしたらUpdateを呼ぶこと
extern void RESORT_DATA_MANAGER_Update( RESORT_DATA_MANAGER *p_wk );

//各データの抽象
extern void *RESORT_DATA_MANAGER_GetDataAdrs( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_MANAGER_TYPE type, u32 data_index );
extern u32 RESORT_DATA_MANAGER_GetPeopleType( RESORT_DATA_MANAGER_TYPE type );

//お店がチェック
extern BOOL RESORT_DATA_MANAGER_IsEmptyInstitution( const RESORT_DATA_MANAGER *cp_wk );

//データ変換処理
extern BOOL RESORT_DATA_MANAGER_IsRecvToResident( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src );
extern void RESORT_DATA_MANAGER_RecvToResident( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src, u16 type );
extern BOOL RESORT_DATA_MANAGER_IsRecvToVisitor( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src );
extern void RESORT_DATA_MANAGER_RecvToVisitor( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src );
extern BOOL RESORT_DATA_MANAGER_IsRecvToSecretary( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src );
extern void RESORT_DATA_MANAGER_RecvToSecretary( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src );
extern BOOL RESORT_DATA_MANAGER_IsResidentToVisitor( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RESIDENT_DATA *cp_src );
extern void RESORT_DATA_MANAGER_ResidentToVisitor( RESORT_DATA_MANAGER *p_wk, RESORT_RESIDENT_DATA *p_src );

//人物の位置情報
extern BOOL RESORT_DATA_MANAGER_IsPlaceEmpty( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID );
extern void RESORT_DATA_MANAGER_GetPlaceData( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID, RESORT_DATA_MANAGER_TYPE *p_type, u16 *p_data_idx );
extern u32 RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID );

//何かをキーにして検索
extern RESORT_RESIDENT_DATA * RESORT_DATA_MANAGER_SearchResident( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u16 peopleID );

//作成
extern STRBUF * RESORT_DATA_MANAGER_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_RESIDENT_DATA *cp_data, HEAPID heapID );


typedef enum
{
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR,
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_SECRETARY,
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_RESIDENT,
}RESORT_DATA_MANAGER_CHECKCONVERT_DST;
extern BOOL RESORT_DATA_MANAGER_CheckConvert( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST param, RESORT_PLACEID placeID, u16 *p_peopleID, void **pp_dst );

//=============================================================================
/**
 *					データ抽象型
*/
//=============================================================================
//-------------------------------------
///	データ抽象型
//=====================================
typedef enum
{
  RESORT_DATA_TYPE_RESIDENT,
  RESORT_DATA_TYPE_PROXY,
  RESORT_DATA_TYPE_RECV,
  RESORT_DATA_TYPE_VISITOR,
  RESORT_DATA_TYPE_SECRETARY,
  RESORT_DATA_TYPE_MAX,
}RESORT_DATA_TYPE;

//-------------------------------------
///	データ抽象型
//=====================================
typedef struct _RESORT_ABSTRUCT_DATA RESORT_ABSTRUCT_DATA;

extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_DATA_Init( HEAPID heapID, void *p_data_adrs, RESORT_DATA_TYPE type );
extern void RESORT_ABSTRUCT_DATA_Exit( RESORT_ABSTRUCT_DATA *p_wk );

extern void RESORT_ABSTRUCT_DATA_Setup( RESORT_ABSTRUCT_DATA *p_wk, void *p_data_adrs, RESORT_DATA_TYPE type );
extern void RESORT_ABSTRUCT_DATA_InitInstance( RESORT_ABSTRUCT_DATA *p_wk );
extern BOOL RESORT_ABSTRUCT_DATA_IsEmpty( const RESORT_ABSTRUCT_DATA *cp_wk );
extern u32 RESORT_ABSTRUCT_DATA_GetData( const RESORT_ABSTRUCT_DATA *cp_wk, int param, void *p_ret_adrs );
extern void RESORT_ABSTRUCT_DATA_SetData( RESORT_ABSTRUCT_DATA *p_wk, int param, u32 data );
extern void * RESORT_ABSTRUCT_DATA_GetDataAdrs( RESORT_ABSTRUCT_DATA *p_wk );

extern RESORT_DATA_TYPE PeopleTypeToResortDataType( u32 people_type );

