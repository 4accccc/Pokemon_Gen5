/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - simple
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <twl.h>
//#include <DEMO.h>
#include <nitro.h>
#include <nnsys.h>
#include <gflib.h>

#include "system\machine_use.h"
#include "system\gfl_use.h"
//#include "system/net_err.h"
#include "system/main.h"
//#include "poke_tool/poke_tool.h"
//#include "print/printsys.h"
//#include "print/global_msg.h"

static void PrintBootType();

static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

static  void  SkeltonHBlankFunc(void);
static  void  SkeltonVBlankFunc(void);
static  void  GameInit(void);
static  void  GameMain(void);
static  void  GameExit(void);
static  void  GameVBlankFunc(void);

//extern const GFL_PROC_DATA ScratchIchirakuMainProcData; //ichi_scratch/ichi_scratch_main.c

// OS_Printfのフック関数
#ifndef SDK_FINALROM
void OS_Printf(const char *fmt, ...)
{
    char dst[256];
    
    int     ret;
    va_list va;
    va_start(va, fmt);
    ret = OS_VSPrintf(dst, fmt, va);
    va_end(va);

    OS_PutString(dst);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  メイン関数です。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
  MachineSystem_Init();
    {
      PMBackLightSwitch top, bottom;
      OS_Printf("OS IRQ =%d\n", OS_GetIrq() );
      OS_Printf("OS CPSR IRQ = %d\n", OS_GetCpsrIrq() );
      OS_Printf("OS PROC MODE = %d\n", OS_GetProcMode() );
      while( PM_GetBackLight( &top, &bottom ) != PM_RESULT_SUCCESS ) { }
      OS_Printf("back light (up)=%d\n", top );
      OS_Printf("back light (bottom)=%d\n", bottom );
    }

  GFLUser_Init();
#if 0
    OS_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
#endif
    OS_Printf("*** finish gflib init\n");

  //HBLANK割り込み許可
  OS_SetIrqFunction(OS_IE_H_BLANK,SkeltonHBlankFunc);
  //VBLANK割り込み許可
  OS_SetIrqFunction(OS_IE_V_BLANK,SkeltonVBlankFunc);

  (void)OS_EnableIrqMask(OS_IE_H_BLANK|OS_IE_V_BLANK);

  (void)GX_HBlankIntr(TRUE);
  (void)GX_VBlankIntr(TRUE);
  (void)OS_EnableInterrupts();

  // 必要なTCBとか登録して…
  GameInit();

    OS_Printf("*** start nandApp demo\n");

    PrintBootType();

#if 0
    // ROM からファイルを読み込んで表示する
    // NAND アプリでも ROM ファイルシステムを使用することができます。
    {
        BOOL bSuccess;
        FSFile f;
        u32 fileSize;
        s32 readSize;
        void* pBuffer;

        FS_InitFile(&f);

        bSuccess = FS_OpenFileEx(&f, "build_time.txt", FS_FILEMODE_R);
        SDK_ASSERT( bSuccess );

        fileSize = FS_GetFileLength(&f);
        pBuffer = OS_Alloc(fileSize + 1);
        SDK_POINTER_ASSERT(pBuffer);

        readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
        SDK_ASSERT( readSize == fileSize );

        bSuccess = FS_CloseFile(&f);
        SDK_ASSERT( bSuccess );

        ((char*)pBuffer)[fileSize] = '\0';
        OS_Printf("%s\n", pBuffer);
        OS_Free(pBuffer);
    }
#endif

    OS_Printf("*** End of demo\n");

    // ランチャーに戻れるように、
    // 終了しない
    for (;;)
    {
        MachineSystem_Main();
        // メイン処理して…
        GFLUser_Main();
        GameMain();

        // 描画に必要な準備して…
        GFLUser_Display();

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

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  BootType を print します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  コンソールの画面出力用の表示設定を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
#if 0
    // 画面表示の初期化。
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
#endif
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  割り込みを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // マスター割り込みフラグを許可に
    (void)OS_EnableIrq();

    // IRQ 割り込みを許可します
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  ヒープを作成して OS_Alloc が使えるようにします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // メインアリーナのアロケートシステムを初期化
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // メインアリーナ上にヒープを作成
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  FS を初期化して rom にアクセスできるようにします。
                この関数を呼び出す前に InitInteruptSystem() が
                呼ばれている必要があります。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // ファイルシステム初期化
    FS_Init( FS_DMA_NOT_USE );
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
	//GFLUser_VTiming();
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
  // 通信ブート処理 VBlank割り込み後に行うためここに記述
  //GFL_NET_Boot( GFL_HEAPID_APP, FatalError_Disp, GFL_HEAPID_APP, GFL_HEAPID_APP );
  /* 起動プロセスの設定 */
	//GFL_PROC_SysCallProc(NO_OVERLAY_ID, &ScratchIchirakuMainProcData, NULL);
	//GFL_PROC_SysCallProc(NO_OVERLAY_ID, &MultiBootChild_ProcData, NULL);

  /* 文字描画システム初期化 */
  //PRINTSYS_Init( GFL_HEAPID_SYSTEM );
  //MSGSPEED_InitSystem( SaveControl_GetPointer() );

  //通信エラー画面管理システム初期化
  //NetErr_SystemInit();
  //NetErr_SystemCreate(HEAPID_NET_ERR);//※check　とりあえずゲーム中、ずっとシステムが存在するようにしている

  /* グローバルメッセージデータ＆フォント初期化 */
  // ファイルの展開が違うからサイズでかいので・・・
  //GLOBALMSG_Init( GFL_HEAPID_APP );

  /* poketoolシステム初期化 */
  //POKETOOL_InitSystem( GFL_HEAPID_SYSTEM );

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
  //BrightnessChgMain();
  //WIPE_SYS_Main();

  //NetErr_Main();
  //if((GFL_UI_KEY_GetCont() & PAD_BUTTON_L) && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG)){
  //  NetErr_DEBUG_ErrorSet();
  //}
  //PMSND_Main();
  //PMVOICE_Main();
  //SND_STRM_Main();

}

