//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_light.h
 *	@brief		フィールドライトシステム
 *	@author		tomoya takahashi
 *	@date		2009.03.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __FIELD_LIGHT_H__
#define __FIELD_LIGHT_H__

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "field_fog.h"

#ifdef PM_DEBUG
#define DEBUG_FIELD_LIGHT		// フィールドライトのデバック機能を有効にする
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------



//-------------------------------------
/// ライトフェード
//=====================================
#define LIGHT_FADE_COUNT_MAX  ( 60 )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フィールドライトシステム
//=====================================
typedef struct _FIELD_LIGHT FIELD_LIGHT;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	システム作成・破棄
//=====================================
extern FIELD_LIGHT* FIELD_LIGHT_Create( u32 light_no, int rtc_second, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight, HEAPID heapID );
extern void FIELD_LIGHT_Delete( FIELD_LIGHT* p_sys );

//-------------------------------------
///	システムメイン
//=====================================
extern void FIELD_LIGHT_Main( FIELD_LIGHT* p_sys, int rtc_second );
extern void FIELD_LIGHT_Reflect( FIELD_LIGHT* p_sys, BOOL force );


//-------------------------------------
///	ライト情報の変更
//=====================================
extern void FIELD_LIGHT_Change( FIELD_LIGHT* p_sys, u32 light_no );
extern void FIELD_LIGHT_ChangeEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, s32 sync );

// FIELD_LIGHT_Create or FIELD_LIGHT_Changeで設定したlight_noのライト情報に戻す
extern void FIELD_LIGHT_ReLoadDefault( FIELD_LIGHT* p_sys );


//-------------------------------------
///	ライトフェード機能
//=====================================
extern void FIELD_LIGHT_COLORFADE_Start( FIELD_LIGHT* p_sys, GXRgb color, u32 insync, u32 outsync );
extern void FIELD_LIGHT_COLORFADE_StartOneWay( FIELD_LIGHT* p_sys, GXRgb color, u32 sync );
extern BOOL FIELD_LIGHT_COLORFADE_IsFade( const FIELD_LIGHT* cp_sys );

//-------------------------------------
///	ライト情報反映のON・OFF
//=====================================
extern void FIELD_LIGHT_SetReflect( FIELD_LIGHT* p_sys, BOOL flag );
extern BOOL FIELD_LIGHT_GetReflect( const FIELD_LIGHT* cp_sys );


//-------------------------------------
///	色の取得
// (読み込んでいるライトデータ（エクセルデータ）を反映するため、フェード途中のカラーなどではない)
//=====================================
extern GXRgb FIELD_LIGHT_GetLightColor( const FIELD_LIGHT* cp_sys, u32 index );
extern GXRgb FIELD_LIGHT_GetDiffuseColor( const FIELD_LIGHT* cp_sys );
extern GXRgb FIELD_LIGHT_GetAmbientColor( const FIELD_LIGHT* cp_sys );
extern GXRgb FIELD_LIGHT_GetSpecularColor( const FIELD_LIGHT* cp_sys );
extern GXRgb FIELD_LIGHT_GetEmissionColor( const FIELD_LIGHT* cp_sys );


#ifdef DEBUG_FIELD_LIGHT

extern void FIELD_LIGHT_DEBUG_Init( FIELD_LIGHT* p_sys, HEAPID heapID );
extern void FIELD_LIGHT_DEBUG_Exit( FIELD_LIGHT* p_sys );

extern void FIELD_LIGHT_DEBUG_Control( FIELD_LIGHT* p_sys );
extern void FIELD_LIGHT_DEBUG_PrintData( FIELD_LIGHT* p_sys, GFL_BMPWIN* p_win );

#else

#define FIELD_LIGHT_DEBUG_Init( a,b )		((void)0)
#define FIELD_LIGHT_DEBUG_Exit( a )			((void)0)
#define FIELD_LIGHT_DEBUG_Control( a )		((void)0)
#define FIELD_LIGHT_DEBUG_PrintData( a,b )	((void)0)

#endif



//-----------------------------------------------------------------------------
/**
 *					ライトデータアクセス
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// ライト１データ
//=====================================
typedef struct {
  u32     endtime;
  u8      light_flag[4];
  GXRgb   light_color[4];
  VecFx16   light_vec[4];

  GXRgb   diffuse;
  GXRgb   ambient;
  GXRgb   specular;
  GXRgb   emission;
  GXRgb   fog_color;
  GXRgb   bg_color;
} LIGHT_DATA;



#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __FIELD_LIGHT_H__


