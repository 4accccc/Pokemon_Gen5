//==============================================================================
/**
 * @file    save_control_intr.c
 * @brief   イントロ画面用、中断機能付きセーブシステム
 * @author  matsuda
 * @date    2009.12.10(木)
 *
 * イントロ画面の裏で初回セーブを実行しておき、カードアクセスしたい場合に
 * 中断処理をかけることで、カードアクセスできるタイミングを作るシステムです
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control_intr.h"


//==============================================================================
//  定数定義
//==============================================================================
typedef enum{
  INTR_SAVE_STATUS_NULL,          ///<何も処理していない
  INTR_SAVE_STATUS_MAIN,          ///<セーブ実行中
  INTR_SAVE_STATUS_SUSPEND,       ///<セーブは中断しています
  INTR_SAVE_STATUS_1ST_FINISH,    ///<第1段階のセーブ完了(名前入力、性別セット前)
  INTR_SAVE_STATUS_2ND_FINISH,    ///<第2段階のセーブ完了(名前入力、性別セット後)
}INTR_SAVE_STATUS;


//==============================================================================
//  構造体定義
//==============================================================================
struct _INTR_SAVE_CONTROL{
  SAVE_CONTROL_WORK *ctrl;    ///<セーブシステムへのポインタ
  
  u8 status;                  ///<セーブ状況(INTR_SAVE_STATUS)
  u8 backup_status;           ///<中断時、直前のセーブ状況をバックアップ
  u8 suspend_req;             ///<TRUE:中断リクエスト発生
  u8 mystatus_save_req;       ///<TRUE:MYSTATUSが代入されたのでもう一度セーブを行う
  
  u8 first_save_end;          ///<TRUE:第1段階のセーブが完了した
  u8 no_save;                 ///<TRUE:既にセーブデータがあるのでセーブはしない
  u8 padding[2];
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * イントロ画面用、中断機能付きセーブシステム作成
 *
 * @param   heap_id		
 * @param   ctrl		
 *
 * @retval  INTR_SAVE_CONTROL *		
 */
//==================================================================
INTR_SAVE_CONTROL * IntrSave_Init(HEAPID heap_id, SAVE_CONTROL_WORK *ctrl)
{
  INTR_SAVE_CONTROL *isc;
  
  isc = GFL_HEAP_AllocClearMemory(heap_id, sizeof(INTR_SAVE_CONTROL));
  isc->ctrl = ctrl;
  if(SaveData_GetExistFlag(ctrl) == TRUE){
    isc->no_save = TRUE;
  }
  return isc;
}

//==================================================================
/**
 * イントロ画面用、中断機能付きセーブシステム削除
 *
 * @param   isc		
 */
//==================================================================
void IntrSave_Exit(INTR_SAVE_CONTROL *isc)
{
  if(isc->no_save == FALSE){
    GF_ASSERT(isc->status == INTR_SAVE_STATUS_2ND_FINISH);
  }
  GFL_HEAP_FreeMemory(isc);
}

//==================================================================
/**
 * セーブ開始
 *
 * @param   isc		
 */
//==================================================================
void IntrSave_Start(INTR_SAVE_CONTROL *isc)
{
  if(isc->no_save == TRUE){
    return;
  }
  SaveControl_SaveAsyncInit(isc->ctrl);
  isc->status = INTR_SAVE_STATUS_MAIN;
}

//==================================================================
/**
 * セーブメイン
 *
 * @param   isc		
 *
 * @retval  SAVE_RESULT		
 */
//==================================================================
SAVE_RESULT IntrSave_Main(INTR_SAVE_CONTROL *isc)
{
  SAVE_RESULT result;
  
  if(isc->no_save == TRUE){
    return SAVE_RESULT_OK;
  }
  
  switch(isc->status){
  case INTR_SAVE_STATUS_MAIN:
    if(isc->suspend_req == TRUE && GFL_BACKUP_SAVEASYNC_CheckCardLock() == FALSE){
      isc->backup_status = isc->status;
      isc->status = INTR_SAVE_STATUS_SUSPEND;
      OS_TPrintf("IntSave ... カードUnlock確認 stateサスペンド\n");
      return SAVE_RESULT_CONTINUE;
    }
    
    result = SaveControl_SaveAsyncMain(isc->ctrl);
    if(result == SAVE_RESULT_OK || result == SAVE_RESULT_NG){
      if(isc->first_save_end == FALSE){
        isc->first_save_end = TRUE;
        isc->status = INTR_SAVE_STATUS_1ST_FINISH;
        OS_TPrintf("IntSave ... 第1段階のセーブ終了\n");
      }
      else{
        isc->status = INTR_SAVE_STATUS_2ND_FINISH;
        OS_TPrintf("IntSave ... 第2段階のセーブ終了\n");
      }
    }
    return result;
  
  case INTR_SAVE_STATUS_SUSPEND:
    if(isc->suspend_req == FALSE){
      isc->status = isc->backup_status;
      OS_TPrintf("セーブは中断しています...\n");
    }
    return SAVE_RESULT_CONTINUE;

  case INTR_SAVE_STATUS_1ST_FINISH:
    if(isc->mystatus_save_req == TRUE){
      IntrSave_Start(isc);
    }
    return SAVE_RESULT_CONTINUE;

  case INTR_SAVE_STATUS_2ND_FINISH:
    return SAVE_RESULT_OK;
  }
  
  return SAVE_RESULT_CONTINUE;
}

//==================================================================
/**
 * セーブ中断リクエスト
 *
 * @param   isc		
 *
 * 実際に中断されたかはIntrSave_CheckSuspend関数で監視してください
 */
//==================================================================
void IntrSave_ReqSuspend(INTR_SAVE_CONTROL *isc)
{
  if(isc->status == INTR_SAVE_STATUS_1ST_FINISH || isc->status == INTR_SAVE_STATUS_2ND_FINISH){
    return;
  }
  
  GF_ASSERT(isc->suspend_req == FALSE);
  isc->suspend_req = TRUE;
}

//==================================================================
/**
 * 中断していたセーブを再開します
 *
 * @param   isc		
 */
//==================================================================
void IntrSave_Resume(INTR_SAVE_CONTROL *isc)
{
  if(isc->status == INTR_SAVE_STATUS_1ST_FINISH || isc->status == INTR_SAVE_STATUS_2ND_FINISH){
    return;
  }
  
  GF_ASSERT(isc->suspend_req == TRUE);
  isc->suspend_req = FALSE;
}

//==================================================================
/**
 * セーブを中断しているか調べる
 *
 * @param   isc		
 *
 * @retval  BOOL		TRUE:中断している(カードアクセス可能)
 * @retval  BOOL		FALSE:中断していない(カードアクセス禁止)
 */
//==================================================================
BOOL IntrSave_CheckSuspend(INTR_SAVE_CONTROL *isc)
{
  if(isc->no_save == TRUE 
      || isc->status == INTR_SAVE_STATUS_1ST_FINISH 
      || isc->status == INTR_SAVE_STATUS_2ND_FINISH){
    //外部シーケンスの進行停止の原因になるのでセーブが終了の時はTRUEを返す
    return TRUE;
  }
  
  if(isc->status == INTR_SAVE_STATUS_SUSPEND){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 第1段階のセーブが完了後、MYSTATUSに性別、名前をセットした後、
 * この関数を呼び出してください
 * MYSTATUSセット完了後の2度目のセーブを行います
 *
 * @param   isc		
 */
//==================================================================
void IntrSave_ReqMyStatusSave(INTR_SAVE_CONTROL *isc)
{
  isc->mystatus_save_req = TRUE;
}

//==================================================================
/**
 * 全てのセーブが完了しているか調べる
 *
 * @param   isc		
 *
 * @retval  BOOL		TRUE:全てのセーブが完了している
 * @retval  BOOL		FALSE:まだセーブが残っている
 */
//==================================================================
BOOL IntrSave_CheckAllSaveEnd(INTR_SAVE_CONTROL *isc)
{
  if(isc->no_save == TRUE){
    return TRUE;
  }
  if(isc->status == INTR_SAVE_STATUS_2ND_FINISH){
    return TRUE;
  }
  return FALSE;
}

