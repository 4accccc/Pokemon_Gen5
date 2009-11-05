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

#if 0
	if( PMSI_GetCategoryMode( wk->mwk ) == CATEGORY_MODE_GROUP )
	{
		GF_BGL_ScrWriteExpand( wk->bgl, FRM_SUB_BG, BUTTON_GROUP_SCRN_XPOS, BUTTON_GROUP_SCRN_YPOS,
					MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, wk->button_scrn[SCRN_ID_GROUP_ON],
					0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

		GF_BGL_ScrWriteExpand( wk->bgl, FRM_SUB_BG, BUTTON_INITIAL_SCRN_XPOS, BUTTON_INITIAL_SCRN_YPOS,
					MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, wk->button_scrn[SCRN_ID_INITIAL_OFF],
					0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );
	}
	else
	{
		GF_BGL_ScrWriteExpand( wk->bgl, FRM_SUB_BG, BUTTON_GROUP_SCRN_XPOS, BUTTON_GROUP_SCRN_YPOS,
					MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, wk->button_scrn[SCRN_ID_GROUP_OFF],
					0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

		GF_BGL_ScrWriteExpand( wk->bgl, FRM_SUB_BG, BUTTON_INITIAL_SCRN_XPOS, BUTTON_INITIAL_SCRN_YPOS,
					MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, wk->button_scrn[SCRN_ID_INITIAL_ON],
					0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

	}
#endif
	setup_actors( wk, p_handle );

	GFL_BG_LoadScreenReq( FRM_SUB_BG );

}
static void load_scrn_datas( PMSIV_SUB* wk, ARCHANDLE* p_handle )
{
#if 0
	static const u16 dat_id[] = {
		NARC_pmsi_pms_bg_sub_a1_NSCR,
		NARC_pmsi_pms_bg_sub_a2_NSCR,
		NARC_pmsi_pms_bg_sub_a3_NSCR,
		NARC_pmsi_pms_bg_sub_a4_NSCR,
		NARC_pmsi_pms_bg_sub_b1_NSCR,
		NARC_pmsi_pms_bg_sub_b2_NSCR,
		NARC_pmsi_pms_bg_sub_b3_NSCR,
		NARC_pmsi_pms_bg_sub_b4_NSCR,
	};
	int i;
	void* loadPtr;
	NNSG2dScreenData* scrnData;

	for(i=0; i<NELEMS(dat_id); i++)
	{
		loadPtr = GFL_ARCHDL_UTIL_LoadScreen( p_handle, dat_id[i], FALSE, &scrnData, HEAPID_PMS_INPUT_VIEW );
		if( loadPtr )
		{
			MI_CpuCopy16( scrnData->rawData, wk->button_scrn[i], MODEBUTTON_SCRN_SIZE*2 );
			DC_FlushRange( wk->button_scrn[i], MODEBUTTON_SCRN_SIZE*2 );
			GFL_HEAP_FreeMemory( loadPtr );
		}
	}
#endif
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

//	FontProc_LoadFont( FONT_BUTTON, HEAPID_BASE_SYSTEM );
//	str_group = MSGDAT_GetStrDirectAlloc(ARC_MSG, NARC_msg_pms_input_dat, str_group_mode, HEAPID_PMS_INPUT_VIEW );
//	str_initial = MSGDAT_GetStrDirectAlloc(ARC_MSG, NARC_msg_pms_input_dat, str_initial_mode, HEAPID_PMS_INPUT_VIEW );
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
//		GF_BGL_BmpWinInit(&win);
//		win.ini = wk->bgl;
//		win.sizx = CGX_WIDTH;
//		win.sizy = CGX_HEIGHT;
//		win.bitmode = GF_BGL_BMPWIN_BITMODE_4;
//		win.chrbuf = charData->pRawData;
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
//	CLACT_HEADER  header;

#if 0
	NNSG2dImagePaletteProxy	palProxy;
	NNSG2dImageProxy		imgProxy;

	NNS_G2dInitImagePaletteProxy( &palProxy );
	NNS_G2dInitImageProxy( &imgProxy );
	ArcUtil_HDL_PalSysLoad( p_handle,
			NARC_pmsi_obj_sub_nclr, NNS_G2D_VRAM_TYPE_2DSUB, 0,HEAPID_PMS_INPUT_VIEW, &palProxy );
	ArcUtil_HDL_CharSysLoad( p_handle,
			NARC_pmsi_obj_sub_lz_ncgr, TRUE, CHAR_MAP_1D, 0, NNS_G2D_VRAM_TYPE_2DSUB, 0, 
							HEAPID_PMS_INPUT_VIEW, &imgProxy );

	load_clpack( &(wk->clpack), p_handle, NARC_pmsi_obj_sub_lz_ncer, NARC_pmsi_obj_sub_lz_nanr );

	wk->up_button = add_actor(wk, &imgProxy, &palProxy, &wk->clpack, BUTTON_UP_XPOS, BUTTON_UP_YPOS, 0, 0);
	CLACT_AnmChg( wk->up_button, ACTANM_ARROW_UP_STOP );
	CLACT_SetDrawFlag( wk->up_button, FALSE );

	wk->down_button = add_actor(wk, &imgProxy, &palProxy, &wk->clpack, BUTTON_DOWN_XPOS, BUTTON_DOWN_YPOS, 0, 0);
	CLACT_AnmChg( wk->down_button, ACTANM_ARROW_DOWN_STOP );
	CLACT_SetDrawFlag( wk->down_button, FALSE );
#elsif 0
	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_SUB, BGPRI_MAIN_EDITAREA );
	wk->up_button = PMSIView_AddActor( wk->vwk, &header, BUTTON_UP_XPOS, BUTTON_UP_YPOS,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DSUB );
	CLACT_AnmChg( wk->up_button, ACTANM_ARROW_UP_STOP );
	CLACT_SetDrawFlag( wk->up_button, FALSE );
	
	PMSIView_SetupDefaultActHeader( wk->vwk, &header, PMSIV_LCD_SUB, BGPRI_MAIN_EDITAREA );
	wk->down_button = PMSIView_AddActor( wk->vwk, &header, BUTTON_DOWN_XPOS, BUTTON_DOWN_YPOS,
			ACTPRI_EDITAREA_CURSOR, NNS_G2D_VRAM_TYPE_2DSUB );
	CLACT_AnmChg( wk->down_button, ACTANM_ARROW_DOWN_STOP );
	CLACT_SetDrawFlag( wk->down_button, FALSE );
#else
	/*
	NNSG2dImagePaletteProxy	palProxy;
	NNSG2dImageProxy		imgProxy;

	NNS_G2dInitImagePaletteProxy( &palProxy );
	NNS_G2dInitImageProxy( &imgProxy );
	ArcUtil_HDL_PalSysLoad( p_handle,
			NARC_pmsi_obj_sub_nclr, NNS_G2D_VRAM_TYPE_2DSUB, 0,HEAPID_PMS_INPUT_VIEW, &palProxy );
	ArcUtil_HDL_CharSysLoad( p_handle,
			NARC_pmsi_obj_sub_lz_ncgr, TRUE, CHAR_MAP_1D, 0, NNS_G2D_VRAM_TYPE_2DSUB, 0, 
							HEAPID_PMS_INPUT_VIEW, &imgProxy );
							*/
#endif

}

static void cleanup_actors( PMSIV_SUB* wk )
{
#if 0
	if( wk->up_button )
	{
		CLACT_Delete( wk->up_button );
	}
	if( wk->down_button )
	{
		CLACT_Delete( wk->down_button );
	}
	unload_clpack(&wk->clpack);
#endif
}




static void load_clpack( CELL_ANIM_PACK* clpack, ARCHANDLE* p_handle, u32 cellDatID, u32 animDatID )
{
#if 0
	clpack->cellLoadPtr = ArcUtil_HDL_CellBankDataGet(p_handle, cellDatID, TRUE, &(clpack->cellData), HEAPID_PMS_INPUT_VIEW);
	clpack->animLoadPtr = ArcUtil_HDL_AnimBankDataGet(p_handle, animDatID, TRUE, &(clpack->animData), HEAPID_PMS_INPUT_VIEW);
#endif
}
static void unload_clpack( CELL_ANIM_PACK* clpack )
{
#if 0
	sys_FreeMemoryEz( clpack->cellLoadPtr );
	sys_FreeMemoryEz( clpack->animLoadPtr );
#endif
}


static GFL_CLWK* add_actor(
	PMSIV_SUB* wk, NNSG2dImageProxy* imgProxy, NNSG2dImagePaletteProxy* palProxy,
	CELL_ANIM_PACK* clpack, int xpos, int ypos, int bgpri, int actpri )
{
#if 0
	CLACT_HEADER  header;
	CLACT_ADD_SIMPLE  add;
	CLACT_WORK_PTR   act;

	header.pImageProxy = imgProxy;
	header.pPaletteProxy = palProxy;
	header.pCellBank     = clpack->cellData;
	header.pAnimBank     = clpack->animData;
	header.priority      = bgpri;
	header.pCharData = NULL;
	header.pMCBank = NULL;
	header.pMCABank = NULL;
	header.flag = FALSE;

	add.ClActSet = wk->actsys;
	add.ClActHeader = &header;
	add.mat.x = xpos * FX32_ONE;
	add.mat.y = (ypos+192) * FX32_ONE;
	add.mat.z = 0;
	add.pri = actpri;
	add.DrawArea = NNS_G2D_VRAM_TYPE_2DSUB;
	add.heap = HEAPID_PMS_INPUT_VIEW;

	act = CLACT_AddSimple(&add);
	if( act )
	{
		CLACT_SetAnmFlag( act, TRUE );
		CLACT_SetAnmFrame( act, PMSI_ANM_SPEED );
	}
	return act;
#endif
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
#if 0
	CHANGE_BUTTON_WORK* twk = GFL_HEAP_AllocMemory(HEAPID_PMS_INPUT_VIEW, sizeof(CHANGE_BUTTON_WORK));

	if( twk )
	{
		static const struct {
			u16 on_id_start;
			u16 on_id_end;
			u16 on_x;
			u16 on_y;
			u16 off_id;
			u16 off_x;
			u16 off_y;
		}TaskParam[] = {
			{
				SCRN_ID_GROUP_PUSH1, SCRN_ID_GROUP_ON, BUTTON_GROUP_SCRN_XPOS, BUTTON_GROUP_SCRN_YPOS,
				SCRN_ID_INITIAL_OFF, BUTTON_INITIAL_SCRN_XPOS, BUTTON_INITIAL_SCRN_YPOS
			},{
				SCRN_ID_INITIAL_PUSH1, SCRN_ID_INITIAL_ON, BUTTON_INITIAL_SCRN_XPOS, BUTTON_INITIAL_SCRN_YPOS,
				SCRN_ID_GROUP_OFF, BUTTON_GROUP_SCRN_XPOS, BUTTON_GROUP_SCRN_YPOS
			}
		};

		int mode = PMSI_GetCategoryMode( wk->mwk );

		twk->wk = wk;
		twk->seq = 0;
		twk->timer = 0;

		twk->on_scrnID		= TaskParam[mode].on_id_start;
		twk->on_scrnID_end	= TaskParam[mode].on_id_end;
		twk->on_scrn_x		= TaskParam[mode].on_x;
		twk->on_scrn_y		= TaskParam[mode].on_y;
		twk->off_scrnID		= TaskParam[mode].off_id;
		twk->off_scrn_x		= TaskParam[mode].off_x;
		twk->off_scrn_y		= TaskParam[mode].off_y;

		wk->changeButtonTask = GFL_TCB_AddTask( PMSI_GetTcbSystem(wk->mwk) , ChangeButtonTask, twk, TASKPRI_VIEW_COMMAND );

	}
	else
	{
		wk->changeButtonTask = NULL;
	}
#endif
}

BOOL PMSIV_SUB_WaitChangeCategoryButton( PMSIV_SUB* wk )
{
  return TRUE;
//	return (wk->changeButtonTask == NULL);
}

static void ChangeButtonTask( GFL_TCB *tcb, void* wk_adrs )
{
#if 0
	enum {
		ANM_WAIT1 = 2,
		ANM_WAIT2 = 4,
	};

	CHANGE_BUTTON_WORK* twk = wk_adrs;

	switch( twk->seq ){
	case 0:
		GFL_BG_WriteScreenExpand( FRM_SUB_BG, twk->on_scrn_x, twk->on_scrn_y,
						MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, twk->wk->button_scrn[twk->on_scrnID],
						0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

		GFL_BG_WriteScreenExpand( FRM_SUB_BG, twk->off_scrn_x, twk->off_scrn_y,
						MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, twk->wk->button_scrn[twk->off_scrnID],
						0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

		GFL_BG_LoadScreenReq( FRM_SUB_BG );
		twk->on_scrnID++;
		twk->seq++;
		break;

	case 1:
		if( ++(twk->timer) >= ANM_WAIT1 )
		{
			GFL_BG_WriteScreenExpand(FRM_SUB_BG, twk->on_scrn_x, twk->on_scrn_y,
						MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, twk->wk->button_scrn[twk->on_scrnID],
						0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

			GFL_BG_LoadScreenReq( FRM_SUB_BG );
			twk->timer = 0;
			twk->on_scrnID++;
			twk->seq++;
		}
		break;

	case 2:
		if( ++(twk->timer) >= ANM_WAIT2 )
		{
			GFL_BG_WriteScreenExpand(FRM_SUB_BG, twk->on_scrn_x, twk->on_scrn_y,
						MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT, twk->wk->button_scrn[twk->on_scrnID],
						0, 0, MODEBUTTON_SCRN_WIDTH, MODEBUTTON_SCRN_HEIGHT );

			GFL_BG_LoadScreenReq( FRM_SUB_BG );
			twk->seq++;
		}
		break;

	case 3:
		twk->wk->changeButtonTask = NULL;
		GFL_HEAP_FreeMemory(wk_adrs);
		GFL_TCB_DeleteTask(tcb);
	}
#endif
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
#if 0
	if( flag )
	{
		CLACT_SetDrawFlag( wk->up_button, PMSI_GetWordWinUpArrowVisibleFlag(wk->mwk) );
		CLACT_SetDrawFlag( wk->down_button, PMSI_GetWordWinDownArrowVisibleFlag(wk->mwk) );
	}
	else
	{
		CLACT_SetDrawFlag( wk->up_button, FALSE );
		CLACT_SetDrawFlag( wk->down_button, FALSE );
	}
#endif
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
#if 0
	switch(pos){
	case SUB_BUTTON_UP:
		switch( state ){
		case SUB_BUTTON_STATE_HOLD:
			CLACT_AnmChg( wk->up_button, ACTANM_ARROW_UP_HOLD );
			break;
		case SUB_BUTTON_STATE_RELEASE:
			CLACT_AnmChg( wk->up_button, ACTANM_ARROW_UP_STOP );
			break;
		}
		break;

	case SUB_BUTTON_DOWN:
		switch( state ){
		case SUB_BUTTON_STATE_HOLD:
			CLACT_AnmChg( wk->down_button, ACTANM_ARROW_DOWN_HOLD );
			break;
		case SUB_BUTTON_STATE_RELEASE:
			CLACT_AnmChg( wk->down_button, ACTANM_ARROW_DOWN_STOP );
			break;
		}
		break;
	}
#endif
}
