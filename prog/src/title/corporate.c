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
#include "system\gfl_use.h"
#include "arc_def.h"

#include "corporate.h"
#include "corp.naix"


//==============================================================================
//	定数定義
//==============================================================================
///フレーム番号
enum{
	//メイン画面
	FRAME_CORP = GFL_BG_FRAME1_M,
	FRAME_LOGO = GFL_BG_FRAME1_S,
};

#define TIMER_WAIT			(90)
#define END_TIMER_WAIT	(30)		// 終了待ち

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct {
	u16			seq;
	HEAPID	heapID;
	
	int			timer;
}CORP_WORK;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT CorpProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT CorpProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT CorpProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
#ifdef PM_DEBUG
static void SetEndMode( u32 * buf, u32 mode );
#endif

//==============================================================================
//	外部関数宣言
//==============================================================================
//extern const	GFL_PROC_DATA TitleProcData;
//FS_EXTERN_OVERLAY(title);

//==============================================================================
//	データ
//==============================================================================

///PROCデータ
const GFL_PROC_DATA CorpProcData = {
	CorpProcInit,
	CorpProcMain,
	CorpProcEnd,
};

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

static const GFL_BG_SYS_HEADER sysHeader = {
	GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
};
	
static const GFL_BG_BGCNT_HEADER corpM_frame = {
	//FRAME_CORP
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

static const GFL_BG_BGCNT_HEADER logoS_frame = {
	//FRAME_LOGO
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

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
static GFL_PROC_RESULT CorpProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	CORP_WORK* cw;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TITLE_DEMO, 0x70000 );
	cw = GFL_PROC_AllocWork( proc, sizeof(CORP_WORK), HEAPID_TITLE_DEMO );
	GFL_STD_MemClear(cw, sizeof(CORP_WORK));

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT CorpProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	CORP_WORK* cw = mywk;
	enum{
		SEQ_SETUP = 0,
		SEQ_FADEIN,
		SEQ_MAIN,
		SEQ_NEXT,
		SEQ_FADEOUT,
		SEQ_WAIT,
		SEQ_END,
	};
	
	switch(cw->seq){
	case SEQ_SETUP:
		// 各種効果レジスタ初期化
		GX_SetMasterBrightness(-16);
		GXS_SetMasterBrightness(-16);
		GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
		GFL_DISP_GXS_SetVisibleControlDirect(0);
		G2_BlendNone();
		G2S_BlendNone();
		GX_SetVisibleWnd(GX_WNDMASK_NONE);
		GXS_SetVisibleWnd(GX_WNDMASK_NONE);

		// VRAM設定
		GFL_DISP_SetBank( &vramBank );
		GFL_BG_Init( HEAPID_TITLE_DEMO );
		GFL_BG_SetBGMode( &sysHeader );

		// BG設定
		GFL_BG_SetBGControl( FRAME_CORP,   &corpM_frame,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( FRAME_LOGO,   &logoS_frame,   GFL_BG_MODE_TEXT );
		{
			// メイン画面
			GFL_ARC_UTIL_TransVramBgCharacter
				(ARCID_CORP, NARC_corp_credit_u_NCGR, FRAME_CORP, 0, 0x8000, 0, HEAPID_TITLE_DEMO);
			GFL_ARC_UTIL_TransVramScreen
				(ARCID_CORP, NARC_corp_credit_u_NSCR, FRAME_CORP, 0, 0, 0, HEAPID_TITLE_DEMO);
			GFL_ARC_UTIL_TransVramPalette
				(ARCID_CORP, NARC_corp_corp_NCLR, PALTYPE_MAIN_BG, 0, 0, HEAPID_TITLE_DEMO);

			// サブ画面
			GFL_ARC_UTIL_TransVramBgCharacter
				(ARCID_CORP, NARC_corp_logo_d_NCGR, FRAME_LOGO, 0, 0x8000, 0, HEAPID_TITLE_DEMO);
			GFL_ARC_UTIL_TransVramScreen
				(ARCID_CORP, NARC_corp_logo_d_NSCR, FRAME_LOGO, 0, 0, 0, HEAPID_TITLE_DEMO);
			GFL_ARC_UTIL_TransVramPalette
				(ARCID_CORP, NARC_corp_corp_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_TITLE_DEMO);

			GFL_BG_LoadScreenReq( FRAME_CORP );
			GFL_BG_LoadScreenReq( FRAME_LOGO );
		}

		// 上下画面設定
		GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
		GFL_DISP_SetDispOn();

		GFL_BG_SetVisible(FRAME_CORP, VISIBLE_ON);
		GFL_BG_SetVisible(FRAME_LOGO, VISIBLE_ON);
		{
			int mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
			GFL_FADE_SetMasterBrightReq(mode, 16, 0, 2);
		}
		cw->seq = SEQ_FADEIN;
		break;

	case SEQ_FADEIN:
		if(GFL_FADE_CheckFade() == FALSE){
			cw->seq = SEQ_MAIN;
		}
#ifdef PM_DEBUG
		if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_SELECT)){
			int mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
			GFL_FADE_SetMasterBrightReq(mode, 0, 0, 0 );
			SetEndMode( pwk, CORPORATE_RET_DEBUG );
			cw->seq = SEQ_END;
			break;
		}
#endif
		break;

	case SEQ_MAIN:
		cw->timer++;

#ifdef PM_DEBUG
		if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_START)){
			SetEndMode( pwk, CORPORATE_RET_SKIP );
			cw->seq = SEQ_END;
			break;
		}
		if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_SELECT)){
			SetEndMode( pwk, CORPORATE_RET_DEBUG );
			cw->seq = SEQ_END;
			break;
		}
#endif

		if(cw->timer > TIMER_WAIT){
#ifdef PM_DEBUG
			SetEndMode( pwk, CORPORATE_RET_NORMAL );
#endif
			cw->seq = SEQ_NEXT;
			cw->timer = 0;
			break;
		}
		break;

	case SEQ_NEXT:
		{
			int mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
			GFL_FADE_SetMasterBrightReq(mode, 0, 16, 2);
			cw->seq = SEQ_FADEOUT;
		}
		break;

	case SEQ_FADEOUT:
		if(GFL_FADE_CheckFade() == FALSE){
			cw->seq = SEQ_WAIT;
		}
#ifdef PM_DEBUG
		if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_SELECT)){
			int mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
			GFL_FADE_SetMasterBrightReq(mode, 16, 16, 0);
			SetEndMode( pwk, CORPORATE_RET_DEBUG );
			cw->seq = SEQ_END;
			break;
		}
#endif
		break;

	case SEQ_WAIT:
		cw->timer++;
		if(cw->timer > END_TIMER_WAIT){
			cw->seq = SEQ_END;
		}
		break;

	case SEQ_END:
		GFL_BG_Exit();
		return GFL_PROC_RES_FINISH;

	}
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT CorpProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	CORP_WORK* cw = mywk;
	
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_TITLE_DEMO);
	
//	GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
	return GFL_PROC_RES_FINISH;
}


#ifdef PM_DEBUG
//--------------------------------------------------------------------------
/**
 * 終了モード設定
 */
//--------------------------------------------------------------------------
static void SetEndMode( u32 * buf, u32 mode )
{
	*buf = mode;
}
#endif
