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

	GFL_BG_sysInit( heapID );

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
		GFL_BG_BGControlSet( TEXT_FRM, &bgCont, GFL_BG_MODE_TEXT );
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
		GFL_BMPWIN_sysInit( &bmpwinSet, heapID );
	}
	{//パレット作成＆転送
		u16* plt = GFL_HEAP_AllocMemoryLowClear( heapID, 16*2 );
		plt[0] = 0x5041;	//青(背景)

		plt[1] = 0x7fff;	//白
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = 0x001f;	//赤
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 16*2 );

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
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	bg_exit( void )
{
	GFL_BMPWIN_sysExit();
	GFL_BG_BGControlExit( TEXT_FRM );
	GFL_BG_sysExit();
}

//------------------------------------------------------------------
/**
 * @brief		メッセージビットマップウインドウコントロール
 */
//------------------------------------------------------------------
static void msg_bmpwin_make( u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 0 );

	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	testmode->textParam->writex = 0;
	testmode->textParam->writey = 0;
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	GFL_BMPWIN_UploadChar( testmode->bmpwin[bmpwinNum] );
	GFL_BMPWIN_MakeScrn( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_trush( u8 bmpwinNum )
{
	GFL_BMPWIN_Delete( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_palset( u8 bmpwinNum, u8 pal )
{
	GFL_BMPWIN_SetPal( testmode->bmpwin[bmpwinNum], pal );
}
	
//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
//------------------------------------------------------------------
enum {
	NUM_TITLE = 0,
	NUM_URL,
	NUM_SELECT1,
	NUM_SELECT2,
	NUM_SELECT3,
	NUM_SELECT4,
	NUM_SELECT5,
};

enum {
	MSG_WHITE = 0,
	MSG_RED,
};

static const char	test_index1[] = 
{"ゲームフリーク　プログラムライブラリ　テストモード\nGame Freak Libraries TestMode"};
static const char	test_index2[] = {"http://www.gamefreak.co.jp"};

static const char	test_select1[] = {"わたなべ　てつや\nTetsuya Watanabe"};
static const char	test_select2[] = {"たまだ　そうすけ\nSousuke Tamada"};
static const char	test_select3[] = {"そがべ　ひさし\nHisashi Sogabe"};
static const char	test_select4[] = {"おおの　かつみ\nKatsumi Ohno"};
static const char	test_select5[] = {"みつはら　さとし\nSatoshi Mitsuhara"};

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

static const TESTMODE_PRINTLIST indexList[] = {
	{ test_index1, 2,  1, 30, 2 },
	{ test_index2, 2, 22, 30, 1 },
};

static const TESTMODE_PRINTLIST selectList[] = {
#if 0
	{ test_select1, 4,  5, 20, 3 },
	{ test_select2, 4,  8, 20, 3 },
	{ test_select3, 4, 11, 20, 3 },
	{ test_select4, 4, 14, 20, 3 },
	{ test_select5, 4, 17, 20, 3 },
#else
	{ test_select1,  1,  5, 14, 3 },
	{ test_select2, 15,  5, 14, 3 },
	{ test_select3,  1,  8, 14, 3 },
	{ test_select4, 15,  8, 14, 3 },
	{ test_select5,  1, 11, 14, 3 },
#endif
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
			msg_bmpwin_make( NUM_TITLE+i, indexList[i].msg,
				indexList[i].posx, indexList[i].posy, indexList[i].sizx, indexList[i].sizy );
		}
		for(i=0;i<NELEMS(selectList);i++){
			msg_bmpwin_make( NUM_SELECT1+i, selectList[i].msg,
				selectList[i].posx, selectList[i].posy, selectList[i].sizx, selectList[i].sizy );
		}
		testmode->seq++;
		break;

	case 2:
		for(i=0;i<NELEMS(selectList);i++){
			if( i == testmode->listPosition ){
				msg_bmpwin_palset( NUM_SELECT1+i, MSG_RED );
			}else{
				msg_bmpwin_palset( NUM_SELECT1+i, MSG_WHITE );
			}
			GFL_BMPWIN_MakeScrn( testmode->bmpwin[NUM_SELECT1+i] );
		}
		GFL_BG_LoadScreenReq( TEXT_FRM );
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
		for(i=0;i<NELEMS(indexList);i++){
			msg_bmpwin_trush( NUM_TITLE+i );
		}
		for(i=0;i<NELEMS(selectList);i++){
			msg_bmpwin_trush( NUM_SELECT1+i );
		}
		bg_exit();
		break;
	}
	return return_flag;
}


