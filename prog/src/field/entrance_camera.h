/////////////////////////////////////////////////////////////////////
/**
 * @brief  特殊出入り口のカメラ演出データ
 * @file   entrance_camera.h
 * @author obata
 * @date   2010.02.16
 */
/////////////////////////////////////////////////////////////////////
#pragma once 
#include <gflib.h>
#include "field/eventdata_type.h" // for EXIT_TYPE_xxxx
#include "field/fieldmap_proc.h"  // for FIELDMAP_WORK


typedef struct _ECAM_WORK ECAM_WORK;


// 自機の一歩移動の有無
typedef enum {
  ECAM_ONESTEP_ON,  // 一歩移動あり
  ECAM_ONESTEP_OFF, // 一歩移動なし
} ECAM_ONESTEP;

// シチュエーション
typedef enum {
  ECAM_SITUATION_IN,  // 出入り口へ入る
  ECAM_SITUATION_OUT, // 出入り口から出る
} ECAM_SITUATION;


// 演出パラメータ
typedef struct { 

  EXIT_TYPE      exitType;  // 出入り口タイプ
  ECAM_SITUATION situation; // 入る or 出る
  ECAM_ONESTEP   oneStep;   // 自機の一歩移動があるかどうか

} ECAM_PARAM;


// ワークの生成・破棄
extern ECAM_WORK* ENTRANCE_CAMERA_CreateWork( FIELDMAP_WORK* fieldmap );
extern void ENTRANCE_CAMERA_DeleteWork( ECAM_WORK* work );

// 演出をセットアップする
extern void ENTRANCE_CAMERA_Setup( ECAM_WORK* work, const ECAM_PARAM* param );
// 演出を開始する
extern void ENTRANCE_CAMERA_Start( ECAM_WORK* work );
// 演出によって操作したカメラを復帰する
extern void ENTRANCE_CAMERA_Recover( ECAM_WORK* work );

// 演出の有無をチェックする
extern BOOL ENTRANCE_CAMERA_IsAnimeExist( const ECAM_WORK* work );
// 演出が０フレームかどうかをチェックする
extern BOOL ENTRANCE_CAMERA_IsZeroFrameAnime( const ECAM_WORK* work );
