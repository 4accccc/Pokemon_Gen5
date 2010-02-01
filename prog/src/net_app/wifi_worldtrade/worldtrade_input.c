//============================================================================================
/**
 * @file	worldtrade_input.c
 * @brief	世界交換条件入力ウインドウ処理
 * @author	Akito Mori
 * @date	08.11.01
 */
//============================================================================================
#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dpw_tr.h>
#include "print/wordset.h"
#include "message.naix"
//#include "system/fontproc.h"
//#include "system/fontoam.h"
//#include "system/window.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_gts.h"
#include "msg/msg_wifi_system.h"

#include "worldtrade.naix"			// グラフィックアーカイブ定義
#include "wifip2pmatch.naix"			// グラフィックアーカイブ定義


static int	wi_seq_none( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_winin( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_winwait( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_winout( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head1_init( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head1_main( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head1_exit( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head1_return( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head2_init( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head2_main( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head2_exit( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_head2_return( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_pokename_init( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_pokename_main( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_pokename_cancel_exit( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_pokename_exit( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head1_init( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head1_main( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head1_exit( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head1_return( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head2_init( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head2_main( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head2_exit( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_head2_return( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_init( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_main( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_cancel_exit( WORLDTRADE_INPUT_WORK *wk );
static int  wi_seq_nation_exit( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_sex_init( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_sex_main( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_sex_exit( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_level_init( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_level_main( WORLDTRADE_INPUT_WORK *wk );
static int	wi_seq_level_exit( WORLDTRADE_INPUT_WORK *wk );

static void select_bmpwin_add( WORLDTRADE_INPUT_WORK *wk, int mode );
static void select_bmpwin_del( WORLDTRADE_INPUT_WORK *wk, int mode );
static void SelectFrameBoxWrite( u16 *scr, int x, int y, int type );
static void SexSelect_Init( WORLDTRADE_INPUT_WORK *wk );
static void SexSelect_Main( WORLDTRADE_INPUT_WORK *wk );
static void LevelSelect_Init( WORLDTRADE_INPUT_WORK *wk );

static void HeadWord1_Init( WORLDTRADE_INPUT_WORK *wk, int type, int x );
static  u32 WordHead_SelectMain( WORLDTRADE_INPUT_WORK *wk, u8 *see_check8 );
static  u32 WordHead2_SelectMain( WORLDTRADE_INPUT_WORK *wk, u8 *see_check );
static  u32 LevelSelect_SelectMain( WORLDTRADE_INPUT_WORK *wk );
static  u32 Nation_SelectMain( WORLDTRADE_INPUT_WORK *wk );
static void system_print( BGWINFRM_WORK *wfwk, GFL_MSGDATA *mm, GFL_BMPWIN *win, int mesno, PRINTSYS_LSB color, WT_PRINT *print );
static void touch_print( BGWINFRM_WORK *wfwk, GFL_MSGDATA *mm, GFL_BMPWIN *win, int mesno, WT_PRINT *print );
static  int PokeName_MakeSortList( BMPLIST_DATA **list, GFL_MSGDATA *monsnameman, 
									GFL_MSGDATA *msgman,u8 *sinou, int select, ZUKAN_WORK *zukan );
static  int PokeName_GetSortNum( u8 *sinou, ZUKAN_WORK *zukan, int pokenum, u16 *sortlist );
static void PokeName_ListPrint( WORLDTRADE_INPUT_WORK *wk, int page, int max );
static  int _list_page_num( int num, int in_page );
static  int Nation_GetSortListNum( int start, int *number );
static  int Nation_MakeSortList( BMP_MENULIST_DATA **menulist, GFL_MSGDATA *CountryNameManager, int start );
static void Nation_ListPrint( WORLDTRADE_INPUT_WORK *wk, int page, int max );
static void PagePrint( BGWINFRM_WORK *wfwk, NUMFONT *numfont, GFL_BMPWIN *win, int page, int max );
static  u32 TouchPanelFunc( WORLDTRADE_INPUT_WORK *wk, int mode );
static  u32 Head2DecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide );
static  u32 NameDecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide );
static  u32 NationHead1DecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide );
static  u32 NationDecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide );
static void LevelPageRefresh( WORLDTRADE_INPUT_WORK *wk, int move );
static  u32 LevelDecideFunc( WORLDTRADE_INPUT_WORK *wk, u32 decide );
static void NationPageRefresh( WORLDTRADE_INPUT_WORK *wk, int move);
static void NamePageRefresh( WORLDTRADE_INPUT_WORK *wk, int move );

static int PokeSeeCount_Siin(WORLDTRADE_INPUT_WORK *wk, int siin);
static BOOL PokeSeeCount_Category(WORLDTRADE_INPUT_WORK *wk, int select);
static int NationCount_Siin(WORLDTRADE_INPUT_WORK *wk, int siin);
static BOOL NationCount_Category(WORLDTRADE_INPUT_WORK *wk, int select);

///検索文字色:ノーマル
#define SEARCH_MOJI_COLOR_NORMAL       (PRINTSYS_LSB_Make(15,14,2))
///検索文字色:灰色
#define SEARCH_MOJI_COLOR_GRAY       (PRINTSYS_LSB_Make(8,9,2))


// 条件入力シーケンス
enum{
	WI_SEQ_NONE=0,
	WI_SEQ_WININ,
	WI_SEQ_WINWAIT,
	WI_SEQ_WINOUT,

	WI_SEQ_HEAD1_INIT,
	WI_SEQ_HEAD1_MAIN,
	WI_SEQ_HEAD1_EXIT,
	WI_SEQ_HEAD1_RETURN,

	WI_SEQ_HEAD2_INIT,
	WI_SEQ_HEAD2_MAIN,
	WI_SEQ_HEAD2_EXIT,
	WI_SEQ_HEAD2_RETURN,

	WI_SEQ_POKENAME_INIT,
	WI_SEQ_POKENAME_MAIN,
	WI_SEQ_POKENAME_CANCEL_EXIT,
	WI_SEQ_POKENAME_EXIT,
	
	WI_SEQ_NATION_HEAD1_INIT,
	WI_SEQ_NATION_HEAD1_MAIN,
	WI_SEQ_NATION_HEAD1_EXIT,
	WI_SEQ_NATION_HEAD1_RETURN,

	WI_SEQ_NATION_HEAD2_INIT,
	WI_SEQ_NATION_HEAD2_MAIN,
	WI_SEQ_NATION_HEAD2_EXIT,
	WI_SEQ_NATION_HEAD2_RETURN,

	WI_SEQ_NATION_INIT,
	WI_SEQ_NATION_MAIN,
	WI_SEQ_NATION_CANCEL_EXIT,
	WI_SEQ_NATION_EXIT,
	
	WI_SEQ_SEX_INIT,
	WI_SEQ_SEX_MAIN,
	WI_SEQ_SEX_EXIT,

	WI_SEQ_LEVEL_INIT,
	WI_SEQ_LEVEL_MAIN,
	WI_SEQ_LEVEL_EXIT,

	WI_SEQ_MAX,
};


static int (*FuncTable[])( WORLDTRADE_INPUT_WORK *wk ) = {
	wi_seq_none,
	wi_seq_winin,
	wi_seq_winwait,
	wi_seq_winout,

	wi_seq_head1_init,
	wi_seq_head1_main,
	wi_seq_head1_exit,
	wi_seq_head1_return,

	wi_seq_head2_init,
	wi_seq_head2_main,
	wi_seq_head2_exit,
	wi_seq_head2_return,

	wi_seq_pokename_init,
	wi_seq_pokename_main,
	wi_seq_pokename_cancel_exit,
	wi_seq_pokename_exit,
	
	wi_seq_nation_head1_init,
	wi_seq_nation_head1_main,
	wi_seq_nation_head1_exit,
	wi_seq_nation_head1_return,

	wi_seq_nation_head2_init,
	wi_seq_nation_head2_main,
	wi_seq_nation_head2_exit,
	wi_seq_nation_head2_return,

	wi_seq_nation_init,
	wi_seq_nation_main,
	wi_seq_nation_cancel_exit,
	wi_seq_nation_exit,
	
	wi_seq_sex_init,
	wi_seq_sex_main,
	wi_seq_sex_exit,

	wi_seq_level_init,
	wi_seq_level_main,
	wi_seq_level_exit,

};

// ５０音順の頭文字１文字目定義
enum{
	INPUT_HEAD_A  =0,
	INPUT_HEAD_KA,
	INPUT_HEAD_SA,
	INPUT_HEAD_TA,
	INPUT_HEAD_NA,
	INPUT_HEAD_HA,
	INPUT_HEAD_MA,
	INPUT_HEAD_YA,
	INPUT_HEAD_RA,
	INPUT_HEAD_WA,
	INPUT_HEAD_CANCEL,	// 未使用：「やめる」の位置
	INPUT_HEAD_NONE,	// 「きにしない」
};


// 頭文字選択を行う際に
// 文字を書く座標
static const u8 word_table[][2]={
	{  1,  5 },	// １列目
	{  4,  5 },	
	{  7,  5 },
	{ 10,  5 },
	{ 13,  5 },

	{  1,  8 },	// ２列目
	{  4,  8 },
	{  7,  8 },
	{ 10,  8 },
	{ 13,  8 },
	
	{  9,  17 },// もどる
	{  1,   2 },// 頭文字入力２ページ目の（アイウエの「ア」）の位置
	{  1,   2 },// 国選択の時の「きにしない」
};

// 頭文字選択を行う際の
// カーソル位置座標
static const u8 word_cur_table[][2]={
	{  1,  5 },	// １列目
	{  4,  5 },	
	{  7,  5 },
	{ 10,  5 },
	{ 13,  5 },

	{  1,  8 },	// ２列目
	{  4,  8 },
	{  7,  8 },
	{ 10,  8 },
	{ 13,  8 },
	
	{  8,  17 },// もどる
	{  1,   2 },// 頭文字入力２ページ目の（アイウエの「ア」）の位置
	{  1,   2 },// 国選択の時の「きにしない」
};



static const u8 n_word_table[][2]={
	{  1,  5 },	// １列目
	{  4,  5 },	
	{  7,  5 },
	{ 10,  5 },
	{ 13,  5 },

	{  1,  8 },	// ２列目
	{  4,  8 },
	{  7,  8 },
	{ 10,  8 },
	{ 13,  8 },
	
	{  8,  17 },// もどる
	{  8,   3 },// 国選択の時の「きにしない」
};

// ポケモン名表示BMPWIN位置
static const u8 name_table[][3]={
	{ 4,    2 },
	{ 4,    5 },
	{ 4,    8 },
	{ 4,   11 },

};


// 性別選択BMPWIN表示位置
static const u8 sexselect_table[][2]={
	{ 4,  3 },
	{ 4,  7 },
	{ 4, 11 },
};

// 性別選択BMPWIN表示位置
static const u8 levelselect_table[][2]={
	{ 4,  2 },
	{ 4,  5 },
	{ 4,  8 },
	{ 4, 11 },

};


static const u8 nation_table[][2]={
	{  3,  1, },
	{  3,  4, },
	{  3,  7, },
	{  3, 10, },
	{  3, 13, },
	{ 24, 17  },	// もどる
};

#define INPUT_LETTER_W		(  2 )	// 1文字表示W
#define INPUT_LETTER_H		(  2 )	// 1文字表示H
#define INPUT_BACK_W		(  6 )	// もどるW
#define INPUT_BACK_H		(  2 )	// もどるH
#define INPUT_POKENAME_W	(  8 )	// ポケモン名W
#define INPUT_POKENAME_H	(  2 )	// ポケモン名H
#define INPUT_NAME_PAGE_W	(  3 )	// 名前表示状態ページ数W
#define INPUT_NAME_PAGE_H	(  1 )	// 名前表示状態ページ数H
#define INPUT_SEXSELECT_W	(  7 )	// 性別選択W
#define INPUT_SEXSELECT_H	(  2 )	// 性別選択H
#define INPUT_LEVELSELECT_W	(  9 )	// レベル条件選択W
#define INPUT_LEVELSELECT_H	(  2 )	// レベル条件選択H
#define INPUT_NONE_W		(  9 )	// 「きにしない」W
#define INPUT_NONE_H		(  2 )	// 「きにしない」H
#define INPUT_NATION_W		( 23 )	// 国名W
#define INPUT_NATION_H		(  2 )	// 国名H

#define INPUT_NATION_NONE_SELECT_X ( 1 )	// 国名「きにしない」位置X
#define INPUT_NATION_NONE_SELECT_Y ( 2 )	// 国名「きにしない」位置Y

#define INPUT_PAGE_X		(  7 )	// レベルとポケモン名のページ表示座標
#define INPUT_PAGE_Y		( 14 )

#define INPUT_NATION_PAGE_X			( 15 )	// 国名ページ位置X
#define INPUT_NATION_PAGE_Y			( 16 )	// 国名ページ位置Y

#define HEAD1_MOVE_INIT_X 		( 32 )
#define HEAD1_REWRITE_INIT_X 	( 32-4*4 )

#define PAGE_ARROW1_X			( 154 )
#define PAGE_ARROW2_X			( 228 )
#define PAGE_ARROW_Y			( 120 )
#define NATION_PAGE_ARROW1_X	(  88 )
#define NATION_PAGE_ARROW2_X	( 176 )
#define NATION_PAGE_ARROW_Y		( 136 )
#define POKE_ARROW1_X			( 154 )
#define POKE_ARROW2_X			( 228 )
#define POKE_ARROW_Y			( 120 )

#if 0   //BTS1365対処用

#define HEAD1_BACK_BMPWIN_OFFSET	( INPUT_BMPWIN_OFFSET + INPUT_LETTER_W*INPUT_LETTER_H*10 )
#define HEAD1_NONE_BMPWIN_OFFSET	( HEAD1_BACK_BMPWIN_OFFSET+INPUT_BACK_W*INPUT_BACK_H )
#define HEAD2_BACK_BMPWIN_OFFSET	( INPUT_BMPWIN_OFFSET + INPUT_LETTER_W*INPUT_LETTER_H*6 )

#define POKENAME_PAGE_BMPWIN_OFFSET ( INPUT_BMPWIN_OFFSET+INPUT_POKENAME_W*INPUT_POKENAME_H*MODE_POKENAME_WIN_NUM )
#define POKENAME_BACK_BMPWIN_OFFSET	( POKENAME_PAGE_BMPWIN_OFFSET + INPUT_NAME_PAGE_W*INPUT_NAME_PAGE_W)
#define LEVEL_PAGE_BMPWIN_OFFSET    ( INPUT_BMPWIN_OFFSET+INPUT_LEVELSELECT_W*INPUT_LEVELSELECT_H*MODE_LEVEL_WIN_NUM )
#define LEVEL_BACK_BMPWIN_OFFSET    ( LEVEL_PAGE_BMPWIN_OFFSET + INPUT_NAME_PAGE_W*INPUT_NAME_PAGE_W )
#define NATION_PAGE_BMPWIN_OFFSET  	( INPUT_BMPWIN_OFFSET+INPUT_NATION_W*INPUT_NATION_H*MODE_NATION_WIN_NUM )
#define NATION_BACK_BMPWIN_OFFSET 	( NATION_PAGE_BMPWIN_OFFSET + INPUT_NAME_PAGE_W*INPUT_NAME_PAGE_W)
#define SEX_BACK_BMPWIN_OFFSET		( INPUT_BMPWIN_OFFSET+INPUT_SEXSELECT_W*INPUT_SEXSELECT_H*3 )

#else

#define NATION_PAGE_BMPWIN_OFFSET  	( INPUT_BMPWIN_OFFSET+INPUT_NATION_W*INPUT_NATION_H*MODE_NATION_WIN_NUM )
#define NATION_BACK_BMPWIN_OFFSET 	( NATION_PAGE_BMPWIN_OFFSET + INPUT_NAME_PAGE_W*INPUT_NAME_PAGE_W)

#define HEAD1_BACK_BMPWIN_OFFSET	( NATION_BACK_BMPWIN_OFFSET )
#define HEAD1_NONE_BMPWIN_OFFSET	( INPUT_BMPWIN_OFFSET + INPUT_LETTER_W*INPUT_LETTER_H*10 )
#define HEAD2_BACK_BMPWIN_OFFSET	( NATION_BACK_BMPWIN_OFFSET )

#define POKENAME_PAGE_BMPWIN_OFFSET ( INPUT_BMPWIN_OFFSET+INPUT_POKENAME_W*INPUT_POKENAME_H*MODE_POKENAME_WIN_NUM )
#define POKENAME_BACK_BMPWIN_OFFSET	( NATION_BACK_BMPWIN_OFFSET )
#define LEVEL_PAGE_BMPWIN_OFFSET    ( INPUT_BMPWIN_OFFSET+INPUT_LEVELSELECT_W*INPUT_LEVELSELECT_H*MODE_LEVEL_WIN_NUM )
#define LEVEL_BACK_BMPWIN_OFFSET    ( LEVEL_PAGE_BMPWIN_OFFSET + INPUT_NAME_PAGE_W*INPUT_NAME_PAGE_W )
#define SEX_BACK_BMPWIN_OFFSET		( INPUT_BMPWIN_OFFSET+INPUT_SEXSELECT_W*INPUT_SEXSELECT_H*3 )

#endif

// 文字表示BMPWINをクリアする際の背景色カラー指定
#define INPUTPLATE_BASE_COLOR_DATA	( 0x22 )

#define LV_PAGE_MAX	(2)

//------------------------------------------------------------------
/**
 * @brief   必要なBMPWINワークを確保する
 *
 * @param   wk		
 * @param   mode		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void select_bmpwin_add( WORLDTRADE_INPUT_WORK *wk, int mode )
{
	int i=0;
	switch(mode){

	// 名前入力頭文字１
	case MODE_HEADWORD_1:
		// 子音BMPWIN確保
		for(i=0;i<MODE_HEADWORD1_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   word_table[i][0],		word_table[i][1],	
									   INPUT_LETTER_W, 			INPUT_LETTER_H, 
									   WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+2*2*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
		//	GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}
		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	HEAD1_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );

		break;

	// 名前入力頭文字２
	case MODE_HEADWORD_2:
		// 母音BMPWIN確保
		wk->MenuBmp[0]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
								   word_table[11][0],		word_table[11][1],	
								   INPUT_LETTER_W, 			INPUT_LETTER_H, 
								   WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+2*2*0 );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[0]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[0] );

		for(i=1;i<MODE_HEADWORD2_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
								word_table[i-1][0],		word_table[i-1][1],	
								INPUT_LETTER_W, 		INPUT_LETTER_H, 
								WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+2*2*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}
		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	HEAD2_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );
		break;

	// 名前入力
	case MODE_POKEMON_NAME:
		for(i=0;i<MODE_POKENAME_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   name_table[i][0],		name_table[i][1],	
									   INPUT_POKENAME_W, 		INPUT_POKENAME_H, 
									   WORLDTRADE_INPUT_PAL,  	
									   INPUT_BMPWIN_OFFSET+INPUT_POKENAME_W*INPUT_POKENAME_H*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}

		// ページ数表示用BMP
		wk->MenuBmp[MODE_POKENAME_WIN_NUM]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   INPUT_PAGE_X,			INPUT_PAGE_Y,	
									   INPUT_NAME_PAGE_W, 		INPUT_NAME_PAGE_H, 
									   WORLDTRADE_INPUT_PAL,  	POKENAME_PAGE_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[MODE_POKENAME_WIN_NUM]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[MODE_POKENAME_WIN_NUM] );

		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	POKENAME_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );
		break;
	case MODE_NATION_HEAD1:
		// 子音BMPWIN確保
		for(i=0;i<MODE_NATION_HEADWORD1_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   word_table[i][0],		word_table[i][1],	
									   INPUT_LETTER_W, 			INPUT_LETTER_H, 
									   WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+2*2*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}

		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	HEAD1_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );

		// 「きにしない」
		wk->MenuBmp[BMPWIN_NONE_SELECT_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
								   INPUT_NATION_NONE_SELECT_X,	INPUT_NATION_NONE_SELECT_Y,	
								   INPUT_NONE_W, 				INPUT_NONE_H, 
								   WORLDTRADE_INPUT_PAL,  		HEAD1_NONE_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_NONE_SELECT_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_NONE_SELECT_WIN] );


		break;
	case MODE_NATION:
		// 国名BMPWIN確保
		for(i=0;i<MODE_NATION_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   nation_table[i][0],		nation_table[i][1],	
									   INPUT_NATION_W, 			INPUT_NATION_H, 
									   WORLDTRADE_INPUT_PAL,  	
									   INPUT_BMPWIN_OFFSET+INPUT_NATION_W*INPUT_NATION_H*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}
		// ページ数表示用BMP
		wk->MenuBmp[MODE_NATION_WIN_NUM]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   INPUT_NATION_PAGE_X,		INPUT_NATION_PAGE_Y,	
									   INPUT_NAME_PAGE_W, 		INPUT_NAME_PAGE_H, 
									   WORLDTRADE_INPUT_PAL,  	NATION_PAGE_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[MODE_NATION_WIN_NUM]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[MODE_NATION_WIN_NUM] );

		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									nation_table[5][0],		nation_table[5][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	NATION_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );

		break;
	case MODE_SEX:
		// 性別選択
		for(i=0;i<MODE_SEX_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   sexselect_table[i][0],	sexselect_table[i][1],	
									   INPUT_SEXSELECT_W, 		INPUT_SEXSELECT_H, 
									   WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+INPUT_SEXSELECT_W*INPUT_SEXSELECT_H*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}
		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	SEX_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );
		
		break;
	case MODE_LEVEL:
		// 性別選択
		for(i=0;i<MODE_LEVEL_WIN_NUM;i++){
			wk->MenuBmp[i]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   levelselect_table[i][0],	levelselect_table[i][1],	
									   INPUT_LEVELSELECT_W, 	INPUT_LEVELSELECT_H, 
									   WORLDTRADE_INPUT_PAL,  	INPUT_BMPWIN_OFFSET+INPUT_LEVELSELECT_W*INPUT_LEVELSELECT_H*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[i] );
		}

		// ページ数表示用
		wk->MenuBmp[MODE_LEVEL_WIN_NUM]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									   INPUT_PAGE_X,			INPUT_PAGE_Y,	
									   INPUT_NAME_PAGE_W, 		INPUT_NAME_PAGE_H, 
									   WORLDTRADE_INPUT_PAL,  	LEVEL_PAGE_BMPWIN_OFFSET);
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[MODE_LEVEL_WIN_NUM]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[MODE_LEVEL_WIN_NUM] );

		// もどる
		wk->MenuBmp[BMPWIN_SELECT_END_WIN]	= GFL_BMPWIN_CreateFixPos( wk->BgFrame,
									word_table[10][0],		word_table[10][1],	
									INPUT_BACK_W, 			INPUT_BACK_H, 
									WORLDTRADE_INPUT_PAL,  	LEVEL_BACK_BMPWIN_OFFSET );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[BMPWIN_SELECT_END_WIN]), INPUTPLATE_BASE_COLOR_DATA );
		//GFL_BMPWIN_MakeTransWindow( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );

		break;

	}


}

//------------------------------------------------------------------
/**
 * @brief   モード毎に確保したBMPWINワークを解放する
 *
 * @param   wk		
 * @param   mode		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void select_bmpwin_del( WORLDTRADE_INPUT_WORK *wk, int mode )
{
	int i;
	s8 px, py;
	
	BGWINFRM_PosGet( wk->BgWinFrm, 0, &px, &py );

	switch(mode){
	case MODE_HEADWORD_1:
		for(i=0;i<MODE_HEADWORD1_WIN_NUM;i++){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		if(px == HEAD1_REWRITE_INIT_X){
    		GFL_BG_FillScreen(
    		     GFL_BG_FRAME2_M, 5, 0x11, 1, 0x10-1, 0x10-1, GFL_BG_SCRWRT_PALNL);
    		GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M);
    	}
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );
		break;
	case MODE_HEADWORD_2:
		for(i=0;i<MODE_HEADWORD2_WIN_NUM;i++){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		if(px == HEAD1_REWRITE_INIT_X){
    		GFL_BG_FillScreen(
    		     GFL_BG_FRAME2_M, 5, 0x11, 1, 0x10-1, 0x10-1, GFL_BG_SCRWRT_PALNL);
    		GFL_BG_LoadScreenReq(GFL_BG_FRAME2_M);
    	}
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_SELECT_END_WIN] );
		break;
	case MODE_POKEMON_NAME:
		for(i=0;i<MODE_PAGE_WIN_NUM+MODE_POKENAME_WIN_NUM;i++){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		if(px == HEAD1_REWRITE_INIT_X){
    		GFL_BG_FillScreen(GFL_BG_FRAME2_M, 5, 0x11, 1, 0x10-1, 0x10-1, GFL_BG_SCRWRT_PALNL);
    		GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M);
    	}
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_SELECT_END_WIN]);
		break;
	case MODE_NATION_HEAD1:
		for(i=0;i<MODE_NATION_HEADWORD1_WIN_NUM;i++){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		if(px == HEAD1_REWRITE_INIT_X){
    		GFL_BG_FillScreen(GFL_BG_FRAME2_M, 5, 0x11, 1, 0x10-1, 0x10-1, GFL_BG_SCRWRT_PALNL);
    		GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M);
    	}
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_SELECT_END_WIN]);
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_NONE_SELECT_WIN]);
		break;
	case MODE_NATION:
		for(i=0;i<MODE_NATION_WIN_NUM+1;i++){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		if(px == HEAD1_REWRITE_INIT_X || px == 17){ //17=wipeoutしてくる時、この値(BTS4836)
    		GFL_BG_FillScreen(
    		    GFL_BG_FRAME2_M, 5, 0x11+1, 1, 0x10-2, 0x10-1, GFL_BG_SCRWRT_PALNL);
    		GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M);
    	}
		GFL_BMPWIN_Delete( wk->MenuBmp[BMPWIN_SELECT_END_WIN]);
		break;
	case MODE_SEX:
		for(i=0;i<MODE_SEX_WIN_NUM;i++){
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		// もどる
		GFL_BMPWIN_Delete(wk->MenuBmp[BMPWIN_SELECT_END_WIN]);
		break;
	case MODE_LEVEL:
		for(i=0;i<MODE_LEVEL_WIN_NUM+MODE_PAGE_WIN_NUM;i++){
			GFL_BMPWIN_Delete( wk->MenuBmp[i] );
		}
		// もどる
		GFL_BMPWIN_Delete(wk->MenuBmp[BMPWIN_SELECT_END_WIN]);
		break;
	}
}


// この入力システムはworldtrade_search.cとworldtrade_deposit.cで使用される。
// 各々の画面で必ずBG１を一面空けておき、そのBGで入力の進行が行われる。
// 
// 
// 
//==============================================================================
/**
 * @brief   各種条件（名前・性別・レベル・国）・入力ウインドウシステム
 *
 * @param   wih		
 *
 * @retval  WORLDTRADE_INPUT_WORK *		
 */
//==============================================================================
WORLDTRADE_INPUT_WORK * WorldTrade_Input_Init( WORLDTRADE_INPUT_HEADER *wih, int frame, int situation  )
{
	WORLDTRADE_INPUT_WORK *wk = GFL_HEAP_AllocMemory(HEAPID_WORLDTRADE, sizeof(WORLDTRADE_INPUT_WORK));

	wk->MenuBmp     = wih->MenuBmp;
	wk->BackBmp     = wih->BackBmp;
	wk->CursorAct   = wih->CursorAct;
	wk->ArrowAct[0] = wih->ArrowAct[0];
	wk->ArrowAct[1] = wih->ArrowAct[1];
	wk->SearchCursorAct = wih->SearchCursorAct;
	wk->MsgManager      = wih->MsgManager;			// 名前入力メッセージデータマネージャー
	wk->MonsNameManager = wih->MonsNameManager ;		// ポケモン名メッセージデータマネージャー
	wk->CountryNameManager = wih->CountryNameManager;
	wk->zukan			= wih->Zukan;
	wk->SinouTable		= wih->SinouTable;

	wk->BgFrame   = frame;
	wk->Situation = situation;
	wk->Head1   = 0;
	wk->Head2   = 0;
	wk->Poke    = 0;
	wk->Nation  = 0;
	wk->Sex     = 0;
	wk->Level   = 0;
	wk->listpos = 0;
	wk->listpos_backup_x = 0;
	wk->seq     = WI_SEQ_NONE;

	// BGウインドウフレームワーク確保
	wk->BgWinFrm = BGWINFRM_Create( BGWINFRM_TRANS_VBLANK, 1, HEAPID_WORLDTRADE );

	// 仮想スクリーン面追加
	BGWINFRM_Add( wk->BgWinFrm, 0, GFL_BG_FRAME2_M, 32, 20 );

	// サブカーソル初期座標設定
	WorldTrade_CLACT_PosChange( wk->CursorAct, (word_cur_table[0][0]+16)*8, word_cur_table[0][1]*8 );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 47 );

	// 数字フォントシステム初期化
	wk->NumFontSys = NUMFONT_Create( 15, 14, 2, HEAPID_WORLDTRADE );

	//	プリント初期化
	WT_PRINT_Init( &wk->print, wih->config );
	
	return wk;
}


//==============================================================================
/**
 * @brief   条件入力開始
 *			以後WorldTrade_Input_Main()呼び続けで入力は完了するはず
 *
 * @param   wk		WORLDTRADE_INPUT_WORK
 * @param   type	MODE_POKEMON_NAME ～ MODE_LEVEL  (worldtarde_input.h)
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_Input_Start( WORLDTRADE_INPUT_WORK *wk, int type )
{
	wk->type = type;
	switch(type){
	case MODE_POKEMON_NAME:
		//カーソル位置を初期化
		wk->listpos = 0;
    	wk->listpos_backup_x = 0;
		wk->Head1 = 0;
		wk->Head2 = 0;
		wk->seq = WI_SEQ_HEAD1_INIT;
		break;
	case MODE_NATION:
		//カーソル位置を初期化
		wk->listpos = 0;
    	wk->listpos_backup_x = 0;
		wk->Head1 = 0;
		wk->Head2 = 0;
		wk->seq = WI_SEQ_NATION_HEAD1_INIT;
		break;
	case MODE_SEX:
		//カーソル位置を初期化
		wk->listpos = 0;
    	wk->listpos_backup_x = 0;
		wk->seq = WI_SEQ_SEX_INIT;
		break;
	case MODE_LEVEL:
		//カーソル位置を初期化
		wk->listpos = 0;
    	wk->listpos_backup_x = 0;
		wk->seq = WI_SEQ_LEVEL_INIT;
		break;
	}
}

//==============================================================================
/**
 * @brief   各種条件（名前・性別・レベル・国）・入力ウインドウシステム解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_Input_Exit( WORLDTRADE_INPUT_WORK *wk )
{
	WT_PRINT_Exit( &wk->print );

	NUMFONT_Delete( wk->NumFontSys );
	BGWINFRM_Exit( wk->BgWinFrm );
	GFL_HEAP_FreeMemory( wk );
}

//==============================================================================
/**
 * @brief   入力処理メイン
 *
 * @param   wk		WORLDTRADE_INPUT_WORK
 *
 * @retval  u32		
 */
//==============================================================================
u32 WorldTrade_Input_Main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	ret=(*FuncTable[wk->seq])(wk);

	BGWINFRM_MoveMain( wk->BgWinFrm );
	WT_PRINT_Main( &wk->print );
#ifdef PM_DEBUG
	{
		int d = ret;
		if (d !=-1){
			OS_Printf("ret=%d\n",ret);
		}
	}
#endif	
	return ret;
}



// 頭文字ウインドウカーソル移動テーブル
// 0 1 2 3 4
// 5 6 7 8 9 
//        10
// 上下左右の順番で入力したキーに対応した場所に移動
static const u8 head1table[][4]={   //キーを押した時の次の飛び先　UP, DOWN, LEFT, RIGHT
	{ 10, 5, 4, 1, },        //0　あ
	{ 10, 6, 0, 2, },        //1　か
	{ 10, 7, 1, 3, },        //2　さ
	{ 10, 8, 2, 4, },        //3　た
	{ 10, 9, 3, 0, },        //4　な
	
	{ 0, 10, 9, 6, },        //5　は
	{ 1, 10, 5, 7, },        //6　ま
	{ 2, 10, 6, 8, },        //7　や
	{ 3, 10, 7, 9, },        //8　ら
	{ 4, 10, 8, 5, },        //9　わ
	
	{ 9,  4, 10, 10, },      //10　もどる
};

// 国指定のときに頭文字を選択する際のテーブル
// 11
// 0 1 2 3 4
// 5 6 7 8 9
//         10
static const u8 n_head1table[][4]={     //キーを押した時の次の飛び先　UP, DOWN, LEFT, RIGHT
	{ 11,  5,  4,  1, },        //0　あ
	{ 11,  6,  0, 2, },         //1　か
	{ 11,  7, 1, 3, },          //2　さ
	{ 11,  8, 2, 4, },          //3　た
	{ 11, 9, 3,    0, },        //4　な
	
	{ 0, 10,  9,  6, },         //5　は
	{  1, 10 , 5, 7, },         //6　ま
	{  2, 10, 6, 8, },          //7　や
	{  3, 10, 7, 9, },          //8　ら
	{  4, 10, 8,   5, },        //9　わ
	
	{  9, 11, 10, 10, },        //10　もどる
	{ 10, 0, 11, 11, },	        //11　きにしない
		
};

// 頭文字ウインドウカーソル移動テーブル2
// 0 1 2 3 4
//         5

static const u8 head2table[][4]={   //キーを押した時の次の飛び先　UP, DOWN, LEFT, RIGHT
	{ 5, 5, 4, 1, },        //0　あ
	{ 5, 5, 0, 2, },        //1　い
	{ 5, 5, 1, 3, },        //2　う
	{ 5, 5, 2, 4, },       //3　え
	{ 5, 5, 3, 0, },        //4　お
	
	{ 4,  4, 5, 5, },       //5　もどる
};



static const u8 namepostable[][2]={
	{ 192,  24 },
	{ 192,  48 },
	{ 192,  72 },
	{ 192,  96 },
	{ 192, 136 },
};

static const u8 sexpostable[][2]={
	{ 192,  32  },
	{ 192,  64  },
	{ 192,  96  },
	{ 192, 136  },
};

static const u8 levelpostable[][2]={
	{ 192,  24  },
	{ 192,  48  },
	{ 192,  72  },
	{ 192,  96  },
	{ 192, 136  },
};

static const u8 nationpostable[][2]={
	{ 136,  16  },
	{ 136,  40  },
	{ 136,  64  },
	{ 136,  88  },
	{ 136, 112  },
	{ 192, 136  },
};


// 頭文字１用タッチーテーブル
static const GFL_UI_TP_HITTBL Head1TouchTable[]={
	{  40,  60, 136, 151,},	// あ
	{  40,  60, 160, 175,},	// か
	{  40,  60, 184, 199,},	// さ
	{  40,  60, 208, 223,},	// た
	{  40,  60, 232, 247,},	// な
	{  64,  84, 136, 151,},	// は
	{  64,  84, 160, 175,},	// ま
	{  64,  84, 184, 199,},	// や
	{  64,  84, 208, 223,},	// ら
	{  64,  84, 232, 247,},	// わ
	{ 133, 154, 197, 250,},	// もどる
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
	
};


// 頭文字１用タッチーテーブル
static const GFL_UI_TP_HITTBL NationHead1TouchTable[]={
	{  40,  60, 136, 151,},	// あ
	{  40,  60, 160, 175,},	// か
	{  40,  60, 184, 199,},	// さ
	{  40,  60, 208, 223,},	// た
	{  40,  60, 232, 247,},	// な
	{  64,  84, 136, 151,},	// は
	{  64,  84, 160, 175,},	// ま
	{  64,  84, 184, 199,},	// や
	{  64,  84, 208, 223,},	// ら
	{  64,  84, 232, 247,},	// わ
	{ 133, 154, 197, 250,},	// もどる
	{  16,  35, 136, 248,},	// きにしない
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
	
};

// 頭文字２用タッチーテーブル
static const GFL_UI_TP_HITTBL Head2TouchTable[]={
	{  40,  60, 136, 151,},	// あ
	{  40,  60, 160, 175,},	// い
	{  40,  60, 184, 199,},	// う
	{  40,  60, 208, 223,},	// え
	{  40,  60, 232, 247,},	// お
	{ 133, 154, 197, 250,},	// もどる
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
	
};

// レベル＆ポケモン名用タッチーテーブル
static const GFL_UI_TP_HITTBL LevelTouchTable[]={
	{  16,  35, 136, 248,},	// きにしない
	{  40,  60, 136, 248,},	// LV10～
	{  64,  84, 136, 248,},	// LV20～
	{  88, 108, 136, 248,},	// LV30～
	{ 133, 154, 198, 248,},	// もどる
	{ 108, 129, 143, 160,}, // 左向き矢印
	{ 108, 126, 223, 240,}, // 右向き矢印
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
};

// 性別用タッチーテーブル
static const GFL_UI_TP_HITTBL SexTouchTable[]={
	{  24,  44, 136, 248,},	// きにしない
	{  56,  76, 136, 248,},	// オス
	{  88, 108, 136, 248,},	// メス
	{ 133, 154, 197, 250,},	// もどる
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
};

// 国名用タッチーテーブル
static const GFL_UI_TP_HITTBL NationTouchTable[]={
	{   8,  28,   8, 247,},	// 国名１
	{  32,  52,   8, 247,},	// 国名２
	{  56,  76,   8, 247,},	// 国名３
	{  80, 100,   8, 247,},	// 国名４
	{ 104, 124,   8, 247,},	// 国名５
	{ 133, 154, 195, 248,},	// もどる
	{ 124, 145,  77,  96,}, // 左向き矢印
	{ 124, 145, 173, 190,}, // 右向き矢印
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
};



static const u8 word2_table[][3]={
	{  2,  2, SELECT_BOX_R_LETTER},
	{  2,  5, SELECT_BOX_N_LETTER},
	{  5,  5, SELECT_BOX_N_LETTER},
	{  8,  5, SELECT_BOX_N_LETTER},
	{ 11,  5, SELECT_BOX_N_LETTER},
	{ 14,  5, SELECT_BOX_N_LETTER},
	
};

static const int head2_word_start[][2]={
	{  0,	5,},	// あ
	{  5,	5,},	// か
	{ 10,	5,},	// さ
	{ 15,	5,},	// た
	{ 20,	5,},	// な
	{ 25,	5,},	// は
	{ 30,	5,},	// ま
	{ 35,	3,},	// や
	{ 40,	5,},	// ら
	{ 45,	0,},	// わ
};


static const int head_letter_table[]={
	msg_gts_word_001,	msg_gts_word_002, msg_gts_word_003, msg_gts_word_004, msg_gts_word_005,	// あ
	msg_gts_word_006,	msg_gts_word_007, msg_gts_word_008, msg_gts_word_009, msg_gts_word_010, // か
	msg_gts_word_011,	msg_gts_word_012, msg_gts_word_013, msg_gts_word_014, msg_gts_word_015, // さ
	msg_gts_word_016,	msg_gts_word_017, msg_gts_word_018, msg_gts_word_019, msg_gts_word_020, // た
	msg_gts_word_021,	msg_gts_word_022, msg_gts_word_023, msg_gts_word_024, msg_gts_word_025, // な
	msg_gts_word_026,	msg_gts_word_027, msg_gts_word_028, msg_gts_word_029, msg_gts_word_030, // は
	msg_gts_word_031,	msg_gts_word_032, msg_gts_word_033, msg_gts_word_034, msg_gts_word_035, // ま
	msg_gts_word_036,	msg_gts_word_037, msg_gts_word_038, msg_gts_word_045, msg_gts_word_045, // や
	msg_gts_word_039,	msg_gts_word_040, msg_gts_word_041, msg_gts_word_042, msg_gts_word_043, // ら
	msg_gts_word_044,                                                                           // わ
};

static const int head2pokename[]={
	  0,	// あ
	  5,	// か
	 10,	// さ
	 15,	// た
	 20,	// な
	 25,	// は
	 30,	// ま
	 35,	// や
	 38,	// ら
	 43,	// わ
};




//------------------------------------------------------------------
/**
 * @brief   タッチパネル検出
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 TouchPanelFunc( WORLDTRADE_INPUT_WORK *wk, int mode )
{
	u32 ret;
	switch(mode){
	case MODE_HEADWORD_1:	// 頭文字子音指定
		ret=GFL_UI_TP_HitTrg(Head1TouchTable);
		break;
	case MODE_NATION_HEAD1:	// 国名頭文字指定
		ret=GFL_UI_TP_HitTrg(NationHead1TouchTable);
		break;
	case MODE_POKEMON_NAME:	// ポケモン名前指定
	case MODE_LEVEL:		// レベル条件指定
		ret=GFL_UI_TP_HitTrg(LevelTouchTable);
		break;
	case MODE_SEX:			// 性別条件指定
		ret=GFL_UI_TP_HitTrg(SexTouchTable);
		break;
	case MODE_NATION:		// 国名
		ret=GFL_UI_TP_HitTrg(NationTouchTable);
		break;
	case MODE_HEADWORD_2:	// 頭文字母音指定
	case MODE_NATION_HEAD2:	// 国名頭文字指定
		ret=GFL_UI_TP_HitTrg(Head2TouchTable);
		break;
	}

	return ret;
}


//------------------------------------------------------------------
/**
 * @brief   頭文字選択メイン
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 WordHead_SelectMain( WORLDTRADE_INPUT_WORK *wk, u8 *see_check )
{
	int oldpos = wk->listpos;
	
	if(oldpos < 10){    //あ～わ
	    wk->listpos_backup_x = oldpos;  //X位置を取っておく
	}
	
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		wk->listpos = head1table[wk->listpos][0];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		wk->listpos = head1table[wk->listpos][1];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		wk->listpos = head1table[wk->listpos][2];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		wk->listpos = head1table[wk->listpos][3];
	}

	//「もどる」の位置から「あ～わ」ラインに復帰した場合はX位置を戻す補正
	if(oldpos >= 10 && wk->listpos < 10){
        if(wk->listpos > 4){    //下段にもどってきた
            if(wk->listpos_backup_x > 4){
                wk->listpos = wk->listpos_backup_x;
            }
            else{
                wk->listpos = wk->listpos_backup_x + 5;
            }
            wk->listpos_backup_x = wk->listpos;
        }
        else{   //上段に戻ってきた
            if(wk->listpos_backup_x > 4){
                wk->listpos = wk->listpos_backup_x - 5;
            }
            else{
                wk->listpos = wk->listpos_backup_x;
            }
            wk->listpos_backup_x = wk->listpos;
        }
    }

	if(oldpos!=wk->listpos){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(word_cur_table[wk->listpos][0]+16)*8,
						word_cur_table[wk->listpos][1]*8 );
		if(wk->listpos==10){
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 47 );
		}

	}
	
	{
		u32 ret;
		// タッチ処理
		ret = TouchPanelFunc( wk, MODE_HEADWORD_1 );
		if(ret!=GFL_UI_TP_HIT_NONE){
            if(ret == 10){
    			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
                return BMPMENU_CANCEL;
            }
            GF_ASSERT(ret < 10);
            if(see_check == NULL || see_check[ret]){
    			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return ret;
			}
		}else{
		// キー処理
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				if(wk->listpos == 10){
					return BMPMENU_CANCEL;
				}
				GF_ASSERT(wk->listpos < 10);
				if(see_check == NULL || see_check[wk->listpos]){
    				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
					return wk->listpos;
				}
		
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   頭文字２の決定処理
 *
 * @param   wk		
 * @param   decide		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 Head2DecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide )
{
	if(decide!=5){
		if(wk->Head1==INPUT_HEAD_YA){
            if(decide<3){
    			return decide;
    		}
    		else{
                return BMPMENU_NULL;
            }
		}else if(wk->Head1!=INPUT_HEAD_YA){
			return decide;
		}
	}

	return BMPMENU_CANCEL;

}

//------------------------------------------------------------------
/**
 * @brief   頭文字２文字目選択メイン
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 WordHead2_SelectMain( WORLDTRADE_INPUT_WORK *wk, u8 *see_check )
{
	int oldpos = wk->listpos;

	if(oldpos < 5){    //あ～お
	    wk->listpos_backup_x = oldpos;  //X位置を取っておく
	}

	// カーソル移動
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		wk->listpos = head2table[wk->listpos][0];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		wk->listpos = head2table[wk->listpos][1];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		wk->listpos = head2table[wk->listpos][2];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		wk->listpos = head2table[wk->listpos][3];
	}

	//「もどる」の位置から「あ～お」ラインに復帰した場合はX位置を戻す補正
	if(oldpos == 5 && wk->listpos < 5){
        wk->listpos = wk->listpos_backup_x;
    }

	// 移動時はアクターを反映させる
	if(oldpos!=wk->listpos){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos==5){
			WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(word_cur_table[10][0]+16)*8,
						 word_cur_table[10][1]*8 );
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(word_cur_table[wk->listpos][0]+16)*8,
						 word_cur_table[wk->listpos][1]*8 );
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 47 );
		}

	}

	// 決定・キャンセル
	{
		u32 ret;
		u32 decide;
		// タッチ処理
		ret = TouchPanelFunc( wk, MODE_HEADWORD_2 );
		if(ret!=GFL_UI_TP_HIT_NONE){
			decide = Head2DecideFunc( wk, ret );
			if(decide == BMPMENU_NULL){ //ヤ行の端2文字分の空白をタッチした時のみの処理
                return decide;
            }
			if(decide == BMPMENU_CANCEL || see_check == NULL || (see_check[head2pokename[wk->Head1] + decide])){
    			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
	    		return decide;
	    	}
		}else{
		// キー処理
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				decide = Head2DecideFunc( wk, wk->listpos );
    			if(decide == BMPMENU_NULL){ //ヤ行の端2文字分の空白をタッチした時のみの処理
                    return decide;
                }
    			if(decide == BMPMENU_CANCEL || see_check == NULL || (see_check[head2pokename[wk->Head1] + decide])){
	    			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		    		return decide;
		    	}
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}
	return BMPMENU_NULL;
}



#define POKENAME_1PAGE_MAX	( 4 )
#define LEVEL_1PAGE_MAX	( 4 )
#define NATION_1PAGE_MAX	( 5 )

//------------------------------------------------------------------
/**
 * @brief   その総数で４項目ずつだと何ページになる？
 *
 * @param   num			項目総数
 * @param   in_page		１ページ何項目表示可能か？
 *
 * @retval  int			ページ数	1～	
 */
//------------------------------------------------------------------
static int _list_page_num( int num, int in_page)
{
	int page;
	if (num == 0){
		return 1;
	}
	page = (num+(in_page-1))/in_page;

	return page;
}

//------------------------------------------------------------------
/**
 * @brief   名前入力決定処理
 *
 * @param   wk		
 * @param   decide		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 NameDecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide )
{
	switch(decide){
	case 4:
		return BMPMENU_CANCEL;
		break;
	case 5:
		NamePageRefresh( wk, -1 );
		return BMPMENU_NULL;
		break;
	case 6:
		NamePageRefresh( wk, +1 );
		return BMPMENU_NULL;
		break;
	}

	// 項目決定
	{
		int namepos = decide+wk->page*POKENAME_1PAGE_MAX;
		// 名前のある場所なら入力決定
		if(namepos<wk->listMax){
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			return wk->NameList[namepos].param;
		}
	}

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン名選択ページ切り替え処理
 *
 * @param   wk		
 * @param   move	-1 or +1
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void NamePageRefresh( WORLDTRADE_INPUT_WORK *wk, int move )
{
	int page_no = _list_page_num( wk->listMax, MODE_POKENAME_WIN_NUM ) - 1;

	if (page_no == 0){
		return;
	}

	if(move<0){
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//左アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[1], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[1], CELL_BOXARROW_NO+1 );
		if(wk->page!=0){
			wk->page--;	
		}else{
			wk->page = page_no;
		}
	}else{
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//右アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[0], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[0], CELL_BOXARROW_NO );
		if(wk->page < page_no){
			wk->page++;
		}else{
			wk->page = 0;
		}
	}

	PokeName_ListPrint( wk, wk->page, wk->listMax );
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_POKENAME_WIN_NUM], 
				wk->page,
				_list_page_num( wk->listMax, MODE_POKENAME_WIN_NUM) );

}

//------------------------------------------------------------------
/**
 * @brief   名前選択メイン
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 PokeName_SelectMain( WORLDTRADE_INPUT_WORK *wk )
{
	int oldpos = wk->listpos;
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		if(wk->listpos!=0){
			wk->listpos--;
		}else{
			wk->listpos=4;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		if(wk->listpos!=4){
			wk->listpos++;
		}else{
			wk->listpos=0;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		NamePageRefresh( wk, -1 );
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		NamePageRefresh( wk, +1 );
	}

	if(oldpos!=wk->listpos){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
					namepostable[wk->listpos][0],namepostable[wk->listpos][1] );
		if(wk->listpos==4){
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
		}
	}

	{
		u32 ret;
		// タッチ決定
		ret = TouchPanelFunc( wk, MODE_POKEMON_NAME );
		if(ret!=GFL_UI_TP_HIT_NONE){
			return NameDecideFunc( wk, ret );
		}else{
		// キー決定
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				return NameDecideFunc( wk, wk->listpos );
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}
	return BMPMENU_NULL;
}


static const u32 s_ret_table[]={
	PARA_UNK,
    PARA_MALE,
    PARA_FEMALE,
    BMPMENU_CANCEL,
};
//------------------------------------------------------------------
/**
 * @brief   性別条件入力コントロール
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 SexSelect_SelectMain( WORLDTRADE_INPUT_WORK *wk )
{
	int oldpos = wk->listpos;
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=0){
			wk->listpos--;
		}else{
			wk->listpos=3;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=3){
			wk->listpos++;
		}else{
			wk->listpos=0;
		}
	}

	if(oldpos!=wk->listpos){
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
					sexpostable[wk->listpos][0],sexpostable[wk->listpos][1] );
		if(wk->listpos==3){
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
		}
	}


	{
		u32 ret;
		// タッチ決定
		ret = TouchPanelFunc( wk, MODE_SEX );
		if(ret!=GFL_UI_TP_HIT_NONE){
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			return s_ret_table[ret];
		}else{
		// キー決定
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return s_ret_table[wk->listpos];
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   レベル選択のページ切り替え
 *
 * @param   wk		
 * @param   move	-1 or +1
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void LevelPageRefresh( WORLDTRADE_INPUT_WORK *wk, int move )
{
	if(move<0){
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//左アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[1], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[1], CELL_BOXARROW_NO+1 );
		if(wk->page!=0){
			wk->page--;
		}else{
			wk->page = LV_PAGE_MAX;
		}
	}else{
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//右アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[0], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[0], CELL_BOXARROW_NO );
		if(wk->page < LV_PAGE_MAX){
			wk->page++;
		}else{
			wk->page = 0;
		}
	}
	PokeName_ListPrint( wk, wk->page, wk->listMax );
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_LEVEL_WIN_NUM], 
				wk->page,
				_list_page_num( wk->listMax, MODE_LEVEL_WIN_NUM ) );

}

//------------------------------------------------------------------
/**
 * @brief   レベル選択決定処理
 *
 * @param   wk		
 * @param   decide		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 LevelDecideFunc( WORLDTRADE_INPUT_WORK *wk, u32 decide )
{
	switch(decide){
	case 4:
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		return BMPMENU_CANCEL;
		break;
	case 5:
		LevelPageRefresh( wk, -1 );
		return BMPMENU_NULL;
		break;
	case 6:
		LevelPageRefresh( wk, +1 );
		return BMPMENU_NULL;
		break;
	}
	// 項目決定
	{
		int pos = decide+wk->page*LEVEL_1PAGE_MAX;
		// 名前のある場所なら入力決定
		if(pos<wk->listMax){
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			return wk->NameList[decide].param;
		}
	}
	
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   レベル条件入力コントロール
 *
 * @param   wk		
 *
 * @retval  u32 	
 */
//------------------------------------------------------------------
static u32 LevelSelect_SelectMain( WORLDTRADE_INPUT_WORK *wk )
{
	int oldpos = wk->listpos;
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=0){
			wk->listpos--;
		}else{
			wk->listpos=4;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=4){
			wk->listpos++;
		}else{
			wk->listpos=0;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		LevelPageRefresh( wk, -1 );
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		LevelPageRefresh( wk, +1 );
	}

	if(oldpos!=wk->listpos){
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
					levelpostable[wk->listpos][0],levelpostable[wk->listpos][1] );
		if(wk->listpos==4){
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
		}
	}

	{
		u32 ret;
		// タッチ決定
		ret = TouchPanelFunc( wk, MODE_LEVEL );
		if(ret!=GFL_UI_TP_HIT_NONE){
			return LevelDecideFunc( wk, ret );
		}else{
		// キー決定
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				return LevelDecideFunc( wk, wk->listpos );
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}

	return BMPMENU_NULL;
	
}


//------------------------------------------------------------------
/**
 * @brief   国名選択頭文字１の決定処理
 *
 * @param   wk		
 * @param   decide		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 NationHead1DecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide )
{
	switch(decide){
	case 10:	
		return BMPMENU_CANCEL;	
		break;
	case 11:	
		return 11;				
		break;
	}

	return decide;
}

//------------------------------------------------------------------
/**
 * @brief   頭文字選択メイン
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 NationHead1_SelectMain( WORLDTRADE_INPUT_WORK *wk, u8 *see_check )
{
	int oldpos = wk->listpos;
	
	if(oldpos < 10){    //あ～わ
	    wk->listpos_backup_x = oldpos;  //X位置を取っておく
	}
        
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		wk->listpos = n_head1table[wk->listpos][0];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		wk->listpos = n_head1table[wk->listpos][1];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		wk->listpos = n_head1table[wk->listpos][2];
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		wk->listpos = n_head1table[wk->listpos][3];
	}
	
	//「もどるorきにしない」の位置から「あ～わ」ラインに復帰した場合はX位置を戻す補正
	if(oldpos >= 10 && wk->listpos < 10){
        switch(oldpos){
        case 10:    //もどる
            if(wk->listpos_backup_x > 4){
                wk->listpos = wk->listpos_backup_x;
            }
            else{
                wk->listpos = wk->listpos_backup_x + 5;
            }
            wk->listpos_backup_x = wk->listpos;
            break;
        case 11:    //きにしない
            if(wk->listpos_backup_x > 4){
                wk->listpos = wk->listpos_backup_x - 5;
            }
            else{
                wk->listpos = wk->listpos_backup_x;
            }
            wk->listpos_backup_x = wk->listpos;
            break;
        }
    }

	if(oldpos!=wk->listpos){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(n_word_table[wk->listpos][0]+16)*8,
						 n_word_table[wk->listpos][1]*8 );
		switch(wk->listpos){
		case 10:	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );	break;
		case 11:	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );	break;
		default:	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 47 );	
		}

	}


	{
		u32 ret, decide;
		// タッチ決定
		ret = TouchPanelFunc( wk, MODE_NATION_HEAD1 );
		if(ret!=GFL_UI_TP_HIT_NONE){
            decide = NationHead1DecideFunc( wk, ret );
            if(decide == BMPMENU_CANCEL || decide == 11 || see_check == NULL || see_check[decide]){
    			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
	    		return decide;
	    	}
		}else{
		// キー決定
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				decide = NationHead1DecideFunc( wk, wk->listpos );
                if(decide == BMPMENU_CANCEL || decide == 11 || see_check == NULL || see_check[decide]){
    				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
    				return decide;
    			}
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   国名選択決定処理
 *
 * @param   wk		
 * @param   decide	NationTouchTableの矩形から取得した番号
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 NationDecideFunc( WORLDTRADE_INPUT_WORK *wk, int decide )
{
	switch(decide){
	case 5:		// キャンセル
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		return BMPMENU_CANCEL;
		break;
	case 6:		// 左ページへ
		NationPageRefresh(wk, -1);
		return BMPMENU_NULL;
		break;
	case 7:		// 右ページへ
		NationPageRefresh(wk, +1);
		return BMPMENU_NULL;
		break;
	}

	// 矢印・キャンセル以外
	{
		int namepos = decide+wk->page*NATION_1PAGE_MAX;
		// 名前のある場所なら入力決定
		if(namepos<wk->listMax){
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			return wk->NameList[namepos].param;
		}
	}

	return BMPMENU_NULL;

}


//------------------------------------------------------------------
/**
 * @brief   国指定ページ変更処理
 *
 * @param   wk		
 * @param   move	-1 or +1
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void NationPageRefresh( WORLDTRADE_INPUT_WORK *wk, int move)
{
	int page_no = _list_page_num( wk->listMax, MODE_NATION_WIN_NUM ) -1;

	OS_Printf("page_no = %d\n",page_no);

	if (page_no == 0){
		return;
	}

	if(move<0){
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//左アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[1], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[1], CELL_BOXARROW_NO+1 );
		if(wk->page!=0){
			wk->page--;
		}else{
			wk->page = page_no;
		}
	}else{
		PMSND_PlaySE(WORLDTRADE_PAGE_SE);
		//右アローアニメ
		GFL_CLACT_WK_SetAutoAnmFlag( wk->ArrowAct[0], 1 );
		GFL_CLACT_WK_SetAnmSeq( wk->ArrowAct[0], CELL_BOXARROW_NO );
		if(wk->page < page_no){
			wk->page++;	
		}else{
			wk->page = 0;
		}
	}

	Nation_ListPrint( wk, wk->page, wk->listMax );
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_NATION_WIN_NUM], 
				wk->page,
				_list_page_num( wk->listMax, MODE_NATION_WIN_NUM ) );
	
}

//------------------------------------------------------------------
/**
 * @brief   国選択操作メイン
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 Nation_SelectMain( WORLDTRADE_INPUT_WORK *wk )
{
	int oldpos = wk->listpos;
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=0){
			wk->listpos--;
		}else{
			wk->listpos=5;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		if(wk->listpos!=5){
			wk->listpos++;
		}else{
			wk->listpos=0;
		}
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		NationPageRefresh( wk, -1);
	}else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		NationPageRefresh( wk, +1);
	}

	if(oldpos!=wk->listpos){
		WorldTrade_CLACT_PosChange( wk->CursorAct, 
					nationpostable[wk->listpos][0],nationpostable[wk->listpos][1] );
		if(wk->listpos==5){
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 48 );
		}else{
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 50 );
		}
	}

	{
		u32 ret;
		// タッチ決定
		ret = TouchPanelFunc( wk, MODE_NATION );
		if(ret!=GFL_UI_TP_HIT_NONE){
			return NationDecideFunc( wk, ret );
		}else{
		// キー決定
			if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
				return NationDecideFunc( wk, wk->listpos );
			}else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return BMPMENU_CANCEL;
			}
		}
	}

	return BMPMENU_NULL;
	
}

//------------------------------------------------------------------
/**
 * @brief   子音頭文字入力画面初期化のサブ処理
 *
 * @param   wk		
 * @param   type	MODE_HEAD1 or MODE_NATION_HEAD1
 * @param   x		スクリーン書き出し位置の初期位置x
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void HeadWord1_Init( WORLDTRADE_INPUT_WORK *wk, int type, int x )
{
	int scr_dat;
	u16 *scr;

	// デフォルトスクリーンを仮想面に読み込み
	
	if(type==MODE_NATION_HEAD1){	// 国選択の時
		scr_dat = NARC_worldtrade_search_name2_lz_nscr;
	}else{							//ポケモン選択の時
		scr_dat = NARC_worldtrade_search_name3_lz_nscr;
	}

	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, scr_dat, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );

	// BMPWIN確保
	select_bmpwin_add( wk, type );
//	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_sub_lz_nscr, 1 );

	// アカサタナハマヤラワBMPWIN確保
	{
		int i;
		for(i=0;i<MODE_HEADWORD1_WIN_NUM;i++){
//			SelectFrameBoxWrite( scr, word_table[i][0], word_table[i][1], SELECT_BOX_N_LETTER );
		}
	}
//	SelectFrameBoxWrite( scr, word_table[10][0]-1, word_table[10][1]-1, SELECT_BOX_END );

	// 文字列描画
	{
		int i;
		STRBUF *strbuf;
		PRINTSYS_LSB color;
		
        MI_CpuFill8(wk->see_check, TRUE, SEE_CHECK_MAX);
		for(i=0;i<MODE_HEADWORD1_WIN_NUM;i++){
			strbuf = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_10_001+i );
			if(type == MODE_HEADWORD_1){    //ポケモン検索
    			if(PokeSeeCount_Category(wk, i) == TRUE){
                    color = SEARCH_MOJI_COLOR_NORMAL;
                    wk->see_check[i] = TRUE;
                }
                else{
                    color =  SEARCH_MOJI_COLOR_GRAY;
                    wk->see_check[i] = FALSE;
                }
    		}
    		else{   //国検索
    		    if(NationCount_Category(wk, i) == TRUE){
        		    color = SEARCH_MOJI_COLOR_NORMAL;
        		    wk->see_check[i] = TRUE;
        		}
        		else{
                    color = SEARCH_MOJI_COLOR_GRAY;
        		    wk->see_check[i] = FALSE;
                }
            }
            
			WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[i], strbuf,  2, color, &wk->print );
			GFL_STR_DeleteBuffer(strbuf);
		}

	}

	// もどる
	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print );

	// 国選択の時は「きにしない」を追加する
	if(type==MODE_NATION_HEAD1){
		STRBUF *strbuf = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_search_015_01 );
//		SelectFrameBoxWrite( scr, 	word_table[BMPWIN_NONE_SELECT_WIN][0]-1, 
//									word_table[BMPWIN_NONE_SELECT_WIN][1]-1, SELECT_BOX_N_TEXT );
		WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[BMPWIN_NONE_SELECT_WIN], 
									strbuf,  2, PRINTSYS_LSB_Make(15,14,2), &wk->print );
		GFL_STR_DeleteBuffer(strbuf);
	}

	BGWINFRM_FramePut( wk->BgWinFrm, 0, x, 0 );
	
	BGWINFRM_FrameOn( wk->BgWinFrm, 0 );	

}

//------------------------------------------------------------------
/**
 * @brief   子音頭文字入力画面初期化のサブ処理
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SexSelect_Init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;

	// デフォルトスクリーンを仮想面に読み込み
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_seibetsu_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );

	// BMPWIN確保
	select_bmpwin_add( wk, MODE_SEX );

//	SelectFrameBoxWrite( scr, word_table[10][0]-1, word_table[10][1]-1, SELECT_BOX_END );

	// 文字列描画
	{
		int i;
		STRBUF *strbuf;
		for(i=0;i<MODE_SEX_WIN_NUM;i++){
			strbuf = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_11_001+i );
			WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[i], strbuf,  2, PRINTSYS_LSB_Make(15,14,2), &wk->print );
			GFL_STR_DeleteBuffer(strbuf);
		}
	}

	// もどる
	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print );

	BGWINFRM_FramePut( wk->BgWinFrm, 0, 32, 0 );
	BGWINFRM_FrameOn( wk->BgWinFrm, 0 );	

	BGWINFRM_MoveInit( wk->BgWinFrm, 0, -4, 0, 4 );

}

//------------------------------------------------------------------
/**
 * @brief   レベル条件選択BMPWIN確保
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void LevelSelect_Init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_level_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );

	// 子音頭文字入力用BMPWINの確保
	select_bmpwin_add( wk, MODE_LEVEL );

//	SelectFrameBoxWrite( scr, 9, 15, SELECT_BOX_END );
	
	if(wk->Situation==SITUATION_SEARCH){
		wk->listMax  = WorldTrade_LevelListAdd( &wk->NameList, wk->MsgManager, LEVEL_PRINT_TBL_SEARCH);
	}else if(wk->Situation==SITUATION_DEPOSIT){
		wk->listMax  = WorldTrade_LevelListAdd( &wk->NameList, wk->MsgManager, LEVEL_PRINT_TBL_DEPOSIT);
	}

	PokeName_ListPrint( wk, 0, wk->listMax );
	wk->page    = 0;
	
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_LEVEL_WIN_NUM], 
				wk->page, _list_page_num( wk->listMax, MODE_LEVEL_WIN_NUM ) );

	// もどる
	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print );


	BGWINFRM_FramePut( wk->BgWinFrm, 0, 32, 0 );
	GFL_CLACT_WK_SetAnmSeq(wk->CursorAct, 49);
	
	wk->listpos = 0;
	wk->listpos_backup_x = 0;
	
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
				namepostable[wk->listpos][0], namepostable[wk->listpos][1] );

	BGWINFRM_FrameOn( wk->BgWinFrm, 0 );	
	BGWINFRM_MoveInit( wk->BgWinFrm, 0, -4, 0, 4 );

	wk->seq  = WI_SEQ_LEVEL_MAIN;


}


//==============================================================================
/**
 * @brief   BMPWIN内の表示位置を指定してFONT_SYSTEMでプリント(CGX転送のみ）
 *
 * @param   win		
 * @param   strbuf		
 * @param   x		
 * @param   color		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_Input_SysPrint( BGWINFRM_WORK *wfwk, GFL_BMPWIN *win, STRBUF *strbuf, int x, PRINTSYS_LSB color, WT_PRINT *print )
{
	GF_STR_PrintColor( win, FONT_SYSTEM, strbuf, x, 0, MSG_NO_PUT, color,print);
	GFL_BMPWIN_TransVramCharacter( win );
	BGWINFRM_BmpWinOn( wfwk, 0, win );
}


static const u8 box_table[][9]={
	{ 16*0+7, 16*0+9, 16*0+10,  16*0+12, 16*0+8, 16*0+11,},
	{ 16*1+0, 16*1+2, 16*1+3,   16*1+5,  16*1+1, 16*1+4, },
	{ 16*1+6, 16*1+8, 16*1+12,  16*1+14, 16*1+7, 16*1+13, 16*1+9, 16*1+11, 16*1+10},
};

#define SUB_WIN_W			( 32 )
#define SELECT_TEXT_W		( 14 )
#define SELECT_NATION_W		( 28 )
#define SELECT_END_W		(  9 )

//------------------------------------------------------------------
/**
 * @brief   スクリーンデータは32x20で決めうちとして文字入力用加工
 *
 * @param   scr		スクリーンバッファ
 * @param   x		書き込み開始X座標
 * @param   y		書き込み開始Y座標
 * @param   type	書き込みボックスタイプ
 *
 * @retval  none	
 */
//------------------------------------------------------------------
static void SelectFrameBoxWrite( u16 *scr, int x, int y, int type )
{
	int index=0;
	int i;
	switch(type){

	// １文字分のボックス
	case SELECT_BOX_N_LETTER:	case SELECT_BOX_R_LETTER:
		// １文字分のボックス
		scr[  x+y*SUB_WIN_W]     = box_table[type][0]+0x1000;
		scr[1+x+y*SUB_WIN_W]     = box_table[type][1]+0x1000;
		scr[x+(y+1)*SUB_WIN_W]   = box_table[type][2]+0x1000;
		scr[1+x+(y+1)*SUB_WIN_W] = box_table[type][3]+0x1000;
		break;

	// 文字列表示のボックス
	case SELECT_BOX_N_TEXT:     case SELECT_BOX_R_TEXT:
		index = type-SELECT_BOX_N_TEXT;

		// 四隅を埋めて
		scr[  x+y*SUB_WIN_W]     			   = box_table[index][0]+0x1000;
		scr[SELECT_TEXT_W+x+y*SUB_WIN_W-1]     = box_table[index][1]+0x1000;
		scr[x+(y+1)*SUB_WIN_W]   			   = box_table[index][2]+0x1000;
		scr[SELECT_TEXT_W+x+(y+1)*SUB_WIN_W-1] = box_table[index][3]+0x1000;
		// 単語の長さ分横に埋める
		for(i=1;i<SELECT_TEXT_W-1;i++){
			scr[x+y*SUB_WIN_W+i]     = box_table[index][4]+0x1000;
			scr[x+(y+1)*SUB_WIN_W+i] = box_table[index][5]+0x1000;
			
		}
		break;

	// 国名表示のボックス
	case SELECT_BOX_N_NATION:	case SELECT_BOX_R_NATION:
		index = type - SELECT_BOX_N_NATION;
		// 四隅を埋めて
		scr[  x+y*SUB_WIN_W]     			     = box_table[index][0]+0x1000;
		scr[SELECT_NATION_W+x+y*SUB_WIN_W-1]     = box_table[index][1]+0x1000;
		scr[x+(y+1)*SUB_WIN_W]   			     = box_table[index][2]+0x1000;
		scr[SELECT_NATION_W+x+(y+1)*SUB_WIN_W-1] = box_table[index][3]+0x1000;
		// 単語の長さ分横に埋める
		for(i=1;i<SELECT_NATION_W-1;i++){
			scr[x+y*SUB_WIN_W+i]     = box_table[index][4]+0x1000;
			scr[x+(y+1)*SUB_WIN_W+i] = box_table[index][5]+0x1000;
			
		}
		break;

	// 戻るボタン
	case SELECT_BOX_END:
		index = 2;
		// 四隅
		scr[  x+y*SUB_WIN_W]     			   = box_table[index][0]+0x1000;
		scr[SELECT_END_W+x+y*SUB_WIN_W-1]      = box_table[index][1]+0x1000;
		scr[x+(y+3)*SUB_WIN_W]   			   = box_table[index][2]+0x1000;
		scr[SELECT_END_W+x+(y+3)*SUB_WIN_W-1]  = box_table[index][3]+0x1000;
		// 左端と右端の縦列
		scr[  x+(y+1)*SUB_WIN_W]     		   = box_table[index][6]+0x1000;
		scr[SELECT_END_W+x+(y+1)*SUB_WIN_W-1]  = box_table[index][7]+0x1000;
		scr[x+(y+2)*SUB_WIN_W]   			   = box_table[index][6]+0x1000;
		scr[SELECT_END_W+x+(y+2)*SUB_WIN_W-1]  = box_table[index][7]+0x1000;

		for(i=1;i<SELECT_END_W-1;i++){
			scr[x+y*SUB_WIN_W+i]    		 = box_table[index][4]+0x1000;
			scr[x+(y+3)*SUB_WIN_W+i]		 = box_table[index][5]+0x1000;
			scr[x+(y+1)*SUB_WIN_W+i]		 = box_table[index][8]+0x1000;
			scr[x+(y+2)*SUB_WIN_W+i]		 = box_table[index][8]+0x1000;
		}


		break;

	}
}


static int	wi_seq_none( WORLDTRADE_INPUT_WORK *wk )
{

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   入力ウインドウインを監視し次のシーケンスへ
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_winin( WORLDTRADE_INPUT_WORK *wk )
{
	// フレームイン待ち
	if(BGWINFRM_MoveCheck( wk->BgWinFrm, 0)==0){
		// カーソル表示開始
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1 );
		if(wk->type==MODE_LEVEL){
			WorldTrade_CLACT_PosChange( wk->ArrowAct[0], PAGE_ARROW2_X, PAGE_ARROW_Y);
			WorldTrade_CLACT_PosChange( wk->ArrowAct[1], PAGE_ARROW1_X, PAGE_ARROW_Y);
			GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 1 );
			GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 1 );
		}
		wk->seq  = wk->next;
		MORI_PRINT("input win in\n");
	}

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   入力ウインドウインを監視し次のシーケンスへ(影響なし）
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_winwait( WORLDTRADE_INPUT_WORK *wk )
{
	// フレームイン待ち
	if(BGWINFRM_MoveCheck( wk->BgWinFrm, 0)==0){
		wk->seq  = wk->next;
		MORI_PRINT("input win no effectin\n");
	}

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   入力ウインドウアウトを監視し、次のシーケンスへ
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_winout( WORLDTRADE_INPUT_WORK *wk )
{
	// フレームアウト待ち
	if(BGWINFRM_MoveCheck( wk->BgWinFrm, 0)==0){
		BGWINFRM_FrameOff( wk->BgWinFrm, 0 );
		MORI_PRINT("input win out\n");
		wk->seq  = wk->next;
	}
	return BMPMENU_NULL;

}

//------------------------------------------------------------------
/**
 * @brief   子音頭文字入力画面の初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head1_init( WORLDTRADE_INPUT_WORK *wk )
{
	HeadWord1_Init( wk, MODE_HEADWORD_1, HEAD1_MOVE_INIT_X );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 47 );
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(word_cur_table[0][0]+16)*8,
						word_cur_table[0][1]*8 );

	BGWINFRM_MoveInit( wk->BgWinFrm, 0, -4, 0, 4 );
	wk->seq   = WI_SEQ_WININ;
	wk->next  = WI_SEQ_HEAD1_MAIN;
	wk->Head1 = -1;

	MORI_PRINT("input head1 init\n");

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   子音頭文字入力画面メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head1_main( WORLDTRADE_INPUT_WORK *wk )
{
	int i;
	u32 ret;

	switch((ret=WordHead_SelectMain( wk, wk->see_check ))){
	case INPUT_HEAD_A:	case INPUT_HEAD_KA:	case INPUT_HEAD_SA:	case INPUT_HEAD_TA:	case INPUT_HEAD_NA:
	case INPUT_HEAD_HA:	case INPUT_HEAD_MA: case INPUT_HEAD_YA: case INPUT_HEAD_RA: case INPUT_HEAD_WA:
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );

		wk->seq  = WI_SEQ_HEAD1_EXIT;
		wk->Head1 = ret;
		// 入力を行う
		break;
	// キャンセル
	case BMPMENU_CANCEL:
//		BmpWinFrame_Clear( &wk->MsgWin, WINDOW_TRANS_ON );

		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		wk->seq  = WI_SEQ_WINOUT;
		wk->next = WI_SEQ_HEAD1_EXIT;
		wk->Head1 = -1;

		// キャンセルなので入力が無い
		break;

	}
	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   子音頭文字入力画面終了
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head1_exit( WORLDTRADE_INPUT_WORK *wk )
{
	// 子音頭文字入力用BMPWINの解放
	select_bmpwin_del( wk, MODE_HEADWORD_1 );

	// カーソル表示OFF
	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );

	// 入力しているかしていないかで分岐
	if(wk->Head1 >= 0){
		if(wk->Head1!=INPUT_HEAD_WA){
			wk->seq = WI_SEQ_HEAD2_INIT;
		}else{
			wk->Head2 = 0;
			wk->seq   = WI_SEQ_POKENAME_INIT;
		}
		MORI_PRINT("input head1 exit ---> input next Head2(Head1=%d) \n", wk->Head1);
	}else{
		MORI_PRINT("input head1 exit ---> input end (Head1=%d) \n", wk->Head1);
		return BMPMENU_CANCEL;
	}
	
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   HEAD2から戻ってきたときの初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head1_return( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_name3_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );
	select_bmpwin_add( wk, MODE_HEADWORD_1 );
	// アカサタナハマヤラワBMPWIN確保
	{
		int i;
		for(i=0;i<10;i++){
//			SelectFrameBoxWrite( scr, word_table[i][0], word_table[i][1], SELECT_BOX_N_LETTER );
		}
	}
//	SelectFrameBoxWrite( scr, 9, 15, SELECT_BOX_END );

	{
		int i;
		STRBUF *strbuf;
		PRINTSYS_LSB color;
		
        MI_CpuFill8(wk->see_check, TRUE, SEE_CHECK_MAX);
		for(i=0;i<MODE_HEADWORD1_WIN_NUM;i++){
			strbuf = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_10_001+i );
			if(PokeSeeCount_Category(wk, i) == TRUE){
                color = SEARCH_MOJI_COLOR_NORMAL;
                wk->see_check[i] = TRUE;
            }
            else{
                color = SEARCH_MOJI_COLOR_GRAY;
                wk->see_check[i] = FALSE;
            }

			WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[i], strbuf,  2, color, &wk->print );
			GFL_STR_DeleteBuffer(strbuf);
		}
	}

	// もどる
	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print );

	BGWINFRM_FramePut( wk->BgWinFrm, 0, HEAD1_REWRITE_INIT_X, 0 );
	BGWINFRM_FrameOn( wk->BgWinFrm, 0 );	

	GFL_CLACT_WK_SetAnmSeq(wk->CursorAct, 47);
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
						(word_cur_table[wk->Head1][0]+16)*8,
						word_cur_table[wk->Head1][1]*8 );
	wk->listpos = wk->Head1;
	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1 );

//	GFL_HEAP_FreeMemory( buf );
	wk->seq = WI_SEQ_HEAD1_MAIN;

	OS_Printf("input head1 return -----\n");
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   母音頭文字入力初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head2_init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_name1_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );
	
	select_bmpwin_add( wk, MODE_HEADWORD_2 );
	{
		int i;
		for(i=0;i<MODE_HEADWORD2_WIN_NUM;i++){
//			SelectFrameBoxWrite( scr, word2_table[i][0], word2_table[i][1], word2_table[i][2] );
		}
	}
//	SelectFrameBoxWrite( scr, 9, 15, SELECT_BOX_END );
	
	{
		int i;
    	int see_count;
        PRINTSYS_LSB color;
        
        MI_CpuFill8(wk->see_check, TRUE, SEE_CHECK_MAX);
		system_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[0], 
				  head_letter_table[head2_word_start[wk->Head1][0]], SEARCH_MOJI_COLOR_NORMAL, &wk->print );
		for(i=1;i<MODE_HEADWORD2_WIN_NUM;i++){
        	see_count = PokeSeeCount_Siin(wk, head2pokename[wk->Head1]+i-1);
            if(see_count > 0){
                color = SEARCH_MOJI_COLOR_NORMAL;
                wk->see_check[head2pokename[wk->Head1]+i-1] = TRUE;
            }
            else{
                color = SEARCH_MOJI_COLOR_GRAY;
                wk->see_check[head2pokename[wk->Head1]+i-1] = FALSE;
            }
			system_print( wk->BgWinFrm,wk->MsgManager, wk->MenuBmp[i], 
				  head_letter_table[head2_word_start[wk->Head1][0]+i-1], color, &wk->print);
		}
	}
	touch_print( wk->BgWinFrm,wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print);

	BGWINFRM_FramePut( wk->BgWinFrm, 0, HEAD1_REWRITE_INIT_X, 0 );
	GFL_CLACT_WK_SetAnmSeq(wk->CursorAct, 47);
	
	if(wk->Head2<0){
		wk->listpos = 0;
	}else{
		wk->listpos = wk->Head2;
	}
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
				(word_cur_table[wk->listpos][0]+16)*8,
				 word_cur_table[wk->listpos][1]*8 );
	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1 );

	wk->seq  = WI_SEQ_HEAD2_MAIN;

	MORI_PRINT("input head2 init\n");

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   母音選択メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head2_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	
	switch((ret=WordHead2_SelectMain( wk, wk->see_check ))){
	case 0:	case 1: case 2: case 3: case 4:
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );

		wk->seq  = WI_SEQ_HEAD2_EXIT;
		wk->Head2 = ret;
		break;
	case BMPMENU_CANCEL:
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		wk->seq  = WI_SEQ_HEAD2_EXIT;
//		wk->next  = WI_SEQ_HEAD1_RETURN;
		wk->Head2 = -1;
		break;
	}

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   母音選択処理BMP解放
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head2_exit( WORLDTRADE_INPUT_WORK *wk )
{
	// 子音頭文字入力用BMPWINの解放
	select_bmpwin_del( wk, MODE_HEADWORD_2 );

	// 入力しているかしていないかで分岐
	if(wk->Head2 >= 0){
		wk->seq = WI_SEQ_POKENAME_INIT;
		MORI_PRINT("input head2 exit ---> input return (Head2=%d) \n", wk->Head2);
	}else{
		wk->seq = WI_SEQ_HEAD1_RETURN;
		MORI_PRINT("input head1 return ---> input next Name(Head2=%d) \n", wk->Head2);
	}

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   母音選択処理復帰
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_head2_return( WORLDTRADE_INPUT_WORK *wk )
{

	wk->seq  = WI_SEQ_HEAD2_INIT;

	MORI_PRINT("input head2 return\n");
	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   指定のポケモン名は何対存在しているか？
 *
 * @param   sinou		
 * @param   zukan		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int PokeName_GetSortNum( u8 *sinou, ZUKAN_WORK *zukan, int pokenum, u16 *sortlist )
{
	int i, count;
	for(i=0,count=0;i<pokenum;i++){
		if(ZukanWork_GetPokeSeeFlag( zukan, sortlist[i] )){
			OS_Printf(" SeeCheck i = %d, table[index+i] = %d\n", i, sortlist[i]);
			count++;
		}
	}
	return count;
}
//------------------------------------------------------------------
/**
 * @brief   
 *
 * @param   list			ポケモンの名前を登録するBMPLIST_DATAのポインタ
 * @param   monsnameman		ポケモンの名前のMSGMAN
 * @param   msgman			GTSのMSGMAN
 * @param   sinou			シンオウずかんのテーブル
 * @param   num				取得する名前の頭文字
 * @param   zukan			ずかんワークへのポインタ
 *
 * @retval  int		登録した名前の数
 */
//------------------------------------------------------------------
static int PokeName_MakeSortList( BMPLIST_DATA **list, GFL_MSGDATA *monsnameman, 
									GFL_MSGDATA *msgman, u8 *sinou, int select, ZUKAN_WORK *zukan )
{
	int i,index,see_count=0;
	int pokenum;
	u16 *sortlist = WorldTrade_ZukanSortDataGet2( HEAPID_WORLDTRADE, select, &pokenum );


	OS_TPrintf("select = %d, num = %d\n",select, pokenum);
	
	// 図鑑と比べて何体見ているかを確認
	//	index = NameHeadTable[select];
	see_count = PokeName_GetSortNum( sinou, zukan, pokenum, sortlist );

	// 項目リストの作成
	*list = BmpMenuWork_ListCreate( see_count+1, HEAPID_WORLDTRADE );


	// ポケモン名の登録
	for(i=0;i<pokenum;i++){
		if(ZukanWork_GetPokeSeeFlag( zukan, sortlist[i] )){
			OS_Printf(" ListAdd i = %d, table[index+i] = %d\n", i, sortlist[i]);
			BmpMenuWork_ListAddArchiveString( *list, monsnameman, sortlist[i], sortlist[i], HEAPID_WORLDTRADE );
		}
	}

	OS_Printf("----------------------list_print\n");
	for(i=0;i<see_count;i++){
		OS_Printf("%02d: %d\n", i, (*list)[i].param);
	}

	GFL_HEAP_FreeMemory( sortlist );

	return see_count;
}




//------------------------------------------------------------------
/**
 * @brief   ポケモン名をページ毎に表示する
 *
 * @param   wk		WORLDTRADE_INPUT_WORK
 * @param   page	表示ページの指定
 * @param   max		ポケモン名の最大数
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void PokeName_ListPrint( WORLDTRADE_INPUT_WORK *wk, int page, int max )
{
	int i;
	
	for(i=0;i<POKENAME_1PAGE_MAX;i++){
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
		if(page*POKENAME_1PAGE_MAX+i<max){
			WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[i], 
									   (STRBUF*)wk->NameList[page*POKENAME_1PAGE_MAX+i].str,  
									   0, PRINTSYS_LSB_Make(15,14,2),&wk->print );
		}else{
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			BGWINFRM_BmpWinOn( wk->BgWinFrm, 0, wk->MenuBmp[i] );
		}
	}

}
//------------------------------------------------------------------
/**
 * @brief   国名を１ページ分表示する
 *
 * @param   wk		
 * @param   page		
 * @param   max		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void Nation_ListPrint( WORLDTRADE_INPUT_WORK *wk, int page, int max )
{
	int i;
	for(i=0;i<NATION_1PAGE_MAX;i++){
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuBmp[i]), INPUTPLATE_BASE_COLOR_DATA );
		if(page*NATION_1PAGE_MAX+i<max){
			WorldTrade_Input_SysPrint( wk->BgWinFrm, wk->MenuBmp[i], 
									   (STRBUF*)wk->NameList[page*NATION_1PAGE_MAX+i].str,  
									   0, PRINTSYS_LSB_Make(15,14,2),&wk->print );
		}else{
			GFL_BMPWIN_TransVramCharacter( wk->MenuBmp[i] );
			BGWINFRM_BmpWinOn( wk->BgWinFrm, 0, wk->MenuBmp[i] );
		}
	}
	
}




//------------------------------------------------------------------
/**
 * @brief   ポケモン名選択画面初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_pokename_init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_level_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );

	// 子音頭文字入力用BMPWINの確保
	select_bmpwin_add( wk, MODE_POKEMON_NAME );

//	SelectFrameBoxWrite( scr, 9, 15, SELECT_BOX_END );
	
	// ポケモン名リストの作成
	wk->listMax = PokeName_MakeSortList( &wk->NameList, wk->MonsNameManager, wk->MsgManager, 
							wk->SinouTable, head2pokename[wk->Head1]+wk->Head2, wk->zukan );
	wk->page = 0;

	// リスト表示
	PokeName_ListPrint( wk, 0, wk->listMax );

	// ページ数表示
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_POKENAME_WIN_NUM], 
				wk->page, _list_page_num( wk->listMax, MODE_POKENAME_WIN_NUM ) );

	// もどる表示
	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print );


	// スクリーン反映
	BGWINFRM_FramePut( wk->BgWinFrm, 0, HEAD1_REWRITE_INIT_X, 0 );

	// カーソル表示
	GFL_CLACT_WK_SetAnmSeq(wk->CursorAct, 49);
	
	wk->listpos = 0;
	
	// カーソル位置移動・左右移動カーソル表示
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
				namepostable[wk->listpos][0], namepostable[wk->listpos][1] );
	WorldTrade_CLACT_PosChange( wk->ArrowAct[0], POKE_ARROW2_X, POKE_ARROW_Y);
	WorldTrade_CLACT_PosChange( wk->ArrowAct[1], POKE_ARROW1_X, POKE_ARROW_Y);

	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1 );
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 1 );
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 1 );

	wk->seq  = WI_SEQ_POKENAME_MAIN;



	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   ポケモン名選択
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_pokename_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	switch((ret=PokeName_SelectMain( wk ))){
	case BMPMENU_CANCEL:
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		wk->seq  = WI_SEQ_POKENAME_CANCEL_EXIT;
		wk->Poke = -1;
		break;
	case BMPMENU_NULL:
		break;
	default:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		// カーソル表示OFF
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );

		wk->seq  = WI_SEQ_WINOUT;
		wk->next = WI_SEQ_POKENAME_EXIT;
		wk->Poke = ret;
		break;
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   名前選択をキャンセルで抜ける
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_pokename_cancel_exit( WORLDTRADE_INPUT_WORK *wk )
{
	MORI_PRINT("input pokename exit ---> input next Name(Head2=%d) \n", wk->Head2);
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );
	select_bmpwin_del( wk, MODE_POKEMON_NAME );
	// ポケモン名リストの解放
	BmpMenuWork_ListDelete( wk->NameList );

	if(wk->Head1!=INPUT_HEAD_WA){
		wk->seq = WI_SEQ_HEAD2_RETURN;
	}else{
		wk->seq = WI_SEQ_HEAD1_RETURN;
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   名前選択が行われたので、ウインドウを隠して終了
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int wi_seq_pokename_exit( WORLDTRADE_INPUT_WORK *wk )
{
	int ret=BMPMENU_NULL;

	select_bmpwin_del( wk, MODE_POKEMON_NAME );
	// ポケモン名リストの解放
	BmpMenuWork_ListDelete( wk->NameList );

	return wk->Poke;
}

//------------------------------------------------------------------
/**
 * @brief   国選択頭文字指定
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head1_init( WORLDTRADE_INPUT_WORK *wk )
{
	HeadWord1_Init( wk, MODE_NATION_HEAD1, HEAD1_MOVE_INIT_X );
	GFL_CLACT_WK_SetAnmSeq(   wk->CursorAct, 47 );


	BGWINFRM_MoveInit( wk->BgWinFrm, 0, -4, 0, 4 );
	wk->seq     = WI_SEQ_WININ;
	wk->next    = WI_SEQ_NATION_HEAD1_MAIN;
	wk->listpos = INPUT_HEAD_NONE;
	wk->Head1   = -1;

	WorldTrade_CLACT_PosChange( wk->CursorAct, 
					(n_word_table[wk->listpos][0]+16)*8,
					 n_word_table[wk->listpos][1]*8 );
    GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
    
	MORI_PRINT("input nation head1 init\n");


	return BMPMENU_NULL;
}
//------------------------------------------------------------------
/**
 * @brief   国選択１文字目入力メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head1_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	switch((ret=NationHead1_SelectMain(wk, wk->see_check))){
	case BMPMENU_NULL:
		break;
	case BMPMENU_CANCEL:	// キャンセル
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		// カーソル表示OFF
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		wk->seq   = WI_SEQ_WINOUT;
		wk->next  = WI_SEQ_NATION_HEAD1_EXIT;
		wk->Head1 = -1;
		break;
	case INPUT_HEAD_NONE:	// 「きにしない」を入力した
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		// カーソル表示OFF
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		wk->seq   = WI_SEQ_WINOUT;
		wk->next  = WI_SEQ_NATION_HEAD1_EXIT;
		wk->Head1 = -2;
		break;
	case INPUT_HEAD_A:	case INPUT_HEAD_KA:	case INPUT_HEAD_SA:	case INPUT_HEAD_TA:	case INPUT_HEAD_NA:
	case INPUT_HEAD_HA:	case INPUT_HEAD_MA: case INPUT_HEAD_YA: case INPUT_HEAD_RA: case INPUT_HEAD_WA:
	default:				// 通常の入力
		wk->Head1 = ret;
		wk->seq   = WI_SEQ_NATION_HEAD1_EXIT;
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   国名入力頭文字１終了
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head1_exit( WORLDTRADE_INPUT_WORK *wk )
{
	select_bmpwin_del( wk, MODE_NATION_HEAD1 );
	GFL_CLACT_WK_SetDrawEnable(   wk->CursorAct, 0 );
	
	if(wk->Head1<0){
		if(wk->Head1==-1){
			return BMPMENU_CANCEL;	// キャンセル
		}else if(wk->Head1==-2){
			return 0;				// 「きにしない」を選んだ
		}
	}else{
		if(wk->Head1!=INPUT_HEAD_WA){
			wk->seq = WI_SEQ_NATION_HEAD2_INIT;
		}else{
			wk->Head2 = 0;
			BGWINFRM_MoveInit( wk->BgWinFrm, 0,  -4, 0, 3 );
			// 検索画面のメインカーソルを隠す
			if(wk->SearchCursorAct!=NULL){
				GFL_CLACT_WK_SetDrawEnable( wk->SearchCursorAct, 0 );
			}
			GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 50 );
			wk->seq  = WI_SEQ_WINWAIT;
			wk->next = WI_SEQ_NATION_INIT;
		}
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   国名入力頭文字１の復帰
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head1_return( WORLDTRADE_INPUT_WORK *wk )
{
	HeadWord1_Init( wk, MODE_NATION_HEAD1, HEAD1_REWRITE_INIT_X );
	GFL_CLACT_WK_SetAnmSeq(   wk->CursorAct, 47 );
	GFL_CLACT_WK_SetDrawEnable(   wk->CursorAct, 1 );
	wk->listpos = wk->Head1;
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
					(n_word_table[wk->listpos][0]+16)*8,
					 n_word_table[wk->listpos][1]*8 );

	//イニットに戻らないからここで検索画面のメインカーソル表示
	if(wk->SearchCursorAct!=NULL){
		GFL_CLACT_WK_SetDrawEnable( wk->SearchCursorAct, 1 );
	}

	wk->seq   = WI_SEQ_NATION_HEAD1_MAIN;
	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   国選択２文字目の選択画面初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head2_init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_name1_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );
	select_bmpwin_add( wk, MODE_HEADWORD_2 );
	{
		int i;
		for(i=0;i<MODE_HEADWORD2_WIN_NUM;i++){
//			SelectFrameBoxWrite( scr, word2_table[i][0], word2_table[i][1], word2_table[i][2] );
		}
	}
//	SelectFrameBoxWrite( scr, 9, 15, SELECT_BOX_END );
	
	{
		int i;
        MI_CpuFill8(wk->see_check, TRUE, SEE_CHECK_MAX);
		system_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[0], 
				   head_letter_table[head2_word_start[wk->Head1][0]], SEARCH_MOJI_COLOR_NORMAL, &wk->print );
		for(i=1;i<MODE_HEADWORD2_WIN_NUM;i++){
            int nation_num;
            PRINTSYS_LSB color;
            nation_num = NationCount_Siin(wk, head2pokename[wk->Head1]+i-1);
            if(nation_num > 0){
                color = SEARCH_MOJI_COLOR_NORMAL;
                wk->see_check[head2pokename[wk->Head1]+i-1] = TRUE;
            }
            else{
                color = SEARCH_MOJI_COLOR_GRAY;
                wk->see_check[head2pokename[wk->Head1]+i-1] = FALSE;
            }
			system_print( wk->BgWinFrm,wk->MsgManager, wk->MenuBmp[i], 
				  head_letter_table[head2_word_start[wk->Head1][0]+i-1], color, &wk->print);
		}


	}
	touch_print( wk->BgWinFrm,wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009, &wk->print);

	BGWINFRM_FramePut( wk->BgWinFrm, 0, HEAD1_REWRITE_INIT_X, 0 );
	GFL_CLACT_WK_SetAnmSeq(wk->CursorAct, 47);
	
	if(wk->Head2<0){
		wk->listpos = 0;
	}else{
		wk->listpos = wk->Head2;
	}
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
				(word_cur_table[wk->listpos][0]+16)*8,
				 word_cur_table[wk->listpos][1]*8 );
	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1 );
	if(wk->SearchCursorAct!=NULL){
		GFL_CLACT_WK_SetDrawEnable( wk->SearchCursorAct, 1 );
	}

	wk->seq    = WI_SEQ_NATION_HEAD2_MAIN;

	MORI_PRINT("input head2 init\n");

	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   国名入力頭文字２メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head2_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	switch((ret=WordHead2_SelectMain( wk, wk->see_check ))){
	case 0:	case 1: case 2: case 3: case 4:
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );

		wk->seq  = WI_SEQ_NATION_HEAD2_EXIT;
		wk->Head2 = ret;
		break;
	case BMPMENU_CANCEL:
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		wk->seq  = WI_SEQ_NATION_HEAD2_EXIT;
		wk->Head2 = -1;
		break;
	}

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   頭文字２つ目の終了処理(頭文字１に戻るか国名入力か分岐）
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head2_exit( WORLDTRADE_INPUT_WORK *wk )
{
	select_bmpwin_del( wk, MODE_HEADWORD_2 );
	GFL_CLACT_WK_SetDrawEnable(   wk->CursorAct, 0 );
	
	if(wk->Head2<0){
		wk->seq = WI_SEQ_NATION_HEAD1_RETURN;
	}else{
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  -4, 0, 3 );
		// 検索画面のメインカーソルを隠す
		if(wk->SearchCursorAct!=NULL){
			GFL_CLACT_WK_SetDrawEnable( wk->SearchCursorAct, 0 );
		}
		GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 50 );
		wk->seq  = WI_SEQ_WINWAIT;
		wk->next = WI_SEQ_NATION_INIT;
	}
	return BMPMENU_NULL;
	
	
}
//------------------------------------------------------------------
/**
 * @brief   
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_head2_return( WORLDTRADE_INPUT_WORK *wk )
{
	wk->seq     = WI_SEQ_NATION_HEAD2_INIT;
	wk->listpos = wk->Head2;
	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   国名入力初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_init( WORLDTRADE_INPUT_WORK *wk )
{
	u16 *scr;
	BGWINFRM_FrameSetArc( wk->BgWinFrm, 0, ARCID_WORLDTRADE_GRA, NARC_worldtrade_search_sub_lz_nscr, 1 );
	scr = BGWINFRM_FrameBufGet( wk->BgWinFrm, 0 );

	// 子音頭文字入力用BMPWINの確保
	select_bmpwin_add( wk, MODE_NATION );

//	SelectFrameBoxWrite( scr, 22, 15, SELECT_BOX_END );
	
	wk->listMax = WorldTrade_NationSortListMake( &wk->NameList, wk->CountryNameManager, 
												 head2pokename[wk->Head1]+wk->Head2 );
	Nation_ListPrint( wk, 0, wk->listMax );
	wk->page    = 0;
	PagePrint(  wk->BgWinFrm, wk->NumFontSys, wk->MenuBmp[MODE_NATION_WIN_NUM], 
				wk->page, _list_page_num( wk->listMax, MODE_NATION_WIN_NUM ) );

	touch_print( wk->BgWinFrm, wk->MsgManager, wk->MenuBmp[BMPWIN_SELECT_END_WIN], msg_gtc_03_009,&wk->print );

	BGWINFRM_FramePut( wk->BgWinFrm, 0, 1, 0 );
	
	wk->listpos = 0;
	
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
				nationpostable[wk->listpos][0], nationpostable[wk->listpos][1] );

	WorldTrade_CLACT_PosChange( wk->ArrowAct[0], NATION_PAGE_ARROW2_X, NATION_PAGE_ARROW_Y);
	WorldTrade_CLACT_PosChange( wk->ArrowAct[1], NATION_PAGE_ARROW1_X, NATION_PAGE_ARROW_Y);

	GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 1   );
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 1 );
	GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 1 );

	wk->seq  = WI_SEQ_NATION_MAIN;
	

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   国名入力メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	switch((ret=Nation_SelectMain(wk))){
	case BMPMENU_CANCEL:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,   4, 0, 4 );
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );
		wk->seq  = WI_SEQ_WINWAIT;
		wk->next = WI_SEQ_NATION_CANCEL_EXIT;
		break;
	case BMPMENU_NULL:
		break;
	default:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,   6, 0, 6 );
		// 検索画面のメインカーソルを隠す
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );

		wk->seq    = WI_SEQ_WINOUT;
		wk->next   = WI_SEQ_NATION_EXIT;
		wk->Nation = ret;
		break;
	}

	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   国名入力キャンセル時（頭文字２に戻る）
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_cancel_exit( WORLDTRADE_INPUT_WORK *wk )
{
	select_bmpwin_del( wk, MODE_NATION );

	// 国名リストの解放
	BmpMenuWork_ListDelete( wk->NameList );

	if(wk->Head1!=INPUT_HEAD_WA){
		wk->seq = WI_SEQ_NATION_HEAD2_RETURN;
	}else{
		wk->seq = WI_SEQ_NATION_HEAD1_RETURN;
	}
	return BMPMENU_NULL;
}


//------------------------------------------------------------------
/**
 * @brief   入力終了
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_nation_exit( WORLDTRADE_INPUT_WORK *wk )
{
	select_bmpwin_del( wk, MODE_NATION );

	// 検索画面のメインカーソルを表示ON
	if(wk->SearchCursorAct!=NULL){
		GFL_CLACT_WK_SetDrawEnable( wk->SearchCursorAct, 1 );
	}
	// 国名リストの解放
	BmpMenuWork_ListDelete( wk->NameList );

	OS_Printf("result nation = %d\n", wk->Nation);	
	return wk->Nation;
	
}

//------------------------------------------------------------------
/**
 * @brief   性別条件入力画面初期化
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int	wi_seq_sex_init( WORLDTRADE_INPUT_WORK *wk )
{
	wk->listpos = 0;

	SexSelect_Init( wk );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
								sexpostable[wk->listpos][0],sexpostable[wk->listpos][1] );

	wk->seq     = WI_SEQ_WININ;
	wk->next    = WI_SEQ_SEX_MAIN;

	MORI_PRINT("input sex init\n");
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   性別条件入力画面メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_sex_main( WORLDTRADE_INPUT_WORK *wk )
{
	u32 ret;
	switch((ret=SexSelect_SelectMain( wk ))){
	case BMPMENU_CANCEL:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		wk->seq  = WI_SEQ_WINOUT;
		wk->next = WI_SEQ_SEX_EXIT;
		wk->Sex  = BMPMENU_CANCEL;
		break;
	case BMPMENU_NULL:
		break;
	default:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		// カーソル表示OFF
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );

		wk->seq  = WI_SEQ_WINOUT;
		wk->next = WI_SEQ_SEX_EXIT;
		wk->Sex  = ret;
		break;
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   性別条件入力終了
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_sex_exit( WORLDTRADE_INPUT_WORK *wk )
{
	select_bmpwin_del( wk, MODE_SEX );
//	BmpMenuWork_ListDelete( wk->NameList );

	return wk->Sex;

}

//------------------------------------------------------------------
/**
 * @brief   レベル条件入力初期化
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_level_init( WORLDTRADE_INPUT_WORK *wk )
{
	wk->listpos = 0;

	LevelSelect_Init( wk );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorAct, 49 );
	WorldTrade_CLACT_PosChange( wk->CursorAct, 
								levelpostable[wk->listpos][0],levelpostable[wk->listpos][1] );

	wk->seq     = WI_SEQ_WININ;
	wk->next    = WI_SEQ_LEVEL_MAIN;


	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   レベル条件入力メイン
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_level_main( WORLDTRADE_INPUT_WORK *wk )
{

	u32 ret;
	switch((ret=LevelSelect_SelectMain( wk ))){
	case BMPMENU_CANCEL:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );
		wk->seq   = WI_SEQ_WINOUT;
		wk->next  = WI_SEQ_LEVEL_EXIT;
		wk->Level = BMPMENU_CANCEL;
		break;
	case BMPMENU_NULL:
		break;
	default:
		BGWINFRM_MoveInit( wk->BgWinFrm, 0,  4, 0, 4 );
		// カーソル表示OFF
		GFL_CLACT_WK_SetDrawEnable( wk->CursorAct, 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[0], 0 );
		GFL_CLACT_WK_SetDrawEnable( wk->ArrowAct[1], 0 );

		wk->seq   = WI_SEQ_WINOUT;
		wk->next  = WI_SEQ_LEVEL_EXIT;
		wk->Level = wk->NameList[ret+wk->page*MODE_LEVEL_WIN_NUM].param;
		break;
	}
	return BMPMENU_NULL;
}

//------------------------------------------------------------------
/**
 * @brief   レベル条件入力終了
 *
 * @param   wk		
 *
 * @retval  int			
 */
//------------------------------------------------------------------
static int	wi_seq_level_exit( WORLDTRADE_INPUT_WORK *wk )
{

	select_bmpwin_del( wk, MODE_LEVEL );
	BmpMenuWork_ListDelete( wk->NameList );

	return wk->Level;
}


//------------------------------------------------------------------
/**
 * @brief   単語を表示するためだけのプリント
 *
 * @param   win		
 * @param   mesno		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void system_print( BGWINFRM_WORK *wfwk, GFL_MSGDATA *mm, GFL_BMPWIN *win, int mesno, PRINTSYS_LSB color, WT_PRINT *print )
{
	STRBUF *strbuf;

	strbuf = GFL_MSG_CreateString( mm, mesno );
//	WorldTrade_SysPrint( win, strbuf,  2, 0, 0, PRINTSYS_LSB_Make(15,2,0) );
	WorldTrade_Input_SysPrint( wfwk, win, strbuf,  2, color, print );
	GFL_STR_DeleteBuffer(strbuf);
	
}
//------------------------------------------------------------------
/**
 * @brief   単語を表示するためだけのプリント
 *
 * @param   win		
 * @param   mesno		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void touch_print( BGWINFRM_WORK *wfwk, GFL_MSGDATA *mm, GFL_BMPWIN *win, int mesno, WT_PRINT *print )
{
	STRBUF *strbuf;

	strbuf = GFL_MSG_CreateString( mm, mesno );

	GF_STR_PrintColor( win, FONT_TOUCH, strbuf, 0, 0, MSG_NO_PUT, PRINTSYS_LSB_Make(15,14,2),print);
	GFL_BMPWIN_TransVramCharacter( win );
	BGWINFRM_BmpWinOn( wfwk, 0, win );

	GFL_STR_DeleteBuffer(strbuf);
	
}


//------------------------------------------------------------------
/**
 * @brief   ページ数を描画
 *
 * @param   wfwk	BGFrameWorkd構造体
 * @param   win		BMPWIN
 * @param   page	現在のページ数	0～
 * @param   max		項目数最大数	1～
 *
 * @retval  none	
 */
//------------------------------------------------------------------
static void PagePrint( BGWINFRM_WORK *wfwk, NUMFONT *numfont, GFL_BMPWIN *win, int page, int max )
{
	// 「ページ数 ／ ページMAX」描画
	NUMFONT_WriteNumber( numfont, page+1, 1, NUMFONT_MODE_LEFT, win,  0, 0 );
	NUMFONT_WriteMark(   numfont, NUMFONT_MARK_SLASH, win, 8, 0 );
	NUMFONT_WriteNumber( numfont,  max, 1, NUMFONT_MODE_LEFT, win, 8*2, 0 );

	// BGFrameに反映
	GFL_BMPWIN_TransVramCharacter( win );
	BGWINFRM_BmpWinOn( wfwk, 0, win );


}

//==================================================================
/**
 * 子音指定でポケモンが何匹見たフラグが立っているかカウント
 *
 * @param   wk		
 *
 * @retval  int		見たフラグ数
 */
//==================================================================
static int PokeSeeCount_Siin(WORLDTRADE_INPUT_WORK *wk, int siin)
{
	int pokenum, see_count;
    u16 *sortlist;
    
	sortlist = WorldTrade_ZukanSortDataGet2( HEAPID_WORLDTRADE, siin, &pokenum );
	see_count = PokeName_GetSortNum( wk->SinouTable, wk->zukan, pokenum, sortlist );
    GFL_HEAP_FreeMemory(sortlist);
    
    OS_TPrintf("子音 %d 見たフラグ数=%d\n", siin, see_count);
    return see_count;
}

//--------------------------------------------------------------
/**
 * 「あかさたなはまやらわ」指定でポケモンが見たフラグが立っているか調べる
 *
 * @param   wk		
 * @param   select
 *
 * @retval  BOOL		TRUE:1匹以上いる。　FALSE:0匹
 */
//--------------------------------------------------------------
static BOOL PokeSeeCount_Category(WORLDTRADE_INPUT_WORK *wk, int select)
{
    int siin_start_index = 0;
    int i;
    
    siin_start_index = head2pokename[select];
    
    if(head2_word_start[select][1] > 0){
        for(i = 0; i < head2_word_start[select][1]; i++){
            if(PokeSeeCount_Siin(wk, siin_start_index + i) > 0){
                return TRUE;
            }
        }
    }
    else{
        if(PokeSeeCount_Siin(wk, siin_start_index) > 0){
            return TRUE;
        }
    }
    return FALSE;
}

//==================================================================
/**
 * 子音指定で国がいくつ存在するかカウント
 *
 * @param   wk		
 *
 * @retval  int		見たフラグ数
 */
//==================================================================
static int NationCount_Siin(WORLDTRADE_INPUT_WORK *wk, int siin)
{
    int nation_num, number, i;
    
    nation_num = WorldTrade_NationSortListNumGet( siin, &number );
    OS_TPrintf("国 子音 %d 見たフラグ数=%d\n", siin, nation_num);
    return nation_num;
}

//--------------------------------------------------------------
/**
 * 「あかさたなはまやらわ」指定で国がいくつ存在するか調べる
 *
 * @param   wk		
 * @param   select
 *
 * @retval  BOOL		TRUE:1国以上ある。　FALSE:0
 */
//--------------------------------------------------------------
static BOOL NationCount_Category(WORLDTRADE_INPUT_WORK *wk, int select)
{
    int siin_start_index = 0;
    int i;
    
    siin_start_index = head2pokename[select];
    
    if(head2_word_start[select][1] > 0){
        for(i = 0; i < head2_word_start[select][1]; i++){
            if(NationCount_Siin(wk, siin_start_index + i) > 0){
                return TRUE;
            }
        }
    }
    else{
        if(NationCount_Siin(wk, siin_start_index) > 0){
            return TRUE;
        }
    }
    return FALSE;
}
