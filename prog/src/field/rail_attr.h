//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		rail_attr.h
 *	@brief  レール　アトリビュート  
 *	@author	tomoya takahashi
 *	@date		2009.08.21
 *
 *	モジュール名：RAIL_ATTR
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "map_attr.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	RAIL_ATTR_DATA
//=====================================
typedef struct _RAIL_ATTR_DATA RAIL_ATTR_DATA;    


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// レールアトリビュートデータの生成破棄
extern RAIL_ATTR_DATA* RAIL_ATTR_Create( u32 heapID );
extern void RAIL_ATTR_Delete( RAIL_ATTR_DATA* p_work );

// データ読み込み、リリース
extern void RAIL_ATTR_Load( RAIL_ATTR_DATA* p_work, u32 datano, u32 heapID );
extern void RAIL_ATTR_Release( RAIL_ATTR_DATA* p_work );
extern BOOL RAIL_ATTR_IsLoadData( const RAIL_ATTR_DATA* cp_work );


// レールのアトリビュート取得
extern MAPATTR RAIL_ATTR_GetAttribute( const RAIL_ATTR_DATA* cp_work, const RAIL_LOCATION* cp_location );


// レールマップ用アトリビュートVALUEチェック
extern BOOL RAIL_ATTR_VALUE_CheckSlipDown( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckHybridBaseSystemChange( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckIceSpinL( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckIceSpinR( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckIceJumpL( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckIceJumpR( const MAPATTR_VALUE val );
extern BOOL RAIL_ATTR_VALUE_CheckIceTurnR( const MAPATTR_VALUE val );


#ifdef PM_DEBUG
extern void RAIL_ATTR_DEBUG_LoadBynary( RAIL_ATTR_DATA* p_work, void* p_data, u32 datasize, u32 heapID );
extern const void* RAIL_ATTR_DEBUG_GetData( const RAIL_ATTR_DATA* cp_work );
extern u32 RAIL_ATTR_DEBUG_GetDataSize( const RAIL_ATTR_DATA* cp_work );
#endif

#ifdef _cplusplus
}	// extern "C"{
#endif



