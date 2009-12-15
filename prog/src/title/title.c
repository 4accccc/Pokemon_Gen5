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
#include "title/title.h"
#include "title/startmenu.h"
#include "system\gfl_use.h"



//==============================================================================
//	定数定義
//==============================================================================
///アクター最大数
#define ACT_MAX			(64)

///フォントが使用するパレット番号
#define D_FONT_PALNO	(14)

///アイテムアイコンのパレット展開位置
#define D_MATSU_ICON_PALNO		(0)

///[PUSH START BUTTON]の点滅間隔
#define PUSH_TIMER_WAIT			(45)

///BMPウィンドウ
enum{
	WIN_PUSH_JPN,			//「スタートボタンを押してください」
	WIN_PUSH_ENG,			//「PUSH START」
	
	WIN_MAX,
};

///タイトルロゴBGのBGプライオリティ
enum{
	BGPRI_MSG = 0,
	BGPRI_MIST = 0,
	BGPRI_TITLE_LOGO = 2,
	BGPRI_3D = 1,
};

///フレーム番号
enum{
	//メイン画面
	FRAME_LOGO_S = GFL_BG_FRAME3_M,		///<サブ画面タイトルロゴのBGフレーム
	FRAME_MIST_M = GFL_BG_FRAME1_M,		///<霧
	
	//サブ画面
	FRAME_MSG_S = GFL_BG_FRAME1_S,		///<メッセージフレーム
	FRAME_LOGO_M = GFL_BG_FRAME3_S,		///<タイトルロゴのBGフレーム
};

//--------------------------------------------------------------
//	3D
//--------------------------------------------------------------
#define DTCM_SIZE		(0x1000)

static const GFL_G3D_LOOKAT cameraLookAt = {
	{ -252.446045*FX32_ONE, 268.627930*FX32_ONE, 721.645996*FX32_ONE },	//カメラの位置(＝視点)
	{ 0, FX32_ONE, 0 },												//カメラの上方向
	{ 110.435803*FX32_ONE, 1395.428467*FX32_ONE, 279.511475*FX32_ONE },	//カメラの焦点(＝注視点)

};
#define cameraPerspway	( 30/2 * PERSPWAY_COEFFICIENT )
#define cameraAspect	( FX32_ONE * 4/3 )
#define cameraNear		( 32 << FX32_SHIFT )
#define cameraFar		( 2048 << FX32_SHIFT )

#define g3DanmRotateSpeed	( 0x100 )
#define g3DanmFrameSpeed	( FX32_ONE )

static const GFL_G3D_OBJSTATUS drawStatus[] = {
	{
		{ 0, 0, 0 },																				//座標
		{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
	},
	{
		{ 0xfffebef9, 0x00378800, 0x0019a82f },	//座標
		{ 0x3000, 0x3000, 0x3000 },											//スケール
		{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },					//回転
	},
};

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	int wait;
	int master_bright;
	int mode;			///<次のメニュー画面へ引き渡すモード
	
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	PRINT_STREAM	*printStream;
	GFL_MSGDATA		*mm;
	STRBUF			*strbuf_push_jpn;
	STRBUF			*strbuf_push_eng;
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin[WIN_MAX];
	
	int push_timer;				//[PUSH START BUTTON]の点滅間隔をカウント
	int push_visible;
	
	//アクター
	GFL_CLUNIT *clunit;
	GFL_CLWK	*clwk_icon;		//アイテムアイコンアクター
	NNSG2dImagePaletteProxy	icon_pal_proxy;
	NNSG2dCellDataBank	*icon_cell_data;
	NNSG2dAnimBankData	*icon_anm_data;
	void	*icon_cell_res;
	void	*icon_anm_res;
	
	GFL_G3D_UTIL*			g3Dutil;
	u16								g3DutilUnitIdx;
	GFL_TCB						*vintr_tcb;
}TITLE_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
GFL_PROC_RESULT TitleProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT TitleProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT TitleProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void Local_VramSetting(TITLE_WORK *tw);
static void Local_BGFrameSetting(TITLE_WORK *tw);
static void Local_MessageSetting(TITLE_WORK *tw);
static void Local_ActorSetting(TITLE_WORK *tw);
static void Local_MessagePrintMain(TITLE_WORK *tw);
static void Local_BGGraphicLoad(TITLE_WORK *tw);
static void Local_MessagePut(TITLE_WORK *tw, int win_index, STRBUF *strbuf, int x, int y);
static void Local_MsgLoadPushStart(TITLE_WORK *tw);
static void Local_3DSetting(TITLE_WORK *tw);
static void Local_Load3Dobject(TITLE_WORK *tw);
static void Local_Draw3Dobject(TITLE_WORK *tw);
static void Local_Free3Dobject(TITLE_WORK *tw);
static void VintrTCB_VblankFunc(GFL_TCB *tcb, void *work);

//==============================================================================
//	外部関数宣言
//==============================================================================
extern void	TestModeSet(int mode);
extern const	GFL_PROC_DATA TestMainProcData;
FS_EXTERN_OVERLAY(testmode);


//==============================================================================
//	データ
//==============================================================================
///タイトル画面呼び出しようのPROCデータ
const GFL_PROC_DATA TitleProcData = {
	TitleProcInit,
	TitleProcMain,
	TitleProcEnd,
};

//--------------------------------------------------------------
//	3D
//--------------------------------------------------------------
#include "title1.naix"

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

//==============================================================================
//
//	
//
//==============================================================================
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
	
	DEBUG_PerformanceSetActive(TRUE);	//デバッグ：パフォーマンスメーター有効
	
	// 各種効果レジスタ初期化
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	G2_BlendNone();
	G2S_BlendNone();
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TITLE_DEMO, 0x70000 );
	tw = GFL_PROC_AllocWork( proc, sizeof(TITLE_WORK), HEAPID_TITLE_DEMO );
	GFL_STD_MemClear(tw, sizeof(TITLE_WORK));

	// 上下画面設定
	//GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();

	//TCB作成
	tw->tcbl = GFL_TCBL_Init( HEAPID_TITLE_DEMO, HEAPID_TITLE_DEMO, 4, 32 );

	//VRAM設定 & BGフレーム設定
	Local_VramSetting(tw);
	Local_BGFrameSetting(tw);
	Local_BGGraphicLoad(tw);
	
	//メッセージ関連作成
	GFL_BMPWIN_Init( HEAPID_TITLE_DEMO );
	GFL_FONTSYS_Init();
	Local_MessageSetting(tw);
	Local_MsgLoadPushStart(tw);

	//アクター設定
	Local_ActorSetting(tw);

	//3D設定
	Local_3DSetting(tw);
	Local_Load3Dobject(tw);
	
	GFL_BG_SetVisible(FRAME_LOGO_M, VISIBLE_ON);
//	GFL_BG_SetVisible(FRAME_MIST_M, VISIBLE_ON);
//	GFL_BG_SetVisible(FRAME_LOGO_S, VISIBLE_ON);
//	GFL_BG_SetVisible(FRAME_MSG_S, VISIBLE_ON);

	tw->vintr_tcb = GFUser_VIntr_CreateTCB(VintrTCB_VblankFunc, tw, 5);

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
	enum{
		SEQ_WAIT,
		SEQ_FADEIN,
		SEQ_MAIN,
		SEQ_FADEOUT,
	};
	
	GFL_TCBL_Main( tw->tcbl );
	Local_MessagePrintMain(tw);
	
	if(tw->seq == SEQ_MAIN){
		int trg = GFL_UI_KEY_GetTrg();
		if(trg & (PAD_BUTTON_START | PAD_BUTTON_SELECT | PAD_BUTTON_A)){
			tw->mode = trg;
			tw->seq = SEQ_FADEOUT;
		}
		if( GFL_UI_TP_GetTrg() == TRUE )
		{
			tw->mode = trg;
			tw->seq = SEQ_FADEOUT;
    }
	}
	
	switch(tw->seq){
	case SEQ_WAIT:
		tw->wait++;
		if(tw->wait > 30){
			tw->master_bright = -16;
			tw->seq++;
		}
		break;
	case SEQ_FADEIN:
		tw->master_bright++;
		GX_SetMasterBrightness(tw->master_bright);
		GXS_SetMasterBrightness(tw->master_bright);
		if(tw->master_bright >= 0){
			tw->seq++;
		}
		break;
	case SEQ_MAIN:
		tw->push_timer++;
		if(tw->push_timer > PUSH_TIMER_WAIT){
			tw->push_timer = 0;
			tw->push_visible ^= 1;
			GFL_BG_SetVisible(FRAME_MSG_S, tw->push_visible);
		}
		break;
	case SEQ_FADEOUT:
		tw->master_bright--;
		GX_SetMasterBrightness(tw->master_bright);
		GXS_SetMasterBrightness(tw->master_bright);
		if(tw->master_bright <= -16){
			return GFL_PROC_RES_FINISH;
		}
		break;
	}
	
	Local_Draw3Dobject(tw);
	
	GFL_CLACT_SYS_Main();
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
	
	GFL_TCB_DeleteTask(tw->vintr_tcb);
	
	GFL_STR_DeleteBuffer(tw->strbuf_push_jpn);
	GFL_STR_DeleteBuffer(tw->strbuf_push_eng);
	for(i = 0; i < WIN_MAX; i++){
		GFL_BMPWIN_Delete(tw->drawwin[i].win);
	}

	Local_Free3Dobject(tw);
	GFL_G3D_Exit();

	PRINTSYS_QUE_Delete(tw->printQue);
	GFL_MSG_Delete(tw->mm);

	GFL_CLACT_UNIT_Delete(tw->clunit);
	GFL_CLACT_SYS_Delete();

	GFL_FONT_Delete(tw->fontHandle);
	GFL_TCBL_Exit(tw->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_TITLE_DEMO);
	
//	TestModeSet(mode);	//次のPROCとしてテスト画面を設定
	//StartMenuへの分岐を作成しました Ari090107
	if( mode & PAD_BUTTON_SELECT )
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(testmode), &TestMainProcData, NULL );
	else
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &StartMenuProcData, NULL);
	return GFL_PROC_RES_FINISH;
}

static void VintrTCB_VblankFunc(GFL_TCB *tcb, void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
}

//==============================================================================
//	
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

//--------------------------------------------------------------
/**
 * @brief   VRAMバンク＆モード設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_VramSetting(TITLE_WORK *tw)
{

	static const GFL_BG_SYS_HEADER sysHeader = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
	};
	
	GFL_DISP_SetBank( &vramBank );
	GFL_BG_Init( HEAPID_TITLE_DEMO );
	GFL_BG_SetBGMode( &sysHeader );
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_BGFrameSetting(TITLE_WORK *tw)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {	//メイン画面BGフレーム
		{//FRAME_LOGO_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
		},
		{//FRAME_MIST_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x4000,
			GX_BG_EXTPLTT_01, BGPRI_MIST, 0, 0, FALSE
		},
	};

	static const GFL_BG_BGCNT_HEADER sub_bgcnt_frame[] = {	//サブ画面BGフレーム
		{//FRAME_MSG_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x04000,
			GX_BG_EXTPLTT_01, BGPRI_MSG, 0, 0, FALSE
		},
		{//FRAME_LOGO_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( FRAME_LOGO_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_MIST_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_MSG_S,   &sub_bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_LOGO_S,   &sub_bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillCharacter( FRAME_LOGO_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_MIST_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_MSG_S, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_LOGO_S, 0x00, 1, 0 );

	GFL_BG_FillScreen( FRAME_LOGO_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_MIST_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_MSG_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_LOGO_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	GFL_BG_LoadScreenReq( FRAME_LOGO_M );
	GFL_BG_LoadScreenReq( FRAME_MIST_M );
	GFL_BG_LoadScreenReq( FRAME_MSG_S );
	GFL_BG_LoadScreenReq( FRAME_LOGO_S );
}

//--------------------------------------------------------------
/**
 * @brief   BGグラフィックをVRAMへ転送
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_BGGraphicLoad(TITLE_WORK *tw)
{
	//メイン画面
	GFL_ARC_UTIL_TransVramBgCharacter(
		ARCID_TITLE, NARC_title_wb_logo_bk_NCGR, FRAME_LOGO_M, 0, 0x8000, 0, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramScreen(
		ARCID_TITLE, NARC_title_wb_logo_bk_NSCR, FRAME_LOGO_M, 0, 0, 0, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_TITLE, NARC_title_wb_logo_bk_NCLR, PALTYPE_MAIN_BG, 0, 0, HEAPID_TITLE_DEMO);

	GFL_ARC_UTIL_TransVramBgCharacter(
		ARCID_TITLE, NARC_title_mist_NCGR, FRAME_MIST_M, 0, 0x4000, 0, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramScreen(
		ARCID_TITLE, NARC_title_mist_NSCR, FRAME_MIST_M, 0, 0, 0, HEAPID_TITLE_DEMO);
//	GFL_ARC_UTIL_TransVramPalette(
//		ARCID_TITLE, NARC_title_mist_NCLR, PALTYPE_MAIN_BG, 0, 0, HEAPID_TITLE_DEMO);

	//サブ画面
	GFL_ARC_UTIL_TransVramBgCharacter(
		ARCID_TITLE, NARC_title_wb_logo_bk_NCGR, FRAME_LOGO_S, 0, 0x8000, 0, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramScreen(
		ARCID_TITLE, NARC_title_wb_logo_bk_NSCR, FRAME_LOGO_S, 0, 0, 0, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_TITLE, NARC_title_wb_logo_bk_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_TITLE_DEMO);

	GFL_BG_SetBackGroundColor(GFL_BG_FRAME0_M, GX_RGB(25, 31, 31));
}

//--------------------------------------------------------------
/**
 * @brief   3D初期設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_3DSetting(TITLE_WORK *tw)
{
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT32K,
						DTCM_SIZE, HEAPID_TITLE_DEMO, NULL );
	GFL_BG_SetBGControl3D(BGPRI_3D);
}

//--------------------------------------------------------------
/**
 * @brief   3Dobjectをロード
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_Load3Dobject(TITLE_WORK *tw)
{
	u16 objIdx;
	
	tw->g3Dutil = GFL_G3D_UTIL_Create(G3DUTIL_RESCOUNT, G3DUTIL_OBJCOUNT, HEAPID_TITLE_DEMO);
	tw->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( tw->g3Dutil, &g3Dutil_setup );

	//アニメーションを有効にする
	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( tw->g3Dutil, tw->g3DutilUnitIdx );
	GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(tw->g3Dutil, objIdx+1), 0); 
	OS_TPrintf("3d objIdx = %d\n", objIdx);

	//カメラセット
	{
		GFL_G3D_PROJECTION initProjection = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, 
												cameraNear, cameraFar, 0 };
		initProjection.param1 = FX_SinIdx( cameraPerspway ); 
		initProjection.param2 = FX_CosIdx( cameraPerspway ); 
		GFL_G3D_SetSystemProjection( &initProjection );	
		GFL_G3D_SetSystemLookAt( &cameraLookAt );	
	}
}

//--------------------------------------------------------------
/**
 * @brief   3Dobject描画
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_Draw3Dobject(TITLE_WORK *tw)
{
	GFL_G3D_OBJ*	g3Dobj;
	u16						objIdx;

	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( tw->g3Dutil, tw->g3DutilUnitIdx );

	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	{
		int i;

		for(i=0; i<NELEMS(g3Dutil_objTbl); i++){
			g3Dobj = GFL_G3D_UTIL_GetObjHandle( tw->g3Dutil, objIdx + i );
			GFL_G3D_DRAW_DrawObject( g3Dobj, &drawStatus[i]);
		}
	}
	GFL_G3D_DRAW_End();

	{
		g3Dobj = GFL_G3D_UTIL_GetObjHandle( tw->g3Dutil, objIdx + 1 );
		GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 0, FX32_ONE );
	}
}

//--------------------------------------------------------------
/**
 * @brief   3Dobjectをアンロード
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_Free3Dobject(TITLE_WORK *tw)
{
	GFL_G3D_UTIL_DelUnit( tw->g3Dutil, tw->g3DutilUnitIdx );
	GFL_G3D_UTIL_Delete( tw->g3Dutil );
}

//--------------------------------------------------------------
/**
 * @brief   アクター設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_ActorSetting(TITLE_WORK *tw)
{
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.tr_cell = ACT_MAX;
	GFL_CLACT_SYS_Create(&clsys_init, &vramBank, HEAPID_TITLE_DEMO);
	
	tw->clunit = GFL_CLACT_UNIT_Create(ACT_MAX, 0, HEAPID_TITLE_DEMO);
	GFL_CLACT_UNIT_SetDefaultRend(tw->clunit);

	//OBJ表示ON
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ関連設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_MessageSetting(TITLE_WORK *tw)
{
	int i;

	//BMPWIN作成
#if 0
	tw->drawwin[WIN_PUSH_JPN].win 
		= GFL_BMPWIN_Create(FRAME_MSG_S, 0, 10, 32, 2, D_FONT_PALNO, GFL_BMP_CHRAREA_GET_F);
	tw->drawwin[WIN_PUSH_ENG].win 
		= GFL_BMPWIN_Create(FRAME_MSG_S, 0, 12, 32, 2, D_FONT_PALNO, GFL_BMP_CHRAREA_GET_F);
#else
	tw->drawwin[WIN_PUSH_JPN].win 
		= GFL_BMPWIN_Create(FRAME_MSG_S, 0, 16, 32, 2, D_FONT_PALNO, GFL_BMP_CHRAREA_GET_F);
	tw->drawwin[WIN_PUSH_ENG].win 
		= GFL_BMPWIN_Create(FRAME_MSG_S, 0, 18, 32, 2, D_FONT_PALNO, GFL_BMP_CHRAREA_GET_F);
#endif
	for(i = 0; i < WIN_MAX; i++){
		tw->drawwin[i].bmp = GFL_BMPWIN_GetBmp(tw->drawwin[i].win);
		GFL_BMP_Clear( tw->drawwin[i].bmp, 0x00 );
		GFL_BMPWIN_MakeScreen( tw->drawwin[i].win );
		GFL_BMPWIN_TransVramCharacter( tw->drawwin[i].win );
		PRINT_UTIL_Setup( tw->drawwin[i].printUtil, tw->drawwin[i].win );
	}
	GFL_BG_LoadScreenReq( FRAME_MSG_S );

	//フォント作成
	tw->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_TITLE_DEMO );

	tw->printQue = PRINTSYS_QUE_Create( HEAPID_TITLE_DEMO );

	tw->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_title_dat, HEAPID_TITLE_DEMO );

	//STRBUF作成
	tw->strbuf_push_jpn = GFL_STR_CreateBuffer( 64, tw->heapID );
	tw->strbuf_push_eng = GFL_STR_CreateBuffer( 64, tw->heapID );

	//フォントパレット転送
#if 0
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0x20*D_FONT_PALNO, 0x20, HEAPID_TITLE_DEMO);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
		0x20*D_FONT_PALNO, 0x20, HEAPID_TITLE_DEMO);
#else	//白字にする
	//GFL_STD_MemFill16((void*)(HW_BG_PLTT + D_FONT_PALNO*0x20+2), 0x7fff, 0x20-2);
	//GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT + D_FONT_PALNO*0x20+2), 0x7fff, 0x20-2);
	//緑字にする
	GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT + D_FONT_PALNO*0x20+2), GX_RGB(0, 20, 0), 0x20-2);
	GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT + D_FONT_PALNO*0x20+6), GX_RGB(0, 31, 0), 2);
#endif
	
	GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ描画メイン
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_MessagePrintMain(TITLE_WORK *tw)
{
	int i;
	BOOL que_ret;
	
	que_ret = PRINTSYS_QUE_Main( tw->printQue );

	for(i = 0; i < WIN_MAX; i++){
		if( PRINT_UTIL_Trans( tw->drawwin[i].printUtil, tw->printQue ) == FALSE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(que_ret == TRUE && tw->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( tw->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( tw->drawwin[i].win );
				tw->drawwin[i].message_req = FALSE;
			}
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   tw			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標(dot)
 * @param   y			Y座標(dot)
 */
//--------------------------------------------------------------
static void Local_MessagePut(TITLE_WORK *tw, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( tw->drawwin[win_index].bmp, 0x00 );
	PRINTSYS_PrintQue(tw->printQue, tw->drawwin[win_index].bmp, x, y, strbuf, tw->fontHandle);
	tw->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   [PUSH START BUTTON]の文字を描画
 *
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_MsgLoadPushStart(TITLE_WORK *tw)
{
	STRBUF *strbuf;
	
	GFL_MSG_GetString(tw->mm, TITLE_STR_000, tw->strbuf_push_jpn);
	GFL_MSG_GetString(tw->mm, TITLE_STR_001, tw->strbuf_push_eng);
	Local_MessagePut(tw, WIN_PUSH_JPN, tw->strbuf_push_jpn, 6*8, 0);
	Local_MessagePut(tw, WIN_PUSH_ENG, tw->strbuf_push_eng, 10*8, 0);
}

