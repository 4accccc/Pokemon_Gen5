//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_task_camera_shake.c
 *	@brief  フィールドタスク（カメラ振動）
 *	@author	Masayuki Onoue
 *	@date		2011.06.28
 *  @note   飛行ジムから移植 <- 2011.06.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field/field_camera.h"
#include "field_task_camera_shake.h"


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	カメラ振動ワーク
//=====================================
typedef struct {
  FIELDMAP_WORK*  fieldmap;
  FIELD_CAMERA*   camera;

  u32             seq;
  CAM_SHAKE_PARAM shakeParam;
  const VecFx32   *WatchTarget;

} CAM_SHAKE_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static FIELD_TASK_RETVAL ShakeCamera( void* wk );

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動タスクを作成する
 *
 *	@param	fieldmap    動作対象フィールドマップ
 *	@param  shakeParam  振動パラメータ
 *
 *	@return 作成したタスク
 *
 */
//-----------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_ShakeCamera( FIELDMAP_WORK* fieldmap, const CAM_SHAKE_PARAM* shakeParam )
{
  FIELD_TASK* task;
  CAM_SHAKE_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(CAM_SHAKE_WORK), ShakeCamera );
  // 初期化
  work              = FIELD_TASK_GetWork( task );
  work->fieldmap    = fieldmap;
  work->camera      = FIELDMAP_GetFieldCamera( fieldmap );
  work->shakeParam  = *shakeParam;
  work->seq         = 0;

  return task;
}

//=============================================================================
//  タスク処理関数
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動処理
 */
//-----------------------------------------------------------------------------
static FIELD_TASK_RETVAL ShakeCamera( void* wk )
{

  FIELD_CAMERA * camera;
  BOOL end = FALSE;
  CAM_SHAKE_WORK *work = wk;
  CAM_SHAKE_PARAM *param = &(work->shakeParam);

  camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  // 開始フレーム経過まで待つ
  if( param->WaitStartFrame > 0 )
  {
    param->WaitStartFrame--;
    return FIELD_TASK_RETVAL_CONTINUE;
  }

  switch(work->seq){
  case 0:
    //トレース終了
    FIELD_CAMERA_StopTraceRequest(camera);
    work->seq++;
    break;
    /* break through */
  case 1:
    if ( FIELD_CAMERA_CheckTrace(work->camera) )
    {
      break;
    }
    {
      //復帰パラメータをセットする
      FIELD_CAMERA_SetRecvCamParam(work->camera);

      //カメラのターゲットを取得
      work->WatchTarget = FIELD_CAMERA_GetWatchTarget(camera);

      //バインド解除
      FIELD_CAMERA_FreeTarget(camera);
      
      work->seq++;
    }
    /* break through */
  case 2:
    {
      //ターゲットを動かす
      int tmp;
      u16 rad;
      param->NowSync++;
      tmp = (param->NowSync * 0x10000) / param->Sync;
      rad = tmp;

      if ( param->SubStartTime != 0)
      {
        if ( param->TimeMax-param->Time >= param->SubStartTime )
        {
          param->SubMargineCount++;
          if (param->SubMargineCount>=param->SubMargine)
          {
            param->SubMargineCount = 0;
            param->Height -= param->SubH;
            param->Width -= param->SubW;
            if (param->Height <= 0) param->Height = 1;
            if (param->Width <= 0) param->Width = 1;
          }
        }
      }

      if (param->NowSync >= param->Sync)
      {
        param->Time--;
        param->NowSync = 0;
      }
      if (param->Time == 0)
      {
        param->NowSync = 0;
        end = TRUE;
      }

      if (end)
      {
        //カメラオフセットクリア
        {
          VecFx32 cam_ofs = {0,0,0};
          FIELD_CAMERA_SetCamPosOffset( camera, &cam_ofs );
        }

        //復帰パラメータをクリアする
        FIELD_CAMERA_ClearRecvCamParam(camera);
        
        //際バインド
        FIELD_CAMERA_BindTarget(camera, work->WatchTarget);

        //トレース再開
        FIELD_CAMERA_RestartTrace(camera);
        work->seq++;
      }
      else
      {
        fx32 w, h;
        VecFx32 target;
        VecFx32 cam_ofs;
        w = param->Width * FX_SinIdx(rad);
        h = param->Height * FX_SinIdx(rad);
        target = *work->WatchTarget;
        target.x += w;
        target.y += h;
        cam_ofs.x = w;
        cam_ofs.y = h;
        cam_ofs.z = 0;

        //座標セット
        {
          u8 ShakeType = param->ShakeType;
          if( (ShakeType == CAM_SHAKE_TYPE_ALL) || 
              (ShakeType == CAM_SHAKE_TYPE_TARGET_POS) )
          {
            FIELD_CAMERA_SetTargetPos( camera, &target );
          }
          if( (ShakeType == CAM_SHAKE_TYPE_ALL) || 
              (ShakeType == CAM_SHAKE_TYPE_CAM_POS_OFFSET) )
          {
            FIELD_CAMERA_SetCamPosOffset( camera, &cam_ofs );
          }
        }
      }
    }
    break;
  case 3:
    return FIELD_TASK_RETVAL_FINISH;
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}

