//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_work_func.h
 *	@brief	フィールドレール　基本関数郡
 *	@author	tomoya takahashi
 *	@date		2009.07.06
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field_rail.h"

//-----------------------------------------------------------------------------
/**
 *					位置移動
*/
//-----------------------------------------------------------------------------
//------------------------------------------------------------------
//  直線
//------------------------------------------------------------------
extern void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL_WORK * work, VecFx32 * pos);

//------------------------------------------------------------------
//  カーブ
//------------------------------------------------------------------
typedef struct
{
	// 中心座標
	fx32 x;
	fx32 y;
	fx32 z;
} RAIL_POSFUNC_CURVE_WORK;
extern void FIELD_RAIL_POSFUNC_CurveLine(const FIELD_RAIL_WORK * work, VecFx32 * pos);
extern void FIELD_RAIL_POSFUNC_YCurveLine(const FIELD_RAIL_WORK * work, VecFx32 * pos);


//-----------------------------------------------------------------------------
/**
 *					ロケーションあたり判定
*/
//-----------------------------------------------------------------------------
//------------------------------------------------------------------
//  直線
//------------------------------------------------------------------
extern BOOL FIELD_RAIL_LINE_HIT_LOCATION_FUNC_StraitLine( u32 rail_index, const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_check_pos, RAIL_LOCATION* p_location, VecFx32* p_pos );

//------------------------------------------------------------------
//  カーブ
//	FIELD_RAIL_POSFUNC_CurveLine専用
//------------------------------------------------------------------
extern BOOL FIELD_RAIL_LINE_HIT_LOCATION_FUNC_CircleLine( u32 rail_index, const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_check_pos, RAIL_LOCATION* p_location, VecFx32* p_pos );

//-----------------------------------------------------------------------------
/**
 *					カメラ
*/
//-----------------------------------------------------------------------------
//------------------------------------------------------------------
//  固定位置カメラ
//------------------------------------------------------------------
typedef struct
{
	// カメラ座標
	fx32 x;
	fx32 y;
	fx32 z;
} RAIL_CAMERAFUNC_FIXPOS_WORK;
extern void FIELD_RAIL_CAMERAFUNC_FixPosCamera(const FIELD_RAIL_MAN* man);

//------------------------------------------------------------------
//  固定アングルカメラ
//------------------------------------------------------------------
typedef struct
{
	// カメラ座標
	u32	pitch;
	u32 yaw;
	u32 len;
} RAIL_CAMERAFUNC_FIXANGLE_WORK;
extern void FIELD_RAIL_CAMERAFUNC_FixAngleCamera(const FIELD_RAIL_MAN* man);

//------------------------------------------------------------------
//  オフセットアングルカメラ	（ライン専用）
//  開始ポイント - 終了ポイント　は　FixAngleCameraである必要があります。
//------------------------------------------------------------------
extern void FIELD_RAIL_CAMERAFUNC_OfsAngleCamera(const FIELD_RAIL_MAN* man);

//------------------------------------------------------------------
//  完全固定カメラ
//------------------------------------------------------------------
typedef struct
{
	// カメラ座標
	fx32 pos_x;
	fx32 pos_y;
	fx32 pos_z;

	// ターゲット座標
	fx32 target_x;
	fx32 target_y;
	fx32 target_z;
} RAIL_CAMERAFUNC_FIXALL_WORK;
extern void FIELD_RAIL_CAMERAFUNC_FixAllCamera(const FIELD_RAIL_MAN* man);


//------------------------------------------------------------------
//  完全固定間の線形補間カメラ
//------------------------------------------------------------------
extern void FIELD_RAIL_CAMERAFUNC_FixAllLineCamera(const FIELD_RAIL_MAN* man);

//------------------------------------------------------------------
//  円動作カメラ
//------------------------------------------------------------------
typedef struct
{
	u32		pitch;
	fx32	len;
	
	fx32 target_x;
	fx32 target_y;
	fx32 target_z;
} RAIL_CAMERAFUNC_CIRCLE_WORK;
extern void FIELD_RAIL_CAMERAFUNC_CircleCamera( const FIELD_RAIL_MAN * man );

//------------------------------------------------------------------
//  距離固定円動作カメラ
//------------------------------------------------------------------
typedef struct
{
	u32		pitch;
	fx32	len;
	
	fx32 target_x;
	fx32 target_y;
	fx32 target_z;
} RAIL_CAMERAFUNC_FIXLEN_CIRCLE_WORK;
extern void FIELD_RAIL_CAMERAFUNC_FixLenCircleCamera( const FIELD_RAIL_MAN * man );


//------------------------------------------------------------------
//  ライン、ターゲット変更 円動作カメラ
//------------------------------------------------------------------
typedef struct
{
	u32		pitch;
	fx32	len;
  
	fx32 target_sx;
	fx32 target_sy;
	fx32 target_sz;

	fx32 target_ex;
	fx32 target_ey;
	fx32 target_ez;
} RAIL_CAMERAFUNC_TARGETCHANGE_CIRCLE_WORK;
extern void FIELD_RAIL_CAMERAFUNC_TargetChangeCircleCamera( const FIELD_RAIL_MAN * man );


//------------------------------------------------------------------
//  Ｐｌａｙｅｒターゲット 円動作カメラ
//------------------------------------------------------------------
typedef struct
{
	u32		pitch;
	fx32	len;
  
	fx32 center_x;
	fx32 center_z;
} RAIL_CAMERAFUNC_PLAYERTARGET_CIRCLE_WORK;
extern void FIELD_RAIL_CAMERAFUNC_PlayerTargetCircleCamera( const FIELD_RAIL_MAN * man );

//------------------------------------------------------------------
//  進行方向に対するアングルを固定
//------------------------------------------------------------------
typedef struct
{
  u32 yaw;
	u32 pitch;
	fx32  len;
} RAIL_CAMERAFUNC_FIXANGLE_LINEWAY;
extern void FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XZ( const FIELD_RAIL_MAN * man );
extern void FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XYZ( const FIELD_RAIL_MAN * man );


#ifdef _cplusplus
}	// extern "C"{
#endif



