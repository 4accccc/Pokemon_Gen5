//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldmap_ctrl_hybrid.h
 *	@brief  マップコントロール　ハイブリッド
 *	@author	tomoya takahashi
 *	@date		2009.10.27
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "fieldmap.h"

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
///	FIELDMAP_CTRL_HYBRIDワーク
//=====================================
typedef struct _FIELDMAP_CTRL_HYBRID FIELDMAP_CTRL_HYBRID;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern const DEPEND_FUNCTIONS FieldMapCtrl_HybridFunctions;



// 情報の取得
extern FLDMAP_BASESYS_TYPE FIELDMAP_CTRL_HYBRID_GetBaseSystemType( const FIELDMAP_CTRL_HYBRID* cp_wk );

// その場で、レールとグリッドを切り替える
extern void FIELDMAP_CTRL_HYBRID_ChangeBaseSystem( FIELDMAP_CTRL_HYBRID* p_wk, FIELDMAP_WORK* p_fieldmap );

#ifdef _cplusplus
}	// extern "C"{
#endif



