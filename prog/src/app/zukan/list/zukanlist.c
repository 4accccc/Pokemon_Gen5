//============================================================================================
/**
 * @file		zukanlist.c
 * @brief		図鑑リスト画面
 * @author	Hiroyuki Nakamura
 * @date		09.12.15
 *
 *	モジュール名：ZUKANLIST
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "zknlist_main.h"
#include "zknlist_seq.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT ZukanListProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanListProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanListProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA ZUKANLIST_ProcData = {
	ZukanListProc_Init,
	ZukanListProc_Main,
	ZukanListProc_End
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
static GFL_PROC_RESULT ZukanListProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZKNLISTMAIN_WORK * wk;

	OS_Printf( "↓↓↓↓↓　図鑑リスト処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_LIST, 0x60000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(ZKNLISTMAIN_WORK), HEAPID_ZUKAN_LIST );
	GFL_STD_MemClear( wk, sizeof(ZKNLISTMAIN_WORK) );

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
static GFL_PROC_RESULT ZukanListProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( ZKNLISTSEQ_MainSeq( mywk ) == FALSE ){
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
static GFL_PROC_RESULT ZukanListProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_LIST );

	OS_Printf( "↑↑↑↑↑　図鑑リスト処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
