//============================================================================================
/**
 * @file		zukan.c
 * @brief		図鑑画面
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZUKAN
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "app/zukan.h"

#include "zukan_common.h"
#include "list/zukanlist.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	SEQ_LIST_CALL = 0,		// リスト呼び出し
	SEQ_LIST_END,					// リスト終了後
	SEQ_INFO_CALL,				// 情報呼び出し
	SEQ_INFO_END,					// 情報終了後
	SEQ_MAP_CALL,					// 分布呼び出し
	SEQ_MAP_END,					// 分布終了後
	SEQ_VOICE_CALL,				// 鳴き声呼び出し
	SEQ_VOICE_END,				// 鳴き声終了後
	SEQ_FORM_CALL,				// 姿呼び出し
	SEQ_FORM_END,					// 姿終了後
	SEQ_SEARCH_CALL,			// 検索呼び出し
	SEQ_SEARCH_END,				// 検索終了後
	SEQ_PROC_FINISH,			// 図鑑終了
};

typedef struct {
	ZUKAN_PARAM * prm;
	u16	defaultList[MONSNO_END];
	u16 * list;
	int	seq;
	void * work;
}ZUKAN_MAIN_WORK;

typedef int (*pZUKAN_FUNC)(ZUKAN_MAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT ZukanProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void MakeDefaultList( ZUKAN_MAIN_WORK * wk );

static int MainSeq_CallList( ZUKAN_MAIN_WORK * wk );
static int MainSeq_EndList( ZUKAN_MAIN_WORK * wk );



//============================================================================================
//	グローバル
//============================================================================================

// PROCデータ
const GFL_PROC_DATA ZUKAN_ProcData = {
	ZukanProc_Init,
	ZukanProc_Main,
	ZukanProc_End
};

// メインシーケンス関数テーブル
static const pZUKAN_FUNC MainSeq[] = {
	MainSeq_CallList,			// リスト呼び出し
	MainSeq_EndList,			// リスト終了後

	NULL,	// 情報呼び出し
	NULL,	// 情報終了後
	NULL,	// 分布呼び出し
	NULL,	// 分布終了後
	NULL,	// 鳴き声呼び出し
	NULL,	// 鳴き声終了後
	NULL,	// 姿呼び出し
	NULL,	// 姿終了後
	NULL,	// 検索呼び出し
	NULL,	// 検索終了後
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
static GFL_PROC_RESULT ZukanProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZUKAN_MAIN_WORK * wk;

	OS_Printf( "↓↓↓↓↓　図鑑処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_SYS, 0x1000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(ZUKAN_MAIN_WORK), HEAPID_ZUKAN_SYS );
	GFL_STD_MemClear( wk, sizeof(ZUKAN_MAIN_WORK) );

	wk->prm = pwk;

	ZKNCOMM_MakeDefaultList( wk->prm->savedata, wk->defaultList );

	if( wk->prm->callMode == ZUKAN_MODE_LIST ){
		wk->list = wk->defaultList;
		wk->seq = SEQ_LIST_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_INFO ){
		wk->seq = SEQ_INFO_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_MAP ){
		wk->seq = SEQ_MAP_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_VOICE ){
		wk->seq = SEQ_VOICE_CALL;
	}else if( wk->prm->callMode == ZUKAN_MODE_FORM ){
		wk->seq = SEQ_FORM_CALL;
	}else{
		wk->seq = SEQ_SEARCH_CALL;
	}

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
static GFL_PROC_RESULT ZukanProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	ZUKAN_MAIN_WORK * wk = mywk;

	if( wk->seq == SEQ_PROC_FINISH ){
		return GFL_PROC_RES_FINISH;
	}

	wk->seq = MainSeq[wk->seq]( wk );

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
static GFL_PROC_RESULT ZukanProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_SYS );

	OS_Printf( "↑↑↑↑↑　図鑑処理終了　↑↑↑↑↑\n" );

	return GFL_PROC_RES_FINISH;
}



static int MainSeq_CallList( ZUKAN_MAIN_WORK * wk )
{
	ZUKANLIST_DATA * list;

	wk->work = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_SYS, sizeof(ZUKANLIST_DATA) );
	list = wk->work;

	list->gamedata = wk->prm->gamedata;
	list->savedata = wk->prm->savedata;
	list->list = wk->list;

	GFL_PROC_SysCallProc( FS_OVERLAY_ID(zukan_list), &ZUKANLIST_ProcData, wk->work );

	return SEQ_LIST_END;
}

static int MainSeq_EndList( ZUKAN_MAIN_WORK * wk )
{
	ZUKANLIST_DATA * list;
	int	ret;

	list = wk->work;

	switch( list->retMode ){
	case ZKNLIST_RET_EXIT:		// 図鑑終了
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNLIST_RET_EXIT_X:	// 図鑑を終了してメニューを閉じる
		wk->prm->retMode = ZUKAN_RET_MENU_CLOSE;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNLIST_RET_INFO:		// 詳細画面へ
//		ret = SEQ_INFO_CALL;
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
		break;

	case ZKNLIST_RET_SEARCH:	// 検索画面へ
//		ret = SEQ_SEARCH_CALL;
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
		break;

	default:
		wk->prm->retMode = ZUKAN_RET_NORMAL;
		ret = SEQ_PROC_FINISH;
	}

	GFL_HEAP_FreeMemory( wk->work );

	return ret;
}
