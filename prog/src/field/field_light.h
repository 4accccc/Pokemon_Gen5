//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_light.h
 *	@brief		フィールドライトシステム
 *	@author		tomoya takahashi
 *	@data		2009.03.24
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
extern FIELD_LIGHT* FIELD_LIGHT_Create( u32 light_no, u32 season, int rtc_second, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight, u32 heapID );
extern void FIELD_LIGHT_Delete( FIELD_LIGHT* p_sys );

//-------------------------------------
///	システムメイン
//=====================================
extern void FIELD_LIGHT_Main( FIELD_LIGHT* p_sys, int rtc_second );


//-------------------------------------
///	ライト情報の変更
//=====================================
extern void FIELD_LIGHT_Change( FIELD_LIGHT* p_sys, u32 light_no, u32 season, u32 heapID );
extern void FIELD_LIGHT_ChangeEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID );

// FIELD_LIGHT_Create or FIELD_LIGHT_Changeで設定したlight_no seasonのライト情報に戻す
extern void FIELD_LIGHT_ReLoadDefault( FIELD_LIGHT* p_sys, u32 heapID );

//-------------------------------------
///	ライト情報反映のON・OFF
//=====================================
extern void FIELD_LIGHT_SetReflect( FIELD_LIGHT* p_sys, BOOL flag );
extern BOOL FIELD_LIGHT_GetReflect( const FIELD_LIGHT* cp_sys );


//-------------------------------------
///	状態の取得
//=====================================
extern BOOL FIELD_LIGHT_GetNight( const FIELD_LIGHT* cp_sys );





#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __FIELD_LIGHT_H__


