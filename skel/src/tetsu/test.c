//===================================================================
/**
 * @file	test.c
 */
//===================================================================
#include "gflib.h"
#include "main.h"

#include "ui.h"
#include "textprint.h"

typedef struct {
	int		seq;
	u16		work[32];
	void*	wp[32];

}TEST_WORK;

TEST_WORK* test;

void	TetsuTestInit(void);
void	TetsuTestMain(void);

static void	TestPrintInit(void);
static void	TestPrintMain(void);
//------------------------------------------------------------------
/**
 * @brief		ゲームごとの初期化処理
 */
//------------------------------------------------------------------
void	TetsuTestInit(void)
{
	test = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,sizeof(TEST_WORK));
	TestPrintInit();
}

//------------------------------------------------------------------
/**
 * @brief		ゲームごとのメイン処理
 */
//------------------------------------------------------------------
void	TetsuTestMain(void)
{
	TestPrintMain();
}


//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
//------------------------------------------------------------------
const char		test_text[] = {	
"ゲームフリーク　プログラムライブラリ　さくせいチーム\nGame Freak program_Libraries product_team\n\nわたなべ　てつや\nTetsuya Watanabe\n\nたまだ　そうすけ\nSousuke Tamada\n\nそがべ　ひさし\nHisashi Sogabe\n\nおおの　かつみ\nKatsumi Ohno\n\nみつはら　さとし\nSatoshi Mitsuhara\n\nhttp://www.gamefreak.co.jp"
};

#define XYSIZE (32*32)

static void	TestPrintInit(void)
{
	GFL_TEXT_sysInit( "gfl_graphic/gfl_font.dat" );
}

static void	TestPrintMain(void)
{
	switch( test->seq ){
	case 0:
		GX_SetBankForBG(GX_VRAM_BG_64_E);
		GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D); 
		G2_SetBG3ControlText(	GX_BG_SCRSIZE_TEXT_256x256,GX_BG_COLORMODE_16,
								GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000 );
		GX_SetVisiblePlane(GX_PLANEMASK_BG3);
		G2_SetBG3Priority(0);
		{
			//スクリーン作成＆転送
			int	i;
			u16* scr = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,XYSIZE*2);

			for(i=0;i<XYSIZE;i++){
				scr[i] = i; 	//キャラナンバー0から埋める
			}
			//GFL_STD_MemCopy32(scr,(void*)G2_GetBG3ScrPtr(),XYSIZE*2);
			DC_FlushRange( scr, XYSIZE*2 );
			GX_LoadBG3Scr( scr, 0, XYSIZE*2 ); 

			GFL_HEAP_FreeMemory(scr);
			test->wp[1] = (void*)G2_GetBG3ScrPtr();
		}
		{
			//キャラクターバッファ作成＆転送
			u16* chr = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,XYSIZE*0x20);
			//GFL_STD_MemCopy32(chr,(void*)(u32)G2_GetBG3CharPtr(),XYSIZE*0x20);
			DC_FlushRange( chr, XYSIZE*0x20 );
			GX_LoadBG3Char( chr, 0, XYSIZE*0x20 ); 

			test->wp[0] = chr;	//キャラ転送用に保存
			test->wp[2] = (void*)G2_GetBG3CharPtr();
		}
		{
			//パレット作成＆転送
			u16* plt = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,16*2);

			plt[0] = 0x5041;
			plt[3] = 0x7fff;
			plt[7] = 0x5041;
			//GFL_STD_MemCopy16(plt,(void*)HW_BG_PLTT,16*2);
			DC_FlushRange( plt, 16*2 );
			GX_LoadBGPltt( plt, 0, 16*2 );

			GFL_HEAP_FreeMemory(plt);
			test->wp[3] = (void*)HW_BG_PLTT;
		}
		GX_DispOn(); 
		test->seq++;
		break;
	case 1:
		if( GFL_UI_KeyGetCont() == PAD_BUTTON_A ){
			GFL_TEXT_PRINTPARAM*	test_param;
			GFL_BMP_DATA*			bmpchr;

			test_param = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,sizeof(GFL_TEXT_PRINTPARAM));
			bmpchr = GFL_HEAP_AllocMemoryLowClear(GFL_HEAPID_APP,sizeof(GFL_BMP_DATA));
	
			bmpchr->adrs = test->wp[0];
			bmpchr->size_x = 32*8;
			bmpchr->size_y = 32*8;
			test_param->bmp = bmpchr;
			test_param->writex = 16;
			test_param->writey = 8;
			test_param->spacex = 1;
			test_param->spacey = 1;
			test_param->colorF = 3;
			test_param->colorB = 7;
			test_param->mode = GFL_TEXT_WRITE_16;
			GFL_TEXT_PrintSjisCode( test_text, test_param );
			{
				//キャラクターバッファ作成＆転送
				//GFL_STD_MemCopy32(test->wp[0],(void*)(u32)G2_GetBG3CharPtr(),XYSIZE*0x20);
				DC_FlushRange( test->wp[0], XYSIZE*0x20 );
				GX_LoadBG3Char( test->wp[0], 0, XYSIZE*0x20 ); 
			}
			GFL_HEAP_FreeMemory( bmpchr );
			GFL_HEAP_FreeMemory( test_param );
		} else if( GFL_UI_KeyGetCont() == PAD_BUTTON_B ){
			{
				GFL_STD_MemClear32(test->wp[0],XYSIZE*0x20);
				//キャラクターバッファ作成＆転送
				//GFL_STD_MemCopy32(test->wp[0],(void*)(u32)G2_GetBG3CharPtr(),XYSIZE*0x20);
				DC_FlushRange( test->wp[0], XYSIZE*0x20 );
				GX_LoadBG3Char( test->wp[0], 0, XYSIZE*0x20 ); 
			}
		}
		break;
	}
}


