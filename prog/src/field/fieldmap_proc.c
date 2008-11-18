//============================================================================================
/**
 * @file	fieldmap_proc.c
 * @brief	フィールドプロセス 
 * @author	tamada
 * @date	2008.11.18
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"				//HEAPID参照のため

#include "gamesystem/gamesystem.h"
#include "field/fieldmap.h"

//============================================================================================
//
//
//		プロセスの定義
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	FIELD_MAIN_WORK * fieldWork;
}FPROC_WORK;
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FieldMapProcInit
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FIELD_MAIN_WORK * fieldWork;
	FPROC_WORK * fpwk;
	GAMESYS_WORK * gsys = pwk;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_FIELDMAP, 0x200000 );
	fpwk = GFL_PROC_AllocWork(proc, sizeof(FPROC_WORK), HEAPID_FIELDMAP);
	fpwk->fieldWork = FIELDMAP_Create(gsys, HEAPID_FIELDMAP );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FieldMapProcMain
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAMESYS_WORK * gsys = pwk;
	FPROC_WORK * fpwk = mywk;
	if( FIELDMAP_Main(gsys, fpwk->fieldWork) == TRUE ){
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
static GFL_PROC_RESULT FieldMapProcEnd
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FPROC_WORK * fpwk = mywk;
	FIELDMAP_Delete(fpwk->fieldWork);
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap( HEAPID_FIELDMAP );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA FieldMapProcData = {
	FieldMapProcInit,
	FieldMapProcMain,
	FieldMapProcEnd,
};




