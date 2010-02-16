/////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  特殊出入り口のカメラ演出データ
 * @file   entrance_camera_settings.h
 * @author obata
 * @date   2010.02.16
 */
/////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <gflib.h>
#include "field/eventdata_type.h"  // for EXIT_TYPE_xxxx

//=======================================================================================
// ■特殊進入イベントのカメラ動作データ
//=======================================================================================
typedef struct 
{
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

} ENTRANCE_CAMERA_SETTINGS;


// データ取得
extern void ENTRANCE_CAMERA_SETTINGS_LoadData( ENTRANCE_CAMERA_SETTINGS* dest, EXIT_TYPE exitType );
