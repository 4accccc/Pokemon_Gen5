////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB タスク生成関数群
 * @file   fieldmap_tcb.h
 * @author obata
 * @date   2009.08.25
 *
 */
////////////////////////////////////////////////////////////////////////
#pragma once


//============================
// fieldmap_tcb_camera_zoom.c
//============================
// カメラのズーム
extern void FIELDMAP_TCB_AddTask_CameraZoom(                   // 等速ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  
extern void FIELDMAP_TCB_AddTask_CameraZoom_Sharp(             // 急発進ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  
