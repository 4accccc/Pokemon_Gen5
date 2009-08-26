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
//=====================================
typedef struct 
{
  u16 railDataID;
  u16 areaDataID;
  u16 attrDataID;

  u16 pad;
} FLDNOGRID_RESISTDATA;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// マッパー生成・破棄
extern FLDNOGRID_MAPPER* FLDNOGRID_MAPPER_Create( u32 heapID, FIELD_CAMERA* p_camera );
extern void FLDNOGRID_MAPPER_Delete( FLDNOGRID_MAPPER* p_mapper );


// 情報の設定、破棄
extern void FLDNOGRID_MAPPER_ResistData( FLDNOGRID_MAPPER* p_mapper, const FLDNOGRID_RESISTDATA* cp_data, u32 heapID );
extern void FLDNOGRID_MAPPER_Release( FLDNOGRID_MAPPER* p_mapper );


// メイン管理
extern void FLDNOGRID_MAPPER_Main( FLDNOGRID_MAPPER* p_mapper );


// レールワークの取得・返却
extern FIELD_RAIL_WORK* FLDNOGRID_MAPPER_CreateRailWork( FLDNOGRID_MAPPER* p_mapper );
extern void FLDNOGRID_MAPPER_DeleteRailWork( FLDNOGRID_MAPPER* p_mapper, FIELD_RAIL_WORK* p_railWork );


// アトリビュート情報の取得
extern MAPATTR FLDNOGRID_MAPPER_GetAttr( const FLDNOGRID_MAPPER* cp_mapper, const RAIL_LOCATION* cp_location );


// 各モジュール取得
extern const FIELD_RAIL_MAN* FLDNOGRID_MAPPER_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper );
extern const FLD_SCENEAREA* FLDNOGRID_MAPPER_GetSceneAreaMan( const FLDNOGRID_MAPPER* cp_mapper );
extern const FIELD_RAIL_LOADER* FLDNOGRID_MAPPER_GetRailLoader( const FLDNOGRID_MAPPER* cp_mapper );
extern const FLD_SCENEAREA_LOADER* FLDNOGRID_MAPPER_GetSceneAreaLoader( const FLDNOGRID_MAPPER* cp_mapper );



#ifdef _cplusplus
}	// extern "C"{
#endif



