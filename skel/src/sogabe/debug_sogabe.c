//============================================================================================
/**
 * @file	debug_sogabe.c
 * @brief	デバッグ＆テスト用アプリ（曽我部さん用）
 * @author	tamada
 * @date	2007.02.01
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"

#include "yt_common.h"
#include "title.h"
#include "game.h"

#include "testmode.h"

#define	TCB_WORK_SIZE		(0x10000)
#define	ACTIVE_TCB_MAX		(144)

typedef	void	(*ytFunc)(GAME_PARAM *);

static	const	char	*GraphicFileTable[]={
	"src/sample_graphic/yossyegg.narc",
};

static	void	game_init(GAME_PARAM *gp);

extern	void	YT_InitTitle(GAME_PARAM *);
extern	void	YT_MainTitle(GAME_PARAM *);

//============================================================================================
//
//
//		Jobテーブル
//
//
//============================================================================================

static	ytFunc	YT_JobTable[]={
	YT_InitTitle,
	YT_MainTitle,
//	YT_InitGame,
//	YT_MainGame,
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
static GFL_PROC_RESULT DebugSogabeMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAME_PARAM	*gp;

	gp=GFL_PROC_AllocWork(proc, sizeof(GAME_PARAM), GFL_HEAPID_APP);

	gp->job_no=YT_InitTitleNo;

	game_init(gp);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
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
static GFL_PROC_RESULT DebugSogabeMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugSogabeMainProcData = {
	DebugSogabeMainProcInit,
	DebugSogabeMainProcMain,
	DebugSogabeMainProcEnd,
};

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
	gp->tcbsys=GFL_TCB_SysInit(ACTIVE_TCB_MAX,gp->tcb_work);

	//ARCシステム初期化
	GFL_ARC_sysInit(&GraphicFileTable[0],1);

	//FADEシステム初期化
	GFL_FADE_sysInit(GFL_HEAPID_APP);

	//タッチパネル初期化
	GFL_UI_sysInit(GFL_HEAPID_APP);
	GFL_UI_TP_sysInit(GFL_HEAPID_APP);

	//セルアクター初期化
//	gp->clact_wk = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(DEBUG_CLACT) );

	// セルアクターシステム初期化
	// まずこの処理を行う必要があります。
	{
		static const CLSYS_INIT	param = {
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
			0, 128,		// メインOAMマネージャのOamAttr管理数(開始No,管理数)
			0, 128,		// サブOAMマネージャのOamAttr管理数(開始No,管理数)
		};
		GFL_CLACT_SysInit( &param, GFL_HEAPID_APP );
	}
}

