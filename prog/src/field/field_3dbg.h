//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_3dbg.h
 *	@brief		フィールド　３ＤＢＧ面
 *	@author		tomoya takahshi
 *	@date		2009.03.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __FIELD_3DBG_H__
#define __FIELD_3DBG_H__

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

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
///	フィールド３ＤＢＧシステム
//=====================================
typedef struct _FIELD_3DBG FIELD_3DBG;


//-------------------------------------
///	フィールド３ＤＢＧシステム
//	初期化関数
//=====================================
typedef struct {
	u16		size_x;			// ピクセル単位のＢＧ０に描画するさいの面サイズ	
	u16		size_y;			// ピクセル単位のＢＧ０に描画するさいの面サイズ
	fx32	camera_dist;	// 面を配置するカメラ（ニア面）からの距離
	u8		polygon_id;		// ポリゴンID

	u8		pad[3];
} FIELD_3DBG_SETUP;

//-------------------------------------
///	描画データ
//=====================================
typedef struct {
	u16		nsbtex_id;
	u8		texsiz_s;		// GXTexSizeS
	u8		texsiz_t;		// GXTexSizeT
	u8		repeat;			// GXTexRepeat
	u8		flip;			// GXTexFlip
	u8		texfmt;			// GXTexFmt
	u8		texpltt;		// GXTexPlttColor0
	u8		alpha;			// アルファ設定

	u8		pad[3];
} FIELD_3DBG_WRITE_DATA;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	システム管理
//=====================================
extern FIELD_3DBG* FIELD_3DBG_Create( const FIELD_3DBG_SETUP* cp_setup, u32 heapID );
extern void FIELD_3DBG_Delete( FIELD_3DBG* p_sys );
extern void FIELD_3DBG_Write( FIELD_3DBG* p_sys );
extern void FIELD_3DBG_VBlank( FIELD_3DBG* p_sys );


//-------------------------------------
///	BGリソースの設定
//=====================================
extern void FIELD_3DBG_ClearWriteData( FIELD_3DBG* p_sys );
extern void FIELD_3DBG_SetWriteData( FIELD_3DBG* p_sys, ARCHANDLE* p_handle, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID );


//-------------------------------------
///	情報の設定・取得
//=====================================
// ON/OFF
extern void FIELD_3DBG_SetVisible( FIELD_3DBG* p_sys, BOOL flag );
extern BOOL FIELD_3DBG_GetVisible( const FIELD_3DBG* cp_sys );

// ピクセル単位のＢＧ０に描画するさいの面サイズ
extern void FIELD_3DBG_SetScreenSizeX( FIELD_3DBG* p_sys, u16 size_x );
extern u16 FIELD_3DBG_GetScreenSizeX( const FIELD_3DBG* cp_sys );
extern void FIELD_3DBG_SetScreenSizeY( FIELD_3DBG* p_sys, u16 size_y );
extern u16 FIELD_3DBG_GetScreenSizeY( const FIELD_3DBG* cp_sys );

// 面を配置するカメラ（ニア面）からの距離
extern void FIELD_3DBG_SetCameraDist( FIELD_3DBG* p_sys, fx32 dist );
extern fx32 FIELD_3DBG_GetCameraDist( const FIELD_3DBG* cp_sys );

// ポリゴンID
extern void FIELD_3DBG_SetPolygonID( FIELD_3DBG* p_sys, u8 polygon_id );
extern u8 FIELD_3DBG_GetPolygonID( const FIELD_3DBG* cp_sys );

// テクスチャリピート設定
extern void FIELD_3DBG_SetRepeat( FIELD_3DBG* p_sys, GXTexRepeat repeat );
extern GXTexRepeat FIELD_3DBG_GetRepeat( const FIELD_3DBG* cp_sys );

// テクスチャフリップ設定
extern void FIELD_3DBG_SetFlip( FIELD_3DBG* p_sys, GXTexFlip flip );
extern GXTexFlip FIELD_3DBG_GetFlip( const FIELD_3DBG* cp_sys );

// スクロール設定
extern void FIELD_3DBG_SetScrollX( FIELD_3DBG* p_sys, s32 x );
extern s32 FIELD_3DBG_GetScrollX( const FIELD_3DBG* cp_sys );
extern void FIELD_3DBG_SetScrollY( FIELD_3DBG* p_sys, s32 y );
extern s32 FIELD_3DBG_GetScrollY( const FIELD_3DBG* cp_sys );

// 回転
extern void FIELD_3DBG_SetRotate( FIELD_3DBG* p_sys, u16 rotate );
extern u16 FIELD_3DBG_GetRotate( const FIELD_3DBG* cp_sys );

// 拡大
extern void FIELD_3DBG_SetScaleX( FIELD_3DBG* p_sys, fx32 x );
extern fx32 FIELD_3DBG_GetScaleX( const FIELD_3DBG* cp_sys );
extern void FIELD_3DBG_SetScaleY( FIELD_3DBG* p_sys, fx32 y );
extern fx32 FIELD_3DBG_GetScaleY( const FIELD_3DBG* cp_sys );


// アルファ設定
// *使用には注意が必要です。
// 半透明ポリゴンがフィールドに描画されている場所では
// 半透明にしないでください。
// 重なり方がおかしくなります。
extern void FIELD_3DBG_SetAlpha( FIELD_3DBG* p_sys, u8 alpha );
extern u8 FIELD_3DBG_GetAlpha( const FIELD_3DBG* cp_sys );

#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __FIELD_3DBG_H__


