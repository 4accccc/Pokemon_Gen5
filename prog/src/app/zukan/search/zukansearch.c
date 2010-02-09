//============================================================================================
/**
 * @file		zukansearch.c
 * @brief		図鑑検索画面
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZUKANSEARCH
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "zknsearch_main.h"
#include "zknsearch_seq.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT ZukanSearchProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanSearchProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanSearchProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA ZUKANSEARCH_ProcData = {
	ZukanSearchProc_Init,
	ZukanSearchProc_Main,
	ZukanSearchProc_End
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
static GFL_PROC_RESULT ZukanSearchProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZKNSEARCHMAIN_WORK * wk;

	OS_Printf( "↓↓↓↓↓　図鑑検索処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_SEARCH, 0x60000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(ZKNSEARCHMAIN_WORK), HEAPID_ZUKAN_SEARCH );
	GFL_STD_MemClear( wk, sizeof(ZKNSEARCHMAIN_WORK) );

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
static GFL_PROC_RESULT ZukanSearchProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( ZKNSEARCHSEQ_MainSeq( mywk ) == FALSE ){
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
static GFL_PROC_RESULT ZukanSearchProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_SEARCH );

	OS_Printf( "↑↑↑↑↑　図鑑検索処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
