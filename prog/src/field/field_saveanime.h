//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_saveaime.h
 *	@brief  セーブアニメ    VBlankを使用したBGアニメ
 *	@author	tomoya takahashi
 *	@date		2010.02.26
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
///	FIELD_SAVEANIME
//=====================================
typedef struct _FIELD_SAVEANIME FIELD_SAVEANIME;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	FIELD_SAVEANIME
//=====================================
//  アニメーション生成・破棄
extern FIELD_SAVEANIME* FIELD_SAVEANIME_Create( HEAPID heapID, FIELDMAP_WORK* p_fieldmap );
extern void FIELD_SAVEANIME_Delete( FIELD_SAVEANIME* p_wk );

// アニメーション開始
extern void FIELD_SAVEANIME_Start( FIELD_SAVEANIME* p_wk );

// アニメーション停止
extern void FIELD_SAVEANIME_End( FIELD_SAVEANIME* p_wk );




#ifdef _cplusplus
}	// extern "C"{
#endif



