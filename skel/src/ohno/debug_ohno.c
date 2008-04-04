//============================================================================================
/**
 * @file	debug_ohno.c
 * @brief	デバッグ用関数
 * @author	ohno
 * @date	2006.11.29
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "debug_ohno.h"
#include "test_ui.h"
#include "test_rand.h"
#include "test_net.h"
#include "test_beacon.h"
#include "gfl_use.h"

#include "main.h"


//------------------------------------------------------------------
typedef struct {
	u32 debug_heap_id;
	GFL_PROCSYS * psys;
    SKEL_TEST_BEACON_WORK* pBeaconWork;
}DEBUG_OHNO_CONTROL;

static DEBUG_OHNO_CONTROL * DebugOhnoControl;


//------------------------------------------------------------------
// スリープ解除時に呼ばれる関数
//------------------------------------------------------------------

static void _sleepRelease(void *pWork)
{
    OS_TPrintf(" sleep release \n");
}


static GFL_PROC_RESULT _debugUIProcInit(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_OHNO_CONTROL * ctrl = p_work;

    GFL_UI_SleepReleaseSetFunc(_sleepRelease, NULL);
#if 1
    TEST_NET_Init();
#else
    //ビーコン探知機
    ctrl->pBeaconWork = TEST_BEACON_Init(ctrl->debug_heap_id);
#endif

    return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT _debugUIProcMain(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	GFL_PROC * subproc;
	DEBUG_OHNO_CONTROL * ctrl = p_work;

#if 1
//    TEST_UI_Main();  //UI TEST

    OS_TPrintf("%d \n",GFUser_GetPublicRand(32));
    OS_TPrintf("%d \n",GFUser_GetPublicRand(32));
    OS_TPrintf("%d \n",GFUser_GetPublicRand(32));
    OS_TPrintf("%d \n",GFUser_GetPublicRand(32));
    OS_TPrintf("%d \n",GFUser_GetPublicRand(32));
    
    
    if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_A){

        RTCDate date;
        RTCTime time;
        u32 seed;
        RTC_GetDateTime(&date, &time);
        seed = date.year + date.month * 0x100 * date.day * 0x10000
		+ time.hour * 0x10000 + (time.minute + time.second) * 0x1000000;

        TEST_RAND_Main(seed);  // RAND TEST
    }
    TEST_NET_Main();
#else
    // ビーコン探知機
    TEST_BEACON_Main(ctrl->pBeaconWork);
#endif

    
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT _debugUIProcEnd(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_OHNO_CONTROL * ctrl = p_work;

    TEST_BEACON_End(ctrl->pBeaconWork);
    return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
// UIテスト用プロセス定義テーブル
//------------------------------------------------------------------
static const GFL_PROC_DATA UITestProcTbl = {
	_debugUIProcInit,
	_debugUIProcMain,
	_debugUIProcEnd,
};

static void VBlankIntr(void)
{
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static	const	char	*_graphicFileTable[]={
	"src/sample_graphic/radar.narc",
};

//------------------------------------------------------------------
//  デバッグ用メイン関数
//------------------------------------------------------------------
void DebugOhnoInit(HEAPID heap_id)
{
	DEBUG_OHNO_CONTROL * ctrl;
	ctrl = GFL_HEAP_AllocMemory(heap_id, sizeof(DEBUG_OHNO_CONTROL));
	DebugOhnoControl = ctrl;
	ctrl->debug_heap_id = heap_id;

	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &UITestProcTbl, ctrl);

	//ARCシステム初期化
	GFL_ARC_Init(&_graphicFileTable[0],NELEMS(_graphicFileTable));

    // セルアクターシステム初期化
	// まずこの処理を行う必要があります。
	{
		static const GFL_CLSYS_INIT	param = {
			// メインとサブのサーフェース左上座標を設定します。
			// サーフェースのサイズは（256,192）にするのが普通なので、
			// メンバには入れませんでした。
			// 上下の画面をつなげて使用するときは、
			// サブサーフェースの左上座標を(0, 192)などにする必要があると思います。
			0, 0,		// メインサーフェースの左上座標（x,y）
			0, 256,		// サブサーフェースの左上座標（x,y）
			
			// 今はフルにOAMAttrを使用する場合の設定
			// 通信アイコンなどで0～3のOam領域を使えないときなどは、
			// OAMAttr管理数設定を変更する必要があります。
			1, 128-1,		// メインOAMマネージャのOamAttr管理数(開始No,管理数)
			0, 128,		// サブOAMマネージャのOamAttr管理数(開始No,管理数)
		};
		GFL_CLACT_Init( &param, GFL_HEAPID_APP );
	}


    //	GFL_UI_TP_Init(ctrl->debug_heap_id);


//    RTC_Init();
    
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugOhnoMain(void)
{
	DEBUG_OHNO_CONTROL * ctrl = DebugOhnoControl;
//	GFL_UI_TP_Main();
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugOhnoExit(void)
{
	DEBUG_OHNO_CONTROL * ctrl = DebugOhnoControl;
//	GFL_UI_TP_Exit();
	GFL_HEAP_FreeMemory(ctrl);
    DebugOhnoControl = NULL;
}


static GFL_PROC_RESULT DebugOhnoMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_OHNO_DEBUG,0x20000);

    DebugOhnoInit(HEAPID_OHNO_DEBUG);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
    DebugOhnoMain();
    return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
    DebugOhnoExit();
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugOhnoMainProcData = {
	DebugOhnoMainProcInit,
	DebugOhnoMainProcMain,
	DebugOhnoMainProcEnd,
};

