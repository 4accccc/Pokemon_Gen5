//=============================================================================================
/**
 * @file	gfl_use.c
 * @date	2006.12.06
 * @author	GAME FREAK inc.
 * @brief	ユーザーのGFライブラリ使用のためのスケルトンソース
 *
 */
//=============================================================================================

#include <nitro.h>
#include "gflib.h"
//#include "gflib_inter.h"

#include "main.h"

#include "gfl_use.h"
#include "procsys.h"
#include "gf_overlay.h"
#include "tcb.h"


//=============================================================================================
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	TCB_VINTR_MAX = 16,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	TCBSYS * TCBSysVintr;
	void * TCBMemVintr;
}GFL_USE_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static const HEAP_INIT_HEADER hih[]={
	{ HEAPSIZE_SYSTEM,	OS_ARENA_MAIN },
	{ HEAPSIZE_APP,		OS_ARENA_MAIN },
};

static GFL_USE_WORK * gfl_work = NULL;

//=============================================================================================
//
//			関数
//
//=============================================================================================
//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：初期化
 *
 * InitSystemの内部で呼んでいるInitFileSystemはgflibの外では
 * 呼べないため、InitSystemを呼んでいる。
 */
//------------------------------------------------------------------
void GFLUser_Init(void)
{
	InitSystem();
#if 0
#ifdef USE_MAINEXMEMORY
  OS_EnableMainExArena();
#endif

	// システム初期化
	OS_Init();
	OS_InitTick();
	OS_InitAlarm();
	// RTC初期化
	RTC_Init();
	// GX初期化
	GX_Init();
	// 固定小数点API初期化
	FX_Init();
	// ファイルシステム初期化
	InitFileSystem();
#endif

	//ヒープシステム初期化
	GFL_HEAP_Init(&hih[0],GFL_HEAPID_MAX,HEAPID_CHILD_MAX,0);	//メインアリーナ
	GFL_HEAP_DTCM_Init( 0x2000 );				//ＤＴＣＭアリーナ

	//アーカイブシステム初期化
	//GFL_ARC_SysInit(...);

	//UIシステム初期化
	GFL_UI_Boot(GFL_HEAPID_SYSTEM);

	//OVERLAYシステム初期化
	GFL_OVERLAY_boot(GFL_HEAPID_SYSTEM, 8, 4, 4);

	//PROCシステム初期化
	GFL_PROC_boot(GFL_HEAPID_SYSTEM);
  gfl_work = GFL_HEAP_AllocMemory(GFL_HEAPID_SYSTEM, sizeof(GFL_USE_WORK));
  gfl_work->TCBMemVintr = GFL_HEAP_AllocMemory(
		  GFL_HEAPID_SYSTEM, GFL_TCB_CalcSystemWorkSize(TCB_VINTR_MAX));
  gfl_work->TCBSysVintr = GFL_TCB_SysInit(TCB_VINTR_MAX, gfl_work->TCBMemVintr);

    //FADEシステム初期化
    GFL_FADE_sysInit(GFL_HEAPID_SYSTEM);

}


//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：メイン呼び出し
 *
 * 毎フレーム呼び出される必要があるメイン処理を記述する
 */
//------------------------------------------------------------------
void GFLUser_Main(void)
{
	GFL_UI_Main();
    GFL_NET_Main();  //キーの処理の後すぐに通信を処理したい為ここに配置
	
	GFL_PROC_SysMain();
    GFL_FADE_sysMain();
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：描画処理
 *
 * 毎フレーム呼び出される必要がある描画処理を記述する
 */
//------------------------------------------------------------------
void GFLUser_Display(void)
{
	//GFL_OAM_sysMain(...);
	//GFL_BG_sysMain(...);
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：終了処理
 */
//------------------------------------------------------------------
void GFLUser_Exit(void)
{
	GFL_UI_Exit();
	GFL_PROC_SysExit();
	GFL_OVERLAY_SysExit();
	GFL_FADE_sysExit();
	GFL_HEAP_DTCM_Exit();
	GFL_HEAP_Exit();
}


//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：VBlank割り込み処理
 */
//------------------------------------------------------------------
void GFLUser_VIntr(void)
{
	GFL_TCB_SysMain(gfl_work->TCBSysVintr);
	GFL_BG_VBlankFunc();
	// Vブランク期間で実行します。
	// ただ、ユニットの描画が行われていないのに
	// この関数を実行すると、描画しているOBJが消えてしまうため
	// 割り込みないで呼ばないほうが良いかもしれません。
	GFL_CLACT_SysVblank();
    // 通信アイコンの描画のためにあります。通信自体は行っていません
    GFL_NET_VBlankFunc();
}


