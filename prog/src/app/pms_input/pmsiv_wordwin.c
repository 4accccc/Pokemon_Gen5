//============================================================================================
/**
	* @file	pmsiv_wordwin.c
	* @bfief	簡易会話入力画面（描画下請け：単語表示ウィンドウ）
	* @author	taya
	* @date	06.02.10
	*/
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system\main.h"
#include "system\pms_word.h"
#include "system\bmp_winframe.h"
#include "print\printsys.h"
#include "print\wordset.h"
#include "msg\msg_pms_category.h"
#include "message.naix"


#include "pms_input_prv.h"
#include "pms_input_view.h"
#include "pmsiv_tool.h"

//======================================================================
enum {
	INPUTAREA_PALTYPE_MAX = 10,

	STR_TMPBUF_LEN = 128,	// 表示前のテンポラリとして使用。最大文字数。
};

enum {
	CLEAR_CHARPOS = 0,

	WORDWIN_XORG = 3,
	WORDWIN_YORG = 0,
	WORDWIN_WIDTH = 26,
	WORDWIN_HEIGHT = 32,
	WORDWIN_CHARPOS = CLEAR_CHARPOS+1,

	WORDWIN_DISP_LINE_MAX = 8,
	WORDWIN_DISP_COL_MAX = 2,
	WORDWIN_DISP_WORD_MAX = WORDWIN_DISP_LINE_MAX * WORDWIN_DISP_COL_MAX,

	WORDWIN_WRITE_LINE_INIT = 0,
	WORDWIN_WRITE_LINE_MAX = 256,
	WORDWIN_WRITE_LINE_UNIT = 16, 
	WORDWIN_WRITE_LINE_ROUND_BORDER = WORDWIN_WRITE_LINE_MAX - WORDWIN_WRITE_LINE_UNIT, ///< 描画領域

  WORDWIN_WRITE_DECO_OX = 8*3,
	WORDWIN_WRITE_OX = 0,
	WORDWIN_WRITE_OY = 0,
	WORDWIN_WRITE_X_MARGIN = 112,
	WORDWIN_WRITE_Y_MARGIN = 24,
	WORDWIN_WRITE_PAGE_HEIGHT = WORDWIN_DISP_LINE_MAX * WORDWIN_WRITE_Y_MARGIN,


	WORDWIN_SCROLL_WAIT_UNIT = PMSI_FRAMES(1),


	// 折り返し前の一時的な文字列描画領域用
	WORD_TMPWIN_WIDTH = 12,
	WORD_TMPWIN_HEIGHT = 2,

	// カーソル描画位置
	CURSOR_WIDTH = 12*8,
	CURSOR_HEIGHT = 16,
	CURSOR_OX = (WORDWIN_XORG*8)+(CURSOR_WIDTH/2) - 8,
//	CURSOR_OY = (WORDWIN_YORG*8)+(WORDWIN_WRITE_LINE_INIT)+(CURSOR_HEIGHT/2),
	CURSOR_OY = 24,
	CURSOR_X_MARGIN = (CURSOR_WIDTH+16),
	CURSOR_Y_MARGIN = WORDWIN_WRITE_Y_MARGIN,

	CURSOR_BACK_XPOS = 28*8,
	CURSOR_BACK_YPOS = 22*8,
	
	ARROW_UP_X = 240,
	ARROW_UP_Y = 8*10,
	ARROW_DOWN_X = 240,
	ARROW_DOWN_Y = 16*8,

	SCRLL_BAR_X = 244,
	SCRLL_BAR_Y = 12,
};

enum {
	WORD_COL_LETTER = 0x01,
	WORD_COL_SHADOW = 0x02,
	WORD_COL_GROUND = 0x00,
};

#define WORDWIN_DECO_ACT_NUM (PMS_DECOID_MAX-1)

#define	WORDWIN_DEFAOULT_SCROLL_Y		( 8 )		// デフォルトのスクロール値


//======================================================================

//======================================================================
//--------------------------------------------------------------
/**
	*	
	*/
//--------------------------------------------------------------
struct _PMSIV_WORDWIN {
	PMS_INPUT_VIEW*        vwk;
	const PMS_INPUT_WORK*  mwk;
	const PMS_INPUT_DATA*  dwk;

	GFL_BMPWIN  *win;
	GFL_BMPWIN  *tmp_win;

	GFL_CLWK*	cursor_actor;
//	GFL_CLWK*	up_arrow_actor;
//	GFL_CLWK*	down_arrow_actor;
	GFL_CLWK*	scroll_bar_actor;
  GFL_CLWK* deco_actor[ WORDWIN_DECO_ACT_NUM ];
	STRBUF*		tmpbuf;

	PMSIV_TOOL_BLEND_WORK   blend_work;
	PMSIV_TOOL_SCROLL_WORK  scroll_work;
	u32                     eff_seq;

	GXWndPlane	winout_backup;
	int         win_visible_backup;

	int    write_v_line;
	int    write_word_idx;

	int*	p_key_mode;
};


//==============================================================
// Prototype
//==============================================================
static void setup_actor( PMSIV_WORDWIN* wk );
static void clear_scroll_area( PMSIV_WORDWIN* wk, int vector );
static void init_write_params( PMSIV_WORDWIN* wk );
static void print_word( PMSIV_WORDWIN* wk, u32 wordnum, u32 v_line );




//------------------------------------------------------------------
/**
	* 
	*
	* @param   vwk		
	* @param   mwk		
	* @param   dwk		
	*
	* @retval  PMSIV_WORDWIN*		
	*/
//------------------------------------------------------------------
PMSIV_WORDWIN*  PMSIV_WORDWIN_Create( PMS_INPUT_VIEW* vwk, const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk )
{
	PMSIV_WORDWIN*  wk = GFL_HEAP_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_WORDWIN) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;

	wk->tmpbuf = GFL_STR_CreateBuffer( BUFLEN_PMS_WORD, HEAPID_PMS_INPUT_VIEW );

	wk->win = GFL_BMPWIN_Create( FRM_MAIN_WORDWIN, WORDWIN_XORG, WORDWIN_YORG,
					WORDWIN_WIDTH, WORDWIN_HEIGHT, PALNUM_MAIN_WORDWIN, GFL_BMP_CHRAREA_GET_B );

	wk->tmp_win = GFL_BMPWIN_Create( FRM_MAIN_WORDWIN, 0, 0,
					WORD_TMPWIN_WIDTH, 4, PALNUM_MAIN_WORDWIN, GFL_BMP_CHRAREA_GET_B );


	wk->cursor_actor = NULL;
//	wk->up_arrow_actor = NULL;
//	wk->down_arrow_actor = NULL;

	wk->p_key_mode = PMSI_GetKTModePointer(wk->mwk);

	return wk;
}

//------------------------------------------------------------------
/**
	* 
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_Delete( PMSIV_WORDWIN* wk )
{
	if( wk->cursor_actor )
	{
		GFL_CLACT_WK_Remove( wk->cursor_actor );
	}
	if( wk->scroll_bar_actor )
	{
		GFL_CLACT_WK_Remove( wk->scroll_bar_actor );
	}
/*
	if( wk->up_arrow_actor )
	{
		GFL_CLACT_WK_Remove( wk->up_arrow_actor );
	}
	if( wk->down_arrow_actor )
	{
		GFL_CLACT_WK_Remove( wk->down_arrow_actor );
	}
*/
	if( wk->tmpbuf )
	{
		GFL_STR_DeleteBuffer( wk->tmpbuf );
	}

	GFL_BMPWIN_Delete( wk->tmp_win );
	GFL_BMPWIN_Delete( wk->win );

	GFL_HEAP_FreeMemory( wk );
}



//------------------------------------------------------------------
/**
	* 
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_SetupGraphicDatas( PMSIV_WORDWIN* wk )
{
	u32 charpos;

	GFL_BG_SetClearCharacter( FRM_MAIN_WORDWIN, 0x20, CLEAR_CHARPOS*0x20 , HEAPID_PMS_INPUT_VIEW );
	GFL_BG_FillScreen( FRM_MAIN_WORDWIN, CLEAR_CHARPOS, 0, 0, 32, 32, PALNUM_MAIN_WORDWIN );

//	GF_BGL_BmpWinDataFill(&wk->win, WORD_COL_GROUND);
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win ), WORD_COL_GROUND);
//	GF_BGL_BmpWinMakeScrn(&wk->win);
	GFL_BMPWIN_MakeScreen( wk->win );
	//GF_BGL_BmpWinCgxOn(&wk->win);
	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BG_LoadScreenReq( FRM_MAIN_WORDWIN );

	setup_actor( wk );

	GFL_BG_SetVisible( FRM_MAIN_WORDWIN, FALSE );

}

static void setup_actor( PMSIV_WORDWIN* wk )
{
  int i;
	PMSIV_CELL_RES  header;

	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_MAIN, BGPRI_MAIN_EDITAREA );
	wk->cursor_actor = PMSIView_AddActor( wk->vwk, &header, CURSOR_OX, CURSOR_OY,
			ACTPRI_WORDWIN_CURSOR, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor, ANM_WORDWIN_CURSOR_ACTIVE );
	GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor, FALSE );

	wk->scroll_bar_actor = PMSIView_AddActor( wk->vwk, &header, SCRLL_BAR_X, SCRLL_BAR_Y,
			ACTPRI_WORDWIN_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );
	GFL_CLACT_WK_SetAnmSeq( wk->scroll_bar_actor, ANM_EDITAREA_SCR_BTN );
	GFL_CLACT_WK_SetDrawEnable( wk->scroll_bar_actor, FALSE );

/*
	wk->up_arrow_actor = PMSIView_AddActor( wk->vwk, &header, ARROW_UP_X, ARROW_UP_Y,
			ACTPRI_WORDWIN_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );
	GFL_CLACT_WK_SetAnmSeq( wk->up_arrow_actor, ANM_WORD_SCR_U01 );
	GFL_CLACT_WK_SetDrawEnable( wk->up_arrow_actor, FALSE );

	wk->down_arrow_actor = PMSIView_AddActor( wk->vwk, &header, ARROW_DOWN_X, ARROW_DOWN_Y,
			ACTPRI_WORDWIN_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );
	GFL_CLACT_WK_SetAnmSeq( wk->down_arrow_actor, ANM_WORD_SCR_D01 );
	GFL_CLACT_WK_SetDrawEnable( wk->down_arrow_actor, FALSE );
*/

  // デコメ
  for( i=0; i<WORDWIN_DECO_ACT_NUM; i++ )
  {
    PMSIV_CELL_RES resDeco;

    PMSIView_GetDecoResource( wk->vwk, &resDeco, PMSIV_LCD_MAIN );

    wk->deco_actor[i] = PMSIView_AddActor( wk->vwk, &resDeco, 0, 0, 0, NNS_G2D_VRAM_TYPE_2DMAIN );
  
    // @TODO とりあえず直でアニメシーケンス。
    GFL_CLACT_WK_SetAnmSeq( wk->deco_actor[i], i ); 
	  GFL_CLACT_WK_SetDrawEnable( wk->deco_actor[i], FALSE );
  }
}


//------------------------------------------------------------------
/**
	* 単語内容をBitmapWindowに描画
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_SetupWord( PMSIV_WORDWIN* wk )
{
	u32 word_max, v_line, i;

//	GF_BGL_BmpWinDataFill(&wk->win, WORD_COL_GROUND);
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win ), WORD_COL_GROUND);
	init_write_params( wk );

	word_max = PMSI_GetCategoryWordMax( wk->mwk );
	if( word_max > WORDWIN_DISP_WORD_MAX )
	{
		word_max = WORDWIN_DISP_WORD_MAX;
	}

	v_line = wk->write_v_line;
	for(i=0; i<word_max; i++)
	{
		print_word( wk, i, v_line );
		if( i & 1 )
		{
			v_line += WORDWIN_WRITE_Y_MARGIN;
		}
	}

	

//	GF_BGL_BmpWinCgxOn( &wk->win );
	GFL_BMPWIN_TransVramCharacter( wk->win );

}

// 指定位置からの単語内容をBitmapWindowに描画
void PMSIV_WORDWIN_SetupWordBar( PMSIV_WORDWIN* wk, int idx )
{
	u32 word_max, v_line, i;

//	GF_BGL_BmpWinDataFill(&wk->win, WORD_COL_GROUND);
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win ), WORD_COL_GROUND);
	init_write_params( wk );
	wk->write_word_idx = idx * 2;		// ２行なのでｘ２

//	OS_Printf( "linemax = %d\n", idx );

	word_max = PMSI_GetCategoryWordMax( wk->mwk );
	if( word_max > WORDWIN_DISP_WORD_MAX )
	{
		word_max = WORDWIN_DISP_WORD_MAX;
	}

	v_line = wk->write_v_line;
	for(i=0; i<word_max; i++)
	{
		print_word( wk, wk->write_word_idx+i, v_line );
		if( i & 1 )
		{
			v_line += WORDWIN_WRITE_Y_MARGIN;
		}
	}


//	GF_BGL_BmpWinCgxOn( &wk->win );
	GFL_BMPWIN_TransVramCharacter( wk->win );

}


//------------------------------------------------------------------
/**
	* フェードインエフェクト開始
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_StartFadeIn( PMSIV_WORDWIN* wk )
{
	static const int PMSVI_BLEND_ALL = (GX_BLEND_PLANEMASK_BG0|
										GX_BLEND_PLANEMASK_BG1|
										GX_BLEND_PLANEMASK_BG2|
										GX_BLEND_PLANEMASK_BG3|
										GX_BLEND_PLANEMASK_OBJ|
										GX_BLEND_PLANEMASK_BD );
	static const int PMSVI_PLANEMASK_ALL = (GX_WND_PLANEMASK_BG0|
											GX_WND_PLANEMASK_BG1|
											GX_WND_PLANEMASK_BG2|
											GX_WND_PLANEMASK_BG3|
											GX_WND_PLANEMASK_OBJ);


  // このタイミングで背景面を転送
  PMSIView_SetBackScreen( wk->vwk, TRUE );
	
	G2_SetBlendAlpha( FRM_MAIN_WORDWIN_BLENDMASK, PMSVI_BLEND_ALL, 0, 16 );
	GFL_BG_SetVisible( FRM_MAIN_WORDWIN, TRUE );

	// 上下際のスクロール文字列を見られないようにウィンドウで隠す
	G2_SetWnd1InsidePlane(PMSVI_PLANEMASK_ALL, TRUE);

	wk->winout_backup = G2_GetWndOutsidePlane();
	wk->win_visible_backup = GX_GetVisibleWnd();
	G2_SetWndOutsidePlane(PMSVI_PLANEMASK_ALL^FRM_MAIN_WORDWIN_WNDMASK, TRUE);
	G2_SetWnd1Position(0,0,255,168);
	GX_SetVisibleWnd( GX_WNDMASK_W1 );

	wk->eff_seq = 0;
	PMSIV_TOOL_SetupBlendWork( &wk->blend_work, FRM_MAIN_WORDWIN_BLENDMASK, PMSVI_BLEND_ALL, 0, PMSIV_BLEND_MAX, PMSI_FRAMES(6) );
}
//------------------------------------------------------------------
/**
	* フェードインエフェクト終了待ち
	*
	* @param   wk		
	*
	* @retval  BOOL		
	*/
//------------------------------------------------------------------
BOOL PMSIV_WORDWIN_WaitFadeIn( PMSIV_WORDWIN* wk )
{
	switch( wk->eff_seq ){
	case 0:
		if( PMSIV_TOOL_WaitBlend( &wk->blend_work ) )
		{
			return TRUE;
		}
		break;

	default:
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
	* フェードアウトエフェクト開始
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_StartFadeOut( PMSIV_WORDWIN* wk )
{
	static const int PMSVI_BLEND_ALL = (GX_BLEND_PLANEMASK_BG0|
										GX_BLEND_PLANEMASK_BG1|
										GX_BLEND_PLANEMASK_BG2|
										GX_BLEND_PLANEMASK_BG3|
										GX_BLEND_PLANEMASK_OBJ|
										GX_BLEND_PLANEMASK_BD );
	wk->eff_seq = 0;
	PMSIV_TOOL_SetupBlendWork( &wk->blend_work, FRM_MAIN_WORDWIN_BLENDMASK, PMSVI_BLEND_ALL, PMSIV_BLEND_MAX, 0, PMSI_FRAMES(6) );
}
//------------------------------------------------------------------
/**
	* フェードアウトエフェクト終了待ち
	*
	* @param   wk		
	*
	* @retval  BOOL		
	*/
//------------------------------------------------------------------
BOOL PMSIV_WORDWIN_WaitFadeOut( PMSIV_WORDWIN* wk )
{
	switch( wk->eff_seq ){
	case 0:
		if( PMSIV_TOOL_WaitBlend( &wk->blend_work ) )
		{
      int i;

      for( i=0; i<WORDWIN_DECO_ACT_NUM; i++ )
      {
        GFL_CLACT_WK_SetDrawEnable( wk->deco_actor[i], FALSE );
      }
			GFL_BG_SetVisible( FRM_MAIN_WORDWIN, FALSE );
			G2_SetWndOutsidePlane( wk->winout_backup.planeMask, wk->winout_backup.effect );
			GX_SetVisibleWnd( wk->win_visible_backup );
			return TRUE;
		}
		break;

	default:
		return TRUE;
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
	* カーソル表示オンオフ
	*
	* @param   wk		
	* @param   flag		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_VisibleCursor( PMSIV_WORDWIN* wk, BOOL flag )
{

	if( flag )
	{
		if(*wk->p_key_mode == GFL_APP_KTST_KEY){
			GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor,TRUE);
		}else{
			GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor,FALSE);
		}
		GFL_CLACT_WK_SetDrawEnable( wk->scroll_bar_actor, TRUE );
//		GFL_CLACT_WK_SetDrawEnable( wk->up_arrow_actor, PMSI_GetWordWinUpArrowVisibleFlag(wk->mwk) );
//		GFL_CLACT_WK_SetDrawEnable( wk->down_arrow_actor, PMSI_GetWordWinDownArrowVisibleFlag(wk->mwk) );
	}
	else
	{
		GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor, flag );
		GFL_CLACT_WK_SetDrawEnable( wk->scroll_bar_actor, FALSE );
//		GFL_CLACT_WK_SetDrawEnable( wk->up_arrow_actor, FALSE );
//		GFL_CLACT_WK_SetDrawEnable( wk->down_arrow_actor, FALSE );
	}
}

//------------------------------------------------------------------
/**
	* カーソル移動
	*
	* @param   wk		
	* @param   pos		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_MoveCursor( PMSIV_WORDWIN* wk, u32 pos )
{
	GFL_CLACTPOS clPos;
	u32 x, y;

	if(pos == 0xFFFFFFFF){	//back
//    GF_ASSERT(0); // 「もどる」カーソルは廃止
#if 0
		clPos.x = CURSOR_BACK_XPOS;
		clPos.y = CURSOR_BACK_YPOS;

		GFL_CLACT_WK_SetPos( wk->cursor_actor, &clPos , CLSYS_DEFREND_MAIN );
		GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor, ANM_CATEGORY_BACK_CURSOR_ACTIVE );
#endif
	}else{
		x = pos & 1;
		y = pos / 2;

		clPos.x = (CURSOR_OX + CURSOR_X_MARGIN * x );
		clPos.y = (CURSOR_OY + CURSOR_Y_MARGIN * y );

		GFL_CLACT_WK_SetPos( wk->cursor_actor, &clPos , CLSYS_DEFREND_MAIN );
		GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor, ANM_WORDWIN_CURSOR_ACTIVE );
	}
}


//------------------------------------------------------------------
/**
	* スクロール開始
	*
	* @param   wk		
	* @param   vector		
	*
	*/
//------------------------------------------------------------------
void PMSIV_WORDWIN_StartScroll( PMSIV_WORDWIN* wk, int vector )
{
	int next_v_line, next_word_idx, i, v_line, word_idx, write_word_max, scroll_wait;
  
  HOSAKA_Printf("PMSIV_WORDWIN_StartScroll \n" );

	clear_scroll_area( wk, vector );

	next_word_idx = wk->write_word_idx + (vector * WORDWIN_DISP_COL_MAX);
	next_v_line = (wk->write_v_line + (vector * WORDWIN_WRITE_Y_MARGIN)) & 255;

	if( vector < 0 ){
		v_line = next_v_line;
		word_idx = next_word_idx;
		write_word_max = -(vector * WORDWIN_DISP_COL_MAX);
		scroll_wait = -( WORDWIN_SCROLL_WAIT_UNIT * vector );
	}else{
		v_line = (wk->write_v_line + WORDWIN_WRITE_PAGE_HEIGHT) & 255;
		word_idx = wk->write_word_idx + WORDWIN_DISP_WORD_MAX;
		write_word_max = vector * WORDWIN_DISP_COL_MAX;
		if( (write_word_max + word_idx) > PMSI_GetCategoryWordMax(wk->mwk) )
		{
			write_word_max--;
		}
		scroll_wait = WORDWIN_SCROLL_WAIT_UNIT * vector;
	}

	for(i=0; i<write_word_max; i++)
	{
		print_word( wk, word_idx+i, v_line );
		if( i & 1 )
		{
			v_line = (v_line + WORDWIN_WRITE_Y_MARGIN) & 255;
		}
	}


	wk->write_v_line = next_v_line;
	wk->write_word_idx = next_word_idx;

	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BMPWIN_MakeScreen( wk->win );

	PMSIV_TOOL_SetupScrollWork( &(wk->scroll_work), FRM_MAIN_WORDWIN, PMSIV_TOOL_SCROLL_DIRECTION_Y,
		vector*WORDWIN_WRITE_Y_MARGIN, scroll_wait );

}

//------------------------------------------------------------------
/**
	* スクロール終了待ち
	*
	* @param   wk		
	*
	* @retval  BOOL		
	*/
//------------------------------------------------------------------
BOOL PMSIV_WORDWIN_WaitScroll( PMSIV_WORDWIN* wk )
{
	if( PMSIV_TOOL_WaitScroll(&wk->scroll_work) )
	{
//		GFL_CLACT_WK_SetDrawEnable( wk->up_arrow_actor, PMSI_GetWordWinUpArrowVisibleFlag(wk->mwk) );
//		GFL_CLACT_WK_SetDrawEnable( wk->down_arrow_actor, PMSI_GetWordWinDownArrowVisibleFlag(wk->mwk) );
		u16	now, max;
		PMSI_GetWorkScrollData( wk->mwk, &now, &max );
		PMSIV_WORDWIN_SetScrollBarPos( wk, now, max );
		return TRUE;
	}
	return FALSE;
}

// スクロールバーの座標取得
void PMSIV_WINDOW_GetScrollBarPos( PMSIV_WORDWIN * wk, GFL_CLACTPOS * pos )
{
	GFL_CLACT_WK_GetPos( wk->scroll_bar_actor, pos, CLSYS_DEFREND_MAIN );
}

// スクロール値からバーの座標を設定
void PMSIV_WORDWIN_SetScrollBarPos( PMSIV_WORDWIN * wk, u32 now, u32 max )
{
	GFL_CLACTPOS	pos;

//	OS_Printf( "now, max : %d, %d\n", now, max );

	pos.x = SCRLL_BAR_X;

	if( now == 0 ){
		pos.y = SCRLL_BAR_Y;
	}else if( now == max ){
		pos.y = PMSIV_TPWD_RAIL_SY - PMSIV_TPWD_BAR_SY/2;
	}else{
		u32	y = PMSIV_TPWD_RAIL_SY - PMSIV_TPWD_BAR_SY;

		y = ((y<<8)/max*now) >> 8;
		pos.y = SCRLL_BAR_Y + y;
	}

	GFL_CLACT_WK_SetPos( wk->scroll_bar_actor, &pos, CLSYS_DEFREND_MAIN );
}

// スクロールバーの座標を設定
void PMSIV_WORDWIN_MoveScrollBar( PMSIV_WORDWIN * wk, u32 py )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->scroll_bar_actor, &pos, CLSYS_DEFREND_MAIN );
	pos.y = py;
	if( pos.y < SCRLL_BAR_Y ){
		pos.y = SCRLL_BAR_Y;
	}
	if( pos.y > (PMSIV_TPWD_RAIL_SY-PMSIV_TPWD_BAR_SY/2) ){
		pos.y = PMSIV_TPWD_RAIL_SY - PMSIV_TPWD_BAR_SY/2;
	}
	GFL_CLACT_WK_SetPos( wk->scroll_bar_actor, &pos, CLSYS_DEFREND_MAIN );
}

// スクロールバーの座標からスクロール値を取得
u32 PMSIV_WORDWIN_GetScrollBarPosCount( PMSIV_WORDWIN * wk, u32 max )
{
	GFL_CLACTPOS	pos;
	u32	cnt;
	u32	sy;

	PMSIV_WINDOW_GetScrollBarPos( wk, &pos );

	sy = PMSIV_TPWD_RAIL_SY - PMSIV_TPWD_BAR_SY;
	cnt = ( ((pos.y-SCRLL_BAR_Y)<<8) / ((sy<<8)/max) );
	if( cnt > max ){
		cnt = max;
	}
	return cnt;
}



static void clear_scroll_area( PMSIV_WORDWIN* wk, int vector )
{
	int  top, bottom;

	if( vector > 0 )
	{
		top = (wk->write_v_line + WORDWIN_WRITE_PAGE_HEIGHT) & 0xff;
		bottom = (top + (vector * WORDWIN_WRITE_Y_MARGIN)) & 0xff;
	}
	else
	{
		bottom = wk->write_v_line;
		top = (bottom + (vector * WORDWIN_WRITE_Y_MARGIN)) & 0xff;
	}

	if( top < bottom )
	{
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->win), 0, top, WORDWIN_WIDTH*8, (bottom-top),WORD_COL_GROUND );
	}
	else
	{
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->win), 0, top, WORDWIN_WIDTH*8, 256-top ,WORD_COL_GROUND);
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->win), 0, 0, WORDWIN_WIDTH*8, bottom ,WORD_COL_GROUND);
	}
}



static void init_write_params( PMSIV_WORDWIN* wk )
{
	wk->write_v_line = WORDWIN_WRITE_LINE_INIT;
	wk->write_word_idx = 0;
	GFL_BG_SetScroll( FRM_MAIN_WORDWIN, GFL_BG_SCROLL_Y_SET, WORDWIN_DEFAOULT_SCROLL_Y );
}


static void print_word( PMSIV_WORDWIN* wk, u32 wordnum, u32 v_line )
{
	GFL_FONT *fontHandle = PMSIView_GetFontHandle(wk->vwk);

	// リストの最上段は空欄とするため、総数を＋２　2009/11/14 by nakahiro
	if( wordnum < PMSI_DUMMY_LABEL_NUM ){ return; }
	if( wordnum >= PMSI_GetCategoryWordMax( wk->mwk ) ){ return; }
	wordnum -= PMSI_DUMMY_LABEL_NUM;
	
  // デコメ例外処理
  if( PMSI_GetCategoryMode( wk->mwk ) == CATEGORY_MODE_GROUP && PMSI_GetCategoryCursorPos( wk->mwk ) == CATEGORY_GROUP_PICTURE )
  {
    GFL_CLACTPOS pos;

    GF_ASSERT( wordnum < WORDWIN_DECO_ACT_NUM );

    pos.x = WORDWIN_WRITE_DECO_OX + (wordnum&1)*WORDWIN_WRITE_X_MARGIN;
    pos.y = v_line-WORDWIN_DEFAOULT_SCROLL_Y;

    HOSAKA_Printf("[%d] deco pos[%d,%d] \n",wordnum, pos.x, pos.y);
    GFL_CLACT_WK_SetWldPos( wk->deco_actor[ wordnum ], &pos );
    GFL_CLACT_WK_SetDrawEnable( wk->deco_actor[ wordnum ], TRUE );
  }
  else
  {
    PMSI_GetCategoryWord( wk->mwk, wordnum, wk->tmpbuf );

    // @TODO
    GFL_FONTSYS_SetDefaultColor();

    if( v_line <= WORDWIN_WRITE_LINE_ROUND_BORDER )
    {
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( wk->win ) , 
              WORDWIN_WRITE_OX + (wordnum&1)*WORDWIN_WRITE_X_MARGIN, 
              v_line,
              wk->tmpbuf,
              fontHandle );
    }
    else
    {
      //スクロールをまたがるとき、仮Winに描画してから切れ目にコピー
      u32  write_v_range = WORDWIN_WRITE_LINE_MAX - v_line;
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->tmp_win ), WORD_COL_GROUND);

      PRINTSYS_Print( GFL_BMPWIN_GetBmp( wk->tmp_win ) , 0, 0,wk->tmpbuf,fontHandle);

      GFL_BMP_Print(
				GFL_BMPWIN_GetBmp(wk->tmp_win),
				GFL_BMPWIN_GetBmp(wk->win),
				0, 0,
        WORDWIN_WRITE_OX + (wordnum&1)*WORDWIN_WRITE_X_MARGIN,   v_line,
        WORD_TMPWIN_WIDTH*8, write_v_range,
				GF_BMPPRT_NOTNUKI);

      GFL_BMP_Print(
				GFL_BMPWIN_GetBmp(wk->tmp_win),
				GFL_BMPWIN_GetBmp(wk->win),
				0, write_v_range,
        WORDWIN_WRITE_OX + (wordnum&1)*WORDWIN_WRITE_X_MARGIN,   0,
        WORD_TMPWIN_WIDTH*8, (WORD_TMPWIN_HEIGHT*8) - write_v_range ,
				GF_BMPPRT_NOTNUKI);
    }
  }
}
