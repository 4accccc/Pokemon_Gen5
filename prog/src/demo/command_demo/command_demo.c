//============================================================================================
/**
 * @file    command_demo.c
 * @brief   コマンドデモ画面 呼び出し
 * @author  Hiroyuki Nakamura
 * @date    09.01.21
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"

#include "cdemo_main.h"
#include "cdemo_seq.h"

/*
#include "common.h"

#include "ld_main.h"
#include "ld_lugia.h"
#include "ld_houou.h"
#include "ld_arceus.h"
*/


//============================================================================================
//  定数定義
//============================================================================================


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT CommDemoProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT CommDemoProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT CommDemoProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//============================================================================================
//  グローバル変数
//============================================================================================

const GFL_PROC_DATA COMMANDDEMO_ProcData = {
  CommDemoProc_Init,
  CommDemoProc_Main,
  CommDemoProc_End,
};

/*
// メイン処理
static const pLegendDemoFunc MainFunc[] = {
  LDHOUOU_Main,   // ホウオウ
  LDLUGIA_Main,   // ルギア
  LDARCEUS_Main,    // アルセウス
};
*/



//--------------------------------------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc  プロセスデータ
 * @param   seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT CommDemoProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
/*
  LEGEND_DEMO_WORK * wk;

  OS_Printf( "↓↓↓↓↓　伝説デモ処理開始　↓↓↓↓↓\n" );

  sys_CreateHeap( HEAPID_BASE_APP, HEAPID_LEGEND_DEMO, 0x80000 );

  wk = PROC_AllocWork( proc, sizeof(LEGEND_DEMO_WORK), HEAPID_LEGEND_DEMO );
  MI_CpuClear8( wk, sizeof(LEGEND_DEMO_WORK) );

  wk->dat = PROC_GetParentWork( proc );

  return PROC_RES_FINISH;
*/

/*
  BOX2_SYS_WORK * syswk;

  OS_Printf( "↓↓↓↓↓　ボックス処理開始　↓↓↓↓↓\n" );

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_SYS, 0x10000 );

  syswk = GFL_PROC_AllocWork( proc, sizeof(BOX2_SYS_WORK), HEAPID_BOX_SYS );
  GFL_STD_MemClear( syswk, sizeof(BOX2_SYS_WORK) );

  syswk->dat      = pwk;
  syswk->tray     = BOXDAT_GetCureentTrayNumber( syswk->dat->sv_box );
  syswk->next_seq = BOX2SEQ_MAINSEQ_START;
*/
  CDEMO_WORK * wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_COMMAND_DEMO, 0x80000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(CDEMO_WORK), HEAPID_COMMAND_DEMO );
  GFL_STD_MemClear( wk, sizeof(CDEMO_WORK) );

  wk->dat = pwk;

  OS_InitTick();

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc  プロセスデータ
 * @param   seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT CommDemoProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
/*
  LEGEND_DEMO_WORK * wk = PROC_GetWork( proc );

  if( MainFunc[wk->dat->mode]( wk ) == 0 ){
    return PROC_RES_FINISH;
  }

  return PROC_RES_CONTINUE;
*/

  if( CDEMOSEQ_Main( mywk, seq ) == FALSE ){
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc  プロセスデータ
 * @param   seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT CommDemoProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
/*
  PROC_FreeWork( proc );

  sys_DeleteHeap( HEAPID_LEGEND_DEMO );

  OS_Printf( "↑↑↑↑↑　伝説デモ処理終了　↑↑↑↑↑\n" );

  return PROC_RES_FINISH;
*/

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_BOX_SYS );

  return GFL_PROC_RES_FINISH;
}
