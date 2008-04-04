//=============================================================================================
/**
 * @file	gfl_use.c
 * @date	2006.12.06
 * @author	GAME FREAK inc.
 * @brief	ユーザーのGFライブラリ使用のためのスケルトンソース
 *
 */
//=============================================================================================

#include <nitro.h>
#include "gflib.h"
//#include "gflib_inter.h"

#include "main.h"

#include "gfl_use.h"
#include "procsys.h"
#include "gf_overlay.h"
#include "tcb.h"
#include "backup_system.h"


//=============================================================================================
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	TCB_HINTR_MAX = 4,
	TCB_VINTR_MAX = 16,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
    GFL_STD_RandContext publicRandContext;
	GFL_TCBSYS *	TCBSysHintr;
	void *		TCBMemHintr;
	GFL_TCBSYS *	TCBSysVintr;
	void *		TCBMemVintr;
}GFL_USE_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static const HEAP_INIT_HEADER hih[]={
	{ HEAPSIZE_SYSTEM,	OS_ARENA_MAIN },
	{ HEAPSIZE_APP,		OS_ARENA_MAIN },
};

#define	SOUND_HEAP_SIZE	(0x40000)

static GFL_USE_WORK *	gfl_work = NULL;
static int				GFL_USE_VintrCounter;
static	u8				sndHeap[SOUND_HEAP_SIZE];

static void GFUser_PublicRandInit(void);

//=============================================================================================
//
//			関数
//
//=============================================================================================
//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：初期化
 *
 * InitSystemの内部で呼んでいるInitFileSystemはgflibの外では
 * 呼べないため、InitSystemを呼んでいる。
 */
//------------------------------------------------------------------
void GFLUser_Init(void)
{
	InitSystem();
#if 0
#ifdef USE_MAINEXMEMORY
  OS_EnableMainExArena();
#endif

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
	InitFileSystem();
#endif

	//ヒープシステム初期化
	GFL_HEAP_Init(&hih[0],GFL_HEAPID_MAX,HEAPID_CHILD_MAX,0);	//メインアリーナ
	GFL_HEAP_DTCM_Init( 0x2000 );				//ＤＴＣＭアリーナ

	//アーカイブシステム初期化
	//GFL_ARC_SysInit(...);

	//UIシステム初期化
	GFL_UI_Boot(GFL_HEAPID_SYSTEM);

	//OVERLAYシステム初期化
	GFL_OVERLAY_boot(GFL_HEAPID_SYSTEM, 8, 4, 4);

	//PROCシステム初期化
	GFL_PROC_boot(GFL_HEAPID_SYSTEM);
	gfl_work = GFL_HEAP_AllocMemory(GFL_HEAPID_SYSTEM, sizeof(GFL_USE_WORK));
	gfl_work->TCBMemHintr = GFL_HEAP_AllocMemory(
		  GFL_HEAPID_SYSTEM, GFL_TCB_CalcSystemWorkSize(TCB_HINTR_MAX));
	gfl_work->TCBSysHintr = GFL_TCB_Init(TCB_HINTR_MAX, gfl_work->TCBMemHintr);
	gfl_work->TCBMemVintr = GFL_HEAP_AllocMemory(
		  GFL_HEAPID_SYSTEM, GFL_TCB_CalcSystemWorkSize(TCB_VINTR_MAX));
	gfl_work->TCBSysVintr = GFL_TCB_Init(TCB_VINTR_MAX, gfl_work->TCBMemVintr);
    // 汎用乱数初期化
    GFUser_PublicRandInit();
    
    //FADEシステム初期化
    GFL_FADE_Init(GFL_HEAPID_SYSTEM);

    //サウンドシステム初期化
    GFL_SOUND_Init(&sndHeap[0],SOUND_HEAP_SIZE);

	//バックアップシステム初期化
	GFL_BACKUP_Init(GFL_HEAPID_APP, GFL_HEAPID_APP);

	GFL_USE_VintrCounter = 0;
}


//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：メイン呼び出し
 *
 * 毎フレーム呼び出される必要があるメイン処理を記述する
 */
//------------------------------------------------------------------
void GFLUser_Main(void)
{
	GFL_UI_Main();
    GFL_NET_Main();  //キーの処理の後すぐに通信を処理したい為ここに配置
	
	GFL_PROC_Main();
    GFL_FADE_Main();
	GFL_SOUND_Main();
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：描画処理
 *
 * 毎フレーム呼び出される必要がある描画処理を記述する
 */
//------------------------------------------------------------------
void GFLUser_Display(void)
{
	//GFL_OAM_sysMain(...);
	//GFL_BG_sysMain(...);
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：終了処理
 */
//------------------------------------------------------------------
void GFLUser_Exit(void)
{
	GFL_BACKUP_Exit();
	GFL_UI_Exit();
	GFL_PROC_Exit();
	GFL_OVERLAY_Exit();
	GFL_FADE_Exit();
	GFL_TCB_Exit(gfl_work->TCBSysVintr);
	GFL_HEAP_FreeMemory(gfl_work->TCBSysVintr);
	GFL_HEAP_FreeMemory(gfl_work->TCBMemVintr);
	GFL_HEAP_DTCM_Exit();
	GFL_HEAP_Exit();
	GFL_SOUND_Exit();
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：HBlank割り込み処理
 */
//------------------------------------------------------------------
void GFLUser_HIntr(void)
{
	GFL_TCB_Main(gfl_work->TCBSysHintr);
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：HBlank中TCBの登録処理
 */
//------------------------------------------------------------------
GFL_TCB * GFUser_HIntr_CreateTCB(GFL_TCB_FUNC * func, void * work, u32 pri)
{
	return GFL_TCB_AddTask(gfl_work->TCBSysHintr, func, work, pri);
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：VBlank割り込み処理
 */
//------------------------------------------------------------------
void GFLUser_VIntr(void)
{
	GFL_TCB_Main(gfl_work->TCBSysVintr);
	GFL_BG_VBlankFunc();
	// Vブランク期間で実行します。
	// ただ、ユニットの描画が行われていないのに
	// この関数を実行すると、描画しているOBJが消えてしまうため
	// 割り込みないで呼ばないほうが良いかもしれません。
	//GFL_CLACT_VBlankFunc();
    // 通信アイコンの描画のためにあります。通信自体は行っていません
    GFL_NET_VBlankFunc();

	GFL_DMA_Main();
	GFL_USE_VintrCounter++;
}

//------------------------------------------------------------------
/**
 * @brief	GFライブラリ利用部分：VBlank中TCBの登録処理
 */
//------------------------------------------------------------------
GFL_TCB * GFUser_VIntr_CreateTCB(GFL_TCB_FUNC * func, void * work, u32 pri)
{
	return GFL_TCB_AddTask(gfl_work->TCBSysVintr, func, work, pri);
}

//------------------------------------------------------------------
/**
 * @brief	GFUser 汎用乱数作成
 */
//------------------------------------------------------------------
static void GFUser_PublicRandInit(void)
{
    RTCDate date;
    RTCTime time;
    u32 seed;
    u64 seed64;
    RTC_GetDateTime(&date, &time);
    seed = date.year + date.month * 0x100 * date.day * 0x10000
        + time.hour * 0x10000 + (time.minute + time.second) * 0x1000000;

    GFL_STD_MtRandInit(seed);
    seed64 = GFL_STD_MtRand( 0 );
    seed64 = (seed64 << 32) + GFL_STD_MtRand( 0 );
    GFL_STD_RandInit( &gfl_work->publicRandContext, seed64 );
}

//------------------------------------------------------------------
/**
 * @brief	GFUser 汎用乱数を得る
 */
//------------------------------------------------------------------
u32 GFUser_GetPublicRand(u32 range)
{
    return GFL_STD_Rand( &gfl_work->publicRandContext, range );
}

//------------------------------------------------------------------
/**
 * @brief	VBlankカウンター取得とリセット
 */
//------------------------------------------------------------------
void GFUser_VIntr_ResetVblankCounter( void )
{
	GFL_USE_VintrCounter = 0;
}

int GFUser_VIntr_GetVblankCounter( void )
{
	return GFL_USE_VintrCounter;
}



