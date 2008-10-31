//============================================================================================
/**
 * @file	field_proc.c
 * @brief	フィールドプロセス (watanabe080926
 * @author	tamada
 * @date	2007.02.01
 */
//============================================================================================
#include <gflib.h>
#include "system\main.h"

#include "field_main.h"
#include "gamesystem/gamesystem.h"

//============================================================================================
//
//
//		プロセスの定義
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugFieldProcInit
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAMESYS_WORK * gsys = pwk;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WATANABE_DEBUG, 0x200000 );
	FieldBoot(gsys, HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugFieldProcMain
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( FieldMain() == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugFieldProcEnd
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FieldEnd();
	GFL_HEAP_DeleteHeap( HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugFieldProcData = {
	DebugFieldProcInit,
	DebugFieldProcMain,
	DebugFieldProcEnd,
};




