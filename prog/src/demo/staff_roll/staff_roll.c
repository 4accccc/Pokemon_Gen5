//============================================================================================
/**
 * @file		staff_roll.c
 * @brief		エンディング・スタッフロール画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.19
 *
 *	モジュール名：STAFFROLL
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "staff_roll_main.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT StaffRollProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT StaffRollProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT StaffRollProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA STAFFROLL_ProcData = {
	StaffRollProc_Init,
	StaffRollProc_Main,
	StaffRollProc_End
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
static GFL_PROC_RESULT StaffRollProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SRMAIN_WORK * wk;

//	OS_Printf( "↓↓↓↓↓　スタッフロール処理開始　↓↓↓↓↓\n" );

//	OS_Printf( "heap size [0] = 0x%x\n", GFL_HEAP_GetHeapFreeSize(GFL_HEAPID_APP) );
	// OS_Printfの結果、0x1307d4 空いてる ( 2010/05/10 13:00 )
	// GFL_HEAPID_APPが5000h減ったので、-5000h ( 2010/05/12 15:30 )
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_STAFF_ROLL, 0x125000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(SRMAIN_WORK), HEAPID_STAFF_ROLL );
	GFL_STD_MemClear( wk, sizeof(SRMAIN_WORK) );

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
static GFL_PROC_RESULT StaffRollProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( SRMAIN_Main( mywk ) == FALSE ){
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
static GFL_PROC_RESULT StaffRollProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_STAFF_ROLL );

//	OS_Printf( "↑↑↑↑↑　スタッフロール処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}
