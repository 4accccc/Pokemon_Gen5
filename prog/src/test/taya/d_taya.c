//============================================================================================
/**
 * @file	d_taya.c
 * @brief	デバッグ用関数
 * @author	taya
 * @date	2008.08.01
 */
//============================================================================================

// lib includes -----------------------
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <msgdata.h>

// global includes --------------------
#include "system\main.h"

// local includes ---------------------
#include "gf_font.h"
#include "printsys.h"
#include "msg_d_taya.h"

// archive includes -------------------
#include "arc_def.h"
#include "d_taya.naix"
#include "message.naix"


typedef BOOL (*pSubProc)( GFL_PROC*, int*, void*, void* );

typedef struct {

	u16		seq;
	HEAPID	heapID;
	u16		strNum;
	u16		kanjiMode;
	u32		yofs;

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;
	GFL_MSGDATA*		mm;
	STRBUF*				strbuf;
	GFL_TCBLSYS*		tcbl;

	ARCHANDLE*				arcHandle;
	GFL_FONT*				fontHandle;
	PRINT_STREAM_HANDLE		printStream;

	pSubProc		subProc;
	int				subSeq;

//	PRINT_STREAM_HANDLE	psHandle;

}KANJI_WORK;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT DebugTayaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DebugTayaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static BOOL SUBPROC_PrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT DebugTayaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );




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

#if 0
	static const char* arcFiles[] = {
		"test_graphic/d_taya.narc",
	};
	enum {
		ARCID_D_TAYA,
	};
#endif


	KANJI_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TAYA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(KANJI_WORK), HEAPID_TAYA_DEBUG );
	wk->heapID = HEAPID_TAYA_DEBUG;

//	GFL_ARC_Init( arcFiles, NELEMS(arcFiles) );	gfl_use.cで1回だけ初期化に変更

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

		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0xff, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
	wk->bmp = GFL_BMPWIN_GetBmp(wk->win);
	GFL_BMP_Clear( wk->bmp, 0xff );
	GFL_BMPWIN_MakeScreen( wk->win );

	wk->subProc = NULL;

	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

	wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_taya_dat, wk->heapID );
	wk->strbuf = GFL_STR_CreateBuffer( 1024, wk->heapID );
	wk->seq = 0;

	wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

//	GFL_FONT* GFL_FONT_CreateHandle( ARCHANDLE* arcHandle, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL fixedFontFlag, u32 heapID )
	wk->arcHandle = GFL_ARC_OpenDataHandle( ARCID_D_TAYA, wk->heapID );
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
	KANJI_WORK* wk = mywk;

	switch( *seq ){
	case 0:
		{
			u16 key = GFL_UI_KEY_GetTrg();

			if( key & PAD_BUTTON_A )
			{
				wk->subProc = SUBPROC_PrintTest;
			}

			if( wk->subProc != NULL )
			{
				wk->subSeq = 0;
				(*seq)++;
			}
		}
		break;

	case 1:
		if( wk->subProc( proc, &(wk->subSeq), pwk, mywk ) )
		{
			wk->subProc = NULL;
			*seq = 0;
		}
		break;
	}

	return GFL_PROC_RES_CONTINUE;
}
//------------------------------------------------------
// スタンドアロン状態での漢字PrintTest
//------------------------------------------------------
static BOOL SUBPROC_PrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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

	switch( *seq ){
	case 0:
		GFL_MSG_GetString( wk->mm, DEBUG_TAYA_STR00, wk->strbuf );
		GFL_BMP_Clear( wk->bmp, 0xff );
		PRINTSYS_Print( wk->bmp, 0, 0, wk->strbuf, wk->fontHandle );
		GFL_BMPWIN_TransVramCharacter( wk->win );
		(*seq)++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			u64 t1, t2;
			GFL_MSG_GetString( wk->mm, DEBUG_TAYA_STR10, wk->strbuf );
			GFL_BMP_Clear( wk->bmp, 0xff );
			t1 = OS_GetTick();
			PRINTSYS_Print( wk->bmp, 0, 0, wk->strbuf, 0 );
			t2 = OS_GetTick();
			OS_TPrintf("[TICK] AllScreenWrite = %ld\n", t2-t1);
			GFL_BMPWIN_TransVramCharacter( wk->win );
			(*seq) = 10;
			break;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			wk->strNum = 0;
			wk->yofs = 30;
			(*seq)++;
			break;
		}
		break;

	case 2:
		GFL_MSG_GetString( wk->mm, strID[wk->strNum], wk->strbuf );
		wk->printStream = PRINTSYS_PrintStream( wk->win, 0, wk->yofs,
						wk->strbuf, wk->fontHandle, 0, wk->tcbl, 0, wk->heapID, 0xff, NULL );
		(*seq)++;
		break;

	case 3:
		if( PRINTSYS_PrintStreamGetState(wk->printStream) == PRINTSTREAM_STATE_DONE )
		{
			PRINTSYS_PrintStreamDelete( wk->printStream );
			(*seq)++;
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
				(*seq) = 2;
			}
			else
			{
				wk->kanjiMode = !(wk->kanjiMode);
				GFL_MSG_SetLangID( wk->kanjiMode );
				(*seq) = 0;
			}
		}
		break;

	case 10:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			(*seq) = 0;
		}
		break;

	}

	return FALSE;
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




