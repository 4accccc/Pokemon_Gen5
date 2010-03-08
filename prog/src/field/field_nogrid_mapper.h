//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_nogrid_mapper.h
 *	@brief  ノーグリッド動作　マッパー
 *	@author	tomoya takahashi
 *	@date		2009.08.25
 *
 *	モジュール名：FLDNOGRID_MAPPER
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field_rail.h"
#include "field/field_rail_loader.h"
#include "fld_scenearea.h"
#include "fld_scenearea_loader.h"
#include "rail_attr.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
/// ノーグリッドマップ　情報	不定値
//=====================================
#define FLDNOGRID_RESISTDATA_NONE  ( 0xffff )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ノーグリッド動作マッパーワーク
//=====================================
typedef struct _FLDNOGRID_MAPPER FLDNOGRID_MAPPER;



//-------------------------------------
/// ノーグリッドマップ　情報	
//  読みこまない場合  ：FLDNOGRID_RESISTDATA_NONE  
//=====================================
typedef struct 
{
  u16 railDataID;
  u16 areaDataID;
} FLDNOGRID_RESISTDATA;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// マッパー生成・破棄
extern FLDNOGRID_MAPPER* FLDNOGRID_MAPPER_Create( u32 heapID, FIELD_CAMERA* p_camera, FLD_SCENEAREA* p_sceneArea, FLD_SCENEAREA_LOADER* p_sceneAreaLoader );
extern void FLDNOGRID_MAPPER_Delete( FLDNOGRID_MAPPER* p_mapper );


// ノーグリッドマップ処理の更新を制御
// 全体の更新をストップ
extern void FLDNOGRID_MAPPER_SetActive( FLDNOGRID_MAPPER* p_mapper, BOOL flag );
extern BOOL FLDNOGRID_MAPPER_IsActive( const FLDNOGRID_MAPPER* cp_mapper );
// カメラのみストップ
extern void FLDNOGRID_MAPPER_SetRailCameraActive( FLDNOGRID_MAPPER* p_mapper, BOOL flag );

// 情報の設定、破棄
extern void FLDNOGRID_MAPPER_ResistData( FLDNOGRID_MAPPER* p_mapper, const FLDNOGRID_RESISTDATA* cp_data, u32 heapID );
extern void FLDNOGRID_MAPPER_ResistDataArc( FLDNOGRID_MAPPER* p_mapper, u32 dataID, u32 heapID );
extern void FLDNOGRID_MAPPER_Release( FLDNOGRID_MAPPER* p_mapper );
extern BOOL FLDNOGRID_MAPPER_IsResistData( const FLDNOGRID_MAPPER* cp_mapper );


// メイン管理
extern void FLDNOGRID_MAPPER_Main( FLDNOGRID_MAPPER* p_mapper );

// カメラの反映処理のみ行う
extern void FLDNOGRID_MAPPER_UpdateCamera( FLDNOGRID_MAPPER* p_mapper );

// レールワークの取得・返却
extern FIELD_RAIL_WORK* FLDNOGRID_MAPPER_CreateRailWork( FLDNOGRID_MAPPER* p_mapper );
extern void FLDNOGRID_MAPPER_DeleteRailWork( FLDNOGRID_MAPPER* p_mapper, FIELD_RAIL_WORK* p_railWork );

// メインレールワークの設定
extern void FLDNOGRID_MAPPER_BindCameraWork( FLDNOGRID_MAPPER* p_mapper, const FIELD_RAIL_WORK* cp_railWork );
extern void FLDNOGRID_MAPPER_UnBindCameraWork( FLDNOGRID_MAPPER* p_mapper );


// アトリビュート情報の取得
extern MAPATTR FLDNOGRID_MAPPER_GetAttr( const FLDNOGRID_MAPPER* cp_mapper, const RAIL_LOCATION* cp_location );


// RAIL_LOCATIONでのカメラアングルをFIELD_CAEMRAに設定する
extern void FLDNOGRID_MAPPER_SetUpLocationCamera( FLDNOGRID_MAPPER* p_mapper, const RAIL_LOCATION* cp_location );


// レールライン分岐の管理
// flag TRUE:分岐可能　　FALSE：分岐不可能
extern void FLDNOGRID_MAPPER_SetLineActive( FLDNOGRID_MAPPER* p_mapper, u32 line_index, BOOL flag );
extern BOOL FLDNOGRID_MAPPER_GetLineActive( const FLDNOGRID_MAPPER* cp_mapper, u32 line_index );


// 各モジュール取得
extern const FIELD_RAIL_MAN* FLDNOGRID_MAPPER_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper );
extern const FLD_SCENEAREA* FLDNOGRID_MAPPER_GetSceneAreaMan( const FLDNOGRID_MAPPER* cp_mapper );
extern const FIELD_RAIL_LOADER* FLDNOGRID_MAPPER_GetRailLoader( const FLDNOGRID_MAPPER* cp_mapper );
extern const FLD_SCENEAREA_LOADER* FLDNOGRID_MAPPER_GetSceneAreaLoader( const FLDNOGRID_MAPPER* cp_mapper );
extern const RAIL_ATTR_DATA* FLDNOGRID_MAPPER_GetRailAttrData( const FLDNOGRID_MAPPER* cp_mapper );


// デバック用
#ifdef PM_DEBUG
extern void FLDNOGRID_MAPPER_DEBUG_LoadRailBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID );
extern void FLDNOGRID_MAPPER_DEBUG_LoadAreaBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID );
extern void FLDNOGRID_MAPPER_DEBUG_LoadAttrBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID );


extern FIELD_RAIL_MAN* FLDNOGRID_MAPPER_DEBUG_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper );

#endif

#ifdef _cplusplus
}	// extern "C"{
#endif



