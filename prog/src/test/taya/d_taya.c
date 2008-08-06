//============================================================================================
/**
 * @file	d_taya.c
 * @brief	デバッグ用関数
 * @author	taya
 * @date	2008.08.01
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "tcbl.h"
#include "system\main.h"

#include "gf_font.h"
#include "printsys.h"

#include "d_taya.naix"
#include "msg_d_taya.h"


typedef struct {

	u16		seq;
	HEAPID	heapID;
	u16		strNum;
	u16		kanjiMode;
	u32		yofs;

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;
	MSGDATA_MANAGER*	mm;
	STRBUF*				strbuf;
	GFL_TCBLSYS*		tcbl;

	ARCHANDLE*				arcHandle;
	GFL_FONT*				fontHandle;
	PRINT_STREAM_HANDLE		printStream;

//	PRINT_STREAM_HANDLE	psHandle;

}KANJI_WORK;



//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_BG_DISPVRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E			// テクスチャパレットスロット
	};

	static const char* arcFiles[] = {
		"test_graphic/d_taya.narc",
	};
	enum {
		ARCID_DEFAULT,
	};


	KANJI_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TAYA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(KANJI_WORK), HEAPID_TAYA_DEBUG );
	wk->heapID = HEAPID_TAYA_DEBUG;

	GFL_ARC_Init( arcFiles, NELEMS(arcFiles) );

	GFL_DISP_SetBank( &vramBank );

	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	GFL_FONTSYS_Init();

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );
		GFL_ARC_UTIL_TransVramPalette( ARCID_DEFAULT, NARC_d_taya_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
	wk->bmp = GFL_BMPWIN_GetBmp(wk->win);
	GFL_BMP_Clear( wk->bmp, 0xff );
	GFL_BMPWIN_MakeScreen( wk->win );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

	wk->mm = GFL_MSG_MANAGER_Create( MSGMAN_TYPE_NORMAL, ARCID_DEFAULT, NARC_d_taya_d_taya_dat, wk->heapID );
	wk->strbuf = GFL_STR_CreateBuffer( 1024, wk->heapID );
	wk->seq = 0;

	wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

//	GFL_FONT* GFL_FONT_CreateHandle( ARCHANDLE* arcHandle, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL fixedFontFlag, u32 heapID )
	wk->arcHandle = GFL_ARC_OpenDataHandle( ARCID_DEFAULT, wk->heapID );
	wk->fontHandle = GFL_FONT_CreateHandle( wk->arcHandle, NARC_d_taya_lc12_2bit_nftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	PRINTSYS_Init( wk->heapID );

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const u16 strID[] = {
		DEBUG_TAYA_STR01,
		DEBUG_TAYA_STR02,
		DEBUG_TAYA_STR03,
		DEBUG_TAYA_STR04,
		DEBUG_TAYA_STR05,
		DEBUG_TAYA_STR06,
		DEBUG_TAYA_STR07,
		DEBUG_TAYA_STR08,
		DEBUG_TAYA_STR09,
	};

	KANJI_WORK* wk = mywk;

	GFL_TCBL_Main( wk->tcbl );

	GF_ASSERT(wk);
	GF_ASSERT(wk->bmp);


	switch( wk->seq ){
	case 0:
		GFL_MSG_MANAGER_GetString( wk->mm, DEBUG_TAYA_STR00, wk->strbuf );
		GFL_BMP_Clear( wk->bmp, 0xff );
		PRINTSYS_Print( wk->bmp, 0, 0, wk->strbuf, wk->fontHandle );
		GFL_BMPWIN_TransVramCharacter( wk->win );
		wk->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			u64 t1, t2;
			GFL_MSG_MANAGER_GetString( wk->mm, DEBUG_TAYA_STR10, wk->strbuf );
			GFL_BMP_Clear( wk->bmp, 0xff );
			t1 = OS_GetTick();
			PRINTSYS_Print( wk->bmp, 0, 0, wk->strbuf, 0 );
			t2 = OS_GetTick();
			OS_TPrintf("[TICK] AllScreenWrite = %ld\n", t2-t1);
			GFL_BMPWIN_TransVramCharacter( wk->win );
			wk->seq = 10;
			break;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			wk->strNum = 0;
			wk->yofs = 30;
			wk->seq++;
			break;
		}
		break;

	case 2:
		GFL_MSG_MANAGER_GetString( wk->mm, strID[wk->strNum], wk->strbuf );
		wk->printStream = PRINTSYS_PrintStream( wk->win, 0, wk->yofs,
						wk->strbuf, wk->fontHandle, 0, wk->tcbl, 0, wk->heapID, 0xff, NULL );
		wk->seq++;
		break;

	case 3:
		if( PRINTSYS_PrintStreamGetState(wk->printStream) == PRINTSTREAM_STATE_DONE )
		{
			PRINTSYS_PrintStreamDelete( wk->printStream );
			wk->seq++;
		}
		break;

	case 4:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			if( ++(wk->strNum) < NELEMS(strID) )
			{
				wk->yofs += 16;
				if( strID[wk->strNum] == DEBUG_TAYA_STR09 )
				{
					wk->yofs += 16;
				}
				wk->seq = 2;
			}
			else
			{
				wk->kanjiMode = !(wk->kanjiMode);
				GFL_MSG_SetKindID( wk->kanjiMode );
				wk->seq = 0;
			}
		}
		break;

	case 10:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			wk->seq = 0;
		}
		break;

	}

	return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugTayaMainProcData = {
	DebugTayaMainProcInit,
	DebugTayaMainProcMain,
	DebugTayaMainProcEnd,
};

 


