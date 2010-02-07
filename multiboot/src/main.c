//===================================================================
/**
 * @file  main.c
 * @brief アプリケーション　サンプルスケルトン
 * @author  GAME FREAK Inc.
 * @date  06.11.08
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
#include "print/printsys.h"
#include "print/global_msg.h"
#include "poke_tool/poke_tool.h"
#include "system/net_err.h"
#include "system/wipe.h"
#include "system/brightness.h"

#include "sound\pm_sndsys.h"
#include "sound\pm_voice.h"
#include "sound\pm_wb_voice.h"


#ifdef PM_DEBUG
//#include "test/performance.h"
//#include "test/debug_pause.h"
//#include "debug/debugwin_sys.h"
#endif //PM_DEBUG

#include "multiboot/mb_child_sys.h"

static  void  SkeltonHBlankFunc(void);
static  void  SkeltonVBlankFunc(void);
static  void  GameInit(void);
static  void  GameMain(void);
static  void  GameExit(void);
static  void  GameVBlankFunc(void);

#ifdef PM_DEBUG
static void DEBUG_StackOverCheck(void);
#endif
//------------------------------------------------------------------
/**
 * @brief メイン処理
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

/*
#ifdef PM_DEBUG
  DEBUG_PAUSE_Init();
  {
    //デバッグし捨ての初期化＋アドレスを渡す
    u8 *charArea;
    u16 *scrnArea,*plttArea;
    NetErr_GetTempArea( &charArea , &scrnArea , &plttArea );
    DEBUGWIN_InitSystem(charArea , scrnArea , plttArea);
  }
#endif
*/
  OS_TPrintf("[%d]\n",PM_DEBUG);

  while(TRUE){
#ifdef PM_DEBUG
    DEBUG_StackOverCheck();
#endif  //PM_DEBUG

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
    //DEBUG_PerformanceEndLine(PERFORMANCE_ID_MAIN);
#endif //PM_DEBUG

    // VBLANK待ち
    GFL_G3D_SwapBuffers();
    if(GFL_NET_SystemGetConnectNum() > 1){
      OS_WaitIrq(TRUE, OS_IE_V_BLANK);
      GameVBlankFunc();
    }
    else{
      MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);
      //非暗転時のBGM変更のちらつき防止
      /*
      if( OS_IsThreadTerminated( &soundLoadThread ) == FALSE )
      { // サウンド読み込みスレッド休止
        OS_SleepThreadDirect( &soundLoadThread, NULL );
      }
      */
      OS_WaitIrq(TRUE, OS_IE_V_BLANK);
      GameVBlankFunc();
      /*
      if( OS_IsThreadTerminated( &soundLoadThread ) == FALSE )
      { // サウンド読み込みスレッド起動
        OS_WakeupThreadDirect( &soundLoadThread );
      }
      */
      MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
    }
  }

  GameExit();
}

//------------------------------------------------------------------
/**
 * brief  HBlank割り込み処理
 */
//------------------------------------------------------------------
static  void  SkeltonHBlankFunc(void)
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
	GFLUser_VIntr();

	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
	MI_WaitDma(GX_DEFAULT_DMAID);
}

//--------------------------------------------------------------
/**
 * VBlank期間中に行う処理
 */
//--------------------------------------------------------------
static void GameVBlankFunc(void)
{

	MachineSystem_VIntr();
	GFLUser_VTiming();
}

#include "../prog/src/test/ohno/fatal_error.h"
//------------------------------------------------------------------
/**
 * @brief   ゲームごとの初期化処理
 */
//------------------------------------------------------------------
static  void  GameInit(void)
{
  /* ユーザーレベルで必要な初期化をここに記述する */

	// 通信のデバッグプリントを行う定義
#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_ohno)||defined(DEBUG_ONLY_FOR_toru_nagihashi)||defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
	//GFL_NET_DebugPrintOn();
#endif
#endif  //PM_DEBUG
  // 通信ブート処理 VBlank割り込み後に行うためここに記述
  GFL_NET_Boot( GFL_HEAPID_APP, FatalError_Disp, GFL_HEAPID_APP, GFL_HEAPID_APP );
  /* 起動プロセスの設定 */
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &MultiBootChild_ProcData, NULL);

  /* 文字描画システム初期化 */
  PRINTSYS_Init( GFL_HEAPID_SYSTEM );
  //MSGSPEED_InitSystem( SaveControl_GetPointer() );

  //通信エラー画面管理システム初期化
  NetErr_SystemInit();
  NetErr_SystemCreate(HEAPID_NET_ERR);//※check　とりあえずゲーム中、ずっとシステムが存在するようにしている

  /* グローバルメッセージデータ＆フォント初期化 */
  GLOBALMSG_Init( GFL_HEAPID_SYSTEM );

  /* poketoolシステム初期化 */
  POKETOOL_InitSystem( GFL_HEAPID_SYSTEM );

  //サウンドの設定
  //PMSND_Init();
  //鳴き声システムの設定
  //PMVOICE_Init(GFL_HEAPID_APP, PMV_GetVoiceWaveIdx, PMV_CustomVoiceWave);
  //サウンドストリーミング再生システム
  //SND_STRM_Init(GFL_HEAPID_SYSTEM);

  //キーコントロールモード設定
  //CONFIG_SYSTEM_KyeControlTblSetting();

}


//------------------------------------------------------------------
/**
 * @brief   ゲームごとのメイン処理
 */
//------------------------------------------------------------------
static  void  GameMain(void)
{
  /* ユーザーレベルで必要なメイン処理をここに記述する */
  BrightnessChgMain();
  WIPE_SYS_Main();

  NetErr_Main();
  //if((GFL_UI_KEY_GetCont() & PAD_BUTTON_L) && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG)){
  //  NetErr_DEBUG_ErrorSet();
  //}
  //PMSND_Main();
  //PMVOICE_Main();
  //SND_STRM_Main();

}


//------------------------------------------------------------------
/**
 * @brief   ゲームごとの終了処理
 */
//------------------------------------------------------------------
static  void  GameExit(void)
{
#ifdef PM_DEBUG
  //デバッグシステム
  //DEBUGWIN_ExitSystem();
#endif PM_DEBUG
  //PMVOICE_Exit();
  //PMSND_Exit();
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
#endif  //PM_DEBUG
