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


//===================================================================
// ■特殊進入イベントのカメラ動作データ
//===================================================================
typedef struct {

  u32 exitType;        // 出入り口タイプ
  u32 pitch;           // ピッチ
  u32 yaw;             // ヨー
  u32 length;          // 距離
  u32 targetOffsetX;   // ターゲットオフセットx
  u32 targetOffsetY;   // ターゲットオフセットy
  u32 targetOffsetZ;   // ターゲットオフセットz
  u32 frame;           // 動作フレーム数
  u8  validFlag_IN;    // 進入時に有効なデータかどうか
  u8  validFlag_OUT;   // 退出時に有効なデータかどうか

} ENTRANCE_CAMERA;


// 特殊出入り口のカメラ動作データを読み込む
extern void ENTRANCE_CAMERA_LoadData( 
    ENTRANCE_CAMERA* dest, EXIT_TYPE exitType );

// 特殊出入り口に入る時のカメラ動作タスクを登録する
extern void ENTRANCE_CAMERA_AddDoorInTask( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data );

// 特殊出入り口から出る時のカメラ動作タスクを登録する
extern void ENTRANCE_CAMERA_AddDoorOutTask( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data );

// 特殊出入り口から出る時のカメラ動作タスクのための初期化を行う
extern void ENTRANCE_CAMERA_PrepareForDoorOut( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data );
