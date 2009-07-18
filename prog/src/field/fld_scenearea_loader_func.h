//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_scenearea_loader_func.h
 *	@brief	シーンエリア　ローダー　関数ID
 *	@author	tomoya takahashi
 *	@date		2009.07.09
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
 *					範囲チェック関数
*/
//-----------------------------------------------------------------------------
enum
{
	// scenearea_conv.pl用コメント　修正しないで。
	//AREACHECK_FUNC_ID_START
	FLD_SCENEAREA_AREACHECK_CIRCLE,

	FLD_SCENEAREA_AREACHECK_MAX,
};

//-----------------------------------------------------------------------------
/**
 *					情報更新関数
*/
//-----------------------------------------------------------------------------
enum
{
	// scenearea_conv.pl用コメント　修正しないで。
	//UPDATE_FUNC_ID_START
	FLD_SCENEAREA_UPDATE_CIRCLE,
	FLD_SCENEAREA_UPDATE_FXCAMERA,

	FLD_SCENEAREA_UPDATE_MAX,
};


//-----------------------------------------------------------------------------
/**
 *					パラメータ情報
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// 回転移動基本	
//
//対応関数
//FLD_SCENEAREA_AREACHECK_CIRCLE
//FLD_SCENEAREA_UPDATE_CIRCLE
//=====================================
typedef struct {
  u32   rot_start;
  u32   rot_end;
  fx32  dist_min;
  fx32  dist_max;

	// カメラ中心
  fx32  center_x;	
  fx32  center_y;
  fx32  center_z;

  u32   pitch;
  fx32  length;

} FLD_SCENEAREA_CIRCLE_PARAM;

//-------------------------------------
/// 回転移動 FIXCAMERA
//
//対応関数
//FLD_SCENEAREA_AREACHECK_CIRCLE
//FLD_SCENEAREA_UPDATE_CIRCLE
//FLD_SCENEAREA_UPDATE_FXCAMERA,
//=====================================
typedef struct {
  u32   rot_start;
  u32   rot_end;
  fx32  dist_min;
  fx32  dist_max;

	// カメラ中心
  fx32  center_x;	
  fx32  center_y;
  fx32  center_z;

  u32   pitch;
  fx32  length;

  // 固定カメラに必要なパラメータ
  fx32  target_x;
  fx32  target_y;
  fx32  target_z;
  fx32  camera_x;
  fx32  camera_y;
  fx32  camera_z;
} FLD_SCENEAREA_CIRCLE_FIXCAMERA_PARAM;


#ifdef _cplusplus
}	// extern "C"{
#endif



