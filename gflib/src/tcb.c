//=============================================================================================
/**
 * @file	tcb.c
 * @brief	汎用タスク制御モジュール本体
 * @author	tamada, taya	(GAME FREAK Inc.)
 * @date	2006.11.06
 * @since	2003.01.20
 */
//=============================================================================================

#include <nitro.h>
#include <nnsys.h>
#include "include/gflib.h"
#include "include/tcb.h"
//#include "include/heapsys.h"

//=============================================================================================
//	
//=============================================================================================
#define TASK_ADDPROC_CHECK		///< このフラグが有効だと、TCB_Add 動作中に割り込みでTCB_Mainが呼ばれた時、処理を行わない。



//=============================================================================================
//	定義
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum{
	TCB_ENABLE = 0,
	TCB_DISABLE = 1,
};

//------------------------------------------------------------------
/** @brief	TCB構造体の定義
 *
 * TCB構造体は、動作優先順位に従ってリンクされる。
 * そのため双方向リンクを構成するように前後へのポインタを持つ。
 *
 * また、それぞれの実際の処理に必要なワークエリアは持たず、
 * TCB登録時に使用するワークエリアを引数として渡す。
 *
 * ユーザーレベルでは参照する必要のないメンバへのアクセスを制限するため、
 * この構造体はモジュール内からしかアクセスできない。
 * 必要な部分はアクセス関数を経由するようになっている。
 */
//------------------------------------------------------------------
struct _GFL_TCB {
	GFL_TCBSYS* sys;				///<自分を管理しているTCBシステムへのポインタ
	GFL_TCB * prev;				///<前のTCBへのポインタ
	GFL_TCB * next;				///<続くTCBへのポインタ
	u32 pri;					///<プライオリティ
	void * work;				///<ワークエリアへのポインタ
	GFL_TCB_FUNC * func;			///<実行関数へのポインタ

	u32 sw_flag;				///<関数追加フラグ
};	// 52 bytes


//------------------------------------------------------------------
/**
 * @brief	TCB制御構造体の定義
 */
//------------------------------------------------------------------
struct _GFL_TCBSYS {
	u32			tcb_max;		///< 登録可能なTCBの最大数
	u32			tcb_stack_ptr;	///< 自信が抱えているTCB用スタックのポインタ
	GFL_TCB			tcb_first;		///< TCB先頭実体
	GFL_TCB **	tcb_stack;		///< TCB用スタック
	GFL_TCB*		tcb_table;		///< TCB実体テーブル
	BOOL		adding_flag;	///< TCB追加処理実行中フラグ（このフラグが立っている間にMainを実行しない）


	GFL_TCB *		now_chain;		///< メインループ制御内使用連結整合維持TCBポインタ
	GFL_TCB *		next_chain;		///< メインループ制御内使用連結整合維持TCBポインタ
};	// 68 bytes


//==============================================================
// Prototype
//==============================================================
static inline void TCB_WorkClear( GFL_TCBSYS* tcbsys, GFL_TCB * tcb);
static void InitTCBStack( GFL_TCBSYS* tcbsys );
static GFL_TCB * PopTCB( GFL_TCBSYS* tcbsys );
static BOOL PushTCB( GFL_TCBSYS* tcbsys, GFL_TCB * tcb );
static GFL_TCB * AddTask( GFL_TCBSYS* tcbsys, GFL_TCB_FUNC * func, void* work, u32 pri );

//------------------------------------------------------------------
/**
 *	TCBシステム使用前の初期化
 */
//------------------------------------------------------------------
static void TCB_Init( GFL_TCBSYS* tcbsys );



//=============================================================================================
//
//	ローカル関数
//
//=============================================================================================
//------------------------------------------------------------------
/**
 * @brief	TCB構造体の初期化
 *
 * TCBのメンバ変数を初期化する
 *
 * @param	tcb	TCBへのポインタ
 * @return	none	
 */
//------------------------------------------------------------------
static inline void TCB_WorkClear( GFL_TCBSYS* tcbsys, GFL_TCB * tcb)
{
	tcb->sys = tcbsys;

	tcb->prev = 
	tcb->next = &(tcbsys->tcb_first);

	tcb->pri = 0;
	tcb->work = NULL;
	tcb->func = NULL;
}
//------------------------------------------------------------------
/**
 * @brief	TCBスタック初期化
 *
 * 未使用TCBを保持するスタックを初期化する
 */
//------------------------------------------------------------------
static void InitTCBStack( GFL_TCBSYS* tcbsys )
{
	int i;

	for( i=0; i<tcbsys->tcb_max; i++ )
	{
		TCB_WorkClear( tcbsys, &(tcbsys->tcb_table[i]) );
		tcbsys->tcb_stack[i] = tcbsys->tcb_table + i;
	}
	tcbsys->tcb_stack_ptr = 0;
}

//------------------------------------------------------------------
/**
 * @brief	TCBスタックからの取り出し
 *
 * @retval	NULL以外	TCBへのポインタ
 * @retval	NULL		取り出しに失敗（スタックが空だった場合）
 */
//------------------------------------------------------------------
static GFL_TCB * PopTCB( GFL_TCBSYS* tcbsys )
{
	GFL_TCB * tcb;
	if(tcbsys->tcb_stack_ptr == tcbsys->tcb_max)
	{
		return NULL;
	}
	tcb = tcbsys->tcb_stack[ tcbsys->tcb_stack_ptr ];
	tcbsys->tcb_stack_ptr++;
	return tcb;
}

//------------------------------------------------------------------
/**
	@brief	TCBスタックへのプッシュ

	@param	tcb		プッシュするTCBのポインタ
	@retval	TRUE	成功
	@retval	FALSE	失敗（スタックがいっぱいの場合）
*/
//------------------------------------------------------------------
static BOOL PushTCB( GFL_TCBSYS* tcbsys, GFL_TCB * tcb )
{
	if(tcbsys->tcb_stack_ptr == 0)
	{
		return FALSE;
	}
	TCB_WorkClear( tcbsys, tcb );	//値をクリアしてからスタックに積む
	tcbsys->tcb_stack_ptr--;
	tcbsys->tcb_stack[ tcbsys->tcb_stack_ptr ] = tcb;

	return TRUE;
}


//=============================================================================================
//
//	公開関数
//
//=============================================================================================

//------------------------------------------------------------------
/**
 * TCB システムを構築するのに必要なメモリ量を計算
 *
 * @param   task_max		稼働できる最大タスク数
 *
 * @retval  u32		メモリサイズ（バイト単位）
 */
//------------------------------------------------------------------
u32 GFL_TCB_CalcSystemWorkSize( u32 task_max )
{
	return sizeof(GFL_TCBSYS) + (sizeof(GFL_TCB *)+sizeof(GFL_TCB)) * task_max;
}

//------------------------------------------------------------------
/**
 * TCBシステムを作成
 *
 * @param   task_max		稼働できる最大タスク数
 * @param   work_area		システム作成に必要充分なサイズのワークエリアアドレス
 *
 * @retval  GFL_TCBSYS*		作成されたTCBシステムポインタ
 *
 * work_area に必要なサイズは、GFL_TCB_CalcSystemWorkSize で計算する。
 */
//------------------------------------------------------------------
GFL_TCBSYS*  GFL_TCB_Init( u32 task_max, void* work_area )
{
	GFL_TCBSYS* tcbsys;

	GF_ASSERT( work_area );

	tcbsys = work_area;

	tcbsys->tcb_stack = (GFL_TCB **) ((u8*)(tcbsys) + sizeof(GFL_TCBSYS));
	tcbsys->tcb_table = (GFL_TCB*) ((u8*)(tcbsys->tcb_stack) + sizeof(GFL_TCB *)*task_max);

	tcbsys->tcb_max = task_max;
	tcbsys->tcb_stack_ptr = 0;
	tcbsys->adding_flag = FALSE;

	TCB_Init( tcbsys );

	return tcbsys;
}

//------------------------------------------------------------------
/**
 *	システム使用前の初期化
 */
//------------------------------------------------------------------
static void TCB_Init( GFL_TCBSYS* tcbsys )
{
	//スタックの初期化
	InitTCBStack( tcbsys );
	//先頭ブロックの初期化
	TCB_WorkClear( tcbsys, &tcbsys->tcb_first );

	tcbsys->now_chain = tcbsys->tcb_first.next;
}

//------------------------------------------------------------------
//	TCBメイン
//------------------------------------------------------------------
void GFL_TCB_Main( GFL_TCBSYS* tcbsys )
{
	#ifdef TASK_ADDPROC_CHECK
	if( tcbsys->adding_flag )
	{
		return;
	}
	#endif

	tcbsys->now_chain = tcbsys->tcb_first.next;	//ここが実行の始めの場所
	while (tcbsys->now_chain != &(tcbsys->tcb_first))
	{
		//実行関数内でブロックが削除された時用にアドレスを保存
		tcbsys->next_chain = tcbsys->now_chain->next;

		if(tcbsys->now_chain->sw_flag == TCB_ENABLE){//登録直後の動作を避ける
			if(tcbsys->now_chain->func != NULL){
				tcbsys->now_chain->func(tcbsys->now_chain, tcbsys->now_chain->work);
			}
		}else{
			tcbsys->now_chain->sw_flag = TCB_ENABLE;
		}
		tcbsys->now_chain = tcbsys->next_chain;	//次の連結へ
	}
	tcbsys->now_chain->func = NULL;
}

//------------------------------------------------------------------
/**
 *	@brief	TCBシステム終了
 *  @param	tcbsys		TCBシステムワークポインタ
 */
//------------------------------------------------------------------
void GFL_TCB_Exit( GFL_TCBSYS* tcbsys )
{
#if 0
	tcbsys->now_chain = tcbsys->tcb_first.next;	//ここが実行の始めの場所
	while (tcbsys->now_chain != &(tcbsys->tcb_first))
	{
		//実行関数内でブロックが削除された時用にアドレスを保存
		tcbsys->next_chain = tcbsys->now_chain->next;

		if(tcbsys->now_chain->sw_flag == TCB_ENABLE){//登録直後の動作を避ける
			if(tcbsys->now_chain->func != NULL){
				tcbsys->now_chain->func(tcbsys->now_chain, tcbsys->now_chain->work);
			}
		}else{
			tcbsys->now_chain->sw_flag = TCB_ENABLE;
		}
		tcbsys->now_chain = tcbsys->next_chain;	//次の連結へ
	}
	tcbsys->now_chain->func = NULL;
#endif
}

//------------------------------------------------------------------------------
/**
	@brief	TCBを追加する
	@param	tcbsys	TCBシステムポインタ
	@param	func	GFL_TCB_FUNC:関連付ける実行関数ポインタ
	@param	work	void*:関連付けるワークエリアへのvoid型ポインタ
	@param	pri		u32:タスクプライオリティ

	@return	TCB *	追加したTCBを示すポインタ
*/
//------------------------------------------------------------------------------
GFL_TCB * GFL_TCB_AddTask( GFL_TCBSYS* tcbsys, GFL_TCB_FUNC * func, void* work, u32 pri )
{
	GFL_TCB *  ret;

	tcbsys->adding_flag = TRUE;
	ret = AddTask( tcbsys, func, work, pri );
	tcbsys->adding_flag = FALSE;
	return ret;

}
//------------------------------------------------------------------
/**
 * TCB追加処理
 *
 * @param   tcbsys		TCBシステムポインタ
 * @param   func		TCBに関連付ける関数ポインタ
 * @param   work		TCBに関連付けるワークエリアへのポインタ
 * @param   pri			タスクプライオリティ
 *
 * @retval  GFL_TCB *		追加したTCBポインタ
 */
//------------------------------------------------------------------
static GFL_TCB * AddTask( GFL_TCBSYS* tcbsys, GFL_TCB_FUNC * func, void* work, u32 pri )
{
	GFL_TCB * get;
	GFL_TCB * find;

	get = PopTCB( tcbsys );	//空いているTCBを拾う
	if(get == NULL)
	{
		//登録しようとしたがタスクがなかった
		OS_TPrintf("ERR : TCB wasn't get!");
		return NULL;
	}

	//パラメータをセット
	get->pri = pri;
	get->work = work;
	get->func = func;
	//動作フラグ設定
	if(tcbsys->now_chain->func != NULL){
		//動作フラグ設定(別のタスクから追加されようとしている場合、登録直後の動作を避ける処理)
		if(tcbsys->now_chain->pri <= pri){
			get->sw_flag = TCB_DISABLE;
		}else{
			get->sw_flag = TCB_ENABLE;
		}
	}
	else{
		//本タスク外からの登録
		get->sw_flag = TCB_ENABLE;
	}

	//実行リストに接続(同じプライオリティの時には、先に登録したほうが先)
	for(find = tcbsys->tcb_first.next; find != &(tcbsys->tcb_first); find = find->next)
	{
		if(find->pri > get->pri){	//findの前にgetをつなげる
			get->prev = find->prev;
			get->next = find;
			find->prev->next = get;
			find->prev = get;
			if(find == tcbsys->next_chain){
				tcbsys->next_chain = get;
			}
			return get;
		}
	}
	//最後まで繋ぐところが見つからなければ最後に繋ぐ
	if(tcbsys->next_chain == &(tcbsys->tcb_first)){
		tcbsys->next_chain = get;
	}
	get->prev = tcbsys->tcb_first.prev;	//今までの最後
	get->next = &(tcbsys->tcb_first);	//自分が最後
	tcbsys->tcb_first.prev->next = get;	//今まで最後だったブロックの次は自分
	tcbsys->tcb_first.prev = get;		//リストの最後を更新
	return get;
}

//------------------------------------------------------------------------------
/**
	@brief	TCBを消去する
	@param	tcb		TCBポインタ
*/
//------------------------------------------------------------------------------
void GFL_TCB_DeleteTask( GFL_TCB * tcb )
{
	//連結システムの次のリンク先が削除対象の場合の処理
	if(tcb->sys->next_chain == tcb){
		tcb->sys->next_chain = tcb->next;	//削除対象TCBの次TCBへ連結情報を移動
	}
	//リストから削除
	tcb->prev->next = tcb->next;
	tcb->next->prev = tcb->prev;

	PushTCB( tcb->sys, tcb );
}


//==================================================================
//	TCBアクセス関数
//==================================================================

//------------------------------------------------------------------
//TCBの動作関数を切り替える
//------------------------------------------------------------------
void GFL_TCB_ChangeFunc(GFL_TCB * tcb, GFL_TCB_FUNC * func)
{
	tcb->func = func;
}

//------------------------------------------------------------------
//TCBのワークアドレス取得
//------------------------------------------------------------------
void * GFL_TCB_GetWork(GFL_TCB * tcb)
{
	return tcb->work;
}

//------------------------------------------------------------------
//TCBプライオリティの取得
//------------------------------------------------------------------
u32 GFL_TCB_GetPriority(const GFL_TCB * tcb)
{
	return tcb->pri;
}







