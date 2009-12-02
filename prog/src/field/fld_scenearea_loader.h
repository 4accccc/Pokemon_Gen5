//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_scenearea_loader.h
 *	@brief  フィールド　特殊シーン領域外部データ読み込み
 *	@author	tomoya takahashi
 *	@date		2009.07.09
 *
 *	モジュール名：FLD_SCENEAREA_LOADER
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "fld_scenearea.h"

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
///	読み込みシステム
//=====================================
typedef struct _FLD_SCENEAREA_LOADER FLD_SCENEAREA_LOADER;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//生成　破棄
extern FLD_SCENEAREA_LOADER* FLD_SCENEAREA_LOADER_Create( HEAPID heapID );
extern void FLD_SCENEAREA_LOADER_Delete( FLD_SCENEAREA_LOADER* p_sys );


// 読み込み、クリア
extern void FLD_SCENEAREA_LOADER_Load( FLD_SCENEAREA_LOADER* p_sys, u32 datano, HEAPID heapID );
extern void FLD_SCENEAREA_LOADER_LoadOriginal( FLD_SCENEAREA_LOADER* p_sys, u32 arcID, u32 datano, HEAPID heapID );
extern void FLD_SCENEAREA_LOADER_Clear( FLD_SCENEAREA_LOADER* p_sys );

// 情報の取得
extern const FLD_SCENEAREA_DATA* FLD_SCENEAREA_LOADER_GetData( const FLD_SCENEAREA_LOADER* cp_sys );
extern u32 FLD_SCENEAREA_LOADER_GetDataNum( const FLD_SCENEAREA_LOADER* cp_sys );
extern const FLD_SCENEAREA_FUNC* FLD_SCENEAREA_LOADER_GetFunc( const FLD_SCENEAREA_LOADER* cp_sys );

// 指定エリアの情報を取得

// デバック機能
#ifdef PM_DEBUG
extern void FLD_SCENEAREA_LOADER_LoadBinary( FLD_SCENEAREA_LOADER* p_sys, void* p_dat, u32 size );
extern void* FLD_SCENEAREA_LOADER_DEBUG_GetData( const FLD_SCENEAREA_LOADER* cp_sys );
extern u32 FLD_SCENEAREA_LOADER_DEBUG_GetDataSize( const FLD_SCENEAREA_LOADER* cp_sys );
#endif

#ifdef _cplusplus
}	// extern "C"{
#endif



