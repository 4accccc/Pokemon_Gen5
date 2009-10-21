//=============================================================================
/**
 *
 *	@file		pms_select.c
 *	@brief  登録済み簡易会話選択画面
 *	@author	hosaka genya
 *	@data		2009.10.20
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/pms_data.h"
#include "system/pms_draw.h"

#include "app/pms_select.h"

#include "system/main.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  PMS_SELECT_HEAP_SIZE = 0x20000,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _PMS_SELECT_WORK {
  int dmy;
};

const GFL_PROC_DATA ProcData_PMSSelect = {
  PMSSelect_Init,
  PMSSelect_Main,
  PMSSelect_Exit,
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  登録済み簡易会話選択 初期化
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT PMSSelect_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  PMS_SELECT_WORK* wk;

  switch( *seq )
  {
  case 0:
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PMS_SELECT, PMS_SELECT_HEAP_SIZE );
    wk = GFL_PROC_AllocWork( proc, sizeof(PMS_SELECT_WORK), HEAPID_PMS_SELECT );
    (*seq)++;
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
    break;
  default : GF_ASSERT(0);
  }

  return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  登録済み簡易会話選択 終了
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT PMSSelect_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  PMS_SELECT_WORK* wk = mywk;

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_PMS_SELECT );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  登録済み簡易会話選択 主処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT PMSSelect_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
#ifdef PM_DEBUG
  // デバッグキーで抜ける
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }
#endif 

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================


