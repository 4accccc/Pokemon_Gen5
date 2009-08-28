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
void FIELDMAP_TCB_ROT_PLAYER_AddTask(                // 等速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );    
void FIELDMAP_TCB_ROT_PLAYER_AddTask_SpeedUp(        // 加速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );    
void FIELDMAP_TCB_ROT_PLAYER_AddTask_SlowDown(       // 減速
    FIELDMAP_WORK* fieldmap, int frame, int rot_num );   

//============================
// fieldmap_tcb_warp_player.c
//============================
// プレイヤーのワープ移動
void FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp(      // 上昇退場
    FIELDMAP_WORK* fieldmap, int frame, int dist );
void FIELDMAP_TCB_WARP_PLAYER_AddTask_AppearDown(       // 降下登場
    FIELDMAP_WORK* fieldmap, int frame, int dist );
void FIELDMAP_TCB_WARP_PLAYER_AddTask_FallIn(           // 落下登場
    FIELDMAP_WORK* fieldmap, int frame, int dist );

//=======================
// fieldmap_tcb_camera.c
//=======================
// カメラのズーム
void FIELDMAP_TCB_CAMERA_AddTask_Zoom(
    FIELDMAP_WORK* fieldmap, int frame, fx32 dist );  

// カメラの回転
void FIELDMAP_TCB_CAMERA_AddTask_Yaw(                   // ヨー回転
    FIELDMAP_WORK* fieldmap, int frame, u16 angle );   
void FIELDMAP_TCB_CAMERA_AddTask_Pitch(                 // ピッチ回転
    FIELDMAP_WORK* fieldmap, int frame, u16 angle );   
