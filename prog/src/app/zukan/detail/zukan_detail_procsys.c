//============================================================================
/**
 *  @file   zukan_detail_procsys.c
 *  @brief  図鑑詳細画面共通PROC
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZKNDTL_PROC
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_procsys.h"

// アーカイブ

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
enum
{
  SEQ_INIT,
  SEQ_MAIN,
  SEQ_END,
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
struct _ZKNDTL_PROC
{
  const ZKNDTL_PROC_DATA* data;
  int                     proc_seq;
  int                     subseq;
  void*                   parent_work;
  void*                   work;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
ZKNDTL_PROC* ZKNDTL_PROC_SysCreateProc(const ZKNDTL_PROC_DATA* procdata, void* pwork, HEAPID heap_id)
{
  ZKNDTL_PROC* proc;
  proc = GFL_HEAP_AllocMemory(heap_id, sizeof(ZKNDTL_PROC));
  
  proc->data          = procdata;
  proc->proc_seq      = SEQ_INIT;
  proc->subseq        = 0;
  proc->parent_work   = pwork;
  proc->work          = NULL;

  return proc;
}

//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZKNDTL_PROC_SysDeleteProc(ZKNDTL_PROC* proc)
{
  GFL_HEAP_FreeMemory(proc);
}

//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
BOOL ZKNDTL_PROC_SysMain(ZKNDTL_PROC* proc, ZKNDTL_COMMON_WORK* cmn)
{
  ZKNDTL_PROC_RESULT result = ZKNDTL_PROC_RES_FINISH;

  switch(proc->proc_seq)
  {
  case SEQ_INIT:
    {
      if( proc->data->init_func )
		    result = proc->data->init_func(proc, &proc->subseq, proc->parent_work, proc->work, cmn);
  		if(result == ZKNDTL_PROC_RES_FINISH)
      {
        proc->proc_seq = SEQ_MAIN;
  			proc->subseq = 0;
  		}
    }
    break;
  case SEQ_MAIN:
    {
      if( proc->data->main_func )
		    result = proc->data->main_func(proc, &proc->subseq, proc->parent_work, proc->work, cmn);
  		if(result == ZKNDTL_PROC_RES_FINISH)
      {
        proc->proc_seq = SEQ_END;
  			proc->subseq = 0;
  		}
    }
    break;
  case SEQ_END:
    {
      if( proc->data->end_func )
		    result = proc->data->end_func(proc, &proc->subseq, proc->parent_work, proc->work, cmn);
  		if(result == ZKNDTL_PROC_RES_FINISH)
      {
        return TRUE;
  		}
    }
    break;
  }

  return FALSE;
}

//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZKNDTL_PROC_SysCommand(ZKNDTL_PROC* proc, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd)
{
  if( proc->data->cmd_func )
    proc->data->cmd_func(proc, &proc->subseq, proc->parent_work, proc->work, cmn, cmd);
}

//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZKNDTL_PROC_SysDraw3D(ZKNDTL_PROC* proc, ZKNDTL_COMMON_WORK* cmn)
{
  if( proc->data->draw_3d_func )
    proc->data->draw_3d_func(proc, &proc->subseq, proc->parent_work, proc->work, cmn);
}




//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void* ZKNDTL_PROC_AllocWork(ZKNDTL_PROC* proc, u32 size, HEAPID heap_id)
{
	proc->work = GFL_HEAP_AllocMemory(heap_id, size);
	return proc->work;
}

//------------------------------------------------------------------
/**
 *  @brief          
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void ZKNDTL_PROC_FreeWork(ZKNDTL_PROC* proc)
{
	GFL_HEAP_FreeMemory(proc->work);
	proc->work = NULL;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 
//=====================================

