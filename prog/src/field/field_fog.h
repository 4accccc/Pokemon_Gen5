//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_fog.h
 *	@brief		フォグ管理システム
 *	@author		tomoya takahashi
 *	@date		2009.03.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __FIELD_FOG_H__
#define __FIELD_FOG_H__

#ifdef _cplusplus
extern "C"{
#endif

#include <nitro.h>

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FOG設定の定数
//  NITRO-SDKの定数をリネームしてるだけです
//=====================================
typedef enum{
	FIELD_FOG_BLEND_COLOR_ALPHA	= GX_FOGBLEND_COLOR_ALPHA, 		// ピクセルのカラー値とα値にフォグブレンディング 
	FIELD_FOG_BLEND_ALPHA		= GX_FOGBLEND_ALPHA,			// ピクセルのα値のみにフォグブレンディング 

	FIELD_FOG_BLEND_MAX,	// システム内で使用
} FIELD_FOG_BLEND;

typedef enum{
	FIELD_FOG_SLOPE_0x8000		= GX_FOGSLOPE_0x8000,			// フォグがデプス値で0x8000の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x4000		= GX_FOGSLOPE_0x4000,			// フォグがデプス値で0x4000の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x2000		= GX_FOGSLOPE_0x2000,			// フォグがデプス値で0x2000の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x1000		= GX_FOGSLOPE_0x1000,			// フォグがデプス値で0x1000の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0800		= GX_FOGSLOPE_0x0800,			// フォグがデプス値で0x0800の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0400		= GX_FOGSLOPE_0x0400,			// フォグがデプス値で0x0400の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0200		= GX_FOGSLOPE_0x0200,			// フォグがデプス値で0x0200の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0100		= GX_FOGSLOPE_0x0100,			// フォグがデプス値で0x0100の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0080		= GX_FOGSLOPE_0x0080,			// フォグがデプス値で0x0080の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0040		= GX_FOGSLOPE_0x0040,			// フォグがデプス値で0x0040の幅で段階的に濃くなっていきます 
	FIELD_FOG_SLOPE_0x0020		= GX_FOGSLOPE_0x0020,			// フォグがデプス値で0x0020の幅で段階的に濃くなっていきます 

	FIELD_FOG_SLOPE_MAX,	// システム内で使用
} FIELD_FOG_SLOPE;

#define FIELD_FOG_TBL_MAX		( 32 )	// フォグテーブルの段階数

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	システムワーク
//=====================================
typedef struct _FIELD_FOG_WORK FIELD_FOG_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	システム生成・破棄
//=====================================
extern FIELD_FOG_WORK* FIELD_FOG_Create( HEAPID heapID );
extern void FIELD_FOG_Delete( FIELD_FOG_WORK* p_wk );


//-------------------------------------
///	メイン動作
//=====================================
extern void FIELD_FOG_Main( FIELD_FOG_WORK* p_wk );
extern void FIELD_FOG_Reflect( FIELD_FOG_WORK* p_wk );


//-------------------------------------
///	情報設定・取得
//=====================================
// 設定
extern void FIELD_FOG_SetFlag( FIELD_FOG_WORK* p_wk, BOOL flag );
extern void FIELD_FOG_SetBlendMode( FIELD_FOG_WORK* p_wk, FIELD_FOG_BLEND blendmode );
extern void FIELD_FOG_SetSlope( FIELD_FOG_WORK* p_wk, FIELD_FOG_SLOPE slope );
extern void FIELD_FOG_SetOffset( FIELD_FOG_WORK* p_wk, s32 depth_offset );
extern void FIELD_FOG_SetColorRgb( FIELD_FOG_WORK* p_wk, GXRgb rgb );
extern void FIELD_FOG_SetColorA( FIELD_FOG_WORK* p_wk, u8 a );
extern void FIELD_FOG_SetTbl( FIELD_FOG_WORK* p_wk, u8 index, u8 data );
extern void FIELD_FOG_SetTblAll( FIELD_FOG_WORK* p_wk, const u8* cp_tbl );

// 取得
extern BOOL FIELD_FOG_GetFlag( const FIELD_FOG_WORK* cp_wk );
extern FIELD_FOG_BLEND FIELD_FOG_GetBlendMode( const FIELD_FOG_WORK* cp_wk );
extern FIELD_FOG_SLOPE FIELD_FOG_GetSlope( const FIELD_FOG_WORK* cp_wk );
extern s32 FIELD_FOG_GetOffset( const FIELD_FOG_WORK* cp_wk );
extern GXRgb FIELD_FOG_GetColorRgb( const FIELD_FOG_WORK* cp_wk );
extern u8 FIELD_FOG_GetColorA( const FIELD_FOG_WORK* cp_wk );
extern u8 FIELD_FOG_GetTbl( const FIELD_FOG_WORK* cp_wk, u8 index );

//-------------------------------------
///	基本的なフォグテーブルの生成
// 典型的な１次元のグラフ
//=====================================
extern void FIELD_FOG_TBL_SetUpDefault( FIELD_FOG_WORK* p_wk );

//-------------------------------------
///	フェードリクエスト
//=====================================
extern void FIELD_FOG_FADE_Init( FIELD_FOG_WORK* p_wk, s32 offset_end, FIELD_FOG_SLOPE slope_end, u32 count_max );
extern void FIELD_FOG_FADE_InitEx( FIELD_FOG_WORK* p_wk, s32 offset_start, s32 offset_end, FIELD_FOG_SLOPE slope_start, FIELD_FOG_SLOPE slope_end, u16 count_max  );
extern BOOL FIELD_FOG_FADE_IsFade( const FIELD_FOG_WORK* cp_wk );


#ifdef PM_DEBUG
extern void FIELD_FOG_DEBUG_Init( FIELD_FOG_WORK* p_wk, HEAPID heapID );
extern void FIELD_FOG_DEBUG_Exit( FIELD_FOG_WORK* p_wk );

extern void FIELD_FOG_DEBUG_Control( FIELD_FOG_WORK* p_wk );
extern void FIELD_FOG_DEBUG_PrintData( FIELD_FOG_WORK* p_wk, GFL_BMPWIN* p_win );
#endif


#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __FIELD_FOG_H__


