//=============================================================================================
/**
 * @file	tcb_util.c
 * @author	tamada GAME FREAK inc.
 * @date	2007.02.02
 * @brief	TCB + メモリ管理
 *
 */
//=============================================================================================


#include <nitro.h>
#include <nnsys.h>

#include "gflib.h"
//#include "tcb.h"
#include "heapsys.h"

#include "tcbl.h"

//=============================================================================================
//=============================================================================================
//#define TCB_INLINE	inline
#define TCB_INLINE	

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _GFL_TCBL {
	GFL_TCBLSYS * tcbsys;
	GFL_TCBL * prev;
	GFL_TCBL * next;
	u32 pri;
	void * work;
	GFL_TCBLFUNC * func;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _GFL_TCBLSYS {
	u32 tcb_max;
	u32 work_size;
	HEAPID heap_id;
	BOOL add_flag;
	BOOL sucide_flag;
	void  * tcb_array;
	GFL_TCBL tcb_first;
	GFL_TCBL * now_chain;
	GFL_TCBL * stack_top;
};


//=============================================================================================
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static TCB_INLINE GFL_TCBL * GetTCBPtr(GFL_TCBLSYS * tcbsys, u32 offset)
{
	return (GFL_TCBL *)( (u32)(tcbsys->tcb_array) + offset * (sizeof(GFL_TCBL) + tcbsys->work_size) );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static TCB_INLINE GFL_TCBL * PopTCB(GFL_TCBLSYS * tcbsys)
{
	GFL_TCBL * tcb;
	tcb = tcbsys->stack_top;
	if (tcbsys->stack_top == NULL) {
		//Popできない
		return NULL;
	}
	tcbsys->stack_top =tcb->next;
	tcb->next->prev = NULL;

	return tcb;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static TCB_INLINE void PushTCB(GFL_TCBLSYS * tcbsys, GFL_TCBL * tcb)
{
	GFL_TCBL * stack_top;
	stack_top = tcbsys->stack_top;
	tcb->prev = NULL;
	tcb->next = stack_top;
	if (stack_top) {
		stack_top->prev = tcb;
	}
	tcbsys->stack_top = tcb;
	OS_Printf("top:%08x\n", tcbsys->stack_top);
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void ClearTCBWork(GFL_TCBLSYS * tcbsys, GFL_TCBL * tcb)
{
	tcb->tcbsys = tcbsys;
	tcb->prev = &(tcbsys->tcb_first);
	tcb->next = &(tcbsys->tcb_first);
	tcb->pri = 0;
	tcb->work = NULL;
	tcb->func = NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void DeleteTCB(GFL_TCBL * tcb)
{
	tcb->func = NULL;
	//リストから削除
	tcb->prev->next = tcb->next;
	tcb->next->prev = tcb->prev;

	PushTCB(tcb->tcbsys, tcb);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void * GetWork(GFL_TCBL * tcb)
{
	if (tcb->work == NULL) {
		return (u8 *)tcb + sizeof(GFL_TCBL);
	} else {
		return tcb->work;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void InitSysWork( GFL_TCBLSYS * tcbsys )
{
	int i;
	GFL_TCBL * prev = NULL;
	GFL_TCBL * now;

	for (i = 0; i < tcbsys->tcb_max; i++) {
		now = GetTCBPtr(tcbsys, i);
		ClearTCBWork(tcbsys,now);
		PushTCB(tcbsys, now);
	}
}

//=============================================================================================
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GFL_TCBLSYS * GFL_TCBL_SysInit( HEAPID heap_id, u32 task_max, u32 default_work_size)
{
	GFL_TCBLSYS * tcbsys;

	tcbsys = GFL_HEAP_AllocMemory(heap_id, sizeof(GFL_TCBLSYS));
	tcbsys->tcb_max = task_max;
	tcbsys->work_size = default_work_size;
	tcbsys->heap_id = heap_id;
	tcbsys->add_flag = FALSE;
	tcbsys->sucide_flag = FALSE;
	tcbsys->tcb_array = GFL_HEAP_AllocMemory(heap_id, (sizeof(GFL_TCBL) + default_work_size) * task_max);
	tcbsys->stack_top = NULL;
	InitSysWork(tcbsys);
	ClearTCBWork(tcbsys, &(tcbsys->tcb_first));
	tcbsys->now_chain = NULL;
	return tcbsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_TCBL_SysMain( GFL_TCBLSYS * tcbsys )
{
	GFL_TCBL * now;
	GFL_TCBL * head = &(tcbsys->tcb_first);

	tcbsys->now_chain = head->next;
	while (tcbsys->now_chain != head) {
		tcbsys->sucide_flag = FALSE;
		tcbsys->now_chain->func(tcbsys->now_chain, GetWork(tcbsys->now_chain));
		if (tcbsys->sucide_flag) {
			//自殺した場合
			GFL_TCBL * next = tcbsys->now_chain->next;
			DeleteTCB(tcbsys->now_chain);
			tcbsys->now_chain = next;
		} else {
			tcbsys->now_chain = tcbsys->now_chain->next;
		}
	}

}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_TCBL_SysExit( GFL_TCBLSYS * tcbsys )
{
	//実行するべきタスクが残っていたらエラーにする
	if(tcbsys->tcb_first.next != &(tcbsys->tcb_first)) {
		OS_Printf("%08X != %08X\n", tcbsys->tcb_first.next, &(tcbsys->tcb_first));
		GF_ASSERT(0);
	}
	GFL_HEAP_FreeMemory(tcbsys->tcb_array);
	GFL_HEAP_FreeMemory(tcbsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_TCBL_Delete(GFL_TCBL * tcb)
{
	GF_ASSERT(tcb->func != NULL);
	if (tcb->tcbsys->now_chain == tcb) {
		//自殺の場合はリンクのつなぎ替えをシステムに任せる
		tcb->tcbsys->sucide_flag = TRUE;
		return;
	} else {
		//他殺の場合はその場で処理
		DeleteTCB(tcb);
	}
}


//------------------------------------------------------------------
//------------------------------------------------------------------
GFL_TCBL * GFL_TCBL_Create(GFL_TCBLSYS * tcbsys, GFL_TCBLFUNC * func, u32 work_size, u32 pri)
{
	GFL_TCBL * get;
	GFL_TCBL * find;
	GFL_TCBL * head = &(tcbsys->tcb_first);

	get = PopTCB(tcbsys);
	if (get == NULL) {
		//TCBが足りなくなった場合
		return NULL;
	}
	get->pri = pri;
	get->func = func;
	if (work_size <= tcbsys->work_size) {
		get->work = NULL;
	} else {
		get->work = GFL_HEAP_AllocMemory(tcbsys->heap_id, work_size);
	}

	//実行リストに接続(同じプライオリティの時には、先に登録したほうが先)
	for (find = head->next; find != head; find = find->next) {
		if (find->pri > get->pri) {
			get->prev = find->prev;
			get->next = find;
			find->prev->next = get;
			find->prev = get;
			return get;
		}
	}
	//最後まで繋ぐところが見つからなければ最後に繋ぐ
	/*
	get->prev = head->prev;		//今までの最後
	get->next = head;			//自分が最後
	head->prev->next = get;		//今まで最後だったブロックの次は自分
	head->prev = get;			//リストの最後を更新
	*/
	get->prev = tcbsys->tcb_first.prev;	//今までの最後
	get->next = &(tcbsys->tcb_first);	//自分が最後
	tcbsys->tcb_first.prev->next = get;	//今まで最後だったブロックの次は自分
	tcbsys->tcb_first.prev = get;		//リストの最後を更新
	return get;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void * GFL_TCBL_GetWork(GFL_TCBL * tcb)
{
	return GetWork(tcb);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u32 GFL_TCBL_GetPriority(const GFL_TCBL * tcb)
{
	return tcb->pri;
}
