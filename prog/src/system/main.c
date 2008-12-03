//===================================================================
/**
 * @file	main.c
 * @brief	アプリケーション　サンプルスケルトン
 * @author	GAME FREAK Inc.
 * @date	06.11.08
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "system\machine_use.h"
#include "system\gfl_use.h"
#include "system\main.h"
#include "savedata/save_control.h"
#include "print/printsys.h"
#include "print/global_font.h"
#include "print/global_msg.h"
#include "poke_tool/poke_tool.h"
#include "system/net_err.h"

#ifdef PM_DEBUG
#include "test/performance.h"
#include "test/goto/comm_error.h"
#endif //PM_DEBUG

#include "title/title.h"

static	void	SkeltonHBlankFunc(void);
static	void	SkeltonVBlankFunc(void);
static	void	GameInit(void);
static	void	GameMain(void);

//==============================================================================
//	タイトル画面PROC呼び出しようのデータ
//==============================================================================
const GFL_PROC_DATA TitleProcData = {
	TitleProcInit,
	TitleProcMain,
	TitleProcEnd,
};


//------------------------------------------------------------------
/**
 * @brief	メイン処理
 */
//------------------------------------------------------------------
void NitroMain(void)
{
	// ハード環境システム郡を初期化する関数
	MachineSystem_Init();
	// ＧＦＬ初期化
	GFLUser_Init();

	//HBLANK割り込み許可
	OS_SetIrqFunction(OS_IE_H_BLANK,SkeltonHBlankFunc);
	//VBLANK割り込み許可
	OS_SetIrqFunction(OS_IE_V_BLANK,SkeltonVBlankFunc);

	(void)OS_EnableIrqMask(OS_IE_H_BLANK|OS_IE_V_BLANK);
	(void)OS_EnableIrq();

	(void)GX_HBlankIntr(TRUE);
	(void)GX_VBlankIntr(TRUE);
	(void)OS_EnableInterrupts();

	// 必要なTCBとか登録して…
	GameInit();

#ifdef PM_DEBUG
	DEBUG_PerformanceInit();
#endif

	while(TRUE){
#ifdef PM_DEBUG
//	DEBUG_PerformanceStart();
	DEBUG_PerformanceMain();
	DEBUG_PerformanceStartLine(PERFORMANCE_ID_MAIN);
#endif //PM_DEBUG

        MachineSystem_Main();
		// メイン処理して…
		GFLUser_Main();
		GameMain();

		// 描画に必要な準備して…
		GFLUser_Display();

		// レンダリングエンジンが参照するデータ群をスワップ
		// ※gflibに適切な関数が出来たら置き換えてください
		//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

#ifdef PM_DEBUG
	//DEBUG_PerformanceDisp();
	DEBUG_PerformanceEndLine(PERFORMANCE_ID_MAIN);
#endif //PM_DEBUG

        // VBLANK待ち
		// ※gflibに適切な関数が出来たら置き換えてください
		OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	}
}

//------------------------------------------------------------------
/**
 * brief	HBlank割り込み処理
 */
//------------------------------------------------------------------
static	void	SkeltonHBlankFunc(void)
{
	GFLUser_HIntr();
}

//------------------------------------------------------------------
/**
 * brief	VBlank割り込み処理
 */
//------------------------------------------------------------------
static	void	SkeltonVBlankFunc(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);

	MI_WaitDma(GX_DEFAULT_DMAID);

#ifdef PM_DEBUG
	DEBUG_PerformanceStartLine(PERFORMANCE_ID_VBLANK);
#endif

	MachineSystem_VIntr();
	GFLUser_VIntr();

#ifdef PM_DEBUG
	DEBUG_PerformanceEndLine(PERFORMANCE_ID_VBLANK);
#endif
}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとの初期化処理
 */
//------------------------------------------------------------------
extern void TestModeSet( void );	//←サンプルデバッグモード

static	void	GameInit(void)
{
	/* ユーザーレベルで必要な初期化をここに記述する */

	//セーブ関連初期化
	SaveControl_SystemInit(GFL_HEAPID_APP);	//※check 使用しているヒープIDは暫定
	
    // 通信ブート処理 VBlank割り込み後に行うためここに記述、第二引数は表示用関数ポインタ
    GFL_NET_Boot( GFL_HEAPID_APP, NULL );
	/* 起動プロセスの設定 */
#if 0
	TestModeSet();	//←サンプルデバッグモード
#else
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TitleProcData, NULL);
#endif

	/* 文字描画システム初期化 */
	PRINTSYS_Init( GFL_HEAPID_SYSTEM );
	
	//通信エラー画面管理システム初期化
	NetErr_SystemInit();
	NetErr_SystemCreate(GFL_HEAPID_APP);//※check　とりあえずゲーム中、ずっとシステムが存在するようにしている

	/* グローバルメッセージデータ＆フォント初期化 */
	GLOBALMSG_Init( GFL_HEAPID_SYSTEM );

	/* poketoolシステム初期化 */
	POKETOOL_InitSystem( GFL_HEAPID_SYSTEM );

}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとのメイン処理
 */
//------------------------------------------------------------------
static	void	GameMain(void)
{
	/* ユーザーレベルで必要なメイン処理をここに記述する */

#ifdef PM_DEBUG

	CommErrorSys_SampleView();
	
#endif	// PM_DEBUG

	NetErr_Main();
	NetErr_App_ErrorCheck();
	if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DEBUG){
		NetErr_ErrorSet();
	}
}
