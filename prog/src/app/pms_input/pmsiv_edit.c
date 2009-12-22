//============================================================================================
/**
	* @file	pmsiv_edit.c
	* @bfief	簡易会話入力画面（描画下請け：編集欄）
	* @author	taya
	* @date	06.02.07
	*/
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system\main.h"
#include "system\gfl_use.h"
#include "system\pms_word.h"
#include "system\bmp_cursor.h"
#include "system\bmp_winframe.h"
#include "system\touch_subwindow.h"
#include "print\printsys.h"
#include "print\wordset.h"
#include "msg\msg_pms_input.h"
#include "message.naix"

#include "pms_input_prv.h"
#include "pms_input_view.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
enum {
	INPUTAREA_PALTYPE_MAX = 10,

	STR_TMPBUF_LEN = 128,	// 表示前のテンポラリとして使用。最大文字数。
};

enum {
	EDITAREA_WIN_X = 3,
	EDITAREA_WIN_Y = 1,
	EDITAREA_WIN_WIDTH = 27,
	EDITAREA_WIN_HEIGHT = 4,
	EDITAREA_WIN_CHARSIZE = EDITAREA_WIN_WIDTH*EDITAREA_WIN_HEIGHT,

	MESSAGE_WIN_X = 2,
	MESSAGE_WIN_Y = 21,
	MESSAGE_WIN_WIDTH = 27,
	MESSAGE_WIN_HEIGHT = 2,
	MESSAGE_WIN_CHARSIZE = MESSAGE_WIN_WIDTH*MESSAGE_WIN_HEIGHT,

	YESNO_WIN_X = 25,
	YESNO_WIN_Y = 12,
	YESNO_WIN_WIDTH = 8,
	YESNO_WIN_HEIGHT = 4,
	YESNO_WIN_CHARSIZE = YESNO_WIN_WIDTH*YESNO_WIN_HEIGHT,

	DCSEL_WIN_X = 24,
	DCSEL_WIN_Y = 14,
	DCSEL_WIN_WIDTH = 7,
	DCSEL_WIN_HEIGHT = 2,
	DCSEL_WIN_CHARSIZE = DCSEL_WIN_WIDTH*DCSEL_WIN_HEIGHT,
	DCSEL_WIN_SCR_W = DCSEL_WIN_WIDTH+2,
	DCSEL_WIN_SCR_H = 7,
	
	// 単語領域位置情報（ドット単位、中心点）
	WORDAREA_SINGLE_X = (128- EDITAREA_WIN_X*8),
	WORDAREA_SINGLE_Y = 16,
	WORDAREA_DOUBLE_X1 = 48,
	WORDAREA_DOUBLE_Y1 = 16,
	WORDAREA_DOUBLE_X2 = 160,
	WORDAREA_DOUBLE_Y2 = WORDAREA_DOUBLE_Y1,

	WORDAREA_X_MARGIN = 2,	// 前後文字列との余白（ドット）


	WORDAREA_WIDTH = 96,
	WORDAREA_HEIGHT = 16,
	EDITAREA_LINE_HEIGHT = 16,

#if 0
	LEFT_ARROW_OBJ_XPOS = 24,
	LEFT_ARROW_OBJ_YPOS = 64,
	RIGHT_ARROW_OBJ_XPOS = 256-24,
	RIGHT_ARROW_OBJ_YPOS = LEFT_ARROW_OBJ_YPOS,
#else
	LEFT_ARROW_OBJ_XPOS = 8,
	LEFT_ARROW_OBJ_YPOS = 64,
	RIGHT_ARROW_OBJ_XPOS = 256-8,
	RIGHT_ARROW_OBJ_YPOS = LEFT_ARROW_OBJ_YPOS,
#endif
	BAR_OBJ_XPOS = 128,
	BAR_OBJ_YPOS = LEFT_ARROW_OBJ_YPOS,
	BTN_OBJ_XPOS = 128,
	BTN_OBJ_YPOS = LEFT_ARROW_OBJ_YPOS,

	ALLCOVER_CURSOR_XPOS = 128,
	ALLCOVER_CURSOR_YPOS = 24,
};


enum {
	YESNOWIN_CURSOR_OX = 0,
	YESNOWIN_CURSOR_OY = 0,

	YESNOWIN_STR_OX = 14,
	YESNOWIN_STR_OY = 0,

	YESONOWIN_LINE_HEIGHT = 16,
};


enum {
	EDITAREA_WIN_BASE_COLOR_LETTER = 0x01,
	EDITAREA_WIN_BASE_COLOR_SHADOW = 0x02,
	EDITAREA_WIN_BASE_COLOR_GROUND = 0x0f,

	EDITAREA_WIN_WORD_COLOR_LETTER = 0x03,
	EDITAREA_WIN_WORD_COLOR_SHADOW = 0x04,
	EDITAREA_WIN_WORD_COLOR_GROUND = 0x0e,

	MESSAGE_WIN_COL_LETTER = 0x01,
	MESSAGE_WIN_COL_SHADOW = 0x02,
	MESSAGE_WIN_COL_GROUND = 0x09,

	YESNOWIN_COL_LETTER = 0x01,
	YESNOWIN_COL_SHADOW = 0x02,
	YESNOWIN_COL_GROUND = 0x09,
};

//--------------------------------------------------------------
/**
	*	文章解析ルーチンの戻り値
	*/
//--------------------------------------------------------------
enum {
	PMS_ANALYZE_RESULT_STR,
	PMS_ANALYZE_RESULT_TAG,
	PMS_ANALYZE_RESULT_CR,
	PMS_ANALYZE_RESULT_END,
};

//======================================================================
typedef struct {
	s16  x;
	s16  y;
}POS;

typedef struct {
	STRBUF*	src;
	const STRCODE*	p;
	int  state;
}PMS_ANALYZE_WORK;


//======================================================================
//--------------------------------------------------------------
/**
	*	
	*/
//--------------------------------------------------------------
struct _PMSIV_EDIT {
	PMS_INPUT_VIEW*        vwk;
	const PMS_INPUT_WORK*  mwk;
	const PMS_INPUT_DATA*  dwk;

	GFL_BMPWIN	*win_edit[PMSIV_LCD_MAX];
	GFL_BMPWIN	*win_msg[PMSIV_LCD_MAX];
	GFL_BMPWIN	*win_yesno;
	GFL_BMPWIN	**win_message;
	GFL_CLWK	*cursor_actor[PMSIV_LCD_MAX];
	GFL_CLWK	*deco_actor[PMS_WORD_MAX][PMSIV_LCD_MAX];
#if 0
  // 会話文選択バー・ボタン
	GFL_CLWK	*arrow_left_actor;
	GFL_CLWK	*arrow_right_actor;
	GFL_CLWK	*bar_actor;
	GFL_CLWK	*btn_actor;
#endif

	GFL_MSGDATA		*msgman;
	STRBUF			*tmpbuf;
	BMPCURSOR		*bmp_cursor;

	POS                word_pos[PMS_INPUT_WORD_MAX];
	u32                word_pos_max;
	u32                yesnowin_frame_charpos;

	PMS_ANALYZE_WORK   awk;

	u16   pal_data[INPUTAREA_PALTYPE_MAX][16];

	s16					main_scr;
	s16					sub_scr;
	u8					scr_ct;
	u8					scr_dir;

	TOUCH_SW_SYS		*ynbtn_wk;

	u16					msgwin_cgx;
	u16					yesno_win_cgx;

	int*				p_key_mode;
	
	GFL_TCB				*hBlankTask;
};


//==============================================================
// Prototype
//==============================================================
static void pmsiv_edit_hblank(GFL_TCB *, void *vwork);
static void setup_pal_datas( PMSIV_EDIT* wk, ARCHANDLE* p_handle );
static void update_editarea_palette( PMSIV_EDIT* wk );
static void setup_wordarea_pos( PMSIV_EDIT* wk );
static void setup_obj( PMSIV_EDIT* wk );
static u32 print_sentence( PMSIV_EDIT* wk ,GFL_BMPWIN* win, u8 cnt);
static void setup_pms_analyze( PMS_ANALYZE_WORK* awk, PMSIV_EDIT* wk );
static void cleanup_pms_analyze( PMS_ANALYZE_WORK* awk );
static int prog_pms_analyze( PMS_ANALYZE_WORK* awk, STRBUF* buf );
static void wordpos_to_orgpos( const POS* wordpos, POS* orgpos);
static void wordpos_to_lcdpos( const POS* wordpos, POS* lcdpos );
static void fill_wordarea( GFL_BMPWIN* win, const POS* wordpos );
static void print_wordarea( PMSIV_EDIT* wk, GFL_BMPWIN* win, const POS* wordpos, PMS_WORD word, GFL_CLWK* deco_actor );
static void set_cursor_anm( PMSIV_EDIT* wk, BOOL active_flag );
static CLSYS_DRAW_TYPE BGFrameToVramType( u8 frame );




//------------------------------------------------------------------
/**
	* 
	*
	* @param   vwk		
	* @param   mwk		
	* @param   dwk		
	*
	* @retval  PMSIV_EDIT*		
	*/
//------------------------------------------------------------------
PMSIV_EDIT*  PMSIV_EDIT_Create( PMS_INPUT_VIEW* vwk, const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk )
{
	PMSIV_EDIT*  wk = GFL_HEAP_AllocClearMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_EDIT) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;

	wk->p_key_mode = PMSI_GetKTModePointer(wk->mwk);

	wk->cursor_actor[0] = NULL;
	wk->cursor_actor[1] = NULL;
#if 0
	wk->arrow_left_actor = NULL;
	wk->arrow_right_actor = NULL;
	wk->bar_actor = NULL;
	wk->btn_actor = NULL;
#endif
	wk->hBlankTask = NULL;

	wk->tmpbuf = GFL_STR_CreateBuffer( STR_TMPBUF_LEN, HEAPID_PMS_INPUT_VIEW );
	wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pms_input_dat, HEAPID_PMS_INPUT_VIEW );
	wk->bmp_cursor = BmpCursor_Create( HEAPID_PMS_INPUT_VIEW );

	wk->main_scr = 0;
	wk->sub_scr = 0;
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
void PMSIV_EDIT_Delete( PMSIV_EDIT* wk )
{
	if( wk->hBlankTask )
	{
		GFL_TCB_DeleteTask( wk->hBlankTask );
	}
	if( wk->bmp_cursor )
	{
		BmpCursor_Delete( wk->bmp_cursor );
	}
	if( wk->cursor_actor[0] )
	{
		GFL_CLACT_WK_Remove( wk->cursor_actor[0] );
	}
	if( wk->cursor_actor[1] )
	{
		GFL_CLACT_WK_Remove( wk->cursor_actor[1] );
	}
#if 0
	if( wk->arrow_left_actor )
	{
		GFL_CLACT_WK_Remove( wk->arrow_left_actor );
	}
	if( wk->arrow_right_actor )
	{
		GFL_CLACT_WK_Remove( wk->arrow_right_actor );
	}
	if( wk->bar_actor )
	{
		GFL_CLACT_WK_Remove( wk->bar_actor );
	}
	if( wk->btn_actor )
	{
		GFL_CLACT_WK_Remove( wk->btn_actor );
	}
#endif
	if( wk->msgman )
	{
		GFL_MSG_Delete( wk->msgman );
	}
	if( wk->tmpbuf )
	{
		GFL_STR_DeleteBuffer( wk->tmpbuf );
	}

	GFL_BMPWIN_Delete(wk->win_edit[0]);
	GFL_BMPWIN_Delete(wk->win_edit[1]);
	GFL_BMPWIN_Delete(wk->win_msg[0]);
	GFL_BMPWIN_Delete(wk->win_msg[1]);

	//YesNoボタンシステムワーク解放
	TOUCH_SW_FreeWork( wk->ynbtn_wk);

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
void PMSIV_EDIT_SetupGraphicDatas( PMSIV_EDIT* wk, ARCHANDLE* p_handle )
{
	u32 charpos;
	
	setup_pal_datas( wk, p_handle );

	GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms_bg_main0_NSCR,
		FRM_MAIN_EDITAREA, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms_bg_main0_NSCR,
		FRM_SUB_EDITAREA , 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );

	wk->sub_scr = -24*8;
	GFL_BG_SetScroll( FRM_SUB_EDITAREA, GFL_BG_SCROLL_Y_SET,wk->sub_scr);

	charpos = GFL_ARCHDL_UTIL_TransVramBgCharacter(p_handle, NARC_pmsi_pms_bg_main0_NCGR,
		FRM_MAIN_EDITAREA, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(p_handle, NARC_pmsi_pms_bg_main0_NCGR,
		FRM_SUB_EDITAREA, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	charpos /= 0x20;

	//↑までのcharaposはBmpWinを保持して後方確保なので無視

  charpos = 0;

	wk->win_edit[0] = GFL_BMPWIN_Create( FRM_MAIN_EDITAREA, EDITAREA_WIN_X, EDITAREA_WIN_Y, 
						EDITAREA_WIN_WIDTH, EDITAREA_WIN_HEIGHT, 
						PALNUM_MAIN_EDITAREA,GFL_BMP_CHRAREA_GET_B );
	wk->win_edit[1] = GFL_BMPWIN_Create( FRM_SUB_EDITAREA, EDITAREA_WIN_X, EDITAREA_WIN_Y, 
						EDITAREA_WIN_WIDTH, EDITAREA_WIN_HEIGHT, 
						PALNUM_MAIN_EDITAREA,GFL_BMP_CHRAREA_GET_B );
	charpos += EDITAREA_WIN_CHARSIZE;

	wk->win_msg[0] = GFL_BMPWIN_Create( FRM_MAIN_EDITAREA, MESSAGE_WIN_X, MESSAGE_WIN_Y, 
						MESSAGE_WIN_WIDTH, MESSAGE_WIN_HEIGHT, 
						PALNUM_MAIN_CATEGORY,GFL_BMP_CHRAREA_GET_B );
	wk->win_msg[1] = GFL_BMPWIN_Create( FRM_MAIN_EDITAREA, MESSAGE_WIN_X, MESSAGE_WIN_Y,
						MESSAGE_WIN_WIDTH-8, MESSAGE_WIN_HEIGHT, 
						PALNUM_MAIN_CATEGORY,GFL_BMP_CHRAREA_GET_B );
	charpos += MESSAGE_WIN_CHARSIZE;

	wk->yesno_win_cgx = charpos;
	charpos += TOUCH_SW_USE_CHAR_NUM;//YESNO_WIN_CHARSIZE;

	//YesNoボタンシステムワーク確保
	wk->ynbtn_wk = TOUCH_SW_AllocWork(HEAPID_PMS_INPUT_VIEW);

	BmpWinFrame_CgxSet( FRM_MAIN_EDITAREA , charpos , MENU_TYPE_SYSTEM , HEAPID_PMS_INPUT_VIEW );

	wk->yesnowin_frame_charpos = charpos;
	charpos += MENU_WIN_CGX_SIZ;

	wk->msgwin_cgx = charpos;
	BmpWinFrame_GraphicSet( FRM_MAIN_EDITAREA,charpos,PALNUM_MAIN_TALKWIN,
							MENU_TYPE_SYSTEM,HEAPID_PMS_INPUT_VIEW );

#ifdef USE_SYSWIN
	wk->win_message = &wk->win_msg[0];
	BmpWinFrame_Write( *wk->win_message, WINDOW_TRANS_OFF ,charpos, PALNUM_MAIN_TALKWIN );
#endif

	GFL_BMPWIN_MakeScreen( wk->win_edit[0] );
	GFL_BMPWIN_MakeScreen( wk->win_edit[1] );
	GFL_BMPWIN_MakeScreen( wk->win_msg[0]);
	GFL_BMPWIN_MakeScreen( wk->win_msg[1] );

	setup_wordarea_pos(wk);
	PMSIV_EDIT_UpdateEditArea( wk );

	// PMSIV_EDIT_UpdateEditArea で単語数が確定するので、その後に。
	setup_obj( wk );
}

static void pmsiv_edit_hblank(GFL_TCB *, void *vwork)
{
	s32 vc;
	PMSIV_EDIT* wk = (PMSIV_EDIT*)vwork;

	vc = GX_GetVCount();
  
// HOSAKA_Printf("vc=%d\n", vc);

	if(vc < 6*8){
		GFL_BG_SetScroll( FRM_MAIN_EDITAREA, GFL_BG_SCROLL_Y_SET,wk->main_scr);
	}else{
		GFL_BG_SetScroll( FRM_MAIN_EDITAREA, GFL_BG_SCROLL_Y_SET,0);
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PMSIV_EDIT* wk
 *	@param	scr_dir
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIV_EDIT_ScrollSet( PMSIV_EDIT* wk,u8 scr_dir)
{
	wk->scr_ct = 0;
	wk->scr_dir = scr_dir;

	if( wk->hBlankTask != NULL )
	{
		GFL_TCB_DeleteTask( wk->hBlankTask );
	}
	wk->hBlankTask = GFUser_HIntr_CreateTCB( pmsiv_edit_hblank, wk , 1);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  エディットウィンドウ スクロール
 *
 *	@param	PMSIV_EDIT* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
int PMSIV_EDIT_ScrollWait( PMSIV_EDIT* wk)
{
  int i,w;
	s16 dir;
	s16	posY;
	
	if(wk->scr_ct > 5){
    // スクリーンをクリア @TODO hosaka test > これでチラつきはなくなるが、一部残る。
    GFL_BG_FillScreen( FRM_MAIN_EDITAREA, 0x0, 0, 0, 32*8, 4*8, 0 );
		return TRUE;
	}
	
	if(wk->scr_dir){
		dir = -8;
	}else{
		dir = 8;
	}
	wk->sub_scr += dir;
	wk->main_scr += dir;
	GFL_BG_SetScroll( FRM_MAIN_EDITAREA, GFL_BG_SCROLL_Y_SET,wk->main_scr);
	GFL_BG_SetScroll( FRM_SUB_EDITAREA, GFL_BG_SCROLL_Y_SET,wk->sub_scr);
	
  for( i=0; i<PMSIV_LCD_MAX; i++ )
  {
    // カーソル追随
    posY = GFL_CLACT_WK_GetTypePos( wk->cursor_actor[i] , CLSYS_DEFREND_MAIN+i, CLSYS_MAT_Y );
    posY -=dir ;
    GFL_CLACT_WK_SetTypePos( wk->cursor_actor[i] , posY , CLSYS_DEFREND_MAIN+i, CLSYS_MAT_Y );

    // デコメ追随
    for( w=0; w<PMS_WORD_MAX; w++ )
    {
      posY = GFL_CLACT_WK_GetTypePos( wk->deco_actor[w][i] , CLSYS_DEFREND_MAIN+i, CLSYS_MAT_Y );
      posY -=dir ;
      GFL_CLACT_WK_SetTypePos( wk->deco_actor[w][i] , posY , CLSYS_DEFREND_MAIN+i, CLSYS_MAT_Y );
    }
  }

	wk->scr_ct++;

  HOSAKA_Printf("scr_ct=%d \n", wk->scr_ct );

	if(wk->scr_dir && (wk->scr_ct == 6)){
    HOSAKA_Printf("task delete \n" );
		GFL_TCB_DeleteTask( wk->hBlankTask );
		wk->hBlankTask = NULL;
//		sys_HBlankIntrSet(NULL,NULL);
//		sys_HBlankIntrStop();
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
	* 
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
static void setup_pal_datas( PMSIV_EDIT* wk, ARCHANDLE* p_handle )
{
	NNSG2dPaletteData*  palDat;
	void*   data_ptr;

	// 一度全てVRAM転送する
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_pmsi_pms_bg_main_NCLR, PALTYPE_MAIN_BG,
		 0, 14*0x20, HEAPID_PMS_INPUT_VIEW );
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_pmsi_pms_bg_sub_NCLR, PALTYPE_SUB_BG,
		 0, 5*0x20, HEAPID_PMS_INPUT_VIEW );

	// 後の色変え用にヒープにも読み込んでおく
	data_ptr = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_pmsi_pms_bgm_dat_NCLR, &palDat, HEAPID_PMS_INPUT_VIEW );

	MI_CpuCopy16( palDat->pRawData, wk->pal_data, sizeof(wk->pal_data) );
	DC_FlushRange( wk->pal_data, sizeof(wk->pal_data) );

	GFL_HEAP_FreeMemory( data_ptr );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  エディット領域のパレットを PMS_TYPEによって 切替転送
 *
 *	@param	PMSIV_EDIT* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void update_editarea_palette( PMSIV_EDIT* wk )
{
	if( PMSI_GetInputMode(wk->mwk) == PMSI_MODE_SENTENCE )
	{
		int pal = 1 + PMSI_GetSentenceType(wk->mwk);

		GX_LoadBGPltt( wk->pal_data[pal], PALNUM_MAIN_EDITAREA*0x20, 0x20 );
		GXS_LoadBGPltt( wk->pal_data[pal], PALNUM_MAIN_EDITAREA*0x20, 0x20 );
	}
	else
	{
		GX_LoadBGPltt( wk->pal_data[0], PALNUM_MAIN_EDITAREA*0x20, 0x20 );
		GXS_LoadBGPltt( wk->pal_data[0], PALNUM_MAIN_EDITAREA*0x20, 0x20 );
	}
}



static void setup_wordarea_pos( PMSIV_EDIT* wk )
{
	switch(PMSI_GetInputMode(wk->mwk)){
	case PMSI_MODE_SINGLE:
		wk->word_pos[0].x = WORDAREA_SINGLE_X;
		wk->word_pos[0].y = WORDAREA_SINGLE_Y;
		wk->word_pos_max = 1;
		break;

	case PMSI_MODE_DOUBLE:
		wk->word_pos[0].x = WORDAREA_DOUBLE_X1;
		wk->word_pos[0].y = WORDAREA_DOUBLE_Y1;
		wk->word_pos[1].x = WORDAREA_DOUBLE_X2;
		wk->word_pos[1].y = WORDAREA_DOUBLE_Y2;
		wk->word_pos_max = 2;
		break;

	case PMSI_MODE_SENTENCE:
		// 文章モードの場合、文章内容によって位置が変わるので
		// ここでは何もしない
		wk->word_pos_max = 0;
		break;
	}

}

static void setup_obj( PMSIV_EDIT* wk )
{
	PMSIV_CELL_RES  header;
	POS  actpos;

	if( wk->word_pos_max )
	{
		wordpos_to_lcdpos( &wk->word_pos[0], &actpos );
	}
	else
	{
		actpos.x = ALLCOVER_CURSOR_XPOS;
		actpos.y = ALLCOVER_CURSOR_YPOS;
	}

	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_SUB, BGPRI_MAIN_EDITAREA );
	wk->cursor_actor[1] = PMSIView_AddActor( wk->vwk, &header, actpos.x, actpos.y + GX_LCD_SIZE_Y,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DSUB );
	GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor[1], 1);
  GFL_CLACT_WK_SetBgPri( wk->cursor_actor[1], 1 );

	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_MAIN, BGPRI_MAIN_EDITAREA );
	wk->cursor_actor[0] = PMSIView_AddActor( wk->vwk, &header, actpos.x, actpos.y,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DMAIN );

  {
    int i, k;
    PMSIV_CELL_RES deco_res;

    for( i=0; i<PMS_WORD_MAX; i++ )
    {
      PMSIView_GetDecoResource( wk->vwk, &deco_res, PMSIV_LCD_MAIN+i );

      for( k=0; k<PMS_WORD_MAX; k++ )
      {
        wk->deco_actor[k][PMSIV_LCD_MAIN+i] = PMSIView_AddActor( wk->vwk, &deco_res, 0, 0,
          ACTPRI_EDITAREA_DECO, NNS_G2D_VRAM_TYPE_2DMAIN+i );
      
        GFL_CLACT_WK_SetDrawEnable( wk->deco_actor[k][PMSIV_LCD_MAIN+i], FALSE );
      }
    }
  }

	set_cursor_anm( wk, TRUE );

#if 0
	wk->arrow_left_actor = PMSIView_AddActor( wk->vwk, &header, LEFT_ARROW_OBJ_XPOS, LEFT_ARROW_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_CLACT_WK_SetAnmSeq( wk->arrow_left_actor, ANM_EDITAREA_SCR_L01 );

	wk->arrow_right_actor = PMSIView_AddActor( wk->vwk, &header, RIGHT_ARROW_OBJ_XPOS, RIGHT_ARROW_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_CLACT_WK_SetAnmSeq( wk->arrow_right_actor, ANM_EDITAREA_SCR_R01 );
	
	wk->btn_actor = PMSIView_AddActor( wk->vwk, &header, BTN_OBJ_XPOS, BTN_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_CLACT_WK_SetAnmSeq( wk->btn_actor, ANM_EDITAREA_SCR_BTN );
	
	wk->bar_actor = PMSIView_AddActor( wk->vwk, &header, BAR_OBJ_XPOS, BAR_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_CLACT_WK_SetAnmSeq( wk->bar_actor, ANM_EDITAREA_SCR_BAR );

	if( PMSI_GetInputMode( wk->mwk ) != PMSI_MODE_SENTENCE )
	{
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_right_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_left_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->bar_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->btn_actor, FALSE );
	}

#endif

}

//------------------------------------------------------------------
/**
	* 
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_EDIT_UpdateEditArea( PMSIV_EDIT* wk )
{
	int i;

	update_editarea_palette( wk );

	for(i = 0;i < PMSIV_LCD_MAX;i++)
  {

    {
      int w;
      for( w=0; w<PMS_WORD_MAX; w++ )
      {
        // 一端デコメ表示をクリア(OBJ初期化前にこの関数を使う必要があるため、NULLを弾く)
        if( wk->deco_actor[w][i] )
        {
          GFL_CLACT_WK_SetDrawEnable( wk->deco_actor[w][i], FALSE );
        }
      }
    }

		GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win_edit[i]),EDITAREA_WIN_BASE_COLOR_GROUND );
		//GF_BGL_BmpWinDataFill( &wk->win_edit[i], EDITAREA_WIN_BASE_COLOR_GROUND );

		switch(PMSI_GetInputMode(wk->mwk)){
		case PMSI_MODE_SINGLE:
			fill_wordarea( wk->win_edit[i], &wk->word_pos[0] );
			print_wordarea( wk, wk->win_edit[i], &(wk->word_pos[0]), PMSI_GetEditWord(wk->mwk, 0), wk->deco_actor[0][i] );
			break;
		case PMSI_MODE_DOUBLE:
			fill_wordarea( wk->win_edit[i], &wk->word_pos[0] );
			fill_wordarea( wk->win_edit[i], &wk->word_pos[1] );
			print_wordarea( wk, wk->win_edit[i], &(wk->word_pos[0]), PMSI_GetEditWord(wk->mwk, 0), wk->deco_actor[0][i] );
			print_wordarea( wk, wk->win_edit[i], &(wk->word_pos[1]), PMSI_GetEditWord(wk->mwk, 1), wk->deco_actor[1][i] );
			break;
		case PMSI_MODE_SENTENCE:
			wk->word_pos_max = print_sentence( wk ,wk->win_edit[i], i);
//			OS_TPrintf("word max:%d\n", wk->word_pos_max);
			break;
		}

		GFL_BMPWIN_TransVramCharacter( wk->win_edit[i] );
	}
	GFL_BG_LoadScreenV_Req( FRM_MAIN_EDITAREA );
	GFL_BG_LoadScreenV_Req( FRM_SUB_EDITAREA );
}


/**
	*	@brief	ワードポジションの判定矩形を返す
	*/
void PMSIV_EDIT_GetSentenceWordPos( PMSIV_EDIT* wk ,GFL_UI_TP_HITTBL* tbl,u8 idx)
{
	tbl->rect.left = wk->word_pos[idx].x-(WORDAREA_WIDTH/2)+EDITAREA_WIN_X*8;
	tbl->rect.right = tbl->rect.left + WORDAREA_WIDTH; 
	tbl->rect.top = wk->word_pos[idx].y-(WORDAREA_HEIGHT/2)+EDITAREA_WIN_Y*8;
	tbl->rect.bottom = tbl->rect.top + WORDAREA_HEIGHT; 
}

//------------------------------------------------------------------
/**
	* 選択されている文章を解析、整形してプリントする。
	* 解析時に判定して必要な単語数を返す
	*
	* @param   wk		
	*
	* @retval  u32		
	*/
//------------------------------------------------------------------
static u32 print_sentence( PMSIV_EDIT* wk ,GFL_BMPWIN* win, u8 cnt)
{
	STRBUF* buf = PMSI_GetEditSourceString( wk->mwk, HEAPID_PMS_INPUT_VIEW );
	int x, y, word_pos;
	BOOL cont_flag;
	PMS_WORD  word_code;
	GFL_FONT *fontHandle = fontHandle = PMSIView_GetFontHandle(wk->vwk);

  GF_ASSERT( cnt < PMSIV_LCD_MAX );

	x = y = word_pos = 0;
	cont_flag = TRUE;

	setup_pms_analyze( &wk->awk, wk );

	while( cont_flag )
	{
		switch( prog_pms_analyze( &(wk->awk), buf ) ){
		case PMS_ANALYZE_RESULT_STR:
//			GF_STR_PrintColor( win, PMSI_FONT_EDITAREA_BASE, buf, x, y, MSG_NO_PUT,
//				GF_PRINTCOLOR_MAKE(EDITAREA_WIN_BASE_COLOR_LETTER, EDITAREA_WIN_BASE_COLOR_SHADOW, EDITAREA_WIN_BASE_COLOR_GROUND),
//				NULL);
			GFL_FONTSYS_SetColor( EDITAREA_WIN_BASE_COLOR_LETTER, EDITAREA_WIN_BASE_COLOR_SHADOW, EDITAREA_WIN_BASE_COLOR_GROUND);
			PRINTSYS_Print( GFL_BMPWIN_GetBmp(win), x, y, buf,fontHandle );
			x += PRINTSYS_GetStrWidth(buf,fontHandle, 0);
			break;

		case PMS_ANALYZE_RESULT_TAG:
			x += WORDAREA_X_MARGIN;
			wk->word_pos[word_pos].x = x + (WORDAREA_WIDTH/2);
			wk->word_pos[word_pos].y = y + (WORDAREA_HEIGHT/2);
			fill_wordarea( win, &(wk->word_pos[word_pos]) );
			word_code = PMSI_GetEditWord(wk->mwk, word_pos);
			print_wordarea( wk, win, &(wk->word_pos[word_pos]), word_code, wk->deco_actor[word_pos][cnt] );
			word_pos++;
			x += (WORDAREA_X_MARGIN+WORDAREA_WIDTH);
			break;

		case PMS_ANALYZE_RESULT_CR:
			y += EDITAREA_LINE_HEIGHT;
			x = 0;
			break;

		case PMS_ANALYZE_RESULT_END:
			cont_flag = FALSE;
			break;
		}
	}

	cleanup_pms_analyze( &wk->awk );
	GFL_STR_DeleteBuffer( buf );

	return word_pos;
}

//======================================================================================
//======================================================================================
enum {
	PMS_ANALYZE_STATE_STR,
	PMS_ANALYZE_STATE_TAG,
	PMS_ANALYZE_STATE_CR,
	PMS_ANALYZE_STATE_END,
};

static void setup_pms_analyze( PMS_ANALYZE_WORK* awk, PMSIV_EDIT* wk )
{
	awk->src = PMSI_GetEditSourceString( wk->mwk, HEAPID_PMS_INPUT_VIEW );
	awk->p = GFL_STR_GetStringCodePointer( awk->src );

	if( *(awk->p) == 0xf000/*_CTRL_TAG*/ )
	{
		awk->state = PMS_ANALYZE_STATE_TAG;
	}
	else
	{
		awk->state = PMS_ANALYZE_STATE_STR;
	}
}

static void cleanup_pms_analyze( PMS_ANALYZE_WORK* awk )
{
	GFL_STR_DeleteBuffer( awk->src );
}

static int prog_pms_analyze( PMS_ANALYZE_WORK* awk, STRBUF* buf )
{
	const STRCODE* bp = awk->p;
	int ret;

	switch( awk->state ){
	case PMS_ANALYZE_STATE_STR:
		while( awk->state == PMS_ANALYZE_STATE_STR )
		{
			switch( *(awk->p) ){
			case 0xfffe://CR_:
				awk->state = PMS_ANALYZE_STATE_CR;
				break;
			case 0xffff://EOM_:
				awk->state = PMS_ANALYZE_STATE_END;
				break;
			case 0xf000://_CTRL_TAG:
				awk->state = PMS_ANALYZE_STATE_TAG;
				break;
			default:
				awk->p++;
				break;
			}
		}
		GFL_STR_SetStringCodeOrderLength( buf, bp, (awk->p-bp)+1 );
		return PMS_ANALYZE_RESULT_STR;

	case PMS_ANALYZE_STATE_TAG:
		awk->p = PRINTSYS_SkipTag(awk->p);
		ret = PMS_ANALYZE_RESULT_TAG;
		break;

	case PMS_ANALYZE_STATE_CR:
		awk->p++;
		ret = PMS_ANALYZE_RESULT_CR;
		break;

	case PMS_ANALYZE_STATE_END:
	default:
		return PMS_ANALYZE_RESULT_END;
	}

	switch( *(awk->p) ){
	case 0xfffe://CR_:
		awk->state = PMS_ANALYZE_STATE_CR;
		break;
	case 0xffff://EOM_:
		awk->state = PMS_ANALYZE_STATE_END;
		break;
	case 0xf000://_CTRL_TAG:
		awk->state = PMS_ANALYZE_STATE_TAG;
		break;
	default:
		awk->state = PMS_ANALYZE_STATE_STR;
		break;
	}

	return ret;


}

//======================================================================================
//======================================================================================

static void wordpos_to_orgpos( const POS* wordpos, POS* orgpos)
{
	orgpos->x = wordpos->x - (WORDAREA_WIDTH/2);
	orgpos->y = wordpos->y - (WORDAREA_HEIGHT/2);
}
static void wordpos_to_lcdpos( const POS* wordpos, POS* lcdpos )
{
	lcdpos->x = wordpos->x + EDITAREA_WIN_X * 8;
	lcdpos->y = wordpos->y + EDITAREA_WIN_Y * 8;
}

static void fill_wordarea( GFL_BMPWIN* win, const POS* wordpos )
{
	POS   drawpos;

	wordpos_to_orgpos( wordpos, &drawpos );

	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(win), 
			drawpos.x, drawpos.y,
			WORDAREA_WIDTH, WORDAREA_HEIGHT,
			EDITAREA_WIN_WORD_COLOR_GROUND );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  エディットエリアの指定座標に単語表示
 *
 *	@param	PMSIV_EDIT* wk
 *	@param	win
 *	@param	POS* wordpos
 *	@param	word 
 *	@param  deco_actor
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void print_wordarea( PMSIV_EDIT* wk, GFL_BMPWIN* win, const POS* wordpos, PMS_WORD word, GFL_CLWK* deco_actor )
{
  GFL_FONT *fontHandle = fontHandle = PMSIView_GetFontHandle(wk->vwk);
  POS  print_pos;
//	u32  write_xpos;

  // NULLは何もしない
  if( word == PMS_WORD_NULL ){ return; }
    
  wordpos_to_orgpos( wordpos, &print_pos );

  // デコメなら例外表示
  if( PMSW_IsDeco( word ) )
  {
    PMS_DECO_ID deco_id;
    CLSYS_DRAW_TYPE draw_type;
    GFL_CLACTPOS clPos;

    deco_id = PMSW_GetDecoID( word ) - 1;
    draw_type = BGFrameToVramType( GFL_BMPWIN_GetFrame(win) );

    clPos.x = print_pos.x + GFL_BMPWIN_GetPosX( win ) * 8;
    clPos.y = print_pos.y + GFL_BMPWIN_GetPosY( win ) * 8;

    if( draw_type == CLSYS_DRAW_SUB )
    {
      clPos.y += GX_LCD_SIZE_Y;
    }

    HOSAKA_Printf("deco_actor pos=[%d %d], deco_id=%d \n", clPos.x, clPos.y, deco_id);

    GFL_CLACT_WK_SetAnmSeq( deco_actor, deco_id );
    GFL_CLACT_WK_SetPos( deco_actor, &clPos, draw_type );
    GFL_CLACT_WK_SetDrawEnable( deco_actor, TRUE );
  }
  else
  {
    PMSW_GetStr(word, wk->tmpbuf ,HEAPID_PMS_INPUT_VIEW);
  //	write_xpos = (print_pos.x + (WORDAREA_WIDTH / 2)) - (PRINTSYS_GetStrWidth(wk->tmpbuf, fontHandle,0) / 2);

    GFL_FONTSYS_SetColor( EDITAREA_WIN_WORD_COLOR_LETTER, EDITAREA_WIN_WORD_COLOR_SHADOW, EDITAREA_WIN_WORD_COLOR_GROUND);
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(win), print_pos.x, print_pos.y, wk->tmpbuf,fontHandle );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PMSIV_EDIT* wk
 *	@param	active_flag 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void set_cursor_anm( PMSIV_EDIT* wk, BOOL active_flag )
{
	if( wk->word_pos_max != 0 )
	{
		if( active_flag )
		{
			GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor[0], ANM_EDITAREA_CURSOR_ACTIVE );
		}
		else
		{
			GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor[0], ANM_EDITAREA_CURSOR_STOP );
		}
	}
	else
	{
		if( active_flag )
		{
			GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor[0], ANM_EDITAREA_ALLCOVER_CURSOR_ACTIVE );
		}
		else
		{
			GFL_CLACT_WK_SetAnmSeq( wk->cursor_actor[0], ANM_EDITAREA_ALLCOVER_CURSOR_STOP );
		}
	}
}


//-----------------------------------------------------------------------------
/**
 *	@brief  BGフレームからOBJ用VRAMTYPEを取得
 *
 *	@param	u8 frame 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static CLSYS_DRAW_TYPE BGFrameToVramType( u8 frame )
{
  GF_ASSERT( frame <= GFL_BG_FRAME3_S );

  if( frame >= GFL_BG_FRAME0_S )
  {
    return CLSYS_DRAW_SUB;
  }

  return CLSYS_DRAW_MAIN;
}

//======================================================================================
//======================================================================================
u32 PMSIV_EDIT_GetWordPosMax( const PMSIV_EDIT* wk )
{
	return wk->word_pos_max;
}

//======================================================================================
//======================================================================================

/**
	*	@brief	メッセージを書き込むウィンドウをチェンジ
	*/
void PMSIV_EDIT_ChangeSMsgWin(PMSIV_EDIT* wk,u8 mode)
{
#ifndef USE_SYSWIN
  return; 
#endif

	//スクリーンをいったんクリア
	GFL_BG_FillScreen(FRM_MAIN_EDITAREA,0x0000,0,20,24,4,GFL_BG_SCRWRT_PALIN);

	GFL_BMPWIN_ClearScreen( *wk->win_message );
	wk->win_message = &(wk->win_msg[mode]);	
	
	BmpWinFrame_Write( *wk->win_message, WINDOW_TRANS_OFF ,wk->msgwin_cgx, PALNUM_MAIN_TALKWIN );
}

//------------------------------------------------------------------
/**
	* 下画面システムメッセージウィンドウに文字列表示
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_EDIT_SetSystemMessage( PMSIV_EDIT* wk, u32 msg_type )
{
	GFL_FONT *fontHandle = fontHandle = PMSIView_GetFontHandle(wk->vwk);
	
#ifndef USE_SYSWIN
  return; 
#endif
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(*wk->win_message),MESSAGE_WIN_COL_GROUND);

	switch( msg_type ){
	case PMSIV_MSG_GUIDANCE:
	#if 1
		{
			WORDSET* wordset = WORDSET_CreateEx(2, 60, HEAPID_PMS_INPUT_VIEW);
			STRBUF*  exbuf = GFL_STR_CreateBuffer( 300, HEAPID_PMS_INPUT_VIEW );
			
			//FIXME 何を入れる？
			//WORDSET_RegisterItemPocketWithIcon( wordset, 0, 0 );
			GFL_MSG_GetString( wk->msgman, info00 + PMSI_GetGuidanceType(wk->mwk), wk->tmpbuf );
			GFL_FONTSYS_SetColor( MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND);
			PRINTSYS_Print( GFL_BMPWIN_GetBmp(*wk->win_message), 0, 0, wk->tmpbuf,fontHandle );

			GFL_STR_DeleteBuffer(exbuf);
			WORDSET_Delete(wordset);
		}
	#else
		MSGMAN_GetString( wk->msgman, info00 + PMSI_GetGuidanceType(wk->mwk), wk->tmpbuf );
		GF_STR_PrintColor( wk->win_message, PMSI_FONT_MESSAGE, wk->tmpbuf, 0, 0, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
				NULL);
	#endif
		break;

	case PMSIV_MSG_CONFIRM_DECIDE:
		GFL_MSG_GetString( wk->msgman, msg_00, wk->tmpbuf );
		GFL_FONTSYS_SetColor( MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND);
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(*wk->win_message), 0, 0, wk->tmpbuf,fontHandle );
		break;

	case PMSIV_MSG_WARN_INPUT:
		GFL_MSG_GetString( wk->msgman, msg_01, wk->tmpbuf );
		GFL_FONTSYS_SetColor( MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND);
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(*wk->win_message), 0, 0, wk->tmpbuf,fontHandle );
		break;

	case PMSIV_MSG_CONFIRM_CANCEL:
		GFL_MSG_GetString( wk->msgman, msg_02, wk->tmpbuf );
		GFL_FONTSYS_SetColor( MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND);
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(*wk->win_message), 0, 0, wk->tmpbuf,fontHandle );
		break;

	}

	GFL_BMPWIN_MakeScreen( *wk->win_message );
	GFL_BMPWIN_TransVramCharacter( *wk->win_message );
}

void PMSIV_EDIT_StopCursor( PMSIV_EDIT* wk )
{
	set_cursor_anm( wk, FALSE );
}
void PMSIV_EDIT_ActiveCursor( PMSIV_EDIT* wk )
{
	set_cursor_anm( wk, TRUE );
}
void PMSIV_EDIT_VisibleCursor( PMSIV_EDIT* wk, BOOL flag )
{
	if(flag && (*wk->p_key_mode == GFL_APP_KTST_TOUCH)){
		return;
	}
	GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor[0], flag );
	GFL_CLACT_WK_SetDrawEnable( wk->cursor_actor[1], flag );
	set_cursor_anm( wk, TRUE );
}


void PMSIV_EDIT_StopArrow( PMSIV_EDIT* wk )
{
#if 0
	if( PMSI_GetInputMode( wk->mwk ) == PMSI_MODE_SENTENCE )
	{
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_left_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_right_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->btn_actor, FALSE );
		GFL_CLACT_WK_SetDrawEnable( wk->bar_actor, FALSE );
	}
#endif
}
void PMSIV_EDIT_ActiveArrow( PMSIV_EDIT* wk )
{
#if 0
	if( PMSI_GetInputMode( wk->mwk ) == PMSI_MODE_SENTENCE )
	{
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_left_actor, TRUE );
		GFL_CLACT_WK_SetDrawEnable( wk->arrow_right_actor, TRUE );
		GFL_CLACT_WK_SetDrawEnable( wk->btn_actor, TRUE );
		GFL_CLACT_WK_SetDrawEnable( wk->bar_actor, TRUE );

		GFL_CLACT_WK_SetAnmSeq( wk->arrow_left_actor, ANM_EDITAREA_SCR_L01 );
		GFL_CLACT_WK_SetAnmSeq( wk->arrow_right_actor, ANM_EDITAREA_SCR_R01 );
	}
#endif
}


void PMSIV_EDIT_MoveCursor( PMSIV_EDIT* wk, int pos )
{
	GFL_CLACTPOS	clPos;
	if( wk->word_pos_max )
	{
		POS  actpos;

		wordpos_to_lcdpos( &wk->word_pos[pos], &actpos );
		clPos.x = actpos.x;
		clPos.y = actpos.y;
	}
	else
	{
		clPos.x = ALLCOVER_CURSOR_XPOS;
		clPos.y = ALLCOVER_CURSOR_YPOS;

//		CLACT_SetMatrix( wk->cursor_actor, &mtx );
	}
	GFL_CLACT_WK_SetPos( wk->cursor_actor[0], &clPos , CLSYS_DEFREND_MAIN );
	clPos.y += GX_LCD_SIZE_Y;
//	clPos.y -= wk->sub_scr;
	GFL_CLACT_WK_SetPos( wk->cursor_actor[1], &clPos , CLSYS_DEFREND_SUB );
	set_cursor_anm( wk, TRUE );
}



//======================================================================================
//======================================================================================


void PMSIV_EDIT_DispYesNoWin( PMSIV_EDIT* wk, int cursor_pos )
{
	TOUCH_SW_PARAM param;

	MI_CpuClear8(&param,sizeof(TOUCH_SW_PARAM));

	param.bg_frame = FRM_MAIN_EDITAREA;
	param.char_offs = wk->yesno_win_cgx;
	param.pltt_offs = PALNUM_MAIN_YESNO;
	param.x		= YESNO_WIN_X;
	param.y		= YESNO_WIN_Y;
	param.kt_st = *wk->p_key_mode;
	param.key_pos = cursor_pos;
	TOUCH_SW_Init( wk->ynbtn_wk, &param );
}

int PMSIV_EDIT_WaitYesNoBtn(PMSIV_EDIT* wk)
{
	int ret;
	int	mode;
	
	ret = TOUCH_SW_Main( wk->ynbtn_wk );
	switch(ret){
	case TOUCH_SW_RET_YES:
		ret = TRUE;
		break;
	case TOUCH_SW_RET_NO:
		ret = FALSE;
		break;
	default:
		return -1;
	}
	//現在の操作モードを取得
	mode = TOUCH_SW_GetKTStatus(wk->ynbtn_wk);
#if 0
	if(mode != wk->key_mode){
		//ステータス切替
		if(wk->key_mode == APP_KTST_KEY){
			(wk->to_key_func)(wk);
		}else{
			(wk->to_touch_func)(wk);
		}
		wk->key_mode = mode;
	}
#endif
	TOUCH_SW_Reset( wk->ynbtn_wk);
	return ret;
}

