//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		land_data_patch.h
 *	@brief  マップの基本的なアトリビュート、配置データを書き換え、追加設定
 *	@author	tomoya takahashi
 *	@date		2009.11.30
 *
 *	モジュール名：FIELD_LAND_DATA_PATCH
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "arc/fieldmap/land_data_patch_def.h"

#include "map/dp3format.h"

#include "field_g3d_map.h"
#include "field_buildmodel.h"

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
///	  LAND_DATAパッチ情報
//=====================================
typedef struct _FIELD_DATA_PATCH FIELD_DATA_PATCH;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成・破棄
extern FIELD_DATA_PATCH* FIELD_DATA_PATCH_Create( u32 data_id, HEAPID heapID );
extern void FIELD_DATA_PATCH_Delete( FIELD_DATA_PATCH* p_sys );

// アトリビュート上書き
extern void FIELD_DATA_PATCH_OverWriteAttr( const FIELD_DATA_PATCH* cp_sys, NormalVtxFormat* p_buff, u32 grid_x, u32 grid_z );
extern void FIELD_DATA_PATCH_OverWriteAttrEx( const FIELD_DATA_PATCH* cp_sys, NormalVtxFormat* p_buff, u32 read_grid_x, u32 read_grid_z, u32 write_grid_x, u32 write_grid_z, u32 size_grid_x, u32 size_grid_z );

// 配置モデル設定
extern int FIELD_LAND_DATA_PATCH_AddBuildModel( const FIELD_DATA_PATCH* cp_sys, FIELD_BMODEL_MAN * p_bmodel, FLD_G3D_MAP * g3Dmap, int count_start, u32 grid_x, u32 grid_z );

#ifdef _cplusplus
}	// extern "C"{
#endif



