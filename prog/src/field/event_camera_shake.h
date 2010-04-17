/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  カメラ振動イベント
 * @file   event_camera_shake.c
 * @author obata
 * @date   2010.04.17
 *
 * ※scrcmd_camera より移動
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once


//===============================================================================
// ■カメラ振動パラメータ
//===============================================================================
typedef struct CAM_SHAKE_PARAM_tag {

  s16 Width;
  s16 Height;
  u16 SubW;
  u16 SubH;
  u16 SubStartTime;
  u16 SubMargineCount;
  u16 SubMargine;
  u8  TimeMax;
  u8  Time;

  u32 Sync;
  u32 NowSync;

} CAM_SHAKE_PARAM;


//-------------------------------------------------------------------------------
/**
 * @brief カメラ揺れイベントを生成する
 *
 * @param gameSystem
 * @param shakeParam 振動パラメータ
 *
 * @return 生成したイベント
 */
//-------------------------------------------------------------------------------
extern GMEVENT* EVENT_ShakeCamera( 
    GAMESYS_WORK* gameSystem, const CAM_SHAKE_PARAM* shakeParam );
