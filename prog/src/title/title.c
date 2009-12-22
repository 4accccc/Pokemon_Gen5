//==============================================================================
/**
 * @file	title.c
 * @brief	タイトル画面
 * @author	matsuda
 * @date	2008.12.03(水)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "msg\msg_title.h"
#include "test/performance.h"
#include "test/testmode.h"
#include "font/font.naix"
#include "title.naix"
#include "title1.naix"
#include "title/title.h"
#include "title/startmenu.h"
#include "system\gfl_use.h"

#ifdef PM_DEBUG
#include "corporate.h"
#endif


//==============================================================================
//	定数定義
//==============================================================================
/// 入力トリガマスク
#ifdef PM_DEBUG
#define NEXT_PROC_MASK	(PAD_BUTTON_START | PAD_BUTTON_SELECT | PAD_BUTTON_A)
#else
#define NEXT_PROC_MASK	(PAD_BUTTON_START | PAD_BUTTON_A)
#endif

#define TOTAL_WAIT			(60*20)

/// シーケンスID定義
enum{
	SEQ_INIT = 0,
	SEQ_SETUP,
	SEQ_FADEIN,
	SEQ_MAIN,
	SEQ_NEXT,
	SEQ_FADEOUT,
	SEQ_END,
	SEQ_SKIP,
};

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_TCBLSYS*	tcbl;
	GFL_TCB				*vintr_tcb;
}SYS_CONTROL;

typedef struct{
	GFL_BMPWIN*			bmpwin;
	GFL_BMP_DATA*		bmp;
	PRINT_UTIL			printUtil;
	BOOL						message_req;		///<TRUE:メッセージリクエストがあった
	GFL_FONT*				fontHandle;
	PRINT_QUE*			printQue;
	PRINT_STREAM*		printStream;
	GFL_MSGDATA*		mm;
	STRBUF*					strbufENG;
	int							push_timer;				//[PUSH START BUTTON]の点滅間隔をカウント
	int							push_visible;
}G2D_CONTROL;

typedef struct{
	GFL_G3D_UTIL*			g3Dutil;
	u16								g3DutilUnitIdx;
  GFL_G3D_LIGHTSET* g3Dlightset;
}G3D_CONTROL;

typedef struct{
	GFL_CLUNIT*								clunit;
	GFL_CLWK*									clwk_icon;		//アイテムアイコンアクター
	NNSG2dImagePaletteProxy		icon_pal_proxy;
	NNSG2dCellDataBank*				icon_cell_data;
	NNSG2dAnimBankData*				icon_anm_data;
	void*											icon_cell_res;
	void*											icon_anm_res;
}OAM_CONTROL;

typedef struct {
	u16						seq;
	HEAPID				heapID;
	int						mode;			///<次のメニュー画面へ引き渡すモード
	
	SYS_CONTROL		CSys;
	G2D_CONTROL		CG2d;
	G3D_CONTROL		CG3d;
	OAM_CONTROL		COam;
	
	u32						totalWait;
}TITLE_WORK;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
GFL_PROC_RESULT TitleProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT TitleProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT TitleProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//	外部関数宣言
//==============================================================================
extern const	GFL_PROC_DATA TestMainProcData;
FS_EXTERN_OVERLAY(testmode);

extern const	GFL_PROC_DATA CorpProcData;
FS_EXTERN_OVERLAY(title);

//==============================================================================
//	データ
//==============================================================================
//--------------------------------------------------------------
//	PROC
//--------------------------------------------------------------
const GFL_PROC_DATA TitleProcData = {
	TitleProcInit,
	TitleProcMain,
	TitleProcEnd,
};

//==============================================================================
//
//	
//
//==============================================================================
static void initIO(void);
static void initVRAM(void);

static void setupSystem(SYS_CONTROL* CSys, HEAPID heapID);
static void mainSystem(SYS_CONTROL* CSys, HEAPID heapID);
static void releaseSystem(SYS_CONTROL* CSys);

static void setupG2Dcontrol(G2D_CONTROL* CG2d, HEAPID heapID);
static void mainG2Dcontrol(G2D_CONTROL* CG2d, HEAPID heapID);
static void releaseG2Dcontrol(G2D_CONTROL* CG2d);

static void setupG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID);
static void mainG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID);
static void releaseG3Dcontrol(G3D_CONTROL* CG3d);

static void setupOAMcontrol(OAM_CONTROL* COam, HEAPID heapID);
static void mainOAMcontrol(OAM_CONTROL* COam, HEAPID heapID);
static void releaseOAMcontrol(OAM_CONTROL* COam);

static void MainFunc(TITLE_WORK* tw);
static void VintrFunc(GFL_TCB *tcb, void *work);
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT TitleProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK *tw;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TITLE_DEMO, 0x70000 );
	tw = GFL_PROC_AllocWork( proc, sizeof(TITLE_WORK), HEAPID_TITLE_DEMO );
	GFL_STD_MemClear(tw, sizeof(TITLE_WORK));
	tw->heapID = HEAPID_TITLE_DEMO;

#ifdef PM_DEBUG	// デバッグ用スキップ処理
	if( pwk != NULL ){
		u32 * corp = pwk;
		if( *corp == CORPORATE_RET_DEBUG ){
			tw->mode = PAD_BUTTON_SELECT;
			tw->seq = SEQ_SKIP;
		}
	}
	DEBUG_PerformanceSetActive(TRUE);	//デバッグ：パフォーマンスメーター有効
#endif

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
GFL_PROC_RESULT TitleProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK* tw = mywk;

	switch(tw->seq){
	case SEQ_INIT:
		// 初期化
		initIO();
		initVRAM();

		tw->seq = SEQ_SETUP;
		break;

	case SEQ_SETUP:
		// セットアップ
		setupSystem(&tw->CSys, tw->heapID);
		setupG2Dcontrol(&tw->CG2d, tw->heapID);
		setupG3Dcontrol(&tw->CG3d, tw->heapID);
		setupOAMcontrol(&tw->COam, tw->heapID);

		// FADEIN設定
		GFL_FADE_SetMasterBrightReq
			(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 2);
		tw->totalWait = 0;

		tw->seq = SEQ_FADEIN;
		break;

	case SEQ_FADEIN:
		if(GFL_FADE_CheckFade() == FALSE){ tw->seq = SEQ_MAIN; }
		MainFunc(tw);
		break;

	case SEQ_MAIN:
		{
			// 入力判別処理
			int trg = GFL_UI_KEY_GetTrg() & NEXT_PROC_MASK;
				if(trg){
				tw->mode = trg;	// ゲームモード選択/デバッグモード(SELECT)に進む
				tw->seq = SEQ_NEXT;
			}
		}
		if( GFL_UI_TP_GetTrg() == TRUE ){
			tw->mode = 1;				// ゲームモード選択に進む(== push A button)
			tw->seq = SEQ_NEXT;
		}
		// 表示時間判別処理
		tw->totalWait++;
		if(tw->totalWait > TOTAL_WAIT){
			tw->mode = 0;					// Corporate表記に戻る
			tw->seq = SEQ_NEXT;
		}
		MainFunc(tw);
		break;

	case SEQ_NEXT:
		// FADEOUT設定
		GFL_FADE_SetMasterBrightReq
			(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 2);
		tw->seq = SEQ_FADEOUT;
		MainFunc(tw);
		break;

	case SEQ_FADEOUT:
		if(GFL_FADE_CheckFade() == FALSE){ tw->seq = SEQ_END; }
		MainFunc(tw);
		break;

	case SEQ_END:
		// 解放
		releaseOAMcontrol(&tw->COam);
		releaseG3Dcontrol(&tw->CG3d);
		releaseG2Dcontrol(&tw->CG2d);
		releaseSystem(&tw->CSys);

		return GFL_PROC_RES_FINISH;

	case SEQ_SKIP:
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
GFL_PROC_RESULT TitleProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK* tw = mywk;
	int i, mode;
	
	mode = tw->mode;
	
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_TITLE_DEMO);
	
	if( !mode ){
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &TitleControlProcData, NULL);
#ifdef PM_DEBUG	// デバッグ用スキップ処理
	} else if( mode & PAD_BUTTON_SELECT ){
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(testmode), &TestMainProcData, NULL );
#endif
	} else {
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &StartMenuProcData, NULL);
	}
	return GFL_PROC_RES_FINISH;
}

//==============================================================================
//	
//	Init
//
//==============================================================================
//==============================================================================
//	I/O
//==============================================================================
static void initIO(void)
{
	// 各種効果レジスタ初期化
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	G2_BlendNone();
	G2S_BlendNone();
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
}

//==============================================================================
//	VRAM
//==============================================================================
static const GFL_DISP_VRAM vramBank = {
	GX_VRAM_BG_128_D,								// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,					// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,						// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,			// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_64_E,								// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,				// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,						// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,		// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_01_AB,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,					// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_64K,			// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,			// サブOBJマッピングモード
};

static void initVRAM(void)
{
	static const GFL_BG_SYS_HEADER sysHeader = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
	};
	//VRAM設定
	GFL_DISP_SetBank( &vramBank );
	GFL_BG_SetBGMode( &sysHeader );
}

//==============================================================================
//	
//	Function
//
//==============================================================================
//==============================================================================
//	Main
//==============================================================================
static void MainFunc(TITLE_WORK* tw)
{
	mainSystem(&tw->CSys, tw->heapID);
	mainG2Dcontrol(&tw->CG2d, tw->heapID);
	mainG3Dcontrol(&tw->CG3d, tw->heapID);
	mainOAMcontrol(&tw->COam, tw->heapID);
}

//==============================================================================
//	VSync
//==============================================================================
static void VintrFunc(GFL_TCB *tcb, void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
}

//==============================================================================
//	
//	Setup & Release
//
//==============================================================================
//==============================================================================
//	System
//==============================================================================
#define DTCM_SIZE		(0x1000)

//------------------------------------------------------------------------------
static void setupSystem(SYS_CONTROL* CSys, HEAPID heapID)
{
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
								GFL_G3D_VMANLNK, GFL_G3D_PLT32K,
								DTCM_SIZE, heapID, NULL );
	GFL_FONTSYS_Init();		// 初期化のみリリースなし

	CSys->tcbl = GFL_TCBL_Init( heapID, heapID, 4, 32 );
	CSys->vintr_tcb = GFUser_VIntr_CreateTCB(VintrFunc, NULL, 5);

	// 上下画面設定
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();
}

//------------------------------------------------------------------------------
static void mainSystem(SYS_CONTROL* CSys, HEAPID heapID)
{
	GFL_TCBL_Main( CSys->tcbl );
}

//------------------------------------------------------------------------------
static void releaseSystem(SYS_CONTROL* CSys)
{
	// 上下画面設定
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();

	GFL_TCB_DeleteTask(CSys->vintr_tcb);
	GFL_TCBL_Exit(CSys->tcbl);

	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}

//==============================================================================
//	G2DControl
//==============================================================================
///フォントが使用するパレット番号
#define D_FONT_PALNO	(14)
///アイテムアイコンのパレット展開位置
#define D_MATSU_ICON_PALNO		(0)
///[PUSH START BUTTON]の点滅間隔
#define PUSH_TIMER_WAIT			(45)

///タイトルロゴBGのBGプライオリティ
enum{
	BGPRI_MSG = 0,
	BGPRI_TITLE_LOGO = 2,
	BGPRI_3D = 1,
	BGPRI_BKGR = 2,
};

enum{
	FRAME_BKGR = GFL_BG_FRAME3_M,			// メイン画面背景
	FRAME_MSG = GFL_BG_FRAME1_S,		// メッセージフレーム
	FRAME_LOGO = GFL_BG_FRAME3_S,		// タイトルロゴのBGフレーム
};

//--------------------------------------------------------------
static void setupG2Dcontrol(G2D_CONTROL* CG2d, HEAPID heapID)
{
	//BGフレーム設定
	{
		static const GFL_BG_BGCNT_HEADER main_bkgr_frame = {	//メイン画面BGフレーム
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x04000,
			GX_BG_EXTPLTT_01, BGPRI_BKGR, 0, 0, FALSE
		};
		static const GFL_BG_BGCNT_HEADER sub_msg_frame = {	//サブ画面BGフレーム
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x04000,
			GX_BG_EXTPLTT_01, BGPRI_MSG, 0, 0, FALSE
		};
		static const GFL_BG_BGCNT_HEADER sub_logo_frame = {	//サブ画面BGフレーム
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( FRAME_BKGR, &main_bkgr_frame, GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( FRAME_MSG, &sub_msg_frame, GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( FRAME_LOGO, &sub_logo_frame, GFL_BG_MODE_TEXT );
	}
	//メイン画面背景設定
	{
		//GFL_BG_SetBackGroundColor(GFL_BG_FRAME0_M, GX_RGB(25, 31, 31));
		GFL_ARC_UTIL_TransVramBgCharacter(
			ARCID_TITLETEST, NARC_title1_sky_NCGR, FRAME_BKGR, 0, 0x4000, 0, heapID);
		GFL_ARC_UTIL_TransVramScreen(
			ARCID_TITLETEST, NARC_title1_sky_NSCR, FRAME_BKGR, 0, 0, 0, heapID);
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_TITLETEST, NARC_title1_sky_NCLR, PALTYPE_MAIN_BG, 0, 0, heapID);

		GFL_BG_LoadScreenReq( FRAME_BKGR );
		GFL_BG_SetVisible(FRAME_BKGR, VISIBLE_ON);
	}
	//グラフィックデータロード(LOGO)
	{
		GFL_ARC_UTIL_TransVramBgCharacter(
			ARCID_TITLE, NARC_title_wb_logo_bk2_NCGR, FRAME_LOGO, 0, 0x8000, 0, heapID);
		GFL_ARC_UTIL_TransVramScreen(
			ARCID_TITLE, NARC_title_wb_logo_bk2_NSCR, FRAME_LOGO, 0, 0, 0, heapID);
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_TITLE, NARC_title_wb_logo_bk2_NCLR, PALTYPE_SUB_BG, 0, 0, heapID);

		GFL_BG_LoadScreenReq( FRAME_LOGO );
		GFL_BG_SetVisible(FRAME_LOGO, VISIBLE_ON);
	}
	//メッセージ関連作成(MSG)
	{
		GFL_BG_FillCharacter( FRAME_MSG, 0x00, 1, 0 );
		GFL_BG_FillScreen( FRAME_MSG, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		//BMPWIN作成
		CG2d->bmpwin = GFL_BMPWIN_Create
										(FRAME_MSG, 0, 17, 32, 2, D_FONT_PALNO, GFL_BMP_CHRAREA_GET_F);
		CG2d->bmp = GFL_BMPWIN_GetBmp( CG2d->bmpwin );
		GFL_BMP_Clear( CG2d->bmp, 0x00 );
		GFL_BMPWIN_MakeScreen( CG2d->bmpwin );
		GFL_BMPWIN_TransVramCharacter( CG2d->bmpwin );
		PRINT_UTIL_Setup( &CG2d->printUtil, CG2d->bmpwin );
		GFL_BG_LoadScreenReq( FRAME_MSG );

		//フォントパレット転送(緑字)
//		GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT + D_FONT_PALNO*0x20+2), GX_RGB(0, 20, 0), 0x20-2);
//		GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT + D_FONT_PALNO*0x20+6), GX_RGB(0, 31, 0), 2);

		//フォント&プリントキューシステム作成
		CG2d->fontHandle = GFL_FONT_Create
			( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		CG2d->printQue = PRINTSYS_QUE_Create( heapID );
		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI

		//メッセージ取得
		CG2d->mm = GFL_MSG_Create
							( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_title_dat, heapID );
		CG2d->strbufENG = GFL_STR_CreateBuffer( 64, heapID );
		GFL_MSG_GetString(CG2d->mm, TITLE_STR_001, CG2d->strbufENG);
		//メッセージ表示
		PRINTSYS_PrintQue(CG2d->printQue, CG2d->bmp, 64, 0, CG2d->strbufENG, CG2d->fontHandle);
		CG2d->message_req = TRUE;
	}
}

static void mainG2Dcontrol(G2D_CONTROL* CG2d, HEAPID heapID)
{
	// "PUSH START" 点滅処理
	CG2d->push_timer++;
	if(CG2d->push_timer > PUSH_TIMER_WAIT){
		CG2d->push_timer = 0;
		CG2d->push_visible ^= 1;
		GFL_BG_SetVisible(FRAME_MSG, CG2d->push_visible);
	}
	// メッセージキュー処理
	{
		BOOL que_ret = PRINTSYS_QUE_Main( CG2d->printQue );
	
		if( PRINT_UTIL_Trans( &CG2d->printUtil, CG2d->printQue ) != FALSE){
			if(que_ret == TRUE && CG2d->message_req == TRUE){
				GFL_BMPWIN_TransVramCharacter( CG2d->bmpwin );
				CG2d->message_req = FALSE;
			}
		}
	}
}

static void releaseG2Dcontrol(G2D_CONTROL* CG2d)
{
	GFL_STR_DeleteBuffer(CG2d->strbufENG);
	GFL_MSG_Delete(CG2d->mm);
	PRINTSYS_QUE_Delete(CG2d->printQue);
	GFL_FONT_Delete(CG2d->fontHandle);
	GFL_BMPWIN_Delete(CG2d->bmpwin);
}

//==============================================================================
//	G3DControl
//==============================================================================
static const GFL_G3D_LOOKAT cameraLookAt = {
	//{ -252.446045*FX32_ONE, 268.627930*FX32_ONE, 721.645996*FX32_ONE },	//カメラの位置(＝視点)
	{ -477.457031*FX32_ONE, 205.554199*FX32_ONE, 784.570801*FX32_ONE },	//カメラの位置(＝視点)
	{ 0, FX32_ONE, 0 },												//カメラの上方向
	//{ 110.435803*FX32_ONE, 1395.428467*FX32_ONE, 279.511475*FX32_ONE },	//カメラの焦点(＝注視点)
	{ 195.185791*FX32_ONE, 1299.894775*FX32_ONE, 189.700439*FX32_ONE },	//カメラの焦点(＝注視点)
};
#define cameraPerspway	( 30/2 * PERSPWAY_COEFFICIENT )
#define cameraAspect	( FX32_ONE * 4/3 )
#define cameraNear		( 32 << FX32_SHIFT )
#define cameraFar		( 2048 << FX32_SHIFT )

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
  { 0, {{ (FX16_ONE-1), -(FX16_ONE-1)/2, -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 1, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };

#define g3DanmRotateSpeed	( 0x100 )
#define g3DanmFrameSpeed	( FX32_ONE )

static const GFL_G3D_OBJSTATUS drawStatus[] = {
	{
		{ 0, 0, 0 },																				//座標
		{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
	},
	{
		//{ 0xfffebef9, 0x00378800, 0x0019a82f },	//座標
		{ 0xffffecf9, 0x00392c00, 0x001a122f },	//座標
		//{ 0x3000, 0x3000, 0x3000 },											//スケール
		{ 0x4600, 0x4600, 0x4600 },											//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },					//回転
	},
};

enum{
	G3DRES_BRIDGE_BMD = 0,
	G3DRES_PM_BMD,
	G3DRES_PM_BCA,
};

//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_TITLETEST, NARC_title1_h01_all_00002_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_TITLETEST, NARC_title1_wbpm_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_TITLETEST, NARC_title1_wbpm_nsbca, GFL_G3D_UTIL_RESARC },
};

//3Dアニメ
static const GFL_G3D_UTIL_ANM g3Dutil_anm1Tbl[] = {
	{ G3DRES_PM_BCA, 0	},
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{
		G3DRES_BRIDGE_BMD, 		//モデルリソースID
		0,										//モデルデータID(リソース内部INDEX)
		G3DRES_BRIDGE_BMD, 		//テクスチャリソースID
		NULL,									//アニメテーブル(複数指定のため)
		0,										//アニメリソース数
	},
	{
		G3DRES_PM_BMD,				//モデルリソースID
		0,										//モデルデータID(リソース内部INDEX)
		G3DRES_PM_BMD,				//テクスチャリソースID
		g3Dutil_anm1Tbl, 			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anm1Tbl),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl,						//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,						//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

#define G3DUTIL_RESCOUNT	(NELEMS(g3Dutil_resTbl))
#define G3DUTIL_OBJCOUNT	(NELEMS(g3Dutil_objTbl))

//VecFx32 debugVec;
//fx32 debugScale;
//--------------------------------------------------------------
static void setupG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID)
{
	u16 objIdx;
	
	CG3d->g3Dutil = GFL_G3D_UTIL_Create(G3DUTIL_RESCOUNT, G3DUTIL_OBJCOUNT, heapID);
	CG3d->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( CG3d->g3Dutil, &g3Dutil_setup );

	//アニメーションを有効にする
	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );
	GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(CG3d->g3Dutil, objIdx + 1), 0); 

	//モデル１のマテリアル再設定
	{
		NNSG3dResFileHeader* fs = GFL_G3D_GetResourceFileHeader
													(GFL_G3D_UTIL_GetResHandle(CG3d->g3Dutil, G3DRES_PM_BMD));
		NNSG3dResMdlSet* ms = NNS_G3dGetMdlSet(fs);
		NNSG3dResMdl* pm = NNS_G3dGetMdlByIdx(ms, 0);

		NNS_G3dMdlSetMdlAmbAll(pm, GX_RGB(16,16,16));
	}

	//カメラセット
	{
		GFL_G3D_PROJECTION initProjection = 
			{ GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, cameraNear, cameraFar, 0 };
		initProjection.param1 = FX_SinIdx( cameraPerspway ); 
		initProjection.param2 = FX_CosIdx( cameraPerspway ); 
		GFL_G3D_SetSystemProjection( &initProjection );	
		GFL_G3D_SetSystemLookAt( &cameraLookAt );	
	}
	//ライトセット
  CG3d->g3Dlightset = GFL_G3D_LIGHT_Create( &light0Setup, heapID );
  GFL_G3D_LIGHT_Switching(CG3d->g3Dlightset);

	GFL_BG_SetBGControl3D(BGPRI_3D);
#if 0
	debugVec = drawStatus[1].trans;
	debugScale = drawStatus[1].scale.x;
#endif
}

static void mainG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID)
{
	GFL_G3D_OBJ*	g3Dobj;
	u16						objIdx;

	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );

	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	{
		int i;

		for(i=0; i<NELEMS(g3Dutil_objTbl); i++){
			g3Dobj = GFL_G3D_UTIL_GetObjHandle( CG3d->g3Dutil, objIdx + i );
			GFL_G3D_DRAW_DrawObject( g3Dobj, &drawStatus[i]);
		}
	}
	GFL_G3D_DRAW_End();

	{
		g3Dobj = GFL_G3D_UTIL_GetObjHandle( CG3d->g3Dutil, objIdx + 1 );
		GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 0, FX32_ONE/2 );
	}
#if 0
	if(GFL_UI_KEY_GetCont() & PAD_KEY_LEFT){ debugVec.x -= FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT){ debugVec.x += FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_KEY_UP){ debugVec.z -= FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_KEY_DOWN){ debugVec.z += FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_Y){ debugVec.y -= FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_X){ debugVec.y += FX32_ONE/8; }

	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_L){ debugScale -= FX32_ONE/8; }
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){ debugScale += FX32_ONE/8; }
#endif
}

static void releaseG3Dcontrol(G3D_CONTROL* CG3d)
{
	GFL_G3D_LIGHT_Delete( CG3d->g3Dlightset );
	GFL_G3D_UTIL_DelUnit( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );
	GFL_G3D_UTIL_Delete( CG3d->g3Dutil );
}

//==============================================================================
//	OAMControl
//==============================================================================
///アクター最大数
#define ACT_MAX			(64)

//--------------------------------------------------------------
static void setupOAMcontrol(OAM_CONTROL* COam, HEAPID heapID)
{
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.tr_cell = ACT_MAX;
	GFL_CLACT_SYS_Create(&clsys_init, &vramBank, heapID);
	
	COam->clunit = GFL_CLACT_UNIT_Create(ACT_MAX, 0, heapID);
	GFL_CLACT_UNIT_SetDefaultRend(COam->clunit);

	//OBJ表示ON
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
}

static void mainOAMcontrol(OAM_CONTROL* COam, HEAPID heapID)
{
	GFL_CLACT_SYS_Main();
}

static void releaseOAMcontrol(OAM_CONTROL* COam)
{
	GFL_CLACT_UNIT_Delete(COam->clunit);
	GFL_CLACT_SYS_Delete();
}


