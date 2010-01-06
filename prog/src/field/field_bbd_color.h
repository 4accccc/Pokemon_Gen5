//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_bbd_color.h
 *	@brief  ビルボード　カラー　データ
 *	@author	tomoya takahashi
 *	@date		2010.01.05
 *
 *	モジュール名：FLD_BBD_COLOR
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gflib.h"






#ifdef PM_DEBUG

#define DEBUG_BBD_COLOR_CONTROL   // デバックコントロールを有効にする

#endif

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
///	ビルボードカラー設定システム
//=====================================
typedef struct _FLD_BBD_COLOR FLD_BBD_COLOR;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成・破棄
extern FLD_BBD_COLOR* FLD_BBD_COLOR_Create( HEAPID heapID ); 
extern void FLD_BBD_COLOR_Delete( FLD_BBD_COLOR* p_wk );

// 読み込み処理
extern void FLD_BBD_COLOR_Load( FLD_BBD_COLOR* p_wk, u32 idx );

// 設定
extern void FLD_BBD_COLOR_SetData( const FLD_BBD_COLOR* cp_wk, GFL_BBD_SYS* p_bbdsys );


// デバック機能
#ifdef DEBUG_BBD_COLOR_CONTROL

extern void FLD_BBD_COLOR_DEBUG_Init( GFL_BBD_SYS* bbd_sys, const FLD_BBD_COLOR* cp_data, HEAPID heapID );
extern void FLD_BBD_COLOR_DEBUG_Exit( void );

extern void FLD_BBD_COLOR_DEBUG_Control( void );
extern void FLD_BBD_COLOR_DEBUG_PrintData( GFL_BMPWIN* p_win );

#else

#define FLD_BBD_COLOR_DEBUG_Init( a, b, c )		((void)0)
#define FLD_BBD_COLOR_DEBUG_Exit()			((void)0)
#define FLD_BBD_COLOR_DEBUG_Control()		((void)0)
#define FLD_BBD_COLOR_DEBUG_PrintData( a )	((void)0)

#endif



#ifdef _cplusplus
}	// extern "C"{
#endif



