//============================================================================================
/**
 * @file	field_camera_mv_param.h
 * @brief	カメラ線形移動パラメータ
 *
 */
//============================================================================================

#pragma once

//カメラの現在情報（線形補間用）
typedef struct FLD_CAM_MV_PARAM_CORE_tag
{
  //カメラ座標
  VecFx32 CamPos;
  //ターゲット座標
  VecFx32 TrgtPos;
  //オフセット（シフト）値
  VecFx32 Shift;
  //アングル（モード依存）
  u16 AnglePitch;
  u16 AngleYaw;
  //カメラとターゲットの距離
  fx32 Distance;
  //視野角
  u16 Fovy;
  u16 dummy;
}FLD_CAM_MV_PARAM_CORE;

typedef struct FLD_CAM_MV_PARAM_CHK_tag
{
  BOOL Shift;
  BOOL Angle;
  BOOL Dist;
  BOOL Fovy;
  BOOL Pos;
}FLD_CAM_MV_PARAM_CHK;

typedef struct FLD_CAM_MV_PARAM_tag
{
  FLD_CAM_MV_PARAM_CORE Core;
  //有効パラメータ指定
  FLD_CAM_MV_PARAM_CHK Chk;
}FLD_CAM_MV_PARAM;
