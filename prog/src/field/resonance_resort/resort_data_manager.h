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
#include "src/field/resonance_resort/resort_map.h"
#include "src/field/resonance_resort/resort_people_def.h"
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
} RESORT_DATA_MANAGER_INIT_PARAM;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_DATA_MANAGER * RESORT_DATA_MANAGER_Init( const RESORT_DATA_MANAGER_INIT_PARAM *cp_param, HEAPID heapID );
extern void RESORT_DATA_MANAGER_Exit( RESORT_DATA_MANAGER *p_wk );

//データの作成、破棄、移動をしたらSetupを呼ぶこと
extern void RESORT_DATA_MANAGER_Update( RESORT_DATA_MANAGER *p_wk );

//各データの抽象
extern void *RESORT_DATA_MANAGER_GetDataAdrs( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_MANAGER_TYPE type, u32 data_index );
extern u32 RESORT_DATA_MANAGER_GetPeopleType( RESORT_DATA_MANAGER_TYPE type );

//データ変換処理
extern BOOL RESORT_DATA_MANAGER_RecvToResident( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src );
extern BOOL RESORT_DATA_MANAGER_RecvToVistor( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src );
extern BOOL RESORT_DATA_MANAGER_RecvToSecretary( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src );
extern BOOL RESORT_DATA_MANAGER_ResidentToVistor( RESORT_DATA_MANAGER *p_wk, RESORT_RESIDENT_DATA *p_src );

//人物の位置情報
extern BOOL RESORT_DATA_MANAGER_IsPlaceEmpty( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID );
extern void RESORT_DATA_MANAGER_GetPlaceData( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID, RESORT_DATA_MANAGER_TYPE *p_type, u16 *p_data_idx );
extern u32 RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID );
