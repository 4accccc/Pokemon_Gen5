//============================================================================================
/**
 * @file		zukantop.c
 * @brief		図鑑トップ画面
 * @author	Hiroyuki Nakamura
 * @date		10.02.20
 *
 *	モジュール名：ZUKANTOP
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "zkntop_main.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT ZukanTopProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanTopProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanTopProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA ZUKANTOP_ProcData = {
	ZukanTopProc_Init,
	ZukanTopProc_Main,
	ZukanTopProc_End
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
static GFL_PROC_RESULT ZukanTopProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZKNTOPMAIN_WORK * wk;

	OS_Printf( "↓↓↓↓↓　図鑑トップ処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_TOP, 0x60000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(ZKNTOPMAIN_WORK), HEAPID_ZUKAN_TOP );
	GFL_STD_MemClear( wk, sizeof(ZKNTOPMAIN_WORK) );

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
static GFL_PROC_RESULT ZukanTopProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( ZKNTOPMAIN_MainSeq( mywk ) == FALSE ){
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
static GFL_PROC_RESULT ZukanTopProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_TOP );

	OS_Printf( "↑↑↑↑↑　図鑑トップ処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
