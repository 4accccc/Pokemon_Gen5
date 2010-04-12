///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドカメラ アニメーション
 * @file   field_camera_anime.h
 * @author obata
 * @date   2010.04.10
 */
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "field/fieldmap_proc.h"
#include "field_camera.h"


//=============================================================================
// ■カメラパラメータ
//=============================================================================
typedef struct {

  u16     pitch;        // ピッチ
  u16     yaw;          // ヨー
  fx32    length;       // 距離
  VecFx32 targetPos;    // ターゲット座標
  VecFx32 targetOffset; // ターゲットオフセット

} FCAM_PARAM;

// 現在のカメラパラメータを取得する
extern void FCAM_PARAM_GetCurrentParam( const FIELD_CAMERA* camera, FCAM_PARAM* dest );


//=============================================================================
// ■アニメーションデータ
//=============================================================================
typedef struct {

  u32 frame; // 動作フレーム数

  FCAM_PARAM startParam; // 開始パラメータ
  FCAM_PARAM endParam;   // 最終パラメータ

  BOOL targetBindOffFlag; // ターゲットのバインドをOFFにするかどうか
  BOOL cameraAreaOffFlag; // カメラエリアをOFFにするかどうか

} FCAM_ANIME_DATA;


//=============================================================================
// ■カメラ操作ワークの不完全型
//=============================================================================
typedef struct _FCAM_ANIME_WORK FCAM_ANIME_WORK;

// カメラ操作ワークを生成する
extern FCAM_ANIME_WORK* FCAM_ANIME_CreateWork( FIELDMAP_WORK* fieldamp );
// カメラ操作ワークを破棄する
extern void FCAM_ANIME_DeleteWork( FCAM_ANIME_WORK* work );
// アニメーションデータを設定する
extern void FCAM_ANIME_SetAnimeData( FCAM_ANIME_WORK* work, const FCAM_ANIME_DATA* data );
// カメラをセットアップする
extern void FCAM_ANIME_SetupCamera( FCAM_ANIME_WORK* work );
// カメラの状態を開始パラメータの設定にする
extern void FCAM_ANIME_SetCameraStartParam( const FCAM_ANIME_WORK* work );
// アニメーションを開始する
extern void FCAM_ANIME_StartAnime( const FCAM_ANIME_WORK* work );
// アニメーションの終了をチェックする
extern BOOL FCAM_ANIME_IsAnimeFinished( const FCAM_ANIME_WORK* work );
// カメラを復帰する
extern void FCAM_ANIME_RecoverCamera( const FCAM_ANIME_WORK* work );
