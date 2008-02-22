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
#if 0
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

    


    if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_L){
        OSTick tick;
        u64 time;
        MATHCRC16Table localTable;
        int i;
        int testsize = 364+288*2;
        u8* pDummyBuff = GFL_HEAP_AllocMemory(ctrl->debug_heap_id, testsize);

        OS_InitTick();
        OS_SetTick((OSTick)0);
        MATH_CRC16CCITTInitTable( &localTable );
        
        for(i=0;i<100;i++){
            MATH_CalcCRC16CCITT( &localTable, pDummyBuff, testsize );
            pDummyBuff[i%testsize]++;
            MATH_CalcCRC16CCITT( &localTable, pDummyBuff, testsize );
        }
        tick = OS_GetTick();
        time = OS_TicksToMilliSeconds(tick);
        OS_TPrintf("時間 %d\n",time);
        GFL_HEAP_FreeMemory(pDummyBuff);
    }


#if 0
//    TEST_UI_Main();  //UI TEST
    
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


//----------------------------------------------よっしの卵キーシェアリング

#include "key_yt_common.h"
#include "key_title.h"
#include "key_game.h"

#include "testmode.h"

#define	TCB_WORK_SIZE		(0x10000)
#define	ACTIVE_TCB_MAX		(144)

typedef	void	(*ytFunc)(GAME_PARAM *);

static	const	char	*GraphicFileTable[]={
	"src/sample_graphic/yossyegg.narc",
	"src/sample_sound/yossy_egg.narc",
	"src/sample_graphic/spa.narc",
};

static	void	game_init(GAME_PARAM *gp);

extern	void	KEY_YT_Init3DTest(GAME_PARAM *gp);
extern	void	KEY_YT_Main3DTest(GAME_PARAM *gp);
extern	void	KEY_YT_InitBlockOut(GAME_PARAM *gp);
extern	void	KEY_YT_MainBlockOut(GAME_PARAM *gp);

//============================================================================================
//
//
//		Jobテーブル
//
//
//============================================================================================

static	ytFunc	YT_JobTable[]={
	KEY_YT_InitTitle,
	KEY_YT_MainTitle,
	KEY_YT_InitGame,
	KEY_YT_MainGame,
};

//============================================================================================
//
//
//		プロセスの定義
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAME_PARAM	*gp;

	GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_OHNO_DEBUG,0x180000);
	gp=GFL_PROC_AllocWork(proc, sizeof(GAME_PARAM), HEAPID_OHNO_DEBUG);

	GFL_STD_MemClear(gp,sizeof(GAME_PARAM));

	gp->heapID=HEAPID_OHNO_DEBUG;

	KEY_YT_JobNoSet(gp,KEY_YT_InitTitleNo);

	game_init(gp);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAME_PARAM	*gp=(GAME_PARAM *)mywk;

	YT_JobTable[gp->job_no](gp);

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**
 * @brief		ゲームごとの初期化処理
 */
//------------------------------------------------------------------
static	void	game_init(GAME_PARAM *gp)
{
	//TCB初期化
	//タスクワーク確保
	gp->tcb_work=GFL_HEAP_AllocMemory(GFL_HEAPID_APP,TCB_WORK_SIZE);
	gp->tcbsys=GFL_TCB_Init(ACTIVE_TCB_MAX,gp->tcb_work);

	//DMAシステム初期化
	GFL_DMA_Init(GFL_HEAPID_APP);

	//ARCシステム初期化
	GFL_ARC_Init(&GraphicFileTable[0],1);

	//FADEシステム初期化
	GFL_FADE_Init(GFL_HEAPID_APP);

	//タッチパネル初期化
	GFL_UI_TP_Init(GFL_HEAPID_APP);

	//セルアクター初期化
	gp->clact = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(KEY_YT_CLACT) );

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

	//サウンドデータロード
//	GFL_SOUND_LoadArchiveData("src/sample_sound/yossy_egg.sdat");
	GFL_SOUND_LoadArchiveData(1,0);
	GFL_SOUND_LoadGroupData(GROUP_SE);
	GFL_SOUND_LoadGroupData(GROUP_JINGLE);
	gp->mus_level[MUS_LEVEL_JINGLE]=GFL_SOUND_SaveHeapState();
}

//------------------------------------------------------------------
/**
 * @brief		gp->job_noへの代入
 */
//------------------------------------------------------------------
void	KEY_YT_JobNoSet(GAME_PARAM *gp,int job_no)
{
	gp->job_no=job_no;
	gp->seq_no=0;
}






#define COMMENT_OHNO 0
#if COMMENT_OHNO

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
#endif  //COMMENT_OHNO

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugOhnoMainProcData = {
	DebugOhnoMainProcInit,
	DebugOhnoMainProcMain,
	DebugOhnoMainProcEnd,
};

