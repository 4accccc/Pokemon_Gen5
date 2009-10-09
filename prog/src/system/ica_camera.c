/////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  icaアニメーション カメラ応用関数
 * @file   ica_camera.c
 * @author obata
 * @date   2009.10.08
 *
 */
/////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/ica_anime.h"
#include "system/ica_camera.h"


//---------------------------------------------------------------------------
/**
 * @brief カメラ座標・ターゲット座標を現在フレームの状態に設定する
 *        anime の持つ trans をカメラ座標に,
 *                     rotate をカメラの向きに適用する.
 *
 * @param anime   設定アニメーション
 * @param camera  設定対象のカメラ
 */
//---------------------------------------------------------------------------
void ICA_CAMERA_SetCameraStatus( GFL_G3D_CAMERA* camera, const ICA_ANIME* anime )
{
  float x, y, z;
  u16 rx, ry, rz;
  MtxFx33 matrix;
  VecFx32 rotate;
  VecFx32 def_forward = { 0, 0, -FX32_ONE };
  VecFx32 def_upward  = { 0, FX32_ONE, 0 }; 
  VecFx32 pos, target, forward, upward;

  // 回転行列を作成
  ICA_ANIME_GetRotate( anime, &rotate );
  x = FX_FX32_TO_F32( rotate.x );
  y = FX_FX32_TO_F32( rotate.y );
  z = FX_FX32_TO_F32( rotate.z );
  while( x < 0 ) x += 360.0f;
  while( y < 0 ) y += 360.0f;
  while( z < 0 ) z += 360.0f;
  rx = x / 360.0f * 0xffff;
  ry = y / 360.0f * 0xffff;
  rz = z / 360.0f * 0xffff; 
  GFL_CALC3D_MTX_CreateRot( rx, ry, rz, &matrix );

  // ベクトル回転でカメラの向きを算出
  MTX_MultVec33( &def_forward, &matrix, &forward );
  MTX_MultVec33( &def_upward, &matrix, &upward );

  // カメラ・注視点位置を求める
  ICA_ANIME_GetTranslate( anime, &pos );
  VEC_Add( &pos, &forward, &target );

  // カメラの設定
  GFL_G3D_CAMERA_SetPos( camera, &pos );
  GFL_G3D_CAMERA_SetTarget( camera, &target );
  GFL_G3D_CAMERA_SetCamUp( camera, &upward );
}

//-----------------------------------------------------------------------------------
/**
 * @brief カメラ座標を現在フレームの状態に設定する
 *        anime の持つ trans をカメラ座標に反映させる
 *
 * @param camera  設定対象のカメラ
 * @param anime   設定アニメーション
 */
//-----------------------------------------------------------------------------------
void ICA_CAMERA_SetCameraPos( GFL_G3D_CAMERA* camera, const ICA_ANIME* anime )
{
  VecFx32 pos;

  // カメラ位置を求める
  ICA_ANIME_GetTranslate( anime, &pos );

  // カメラの設定
  GFL_G3D_CAMERA_SetPos( camera, &pos );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 注視点座標を現在フレームの状態に設定する
 *        anime の持つ trans を注視点座標に反映させる
 *
 * @param camera  設定対象のカメラ
 * @param anime   設定アニメーション
 */
//-----------------------------------------------------------------------------------
void ICA_CAMERA_SetTargetPos( GFL_G3D_CAMERA* camera, const ICA_ANIME* anime )
{
  VecFx32 pos;

  // 注視点位置を求める
  ICA_ANIME_GetTranslate( anime, &pos );

  // カメラの設定
  GFL_G3D_CAMERA_SetTarget( camera, &pos );
}
