//============================================================================================
/**
	* @file	pmsiv_sub.c
	* @bfief	簡易会話入力画面（描画下請け：サブ画面）
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
#include "msg\msg_pms_input.h"
#include "message.naix"

#include "pms_input_prv.h"
#include "pms_input_view.h"

//======================================================================

enum {


	GROUP_WIN_XPOS = 4,
	GROUP_WIN_YPOS = 5,
	GROUP_WIN_PALNO = 0,

	INITIAL_WIN_XPOS = 19,
	INITIAL_WIN_YPOS = 5,
	INITIAL_WIN_PALNO = 1,

	BUTTON_WIN_WIDTH = 9,
	BUTTON_WIN_HEIGHT = 5,
	BUTTON_WIN_CHARSIZE = BUTTON_WIN_WIDTH*BUTTON_WIN_HEIGHT,

	PUSH_BUTTON_WRITE_OX = 5,
	PUSH_BUTTON_WRITE_OY = 5,
	RELEASE_BUTTON_WRITE_OX = 4,
	RELEASE_BUTTON_WRITE_OY = 4,

	WIN_COL_PUSH_GROUND = 0x0a,
	WIN_COL_RELEASE_GROUND = 0x09,
	WIN_COL1 = 0x01,
	WIN_COL2 = 0x02,
	WIN_COL3 = 0x03,

};

enum {
	SCREEN_PATTERN_PUSH_GROUP,
	SCREEN_PATTERN_PUSH_INITIAL,
};

enum {
	ANM_MODEBUTTON_OFF,
	ANM_MODEBUTTON_ON,
	ANM_MODEBUTTON_PUSH,
};


enum {
	MODEBUTTON_SCRN_SIZE = MODEBUTTON_SCRN_WIDTH * MODEBUTTON_SCRN_HEIGHT,

	SCRN_ID_GROUP_OFF = 0,
	SCRN_ID_GROUP_PUSH1,
	SCRN_ID_GROUP_PUSH2,
	SCRN_ID_GROUP_ON,
	SCRN_ID_INITIAL_OFF,
	SCRN_ID_INITIAL_PUSH1,
	SCRN_ID_INITIAL_PUSH2,
	SCRN_ID_INITIAL_ON,
	SCRN_ID_MAX,

};


enum {
#if 0
	ACTANM_ARROW_UP_STOP,
	ACTANM_ARROW_UP_HOLD,
	ACTANM_ARROW_DOWN_STOP,
	ACTANM_ARROW_DOWN_HOLD,
#else
	ACTANM_ARROW_UP_STOP = 10,
	ACTANM_ARROW_UP_HOLD = 10,
	ACTANM_ARROW_DOWN_STOP = 11,
	ACTANM_ARROW_DOWN_HOLD = 11,
#endif
};


//======================================================================


//======================================================================
typedef struct {
	void*					cellLoadPtr;
	NNSG2dCellDataBank*		cellData;
	void*					animLoadPtr;
	NNSG2dAnimBankData*		animData;
}CELL_ANIM_PACK;


//--------------------------------------------------------------
/**
	*	
	*/
//--------------------------------------------------------------
struct _PMSIV_SUB {
	PMS_INPUT_VIEW*        vwk;
	const PMS_INPUT_WORK*  mwk;
	const PMS_INPUT_DATA*  dwk;

	GFL_CLUNIT		*actsys;
	GFL_CLWK		*up_button;
	GFL_CLWK		*down_button;
	CELL_ANIM_PACK     clpack;

//	u16		button_scrn[SCRN_ID_MAX][MODEBUTTON_SCRN_SIZE];

	GFL_TCB		*changeButtonTask;

};


//==============================================================
// Prototype
//==============================================================
static void load_scrn_datas( PMSIV_SUB* wk, ARCHANDLE* p_handle );
static void setup_cgx_datas( PMSIV_SUB* wk, ARCHANDLE* p_handle );
static void print_mode_name( PMSIV_SUB* wk,GFL_BMP_DATA* bmp, const STRBUF* str, int yofs );
static void setup_actors( PMSIV_SUB* wk, ARCHANDLE* p_handle );
static void cleanup_actors( PMSIV_SUB* wk );
static void load_clpack( CELL_ANIM_PACK* clpack, ARCHANDLE* p_handle, u32 cellDatID, u32 animDatID );
static void unload_clpack( CELL_ANIM_PACK* clpack );
static void ChangeButtonTask( GFL_TCB *tcb, void* wk_adrs );


static GFL_CLWK* add_actor(
	PMSIV_SUB* wk, NNSG2dImageProxy* imgProxy, NNSG2dImagePaletteProxy* palProxy,
	CELL_ANIM_PACK* clpack, int xpos, int ypos, int bgpri, int actpri );



//------------------------------------------------------------------
/**
	* 
	*
	* @param   vwk		
	* @param   mwk		
	* @param   dwk		
	*
	* @retval  PMSIV_SUB*		
	*/
//------------------------------------------------------------------
PMSIV_SUB*  PMSIV_SUB_Create( PMS_INPUT_VIEW* vwk, const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk )
{
	PMSIV_SUB*  wk = GFL_HEAP_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_SUB) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;
	wk->actsys = PMSIView_GetCellUnit(vwk);

	wk->up_button = NULL;
	wk->down_button = NULL;
	wk->changeButtonTask = NULL;

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
void PMSIV_SUB_Delete( PMSIV_SUB* wk )
{
	cleanup_actors( wk );

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
void PMSIV_SUB_SetupGraphicDatas( PMSIV_SUB* wk, ARCHANDLE* p_handle )
{
	GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms_bg_sub_NSCR, FRM_SUB_BG, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	
	load_scrn_datas( wk, p_handle );
	setup_cgx_datas( wk, p_handle );

	setup_actors( wk, p_handle );

	GFL_BG_LoadScreenReq( FRM_SUB_BG );

}
static void load_scrn_datas( PMSIV_SUB* wk, ARCHANDLE* p_handle )
{
}

static void setup_cgx_datas( PMSIV_SUB* wk, ARCHANDLE* p_handle )
{
	enum {
		CGX_WIDTH  = 11,
		CGX_HEIGHT = 57,
	};

	STRBUF* str_group;
	STRBUF* str_initial;
	GFL_MSGDATA *workMsg;
	void* loadPtr;
	NNSG2dCharacterData* charData;

	workMsg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_pms_input_dat, HEAPID_PMS_INPUT_VIEW );
	str_group = GFL_MSG_CreateString(workMsg, str_group_mode );
	str_initial = GFL_MSG_CreateString(workMsg, str_initial_mode );
	GFL_MSG_Delete( workMsg );
	

	loadPtr = GFL_ARCHDL_UTIL_LoadOBJCharacter(p_handle, NARC_pmsi_pms_bg_sub_NCGR, FALSE, &charData, HEAPID_PMS_INPUT_VIEW);
	if(loadPtr)
	{
		GFL_BMP_DATA	*bmp;
		int i;
#if 1
		bmp = GFL_BMP_CreateWithData( charData->pRawData , 
										CGX_WIDTH , CGX_HEIGHT , 
										GFL_BMP_16_COLOR ,
										HEAPID_PMS_INPUT_VIEW );
		print_mode_name( wk, bmp, str_group,   0 );
		GFL_BMP_Delete( bmp );
		bmp = GFL_BMP_CreateWithData( (u8*)(charData->pRawData) + ((CGX_WIDTH * (MODEBUTTON_SCRN_HEIGHT*4))*0x20), 
										CGX_WIDTH , CGX_HEIGHT , 
										GFL_BMP_16_COLOR ,
										HEAPID_PMS_INPUT_VIEW );

//		win.chrbuf = (u8*)(charData->pRawData) + ((CGX_WIDTH * (MODEBUTTON_SCRN_HEIGHT*4))*0x20);
		print_mode_name( wk, bmp, str_initial, 0 );
		GFL_BMP_Delete( bmp );

		DC_FlushRange( charData->pRawData, charData->szByte );
		GFL_BG_LoadCharacter(FRM_SUB_BG, charData->pRawData, charData->szByte, 0);
#endif
		GFL_HEAP_FreeMemory(loadPtr);
	}

	GFL_STR_DeleteBuffer(str_initial);
	GFL_STR_DeleteBuffer(str_group);
//	FontProc_UnloadFont( FONT_BUTTON );
}

static void print_mode_name( PMSIV_SUB* wk,GFL_BMP_DATA* bmp, const STRBUF* str, int yofs )
{
	enum {
		COL_1 = 0x01,
		COL_2 = 0x02,
		COL_3 = 0x03,

		WRITE_X_ORG = 10,
		WRITE_Y_ORG = 22,
		WRITE_Y_DIFF = 56,
	};

	static const struct {
		s16				xofs;
		s16				yofs;
	}writeParam[] = {
		{ WRITE_X_ORG,  WRITE_Y_ORG },
		{ WRITE_X_ORG, (WRITE_Y_ORG + WRITE_Y_DIFF*1)-1 },
		{ WRITE_X_ORG, (WRITE_Y_ORG + WRITE_Y_DIFF*2)-2 },
		{ WRITE_X_ORG, (WRITE_Y_ORG + WRITE_Y_DIFF*3)-1 },
	};
	int i, y;

	for(i=0; i<NELEMS(writeParam); i++)
	{
		y = writeParam[i].yofs + yofs;
		GFL_FONTSYS_SetColor( COL_1,COL_2,COL_3 );
		PRINTSYS_Print( bmp , writeParam[i].xofs, y, str , PMSIView_GetFontHandle(wk->vwk) );
//		GF_STR_PrintColor( win, FONT_BUTTON, str, writeParam[i].xofs, y,
//							MSG_NO_PUT, GF_PRINTCOLOR_MAKE(COL_1,COL_2,COL_3), NULL );
		GFL_FONTSYS_SetDefaultColor();
	}
}


//==============================================================================================
//==============================================================================================
static void setup_actors( PMSIV_SUB* wk, ARCHANDLE* p_handle )
{
}

static void cleanup_actors( PMSIV_SUB* wk )
{
}




static void load_clpack( CELL_ANIM_PACK* clpack, ARCHANDLE* p_handle, u32 cellDatID, u32 animDatID )
{
}
static void unload_clpack( CELL_ANIM_PACK* clpack )
{
}


static GFL_CLWK* add_actor(
	PMSIV_SUB* wk, NNSG2dImageProxy* imgProxy, NNSG2dImagePaletteProxy* palProxy,
	CELL_ANIM_PACK* clpack, int xpos, int ypos, int bgpri, int actpri )
{
	return NULL;
}

//==============================================================================================
//==============================================================================================

typedef struct {
	PMSIV_SUB*  wk;
	int seq;
	u16 timer;

	u16 on_scrnID;
	u16 on_scrnID_end;
	u16 on_scrn_x;
	u16 on_scrn_y;

	u16 off_scrnID;
	u16 off_scrn_x;
	u16 off_scrn_y;

}CHANGE_BUTTON_WORK;

//------------------------------------------------------------------
/**
	* 
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIV_SUB_ChangeCategoryButton( PMSIV_SUB* wk )
{
}

BOOL PMSIV_SUB_WaitChangeCategoryButton( PMSIV_SUB* wk )
{
  return TRUE;
}

static void ChangeButtonTask( GFL_TCB *tcb, void* wk_adrs )
{
}



//------------------------------------------------------------------
/**
	* 矢印ボタン描画オン／オフ
	*
	* @param   wk		
	* @param   flag		
	*
	*/
//------------------------------------------------------------------
void PMSIV_SUB_VisibleArrowButton( PMSIV_SUB* wk, BOOL flag )
{
}


//------------------------------------------------------------------
/**
	* 矢印ボタン状態切り替え
	*
	* @param   wk		
	* @param   pos		
	* @param   state		
	*
	*/
//------------------------------------------------------------------
void PMSIV_SUB_ChangeArrowButton( PMSIV_SUB* wk, int pos, int state )
{
}
