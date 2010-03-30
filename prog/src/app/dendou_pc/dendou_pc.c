//============================================================================================
/**
 * @file		dendou_pc.c
 * @brief		殿堂入り確認画面
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DENDOUPC
 */
//============================================================================================
#include <gflib.h>

#include "dpc_main.h"
#include "dpc_seq.h"



//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT DendouPcProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DendouPcProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DendouPcProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA DENDOUPC_ProcData = {
	DendouPcProc_Init,
	DendouPcProc_Main,
	DendouPcProc_End
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
static GFL_PROC_RESULT DendouPcProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	DPCMAIN_WORK * wk;

	OS_Printf( "↓↓↓↓↓　殿堂入りＰＣ処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DENDOU_PC, 0x60000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(DPCMAIN_WORK), HEAPID_DENDOU_PC );
	GFL_STD_MemClear( wk, sizeof(DPCMAIN_WORK) );

	wk->dat = pwk;

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
static GFL_PROC_RESULT DendouPcProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( DPCSEQ_MainSeq( mywk ) == FALSE ){
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
static GFL_PROC_RESULT DendouPcProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_DENDOU_PC );

	OS_Printf( "↑↑↑↑↑　殿堂入りＰＣ処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
