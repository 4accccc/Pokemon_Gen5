////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(カメラのズーム処理)
 * @file   field_task_target_pos.h
 * @author obata
 * @date   2010.04.03
 */
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

extern FIELD_TASK* FIELD_TASK_CameraTargetMove( 
    FIELDMAP_WORK* fieldmap, u16 frame, const VecFx32* startPos, const VecFx32* endPos );
