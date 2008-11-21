//=============================================================================================
/**
 * @file	btlv_scu.c
 * @brief	ポケモンWB バトル 描画 上画面制御モジュール
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================
#include <gflib.h>

#include "print/printsys.h"

#include "btl_common.h"
#include "btl_util.h"
#include "btl_string.h"
#include "btlv_common.h"
#include "btlv_core.h"
#include "btlv_scu.h"

#include "arc_def.h"
#include "test_graphic\d_taya.naix"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	STRBUF_LEN = 512,
};


typedef struct {
	GFL_BMPWIN*				win;
	GFL_BMP_DATA*			bmp;
	const BTL_POKEPARAM*	bpp;
	BTLV_SCU*				parentWk;
	u8						clientID;
}STATUS_WIN;

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCU {

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;

	PRINT_QUE*			printQue;
	PRINT_UTIL			printUtil;
	GFL_FONT*			defaultFont;
	GFL_TCBLSYS*		tcbl;
	PRINT_STREAM*		printStream;
	STRBUF*				strBuf;

	STATUS_WIN			statusWin[ BTL_CLIENT_MAX ];

	BTL_PROC			proc;
	const BTLV_CORE*	vcore;
	const BTL_MAIN_MODULE*	mainModule;
	HEAPID				heapID;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BOOL btlin_loop( int* seq, void* wk_adrs );
static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, u8 clientID );
static void statwin_disp( STATUS_WIN* stwin );
static void statwin_hide( STATUS_WIN* stwin );
static void statwin_update( STATUS_WIN* stwin );



BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore, const BTL_MAIN_MODULE* mainModule, GFL_TCBLSYS* tcbl, GFL_FONT* defaultFont, HEAPID heapID )
{
	BTLV_SCU* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCU) );

	wk->vcore = vcore;
	wk->mainModule = mainModule;
	wk->heapID = heapID;

	wk->defaultFont = defaultFont;
	wk->printStream = NULL;
	wk->tcbl = tcbl;
	wk->strBuf = GFL_STR_CreateBuffer( STRBUF_LEN, heapID );

	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
	PRINT_UTIL_Setup( &wk->printUtil, wk->win );

//	GFL_ASSERT_SetLCDMode();

	return wk;
}

void BTLV_SCU_Delete( BTLV_SCU* wk )
{
	PRINTSYS_QUE_Delete( wk->printQue );
	GFL_STR_DeleteBuffer( wk->strBuf );
	GFL_HEAP_FreeMemory( wk );
}


void BTLV_SCU_Setup( BTLV_SCU* wk )
{
	// 個別フレーム設定
	static const GFL_BG_BGCNT_HEADER bgcntText = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};
	// 個別フレーム設定
	static const GFL_BG_BGCNT_HEADER bgcntStat = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bgcntText,   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcntStat,   GFL_BG_MODE_TEXT );

	GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
	GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0x22, 9, 1 );
	GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	GFL_BG_FillCharacter( GFL_BG_FRAME2_M, 0xff, 1, 0 );
	GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 19, 30, 4, 0, GFL_BMP_CHRAREA_GET_F );
	wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
	GFL_BMPWIN_MakeScreen( wk->win );
	GFL_BMPWIN_MakeFrameScreen( wk->win, 1, 0 );
	GFL_BMP_Clear( wk->bmp, 0x0f );
	GFL_BMPWIN_TransVramCharacter( wk->win );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );

	GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON  );
	GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_ON  );
	GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );
}


void BTLV_SCU_StartBtlIn( BTLV_SCU* wk )
{
	BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_loop );
}

BOOL BTLV_SCU_WaitBtlIn( BTLV_SCU* wk )
{
	return BTL_UTIL_CallProc( &wk->proc );
}

static BOOL btlin_loop( int* seq, void* wk_adrs )
{
	BTLV_SCU* wk = wk_adrs;

	switch( *seq ){
	case 0:
		BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount );
		BTLV_SCU_StartMsg( wk, wk->strBuf );
		(*seq)++;
		break;
	case 1:
		if( BTLV_SCU_WaitMsg(wk) )
		{
			u8 plClientID, enClientID;

			plClientID = BTLV_CORE_GetPlayerClientID( wk->vcore );
			enClientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, plClientID, 0 );
			statwin_setup( &wk->statusWin[ plClientID ], wk, plClientID );
			statwin_setup( &wk->statusWin[ enClientID ], wk, enClientID );

			statwin_disp( &wk->statusWin[ enClientID ] );
			(*seq)++;
		}
		break;
	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle );
			BTLV_SCU_StartMsg( wk, wk->strBuf );
			(*seq)++;
		}
		break;
	case 3:
		if( BTLV_SCU_WaitMsg(wk) )
		{
			u8 plClientID = BTLV_CORE_GetPlayerClientID( wk->vcore );
			statwin_disp( &wk->statusWin[ plClientID ] );
			(*seq)++;
		}
		break;
	case 4:
		return TRUE;
	}

	return FALSE;
}



void BTLV_SCU_StartMsg( BTLV_SCU* wk, const STRBUF* str )
{
	GF_ASSERT( wk->printStream == NULL );

	GFL_BMP_Clear( wk->bmp, 0x0f );

	wk->printStream = PRINTSYS_PrintStream(
				wk->win, 0, 0, str, wk->defaultFont, 0, wk->tcbl, BTLV_TASKPRI_MAIN_WINDOW,
				wk->heapID, 0x0f
	);
}

BOOL BTLV_SCU_WaitMsg( BTLV_SCU* wk )
{
	if( wk->printStream )
	{
		if( PRINTSYS_PrintStreamGetState( wk->printStream ) == PRINTSTREAM_STATE_DONE )
		{
			PRINTSYS_PrintStreamDelete( wk->printStream );
			wk->printStream = NULL;
		}
		return FALSE;
	}
	return TRUE;
}



//----------------------------

static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, u8 clientID )
{
	static const struct {
		u8 x;
		u8 y;
	} winpos[2] = {
		{ 20,  2 },
		{  2, 13 },
	};

	u8 isPlayer, playerClientID, px, py;

	stwin->clientID = clientID;
	stwin->parentWk = wk;
	stwin->bpp = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, clientID );

	playerClientID = BTLV_CORE_GetPlayerClientID( wk->vcore );
	isPlayer = !BTL_MAIN_IsOpponentClientID( wk->mainModule, playerClientID, clientID );
	px = winpos[isPlayer].x;
	py = winpos[isPlayer].y;

	stwin->win = GFL_BMPWIN_Create( GFL_BG_FRAME2_M, px, py, 10, 4, 0, GFL_BMP_CHRAREA_GET_F );
	stwin->bmp = GFL_BMPWIN_GetBmp( stwin->win );

	GFL_BMP_Clear( stwin->bmp, 3+stwin->clientID*2 );
	GFL_STR_MakeStatusWinStr( wk->strBuf, stwin->bpp );
	PRINTSYS_Print( stwin->bmp, 0, 0, wk->strBuf, wk->defaultFont );
}

static void statwin_disp( STATUS_WIN* stwin )
{
	GFL_BMPWIN_MakeScreen( stwin->win );
	GFL_BMPWIN_TransVramCharacter( stwin->win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
}

static void statwin_hide( STATUS_WIN* stwin )
{
	GFL_BMPWIN_ClearScreen( stwin->win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
}

static void statwin_update( STATUS_WIN* stwin )
{
	BTLV_SCU* wk = stwin->parentWk;

	GFL_BMP_Clear( stwin->bmp, 3+stwin->clientID*2 );
	GFL_STR_MakeStatusWinStr( wk->strBuf, stwin->bpp );
	PRINTSYS_Print( stwin->bmp, 0, 0, wk->strBuf, wk->defaultFont );
	GFL_BMPWIN_TransVramCharacter( stwin->win );
}

