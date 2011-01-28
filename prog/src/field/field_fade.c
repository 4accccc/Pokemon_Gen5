/*
 *  @file   field_fade.c
 *  @brief  フィールドフェードイン・アウトタスク生成
 *  @author Miyuki Iwasawa
 *  @date   11.01.28
 *  @com  scrcmd_screeneffects.c のローカル関数だったCreateBrightFadeTask()を
 *        どこででも使えるようにグローバル関数にした
 */
#include <gflib.h>
#include "system/main.h"  //for HEAPID_PROC
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "fieldmap.h"       //FIELDMAP_GetFieldPlayer
#include "field/field_const.h"

#include "field_fade.h"

typedef struct BRIGHT_CNT_WORK_tag
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
	FIELD_FADE_TYPE fade_type;
  FIELD_FADE_WAIT_FLAG wait_flag;
  FADE_IO   fade_io;
  int Seq;
}BRIGHT_CNT_WORK;

static void BrightCntTcb( GFL_TCB* tcb, void* work );

//------------------------------------------------------------------
/**
 * @brief	ブライトネスフェードタスク生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param fade_io   イン　or アウト
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	none
 */
//------------------------------------------------------------------
void FIELDMAP_CreateMapFadeReqTask( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, FADE_IO fade_io,
                             FIELD_FADE_TYPE type, FIELD_FADE_WAIT_FLAG wait )
{
  BRIGHT_CNT_WORK *task_wk;
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  //ワーク作成
  task_wk = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(BRIGHT_CNT_WORK) );
  // TCBを追加
  GFL_TCB_AddTask( tcbsys, BrightCntTcb, task_wk, 0 );
  
  //リクエストフラグ立てる
  FIELDMAP_SetMapFadeReqFlg( fieldmap, TRUE );

  task_wk->Seq  = 0;
  task_wk->gsys      = gsys;
  task_wk->fieldmap  = fieldmap;
	task_wk->fade_type = type;
  task_wk->wait_flag = wait;
  task_wk->fade_io = fade_io;
}

//--------------------------------------------------------------
/**
 * ブライトネスフェードタスク
 * @param   tcb        タスクポインタ
 * @param   work      ワーク 
 * @retval  none
 */
//--------------------------------------------------------------
static void BrightCntTcb( GFL_TCB* tcb, void* work )
{
  BRIGHT_CNT_WORK *wk = work;
  switch(wk->Seq){
  case 0:
    {
      int mode;
      int start, end;
      int fade_speed;

      if ( (wk->fade_io == FMAP_FADE_IN)||(wk->fade_io == FMAP_FADE_OUT) ) fade_speed = FIELD_FADE_DEFAULT_WAIT;
      else fade_speed = FIELD_FADE_SLOW_WAIT;

      if ( (wk->fade_io == FMAP_FADE_IN)||(wk->fade_io == FMAP_FADE_IN_SLOW) )
      {
        //フェードインのときはＢＧ初期化する
        
        start = 16;
        end = 0;
        
        // BGモード設定と表示設定の復帰
        {
          int mv = GFL_DISP_GetMainVisible();
          FIELDMAP_InitBGMode();
          GFL_DISP_GX_SetVisibleControlDirect( mv );
        }
        FIELDMAP_InitBG(wk->fieldmap);
      }
      else
      {
        start = 0;
        end = 16;
      }

      if ( wk->fade_type ==  FIELD_FADE_BLACK)
      {
        mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
      }
      else
      {
        mode = GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN | GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB;
      }
      GFL_FADE_SetMasterBrightReq(mode, start, end, fade_speed);
    }
    wk->Seq++;
    break;
  case 1:
    {
      BOOL rc = FALSE;
      if( wk->wait_flag == FIELD_FADE_NO_WAIT ) rc = TRUE;
		  if( GFL_FADE_CheckFade() == FALSE ) rc = TRUE;

      if (rc) { //フェード終了
        FIELDMAP_SetMapFadeReqFlg( wk->fieldmap, FALSE ); //リクエスト落す
        GFL_TCB_DeleteTask( tcb );
        GFL_HEAP_FreeMemory( work );
      }
    }
		break;
  }
}


