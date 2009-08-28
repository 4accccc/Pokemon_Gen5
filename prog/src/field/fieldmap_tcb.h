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
// fieldmap_tcb_warp_player.c
//============================
// プレイヤーの移動
void FIELDMAP_TCB_AddTask_DisappearPlayer_LinearUp(      // 上昇退場
    FIELDMAP_WORK* fieldmap, int frame, int dist );
void FIELDMAP_TCB_AddTask_AppearPlayer_LinearDown(       // 降下登場
    FIELDMAP_WORK* fieldmap, int frame, int dist );
void FIELDMAP_TCB_AddTask_AppearPlayer_Fall(             // 落下登場
    FIELDMAP_WORK* fieldmap, int frame, int dist );

//============================
// fieldmap_tcb_camera_zoom.c
//============================
// カメラのズーム
void FIELDMAP_TCB_AddTask_CameraZoom(                   // 等速ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  
void FIELDMAP_TCB_AddTask_CameraZoom_Sharp(             // 急発進ズーム
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  

//============================
// fieldmap_tcb_camera_rot.c
//============================
// カメラの回転
void FIELDMAP_TCB_AddTask_CameraRotate_Yaw(             // ヨー回転
    FIELDMAP_WORK* fieldmap, int frame, u16 angle );   
void FIELDMAP_TCB_AddTask_CameraRotate_Pitch(           // ピッチ回転
    FIELDMAP_WORK* fieldmap, int frame, u16 angle );   
