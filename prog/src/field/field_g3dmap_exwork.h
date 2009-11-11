//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_g3dmap_exwork.h
 *	@brief	field_g3d_mapperのGFL_G3D_MAP拡張ワーク管理処理
 *	@author	tomoya takahashi
 *	@date		2009.05.01
 *
 *	モジュール名：FLD_G3D_MAP_EXWORK
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "field_ground_anime.h"
#include "field_buildmodel.h"
#include "field_wfbc.h"

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
///	GFL_G3D_MAP拡張ワーク
//=====================================
typedef struct _FLD_G3D_MAP_EXWORK FLD_G3D_MAP_EXWORK;

	
//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 拡張ワーク操作
extern BOOL FLD_G3D_MAP_EXWORK_IsGranm( const FLD_G3D_MAP_EXWORK* cp_wk );
extern FIELD_GRANM_WORK* FLD_G3D_MAP_EXWORK_GetGranmWork( const FLD_G3D_MAP_EXWORK* cp_wk );

// 配置モデル制御
extern FIELD_BMODEL_MAN* FLD_G3D_MAP_EXWORK_GetBModelMan( const FLD_G3D_MAP_EXWORK* cp_wk );

// WFBC街情報
extern FIELD_WFBC* FLD_G3D_MAP_EXWORK_GetWFBCWork( const FLD_G3D_MAP_EXWORK* cp_wk );

// マップインデックス取得
const u32 FLD_G3D_MAP_EXWORK_GetMapIndex( const FLD_G3D_MAP_EXWORK* cp_wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



