//=============================================================================================
/**
 * @file	procsys.c
 * @brief	プロセス管理システム
 * @date	2005.07.25
 */
//=============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "gflib.h"

//#include "pm_overlay.h"

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

	BOOL call_flag;				///<子プロセス呼び出しのリクエストフラグ
	BOOL jump_flag;				///<次のメインプロセスが存在するかどうかのフラグ
	FSOverlayID next_ov_id;		///<次のメインプロセスのオーバーレイID（未使用）
	const GFL_PROC_DATA * next_data;	///<次のメインプロセスの生成データ
	void * next_param;			///<次のメインプロセスのワークへのポインタ
};


//------------------------------------------------------------------
//
//		内部使用関数
//		※外部公開していない
//
//------------------------------------------------------------------
extern GFL_PROC * GFI_PROC_Create(const GFL_PROC_DATA * data, void * parent_work, const u32 heap_id);
extern void GFI_PROC_Delete(GFL_PROC * proc);
extern BOOL GFI_PROC_Main(GFL_PROC * proc);

extern GFL_PROCSYS * GFI_PROC_SysInit(u32 heap_id);
extern void GFI_PROC_SysMain(GFL_PROCSYS * psys);
extern void GFI_PROC_SysExit(GFL_PROCSYS * psys);

extern void GFI_PROC_SysCallProc(GFL_PROCSYS * psys, FSOverlayID ov_id,
		const GFL_PROC_DATA * procdata, void * pwork);
extern void GFI_PROC_SysSetNextProc(GFL_PROCSYS * psys, FSOverlayID ov_id,
		const GFL_PROC_DATA * procdata, void * pwork);

//=============================================================================================
//
//
//		外部公開関数
//
//		内部関数GFI_PROC_～のラッパーとして用意されている
//
//
//=============================================================================================

//------------------------------------------------------------------
/**
 * @brief	PROCシステム用ワーク保持ポインタ
 * 
 * PROCシステムはゲーム中にひとつしか存在しないと考えられるので
 * 第一引数を内部変数として保持しておくように修正
 */
//------------------------------------------------------------------
static GFL_PROCSYS * gfl_procsys;

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysInit(u32 heap_id)
{
	gfl_procsys = GFI_PROC_SysInit(heap_id);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysMain(void)
{
	GFI_PROC_SysMain(gfl_procsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysExit(void)
{
	GFI_PROC_SysExit(gfl_procsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysCallProc(FSOverlayID ov_id, const GFL_PROC_DATA * procdata, void * pwork)
{
	GFI_PROC_SysCallProc(gfl_procsys, ov_id, procdata, pwork);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_PROC_SysSetNextProc(FSOverlayID ov_id, const GFL_PROC_DATA * procdata, void * pwork)
{
	GFI_PROC_SysSetNextProc(gfl_procsys, ov_id, procdata, pwork);
}

//=============================================================================================
//
//		関数
//
//=============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセス管理システム初期化
 * @param	heap_id			使用するヒープのID
 * @return	GFL_PROCSYS		制御ワークへのポインタ
 */
//------------------------------------------------------------------
GFL_PROCSYS * GFI_PROC_SysInit(u32 heap_id)
{
	GFL_PROCSYS * psys = GFL_HEAP_AllocMemory(heap_id, sizeof(GFL_PROCSYS));
	psys->heap_id = heap_id;
	psys->ov_id = NO_OVERLAY_ID;
	psys->proc = NULL;
	psys->call_flag = FALSE;
	psys->jump_flag = FALSE;
	psys->next_ov_id = NO_OVERLAY_ID;
	psys->next_data = NULL;
	psys->next_param = NULL;
	return psys;
}

//------------------------------------------------------------------
/**
 * brief	プロセス管理システムメイン
 * @param	psys
 */
//------------------------------------------------------------------
void GFI_PROC_SysMain(GFL_PROCSYS * psys)
{
	BOOL result;
	if (psys->proc == NULL) {
		//OS_Halt();
		return;
	}

	result = GFI_PROC_Main(psys->proc);

	if (psys->call_flag) {
		GFL_PROC * proc;
		proc = GFI_PROC_Create(psys->next_data, psys->next_param, psys->heap_id);
		proc->parent = psys->proc;
		psys->proc->child = proc;
		psys->proc = proc;
		psys->call_flag = FALSE;
	} else if (result == TRUE) {
		if (psys->jump_flag) {
			//プロセスの分岐を行った場合
			GFL_PROC * parent;
			parent = psys->proc->parent;
			GFI_PROC_Delete(psys->proc);
			psys->proc = GFI_PROC_Create(psys->next_data, psys->next_param, psys->heap_id);
			parent->child = psys->proc;
			psys->proc->parent = parent;
			psys->jump_flag = FALSE;
		} else {
			//単に終了→上位プロセスに戻る場合
			GFL_PROC * parent;
			parent = psys->proc->parent;
			GFI_PROC_Delete(psys->proc);
			psys->proc = parent;
			psys->proc->child = NULL;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	プロセス管理システム終了処理
 * @param	psys
 */
//------------------------------------------------------------------
void GFI_PROC_SysExit(GFL_PROCSYS * psys)
{
	if (psys->proc != NULL) {
		OS_Halt();
	}
	GFL_HEAP_FreeMemory(psys);
}

//------------------------------------------------------------------
/**
 * @brief	プロセス切り替えリクエスト
 * @param	psys
 * @param	procdata
 * @param	pwork
 */
//------------------------------------------------------------------
void GFI_PROC_SysSetNextProc(GFL_PROCSYS * psys, FSOverlayID ov_id, 
		const GFL_PROC_DATA * procdata, void * pwork)
{
	psys->jump_flag = TRUE;
	psys->next_ov_id = ov_id;
	psys->next_data = procdata;
	psys->next_param = pwork;
}

//------------------------------------------------------------------
/**
 * @brief	プロセス呼び出し
 * @param	psys
 * @param	procdata
 * @param	pwork
 */
//------------------------------------------------------------------
void GFI_PROC_SysCallProc(GFL_PROCSYS * psys, FSOverlayID ov_id,
		const GFL_PROC_DATA * procdata, void * pwork)
{
	if (psys->proc == NULL) {
		//一番最初のプロセスの場合
		GFL_PROC * proc;
		proc = GFI_PROC_Create(procdata, pwork, psys->heap_id);
		psys->proc = proc;
	} else {
		//サブプロセスの場合
		psys->call_flag = TRUE;
		psys->next_ov_id = ov_id;
		psys->next_data = procdata;
		psys->next_param = pwork;
	}
}

//------------------------------------------------------------------
/**
 * @brief	プロセス生成処理
 * @param	data		プロセス定義データへのポインタ
 * @param	parent_work	上位から引き渡すワークへのポインタ
 * @param	heap_id		使用するヒープ
 * @return	GFL_PROC	生成したプロセスへのポインタ
 */
//------------------------------------------------------------------
GFL_PROC * GFI_PROC_Create(const GFL_PROC_DATA * data, void * parent_work, const u32 heap_id)
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
 * @brief	プロセスを削除する
 * @param	proc	プロセスへのポインタ
 */
//------------------------------------------------------------------
void GFI_PROC_Delete(GFL_PROC * proc)
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
 * @brief	プロセス動作処理メイン
 * @param	proc	プロセスへのポインタ
 * @retval	TRUE	プロセス動作終了
 * @retval	FALSE	プロセス動作継続中
 */
//------------------------------------------------------------------
BOOL GFI_PROC_Main(GFL_PROC * proc)
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
		result = proc->data.init_func(proc, &proc->subseq, proc->parent_work, proc->work);
		if (result == GFL_PROC_RES_FINISH) {
			proc->proc_seq = SEQ_MAIN;
			proc->subseq = 0;
		}
		break;
	case SEQ_MAIN:
		result = proc->data.main_func(proc, &proc->subseq, proc->parent_work, proc->work);
		if (result == GFL_PROC_RES_FINISH) {
			proc->proc_seq = SEQ_END;
			proc->subseq = 0;
		}
		break;
	case SEQ_END:
		result = proc->data.end_func(proc, &proc->subseq, proc->parent_work, proc->work);
		if (result == GFL_PROC_RES_FINISH) {
			if(proc->overlay_id != NO_OVERLAY_ID){
				//Overlay_UnloadID( proc->overlay_id );
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

