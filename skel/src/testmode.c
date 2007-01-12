//===================================================================
/**
 * @file	testmode.c
 */
//===================================================================
#include "gflib.h"
#include "main.h"

#include "ui.h"
#include "textprint.h"

typedef struct {
	int						seq;
	u16						listPosition;
	void*					chrbuf;
	GFL_BMP_DATA*			bmpHeader;
	GFL_TEXT_PRINTPARAM*	textParam;

	GFL_BG_INI*				bgl;
	GFL_BMPWIN*				bmpwin[8];

	u16						work[16];
}TESTMODE_WORK;

TESTMODE_WORK* testmode;

void	TestModeInit(void);
void	TestModeMain(void);

static BOOL	TestModeControl( void );

#define XYSIZE (32*32)	//スクリーンキャラクタサイズ
#define TEXT_FRM	(GFL_BG_FRAME3_M)

//------------------------------------------------------------------
/**
 * @brief		初期化
 */
//------------------------------------------------------------------
void	TestModeInit(void)
{
	testmode = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,sizeof(TESTMODE_WORK));
	GFL_TEXT_sysInit( "gfl_graphic/gfl_font.dat" );
}

//------------------------------------------------------------------
/**
 * @brief		メイン
 */
//------------------------------------------------------------------
void	TestModeMain(void)
{
	if( TestModeControl() == TRUE ){
		GFL_HEAP_FreeMemory( testmode );
	}
}


//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init( void )
{
	u16 heapID = GFL_HEAPID_APP;

	testmode->bgl = GFL_BG_sysInit( heapID );

	GX_SetBankForBG(GX_VRAM_BG_64_E);
	{
		GFL_BG_SYS_HEADER bgsysHeader = {
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};	
		GFL_BG_InitBG( &bgsysHeader );
	}
	{
		GFL_BG_BGCNT_HEADER bgCont = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_BGControlSet( testmode->bgl, TEXT_FRM, &bgCont, GFL_BG_MODE_TEXT );
		GFL_BG_PrioritySet( TEXT_FRM, 0 );
		GFL_BG_VisibleSet( TEXT_FRM, VISIBLE_ON );
	}
	{
		GFL_BMPWIN_SET bmpwinSet;
		int	i;

		for(i=0;i<GFL_BG_FRAME_MAX;i++){
			bmpwinSet.limit[i].start = 0;
			bmpwinSet.limit[i].end = 0;
		}
		bmpwinSet.limit[3].start = 0x4000;
		bmpwinSet.limit[3].end = 0x8000;
		GFL_BMPWIN_sysInit( testmode->bgl, &bmpwinSet, heapID );
	}
	{//パレット作成＆転送
		u16* plt = GFL_HEAP_AllocMemoryLowClear( heapID, 16*2 );

		plt[0] = 0x5041;	//青
		plt[1] = 0x7fff;	//白
		plt[2] = 0x001f;	//赤
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 0 );
		GFL_HEAP_FreeMemory( plt );
	}
	{//文字表示パラメータワーク作成
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocMemoryLowClear
										( heapID,sizeof(GFL_TEXT_PRINTPARAM));
		param->spacex = 1;
		param->spacey = 1;
		param->colorF = 1;
		param->colorB = 0;
		param->mode = GFL_TEXT_WRITE_16;
		testmode->textParam = param;
	}
	GFL_BG_LoadScreenReq( testmode->bgl, TEXT_FRM );
}

static void	bg_exit( void )
{
	GFL_BMPWIN_sysExit();
	GFL_BG_BGControlExit( testmode->bgl, TEXT_FRM );
	GFL_BG_sysExit( testmode->bgl );
}

static void bmpwin_msg_write( GFL_BMPWIN* bmpwin, const char* msg, u8 x, u8 y )
{
	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( bmpwin );
	testmode->textParam->writex = x;
	testmode->textParam->writey = y;
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );
}
	
//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
//------------------------------------------------------------------
static const char	test_index0[] = 
{"ゲームフリーク　プログラムライブラリ　テストモード\nGame Freak Libraries TestMode"};
static const char	test_index1[] = {"http://www.gamefreak.co.jp"};

static const char	test_select0[] = {"わたなべ　てつや\nTetsuya Watanabe"};
static const char	test_select1[] = {"たまだ　そうすけ\nSousuke Tamada"};
static const char	test_select2[] = {"そがべ　ひさし\nHisashi Sogabe"};
static const char	test_select3[] = {"おおの　かつみ\nKatsumi Ohno"};
static const char	test_select4[] = {"みつはら　さとし\nSatoshi Mitsuhara"};

typedef struct {
	const char*	name;
	u8 writex;
	u8 writey;
}TESTMODE_PRINTLIST;

static const TESTMODE_PRINTLIST indexList[] = {
	{ test_index0, 16, 8},
	{ test_index1, 16, 180},
};

static const TESTMODE_PRINTLIST selectList[] = {
	{ test_select0, 32, (0*25)+40},
	{ test_select1, 32, (1*25)+40},
	{ test_select2, 32, (2*25)+40},
	{ test_select3, 32, (3*25)+40},
	{ test_select4, 32, (4*25)+40},
};

static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( testmode->seq ){

	case 0:
		bg_init();
		testmode->listPosition = 0;

		testmode->bmpwin[0] = GFL_BMPWIN_Create( TEXT_FRM,  0,  8, 32, 2, 0, 0 );
		bmpwin_msg_write( testmode->bmpwin[0], test_index0, 16, 0 );
		GFL_BMPWIN_UploadChar( testmode->bmpwin[0] );
		GFL_BMPWIN_MakeScrn( testmode->bmpwin[0] );
		testmode->work[0] = 1;

		testmode->bmpwin[1] = GFL_BMPWIN_Create( TEXT_FRM,  0, 22, 32, 1, 0, 0 );
		bmpwin_msg_write( testmode->bmpwin[1], test_index1, 16, 0 );
		GFL_BMPWIN_UploadChar( testmode->bmpwin[1] );
		GFL_BMPWIN_MakeScrn( testmode->bmpwin[1] );
		testmode->work[1] = 1;

		GFL_BG_LoadScreenReq( testmode->bgl, TEXT_FRM );

		testmode->seq++;
		break;

	case 1:
		if( GFL_UI_KeyGetTrg() == PAD_BUTTON_A ){
			if( testmode->work[0] ){
				GFL_BMPWIN_ClearScrn( testmode->bmpwin[0] );
				testmode->work[0] = 0;
			} else {
				GFL_BMPWIN_MakeScrn( testmode->bmpwin[0] );
				testmode->work[0] = 1;
			}
			GFL_BG_LoadScreenReq( testmode->bgl, TEXT_FRM );
			break;
		}
		if( GFL_UI_KeyGetTrg() == PAD_BUTTON_B ){
			if( testmode->work[1] ){
				GFL_BMPWIN_ClearScrn( testmode->bmpwin[1] );
				testmode->work[1] = 0;
			} else {
				GFL_BMPWIN_MakeScrn( testmode->bmpwin[1] );
				testmode->work[1] = 1;
			}
			GFL_BG_LoadScreenReq( testmode->bgl, TEXT_FRM );
			break;
		}
#if 0
		for(i=0;i<NELEMS(indexList);i++){
			testmode->textParam->writex = indexList[i].writex;
			testmode->textParam->writey = indexList[i].writey;
			GFL_TEXT_PrintSjisCode( indexList[i].name, testmode->textParam );
		}
		testmode->seq++;
#endif
		break;

	case 2:
#if 0
		for(i=0;i<NELEMS(selectList);i++){
			testmode->textParam->writex = selectList[i].writex;
			testmode->textParam->writey = selectList[i].writey;
			if( testmode->listPosition == i ){
				testmode->textParam->colorF = 2;
			} else {
				testmode->textParam->colorF = 1;
			}
			GFL_TEXT_PrintSjisCode( selectList[i].name, testmode->textParam );
		}
		bgchr_trans( testmode->chrbuf );
		testmode->seq++;
#endif
		break;

	case 3:
#if 0
		if( GFL_UI_KeyGetTrg() == PAD_KEY_UP ){
			if( testmode->listPosition > 0 ){
				testmode->listPosition--;
				testmode->seq--;
			}
		} else if( GFL_UI_KeyGetTrg() == PAD_KEY_DOWN ){
			if( testmode->listPosition < NELEMS(selectList)-1 ){
				testmode->listPosition++;
				testmode->seq--;
			}
		}
#endif
		break;

	case 4:
		GFL_BMPWIN_Delete( testmode->bmpwin[1] );
		GFL_BMPWIN_Delete( testmode->bmpwin[0] );
		bg_exit();
		break;
	}
	return return_flag;
}

#if 0
//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bgscr_trans( void* scr )
{
	//GFL_STD_MemCopy32(scr,(void*)G2_GetBG3ScrPtr(),XYSIZE*2);
	DC_FlushRange( scr, XYSIZE*2 );
	GX_LoadBG3Scr( scr, 0, XYSIZE*2 ); 
}

static void	bgchr_trans( void* chr )
{
	//GFL_STD_MemCopy32(chr,(void*)(u32)G2_GetBG3CharPtr(),XYSIZE*0x20);
	DC_FlushRange( chr, XYSIZE*0x20 );
	GX_LoadBG3Char( chr, 0, XYSIZE*0x20 ); 
}

static void	bgplt_trans( void* plt )
{
	//GFL_STD_MemCopy16(plt,(void*)HW_BG_PLTT,16*2);
	DC_FlushRange( plt, 16*2 );
	GX_LoadBGPltt( plt, 0, 16*2 );
}

static void	bg_init( void )
{
	GX_SetBankForBG(GX_VRAM_BG_64_E);
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D); 
	G2_SetBG3ControlText(	GX_BG_SCRSIZE_TEXT_256x256,GX_BG_COLORMODE_16,
							GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000 );
	GX_SetVisiblePlane(GX_PLANEMASK_BG3);
	G2_SetBG3Priority(0);

	{//スクリーン作成＆転送
		int	i;
		u16* scr = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,XYSIZE*2);

		for(i=0;i<XYSIZE;i++){
			scr[i] = i; 	//キャラナンバー0から埋める
		}
		bgscr_trans(scr);
		GFL_HEAP_FreeMemory(scr);
	}
	{//キャラクターバッファ作成＆転送
		u16* chr = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,XYSIZE*0x20);
		bgchr_trans(chr);
		testmode->chrbuf = chr;	//キャラ転送用に保存
	}
	{//パレット作成＆転送
		u16* plt = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,16*2);

		plt[0] = 0x5041;	//青
		plt[1] = 0x7fff;	//白
		plt[2] = 0x001f;	//赤
		bgplt_trans(plt);
		GFL_HEAP_FreeMemory(plt);
	}
	{//ビットマップヘッダ作成
		GFL_BMP_DATA* bmpchr = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,sizeof(GFL_BMP_DATA));

		bmpchr->adrs = testmode->chrbuf;
		bmpchr->size_x = 32*8;
		bmpchr->size_y = 32*8;
		testmode->bmpHeader = bmpchr;
	}
	{//文字表示パラメータワーク作成
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocMemoryLowClear
										(GFL_HEAPID_APP,sizeof(GFL_TEXT_PRINTPARAM));
		param->bmp = testmode->bmpHeader;
		param->writex = 16;
		param->writey = 8;
		param->spacex = 1;
		param->spacey = 1;
		param->colorF = 1;
		param->colorB = 0;
		param->mode = GFL_TEXT_WRITE_16;
		testmode->textParam = param;
	}
	GX_DispOn(); 
}

static void	bg_exit( void )
{
	GFL_HEAP_FreeMemory(testmode->bmpHeader);
	GFL_HEAP_FreeMemory(testmode->textParam);
	GFL_HEAP_FreeMemory(testmode->chrbuf);
}

//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
//------------------------------------------------------------------
static const char	test_index0[] = 
{"ゲームフリーク　プログラムライブラリ　テストモード\nGame Freak Libraries TestMode"};
static const char	test_index1[] = {"http://www.gamefreak.co.jp"};

static const char	test_select0[] = {"わたなべ　てつや\nTetsuya Watanabe"};
static const char	test_select1[] = {"たまだ　そうすけ\nSousuke Tamada"};
static const char	test_select2[] = {"そがべ　ひさし\nHisashi Sogabe"};
static const char	test_select3[] = {"おおの　かつみ\nKatsumi Ohno"};
static const char	test_select4[] = {"みつはら　さとし\nSatoshi Mitsuhara"};

typedef struct {
	const char*	name;
	u8 writex;
	u8 writey;
}TESTMODE_PRINTLIST;

static const TESTMODE_PRINTLIST indexList[] = {
	{ test_index0, 16, 8},
	{ test_index1, 16, 180},
};

static const TESTMODE_PRINTLIST selectList[] = {
	{ test_select0, 32, (0*25)+40},
	{ test_select1, 32, (1*25)+40},
	{ test_select2, 32, (2*25)+40},
	{ test_select3, 32, (3*25)+40},
	{ test_select4, 32, (4*25)+40},
};

static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( testmode->seq ){

	case 0:
		bg_init();
		testmode->listPosition = 0;
		testmode->seq++;
		break;

	case 1:
		for(i=0;i<NELEMS(indexList);i++){
			testmode->textParam->writex = indexList[i].writex;
			testmode->textParam->writey = indexList[i].writey;
			GFL_TEXT_PrintSjisCode( indexList[i].name, testmode->textParam );
		}
		testmode->seq++;
		break;

	case 2:
		for(i=0;i<NELEMS(selectList);i++){
			testmode->textParam->writex = selectList[i].writex;
			testmode->textParam->writey = selectList[i].writey;
			if( testmode->listPosition == i ){
				testmode->textParam->colorF = 2;
			} else {
				testmode->textParam->colorF = 1;
			}
			GFL_TEXT_PrintSjisCode( selectList[i].name, testmode->textParam );
		}
		bgchr_trans( testmode->chrbuf );
		testmode->seq++;
		break;

	case 3:
		if( GFL_UI_KeyGetTrg() == PAD_KEY_UP ){
			if( testmode->listPosition > 0 ){
				testmode->listPosition--;
				testmode->seq--;
			}
		} else if( GFL_UI_KeyGetTrg() == PAD_KEY_DOWN ){
			if( testmode->listPosition < NELEMS(selectList)-1 ){
				testmode->listPosition++;
				testmode->seq--;
			}
		}
		break;

	case 4:
		bg_exit();
		break;
	}
	return return_flag;
}

#endif

