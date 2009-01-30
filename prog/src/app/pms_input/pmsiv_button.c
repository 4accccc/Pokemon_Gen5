//============================================================================================
/**
 * @file	pmsiv_button.c
 * @bfief	簡易会話入力画面（描画下請け：コマンドボタン）
 * @author	taya
 * @date	06.02.07
 */
//============================================================================================
#include "common.h"
#include "gflib\heapsys.h"
#include "gflib\strbuf.h"
#include "system\arc_util.h"
#include "system\window.h"
#include "system\buflen.h"
#include "system\pms_word.h"
#include "system\winframe.naix"
#include "system\msgdata.h"
#include "msgdata\msg_pms_input_button.h"
#include "msgdata\msg.naix"


#include "pms_input_prv.h"
#include "pms_input_view.h"


//======================================================================

enum {
	DECIDE_WIN_X = 24,
	DECIDE_WIN_Y = 14,
	DECIDE_WIN_WIDTH = 7,
	DECIDE_WIN_HEIGHT = 2,
	DECIDE_WIN_CHARSIZE = DECIDE_WIN_WIDTH*DECIDE_WIN_HEIGHT,
	DECIDE_WIN_CHARPOS = 1023-DECIDE_WIN_CHARSIZE,

	CANCEL_WIN_X = DECIDE_WIN_X,
	CANCEL_WIN_Y = DECIDE_WIN_Y+DECIDE_WIN_HEIGHT+1,
	CANCEL_WIN_WIDTH = DECIDE_WIN_WIDTH,
	CANCEL_WIN_HEIGHT = DECIDE_WIN_HEIGHT,
	CANCEL_WIN_CHARPOS = DECIDE_WIN_CHARPOS - DECIDE_WIN_CHARSIZE,
	
	RET_WIN_X = 25,
	RET_WIN_Y = 21,
	RET_WIN_WIDTH = 6,
	RET_WIN_HEIGHT = 2,
	RET_WIN_CHARSIZE = RET_WIN_WIDTH*RET_WIN_HEIGHT,
	RET_WIN_CHARPOS = CANCEL_WIN_CHARPOS-DECIDE_WIN_CHARSIZE,

	DECIDE_WIN_CX = (DECIDE_WIN_X + DECIDE_WIN_WIDTH/2)*8,
	DECIDE_WIN_CY = (DECIDE_WIN_Y + DECIDE_WIN_HEIGHT/2)*8,

	CANCEL_WIN_CX = (CANCEL_WIN_X + CANCEL_WIN_WIDTH/2)*8,
	CANCEL_WIN_CY = (CANCEL_WIN_Y + CANCEL_WIN_HEIGHT/2)*8,

	DECIDE_CURSOR_XPOS = ((DECIDE_WIN_X*8) + (DECIDE_WIN_WIDTH*8)/2),
	DECIDE_CURSOR_YPOS = ((DECIDE_WIN_Y*8) + (DECIDE_WIN_HEIGHT*8)/2),

	CANCEL_CURSOR_XPOS = ((CANCEL_WIN_X*8) + (CANCEL_WIN_WIDTH*8)/2),
	CANCEL_CURSOR_YPOS = ((CANCEL_WIN_Y*8) + (CANCEL_WIN_HEIGHT*8)/2),

#if 0
	DCWIN_SCR_X = DECIDE_WIN_X-1,
	DCWIN_SCR_Y = DECIDE_WIN_Y-1,
	DCWIN_SCR_W = DECIDE_WIN_WIDTH+2,
	DCWIN_SCR_H = DECIDE_WIN_HEIGHT*2+3,
#else
	DCWIN_SCR_X = 23,
	DCWIN_SCR_Y = 13,
	DCWIN_SCR_W = 9,
	DCWIN_SCR_H = 7,
#endif

	MCHG_BTN_PTN = 2,
	MCHG_BTN_W = 8,
	MCHG_BTN_H = 6,
	MCHG_BTN_X = 2,
	MCHG_BTN_Y = 0,
	MCHG_BTN_OX = 21,
	MCHG_BTN_SH = MCHG_BTN_H*MCHG_BTN_PTN,
	MCHG_BTN_DOY = 0,
};

enum {
	WIN_COL_LETTER = 0x01,
	WIN_COL_SHADOW = 0x02,
	WIN_COL_GROUND = 0x0e,
};

//======================================================================


//======================================================================
//--------------------------------------------------------------
/**
 *	
 */
//--------------------------------------------------------------
struct _PMSIV_BUTTON {
	PMS_INPUT_VIEW*        vwk;
	const PMS_INPUT_WORK*  mwk;
	const PMS_INPUT_DATA*  dwk;

	CLACT_WORK_PTR    cursor_actor;
	
	GF_BGL_INI		*bgl;
	MSGDATA_MANAGER*   msgman;

	STRBUF*			str[3];
	GF_BGL_BMPWIN	win[3];

	void*	pSBufBtn;
	void*	pSBufWin;
	NNSG2dScreenData*	pSBtn;	
	NNSG2dScreenData*	pSWin;	

	TCB_PTR       changeButtonTask;
};

typedef struct {
	PMSIV_BUTTON*  wk;
	int seq;
	u16 timer;
	u16	mode;
	u16 on_scrnID;
	u16 on_scrnID_end;
	u16 on_scrn_x;
	u16 on_scrn_y;

	u16 off_scrnID;
	u16 off_scrn_x;
	u16 off_scrn_y;

}CHANGE_BUTTON_WORK;


//==============================================================
// Prototype
//==============================================================
void PMSIV_BUTTON_ReleaseGraphicDatas( PMSIV_BUTTON* wk );
void PMSIV_BUTTON_UpdateButton( PMSIV_BUTTON* wk,BOOL draw_f,BOOL cur_f);
static void setup_button_win( PMSIV_BUTTON* wk,GF_BGL_INI* bgl);
static void setup_obj( PMSIV_BUTTON* wk );
static void draw_mchg_button(PMSIV_BUTTON* wk,u8 mode,u8 ptn);



//------------------------------------------------------------------
/**
 * 
 *
 * @param   vwk		
 * @param   mwk		
 * @param   dwk		
 *
 * @retval  PMSIV_BUTTON*		
 */
//------------------------------------------------------------------
PMSIV_BUTTON*  PMSIV_BUTTON_Create( PMS_INPUT_VIEW* vwk, const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk )
{
	PMSIV_BUTTON*  wk = sys_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_BUTTON) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;

	wk->cursor_actor = NULL;
	wk->changeButtonTask = NULL;
	wk->bgl = PMSIView_GetBGL( wk->vwk );
	
	wk->msgman = MSGMAN_Create( MSGMAN_TYPE_NORMAL, ARC_MSG,
			NARC_msg_pms_input_button_dat, HEAPID_PMS_INPUT_VIEW );
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
void PMSIV_BUTTON_Delete( PMSIV_BUTTON* wk )
{
	PMSIV_BUTTON_ReleaseGraphicDatas( wk );
	
	MSGMAN_Delete( wk->msgman);
	sys_FreeMemoryEz( wk );
}



//------------------------------------------------------------------
/**
 * @brief	けってい/やめる　ボタンリソース初期化 
 *
 * @param   wk		
 *
 */
//------------------------------------------------------------------
void PMSIV_BUTTON_SetupGraphicDatas( PMSIV_BUTTON* wk, ARCHANDLE* p_handle )
{
	GF_BGL_INI* bgl;

	bgl = PMSIView_GetBGL( wk->vwk );

	// screen, cgx
	ArcUtil_HDL_ScrnSet(p_handle, NARC_pmsi_bg_main3_lz_nscr, bgl,
		FRM_MAIN_BACK, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );

	ArcUtil_HDL_BgCharSet(p_handle, NARC_pmsi_bg_main3_lz_ncgr, bgl,
		FRM_MAIN_BACK, 0, 0, TRUE, HEAPID_PMS_INPUT_VIEW );

	// button screen
	wk->pSBufBtn = ArcUtil_HDL_ScrnDataGet(p_handle, NARC_pmsi_pms_bg_main01_nscr,FALSE,
		&wk->pSBtn, HEAPID_PMS_INPUT_VIEW);
	wk->pSBufWin = ArcUtil_HDL_ScrnDataGet(p_handle, NARC_pmsi_pms_bg_m3dat_nscr,FALSE,
		&wk->pSWin, HEAPID_PMS_INPUT_VIEW);

	// bmp window
	setup_button_win(wk, bgl);
	setup_obj( wk );

	PMSIV_BUTTON_UpdateButton(wk,TRUE,FALSE);
//	PMSIV_BUTTON_UpdateRetButton(wk,0);

	//モード書き込み
	draw_mchg_button(wk,0,1);
	draw_mchg_button(wk,1,0);

	GF_BGL_LoadScreenReq( bgl, FRM_MAIN_BACK );
}

//------------------------------------------------------------------
/**
 * @brief	けってい/やめる　ボタンリソース解放
 *
 * @param   wk		
 *
 */
//------------------------------------------------------------------
void PMSIV_BUTTON_ReleaseGraphicDatas( PMSIV_BUTTON* wk )
{
	int i;
	GF_BGL_INI* bgl = PMSIView_GetBGL( wk->vwk );
	
	if( wk->cursor_actor )
	{
		CLACT_Delete( wk->cursor_actor );
	}

	for(i = 0;i < 3;i++){
		GF_BGL_BmpWinOff(&wk->win[i]);
		GF_BGL_BmpWinDel(&wk->win[i]);
		STRBUF_Delete(wk->str[i]);
	}
	sys_FreeMemoryEz( wk->pSBufWin);
	sys_FreeMemoryEz( wk->pSBufBtn);

	GF_BGL_LoadScreenReq( bgl, FRM_MAIN_BACK );
}

/**
 *	@brief	けってい/やめる　ボタンの描画状態コントロール
 *	@param	draw_f	ボタン本体の描画On/Off
 *	@param	cur_f	選択カーソルの描画On/Off
 */
void PMSIV_BUTTON_UpdateButton( PMSIV_BUTTON* wk,BOOL draw_f,BOOL cur_f)
{
	int i,siz;
	GF_BGL_INI* bgl;
	bgl = PMSIView_GetBGL( wk->vwk );

	GF_BGL_ScrFill(bgl,FRM_MAIN_BUTTON,0x0000,DCWIN_SCR_X,DCWIN_SCR_Y,
			DCWIN_SCR_W,DCWIN_SCR_H,GF_BGL_SCRWRT_PALIN);
	
	PMSIV_BUTTON_VisibleCursor(wk,cur_f);
	if(!draw_f){
		GF_BGL_LoadScreenReq(bgl,FRM_MAIN_BUTTON);
		return;
	}
	GF_BGL_ScrWriteExpand(bgl,FRM_MAIN_BUTTON,
		DCWIN_SCR_X,DCWIN_SCR_Y,
		DCWIN_SCR_W,DCWIN_SCR_H,
		wk->pSBtn->rawData,0,0,
		wk->pSBtn->screenWidth/8,wk->pSBtn->screenHeight/8);

	for(i = 0;i < 2;i++){
		GF_BGL_BmpWinDataFill(&wk->win[i],WIN_COL_GROUND);
		
		siz = (DECIDE_WIN_WIDTH*8)-FontProc_GetPrintStrWidth( FONT_BUTTON,wk->str[i], 0 );

		GF_STR_PrintColor(&wk->win[i], FONT_BUTTON, wk->str[i],siz/2,0, MSG_ALLPUT,
			GF_PRINTCOLOR_MAKE(WIN_COL_LETTER,WIN_COL_SHADOW,WIN_COL_GROUND),
			NULL);
	}
	GF_BGL_LoadScreenReq(bgl,FRM_MAIN_BUTTON);
}

/**
 *	@brief	戻るボタンの描画状態コントロール
 *	@param	draw_f	ボタン本体の描画On/Off
 *	@param	cur_f	選択カーソルの描画On/Off
 */
void PMSIV_BUTTON_UpdateRetButton( PMSIV_BUTTON* wk,u8 draw_mode)
{
	int siz;
	GF_BGL_INI* bgl;
	bgl = PMSIView_GetBGL( wk->vwk );
	if(draw_mode == 0){
		GF_BGL_ScrFill(bgl,FRM_MAIN_BUTTON,0x0000,RET_WIN_X-1,RET_WIN_Y-1,
			RET_WIN_WIDTH+2,RET_WIN_HEIGHT+2,GF_BGL_SCRWRT_PALIN);
		GF_BGL_BmpWinOff(&wk->win[2]);
		GF_BGL_LoadScreenV_Req(bgl,FRM_MAIN_BUTTON);
		return;
	}
	GF_BGL_ScrWriteExpand(bgl,FRM_MAIN_BUTTON,
		RET_WIN_X-1,RET_WIN_Y-1,
		RET_WIN_WIDTH+2,RET_WIN_HEIGHT+2,
		wk->pSBtn->rawData,0,DCWIN_SCR_H,
		wk->pSBtn->screenWidth/8,wk->pSBtn->screenHeight/8);

	GF_BGL_BmpWinDataFill(&wk->win[2],WIN_COL_GROUND);
		
	siz = (RET_WIN_WIDTH*8)-FontProc_GetPrintStrWidth( FONT_BUTTON,wk->str[2], 0 );

	GF_STR_PrintColor(&wk->win[2], FONT_BUTTON, wk->str[2],siz/2,0, MSG_ALLPUT,
		GF_PRINTCOLOR_MAKE(WIN_COL_LETTER,WIN_COL_SHADOW,WIN_COL_GROUND),
		NULL);
	GF_BGL_LoadScreenV_Req(bgl,FRM_MAIN_BUTTON);
}


static void setup_button_win( PMSIV_BUTTON* wk,GF_BGL_INI* bgl)
{
	u32 xpos,i;

	static const BMPWIN_DAT  windat[3] = {
	 {
		FRM_MAIN_BUTTON,
		DECIDE_WIN_X, DECIDE_WIN_Y,
		DECIDE_WIN_WIDTH, DECIDE_WIN_HEIGHT,
		PALNUM_MAIN_BUTTON, DECIDE_WIN_CHARPOS
	 },
	 {
		FRM_MAIN_BUTTON,
		CANCEL_WIN_X, CANCEL_WIN_Y,
		CANCEL_WIN_WIDTH, CANCEL_WIN_HEIGHT,
		PALNUM_MAIN_BUTTON, CANCEL_WIN_CHARPOS
	 },
	 {
		FRM_MAIN_BUTTON,
		RET_WIN_X, RET_WIN_Y,
		RET_WIN_WIDTH, RET_WIN_HEIGHT,
		PALNUM_MAIN_BUTTON, RET_WIN_CHARPOS
	 }
	};

	for(i = 0;i < 3;i++){
		GF_BGL_BmpWinAddEx( bgl, &wk->win[i], &windat[i] );

		wk->str[i] = MSGMAN_AllocString( wk->msgman,button_decide+i);

#if 0
	xpos = ((windat->siz_x*8) - FontProc_GetPrintStrWidth(PMSI_FONT_BUTTON, str, 0)) / 2;

	GF_STR_PrintColor( &win, PMSI_FONT_BUTTON, str, xpos, 0, MSG_NO_PUT,
			GF_PRINTCOLOR_MAKE(WIN_COL_LETTER, WIN_COL_SHADOW, WIN_COL_GROUND),
			NULL);

	GF_BGL_BmpWinCgxOn(&win);
#endif
	}
	GF_BGL_BmpWinMakeScrn( &wk->win[0] );
	GF_BGL_BmpWinDataFill( &wk->win[0], WIN_COL_GROUND );
	GF_BGL_BmpWinMakeScrn( &wk->win[1] );
	GF_BGL_BmpWinDataFill( &wk->win[1], WIN_COL_GROUND );
}

static void setup_obj( PMSIV_BUTTON* wk )
{
	CLACT_HEADER  header;

	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_MAIN, BGPRI_MAIN_BUTTON );
	wk->cursor_actor = PMSIView_AddActor( wk->vwk, &header, DECIDE_CURSOR_XPOS, DECIDE_CURSOR_YPOS,
			ACTPRI_BUTTON_CURSOR, NNS_G2D_VRAM_TYPE_2DMAIN );

	CLACT_AnmChg( wk->cursor_actor, ANM_BUTTON_CURSOR_ACTIVE );
	CLACT_SetDrawFlag( wk->cursor_actor, FALSE );

}



//======================================================================================
//======================================================================================

void PMSIV_BUTTON_StopCursor( PMSIV_BUTTON* wk )
{
	CLACT_AnmChg( wk->cursor_actor, ANM_BUTTON_CURSOR_STOP );
}
void PMSIV_BUTTON_ActiveCursor( PMSIV_BUTTON* wk )
{
	CLACT_AnmChg( wk->cursor_actor, ANM_BUTTON_CURSOR_ACTIVE );
}
void PMSIV_BUTTON_VisibleCursor( PMSIV_BUTTON* wk, BOOL flag )
{
	CLACT_SetDrawFlag( wk->cursor_actor, flag );
}

BOOL PMSIV_BUTTON_GetCursorVisibleFlag( PMSIV_BUTTON* wk )
{
	return CLACT_GetDrawFlag( wk->cursor_actor );
}

void PMSIV_BUTTON_MoveCursor( PMSIV_BUTTON* wk, int pos )
{
	VecFx32  mtx;

	mtx.z = 0;

	switch( pos ){
	case BUTTON_POS_DECIDE:
	default:
		mtx.x = DECIDE_CURSOR_XPOS * FX32_ONE;
		mtx.y = DECIDE_CURSOR_YPOS * FX32_ONE;
		break;

	case BUTTON_POS_CANCEL:
		mtx.x = CANCEL_CURSOR_XPOS * FX32_ONE;
		mtx.y = CANCEL_CURSOR_YPOS * FX32_ONE;
		break;
	}

	CLACT_SetMatrix( wk->cursor_actor, &mtx );
	CLACT_AnmChg( wk->cursor_actor, ANM_BUTTON_CURSOR_ACTIVE );
}

/**
 *	@brief	ボタン画像書き込み
 */
static void draw_mchg_button(PMSIV_BUTTON* wk,u8 mode,u8 ptn)
{
	int i;

	GF_BGL_ScrWriteExpand(wk->bgl,FRM_MAIN_BACK,
		MCHG_BTN_X+(MCHG_BTN_OX*mode),MCHG_BTN_Y,
		MCHG_BTN_W,MCHG_BTN_H,
		wk->pSWin->rawData,0,(MCHG_BTN_SH*mode)+(MCHG_BTN_H*ptn)+MCHG_BTN_DOY,
		wk->pSWin->screenWidth/8,wk->pSWin->screenHeight/8);
}

static void ChangeButtonTask( TCB_PTR tcb, void* wk_adrs )
{
	enum {
		ANM_WAIT1 = 4,
		ANM_WAIT2 = 4,
	};

	CHANGE_BUTTON_WORK* twk = wk_adrs;
	
	switch( twk->seq ){
	case 0:
		draw_mchg_button(twk->wk,twk->mode,1);
		draw_mchg_button(twk->wk,twk->mode^1,0);
		GF_BGL_LoadScreenV_Req( twk->wk->bgl, FRM_MAIN_BACK );
		twk->on_scrnID++;
		twk->seq++;
		break;

	case 1:
		if( ++(twk->timer) >= ANM_WAIT1 )
		{
			twk->wk->changeButtonTask = NULL;
			sys_FreeMemoryEz(wk_adrs);
			TCB_Delete(tcb);
		}
		break;
	}
}

void PMSIV_BUTTON_ChangeCategoryButton(PMSIV_BUTTON* wk)
{
	int mode;
	CHANGE_BUTTON_WORK* twk;
	
	mode = PMSI_GetCategoryMode( wk->mwk );
	
	twk = sys_AllocMemory(HEAPID_PMS_INPUT_VIEW, sizeof(CHANGE_BUTTON_WORK));
	twk->wk = wk;
	twk->seq = 0;
	twk->timer = 0;
	twk->mode = mode;

	wk->changeButtonTask = TCB_Add( ChangeButtonTask, twk, TASKPRI_VIEW_COMMAND );

}

BOOL PMSIV_BUTTON_WaitChangeCategoryButton( PMSIV_BUTTON* wk )
{
	return (wk->changeButtonTask == NULL);
}



//------------------------------------------------------------------
/**
 * 半透明時に透けて見えないように、ボタン画像をオフセットずらしで隠す
 *
 * @param   wk		
 *
 */
//------------------------------------------------------------------
void PMSIV_BUTTON_Hide( PMSIV_BUTTON* wk )
{
	PMSIV_BUTTON_UpdateRetButton(wk,1);
	PMSIV_BUTTON_UpdateButton( wk,FALSE,FALSE);
//	GF_BGL_ScrollSet( PMSIView_GetBGL(wk->vwk), FRM_MAIN_BACK, GF_BGL_SCROLL_Y_SET, 64 );
}
//------------------------------------------------------------------
/**
 * ↑PMSIV_BUTTON_Hideでズラしたオフセットを戻す
 *
 * @param   wk		
 *
 */
//------------------------------------------------------------------
void PMSIV_BUTTON_Appear( PMSIV_BUTTON* wk )
{
	PMSIV_BUTTON_UpdateRetButton(wk,0);
	PMSIV_BUTTON_UpdateButton( wk,TRUE,FALSE);
//	GF_BGL_ScrollSet( PMSIView_GetBGL(wk->vwk), FRM_MAIN_BACK, GF_BGL_SCROLL_Y_SET, 0 );
}





