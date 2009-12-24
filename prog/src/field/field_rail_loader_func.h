//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_rail_loader_func.h
 *	@brief	フィールドレールローダー　関数ID
 *	@author	tomoya takahashi
 *	@date		2009.07.07
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


//-----------------------------------------------------------------------------
/**
 *					カメラ関数ID
*/
//-----------------------------------------------------------------------------
typedef enum
{
	// rail_conv.pl用コメント　修正しないで。
	//CAMERA_FUNC_ID_START
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXPOS,				// 固定位置カメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE,			// 固定アングルカメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_OFSANGLE,			// オフセットアングルカメラ	（ライン専用）
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXALL,				// 完全固定カメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXALL_LINE,	// 完全固定カメラ線形補間（ライン専用）
	FIELD_RAIL_LOADER_CAMERA_FUNC_CIRCLE,				// 円動作カメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXLENCIRCLE,	// 距離固定円動作カメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_TARGETCHANGE_CIRCLE,	// 距離固定円動作ターゲット変更カメラ
	FIELD_RAIL_LOADER_CAMERA_FUNC_PLT_CIRCLE,				// プレイヤーターゲット円動作カメラ 

	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE_LINEWAY,	// 進行方向に対するアングルを固定
	FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE_LINEWAY_XYZ,	// 進行方向に対するアングルを固定

	FIELD_RAIL_LOADER_CAMERA_FUNC_MAX,	// 最大値
} FIELD_RAIL_LOADER_CAMERA_FUNC;

//-----------------------------------------------------------------------------
/**
 *					座標計算関数ID
*/
//-----------------------------------------------------------------------------
typedef enum
{
	// rail_conv.pl用コメント　修正しないで。
	//LINEPOS_FUNC_ID_START
	FIELD_RAIL_LOADER_LINEPOS_FUNC_STRAIT,			// 直線
	FIELD_RAIL_LOADER_LINEPOS_FUNC_CURVE,				// カーブ
	FIELD_RAIL_LOADER_LINEPOS_FUNC_YCURVE,		  // Yカーブ

	FIELD_RAIL_LOADER_LINEPOS_FUNC_MAX,				// 最大値
} FIELD_RAIL_LOADER_LINEPOS_FUNC;


//-----------------------------------------------------------------------------
/**
 *					距離計算関数ID
*/
//-----------------------------------------------------------------------------
typedef enum
{
	// rail_conv.pl用コメント　修正しないで。
	//LINEDIST_FUNC_ID_START
	FIELD_RAIL_LOADER_LINEDIST_FUNC_STRAIT,			// 直線
	FIELD_RAIL_LOADER_LINEDIST_FUNC_CURVE,		  // カーブ
	FIELD_RAIL_LOADER_LINEDIST_FUNC_YCURVE,		// Yカーブ

	FIELD_RAIL_LOADER_LINEDIST_FUNC_MAX,				// 最大値
} FIELD_RAIL_LOADER_LINEDIST_FUNC;


#ifdef _cplusplus
}	// extern "C"{
#endif



