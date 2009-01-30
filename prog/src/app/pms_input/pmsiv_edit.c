//============================================================================================
/**
 * @file	pmsiv_edit.c
 * @bfief	簡易会話入力画面（描画下請け：編集欄）
 * @author	taya
 * @date	06.02.07
 */
//============================================================================================
#include "common.h"
#include "gflib\heapsys.h"
#include "gflib\strbuf_family.h"
#include "system\arc_util.h"
#include "system\window.h"
#include "system\buflen.h"
#include "system\pms_word.h"
#include "system\winframe.naix"
#include "system\msgdata.h"
#include "system\bmp_cursor.h"
#include "system\clact_tool.h"
#include "system\touch_subwindow.h"
#include "msgdata\msg_pms_input.h"
#include "msgdata\msg.naix"


#include "pms_input_prv.h"
#include "pms_input_view.h"


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
	EDITAREA_WIN_BASE_COLOR_LETTER = 0x03,
	EDITAREA_WIN_BASE_COLOR_SHADOW = 0x04,
	EDITAREA_WIN_BASE_COLOR_GROUND = 0x0d,

	EDITAREA_WIN_WORD_COLOR_LETTER = 0x05,
	EDITAREA_WIN_WORD_COLOR_SHADOW = 0x06,
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

	GF_BGL_BMPWIN      win_edit[2];
	GF_BGL_BMPWIN      win_msg[2];
	GF_BGL_BMPWIN      win_yesno;
	GF_BGL_BMPWIN      *win_message;
	CLACT_WORK_PTR     cursor_actor[2];
	CLACT_WORK_PTR     arrow_left_actor;
	CLACT_WORK_PTR     arrow_right_actor;
	CLACT_WORK_PTR     bar_actor;
	CLACT_WORK_PTR     btn_actor;

	MSGDATA_MANAGER*   msgman;
	STRBUF*            tmpbuf;
	BMPCURSOR*         bmp_cursor;

	POS                word_pos[PMS_INPUT_WORD_MAX];
	u32                word_pos_max;
	u32                yesnowin_frame_charpos;

	PMS_ANALYZE_WORK   awk;

	u16   pal_data[INPUTAREA_PALTYPE_MAX][16];

	s16					main_scr;
	s16					sub_scr;
	u8					scr_ct;
	u8					scr_dir;

	TOUCH_SW_SYS* ynbtn_wk;

	u16					msgwin_cgx;
	u16					yesno_win_cgx;

	int*				p_key_mode;
};


//==============================================================
// Prototype
//==============================================================
static void setup_pal_datas( PMSIV_EDIT* wk, ARCHANDLE* p_handle );
static void update_editarea_palette( PMSIV_EDIT* wk );
static void setup_wordarea_pos( PMSIV_EDIT* wk );
static void setup_obj( PMSIV_EDIT* wk );
static u32 print_sentence( PMSIV_EDIT* wk ,GF_BGL_BMPWIN* win);
static void setup_pms_analyze( PMS_ANALYZE_WORK* awk, PMSIV_EDIT* wk );
static void cleanup_pms_analyze( PMS_ANALYZE_WORK* awk );
static int prog_pms_analyze( PMS_ANALYZE_WORK* awk, STRBUF* buf );
static void wordpos_to_orgpos( const POS* wordpos, POS* orgpos);
static void wordpos_to_lcdpos( const POS* wordpos, POS* lcdpos );
static void fill_wordarea( GF_BGL_BMPWIN* win, const POS* wordpos );
static void print_wordarea( PMSIV_EDIT* wk, GF_BGL_BMPWIN* win, const POS* wordpos, PMS_WORD word );
static void set_cursor_anm( PMSIV_EDIT* wk, BOOL active_flag );




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
	PMSIV_EDIT*  wk = sys_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_EDIT) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;

	wk->p_key_mode = PMSI_GetKTModePointer(wk->mwk);

	wk->cursor_actor[0] = NULL;
	wk->cursor_actor[1] = NULL;
	wk->arrow_left_actor = NULL;
	wk->arrow_right_actor = NULL;
	wk->bar_actor = NULL;
	wk->btn_actor = NULL;

	wk->tmpbuf = STRBUF_Create( STR_TMPBUF_LEN, HEAPID_PMS_INPUT_VIEW );
	wk->msgman = MSGMAN_Create( MSGMAN_TYPE_NORMAL, ARC_MSG, NARC_msg_pms_input_dat, HEAPID_PMS_INPUT_VIEW );
	wk->bmp_cursor = BMPCURSOR_Create( HEAPID_PMS_INPUT_VIEW );

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
	if( wk->bmp_cursor )
	{
		BMPCURSOR_Delete( wk->bmp_cursor );
	}
	if( wk->cursor_actor[0] )
	{
		CLACT_Delete( wk->cursor_actor[0] );
	}
	if( wk->cursor_actor[1] )
	{
		CLACT_Delete( wk->cursor_actor[1] );
	}
	if( wk->arrow_left_actor )
	{
		CLACT_Delete( wk->arrow_left_actor );
	}
	if( wk->arrow_right_actor )
	{
		CLACT_Delete( wk->arrow_right_actor );
	}
	if( wk->bar_actor )
	{
		CLACT_Delete( wk->bar_actor );
	}
	if( wk->btn_actor )
	{
		CLACT_Delete( wk->btn_actor );
	}
	if( wk->msgman )
	{
		MSGMAN_Delete( wk->msgman );
	}
	if( wk->tmpbuf )
	{
		STRBUF_Delete( wk->tmpbuf );
	}

	GF_BGL_BmpWinDel(&wk->win_edit[0]);
	GF_BGL_BmpWinDel(&wk->win_edit[1]);
	GF_BGL_BmpWinDel(&wk->win_msg[0]);
	GF_BGL_BmpWinDel(&wk->win_msg[1]);
	GF_BGL_BmpWinDel(&wk->win_yesno);

	//YesNoボタンシステムワーク解放
	TOUCH_SW_FreeWork( wk->ynbtn_wk);

	sys_FreeMemoryEz( wk );
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
	GF_BGL_INI* bgl;
	u32 charpos;
	
	bgl = PMSIView_GetBGL( wk->vwk );

	setup_pal_datas( wk, p_handle );

	ArcUtil_HDL_ScrnSet(p_handle, NARC_pmsi_bg_main0_lz_nscr, bgl,
		FRM_MAIN_EDITAREA, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );
	ArcUtil_HDL_ScrnSet(p_handle, NARC_pmsi_bg_main0_lz_nscr, bgl,
		FRM_SUB_EDITAREA, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );

	wk->sub_scr = -24*8;
	GF_BGL_ScrollSet( bgl, FRM_SUB_EDITAREA, GF_BGL_SCROLL_Y_SET,wk->sub_scr);

	charpos = ArcUtil_HDL_BgCharSet(p_handle, NARC_pmsi_bg_main0_lz_ncgr, bgl,
		FRM_MAIN_EDITAREA, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );
	ArcUtil_HDL_BgCharSet(p_handle, NARC_pmsi_bg_main0_lz_ncgr, bgl,
		FRM_SUB_EDITAREA, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );
	charpos /= 0x20;


// BitmapWindow setup
	GF_BGL_BmpWinAdd(	bgl, &wk->win_edit[0], FRM_MAIN_EDITAREA, 
						EDITAREA_WIN_X, EDITAREA_WIN_Y, EDITAREA_WIN_WIDTH, EDITAREA_WIN_HEIGHT,
						PALNUM_MAIN_EDITAREA, charpos );
	GF_BGL_BmpWinAdd(	bgl, &wk->win_edit[1], FRM_SUB_EDITAREA, 
						EDITAREA_WIN_X, EDITAREA_WIN_Y, EDITAREA_WIN_WIDTH, EDITAREA_WIN_HEIGHT,
						PALNUM_MAIN_EDITAREA, charpos );
	charpos += EDITAREA_WIN_CHARSIZE;

	GF_BGL_BmpWinAdd(	bgl, &wk->win_msg[0], FRM_MAIN_EDITAREA, 
						MESSAGE_WIN_X, MESSAGE_WIN_Y, MESSAGE_WIN_WIDTH, MESSAGE_WIN_HEIGHT,
						PALNUM_MAIN_CATEGORY, charpos );
	GF_BGL_BmpWinAdd(	bgl, &wk->win_msg[1], FRM_MAIN_EDITAREA, 
						MESSAGE_WIN_X, MESSAGE_WIN_Y, MESSAGE_WIN_WIDTH-8, MESSAGE_WIN_HEIGHT,
						PALNUM_MAIN_CATEGORY, charpos );
	charpos += MESSAGE_WIN_CHARSIZE;


	GF_BGL_BmpWinAdd(	bgl, &wk->win_yesno, FRM_MAIN_EDITAREA, 
						YESNO_WIN_X, YESNO_WIN_Y, YESNO_WIN_WIDTH, YESNO_WIN_HEIGHT,
						PALNUM_MAIN_CATEGORY, charpos );
	wk->yesno_win_cgx = charpos;
	charpos += TOUCH_SW_USE_CHAR_NUM;//YESNO_WIN_CHARSIZE;

	//YesNoボタンシステムワーク確保
	wk->ynbtn_wk = TOUCH_SW_AllocWork(HEAPID_PMS_INPUT_VIEW);

// window frame setup
	ArcUtil_BgCharSet( ARC_WINFRAME, NARC_winframe_system_ncgr, bgl,
 			FRM_MAIN_EDITAREA, charpos, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
 	wk->yesnowin_frame_charpos = charpos;
	charpos += 9;

	ArcUtil_PalSet( ARC_WINFRAME, NARC_winframe_system_nclr, PALTYPE_MAIN_BG, PALNUM_MAIN_SYSWIN*0x20,
		0x20, HEAPID_PMS_INPUT_VIEW );

	wk->msgwin_cgx = charpos;
	TalkWinGraphicSet( bgl, FRM_MAIN_EDITAREA, charpos, PALNUM_MAIN_TALKWIN,
						PMSI_GetTalkWindowType( wk->mwk ),
						HEAPID_PMS_INPUT_VIEW );

	wk->win_message = &wk->win_msg[0];
	BmpTalkWinScreenSet( wk->win_message, charpos, PALNUM_MAIN_TALKWIN );

	GF_BGL_BmpWinMakeScrn( &wk->win_edit[0] );
	GF_BGL_BmpWinMakeScrn( &wk->win_edit[1] );
	GF_BGL_BmpWinMakeScrn( &wk->win_msg[0]);
	GF_BGL_BmpWinMakeScrn( &wk->win_msg[1] );

	setup_wordarea_pos(wk);
	PMSIV_EDIT_UpdateEditArea( wk );
	PMSIV_EDIT_SetSystemMessage( wk, PMSIV_MSG_GUIDANCE );

	// PMSIV_EDIT_UpdateEditArea で単語数が確定するので、その後に。
	setup_obj( wk );

//	GF_BGL_LoadScreenReq( bgl, FRM_MAIN_EDITAREA );
}

static void pmsiv_edit_hblank(void* vwork)
{
	s32 vc;
	GF_BGL_INI* bgl;
	PMSIV_EDIT* wk = (PMSIV_EDIT*)vwork;

	bgl = PMSIView_GetBGL( wk->vwk );

	vc = GX_GetVCount();

	if(vc < 6*8){
		GF_BGL_ScrollSet( bgl, FRM_MAIN_EDITAREA, GF_BGL_SCROLL_Y_SET,wk->main_scr);
	}else{
		GF_BGL_ScrollSet( bgl, FRM_MAIN_EDITAREA, GF_BGL_SCROLL_Y_SET,0);
	}
}

void PMSIV_EDIT_ScrollSet( PMSIV_EDIT* wk,u8 scr_dir)
{
	wk->scr_ct = 0;
	wk->scr_dir = scr_dir;

	sys_HBlankIntrSet( pmsiv_edit_hblank, wk);
}

int PMSIV_EDIT_ScrollWait( PMSIV_EDIT* wk)
{
	s16 dir;
	GF_BGL_INI* bgl;
	
	if(wk->scr_ct > 5){
		return TRUE;
	}
	
	bgl = PMSIView_GetBGL( wk->vwk );

	if(wk->scr_dir){
		dir = -8;
	}else{
		dir = 8;
	}
	wk->sub_scr += dir;
	wk->main_scr += dir;
	GF_BGL_ScrollSet( bgl, FRM_MAIN_EDITAREA, GF_BGL_SCROLL_Y_SET,wk->main_scr);
	GF_BGL_ScrollSet( bgl, FRM_SUB_EDITAREA, GF_BGL_SCROLL_Y_SET,wk->sub_scr);
	
	CATS_ObjectPosMove(wk->cursor_actor[0],0,-dir);
	CATS_ObjectPosMove(wk->cursor_actor[1],0,-dir);

	wk->scr_ct++;

	if(wk->scr_dir && (wk->scr_ct == 6)){
		sys_HBlankIntrSet(NULL,NULL);
		sys_HBlankIntrStop();
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
	ArcUtil_HDL_PalSet( p_handle, NARC_pmsi_bg_main_nclr, PALTYPE_MAIN_BG,
		 0, 14*0x20, HEAPID_PMS_INPUT_VIEW );
	ArcUtil_HDL_PalSet( p_handle, NARC_pmsi_bg_sub_nclr, PALTYPE_SUB_BG,
		 0, 3*0x20, HEAPID_PMS_INPUT_VIEW );

	// 後の色変え用にヒープにも読み込んでおく
	data_ptr = ArcUtil_HDL_PalDataGet( p_handle, NARC_pmsi_pms_bgm_dat_nclr, &palDat, HEAPID_PMS_INPUT_VIEW );

	MI_CpuCopy16( palDat->pRawData, wk->pal_data, sizeof(wk->pal_data) );
	DC_FlushRange( wk->pal_data, sizeof(wk->pal_data) );


	sys_FreeMemoryEz( data_ptr );
}

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
	CLACT_HEADER  header;
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

	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_MAIN, BGPRI_MAIN_EDITAREA );
	wk->cursor_actor[0] = PMSIView_AddActor( wk->vwk, &header, actpos.x, actpos.y,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DMAIN );
	wk->cursor_actor[1] = PMSIView_AddActor( wk->vwk, &header, actpos.x, actpos.y-wk->sub_scr,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DSUB );
	CLACT_AnmChg( wk->cursor_actor[1], 1);

	set_cursor_anm( wk, TRUE );

	wk->arrow_left_actor = PMSIView_AddActor( wk->vwk, &header, LEFT_ARROW_OBJ_XPOS, LEFT_ARROW_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	CLACT_AnmChg( wk->arrow_left_actor, ANM_EDITAREA_SCR_L01 );

	wk->arrow_right_actor = PMSIView_AddActor( wk->vwk, &header, RIGHT_ARROW_OBJ_XPOS, RIGHT_ARROW_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	CLACT_AnmChg( wk->arrow_right_actor, ANM_EDITAREA_SCR_R01 );
	
	wk->btn_actor = PMSIView_AddActor( wk->vwk, &header, BTN_OBJ_XPOS, BTN_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	CLACT_AnmChg( wk->btn_actor, ANM_EDITAREA_SCR_BTN );
	
	wk->bar_actor = PMSIView_AddActor( wk->vwk, &header, BAR_OBJ_XPOS, BAR_OBJ_YPOS,
			ACTPRI_EDITAREA_ARROW, NNS_G2D_VRAM_TYPE_2DMAIN );

	CLACT_AnmChg( wk->bar_actor, ANM_EDITAREA_SCR_BAR );

	if( PMSI_GetInputMode( wk->mwk ) != PMSI_MODE_SENTENCE )
	{
		CLACT_SetDrawFlag( wk->arrow_right_actor, FALSE );
		CLACT_SetDrawFlag( wk->arrow_left_actor, FALSE );
		CLACT_SetDrawFlag( wk->bar_actor, FALSE );
		CLACT_SetDrawFlag( wk->btn_actor, FALSE );
	}

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
	GF_BGL_INI* bgl;
	
	bgl = PMSIView_GetBGL( wk->vwk );

	update_editarea_palette( wk );

	for(i = 0;i < 2;i++){
		GF_BGL_BmpWinDataFill( &wk->win_edit[i], EDITAREA_WIN_BASE_COLOR_GROUND );

		switch(PMSI_GetInputMode(wk->mwk)){
		case PMSI_MODE_SINGLE:
			fill_wordarea( &wk->win_edit[i], &wk->word_pos[0] );
			print_wordarea( wk, &wk->win_edit[i], &(wk->word_pos[0]), PMSI_GetEditWord(wk->mwk, 0) );
			break;
		case PMSI_MODE_DOUBLE:
			fill_wordarea( &wk->win_edit[i], &wk->word_pos[0] );
			fill_wordarea( &wk->win_edit[i], &wk->word_pos[1] );
			print_wordarea( wk, &wk->win_edit[i], &(wk->word_pos[0]), PMSI_GetEditWord(wk->mwk, 0) );
			print_wordarea( wk, &wk->win_edit[i], &(wk->word_pos[1]), PMSI_GetEditWord(wk->mwk, 1) );
			break;
		case PMSI_MODE_SENTENCE:
			wk->word_pos_max = print_sentence( wk ,&wk->win_edit[i]);
			OS_TPrintf("word max:%d\n", wk->word_pos_max);
			break;
		}

		GF_BGL_BmpWinCgxOn( &wk->win_edit[i] );
	}
	GF_BGL_LoadScreenV_Req( bgl, FRM_MAIN_EDITAREA );
	GF_BGL_LoadScreenV_Req( bgl, FRM_SUB_EDITAREA );
}


/**
 *	@brief	ワードポジションの判定矩形を返す
 */
void PMSIV_EDIT_GetSentenceWordPos( PMSIV_EDIT* wk ,RECT_HIT_TBL* tbl,u8 idx)
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
static u32 print_sentence( PMSIV_EDIT* wk ,GF_BGL_BMPWIN* win)
{
	STRBUF* buf = PMSI_GetEditSourceString( wk->mwk, HEAPID_PMS_INPUT_VIEW );
	int x, y, word_pos;
	BOOL cont_flag;
	PMS_WORD  word_code;

	x = y = word_pos = 0;
	cont_flag = TRUE;

	setup_pms_analyze( &wk->awk, wk );

	while( cont_flag )
	{
		switch( prog_pms_analyze( &(wk->awk), buf ) ){
		case PMS_ANALYZE_RESULT_STR:
			GF_STR_PrintColor( win, PMSI_FONT_EDITAREA_BASE, buf, x, y, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(EDITAREA_WIN_BASE_COLOR_LETTER, EDITAREA_WIN_BASE_COLOR_SHADOW, EDITAREA_WIN_BASE_COLOR_GROUND),
				NULL);
			x += FontProc_GetPrintStrWidth(PMSI_FONT_EDITAREA_BASE, buf, 0);
			break;

		case PMS_ANALYZE_RESULT_TAG:
			x += WORDAREA_X_MARGIN;
			wk->word_pos[word_pos].x = x + (WORDAREA_WIDTH/2);
			wk->word_pos[word_pos].y = y + (WORDAREA_HEIGHT/2);
			fill_wordarea( win, &(wk->word_pos[word_pos]) );
			word_code = PMSI_GetEditWord(wk->mwk, word_pos);
			if( word_code != PMS_WORD_NULL )
			{
				print_wordarea( wk, win, &(wk->word_pos[word_pos]), word_code );
			}
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
	STRBUF_Delete( buf );

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
	awk->p = STRBUF_GetStringCodePointer( awk->src );

	if( *(awk->p) == _CTRL_TAG )
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
	STRBUF_Delete( awk->src );
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
			case CR_:
				awk->state = PMS_ANALYZE_STATE_CR;
				break;
			case EOM_:
				awk->state = PMS_ANALYZE_STATE_END;
				break;
			case _CTRL_TAG:
				awk->state = PMS_ANALYZE_STATE_TAG;
				break;
			default:
				awk->p++;
				break;
			}
		}
		STRBUF_SetStringCodeOrderLength( buf, bp, (awk->p-bp)+1 );
		return PMS_ANALYZE_RESULT_STR;

	case PMS_ANALYZE_STATE_TAG:
		awk->p = STRCODE_SkipTag(awk->p);
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
	case CR_:
		awk->state = PMS_ANALYZE_STATE_CR;
		break;
	case EOM_:
		awk->state = PMS_ANALYZE_STATE_END;
		break;
	case _CTRL_TAG:
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

static void fill_wordarea( GF_BGL_BMPWIN* win, const POS* wordpos )
{
	POS   drawpos;

	wordpos_to_orgpos( wordpos, &drawpos );

	GF_BGL_BmpWinFill( win, EDITAREA_WIN_WORD_COLOR_GROUND,
			drawpos.x, drawpos.y,
			WORDAREA_WIDTH, WORDAREA_HEIGHT );
}

static void print_wordarea( PMSIV_EDIT* wk, GF_BGL_BMPWIN* win, const POS* wordpos, PMS_WORD word )
{
	if( word != PMS_WORD_NULL )
	{
		POS  print_pos;
		u32  write_xpos;

		wordpos_to_orgpos( wordpos, &print_pos );

		PMSW_GetStr(word, wk->tmpbuf);
		write_xpos = (print_pos.x + (WORDAREA_WIDTH / 2)) - (FontProc_GetPrintStrWidth(PMSI_FONT_EDITAREA_WORD, wk->tmpbuf, 0) / 2);
		GF_STR_PrintColor( win, PMSI_FONT_EDITAREA_WORD, wk->tmpbuf, print_pos.x, print_pos.y, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(EDITAREA_WIN_WORD_COLOR_LETTER, EDITAREA_WIN_WORD_COLOR_SHADOW, EDITAREA_WIN_WORD_COLOR_GROUND),
				NULL);
	}
}


//======================================================================================
//======================================================================================
u32 PMSIV_EDIT_GetWordPosMax( const PMSIV_EDIT* wk )
{
	return wk->word_pos_max;
}

//======================================================================================
//======================================================================================
#include "system\wordset.h"

/**
 *	@brief	メッセージを書き込むウィンドウをチェンジ
 */
void PMSIV_EDIT_ChangeSMsgWin(PMSIV_EDIT* wk,u8 mode)
{
	GF_BGL_INI* bgl;
	bgl = PMSIView_GetBGL( wk->vwk );

	//スクリーンをいったんクリア
	GF_BGL_ScrFill(bgl,FRM_MAIN_EDITAREA,0x0000,0,20,24,4,GF_BGL_SCRWRT_PALIN);

	GF_BGL_BmpWinOff(wk->win_message);
	wk->win_message = &(wk->win_msg[mode]);	
	
	BmpTalkWinScreenSet( wk->win_message, wk->msgwin_cgx, PALNUM_MAIN_TALKWIN );
}

//------------------------------------------------------------------
/**
 * 
 *
 * @param   wk		
 *
 */
//------------------------------------------------------------------
void PMSIV_EDIT_SetSystemMessage( PMSIV_EDIT* wk, u32 msg_type )
{
	GF_BGL_BmpWinDataFill( wk->win_message, MESSAGE_WIN_COL_GROUND);

	switch( msg_type ){
	case PMSIV_MSG_GUIDANCE:
	#if 1
		{
			WORDSET* wordset = WORDSET_CreateEx(2, 60, HEAPID_BASE_SYSTEM);
			STRBUF*  exbuf = STRBUF_Create( 300, HEAPID_BASE_SYSTEM );

			WORDSET_RegisterItemPocketWithIcon( wordset, 0, 0 );
			MSGMAN_GetString( wk->msgman, info00 + PMSI_GetGuidanceType(wk->mwk), wk->tmpbuf );
			WORDSET_ExpandStr( wordset, exbuf, wk->tmpbuf );
			GF_STR_PrintColor( wk->win_message, PMSI_FONT_MESSAGE, exbuf, 0, 0, MSG_NO_PUT,
					GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
					NULL);


			STRBUF_Delete(exbuf);
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
		MSGMAN_GetString( wk->msgman, msg_00, wk->tmpbuf );
		GF_STR_PrintColor( wk->win_message, PMSI_FONT_MESSAGE, wk->tmpbuf, 0, 0, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
				NULL);
		break;

	case PMSIV_MSG_WARN_INPUT:
		MSGMAN_GetString( wk->msgman, msg_01, wk->tmpbuf );
		GF_STR_PrintColor( wk->win_message, PMSI_FONT_MESSAGE, wk->tmpbuf, 0, 0, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
				NULL);
		break;

	case PMSIV_MSG_CONFIRM_CANCEL:
		MSGMAN_GetString( wk->msgman, msg_02, wk->tmpbuf );
		GF_STR_PrintColor( wk->win_message, PMSI_FONT_MESSAGE, wk->tmpbuf, 0, 0, MSG_NO_PUT,
				GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
				NULL);
		break;

	}

	GF_BGL_BmpWinOn( wk->win_message );

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
	if(flag && (*wk->p_key_mode == APP_KTST_TOUCH)){
		return;
	}
	CLACT_SetDrawFlag( wk->cursor_actor[0], flag );
	CLACT_SetDrawFlag( wk->cursor_actor[1], flag );
	set_cursor_anm( wk, TRUE );
}


void PMSIV_EDIT_StopArrow( PMSIV_EDIT* wk )
{
	if( PMSI_GetInputMode( wk->mwk ) == PMSI_MODE_SENTENCE )
	{
		CLACT_SetDrawFlag( wk->arrow_left_actor, FALSE );
		CLACT_SetDrawFlag( wk->arrow_right_actor, FALSE );
		CLACT_SetDrawFlag( wk->btn_actor, FALSE );
		CLACT_SetDrawFlag( wk->bar_actor, FALSE );
	}
}
void PMSIV_EDIT_ActiveArrow( PMSIV_EDIT* wk )
{
	if( PMSI_GetInputMode( wk->mwk ) == PMSI_MODE_SENTENCE )
	{
		CLACT_SetDrawFlag( wk->arrow_left_actor, TRUE );
		CLACT_SetDrawFlag( wk->arrow_right_actor, TRUE );
		CLACT_SetDrawFlag( wk->btn_actor, TRUE );
		CLACT_SetDrawFlag( wk->bar_actor, TRUE );

		CLACT_AnmChg( wk->arrow_left_actor, ANM_EDITAREA_SCR_L01 );
		CLACT_AnmChg( wk->arrow_right_actor, ANM_EDITAREA_SCR_R01 );
	}
}


void PMSIV_EDIT_MoveCursor( PMSIV_EDIT* wk, int pos )
{
	VecFx32  mtx;
	if( wk->word_pos_max )
	{
		POS  actpos;

		wordpos_to_lcdpos( &wk->word_pos[pos], &actpos );
		mtx.x = actpos.x * FX32_ONE;
		mtx.y = actpos.y * FX32_ONE;
		mtx.z = 0;
	}
	else
	{
		mtx.x = ALLCOVER_CURSOR_XPOS * FX32_ONE;
		mtx.y = ALLCOVER_CURSOR_YPOS * FX32_ONE;
		mtx.z = 0;

//		CLACT_SetMatrix( wk->cursor_actor, &mtx );
	}
	CLACT_SetMatrix( wk->cursor_actor[0], &mtx );
	mtx.y += FX32_CONST(192-wk->sub_scr);
	CLACT_SetMatrix( wk->cursor_actor[1], &mtx );
	set_cursor_anm( wk, TRUE );
}


static void set_cursor_anm( PMSIV_EDIT* wk, BOOL active_flag )
{
	if( wk->word_pos_max != 0 )
	{
		if( active_flag )
		{
			CLACT_AnmChg( wk->cursor_actor[0], ANM_EDITAREA_CURSOR_ACTIVE );
		}
		else
		{
			CLACT_AnmChg( wk->cursor_actor[0], ANM_EDITAREA_CURSOR_STOP );
		}
	}
	else
	{
		if( active_flag )
		{
			CLACT_AnmChg( wk->cursor_actor[0], ANM_EDITAREA_ALLCOVER_CURSOR_ACTIVE );
		}
		else
		{
			CLACT_AnmChg( wk->cursor_actor[0], ANM_EDITAREA_ALLCOVER_CURSOR_STOP );
		}
	}
}



//======================================================================================
//======================================================================================


void PMSIV_EDIT_DispYesNoWin( PMSIV_EDIT* wk, int cursor_pos )
{
#if 0
	GF_BGL_BmpWinDataFill( &wk->win_yesno, YESNOWIN_COL_GROUND );

	MSGMAN_GetString( wk->msgman, str_yes, wk->tmpbuf );
	GF_STR_PrintColor( &wk->win_yesno, PMSI_FONT_YESNO, wk->tmpbuf, YESNOWIN_STR_OX, YESNOWIN_STR_OY, MSG_NO_PUT,
			GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
			NULL);

	MSGMAN_GetString( wk->msgman, str_no, wk->tmpbuf );
	GF_STR_PrintColor( &wk->win_yesno, PMSI_FONT_YESNO, wk->tmpbuf, YESNOWIN_STR_OX, YESNOWIN_STR_OY+YESONOWIN_LINE_HEIGHT, MSG_NO_PUT,
			GF_PRINTCOLOR_MAKE(MESSAGE_WIN_COL_LETTER, MESSAGE_WIN_COL_SHADOW, MESSAGE_WIN_COL_GROUND),
			NULL);

	BmpMenuWinWrite( &wk->win_yesno, FALSE, wk->yesnowin_frame_charpos, PALNUM_MAIN_SYSWIN );
	BMPCURSOR_Print( wk->bmp_cursor, &wk->win_yesno, YESNOWIN_CURSOR_OX, YESNOWIN_CURSOR_OY+(cursor_pos*YESONOWIN_LINE_HEIGHT) );

	GF_BGL_BmpWinOn( &wk->win_yesno );
#else
	GF_BGL_INI* bgl;
	TOUCH_SW_PARAM param;

	bgl = PMSIView_GetBGL( wk->vwk );
	MI_CpuClear8(&param,sizeof(TOUCH_SW_PARAM));

	param.p_bgl  = bgl;
	param.bg_frame = FRM_MAIN_EDITAREA;
	param.char_offs = wk->yesno_win_cgx;
	param.pltt_offs = PALNUM_MAIN_YESNO;
	param.x		= YESNO_WIN_X;
	param.y		= YESNO_WIN_Y;
	param.kt_st = *wk->p_key_mode;
	param.key_pos = cursor_pos;
	TOUCH_SW_Init( wk->ynbtn_wk, &param );
#endif
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

void PMSIV_EDIT_MoveYesNoCursor( PMSIV_EDIT* wk, int pos )
{
	GF_BGL_BmpWinFill( &wk->win_yesno, YESNOWIN_COL_GROUND, 0, 0, YESNOWIN_STR_OX, YESNO_WIN_HEIGHT*8 );
	BMPCURSOR_Print( wk->bmp_cursor, &wk->win_yesno, YESNOWIN_CURSOR_OX, YESNOWIN_CURSOR_OY+(pos*YESONOWIN_LINE_HEIGHT) );
}

void PMSIV_EDIT_EraseYesNoWin( PMSIV_EDIT* wk )
{
	BmpMenuWinClear( &wk->win_yesno, FALSE );
	GF_BGL_BmpWinOff( &wk->win_yesno );
}
