//============================================================================
/**
 *
 *	@file		debug_3d_demo.h
 *	@brief    3Dデモテスト
 *	@author		hosaka genya
 *	@data		2009.11.20
 *
 */
//============================================================================
#pragma once

typedef struct _DEBUG_3DDEMO_WORK DEBUG_3DDEMO_WORK;

extern const GFL_PROC_DATA Debug3DDemoProcData;

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

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================
extern GFL_PROC_RESULT Debug3DDemoProc_Init( GFL_PROC *proc,int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Debug3DDemoProc_Main( GFL_PROC *proc,int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT Debug3DDemoProc_Exit( GFL_PROC *proc,int *seq, void *pwk, void *mywk );




