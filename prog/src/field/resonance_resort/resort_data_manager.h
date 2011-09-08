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
#include "field/field_msgbg.h"

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
  RESORT_DATA_TYPE_RESIDENT = RESORT_PEOPLE_TYPE_RESIDENT,
  RESORT_DATA_TYPE_PROXY    = RESORT_PEOPLE_TYPE_PROXY,
  RESORT_DATA_TYPE_RECV     = RESORT_PEOPLE_TYPE_TOURIST,
  RESORT_DATA_TYPE_SECRETARY  = RESORT_PEOPLE_TYPE_SECRETARY,
  RESORT_DATA_TYPE_MAX,
}RESORT_DATA_TYPE;

//-------------------------------------
///	データタイプ
//=====================================
typedef enum
{
  RESORT_DATA_BUFFER_TYPE_RESIDENT,
  RESORT_DATA_BUFFER_TYPE_SECRETARY,
  RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
  RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
  RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  RESORT_DATA_BUFFER_TYPE_MAX,
  RESORT_DATA_BUFFER_TYPE_NULL = RESORT_DATA_BUFFER_TYPE_MAX,
} RESORT_DATA_BUFFER_TYPE;

//-------------------------------------
///	データ抽象型
//=====================================
typedef struct _RESORT_ABSTRUCT_DATA RESORT_ABSTRUCT_DATA;

extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_DATA_Init( HEAPID heapID, void *p_data_adrs, RESORT_DATA_BUFFER_TYPE type, u16 index );
extern void RESORT_ABSTRUCT_DATA_Exit( RESORT_ABSTRUCT_DATA *p_wk );

extern void RESORT_ABSTRUCT_DATA_Setup( RESORT_ABSTRUCT_DATA *p_wk, void *p_data_adrs, RESORT_DATA_TYPE type, u32 index );
extern void RESORT_ABSTRUCT_DATA_InitInstance( RESORT_ABSTRUCT_DATA *p_wk );

typedef struct
{
  const RESORT_BINARY_CITY *cp_city_data;
  const RESORT_BINARY_SPOT *cp_spot_data;
  const RESORT_BINARY_PRESENT *cp_present_data;
  const RESORT_CITY_DATA *cp_city;
}RESORT_ABSTRUCT_DATA_INIT;
extern BOOL RESORT_ABSTRUCT_DATA_IsEmpty( const RESORT_ABSTRUCT_DATA *cp_wk );
extern u32 RESORT_ABSTRUCT_DATA_GetData( const RESORT_ABSTRUCT_DATA *cp_wk, int param, void *p_ret_adrs );
extern void RESORT_ABSTRUCT_DATA_SetData( RESORT_ABSTRUCT_DATA *p_wk, int param, u32 data );
extern void * RESORT_ABSTRUCT_DATA_GetDataAdrs( RESORT_ABSTRUCT_DATA *p_wk );

extern RESORT_DATA_TYPE RESORT_ABSTRUCT_DATA_GetPeopleType( const RESORT_ABSTRUCT_DATA *cp_wk );
extern RESORT_DATA_BUFFER_TYPE RESORT_ABSTRUCT_DATA_GetBufferType( const RESORT_ABSTRUCT_DATA *cp_wk );
extern u32 RESORT_ABSTRUCT_DATA_GetBufferIndex( const RESORT_ABSTRUCT_DATA *cp_wk );
extern u32 RESORT_ABSTRUCT_DATA_GetRnd( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_RND_ID id, u32 range );

extern u32 RESORT_ABSTRUCT_DATA_DrawLotsWhereToGo( const RESORT_ABSTRUCT_DATA *cp_wk, const RESORT_BINARY_SPOT *cp_spot_data, const RESORT_BINARY_CITY *cp_city_data, const RESORT_CITY_DATA *cp_city );

extern u32 RESORT_ABSTRUCT_DATA_DrawLotsPresentItem( const RESORT_ABSTRUCT_DATA *cp_wk, const RESORT_BINARY_PRESENT *cp_present_data, const RESORT_BINARY_CITY *cp_city_data, const RESORT_CITY_DATA *cp_city );

//=============================================================================
/**
 *					データ抽象型マネージャ
*/
//=============================================================================

//-------------------------------------
///	データ抽象型
//=====================================
typedef struct _RESORT_ABSTRUCT_MANAGER RESORT_ABSTRUCT_MANAGER;

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
} RESORT_ALL_DATA_PARAM;

//システム
extern RESORT_ABSTRUCT_MANAGER * RESORT_ABSTRUCT_MANAGER_Init( const RESORT_ALL_DATA_PARAM *cp_param, HEAPID heapID );
extern void RESORT_ABSTRUCT_MANAGER_Exit( RESORT_ABSTRUCT_MANAGER * p_wk );

//イテレータ
typedef struct
{
  u16 tbl_index;
  u16 current;
}RESORT_ABSTRUCT_ITERATOR;
extern RESORT_ABSTRUCT_ITERATOR RESORT_ABSTRUCT_MANAGER_BeginIterator( RESORT_ABSTRUCT_MANAGER * p_wk, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max );
extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_NextIterator( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_ABSTRUCT_ITERATOR *p_ite, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max );

//データ取得
extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_ABSTRUCT_MANAGER * p_wk, const void *cp_data_adrs );
extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByIndex( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_DATA_BUFFER_TYPE type, u16 index );
extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByPeopleIndex( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_DATA_TYPE type, u16 index );

//空データ取得
extern RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetEmptyData( RESORT_ABSTRUCT_MANAGER * p_wk,RESORT_DATA_BUFFER_TYPE type );

//データ共通処理
extern void RESORT_ABSTRUCT_DATA_ModifyProfWord( RESORT_ABSTRUCT_DATA *p_wk, HEAPID heapID );

//=============================================================================
/**
 *					データマネージャー
*/
//=============================================================================
#define RESORT_DATA_MANAGER_INDEX_NULL  (0xFF)

//１つの場所にいれる人物最大数
#define RESORT_DATA_MANAGER_MAX (64)

//-------------------------------------
///	データマネージャー
//=====================================
typedef struct _RESORT_DATA_MANAGER RESORT_DATA_MANAGER;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_DATA_MANAGER * RESORT_DATA_MANAGER_Init( RESORT_ABSTRUCT_MANAGER *p_abstruct, HEAPID heapID );
extern void RESORT_DATA_MANAGER_Exit( RESORT_DATA_MANAGER *p_wk );

//データの作成、破棄、移動をしたらUpdateを呼ぶこと
extern void RESORT_DATA_MANAGER_Update( RESORT_DATA_MANAGER *p_wk );

//各データの抽象
extern u16 RESORT_DATA_BUFFER_TYPE_ToDataType( RESORT_DATA_BUFFER_TYPE type );

extern RESORT_DATA_BUFFER_TYPE RESORT_DATA_TYPE_ToBufferType( RESORT_DATA_TYPE type );

//お店がチェック
extern BOOL RESORT_DATA_MANAGER_IsEmptyInstitution( const RESORT_DATA_MANAGER *cp_wk );

//データ変換処理
extern BOOL RESORT_DATA_MANAGER_IsJobChange( const RESORT_DATA_MANAGER *cp_wk, const RESORT_ABSTRUCT_DATA *cp_src, RESORT_DATA_TYPE dst_type );
extern u32 RESORT_DATA_MANAGER_Convert( RESORT_DATA_MANAGER *p_wk, RESORT_ABSTRUCT_DATA *p_src, RESORT_DATA_TYPE dst_type );

//人物の位置情報
extern BOOL RESORT_DATA_MANAGER_IsPlaceEmpty( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID );
extern RESORT_ABSTRUCT_DATA * RESORT_DATA_MANAGER_GetAbstructDataByPlace( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID );
extern u32 RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID );

//作成
extern STRBUF * RESORT_DATA_MANAGER_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_ABSTRUCT_DATA *cp_data, u16 shopno, HEAPID heapID );

extern BOOL RESORT_DATA_MANAGER_CheckConvert( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_BUFFER_TYPE type, RESORT_PLACEID placeID, u16 *p_peopleID, RESORT_ABSTRUCT_DATA **pp_dst );

extern void RESORT_DATA_MANAGER_SetProxyFunRandom( RESORT_PROXY_DATA *p_wk, const RESORT_BINARY_PRESET *cp_preset, const RESORT_RECV_DATA *cp_owner, HEAPID heapID );
extern void RESORT_DATA_MANAGER_SetSecretaryPreset( RESORT_SECRETARY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 preset_type, u16 peopleID, HEAPID heapID );
extern void RESORT_DATA_MANAGER_SetProxyPreset( RESORT_PROXY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 preset_type, HEAPID heapID );

extern void RESORT_DATA_MANAGER_SetPresetPeople( RESORT_DATA_MANAGER *p_wk, RESORT_PEOPLE_ALL_DATA *p_people_all, RESORT_PROXY_BUFF *p_proxy_buff, const RESORT_BINARY_PRESET *cp_preset, const MYSTATUS *cp_mystatus, u16 people_type, u16 preset_type, HEAPID heapID );


//=============================================================================
/**
 *					レコード閲覧
*/
//=============================================================================
extern FLDMSGWIN * RESORT_RANK_WND_Create( RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID );


extern u16 RESORT_SCRCMD_GetMsgID( u16 param, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_MANAGER *p_man, RESORT_CITY_DATA *p_city ,RESORT_BINARY_SPOT *p_spot, RESORT_BINARY_SHOP *p_shop, RESORT_BINARY_CITY *p_city_data, WORDSET *p_wordset, HEAPID heapID );

extern void RESORT_SCRCMD_CheckSpot( u16 index, RESORT_RESIDENT_DATA *p_resident, RESORT_CITY_DATA *p_city_data,RESORT_BINARY_SPOT *p_spot, RESORT_BINARY_SHOP *p_shop, RESORT_BINARY_CITY *p_city, WORDSET *p_wordset, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup );
