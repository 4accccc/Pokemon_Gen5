//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mictest.h
 *	@brief		マイクテスト画面外部公開ヘッダー
 *	@author		Toru=Nagihashi
 *	@data		2008.07.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __MICTEST_H__
#define __MICTEST_H__

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern GFL_PROC_RESULT MicTestProc_Init( GFL_PROC *proc,int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MicTestProc_Exit( GFL_PROC *proc,int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MicTestProc_Main( GFL_PROC *proc,int *seq, void *pwk, void *mywk );


#endif		// __MICTEST_H__

