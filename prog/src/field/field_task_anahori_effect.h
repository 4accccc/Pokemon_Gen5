////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク ( あなほりエフェクト )
 * @file   field_task_anahori_effect.h
 * @author obata
 * @date   2010.05.03
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"

//------------------------------------------------------------------------------------------
/**
 * @brief 穴掘りエフェクト表示タスクを生成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param mmdl     操作対象の動作モデル
 * @param frame    タスク動作フレーム数
 * @param interval エフェクト表示間隔
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
extern FIELD_TASK* FIELD_TASK_AnahoriEffect( 
    FIELDMAP_WORK* fieldmap, MMDL* mmdl, int frame, int interval );
