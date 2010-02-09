//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldmap_ctrl_nogrid_work.h
 *	@brief  フィールドマップコントロール　ノーグリッド　共通ワーク
 *	@author	tomoya takahashi
 *	@date		2009.08.28
 *
 *	モジュール名：FIELDMAP_CTRL_NOGRID_WORK
 *
 *	ノーグリッドマップの　マップコントロールでは、このワークを確保するようにしてください。
 *
 *	
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

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
///	フィールドマップコントロール　ノーグリッドマップ　共通ワーク
//=====================================
typedef struct _FIELDMAP_CTRL_NOGRID_WORK FIELDMAP_CTRL_NOGRID_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// メモリ確保・破棄
extern FIELDMAP_CTRL_NOGRID_WORK* FIELDMAP_CTRL_NOGRID_WORK_Create( FIELDMAP_WORK* p_fieldwork, HEAPID heapID );
extern void FIELDMAP_CTRL_NOGRID_WORK_Delete( FIELDMAP_CTRL_NOGRID_WORK* p_wk );

// 動作
extern void FIELDMAP_CTRL_NOGRID_WORK_Main( FIELDMAP_CTRL_NOGRID_WORK* p_wk );

// ローカルワークの確保・破棄・確保メモリの取得
extern void* FIELDMAP_CTRL_NOGRID_WORK_AllocLocalWork( FIELDMAP_CTRL_NOGRID_WORK* p_wk, HEAPID heapID, u32 size );
extern void FIELDMAP_CTRL_NOGRID_WORK_FreeLocalWork( FIELDMAP_CTRL_NOGRID_WORK* p_wk );
extern void* FIELDMAP_CTRL_NOGRID_WORK_GetLocalWork( const FIELDMAP_CTRL_NOGRID_WORK* cp_wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



