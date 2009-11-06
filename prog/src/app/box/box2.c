//============================================================================================
/**
 * @file		box2.c
 * @brief		ボックス画面
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "app/box2.h"

/*
#include "savedata/config.h"
*/

#include "box2_seq.h"
#include "box2_main.h"


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT Box2Proc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT Box2Proc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT Box2Proc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//	グローバル変数
//============================================================================================
const GFL_PROC_DATA BOX2_ProcData = {
	Box2Proc_Init,
	Box2Proc_Main,
	Box2Proc_End
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
static GFL_PROC_RESULT Box2Proc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	BOX2_SYS_WORK * syswk;

	OS_Printf( "↓↓↓↓↓　ボックス処理開始　↓↓↓↓↓\n" );

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_SYS, 0x10000 );

	syswk = GFL_PROC_AllocWork( proc, sizeof(BOX2_SYS_WORK), HEAPID_BOX_SYS );
	GFL_STD_MemClear( syswk, sizeof(BOX2_SYS_WORK) );

	syswk->dat      = pwk;
	syswk->tray     = BOXDAT_GetCureentTrayNumber( syswk->dat->sv_box );
	syswk->trayMax  = BOXDAT_GetTrayMax( syswk->dat->sv_box );
	syswk->next_seq = BOX2SEQ_MAINSEQ_START;

	// トレイの開放
	if( syswk->trayMax != BOX_MAX_TRAY ){
		u16	max;
		u16	i;
		if( syswk->trayMax == BOX_MIN_TRAY ){
			max = BOX_MIN_TRAY;
		}else if( syswk->trayMax == BOX_MED_TRAY ){
			max = BOX_MED_TRAY;
		}
		for( i=0; i<max; i++ ){
			if( BOXDAT_GetPokeExistCount( syswk->dat->sv_box, i ) == 0 ){
				break;
			}
		}
		if( i == max ){
			syswk->trayMax = BOXDAT_AddTrayMax( syswk->dat->sv_box );
		}
	}


/*
	syswk->box    = SaveData_GetBoxData( syswk->dat->savedata );
	syswk->party  = SaveData_GetTemotiPokemon( syswk->dat->savedata );
	syswk->item   = SaveData_GetMyItem( syswk->dat->savedata );
	syswk->config = SaveData_GetConfig( syswk->dat->savedata );

	syswk->get_pos = BOX2MAIN_GETPOS_NONE;

	syswk->cur_rcv_pos = 0;
*/

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
static GFL_PROC_RESULT Box2Proc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( BOX2SEQ_Main( mywk, seq ) == FALSE ){
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
static GFL_PROC_RESULT Box2Proc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_BOX_SYS );

	OS_Printf( "↑↑↑↑↑　ボックス処理終了　↑↑↑↑↑\n" );

/*
	BOX2_SYS_WORK * syswk = GFL_PROC_GetWork( proc );

	BOXDAT_SetCureentTrayNumber( syswk->box, syswk->tray );		// カレントトレイ更新
*/

	return GFL_PROC_RES_FINISH;
}
