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

#include "field_task_camera_shake.h"  // for CAM_SHAKE_PARAM;

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
    GAMESYS_WORK* gameSystem, CAM_SHAKE_PARAM* shakeParam );

