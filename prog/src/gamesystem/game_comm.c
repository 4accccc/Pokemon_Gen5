//==============================================================================
/**
 * @file    game_comm.c
 * @brief   ゲーム通信管理
 * @author  matsuda
 * @date    2009.05.12(火)
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/game_comm.h"

#include "field/game_beacon_search.h"
#include "field/field_comm/field_comm_func.h"


//==============================================================================
//  定数定義
//==============================================================================
///シーケンス番号
typedef enum{
  GCSSEQ_INIT,
  GCSSEQ_INIT_WAIT,
  GCSSEQ_UPDATE,
  GCSSEQ_EXIT,
  GCSSEQ_EXIT_WAIT,
}GCSSEQ;


//==============================================================================
//  構造体定義
//==============================================================================
///ゲーム実行用ワーク構造体
typedef struct{
  u8 seq;
  u8 padding[3];
}GAME_COMM_SUB_WORK;

///ゲーム通信管理ワーク構造体
typedef struct _GAME_COMM_SYS{
  GAME_COMM_NO game_comm_no;
  GAME_COMM_NO reserve_comm_game_no;   ///<予約通信ゲーム番号
  GAME_COMM_SUB_WORK sub_work;
  void *app_work;                     ///<各アプリケーションが確保したワークのポインタ
}GAME_COMM_SYS;

///ゲーム通信関数データ構造体
typedef struct{
  void *(*init_func)(void);                   ///<初期化処理
  BOOL (*init_wait_func)(void *app_work);     ///<初期化完了待ち
  void (*update_func)(void *app_work);        ///<更新処理
  void (*exit_func)(void *app_work);          ///<終了処理
  BOOL (*exit_wait_func)(void *app_work);     ///<終了完了待ち
}GAME_FUNC_TBL;


//==============================================================================
//  データ
//==============================================================================
///通信ゲーム管理テーブル
static const GAME_FUNC_TBL GameFuncTbl[] = {
  //GAME_COMM_NO_NULL    1origin化
  {
    NULL,       //init
    NULL,       //init_wait
    NULL,       //update
    NULL,       //exit
    NULL,       //exit_wait
  },
  
  //GAME_COMM_NO_FIELD_BEACON_SEARCH
  {
    GameBeacon_Init,       //init
    GameBeacon_InitWait,   //init_wait
    GameBeacon_Update,     //update
    GameBeacon_Exit,       //exit
    GameBeacon_ExitWait,   //exit_wait
  },
  //GAME_COMM_NO_INVASION
  {
    FIELD_COMM_FUNC_InitCommSystem,       //init
    FIELD_COMM_FUNC_InitCommSystemWait,   //init_wait
    FIELD_COMM_FUNC_UpdateSystem,         //update
    FIELD_COMM_FUNC_TermCommSystem,       //exit
    FIELD_COMM_FUNC_TermCommSystemWait,   //exit_wait
  },
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ゲーム通信管理ワーク確保
 *
 * @param   heap_id		
 *
 * @retval  GAME_COMM_SYS_PTR		確保したワークへのポインタ
 */
//==================================================================
GAME_COMM_SYS_PTR GameCommSys_Alloc(HEAPID heap_id)
{
  GAME_COMM_SYS_PTR gcsp;
  
  gcsp = GFL_HEAP_AllocMemory(heap_id, sizeof(GAME_COMM_SYS));
  return gcsp;
}

//==================================================================
/**
 * ゲーム通信管理ワーク解放
 *
 * @param   gcsp		
 */
//==================================================================
void GameCommSys_Free(GAME_COMM_SYS_PTR gcsp)
{
  GF_ASSERT(gcsp->game_comm_no == GAME_COMM_NO_NULL && gcsp->app_work == NULL);
  
  GFL_HEAP_FreeMemory(gcsp);
}

//==================================================================
/**
 * ゲーム通信メイン処理
 *
 * @param   gcsp		
 */
//==================================================================
void GameCommSys_Main(GAME_COMM_SYS_PTR gcsp)
{
  GAME_COMM_SUB_WORK *sub_work;
  const GAME_FUNC_TBL *func_tbl;
  
  if(gcsp == NULL || gcsp->game_comm_no == GAME_COMM_NO_NULL){
    return;
  }
  
  sub_work = &gcsp->sub_work;
  func_tbl = &GameFuncTbl[gcsp->game_comm_no];
  
  switch(sub_work->seq){
  case GCSSEQ_INIT:
    if(func_tbl->init_func != NULL){
      gcsp->app_work = func_tbl->init_func();
    }
    sub_work->seq++;
    break;
  case GCSSEQ_INIT_WAIT:
    if(func_tbl->init_wait_func == NULL || func_tbl->init_wait_func(gcsp->app_work) == TRUE){
      sub_work->seq++;
    }
    break;
  case GCSSEQ_UPDATE:
    func_tbl->update_func(gcsp->app_work);
    break;
  case GCSSEQ_EXIT:
    if(func_tbl->exit_func != NULL){
      func_tbl->exit_func(gcsp->app_work);
    }
    sub_work->seq++;
    break;
  case GCSSEQ_EXIT_WAIT:
    if(func_tbl->exit_wait_func == NULL || func_tbl->exit_wait_func(gcsp->app_work) == TRUE){
      if(gcsp->app_work != NULL){
        GFL_HEAP_FreeMemory(gcsp->app_work);
        gcsp->app_work = NULL;
      }
      gcsp->game_comm_no = GAME_COMM_NO_NULL;
      if(gcsp->reserve_comm_game_no != GAME_COMM_NO_NULL){
        GameCommSys_Boot(gcsp, gcsp->reserve_comm_game_no);
      }
    }
    break;
  }
}

//==================================================================
/**
 * ゲーム通信起動
 *
 * @param   gcsp		        ゲーム通信管理ワークへのポインタ
 * @param   game_comm_no		起動する通信ゲーム番号(GAME_COMM_NO_???)
 */
//==================================================================
void GameCommSys_Boot(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO game_comm_no)
{
  GF_ASSERT(gcsp->game_comm_no == GAME_COMM_NO_NULL);
  
  gcsp->game_comm_no = game_comm_no;
  GFL_STD_MemClear(&gcsp->sub_work, sizeof(GAME_COMM_SUB_WORK));
}

//==================================================================
/**
 * 終了リクエスト
 *
 * @param   gcsp		
 *
 * 終了の確認はGameCommSys_BootCheck関数を使用してください
 */
//==================================================================
void GameCommSys_ExitReq(GAME_COMM_SYS_PTR gcsp)
{
  GF_ASSERT(gcsp->sub_work.seq == GCSSEQ_UPDATE);
  
  OS_TPrintf("GameCommSys_ExitReq\n");
  gcsp->sub_work.seq = GCSSEQ_EXIT;
}

//==================================================================
/**
 * 通信ゲーム切り替えリクエスト
 *
 * @param   gcsp		
 * @param   game_comm_no		切り替え後、起動する通信ゲーム番号(GAME_COMM_NO_???)
 */
//==================================================================
void GameCommSys_ChangeReq(GAME_COMM_SYS_PTR gcsp, GAME_COMM_NO game_comm_no)
{
  GF_ASSERT(gcsp->sub_work.seq == GCSSEQ_UPDATE);
  
  gcsp->sub_work.seq = GCSSEQ_EXIT;
  gcsp->reserve_comm_game_no = game_comm_no;
}

//==================================================================
/**
 * 起動確認
 *
 * @param   gcsp		
 *
 * @retval  GAME_COMM_NO  起動している通信ゲーム番号(何も起動していなければGAME_COMM_NO_NULL)
 */
//==================================================================
GAME_COMM_NO GameCommSys_BootCheck(GAME_COMM_SYS_PTR gcsp)
{
  if(gcsp->game_comm_no != GAME_COMM_NO_NULL){
    return gcsp->game_comm_no;
  }
  if(gcsp->reserve_comm_game_no != GAME_COMM_NO_NULL){
    return gcsp->reserve_comm_game_no;
  }
  return GAME_COMM_NO_NULL;
}

//==================================================================
/**
 * 通信ゲーム番号を取得する
 *
 * @param   gcsp		
 *
 * @retval  GAME_COMM_NO		
 */
//==================================================================
GAME_COMM_NO GameCommSys_GetCommGameNo(GAME_COMM_SYS_PTR gcsp)
{
  return gcsp->game_comm_no;
}

//==================================================================
/**
 * 通信ゲームがシステムの待ち状態か調べる
 *
 * @param   gcsp		
 *
 * @retval  		TRUE:初期化待ち、終了待ち、等のシステム系の待ち状態になっている
 * @retval      FALSE:通常の更新処理を実行している状態
 */
//==================================================================
BOOL GameCommSys_CheckSystemWaiting(GAME_COMM_SYS_PTR gcsp)
{
  if(gcsp->game_comm_no == GAME_COMM_NO_NULL || gcsp->sub_work.seq != GCSSEQ_UPDATE){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * アプリケーションワークのポインタを取得する
 *
 * @param   gcsp		  
 *
 * @retval  void *		アプリケーションワークへのポインタ
 */
//==================================================================
void *GameCommSys_GetAppWork(GAME_COMM_SYS_PTR gcsp)
{
  return gcsp->app_work;
}
