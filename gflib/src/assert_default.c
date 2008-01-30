//=============================================================================================
/**
 * @file	assert_default.c
 * @date	2007.07.03
 * @author	GAME FREAK inc.
 */
//=============================================================================================

#include "gflib.h"
#include "textprint.h"

#include "assert_default.h"


static GFL_TEXT_PRINTPARAM PrintParam = {0};

enum {
	SCREEN_X_SIZE = 32,
	SCREEN_Y_SIZE = 24,
	BMP_CHARDATA_SIZE = SCREEN_X_SIZE*SCREEN_Y_SIZE*0x20,
};

//------------------------------------------------------------------
/**
 * アサート情報の実機表示初期化処理
 */
//------------------------------------------------------------------
static void dispInit( void )
{
	enum {
		FONT_COLOR_POS = 1,
		BASE_COLOR_POS = 2,
		FONT_COLOR = 0x7fff,
		BASE_COLOR = 0x60e1,
	};

	OS_DisableIrq();

	GX_SetVisiblePlane( 0 );

	GX_ResetBankForBG();
	GX_SetBankForBG( GX_VRAM_BG_128_A );
	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	G2_BlendNone();
	GX_SetMasterBrightness(0);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);

	GX_SetVisiblePlane(GX_PLANEMASK_BG0);
	GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);
	GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);
	G2_SetBG0Control( GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
			 GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, GX_BG_EXTPLTT_01 );
	G2_SetBG0Priority( 0 );
	G2_SetBG0Offset(0, 0);

	*(((u16*)HW_BG_PLTT)+FONT_COLOR_POS) = FONT_COLOR;
	*(((u16*)HW_BG_PLTT)+BASE_COLOR_POS) = BASE_COLOR;

	GFL_TEXT_CreateSystem(NULL);

	PrintParam.bmp = GFL_BMP_Create( SCREEN_X_SIZE, SCREEN_Y_SIZE, GFL_BMP_16_COLOR, GFL_HEAPID_SYSTEM );
	PrintParam.writex = 0;
	PrintParam.writey = 0;
	PrintParam.spacex = 1;
	PrintParam.spacey = 1;
	PrintParam.colorF = FONT_COLOR_POS;
	PrintParam.colorB = BASE_COLOR_POS;
	PrintParam.mode = GFL_TEXT_WRITE_16;

	GFL_BMP_Clear( PrintParam.bmp, BASE_COLOR_POS );
}

//------------------------------------------------------------------
/**
 * アサート情報の実機表示メイン処理
 */
//------------------------------------------------------------------
static void dispMain( const char* str )
{
	GFL_TEXT_PrintSjisCode( str, &PrintParam );

	// 文字列終端に改行コードが無い場合、
	// 続けて次の文字列を描画すると上書きしてしまうので、勝手に改行させている
	{
		u8 cr = 0;
		while( *str != '\0' )
		{
			if( *str++ == '\n' )
			{
				cr = 1;
				break;
			}
		}
		if( !cr )
		{
			GFL_TEXT_PrintSjisCode( "\n", &PrintParam );
		}
	}
}

//------------------------------------------------------------------
/**
 * アサート情報の実機表示終了処理
 */
//------------------------------------------------------------------
static void dispFinish( void )
{
	u16 *scr;
	int x, y, c;

	scr = G2_GetBG0ScrPtr();
	c = 0;
	for(y=0; y<SCREEN_Y_SIZE; y++)
	{
		for(x=0; x<SCREEN_X_SIZE; x++)
		{
			scr[x] = c++;
		}
		scr += 32;
	}

	{
		const u8* cgxData =  GFL_BMP_GetCharacterAdrs(PrintParam.bmp);
		DC_FlushRange( cgxData, BMP_CHARDATA_SIZE );
		GX_LoadBG0Char( cgxData, 0, BMP_CHARDATA_SIZE );
	}


	OS_Terminate();
    
}


//------------------------------------------------------------------
/**
 * 実機動作時に、ASSERT情報を実機画面表示するモードに切り替える
 * ※ 実機動作時以外は何もしない
 */
//------------------------------------------------------------------
void GFL_ASSERT_SetLCDMode( void )
{
	if( OS_GetConsoleType() & OS_CONSOLE_NITRO )
	{
		GFL_ASSERT_SetDisplayFunc( dispInit, dispMain, dispFinish );
	}
}



