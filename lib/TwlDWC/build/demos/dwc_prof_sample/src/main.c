#include <nitro.h>
#include <dwc.h>

#include "init.h"
#include "dbs.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------
#define INITIAL_CODE    'NTRJ'    // このサンプルが仕様するイニシャルコード
#define GAME_FRAME      1         // 想定するゲームフレーム（1/60を1とする）
//#define USE_AUTHSERVER_PRODUCTION // 製品向け認証サーバを使用する場合有効にする

//----------------------------------------------------------------------------
// typedef
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// variable
//----------------------------------------------------------------------------
static DWCInetControl stConnCtrl;
static BOOL sPrintOverride;

//----------------------------------------------------------------------------
// prototype
//----------------------------------------------------------------------------
static void InitDWC(void);
static void NetConfigMain(void);
static void StartIPMain(void);
static void StopIPMain(void);
static void GameWaitVBlankIntr(void);
static void VBlankIntr(void);

static void CheckProfMain(void);

//----------------------------------------------------------------------------
// initialized variable
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  メモリ確保関数
 *---------------------------------------------------------------------------*/
static void* AllocFunc(DWCAllocType name, u32 size, int align)
{
    void* ptr;
    OSIntrMode old;
    (void)name;
    (void)align;

    old = OS_DisableInterrupts();
    ptr = OS_AllocFromMain(size);
    OS_RestoreInterrupts(old);

    return ptr;
}

/*---------------------------------------------------------------------------*
  メモリ開放関数
 *---------------------------------------------------------------------------*/
static void FreeFunc(DWCAllocType name, void* ptr, u32 size)
{
    OSIntrMode old;
    (void)name;
    (void)size;

    if (!ptr) return;

    old = OS_DisableInterrupts();
    OS_FreeToMain(ptr);
    OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  OS_TPrintf()のコンソール出力対応
 *---------------------------------------------------------------------------*/
static char common_buffer[256];
#ifdef SDK_FINALROM
#undef OS_TVPrintf
#undef OS_TPrintf
#endif
extern void OS_TVPrintf(const char *fmt, va_list vlist);
void OS_TVPrintf(const char *fmt, va_list vlist)
{
    (void)OS_VSNPrintf(common_buffer, sizeof(common_buffer), fmt, vlist);

#ifndef SDK_FINALROM
    OS_PutString(common_buffer);
#endif

    if (sPrintOverride)
    {
        dbs_CVPrintf(NULL, fmt, vlist);
    }
}

extern void OS_TPrintf(const char *fmt, ...);
void OS_TPrintf(const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVPrintf(fmt, vlist);
    va_end(vlist);
}

/*---------------------------------------------------------------------------*
  Aボタン入力待ち
 *---------------------------------------------------------------------------*/
static void WaitKey(void)
{
    while(!(PAD_Read() & PAD_BUTTON_A))
    {
        GameWaitVBlankIntr();
    }
    while(PAD_Read() & PAD_BUTTON_A)
    {
        GameWaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  メインルーチン
 *---------------------------------------------------------------------------*/
void NitroMain ()
{
    OS_Init();
    GX_Init();

    // スタック溢れチェック初期設定
    OS_SetThreadStackWarningOffset(OS_GetCurrentThread(), 0x100);

    Heap_Init();

    // Vブランク割り込み許可
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    FS_Init( MI_DMA_MAX_NUM ); 

    // 出力初期化
    dbs_DemoInit();
    sPrintOverride = TRUE;  // OS_TPrintf()の出力をconsoleにつなぐ.

    // 表示開始
    OS_WaitVBlankIntr();    // Waiting the end of VBlank interrupt
    GX_DispOn();
    GXS_DispOn();

    // DWCの初期化
    InitDWC();

    // Wi-Fiコネクション設定
    NetConfigMain();

    // インターネット接続
    StartIPMain();

    /* 【注意】認証サーバへのログイン処理は省略しています */

    // 不正文字列のチェック開始待ち
    OS_TPrintf("\nPress [A] to start profanity check.\n\n");
    WaitKey();

    // 不正文字列のチェック
    CheckProfMain();

    // 切断待ち
    OS_TPrintf("\nPress [A] to disconnect.\n\n");
    WaitKey();

    // インターネット切断
    StopIPMain();

    // 終了
    OS_DumpHeap(OS_ARENA_MAIN, OS_CURRENT_HEAP_HANDLE);
    OS_TPrintf("\n");
    OS_TPrintf("*************************\n");
    OS_TPrintf("Please turn off NDS power\n");
    OS_TPrintf("*************************\n");
    while (1)
    {
        GameWaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  DWCの初期化
 *---------------------------------------------------------------------------*/
static void InitDWC(void)
{
    int ret;

    // デバッグ表示レベル指定
    DWC_SetReportLevel(DWC_REPORTFLAG_ALL);

    // DWCライブラリ初期化
#if defined( USE_AUTHSERVER_PRODUCTION )
    ret = DWC_InitForProduction( NULL, INITIAL_CODE, AllocFunc, FreeFunc );
#else
    ret = DWC_InitForDevelopment( NULL, INITIAL_CODE, AllocFunc, FreeFunc );
#endif
    
    OS_TPrintf( "DWC_InitFor*() result = %d\n", ret );

    if ( ret == DWC_INIT_RESULT_DESTROY_OTHER_SETTING )
    {
        OS_TPrintf( "Wi-Fi setting might be broken.\n" );
    }
}

/*---------------------------------------------------------------------------*
  Wi-Fiコネクション設定
 *---------------------------------------------------------------------------*/
FS_EXTERN_OVERLAY(main_overlay_1);

static void NetConfigMain(void)
{
    (void)FS_LoadOverlay(MI_PROCESSOR_ARM9, FS_OVERLAY_ID(main_overlay_1));

    sPrintOverride = FALSE; // OS_TPrintf()の出力を一時的に元に戻す.
    dbs_DemoFinalize();

    (void)DWC_StartUtilityEx(DWC_LANGUAGE_JAPANESE, DWC_UTILITY_TOP_MENU_FOR_JPN);

    (void)FS_UnloadOverlay(MI_PROCESSOR_ARM9, FS_OVERLAY_ID(main_overlay_1));

    // Vブランク割り込み許可
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    // 出力初期化
    dbs_DemoInit();
    sPrintOverride = TRUE;  // OS_TPrintf()の出力をconsoleにつなぐ.

    // 表示開始
    OS_WaitVBlankIntr();    // Waiting the end of VBlank interrupt
    GX_DispOn();
    GXS_DispOn();
}

/*---------------------------------------------------------------------------*
  インターネット接続
 *---------------------------------------------------------------------------*/
static void StartIPMain(void)
{
    DWC_InitInet(&stConnCtrl);

    DWC_ConnectInetAsync();

    // 接続処理
    while (!DWC_CheckInet())
    {
        DWC_ProcessInet();
        GameWaitVBlankIntr();
    }

    // 接続結果確認
    if (DWC_GetInetStatus() != DWC_CONNECTINET_STATE_CONNECTED)
    {
        OS_TPanic("Cannot connect.\n");
    }
}

static void StopIPMain(void)
{
    while(!DWC_CleanupInetAsync())
    {
        GameWaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  不正文字列チェックメインルーチン
 *---------------------------------------------------------------------------*/
// 判定用文字列(50個まで/合計で終端文字を含めて501文字まで)
static const u16 *words[] =
{
    L"badword", // 全リージョン共通不正文字列
    L"mario",
    L"\x308F\x308B\x3082\x3057", // 日本リージョン不正文字列「わるもし」(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\x30EF\x30EB\x30E2\x30B7", // 日本リージョン不正文字列「ワルモシ」(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\xB098\xC05C\xB2E8\xC5B4", // 韓国リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\xB098\xC05C\xBB38\xC7A5", // 韓国リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\x574F\x7528\x8BED", // 中国リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\x0062\x00E0\x0064\x0077", // 北米/欧州リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\x0062\x00E1\x0064\x0077", // 北米/欧州リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"\x00DF\x00E4\x0064\x0077", // 北米/欧州リージョン不正文字列(※諸事情によりこの文字列は現在全リージョンでエラーになります)
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario",
    L"badword",
    L"mario"
};

#define PROFTEST_WORDS_NUM (sizeof(words)/sizeof(words[0]))

static void CheckProfMain(void)
{
    DWCProfState state;
    char prof_result[PROFTEST_WORDS_NUM];
    int  prof_badwordsnum;
    int  i;

    // 不正文字列チェック処理を開始する
    OS_TPrintf("\nStart profanity check...\n");
    if (DWC_CheckProfanityExAsync(words, PROFTEST_WORDS_NUM, NULL, 0, prof_result, &prof_badwordsnum, DWC_PROF_REGION_ALL) == FALSE)
        OS_TPanic("DWC_CheckProfanityExAsync failed.");

    do {
        state = DWC_CheckProfanityProcess();
        GameWaitVBlankIntr();
    } while (state == DWC_PROF_STATE_OPERATING);

    if (state == DWC_PROF_STATE_SUCCESS)
    {
        OS_TPrintf("\nSucceeded.\n");
        OS_TPrintf("Bad words: %d/%d\n", prof_badwordsnum, PROFTEST_WORDS_NUM);
        OS_TPrintf("Result   : ");
        for(i = 0; i < PROFTEST_WORDS_NUM; i++)
            OS_TPrintf("%02x ", (u8)prof_result[i]);
        OS_TPrintf("\n");
    }
    else
    {
        int          errorCode;
        DWCErrorType errorType;
        DWCError     lastError;
        
        lastError = DWC_GetLastErrorEx(&errorCode, &errorType);
        OS_TPrintf("\nFailed.\n");
        OS_TPrintf("Error     : %d\n", lastError);
        OS_TPrintf("Error code: %d\n", -errorCode);
        OS_TPrintf("Error type: %d\n", errorType);
    }
}

/*---------------------------------------------------------------------------*
  Vブランク待ち関数
 *---------------------------------------------------------------------------*/
static void GameWaitVBlankIntr(void)
{
    const char loopstr[] = "/|\\-";
    static u32 loopcnt = 0;
    int i;

    dbs_Print(30, 0, "%c", loopstr[((loopcnt)>>2)&3]);
    loopcnt++;

    dbs_DemoUpdate();

    // 想定するゲームフレームに合わせてVブランク待ちの回数を変える
    for (i = 0; i < GAME_FRAME; i++)
    {
        OS_WaitVBlankIntr();
    }

    Heap_Debug();
    dbs_DemoLoad();

    // スタック溢れチェック
    OS_CheckStack(OS_GetCurrentThread());
}


/*---------------------------------------------------------------------------*
  Vブランク割り込み関数
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
