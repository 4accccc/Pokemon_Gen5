//============================================================================
/**
 *
 *	@file		pms_select.h
 *	@brief  登録済み簡易会話選択画面
 *	@author		hosaka genya
 *	@data		2009.10.20
 *
 */
//============================================================================
#pragma once

typedef struct _PMS_SELECT_WORK PMS_SELECT_WORK;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	
//==============================================================
typedef struct {
  // [IN]
  SAVE_CONTROL_WORK* save_ctrl;
  // [OUT]
  BOOL            out_cancel_flag;  ///< キャンセルで抜けたか？
  const PMS_DATA* out_pms_data;     ///< out_cancel_flagがTRUEの場合は NULL。FALSEの場合はSAVEDATAから
} PMS_SELECT_PARAM;

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================

extern const GFL_PROC_DATA ProcData_PMSSelect;

extern GFL_PROC_RESULT PMSSelect_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT PMSSelect_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT PMSSelect_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

