//============================================================================================
/**
 * @file		dendou_demo.c
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DENDOUDEMO
 */
//============================================================================================
#include <gflib.h>

#include "ddemo_main.h"
#include "ddemo_seq.h"



//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT DendouDemoProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DendouDemoProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DendouDemoProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA DENDOUDEMO_ProcData = {
	DendouDemoProc_Init,
	DendouDemoProc_Main,
	DendouDemoProc_End
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：初期化
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DendouDemoProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	DDEMOMAIN_WORK * wk;

//	OS_Printf( "↓↓↓↓↓　殿堂入りデモ処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DENDOU_DEMO, 0x60000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(DDEMOMAIN_WORK), HEAPID_DENDOU_DEMO );
	GFL_STD_MemClear( wk, sizeof(DDEMOMAIN_WORK) );

	wk->dat = pwk;

#ifdef	PM_DEBUG
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
		wk->debugMode = TRUE;
		wk->debugNext = 1;
	}
#endif	// PM_DEBUG

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：メイン
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DendouDemoProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( DDEMOSEQ_MainSeq( mywk ) == FALSE ){
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：終了
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DendouDemoProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_DENDOU_DEMO );

//	OS_Printf( "↑↑↑↑↑　殿堂入りデモ処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
