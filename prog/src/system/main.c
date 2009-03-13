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
#include "sound\snd_strm.h"
#include "savedata/save_control.h"
#include "print/printsys.h"
#include "print/global_font.h"
#include "print/global_msg.h"
#include "poke_tool/poke_tool.h"
#include "system/net_err.h"
#include "savedata/config.h"
#include "system/wipe.h"

#include "sound\pm_sndsys.h"
#ifdef PM_DEBUG
#include "test/performance.h"
#endif //PM_DEBUG

#include "title/title.h"

static	void	SkeltonHBlankFunc(void);
static	void	SkeltonVBlankFunc(void);
static	void	GameInit(void);
static	void	GameMain(void);
static	void	GameExit(void);

#ifdef PM_DEBUG
static void DEBUG_StackOverCheck(void);
#endif

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

#ifdef PM_DEBUG
		DEBUG_StackOverCheck();
#endif	//PM_DEBUG

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

    GameExit();
    
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
	CPContext context;
	CP_SaveContext( &context );	// 除算器の状態を保存

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
	CP_RestoreContext( &context );	// 除算器の状態を復元
}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとの初期化処理
 */
//------------------------------------------------------------------
static	void	GameInit(void)
{
	/* ユーザーレベルで必要な初期化をここに記述する */

	//セーブ関連初期化
	SaveControl_SystemInit(GFL_HEAPID_APP);	//※check 使用しているヒープIDは暫定
	
    // 通信ブート処理 VBlank割り込み後に行うためここに記述、第二引数は表示用関数ポインタ
    GFL_NET_Boot( GFL_HEAPID_APP, NULL, GFL_HEAPID_APP, HEAPID_NETWORK_FIX);
	/* 起動プロセスの設定 */
#if 0
	TestModeSet();	//←サンプルデバッグモード
#else
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TitleControlProcData, NULL);
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

    //サウンドの設定
	PMSNDSYS_Init();
	//サウンドストリーミング再生システム
	SND_STRM_Init(GFL_HEAPID_SYSTEM);

	//キーコントロールモード設定
	CONFIG_SYSTEM_KyeControlTblSetting();

}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとのメイン処理
 */
//------------------------------------------------------------------
static	void	GameMain(void)
{
	/* ユーザーレベルで必要なメイン処理をここに記述する */

	WIPE_SYS_Main();

	NetErr_Main();
	NetErr_App_ErrorCheck();
	if((GFL_UI_KEY_GetCont() & PAD_BUTTON_L) && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG)){
		NetErr_ErrorSet();
	}
	SND_STRM_Main();
	PMSNDSYS_Main();
}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとの終了処理
 */
//------------------------------------------------------------------
static	void	GameExit(void)
{
	SND_STRM_Exit();
	PMSNDSYS_Exit();
}

//--------------------------------------------------------------
/**
 * @brief   スタック溢れチェック
 */
//--------------------------------------------------------------
#ifdef PM_DEBUG
static void DEBUG_StackOverCheck(void)
{
	OSStackStatus stack_status;
	
	stack_status = OS_GetStackStatus(OS_GetCurrentThread());
	switch(stack_status){
	case OS_STACK_OVERFLOW:
		GF_ASSERT("スタック最下位のマジックナンバーが書き換えられています\n");
		break;
	case OS_STACK_UNDERFLOW:
		GF_ASSERT("スタック最上位のマジックナンバーが書き換えられています\n");
		break;
	case OS_STACK_ABOUT_TO_OVERFLOW:
		OS_TPrintf("スタック溢れの警戒水準に達しています\n");
		break;
	}
	
	stack_status = OS_GetIrqStackStatus();
	switch(stack_status){
	case OS_STACK_OVERFLOW:
		GF_ASSERT("IRQスタック最下位のマジックナンバーが書き換えられています\n");
		break;
	case OS_STACK_UNDERFLOW:
		GF_ASSERT("IRQスタック最上位のマジックナンバーが書き換えられています\n");
		break;
	case OS_STACK_ABOUT_TO_OVERFLOW:
		OS_TPrintf("IRQスタック溢れの警戒水準に達しています\n");
		break;
	}
}
#endif	//PM_DEBUG
