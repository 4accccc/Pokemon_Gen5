//============================================================================================
/**
	* @file	pms_input_view.c
	* @bfief	簡易会話入力画面
	* @author	taya
  * @author	genya hosaka (引継)
	* @date	06.01.24
  *
  * @TODO メッセージ系の描画コマンドは消していい
	*/
//============================================================================================
#include <gflib.h>

#include "system\main.h"
#include "system\gfl_use.h"
#include "system\wipe.h"

#include "savedata\save_control.h"
#include "print\printsys.h"

#include "arc_def.h"
#include "font/font.naix"
#include "pms_input_prv.h"
#include "pms_input_view.h"
#include "pmsi.naix"

// 上画面説明欄に表示するメッセージ
#include "message.naix"
#include "msg/msg_pms_input.h"

#include "test/ariizumi/ari_debug.h"

//==============================================================================================
//==============================================================================================


//==============================================================================================
//==============================================================================================
enum {
	STORE_COMMAND_MAX = 4,		///< 並列実行可能なコマンド総数

	FADE_FRAMES = 5,

};

// 上画面説明欄の文字の色
enum
{
  EXPLAIN_COL_L     = 14,
  EXPLAIN_COL_S     = 15,
  EXPLAIN_COL_B     =  7,
};


//------------------------------------------------------
/**
	*  描画メインワーク
	*/
//------------------------------------------------------
struct _PMS_INPUT_VIEW {
	GFL_TCB *mainTask;
	GFL_TCB *vintrTask;

	GFL_TCB *cmdTask[ STORE_COMMAND_MAX ];

	const PMS_INPUT_WORK*	main_wk;
	const PMS_INPUT_DATA*	data_wk;

	GFL_CLUNIT				*cellUnit;

	// メインとサブで２つずつ	
	PMSIV_CELL_RES	resCell[2];
	PMSIV_CELL_RES	resCellDeco[2]; ///< デコメ

	PMSIV_EDIT*			  edit_wk;
//	PMSIV_BUTTON*		button_wk;
	PMSIV_CATEGORY*		category_wk;
	PMSIV_WORDWIN*		wordwin_wk;
	PMSIV_SUB*			  sub_wk; // 現状SUBBGの読み込みだけに使われているようだったので全てコメントアウトした
  PMSIV_MENU*       menu_wk;

  // 上画面説明欄に表示するフォントカラーをつくるためのバッファ
  GXRgb        explain_font_color_l_s[2];
  // 上画面説明欄に表示するメッセージのウィンドウ
  GFL_BMPWIN*  explain_bmpwin;
  BOOL         explain_bmpwin_trans_req;

	u8					status;
	u8					key_mode;

	int*				p_key_mode;
	
  PRINT_QUE*  print_que;
	GFL_FONT		*fontHandle;
};

//------------------------------------------------------
/**
	*  描画コマンドワーク
	*/
//------------------------------------------------------
typedef struct {
	PMS_INPUT_VIEW*        vwk;
	const PMS_INPUT_WORK*  mwk;
	const PMS_INPUT_DATA*  dwk;

	u32   command_no;
	int   store_pos;
	int   seq;
}COMMAND_WORK;


//==============================================================
// Prototype
//==============================================================
static void PMSIView_MainTask( GFL_TCB *tcb, void* wk_adrs );
static void PMSIView_VintrTask( GFL_TCB *tcb, void* wk_adrs );
static void DeleteCommand( COMMAND_WORK* cwk );
static void Cmd_Init( GFL_TCB *tcb, void* wk_adrs );
static void setup_obj_graphic( COMMAND_WORK* cwk, ARCHANDLE* p_handle );
static void Cmd_Quit( GFL_TCB *tcb, void* wk_adrs );
static void setup_bg_params( COMMAND_WORK* cwk );
static void Cmd_FadeIn( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_UpdateEditArea( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ChangeKTEditArea( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ChangeKTCategory( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ChangeKTWordWin( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_EditAreaToButton( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ButtonToEditArea( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_EditAreaToCategory( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ChangeCategoryModeDisable( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ChangeCategoryModeEnable( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_CategoryToEditArea( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_CategoryToWordWin( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_WordWinToCategory( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_WordWinToEditArea( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_WordWinToButton( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_MoveEditAreaCursor( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_MoveButtonCursor( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_MoveCategoryCursor( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_MoveWordWinCursor( GFL_TCB *tcb, void* wk_adrs  );
static void Cmd_ScrollWordWin( GFL_TCB *tcb, void* wk_adrs  );

static void Cmd_DispMessageDefault( GFL_TCB *tcb, void* wk_adrs );
//static void Cmd_DispMessageOK( GFL_TCB *tcb, void* wk_adrs );
//static void Cmd_DispMessageCancel( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_DispMessageWarn( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_TaskMenuDecide( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_MoveMenuCursor( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_EraseMenu( GFL_TCB *tcb, void* wk_adrs );
static void Cmd_ButtonUpHold(GFL_TCB *tcb, void* wk_adrs);
static void Cmd_ButtonDownHold(GFL_TCB *tcb, void* wk_adrs);
static void Cmd_ButtonUpRelease(GFL_TCB *tcb, void* wk_adrs);
static void Cmd_ButtonDownRelease(GFL_TCB *tcb, void* wk_adrs);
static void Cmd_InputWordUpdate( GFL_TCB* tcb, void* wk_adrs );
static void Cmd_ScrollWordWinBar(GFL_TCB *tcb, void* wk_adrs);

static void Cmd_EraseInCategoryInitial( GFL_TCB *tcb, void* wk_adrs );


static const GFL_DISP_VRAM bank_data = {
	GX_VRAM_BG_128_B,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_0_A,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_64K,
	GX_OBJVRAMMODE_CHAR_1D_64K
};


// 上画面説明欄に表示するメッセージのウィンドウ
typedef enum
{
  PMSIV_EXPLAIN_MSG_EDIT_SENTENCE,
  PMSIV_EXPLAIN_MSG_EDIT_PECULIAR,
  PMSIV_EXPLAIN_MSG_GROUP_CATEGORY,
  PMSIV_EXPLAIN_MSG_GROUP_WORDWIN,
  PMSIV_EXPLAIN_MSG_INITIAL_CATEGORY,
  PMSIV_EXPLAIN_MSG_INITIAL_WORDWIN,
  PMSIV_EXPLAIN_MSG_MAX,
}
PMSIV_EXPLAIN_MSG;
static void set_explain_message( COMMAND_WORK* cwk, PMSIV_EXPLAIN_MSG msg );
static void trans_explain_message( PMS_INPUT_VIEW* vwk );


//------------------------------------------------------------------
/**
	* 
	*
	* @param   main_wk		
	* @param   data_wk		
	*
	* @retval  PMS_INPUT_VIEW*		
	*/
//------------------------------------------------------------------
PMS_INPUT_VIEW*  PMSIView_Create(const PMS_INPUT_WORK* main_wk, const PMS_INPUT_DATA* data_wk)
{
	PMS_INPUT_VIEW*  vwk = GFL_HEAP_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMS_INPUT_VIEW) );

	if( vwk )
	{
		int i;

		vwk->main_wk = main_wk;
		vwk->data_wk = data_wk;

		vwk->p_key_mode = PMSI_GetKTModePointer(main_wk);

		GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN , &bank_data, HEAPID_PMS_INPUT_VIEW );
		vwk->cellUnit = GFL_CLACT_UNIT_Create( 128, 0, HEAPID_PMS_INPUT_VIEW );

		GFL_BG_Init( HEAPID_PMS_INPUT_VIEW );
		GFL_BMPWIN_Init( HEAPID_PMS_INPUT_VIEW );
		GX_SetMasterBrightness( -16 );
		GXS_SetMasterBrightness( -16 );
		
		vwk->mainTask = GFL_TCB_AddTask( PMSI_GetTcbSystem(main_wk) ,PMSIView_MainTask, vwk, TASKPRI_VIEW_MAIN );
		vwk->vintrTask = PMSIView_AddVTask( PMSIView_VintrTask, vwk, VTASKPRI_MAIN );
		for(i=0; i<STORE_COMMAND_MAX; i++)
		{
			vwk->cmdTask[i] = NULL;
		}

    // 上画面説明欄に表示するメッセージのウィンドウ
    vwk->explain_bmpwin_trans_req     = FALSE;

	}

	return vwk;
}


//------------------------------------------------------------------
/**
	* 
	*
	* @param   vwk		
	*
	*/
//------------------------------------------------------------------
void PMSIView_Delete( PMS_INPUT_VIEW* vwk )
{
	if( vwk )
	{
		int i;

		for(i=0; i<STORE_COMMAND_MAX; i++)
		{
			if( vwk->cmdTask[i] )
			{
				GFL_TCB_DeleteTask( vwk->cmdTask[i] );
			}
		}

		GFL_TCB_DeleteTask( vwk->mainTask );
		GFL_TCB_DeleteTask( vwk->vintrTask );
      
    // MAINを回しているのでここで開放
    PMSIV_MENU_Delete( vwk->menu_wk );

		GFL_CLACT_UNIT_Delete( vwk->cellUnit );
		GFL_CLACT_SYS_Delete();

		GFL_BMPWIN_Exit();
		GFL_BG_Exit();
		GFL_HEAP_FreeMemory( vwk );
	}
}


//==============================================================================================
//==============================================================================================


//------------------------------------------------------------------
/**
	* V Blank 動作タスクをセットする
	*
	* @param   func		
	* @param   wk		
	* @param   pri		
	*
	* @retval  TCB_PTR		
	*/
//------------------------------------------------------------------
GFL_TCB *PMSIView_AddVTask( GFL_TCB_FUNC func, void* wk, int pri )
{
#ifdef PMSI_FPS_60
	return GFUser_VIntr_CreateTCB( func, wk, pri );
#else
	return GFUser_VIntr_CreateTCB( func, wk, pri );
#endif
}


//------------------------------------------------------------------
/**
	* メインタスク
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void PMSIView_MainTask( GFL_TCB *tcb, void* wk_adrs )
{
  PMS_INPUT_VIEW* vwk = wk_adrs;

	GFL_CLACT_SYS_Main();

  PMSIV_MENU_Main( vwk->menu_wk );

  PRINTSYS_QUE_Main( vwk->print_que );

  trans_explain_message( vwk );
}

//------------------------------------------------------------------
/**
	* V Blank タスク
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void PMSIView_VintrTask( GFL_TCB *tcb, void* wk_adrs )
{
	PMS_INPUT_VIEW* vwk = wk_adrs;
	
	GFL_BG_VBlankFunc();
	
	GFL_CLACT_SYS_VBlankFunc();
	
}

//==============================================================================================
//==============================================================================================

//------------------------------------------------------------------
/**
	* コマンド登録
	*
	* @param   vwk		
	* @param   cmd		
	*
	*/
//------------------------------------------------------------------
void PMSIView_SetCommand( PMS_INPUT_VIEW* vwk, int cmd )
{
	GFL_TCB_FUNC *func_tbl[VCMD_MAX] = {
		Cmd_Init,
		Cmd_Quit,
		Cmd_FadeIn,

		Cmd_UpdateEditArea,
		Cmd_ChangeKTEditArea,
		Cmd_ChangeKTCategory,
		Cmd_ChangeKTWordWin,
		Cmd_EditAreaToButton,
		Cmd_ButtonToEditArea,
		Cmd_EditAreaToCategory,
		Cmd_CategoryToEditArea,
		Cmd_CategoryToWordWin,
		Cmd_WordWinToCategory,
		Cmd_WordWinToEditArea,
		Cmd_WordWinToButton,

		Cmd_MoveEditAreaCursor,
		Cmd_MoveButtonCursor,
		Cmd_MoveCategoryCursor,
		Cmd_MoveWordWinCursor,
		Cmd_ScrollWordWin,

		Cmd_DispMessageDefault,
//		Cmd_DispMessageOK,
//		Cmd_DispMessageCancel,
		Cmd_DispMessageWarn,
    Cmd_TaskMenuDecide,
		Cmd_MoveMenuCursor,
		Cmd_EraseMenu,

		Cmd_ChangeCategoryModeDisable,
		Cmd_ChangeCategoryModeEnable,

		Cmd_ButtonUpHold,
		Cmd_ButtonDownHold,
		Cmd_ButtonUpRelease,
		Cmd_ButtonDownRelease,

    Cmd_InputWordUpdate,
		Cmd_ScrollWordWinBar,

    Cmd_EraseInCategoryInitial,
	};



	//if( cmd < NELEMS(func_tbl) )
	if( cmd < VCMD_MAX )
	{
		COMMAND_WORK* cwk = GFL_HEAP_AllocMemory( HEAPID_PMS_INPUT_VIEW, sizeof(COMMAND_WORK) );

		if( cwk )
		{
			int i;

			cwk->seq = 0;
			cwk->command_no = cmd;
			cwk->vwk = vwk;
			cwk->mwk = vwk->main_wk;
			cwk->dwk = vwk->data_wk;

			for(i=0; i<STORE_COMMAND_MAX; i++)
			{
				if( vwk->cmdTask[i] == NULL )
				{
					cwk->store_pos = i;
					vwk->cmdTask[i] = GFL_TCB_AddTask( PMSI_GetTcbSystem(vwk->main_wk) ,func_tbl[cmd], cwk, TASKPRI_VIEW_COMMAND );
					break;
				}
			}
		}
	}
	else
	{
		GF_ASSERT_MSG(0, "cmd=%d", cmd);
	}

}

//------------------------------------------------------------------
/**
	* コマンド終了待ち（全てのコマンドが終了するのを待つ）
	*
	* @param   vwk		描画メインワーク
	*
	* @retval  BOOL	TRUEで終了
	*/
//------------------------------------------------------------------
BOOL PMSIView_WaitCommandAll( PMS_INPUT_VIEW* vwk )
{
	int i;

	for(i=0; i<STORE_COMMAND_MAX; i++)
	{
		if( vwk->cmdTask[i] != NULL )
		{
			return FALSE;
		}
	}

	return TRUE;
}
//------------------------------------------------------------------
/**
	* コマンド終了待ち（指定ナンバーのコマンドが終了するのを待つ）
	*
	* @param   vwk		描画メインワーク
	* @param   cmd		コマンドナンバー
	*
	* @retval  BOOL	TRUEで終了
	*/
//------------------------------------------------------------------
BOOL PMSIView_WaitCommand( PMS_INPUT_VIEW* vwk, int cmd )
{
	int i;

	for(i=0; i<STORE_COMMAND_MAX; i++)
	{
		if( vwk->cmdTask[i] != NULL )
		{
			COMMAND_WORK* cwk = GFL_TCB_GetWork( vwk->cmdTask[i] );
			if( cwk->command_no == cmd )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
	* コマンド自殺
	*
	* @param   cwk		
	*
	*/
//------------------------------------------------------------------
static void DeleteCommand( COMMAND_WORK* cwk )
{
	GFL_TCB_DeleteTask( cwk->vwk->cmdTask[ cwk->store_pos ] );
	cwk->vwk->cmdTask[ cwk->store_pos ] = NULL;
	GFL_HEAP_FreeMemory( cwk );
}


//==============================================================================================
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：画面構築
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_Init( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* cwk = wk_adrs;
	ARCHANDLE* p_handle;




/*
これより前に、
pms_input.c
static PMS_INPUT_WORK* ConstructWork( GFL_PROC* proc , void* pwk )
から呼ばれる
pms_input_view.c
PMS_INPUT_VIEW*  PMSIView_Create(const PMS_INPUT_WORK* main_wk, const PMS_INPUT_DATA* data_wk)
で
GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN , &bank_data, HEAPID_PMS_INPUT_VIEW );
をやっているのが非常に気になる。
順番を入れ替えたい。
*/
	//レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();	
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );




	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
	GX_SetVisiblePlane( 0 );
	GXS_SetVisiblePlane( 0 );

	p_handle = GFL_ARC_OpenDataHandle( ARCID_PMSI_GRAPHIC, HEAPID_PMS_INPUT_VIEW );

	setup_bg_params( cwk );
	setup_obj_graphic( cwk, p_handle );

	cwk->vwk->edit_wk = PMSIV_EDIT_Create( cwk->vwk, cwk->mwk, cwk->dwk );
	PMSIV_EDIT_SetupGraphicDatas( cwk->vwk->edit_wk, p_handle );

//	cwk->vwk->button_wk = PMSIV_BUTTON_Create( cwk->vwk, cwk->mwk, cwk->dwk );
//	PMSIV_BUTTON_SetupGraphicDatas( cwk->vwk->button_wk, p_handle );

	cwk->vwk->category_wk = PMSIV_CATEGORY_Create( cwk->vwk, cwk->mwk, cwk->dwk );
	PMSIV_CATEGORY_SetupGraphicDatas( cwk->vwk->category_wk, p_handle );

	cwk->vwk->wordwin_wk = PMSIV_WORDWIN_Create( cwk->vwk, cwk->mwk, cwk->dwk );
	PMSIV_WORDWIN_SetupGraphicDatas( cwk->vwk->wordwin_wk );

  // SUB BGの展開 
  // 背景面
	GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms_bg_sub_NSCR, FRM_SUB_BG, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_pmsi_pms_bg_sub_NCGR, FRM_SUB_BG, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );

  // 入力予測リスト面
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms_bg_sub2_NSCR, FRM_SUB_SEARCH_LIST, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_pmsi_pms_bg_sub_NCGR, FRM_SUB_SEARCH_LIST, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
	GFL_BG_SetVisible( FRM_SUB_SEARCH_LIST, FALSE );


  // 上画面説明欄
  // 他のパレットはこのすぐ上にある
  // PMSIV_EDIT_SetupGraphicDatas
  // で呼ばれている
  GFL_ARCHDL_UTIL_TransVramPaletteEx(
      p_handle,
      NARC_pmsi_pms_bg_sub_NCLR,
      PALTYPE_SUB_BG,
      0x20*PALSRCPOS_SUB_EXPLAIN,
      0x20*PALNUM_SUB_EXPLAIN,
      0x20*PALAMOUNT_SUB_EXPLAIN,
      HEAPID_PMS_INPUT_VIEW );
  {
    // 足りない色をつくっておく
    BOOL result;

    cwk->vwk->explain_font_color_l_s[0] = GX_RGB(11, 10, 10);
    result = NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        PALNUM_SUB_EXPLAIN * 0x20 + EXPLAIN_COL_L * 2 ,
                                        &(cwk->vwk->explain_font_color_l_s[0]) , 2 );
    GF_ASSERT( result );

    cwk->vwk->explain_font_color_l_s[1] = GX_RGB(20, 20, 21);
    result = NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        PALNUM_SUB_EXPLAIN * 0x20 + EXPLAIN_COL_S * 2 ,
                                        &(cwk->vwk->explain_font_color_l_s[1]) , 2 );
    GF_ASSERT( result );
  }

  GFL_ARCHDL_UTIL_TransVramBgCharacter(	p_handle, NARC_pmsi_pms_bg_sub_NCGR,
						FRM_SUB_EXPLAIN, 0, 0, 0, HEAPID_PMS_INPUT_VIEW );
  
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_pmsi_pms2_bg_main4_NSCR,
						FRM_SUB_EXPLAIN, 0, 0, 0, HEAPID_PMS_INPUT_VIEW );
  GFL_BG_ChangeScreenPalette( FRM_SUB_EXPLAIN, 0, 0, 32, 32, PALNUM_SUB_EXPLAIN );

  cwk->vwk->explain_bmpwin = GFL_BMPWIN_Create(
      FRM_SUB_EXPLAIN,
      3, 5, 
      26, 7,
      PALNUM_SUB_EXPLAIN,
      GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( cwk->vwk->explain_bmpwin ), EXPLAIN_COL_B );
  cwk->vwk->explain_bmpwin_trans_req = FALSE;

  // 上画面説明欄に表示するメッセージのウィンドウ
  if(    PMSI_GetLockFlag( cwk->mwk ) 
      || PMSI_GetInputMode( cwk->mwk ) != PMSI_MODE_SENTENCE )
  {
    set_explain_message( cwk, PMSIV_EXPLAIN_MSG_EDIT_PECULIAR );
  }
  else
  {
    set_explain_message( cwk, PMSIV_EXPLAIN_MSG_EDIT_SENTENCE );
  }


  // 背景面転送
  PMSIView_SetBackScreen( cwk->vwk, FALSE );

//	cwk->vwk->sub_wk = PMSIV_SUB_Create( cwk->vwk, cwk->mwk, cwk->dwk );
//	PMSIV_SUB_SetupGraphicDatas( cwk->vwk->sub_wk, p_handle );

  // メニュー
  cwk->vwk->menu_wk = PMSIV_MENU_Create( cwk->vwk, cwk->mwk, cwk->dwk );

  // ボタン表示
  PMSIV_MENU_SetupEdit( cwk->vwk->menu_wk );
  GFL_BG_SetVisible( FRM_MAIN_TASKMENU, TRUE );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GX_DispOn();

	GFL_ARC_CloseDataHandle( p_handle );

	DeleteCommand(cwk);
}

//------------------------------------------------------------------
/**
	* OBJグラフィックデータ転送
	*
	* @param   cwk		
	*
	*/
//------------------------------------------------------------------
static void setup_obj_graphic( COMMAND_WORK* cwk, ARCHANDLE* p_handle )
{
	int i;
	PMS_INPUT_VIEW* vwk = cwk->vwk;

	for(i = 0;i < 2;i++){
		vwk->resCell[i].pltIdx = GFL_CLGRP_PLTT_Register( p_handle ,
        NARC_pmsi_pms_obj_main_NCLR , CLSYS_DRAW_MAIN+i , 0 , HEAPID_PMS_INPUT_VIEW );
		vwk->resCell[i].ncgIdx = GFL_CLGRP_CGR_Register( p_handle ,
        NARC_pmsi_pms_obj_main_NCGR , FALSE , CLSYS_DRAW_MAIN+i , HEAPID_PMS_INPUT_VIEW );
		vwk->resCell[i].anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle ,
        NARC_pmsi_pms_obj_main_NCER , NARC_pmsi_pms_obj_main_NANR, HEAPID_PMS_INPUT_VIEW );
		
    vwk->resCellDeco[i].pltIdx = GFL_CLGRP_PLTT_Register( p_handle ,
        NARC_pmsi_pms2_obj_dekome_NCLR , CLSYS_DRAW_MAIN+i , 0x20*PALNUM_OBJ_M_PMSDRAW , HEAPID_PMS_INPUT_VIEW );
		vwk->resCellDeco[i].ncgIdx = GFL_CLGRP_CGR_Register( p_handle ,
        NARC_pmsi_pms2_obj_dekome_NCGR , FALSE , CLSYS_DRAW_MAIN+i , HEAPID_PMS_INPUT_VIEW );
		vwk->resCellDeco[i].anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle ,
        NARC_pmsi_pms2_obj_dekome_64k_NCER , NARC_pmsi_pms2_obj_dekome_64k_NANR, HEAPID_PMS_INPUT_VIEW );
	}
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：描画終了
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_Quit( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* cwk = wk_adrs;
	PMS_INPUT_VIEW* vwk = cwk->vwk;

	switch( cwk->seq ){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, FADE_FRAMES, 1, 
			HEAPID_PMS_INPUT_VIEW );
		cwk->seq++;
		break;

	case 1:
		if( WIPE_SYS_EndCheck() )
		{
			int i;


      // 上画面説明欄
      GFL_BMPWIN_Delete( cwk->vwk->explain_bmpwin );


			PMSIV_EDIT_Delete( cwk->vwk->edit_wk );
//	  PMSIV_BUTTON_Delete( cwk->vwk->button_wk );
			PMSIV_CATEGORY_Delete( cwk->vwk->category_wk );
			PMSIV_WORDWIN_Delete( cwk->vwk->wordwin_wk );
//		PMSIV_SUB_Delete( cwk->vwk->sub_wk );

			for(i=0; i<2; i++)
			{
				GFL_CLGRP_PLTT_Release( cwk->vwk->resCell[i].pltIdx );
				GFL_CLGRP_CGR_Release( cwk->vwk->resCell[i].ncgIdx );
				GFL_CLGRP_CELLANIM_Release( cwk->vwk->resCell[i].anmIdx );
				
        GFL_CLGRP_PLTT_Release( cwk->vwk->resCellDeco[i].pltIdx );
				GFL_CLGRP_CGR_Release( cwk->vwk->resCellDeco[i].ncgIdx );
				GFL_CLGRP_CELLANIM_Release( cwk->vwk->resCellDeco[i].anmIdx );
			}
			GFL_FONT_Delete(vwk->fontHandle);
			
      PRINTSYS_QUE_Delete(vwk->print_que);
			
			GFL_BG_FreeBGControl( FRM_MAIN_EDITAREA );
			GFL_BG_FreeBGControl( FRM_MAIN_CATEGORY );
			GFL_BG_FreeBGControl( FRM_MAIN_WORDWIN );
			GFL_BG_FreeBGControl( FRM_MAIN_BACK );
			GFL_BG_FreeBGControl( FRM_SUB_EDITAREA );
			GFL_BG_FreeBGControl( FRM_SUB_BG );
			GFL_BG_FreeBGControl( FRM_SUB_SEARCH_LIST );
	
			GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

			DeleteCommand(cwk);
		}
		break;
	}
}

//--------------------------------------------------------------
/**
	* BG システム設定
	*
	* @param   cwk		
	*/
//--------------------------------------------------------------
static void setup_bg_params( COMMAND_WORK* cwk )
{
	int i;

	static const GFL_BG_SYS_HEADER sys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
	};

	// BG0 MAIN
	static const GFL_BG_BGCNT_HEADER header_main0 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x00000,0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
	// BG1 MAIN
	static const GFL_BG_BGCNT_HEADER header_main1 = {
		0, 0, 0x1000, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000,0x8000,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
	// BG2 MAIN
	static const GFL_BG_BGCNT_HEADER header_main2 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000,0x8000,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
	// BG3 MAIN
	static const GFL_BG_BGCNT_HEADER header_main3 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x08000,0x5000,//0x6000,
		GX_BG_EXTPLTT_01, 3, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
	// BG0 SUB エディットエリア
	static const GFL_BG_BGCNT_HEADER header_sub0 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,0x8000,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
  // 背景
	static const GFL_BG_BGCNT_HEADER header_sub1 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000,0x8000,
		GX_BG_EXTPLTT_01, 3, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
  // 単語予測リスト
	static const GFL_BG_BGCNT_HEADER header_sub2 = {
		0, 0, 0x1000, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x14000,0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};
  // 上画面説明欄
	static const GFL_BG_BGCNT_HEADER header_sub3 = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x1c000,0x4000,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};

	PMS_INPUT_VIEW* vwk = cwk->vwk;


	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
	//GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);  // GFL_BG_SetBGModeを直後で呼んでいるから不要でしょ&3Dじゃないでしょ

	GFL_DISP_SetBank( &bank_data );
	GFL_BG_SetBGMode( &sys_data );


	GFL_BG_SetBGControl( FRM_MAIN_EDITAREA,  &header_main0, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_MAIN_CATEGORY,  &header_main1, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_MAIN_WORDWIN,   &header_main2, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_MAIN_BACK,      &header_main3, GFL_BG_MODE_TEXT );

	GFL_BG_SetBGControl( FRM_SUB_EDITAREA, &header_sub0, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_SUB_BG, &header_sub1, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_SUB_SEARCH_LIST, &header_sub2, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRM_SUB_EXPLAIN, &header_sub3, GFL_BG_MODE_TEXT );

	for( i = GFL_BG_FRAME0_M; i <= GFL_BG_FRAME3_S; i++ ){
		GFL_BG_ClearScreen(GFL_BG_FRAME0_M+i);
		GFL_BG_SetVisible(GFL_BG_FRAME0_M+i,TRUE);
	}

	//ボタンフォント読み出し
	vwk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_PMS_INPUT_VIEW );
      
  vwk->print_que = PRINTSYS_QUE_Create( HEAPID_PMS_INPUT_VIEW );
}


//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：フェードイン
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_FadeIn( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* cwk = wk_adrs;
	PMS_INPUT_VIEW* vwk = cwk->vwk;

	switch( cwk->seq ){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, FADE_FRAMES, 1, 
			HEAPID_PMS_INPUT_VIEW );

		cwk->seq++;
		break;

	case 1:
		if( WIPE_SYS_EndCheck() )
		{
			cwk->seq++;
		}
		break;

	default:
		DeleteCommand(cwk);
		break;
	}
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリア更新
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_UpdateEditArea( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_UpdateEditArea( vwk->edit_wk );
	PMSIV_EDIT_MoveCursor( vwk->edit_wk, PMSI_GetEditAreaCursorPos( wk->mwk ) );
  PMSIV_MENU_UpdateEditIcon( vwk->menu_wk );

	DeleteCommand( wk );
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリア タッチorキー更新
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ChangeKTEditArea( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	if(*vwk->p_key_mode == GFL_APP_KTST_TOUCH){	//キーからタッチへ
		PMSIV_EDIT_VisibleCursor( vwk->edit_wk, FALSE );
//		PMSIV_BUTTON_VisibleCursor( vwk->button_wk, FALSE );
	}else{	//タッチからキーへ
		if(vwk->status == PMSI_ST_BUTTON){
//			PMSIV_BUTTON_VisibleCursor( vwk->button_wk, TRUE );
		}else{
			PMSIV_EDIT_VisibleCursor( vwk->edit_wk, TRUE );
		}
	}
	DeleteCommand( wk );
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリア タッチorキー更新
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ChangeKTCategory( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_CATEGORY_VisibleCursor(vwk->category_wk,TRUE);
#if 0 
	if(*vwk->p_key_mode == APP_KTST_TOUCH){	//キーからタッチへ
		PMSIV__VisibleCursor( vwk->edit_wk, FALSE );
		PMSIV_BUTTON_VisibleCursor( vwk->button_wk, FALSE );
	}else{	//タッチからキーへ
		if(vwk->status == PMSI_ST_BUTTON){
			PMSIV_BUTTON_VisibleCursor( vwk->button_wk, TRUE );
		}else{
			PMSIV_EDIT_VisibleCursor( vwk->edit_wk, TRUE );
		}
	}
#endif
	DeleteCommand( wk );
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリア タッチorキー更新
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ChangeKTWordWin( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
	
	PMSIV_WORDWIN_VisibleCursor( vwk->wordwin_wk,TRUE);

#if 0 
	if(*vwk->p_key_mode == APP_KTST_TOUCH){	//キーからタッチへ
	}else{	//タッチからキーへ
	}
#endif
	DeleteCommand( wk );
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリアからコマンドボタンへ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_EditAreaToButton( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	vwk->status = PMSI_ST_BUTTON;

	PMSIV_EDIT_VisibleCursor( vwk->edit_wk, FALSE );
	PMSIV_EDIT_StopArrow( vwk->edit_wk );

  PMSIV_MENU_TaskMenuSetActive( vwk->menu_wk, PMSI_GetButtonCursorPos(vwk->main_wk), TRUE );

//	PMSIV_BUTTON_VisibleCursor( vwk->button_wk, TRUE );
//	PMSIV_BUTTON_MoveCursor( vwk->button_wk, PMSI_GetButtonCursorPos(vwk->main_wk) );

	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：コマンドボタンから編集エリアへ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ButtonToEditArea( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	vwk->status = PMSI_ST_EDIT;
  
  PMSIV_MENU_TaskMenuSetActive( vwk->menu_wk, PMSI_GetButtonCursorPos(vwk->main_wk), FALSE );
	
//	PMSIV_BUTTON_VisibleCursor( vwk->button_wk, FALSE );
	PMSIV_EDIT_ActiveArrow( vwk->edit_wk );
	PMSIV_EDIT_VisibleCursor( vwk->edit_wk, TRUE );

	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリアからカテゴリ選択へ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_EditAreaToCategory( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
	int flag1,flag2;
	
	switch( wk->seq ){
	case 0:
    HOSAKA_Printf("Cmd_EditAreaToCategory\n");
    PMSIV_MENU_SetupCategory( vwk->menu_wk );
		PMSIV_EDIT_StopCursor( vwk->edit_wk );
		PMSIV_EDIT_StopArrow( vwk->edit_wk );
		PMSIV_EDIT_ChangeSMsgWin(vwk->edit_wk,1);
		PMSIV_EDIT_SetSystemMessage( vwk->edit_wk,PMSIV_MSG_GUIDANCE);
//		PMSIV_BUTTON_Hide( vwk->button_wk );
		PMSIV_CATEGORY_StartEnableBG( vwk->category_wk );
		PMSIV_EDIT_ScrollSet( vwk->edit_wk,0);

    // 上画面説明欄に表示するメッセージのウィンドウ
    if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_GROUP_CATEGORY );
    }
    else //if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_INITIAL )
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_INITIAL_CATEGORY );
    }

		wk->seq++;
		break;

	case 1:
		flag1 = PMSIV_CATEGORY_WaitEnableBG( vwk->category_wk );
		flag2 = PMSIV_EDIT_ScrollWait( vwk->edit_wk);
		if(flag1 && flag2){
			PMSIV_CATEGORY_MoveCursor( vwk->category_wk, PMSI_GetCategoryCursorPos(vwk->main_wk) );
			PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, TRUE );
			DeleteCommand( wk );
		}
		break;
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリモード切替え（編集エリア・コマンドボタン選択動作中）
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ChangeCategoryModeDisable( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq ){
	case 0:
//		PMSIV_SUB_ChangeCategoryButton( vwk->sub_wk );
//		PMSIV_BUTTON_ChangeCategoryButton( vwk->button_wk );
		wk->seq++;
//		break;

	case 1:
    PMSIV_MENU_SetupCategory( vwk->menu_wk );
    PMSIV_CATEGORY_StartModeChange( vwk->category_wk );
		PMSIV_CATEGORY_ChangeModeBG( vwk->category_wk );
		wk->seq++;
		break;

	case 2:
		if(	PMSIV_CATEGORY_WaitModeChange( vwk->category_wk ) 
//		&&	PMSIV_SUB_WaitChangeCategoryButton( vwk->sub_wk )
//		&&	PMSIV_BUTTON_WaitChangeCategoryButton( vwk->button_wk )
		)
		{
			DeleteCommand( wk );
		}
		break;
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリモード切替え（カテゴリ選択動作中）
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ChangeCategoryModeEnable( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
	int flag1,flag2;

	switch( wk->seq ){
	case 0:
//		PMSIV_SUB_ChangeCategoryButton( vwk->sub_wk );
//		PMSIV_BUTTON_ChangeCategoryButton( vwk->button_wk );
    PMSIV_MENU_SetDecideCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_CHANGE );
    PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, FALSE ); 
		wk->seq++;
		break;

	case 1:
    {
      flag1 = PMSIV_CATEGORY_WaitMoveSubWinList( vwk->category_wk, FALSE );
      flag2 = PMSIV_MENU_IsFinishCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_CHANGE );

      if( flag1 && flag2 )
      {
        PMSIV_MENU_ResetDecideCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_CHANGE );

        PMSIV_MENU_SetupCategory( vwk->menu_wk );

        PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, FALSE );
        PMSIV_CATEGORY_StartModeChange( vwk->category_wk );
        PMSIV_CATEGORY_ChangeModeBG( vwk->category_wk );

        // 上画面説明欄に表示するメッセージのウィンドウ
        if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
        {
          set_explain_message( wk, PMSIV_EXPLAIN_MSG_GROUP_CATEGORY );
        }
        else //if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_INITIAL )
        {
          set_explain_message( wk, PMSIV_EXPLAIN_MSG_INITIAL_CATEGORY );
        }

        wk->seq++;
      }
    }
		break;

	case 2:
		if( PMSIV_CATEGORY_WaitModeChange( vwk->category_wk ) )
		{
			wk->seq++;
		}
		break;

	case 3:
//		if( PMSIV_SUB_WaitChangeCategoryButton( vwk->sub_wk ) )
//		if( PMSIV_BUTTON_WaitChangeCategoryButton( vwk->button_wk ) )
		{
			PMSIV_CATEGORY_MoveCursor( vwk->category_wk, PMSI_GetCategoryCursorPos(vwk->main_wk) );
			PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, TRUE );
			DeleteCommand( wk );
		}
		break;
	}
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリ選択から編集エリアへ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_CategoryToEditArea( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
	int flag1,flag2;

	switch( wk->seq ){
	case 0:
    HOSAKA_Printf("Cmd_CategoryToEditArea\n");
    PMSIV_MENU_SetDecideCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_CANCEL );
    PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, FALSE );
//	PMSIV_BUTTON_Appear( vwk->button_wk );
		wk->seq++;
		break;

	case 1:
    {
      flag1 = PMSIV_CATEGORY_WaitMoveSubWinList( vwk->category_wk, FALSE );
      flag2 = PMSIV_MENU_IsFinishCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_CANCEL );
		  
      if(flag1 && flag2)
      {
        wk->seq++;
      }
    }
    break;

  case 2:
    PMSIV_MENU_SetupEdit( vwk->menu_wk );
    PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, FALSE );
    PMSIV_CATEGORY_StartDisableBG( vwk->category_wk );
    PMSIV_EDIT_ChangeSMsgWin(vwk->edit_wk,0);
    PMSIV_EDIT_SetSystemMessage( vwk->edit_wk,PMSIV_MSG_GUIDANCE);
    PMSIV_EDIT_ScrollSet( vwk->edit_wk,1);

    // 上画面説明欄に表示するメッセージのウィンドウ
    if(    PMSI_GetLockFlag( wk->mwk )
        || PMSI_GetInputMode( wk->mwk ) != PMSI_MODE_SENTENCE )
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_EDIT_PECULIAR );
    }
    else
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_EDIT_SENTENCE );
    }

    wk->seq++;
    break;

	case 3:
		flag1 = PMSIV_CATEGORY_WaitDisableBG( vwk->category_wk );
		flag2 = PMSIV_EDIT_ScrollWait( vwk->edit_wk);
		if(flag1 && flag2)
    {
			PMSIV_EDIT_ActiveArrow( vwk->edit_wk );
			PMSIV_EDIT_ActiveCursor( vwk->edit_wk );
			vwk->status = PMSI_ST_EDIT;
			DeleteCommand( wk );
		}
		break;
	}
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリ選択から単語選択へ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_CategoryToWordWin( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq )
  {
	case 0:
    PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, FALSE );
    PMSIV_MENU_SetDecideCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_SEARCH );
    wk->seq++;
    break;

  case 1:
    if( PMSIV_MENU_IsFinishCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_SEARCH ) )
    {
      PMSIV_CATEGORY_StartFadeOut( vwk->category_wk );
      PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, FALSE );
      wk->seq++;
    }
    break;

  case 2 :
    {
      BOOL flag1 = PMSIV_CATEGORY_WaitMoveSubWinList( vwk->category_wk, FALSE );
      BOOL flag2 = PMSIV_CATEGORY_WaitFadeOut( vwk->category_wk );

      if( flag1 && flag2 )
      {
        PMSIV_MENU_SetupWordWin( vwk->menu_wk );
		    PMSIV_WORDWIN_SetupWord( vwk->wordwin_wk );
        PMSIV_WORDWIN_StartFadeIn( vwk->wordwin_wk );

        // 上画面説明欄に表示するメッセージのウィンドウ
        if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
        {
          set_explain_message( wk, PMSIV_EXPLAIN_MSG_GROUP_WORDWIN );
        }
        else //if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_INITIAL )
        {
          set_explain_message( wk, PMSIV_EXPLAIN_MSG_INITIAL_WORDWIN );
        }
        
        wk->seq++;
      }
		}
		break;

	case 3:
		if( PMSIV_WORDWIN_WaitFadeIn( vwk->wordwin_wk ) )
		{
			PMSIV_WORDWIN_MoveCursor( vwk->wordwin_wk, PMSI_GetWordWinCursorPos(vwk->main_wk) );
			PMSIV_WORDWIN_VisibleCursor( vwk->wordwin_wk, TRUE );
			PMSIV_SUB_VisibleArrowButton( vwk->sub_wk, TRUE );
			DeleteCommand( wk );
		}
		break;

  default : GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語選択からカテゴリ選択へ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_WordWinToCategory( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
  BOOL flag1;
  BOOL flag2;
  u32 count;
  
  // 検索個数を取得
  count = PMSI_GetSearchResultCount(wk->mwk);

	switch( wk->seq ){
	case 0:
//		PMSIV_SUB_ChangeCategoryButton( vwk->sub_wk );
//		PMSIV_BUTTON_ChangeCategoryButton( vwk->button_wk );
    PMSIV_MENU_SetupCategory( vwk->menu_wk );
		wk->seq++;
		break;

	case 1:
		PMSIV_WORDWIN_VisibleCursor( vwk->wordwin_wk, FALSE );
//		PMSIV_SUB_VisibleArrowButton( vwk->sub_wk, FALSE );
		PMSIV_WORDWIN_StartFadeOut( vwk->wordwin_wk );
		wk->seq++;
		break;

	case 2:
		if( PMSIV_WORDWIN_WaitFadeOut( vwk->wordwin_wk ) )
		{
			PMSIV_CATEGORY_ChangeModeBG( vwk->category_wk );
			wk->seq++;
		}
		break;

	case 3:
    // このタイミングでBG背景面を読み込んでしまう
    PMSIView_SetBackScreen( wk->vwk, FALSE );
		PMSIV_CATEGORY_StartFadeIn( vwk->category_wk );

    if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
  	{
      PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, FALSE );  // フシギバナ ヒトカゲ リザード → ポケモン ステータス わざ
    }
    else
    {
      PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, (count>0) );  // あく アクアテール → あ い う え お か き
    }

    // 上画面説明欄に表示するメッセージのウィンドウ
    if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_GROUP_CATEGORY );
    }
    else //if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_INITIAL )
    {
      set_explain_message( wk, PMSIV_EXPLAIN_MSG_INITIAL_CATEGORY );
    }

		wk->seq++;
		break;

	case 4:
		flag1 = PMSIV_CATEGORY_WaitFadeIn( vwk->category_wk );
    flag2 = PMSIV_CATEGORY_WaitMoveSubWinList( vwk->category_wk, (count>0) );

		if( flag1 && flag2 )
		{
			PMSIV_CATEGORY_MoveCursor( vwk->category_wk, PMSI_GetCategoryCursorPos(vwk->main_wk) );
			PMSIV_CATEGORY_VisibleCursor( vwk->category_wk, TRUE );
			DeleteCommand( wk );
		}
		break;
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語決定後、編集エリアへ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_WordWinToEditArea( GFL_TCB *tcb, void* wk_adrs )
{
	int flag1,flag2;
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq ){
	case 0:
//		PMSIV_BUTTON_Appear( vwk->button_wk );
    PMSIV_MENU_SetupEdit( vwk->menu_wk );
		PMSIV_WORDWIN_VisibleCursor( vwk->wordwin_wk, FALSE );
//		PMSIV_SUB_VisibleArrowButton( vwk->sub_wk, FALSE );
		PMSIV_WORDWIN_StartFadeOut( vwk->wordwin_wk );
		PMSIV_EDIT_ChangeSMsgWin(vwk->edit_wk,0);
		PMSIV_EDIT_SetSystemMessage( vwk->edit_wk,PMSIV_MSG_GUIDANCE);
		PMSIV_EDIT_ScrollSet( vwk->edit_wk,1);
		wk->seq++;
		break;

	case 1:
		flag1 = PMSIV_WORDWIN_WaitFadeOut( vwk->wordwin_wk );
		flag2 = PMSIV_EDIT_ScrollWait( vwk->edit_wk);
		if(flag1 && flag2){
			PMSIV_CATEGORY_SetDisableBG( vwk->category_wk );
      // このタイミングでBG背景面を読み込んでしまう
      PMSIView_SetBackScreen( wk->vwk, FALSE );
			PMSIV_CATEGORY_StartFadeIn( vwk->category_wk );

      // 上画面説明欄に表示するメッセージのウィンドウ
      if(    PMSI_GetLockFlag( wk->mwk ) 
          || PMSI_GetInputMode( wk->mwk ) != PMSI_MODE_SENTENCE )
      {
        set_explain_message( wk, PMSIV_EXPLAIN_MSG_EDIT_PECULIAR );
      }
      else
      {
        set_explain_message( wk, PMSIV_EXPLAIN_MSG_EDIT_SENTENCE );
      }

      wk->seq++;
		}
		break;

	case 2:
		if( PMSIV_CATEGORY_WaitFadeIn( vwk->category_wk ) )
		{
			PMSIV_CATEGORY_StartBrightDown( vwk->category_wk );
			wk->seq++;
		}
		break;

	case 3:
		if( PMSIV_CATEGORY_WaitBrightDown( vwk->category_wk ) )
		{
			PMSIV_EDIT_UpdateEditArea( vwk->edit_wk );
			PMSIV_EDIT_VisibleCursor( vwk->edit_wk, TRUE );
			PMSIV_EDIT_ActiveArrow( vwk->edit_wk );
			PMSIV_EDIT_ActiveCursor( vwk->edit_wk );
			vwk->status = PMSI_ST_EDIT;
			DeleteCommand( wk );
		}
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語決定後、コマンドボタンへ
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_WordWinToButton( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq ){
	case 0:
//		PMSIV_BUTTON_Appear( vwk->button_wk );
		PMSIV_WORDWIN_VisibleCursor( vwk->wordwin_wk, FALSE );
//		PMSIV_SUB_VisibleArrowButton( vwk->sub_wk, FALSE );
		PMSIV_WORDWIN_StartFadeOut( vwk->wordwin_wk );
		wk->seq++;
		break;

	case 1:
		if( PMSIV_WORDWIN_WaitFadeOut( vwk->wordwin_wk ) )
		{
			PMSIV_CATEGORY_SetDisableBG( vwk->category_wk );
      // このタイミングでBG背景面を読み込んでしまう
      PMSIView_SetBackScreen( wk->vwk, FALSE );
			PMSIV_CATEGORY_StartFadeIn( vwk->category_wk );
			wk->seq++;
		}
		break;

	case 2:
		if( PMSIV_CATEGORY_WaitFadeIn( vwk->category_wk ) )
		{
			PMSIV_CATEGORY_StartBrightDown( vwk->category_wk );
			wk->seq++;
		}
		break;

	case 3:
		if( PMSIV_CATEGORY_WaitBrightDown( vwk->category_wk ) )
		{
			PMSIV_EDIT_UpdateEditArea( vwk->edit_wk );
			PMSIV_EDIT_VisibleCursor( vwk->edit_wk, FALSE );
//			PMSIV_BUTTON_VisibleCursor( vwk->button_wk, TRUE );
			DeleteCommand( wk );
		}
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：編集エリアのカーソル移動
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_MoveEditAreaCursor( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;


	PMSIV_EDIT_MoveCursor( vwk->edit_wk, PMSI_GetEditAreaCursorPos(vwk->main_wk) );
	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：ボタンカーソル移動
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_MoveButtonCursor( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;
  
  PMSIV_MENU_TaskMenuSetActive( vwk->menu_wk, PMSI_GetButtonCursorPos(vwk->main_wk), TRUE );

//	PMSIV_BUTTON_MoveCursor( vwk->button_wk, PMSI_GetButtonCursorPos(vwk->main_wk) );
	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリカーソル移動
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_MoveCategoryCursor( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_CATEGORY_MoveCursor( vwk->category_wk, PMSI_GetCategoryCursorPos(vwk->main_wk) );
	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語ウィンドウカーソル移動
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_MoveWordWinCursor( GFL_TCB *tcb, void* wk_adrs  )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_WORDWIN_MoveCursor( vwk->wordwin_wk, PMSI_GetWordWinCursorPos(vwk->main_wk) );
	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語ウィンドウスクロール
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ScrollWordWin( GFL_TCB *tcb, void* wk_adrs  )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq ){
	case 0:
		PMSIV_WORDWIN_StartScroll( vwk->wordwin_wk, PMSI_GetWordWinScrollVector(vwk->main_wk) );
		wk->seq++;
		break;

	case 1:
		if( PMSIV_WORDWIN_WaitScroll( vwk->wordwin_wk ) )
		{
//			PMSIV_SUB_VisibleArrowButton( vwk->sub_wk, TRUE );
			DeleteCommand( wk );
		}
		break;
	}
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：初期状態メッセージ表示
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_DispMessageDefault( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_SetSystemMessage( vwk->edit_wk, PMSIV_MSG_GUIDANCE );

	DeleteCommand( wk );
}

#if 0
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：これでいいですか？メッセージ表示
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_DispMessageOK( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_SetSystemMessage( vwk->edit_wk, PMSIV_MSG_CONFIRM_DECIDE );

//	if( PMSIV_BUTTON_GetCursorVisibleFlag( vwk->button_wk ) )
	if(*vwk->p_key_mode == GFL_APP_KTST_TOUCH){
//		PMSIV_BUTTON_UpdateButton(wk->vwk->button_wk,FALSE,FALSE);
	}else if( vwk->status == PMSI_ST_BUTTON){
//		PMSIV_BUTTON_UpdateButton(wk->vwk->button_wk,FALSE,FALSE);
//		PMSIV_BUTTON_StopCursor( vwk->button_wk );
	}else{
		PMSIV_EDIT_StopCursor( vwk->edit_wk );
	}
	PMSIV_EDIT_DispYesNoWin( vwk->edit_wk, PMSI_GetMenuCursorPos(wk->mwk) );

	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：やめますか？メッセージ表示
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_DispMessageCancel( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_SetSystemMessage( vwk->edit_wk, PMSIV_MSG_CONFIRM_CANCEL );

//	if( PMSIV_BUTTON_GetCursorVisibleFlag( vwk->button_wk ) )
	if(*vwk->p_key_mode == GFL_APP_KTST_TOUCH){
//		PMSIV_BUTTON_UpdateButton(wk->vwk->button_wk,FALSE,FALSE);
	}else if( vwk->status == PMSI_ST_BUTTON){
//		PMSIV_BUTTON_UpdateButton(wk->vwk->button_wk,FALSE,FALSE);
//		PMSIV_BUTTON_StopCursor( vwk->button_wk );
	}else{
		PMSIV_EDIT_StopCursor( vwk->edit_wk );
	}
	PMSIV_EDIT_DispYesNoWin( vwk->edit_wk, PMSI_GetMenuCursorPos(wk->mwk) );

	DeleteCommand( wk );
}
#endif
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：何かことばを入れてください！メッセージ表示
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_DispMessageWarn( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_SetSystemMessage( vwk->edit_wk, PMSIV_MSG_WARN_INPUT );
	DeleteCommand( wk );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  タスクメニュー決定演出
 *
 *	@param	GFL_TCB tcb
 *	@param	wk_adrs 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void Cmd_TaskMenuDecide( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

  PMSIV_MENU_TaskMenuSetDecide( vwk->menu_wk, PMSI_GetButtonCursorPos(wk->mwk), TRUE );

  DeleteCommand( wk );
}


//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：はい／いいえカーソル移動
	*
	* @param   tcb			
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_MoveMenuCursor( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

  GF_ASSERT(0);

	DeleteCommand( wk );
}
//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：メニュー表示消去
	*
	* @param   tcb			
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_EraseMenu( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_EDIT_SetSystemMessage( vwk->edit_wk, PMSIV_MSG_GUIDANCE );

//	if( PMSIV_BUTTON_GetCursorVisibleFlag( vwk->button_wk ) )
	if( vwk->status == PMSI_ST_BUTTON )
	{
//		PMSIV_BUTTON_UpdateButton( vwk->button_wk,TRUE,TRUE);
//		PMSIV_BUTTON_ActiveCursor( vwk->button_wk );
	}
	else
	{
//		PMSIV_BUTTON_UpdateButton( vwk->button_wk,TRUE,FALSE);
		PMSIV_EDIT_ActiveCursor( vwk->edit_wk );
	}

	DeleteCommand( wk );
}

//------------------------------------------------------------------
/**
	* 描画コマンド：三角ボタン（上）押した状態に
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void Cmd_ButtonUpHold(GFL_TCB *tcb, void* wk_adrs)
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

//	PMSIV_SUB_ChangeArrowButton( vwk->sub_wk, SUB_BUTTON_UP, SUB_BUTTON_STATE_HOLD );

	DeleteCommand( wk );
}
//------------------------------------------------------------------
/**
	* 描画コマンド：三角ボタン（下）押した状態に
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void Cmd_ButtonDownHold(GFL_TCB *tcb, void* wk_adrs)
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

//	PMSIV_SUB_ChangeArrowButton( vwk->sub_wk, SUB_BUTTON_DOWN, SUB_BUTTON_STATE_HOLD );

	DeleteCommand( wk );
}
//------------------------------------------------------------------
/**
	* 描画コマンド：三角ボタン（上）離した状態に
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void Cmd_ButtonUpRelease(GFL_TCB *tcb, void* wk_adrs)
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

//	PMSIV_SUB_ChangeArrowButton( vwk->sub_wk, SUB_BUTTON_UP, SUB_BUTTON_STATE_RELEASE );

	DeleteCommand( wk );
}
//------------------------------------------------------------------
/**
	* 描画コマンド：三角ボタン（下）離した状態に
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//------------------------------------------------------------------
static void Cmd_ButtonDownRelease(GFL_TCB *tcb, void* wk_adrs)
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

//	PMSIV_SUB_ChangeArrowButton( vwk->sub_wk, SUB_BUTTON_DOWN, SUB_BUTTON_STATE_RELEASE );

	DeleteCommand( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  描画コマンド：文字入力情報を更新
 *
 *	@param	GFL_TCB* tcb
 *	@param	wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void Cmd_InputWordUpdate( GFL_TCB* tcb, void* wk_adrs )
{
	COMMAND_WORK* cwk = wk_adrs;
  PMS_INPUT_VIEW* vwk = cwk->vwk;
  u32 count;
  
  count = PMSI_GetSearchResultCount(cwk->mwk);

  switch( cwk->seq )
  {
  case 0 :
    PMSIV_CATEGORY_InputWordUpdate( vwk->category_wk );
    PMSIV_CATEGORY_StartMoveSubWinList( vwk->category_wk, (count>0) );
    cwk->seq++;
    break;
  case 1 :
    if( PMSIV_CATEGORY_WaitMoveSubWinList( vwk->category_wk, (count>0) ) )
    {
      cwk->seq++;
    }
    break;
  case 2 :
    DeleteCommand( cwk );
    break;
  default : GF_ASSERT(0);
  }
  

}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：単語ウィンドウスクロールバー
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_ScrollWordWinBar( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	PMSIV_WORDWIN_SetupWordBar( vwk->wordwin_wk, PMSI_GetWordWinLinePos(vwk->main_wk) );

	DeleteCommand( wk );
}

//----------------------------------------------------------------------------------------------
/**
	* 描画コマンド：カテゴリあいうえお入力において、1文字消去したとき、1文字消去ボタンを明滅させる
	*
	* @param   tcb		
	* @param   wk_adrs		
	*
	* @note   1文字消去が確定したときのみ、この描画コマンドを発行して下さい。
  *         1文字消去できるか判定や音鳴らしはしていません。
	*
	*/
//----------------------------------------------------------------------------------------------
static void Cmd_EraseInCategoryInitial( GFL_TCB *tcb, void* wk_adrs )
{
	COMMAND_WORK* wk = wk_adrs;
	PMS_INPUT_VIEW* vwk = wk->vwk;

	switch( wk->seq )
  {
	case 0:
    PMSIV_MENU_SetDecideCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_ERASE );
		wk->seq++;
		break;
	case 1:
    if( PMSIV_MENU_IsFinishCategory( vwk->menu_wk, CATEGORY_DECIDE_ID_ERASE ) )
    {
      if( PMSI_GetCategoryCursorPos(vwk->main_wk) == CATEGORY_POS_ERASE )
      {
        PMSIV_MENU_TaskMenuSetActive( vwk->menu_wk, 1, TRUE );
      }
			DeleteCommand( wk );
    }
		break;
	}
}




//==============================================================================================
// コントロールモジュールへの情報提供
//==============================================================================================
//------------------------------------------------------------------
/**
	* 文章モード時の、単語入力欄がいくつあるか取得
	* （文章解析を描画側で行っているため）
	*
	* @param   wk		
	*
	* @retval  u32		
	*/
//------------------------------------------------------------------
u32 PMSIView_GetSentenceEditPosMax( PMS_INPUT_VIEW* wk )
{
	return PMSIV_EDIT_GetWordPosMax( wk->edit_wk );
}

//------------------------------------------------------------------
/**
	* 文章モード時の、単語入力欄矩形取得
	* （文章解析を描画側で行っているため）
	*
	* @param   wk		
	*
	*/
//------------------------------------------------------------------
void PMSIView_GetSentenceWordArea( PMS_INPUT_VIEW* wk ,GFL_UI_TP_HITTBL* tbl,u8 idx)
{
	PMSIV_EDIT_GetSentenceWordPos( wk->edit_wk ,tbl,idx);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タスクメニュー 決定アニメ終了待ち
 *
 *	@param	wk  ワーク
 *
 *	@retval -1:アニメ中 1:終了
 */
//-----------------------------------------------------------------------------
int PMSIView_WaitYesNo(PMS_INPUT_VIEW* wk)
{
  if( PMSIV_MENU_TaskMenuIsFinish( wk->menu_wk, PMSI_GetButtonCursorPos( wk->main_wk ) ) )
  {
    return 1;  // Yesとみなす
  }

  return -1; // 終了していない

//	return PMSIV_EDIT_WaitYesNoBtn(wk->edit_wk);
}

void PMSIView_TouchEditButton( PMS_INPUT_VIEW* wk, int btn_id )
{
  PMSIV_MENU_TouchEditButton( wk->menu_wk, btn_id );
}

//==============================================================================================
// 下請けモジュールへの情報提供
//==============================================================================================

PMSIV_MENU* PMSIView_GetMenuWork( PMS_INPUT_VIEW* vwk )
{
  return vwk->menu_wk;
}

GFL_CLUNIT*  PMSIView_GetCellUnit( PMS_INPUT_VIEW* vwk )
{
	return vwk->cellUnit;
}

GFL_FONT*  PMSIView_GetFontHandle( PMS_INPUT_VIEW* vwk )
{
	return vwk->fontHandle;
}

PRINT_QUE* PMSIView_GetPrintQue( PMS_INPUT_VIEW* vwk )
{
  return vwk->print_que;
}

void PMSIView_GetDecoResource( PMS_INPUT_VIEW* vwk, PMSIV_CELL_RES* out_res, u32 lcd )
{
  GF_ASSERT( lcd < 2 );
	*out_res = vwk->resCellDeco[lcd];
}

void PMSIView_SetupDefaultActHeader( PMS_INPUT_VIEW* vwk, PMSIV_CELL_RES* header, u32 lcd, u32 bgpri )
{
#if 0
	header->pImageProxy   = &vwk->obj_image_proxy[lcd];
	header->pPaletteProxy = &vwk->obj_pltt_proxy[lcd];
	header->pCellBank     = vwk->cellbank[lcd];
	header->pAnimBank     = vwk->anmbank[lcd];
	header->priority      = bgpri;

	header->pCharData = NULL;
	header->pMCBank = NULL;
	header->pMCABank = NULL;
	header->flag = FALSE;
#endif
  GF_ASSERT( vwk );
  GF_ASSERT( lcd < 2 );
	*header = vwk->resCell[lcd];
}

GFL_CLWK* PMSIView_AddActor( PMS_INPUT_VIEW* vwk, PMSIV_CELL_RES* header, u32 x, u32 y, u32 actpri, int drawArea )
{
	GFL_CLWK_DATA	add;
	GFL_CLWK*		act = NULL;
	OSIntrMode		oldIntr;
	u16				setsf;

	add.pos_x = x;
	add.pos_y = y;
	add.softpri = actpri;
	add.bgpri = 0;
	add.anmseq = 0;


	if(drawArea == NNS_G2D_VRAM_TYPE_2DSUB ){
		setsf = CLSYS_DEFREND_SUB;
	}
	else
	{
		setsf = CLSYS_DEFREND_MAIN;
	}

	oldIntr = OS_DisableInterrupts();
	act = GFL_CLACT_WK_Create( vwk->cellUnit , header->ncgIdx , header->pltIdx ,header->anmIdx ,
								&add , setsf , HEAPID_PMS_INPUT_VIEW);
	OS_RestoreInterrupts( oldIntr );

	if( act )
	{
		GFL_CLACT_WK_SetAutoAnmFlag( act, TRUE );
		GFL_CLACT_WK_SetAutoAnmSpeed( act, PMSI_ANM_SPEED );
	}

	return act;
}

// スクロールバーの座標と指定座標の位置関係を取得
u32 PMSIView_GetScrollBarPos( PMS_INPUT_VIEW* vwk, u32 px, u32 py )
{
	GFL_CLACTPOS pos;

	PMSIV_WINDOW_GetScrollBarPos( vwk->wordwin_wk, &pos );
	if( py < (pos.y-(PMSIV_TPWD_BAR_SY/2)) ){
		return 1;		// 上
	}
	if( py >= (pos.y+(PMSIV_TPWD_BAR_SY/2)) ){
		return 2;		// 下
	}
	if( py >= (pos.y-(PMSIV_TPWD_BAR_SY/2)) &&
			py < (pos.y+(PMSIV_TPWD_BAR_SY/2)) &&
			px >= (pos.x-(PMSIV_TPWD_BAR_SX/2)) &&
			px < (pos.x+(PMSIV_TPWD_BAR_SX/2)) ){
		return 0;		// 同じ
	}
	return 3;			// その他
}

// スクロールバー座標セット
void PMSIView_SetScrollBarPos( PMS_INPUT_VIEW* vwk, u32 py )
{
	PMSIV_WORDWIN_MoveScrollBar( vwk->wordwin_wk, py );
}

// スクロールバーの座標からスクロール値を取得
u32 PMSIView_GetScrollBarPosCount( PMS_INPUT_VIEW* vwk, u32 max )
{
	return PMSIV_WORDWIN_GetScrollBarPosCount( vwk->wordwin_wk, max );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  背景面のBGを転送
 *
 *	@param	PMS_INPUT_VIEW* vwk
 *	@param	is_wordwin TRUE:単語リストウィンドウ FALSE:通常の背景
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIView_SetBackScreen( PMS_INPUT_VIEW* vwk, BOOL is_wordwin )
{
  ARCHANDLE* p_handle;
  u32 nscr, ncgr;

	p_handle = GFL_ARC_OpenDataHandle( ARCID_PMSI_GRAPHIC, HEAPID_PMS_INPUT_VIEW );

  if( is_wordwin )
  {
    nscr = NARC_pmsi_pms_bg_main3_NSCR;
    ncgr = NARC_pmsi_pms_bg_main3_NCGR;
  }
  else
  {
    nscr = NARC_pmsi_pms_bg_main2_NSCR;
    ncgr = NARC_pmsi_pms_bg_main2_NCGR;
  }

  // 背景面転送
	GFL_ARCHDL_UTIL_TransVramScreen(p_handle, nscr,
		FRM_MAIN_BACK, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(p_handle, ncgr,
		FRM_MAIN_BACK, 0, 0, FALSE, HEAPID_PMS_INPUT_VIEW );
  
  GFL_ARC_CloseDataHandle( p_handle );
	
  GFL_BG_LoadScreenReq( FRM_MAIN_BACK );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  上画面説明欄に表示するメッセージのウィンドウ
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void set_explain_message( COMMAND_WORK* cwk, PMSIV_EXPLAIN_MSG msg )
{
  const u32 str_id[PMSIV_EXPLAIN_MSG_MAX] =
  {
    info_02,
    info_03,
    info_07,
    info_04,
    info_05,
    info_06,
  };

  GFL_MSGDATA* msgdata;
  STRBUF* strbuf;
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( cwk->vwk->explain_bmpwin ), EXPLAIN_COL_B );

  msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pms_input_dat, HEAPID_PMS_INPUT_VIEW );
  strbuf = GFL_MSG_CreateString( msgdata, str_id[msg] );
  PRINTSYS_PrintQueColor(
        cwk->vwk->print_que,
        GFL_BMPWIN_GetBmp( cwk->vwk->explain_bmpwin ),
        2, 4,
        strbuf,
        cwk->vwk->fontHandle,
        PRINTSYS_LSB_Make( EXPLAIN_COL_L, EXPLAIN_COL_S, EXPLAIN_COL_B ) );
  cwk->vwk->explain_bmpwin_trans_req = TRUE;
  GFL_STR_DeleteBuffer( strbuf );
  GFL_MSG_Delete( msgdata );
}
static void trans_explain_message( PMS_INPUT_VIEW* vwk )
{
  // 上画面説明欄に表示するメッセージのウィンドウ
  if(    vwk->explain_bmpwin_trans_req
      && ( !PRINTSYS_QUE_IsExistTarget( vwk->print_que, GFL_BMPWIN_GetBmp( vwk->explain_bmpwin ) ) ) )
  {
    GFL_BMPWIN_MakeTransWindow_VBlank( vwk->explain_bmpwin );
    vwk->explain_bmpwin_trans_req = FALSE;
  }
}

