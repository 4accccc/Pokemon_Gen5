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

#include "system\machine_use.h"
//=============================================================================================
//=============================================================================================
// ファイルシステムで利用するDMA番号の定義
#define FS_DMA_NUMBER	1

//VRAM転送マネージャ定義(NNS関数)
#define VRAMTRANS_MAN_TASKNUM    (32)
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
  OS_EnableMainExArena();
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
	// ファイルシステム初期化
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
	// ＶＲＡＭ転送関数初期化
	NNS_GfdInitVramTransferManager( VRAMtransManTaskArray, VRAMTRANS_MAN_TASKNUM );

#ifdef PM_DEBUG
	//スタック溢れチェック
	OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x30);  //VCHATで使っていたので引き下げ0x100->0x30
    OS_CheckStack(OS_GetCurrentThread());
    OS_SetIrqStackWarningOffset(0x100);
#endif
}

//------------------------------------------------------------------
/**
 * @brief	メイン呼び出し
 */
//------------------------------------------------------------------
void MachineSystem_Main(void)
{
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
