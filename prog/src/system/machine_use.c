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
//=============================================================================================
//=============================================================================================
// ファイルシステムで利用するDMA番号の定義
#define FS_DMA_NUMBER	(GFL_DMA_FS_NO)



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
	
	SCFG_SetCpuSpeed( SCFG_CPU_SPEED_1X );
	
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




// 複製 ROM アーカイブ構造体。
typedef struct MyRomArchive
{
    FSArchive   arc[1];
    u32         default_dma_no;
    u32         card_lock_id;
}
MyRomArchive;

// 非同期のROM読み込みが完了したときの処理。
static void MyRom_OnReadDone(void *arc)
{
    // アーカイブへ完了通知。
    FS_NotifyArchiveAsyncEnd((FSArchive *)arc, FS_RESULT_SUCCESS);
}

// FSからアーカイブへのリードアクセスコールバック。
static FSResult MyRom_ReadCallback(FSArchive *arc, void *dst, u32 src, u32 len)
{
    MyRomArchive *const p_rom = (MyRomArchive *)arc;
    CARD_ReadRomAsync(p_rom->default_dma_no,
                      (const void *)(FS_GetArchiveBase(arc) + src), dst, len,
                      MyRom_OnReadDone, arc);
    return FS_RESULT_PROC_ASYNC;
}

// FSからアーカイブへのライトコールバック。
// ユーザプロシージャでFS_RESULT_UNSUPPORTEDを返すので呼ばれない。
static FSResult MyRom_WriteDummyCallback(FSArchive *arc, const void *src, u32 dst, u32 len)
{
    (void)arc;
    (void)src;
    (void)dst;
    (void)len;
    return FS_RESULT_FAILURE;
}

// ユーザプロシージャ。
// 最初のコマンド開始前から最後のコマンド完了後までROMをロック。
// ライト操作はサポート外として応答する。
// それ以外はデフォルトの動作。
static FSResult MyRom_ArchiveProc(FSFile *file, FSCommandType cmd)
{
    MyRomArchive *const p_rom = (MyRomArchive *) FS_GetAttachedArchive(file);
    switch (cmd)
    {
    case FS_COMMAND_ACTIVATE:
        CARD_LockRom((u16)p_rom->card_lock_id);
        return FS_RESULT_SUCCESS;
    case FS_COMMAND_IDLE:
        CARD_UnlockRom((u16)p_rom->card_lock_id);
        return FS_RESULT_SUCCESS;
    case FS_COMMAND_WRITEFILE:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
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

  {
    const u32 base = 0;
    const CARDRomRegion *fnt = &((CARDRomHeader*)CARD_GetRomHeader())->fnt;
    const CARDRomRegion *fat = &((CARDRomHeader*)CARD_GetRomHeader())->fat;
    const char *name = "rom";

    static MyRomArchive newRom;

    FSArchive *oldROM = FS_FindArchive("rom", 3);
    if (oldROM) 
    {
      FS_UnloadArchive(oldROM);
      // FS_ReleaseArchiveName(oldROM);
    }
    
    FS_InitArchive(newRom.arc);
    newRom.default_dma_no = FS_DMA_NUMBER;
    newRom.card_lock_id = (u32)OS_GetLockID();
    if (!FS_RegisterArchiveName(newRom.arc, name, (u32)STD_GetStringLength(name)))
    {
      OS_TPanic("error! FS_RegisterArchiveName(%s) failed.\n", name);
    }
    else
    {
      FS_SetArchiveProc(newRom.arc, MyRom_ArchiveProc,
                        FS_ARCHIVE_PROC_WRITEFILE | FS_ARCHIVE_PROC_ACTIVATE | FS_ARCHIVE_PROC_IDLE);
      if (!FS_LoadArchive(newRom.arc, base,
                        fat->offset, fat->length, fnt->offset, fnt->length,
                        MyRom_ReadCallback, MyRom_WriteDummyCallback))
      {
          OS_TPanic("error! FS_LoadArchive() failed.\n");
      }
    }
  }

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
