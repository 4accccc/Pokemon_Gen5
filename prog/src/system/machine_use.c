//=============================================================================================
/**
 * @file	machine_use.c
 * @author	GAME FREAK inc.
 * @brief	ユーザーのマシン依存システム使用のためのスケルトンソース
 *
 */
//=============================================================================================
#include <nitro.h>
#include <nnsys.h>
#include <gflib.h> // gflib/include/dma.hを参照するため

#include "system\machine_use.h"
#include "system\ds_system.h"
//=============================================================================================
//=============================================================================================
// ファイルシステムで利用するDMA番号の定義
#define FS_DMA_NUMBER	(GFL_DMA_FS_NO)

//ARM9優先の場合に必要なリセットの為の処理
#if (defined(SDK_TWL))
static PMExitCallbackInfo myInfo;
static void myExitCallback(void *arg);
static BOOL bHardResetFlg=FALSE;
#endif

//カード抜けを検出したときに呼ばれるコールバック
#ifdef PM_DEBUG
static  BOOL  card_pull_callback( void );
#endif

//VRAM転送マネージャ定義(NNS関数)
#define VRAMTRANS_MAN_TASKNUM    (48)
NNSGfdVramTransferTask    VRAMtransManTaskArray[ VRAMTRANS_MAN_TASKNUM ];
//=============================================================================================
//
//			関数
//
//=============================================================================================

//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
void MachineSystem_Init(void)
{
#ifdef PM_DEBUG
#ifndef MULTI_BOOT_MAKE
  OS_EnableMainExArena();
#endif //MULTI_BOOT_MAKE
#endif PM_DEBUG
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
	
#if (defined(SDK_TWL))
  if( OS_GetConsoleType() & (OS_CONSOLE_TWL|OS_CONSOLE_TWLDEBUGGER) )
  {
    //TWL拡張機能のOFF
  	SCFG_SetCpuSpeed( SCFG_CPU_SPEED_2X );
  	SCFG_SetVramExpanded( FALSE );
  }
#endif

	// ファイルシステム初期化
#ifndef MULTI_BOOT_MAKE
  //マルチブートは別のところで初期化する
	{
		u32 file_table_size;
		void* p_table;

		OS_EnableIrq();
		FS_Init(FS_DMA_NUMBER);

		// ファイル管理テーブルをRAMへ載せる→ファイルへの高速アクセスが可能
		file_table_size = FS_GetTableSize();
		p_table = OS_AllocFromMainArenaLo(file_table_size, 4);

		SDK_ASSERT(p_table != NULL);

		
		(void)FS_LoadTable(p_table, file_table_size);

		OS_TPrintf("FileTable Size     = 0x%08x bytes.\n", file_table_size);
		OS_TPrintf("remains of MainRAM = 0x%08x bytes.\n", 
						(u32)(OS_GetMainArenaHi())-(u32)(OS_GetMainArenaLo()));
	}
#endif
	// ＶＲＡＭ転送関数初期化
	NNS_GfdInitVramTransferManager( VRAMtransManTaskArray, VRAMTRANS_MAN_TASKNUM );

#ifdef PM_DEBUG
	//スタック溢れチェック
	OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x30);  //VCHATで使っていたので引き下げ0x100->0x30
    OS_CheckStack(OS_GetCurrentThread());
    OS_SetIrqStackWarningOffset(0x100);
#endif

#if (defined(SDK_TWL))
#ifndef MULTI_BOOT_MAKE
  if(DS_SYSTEM_IsRunOnTwl()){
#else
  if(OS_IsRunOnTwl()){
#endif
    //ARM9優先の場合に必要なハードリセットの為の処理
    bHardResetFlg=FALSE;
    PM_SetExitCallbackInfo( &myInfo, myExitCallback, (void*)0 );
    PM_AppendPreExitCallback( &myInfo );
  }
#endif//

#ifndef MULTI_BOOT_MAKE
  //ソフトリセット時マイクがOFFにならなかったのを修正
  {
    PMAmpSwitch swBuf;
    PM_GetAmp( &swBuf );
    if( swBuf == PM_AMP_ON )
    {
      PM_SetAmp( PM_AMP_OFF );
    }
  }
#endif//

#ifdef PM_DEBUG
  //カード抜けを検出したときに呼び出されるコールバックを設定
  CARD_SetPulledOutCallback( &card_pull_callback );
#endif
}

//------------------------------------------------------------------
/**
 * @brief	メイン呼び出し
 */
//------------------------------------------------------------------
void MachineSystem_Main(void)
{
#if (defined(SDK_TWL))
  //ハードリセットコールバックが呼ばれた
  if(bHardResetFlg && GFL_UI_CheckSoftResetDisable(GFL_UI_SOFTRESET_SVLD) == FALSE){
    if(!PM_GetAutoExit()){  //自分が処理しなければいけない事を確認する
      MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);  //ARM7優先に切り替える
      PM_ReadyToExit();  //ハードリセット後処理開始（もどってこない）
    }
  }
#endif
}

//------------------------------------------------------------------
/**
 * @brief	終了処理
 */
//------------------------------------------------------------------
void MachineSystem_Exit(void)
{
}

//------------------------------------------------------------------
/**
 * @brief	VBlank割り込み処理
 */
//------------------------------------------------------------------
void MachineSystem_VIntr(void)
{
	NNS_GfdDoVramTransfer();
}

//------------------------------------------------------------------
/**
 * @brief	カートリッジライブラリ無効化 ※メモリ削減のため
 */
//------------------------------------------------------------------
void CTRDG_Init(void)
{
  CTRDG_DummyInit();
}

//------------------------------------------------------------------
/**
 * @brief	ハードリセット時に呼ばれる関数
 */
//------------------------------------------------------------------
#if (defined(SDK_TWL))
static void myExitCallback(void *arg)
{
#pragma unused(arg)
  PM_SetAutoExit(FALSE); //ハードリセットを任せることなく この後処理する
  bHardResetFlg=TRUE;   //ハードリセットフラグON
}
#endif

#ifdef PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief	カード抜けを検出した時に呼ばれる関数
 */
//------------------------------------------------------------------
static  BOOL  card_pull_callback( void )
{ 
  GF_ASSERT_MSG( 0, "card pull!\n" );

  return TRUE;
}
#endif

