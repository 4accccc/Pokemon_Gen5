//=============================================================================
/**
 * @file	procsys.c
 * @brief	プロセス管理システム
 * @date	2005.07.25
 */
//=============================================================================

#include <nitro.h>
#include <nnsys.h>
//#include "common.h"
//#include "system\pm_overlay.h"

#include "gflib.h"

#include "procsys.h"

#include "heapsys.h"


//------------------------------------------------------------------
/**
 * @brief	プロセス内部の動作シーケンス
 */
//------------------------------------------------------------------
enum {
	SEQ_OVERLAY_LOAD = 0,
	SEQ_INIT,
	SEQ_MAIN,
	SEQ_END,
	SEQ_OVERLAY_UNLOAD
};


//------------------------------------------------------------------
/**
 * @brief	プロセス構造体定義
 */
//------------------------------------------------------------------
struct _GFL_PROC{
	GFL_PROCSYS * sys;
	GFL_PROC_DATA data;				///<プロセス定義データへのポインタ
	FSOverlayID overlay_id;
	int proc_seq;				///<プロセス内部シーケンス
	int subseq;					///<プロセス関数の動作シーケンス
	void * parent_work;			///<上位ワークへのポインタ
	void * work;				///<プロセス内部ワークへのポインタ
	GFL_PROC * parent;				///<上位プロセスへのポインタ
	GFL_PROC * child;				///<下位プロセスへのポインタ
};

//------------------------------------------------------------------
/**
 * @brief	プロセスシステム構造体定義
 */
//------------------------------------------------------------------
struct _GFL_PROCSYS{
	u32 heap_id;				///<使用するヒープ指定
	FSOverlayID ov_id;			///<現在のメインプロセスのオーバーレイID（未使用）
	GFL_PROC * proc;			///<現在のメインプロセスのポインタ

	BOOL next_flag;				///<次のメインプロセスが存在するかどうかのデータ
	GFL_PROC_DATA next_data;	///<次のメインプロセスの生成データ
	void * next_param;			///<次のメインプロセスのワークへのポインタ
	FSOverlayID next_ov_id;		///<次のメインプロセスのオーバーレイID（未使用）
};


//===========================================================================
//
//		関数
//
//===========================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GFL_PROCSYS * GFL_PROC_SysInit(u32 heap_id)
{
	GFL_PROCSYS * psys = GFL_HEAP_AllocMemory(heap_id, sizeof(GFL_PROCSYS));
	psys->proc = NULL;
	psys->heap_id = heap_id;
	return psys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysMain(GFL_PROCSYS * psys)
{
	BOOL result;
	if (psys->proc == NULL) {
		OS_Halt();
		return;
	}

	result = GFL_PROC_Main(psys->proc);

	if (result == TRUE) {
		if (psys->next_flag == FALSE) {
			GFL_PROC * parent;
			parent = psys->proc->parent;
			GFL_PROC_Delete(psys->proc);
			psys->proc = parent;
			psys->proc->child = NULL;
		} else {
			psys->next_flag = FALSE;
			GFL_PROC_Delete(psys->proc);
			psys->proc = GFL_PROC_Create(&psys->next_data, psys->next_param, psys->heap_id);
		}
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysExit(GFL_PROCSYS * psys)
{
	if (psys->proc != NULL) {
		OS_Halt();
	}
	GFL_HEAP_FreeMemory(psys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysSetNextProc(GFL_PROCSYS * psys, const GFL_PROC_DATA * procdata, void * param)
{
	psys->next_flag = TRUE;
	psys->next_data = *procdata;
	psys->next_param = param;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysCallProc(GFL_PROCSYS * psys, const GFL_PROC_DATA * procdata, void * param)
{
	GFL_PROC * proc;
	GFL_PROC * parent;
	proc = GFL_PROC_Create(procdata, param, psys->heap_id);

	if (psys->proc == NULL) {
		//一番最初のプロセスの場合
		psys->proc = proc;
	} else {
		//サブプロセスの場合
		proc->parent = psys->proc;
		psys->proc->child = proc;
		psys->proc = proc;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysCallNextProc(GFL_PROCSYS * psys, GFL_PROC * proc)
{
	if (psys->proc) {
		psys->proc->child = proc;
	}
	psys->proc = proc;
}

//------------------------------------------------------------------
/**
 * @brief	プロセス生成処理
 * @param	func		プロセス動作関数
 * @param	parent_work	上位から引き渡すワークへのポインタ
 * @param	heap_id	使用するヒープ
 * @return	PROC		生成したプロセスへのポインタ
 */
//------------------------------------------------------------------
GFL_PROC * GFL_PROC_Create(const GFL_PROC_DATA * data, void * parent_work, const u32 heap_id)
{
	GFL_PROC * proc;
	proc = GFL_HEAP_AllocMemory(heap_id, sizeof(GFL_PROC));
	proc->data = *data;
	proc->overlay_id = NO_OVERLAY_ID;
	proc->proc_seq = 0;
	proc->subseq = 0;
	proc->parent_work = parent_work;
	proc->work = NULL;
	proc->parent = NULL;
	proc->child = NULL;
	return proc;
}

//------------------------------------------------------------------
/**
 * @brief	子プロセスを生成する
 * @param	proc		現在の動作プロセスへのポインタ
 * @param	func		プロセス動作関数
 * @param	parent_work	上位から引き渡すワークへのポインタ
 * @param	heap_id	使用するヒープ
 * @return	PROC		生成したプロセスへのポインタ
 */
//------------------------------------------------------------------
GFL_PROC * GFL_PROC_CreateChild(GFL_PROC * proc, const GFL_PROC_DATA * data, void * parent_work, const u32 heap_id)
{
	GFL_PROC * child;
	child = GFL_PROC_Create(data, parent_work, heap_id);
	proc->child = child;
	child->parent = proc;
	return child;
}
//------------------------------------------------------------------
/**
 * @brief	プロセスを削除する
 * @param	proc	プロセスへのポインタ
 */
//------------------------------------------------------------------
void GFL_PROC_Delete(GFL_PROC * proc)
{
	SDK_ASSERT(proc->work == NULL);
	GFL_HEAP_FreeMemory(proc);
}

//------------------------------------------------------------------
/**
 * @brief	プロセス内ワークの確保
 * @param	proc	プロセスへのポインタ
 * @param	size	確保するワークサイズ
 * @param	heap_id	使用するヒープ
 * @return	void *	確保したプロセス内ワークへのポインタ
 */
//------------------------------------------------------------------
void * GFL_PROC_AllocWork(GFL_PROC * proc, unsigned int size, u32 heap_id)
{
	proc->work = GFL_HEAP_AllocMemory(heap_id, size);
	return proc->work;
}

//------------------------------------------------------------------
/**
 * @brief	プロセス内ワークのポインタ取得
 * @param	proc	プロセスへのポインタ
 * @return	void *	プロセス内ワークへのポインタ
 */
//------------------------------------------------------------------
void * GFL_PROC_GetWork(GFL_PROC * proc)
{
	SDK_ASSERT(proc->work != NULL);
	return proc->work;
}

//------------------------------------------------------------------
/**
 * @brief	プロセス内ワークの解放
 * @param	proc	プロセスへのポインタ
 */
//------------------------------------------------------------------
void GFL_PROC_FreeWork(GFL_PROC * proc)
{
	SDK_ASSERT(proc->work != NULL);
	GFL_HEAP_FreeMemory(proc->work);
	proc->work = NULL;
}

//------------------------------------------------------------------
/**
 * @brief	上位ワークへのポインタ取得
 * @param	proc	プロセスへのポインタ
 * @return	void *	上位ワークへのポインタ
 */
//------------------------------------------------------------------
void * GFL_PROC_GetParentWork(GFL_PROC * proc)
{
	SDK_ASSERT(proc->parent_work != NULL);
	return proc->parent_work;
}

//------------------------------------------------------------------
/**
 * @brief	プロセス動作処理メイン
 * @param	proc	プロセスへのポインタ
 * @retval	TRUE	プロセス動作終了
 * @retval	FALSE	プロセス動作継続中
 */
//------------------------------------------------------------------
BOOL GFL_PROC_Main(GFL_PROC * proc)
{
	GFL_PROC_RESULT result;

	switch (proc->proc_seq) {
	case SEQ_OVERLAY_LOAD:
		if(proc->overlay_id != NO_OVERLAY_ID){
			//TPRINTF("OVERLAY ID = %d\n", proc->overlay_id);
			//Overlay_Load( proc->overlay_id, OVERLAY_LOAD_NOT_SYNCHRONIZE);
		}
		proc->proc_seq = SEQ_INIT;

		/* fallthru */
			
	case SEQ_INIT:
		result = proc->data.init_func(proc, &proc->subseq);
		if (result == PROC_RES_FINISH) {
			proc->proc_seq = SEQ_MAIN;
			proc->subseq = 0;
		}
		break;
	case SEQ_MAIN:
		result = proc->data.main_func(proc, &proc->subseq);
		if (result == PROC_RES_FINISH) {
			proc->proc_seq = SEQ_END;
			proc->subseq = 0;
		}
		break;
	case SEQ_END:
		result = proc->data.end_func(proc, &proc->subseq);
		if (result == PROC_RES_FINISH) {
			if(proc->overlay_id != NO_OVERLAY_ID){
				//Overlay_UnloadID( proc->overlay_id );
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}


