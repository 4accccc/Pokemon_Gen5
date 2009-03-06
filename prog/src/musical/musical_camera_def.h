//======================================================================
/**
 * @file	musical_camera_def.h
 * @brief	ミュージカル3Dカメラ系設定
 * @author	ariizumi
 * @data	09/03/05
 */
//======================================================================

#ifndef MUSICAL_CAMERA_DEF_H__
#define MUSICAL_CAMERA_DEF_H__

// カメラの設定を各パートとmusical_mcssと共有する必要があるので
// ヘッダとして定義。
// static const で宣言するとメモリ食うのでdefineで数値を設定する



#endif MUSICAL_CAMERA_DEF_H__

#define MUSICAL_CAMERA_POS {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(301.0f)}
#define MUSICAL_CAMERA_TRG {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(0.0f)}
#define MUSICAL_CAMERA_UP  {0,FX32_ONE,0}

#define MUSICAL_CAMERA_NEAR (FX32_ONE)
#define MUSICAL_CAMERA_FAR  (FX32_ONE*300)
