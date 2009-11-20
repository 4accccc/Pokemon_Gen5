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
// fieldmap_tcb_rot_player.c
//============================ 
// プレイヤーの回転
void FIELDMAP_TCB_AddTask_RotatePlayer(                // 等速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );    
void FIELDMAP_TCB_AddTask_RotatePlayer_SpeedUp(        // 加速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );    
void FIELDMAP_TCB_AddTask_RotatePlayer_SpeedDown(      // 減速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );   

//============================
// fieldmap_tcb_move_player.c
//============================
// プレイヤーの移動
void FIELDMAP_TCB_AddTask_MovePlayer(
    FIELDMAP_WORK* fieldmap, int frame, VecFx32* pos );

//============================
// fieldmap_tcb_camera_zoom.c
//============================
// カメラのズーム
void FIELDMAP_TCB_AddTask_CameraZoom(                   // 等速ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  
void FIELDMAP_TCB_AddTask_CameraZoom_Sharp(             // 急発進ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  
