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


//TWLのNITROモードとのHEAPの差分の為確保してあります
//
#if !defined(SDK_TWL)
static u8 dummybufferTWL[0x8d00];
#endif

//VRAM転送マネージャ定義(NNS関数)
#define VRAMTRANS_MAN_TASKNUM    (32)
NNSGfdVramTransferTask    VRAMtransManTaskArray[ VRAMTRANS_MAN_TASKNUM ];
//=============================================================================================
//
//			関数
//
//=============================================================================================

static void MachineSystem_MbInitFile(void);

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
	// ファイルシステム初期化
#ifndef MULTI_BOOT_MAKE
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
#else
  MachineSystem_MbInitFile();
#endif
	// ＶＲＡＭ転送関数初期化
	NNS_GfdInitVramTransferManager( VRAMtransManTaskArray, VRAMTRANS_MAN_TASKNUM );

#ifdef PM_DEBUG
	//スタック溢れチェック
	OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x30);  //VCHATで使っていたので引き下げ0x100->0x30
    OS_CheckStack(OS_GetCurrentThread());
    OS_SetIrqStackWarningOffset(0x100);
#endif

#if !defined(SDK_TWL)
  dummybufferTWL[0]=0;
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


static void MachineSystem_MbInitFile(void)
{
  u32 file_table_size;
  void* p_table;
  MBParam *multi_p = (MBParam *)MB_GetMultiBootParam();

  // ROMアクセスを解除する。
  CARD_Enable(TRUE);

  multi_p->boot_type = MB_TYPE_NORMAL;	/* FS_Init()にROMをenableにさせるため、MULTIBOOTフラグを一瞬OFFにする */
  OS_EnableIrq();
  FS_Init(FS_DMA_NUMBER);
  multi_p->boot_type = MB_TYPE_MULTIBOOT;	/* MULTIBOOTフラグを再セットする */

#if 0
  if (!FS_IsAvailable())
  {
      OS_TPanic("no archive to replace!");
  }
  else
  {
    static const char name[] = "rom";
    static const int name_len = sizeof(name) - 1;
    const CARDRomHeader* header;
    FSArchive *rom;
    {
      CARDRomHeader * const arg_buffer = (CARDRomHeader *)0x027FF000/*HW_MAIN_MEM_SHARED*/;
      CARDRomHeader * const app_header = (CARDRomHeader *)HW_ROM_HEADER_BUF;
      CARDRomHeader * const org_header = (CARDRomHeader *)HW_CARD_ROM_HEADER;
      if (arg_buffer->game_code == 0)
      {
        // ROMヘッダの内容を退避領域にコピーします。
        CARD_Init();
        MI_CpuCopy8(app_header, arg_buffer, HW_CARD_ROM_HEADER_SIZE);
        MI_CpuCopy8(app_header, org_header, HW_CARD_ROM_HEADER_SIZE);
        /*
         * この時点でarg_buffer->game_code はNITROカードのイニシャルコードそのものが入っている
         */
      }
      header = arg_buffer;
    }
    rom = FS_FindArchive(name, name_len);
    rom->fat = header->fat.offset;
    rom->fat_size = header->fat.length;
    rom->fnt = header->fnt.offset;
    rom->fnt_size = header->fnt.length;
  }
#endif
  {
    //実験
    //OSBootInfo *bootInfo = ((OSBootInfo *)HW_WM_BOOT_BUF);
    //bootInfo->boot_type = OS_BOOTTYPE_ROM;
  }
/*  
  // ファイル管理テーブルをRAMへ載せる→ファイルへの高速アクセスが可能
  file_table_size = FS_GetTableSize();
  p_table = OS_AllocFromMainArenaLo(file_table_size, 4);

  SDK_ASSERT(p_table != NULL);


  (void)FS_LoadTable(p_table, file_table_size);

  OS_TPrintf("FileTable Size     = 0x%08x bytes.\n", file_table_size);
  OS_TPrintf("remains of MainRAM = 0x%08x bytes.\n", 
              (u32)(OS_GetMainArenaHi())-(u32)(OS_GetMainArenaLo()));
*/
}