//==============================================================================
/**
 * @file	codein.c
 * @brief	文字入力インターフェース
 * @author	goto
 * @date	2007.07.11(水)
 *
 *	GSより移植 Ari090324
 *
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
//#include "poke_tool/pokeicon.h"
//#include "poke_tool/boxdata.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "codein_pv.h"
#include "codein_snd.h"



// ----------------------------------------
//
//	プロトタイプ
//
// ----------------------------------------
static void CI_VramBankSet( void );
static void CI_VBlank( GFL_TCB *tcb, void *work );

// ----------------------------------------
//
//	プロセス定義
//
// ----------------------------------------
static GFL_PROC_RESULT CI_Proc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT CI_Proc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT CI_Proc_Exit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

const GFL_PROC_DATA CodeInput_ProcData = {
	CI_Proc_Init,
	CI_Proc_Main,
	CI_Proc_Exit,
};

GFL_DISP_VRAM vramSetTable = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_32_H,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_01_BC,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_64K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

//--------------------------------------------------------------
/**
 * @brief	初期化
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CI_Proc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	CODEIN_WORK* wk;
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CODEIN, HEAP_SIZE_CODEIN );
	
	wk = GFL_PROC_AllocWork( proc, sizeof( CODEIN_WORK ), HEAPID_CODEIN );
	GFL_STD_MemFill( wk, 0, sizeof( CODEIN_WORK ) );
	
	if( pwk == NULL )
	{
    //デバッグ作成
    int block[ CODE_BLOCK_MAX ];
  	CODEIN_PARAM* pWork;
    CodeIn_BlockDataMake_4_4_4( block );
    pWork = CodeInput_ParamCreate( HEAPID_CODEIN, 12, block );
  	wk->param = *( ( CODEIN_PARAM* )pWork );
    GFL_HEAP_FreeMemory(pWork);
  }
  else
  {
  	wk->param = *( ( CODEIN_PARAM* )pwk );
  }
	
	///< 初期化
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
	GX_SetVisiblePlane( 0 );
	GXS_SetVisiblePlane( 0 );

	///< システムワークの作成	
	wk->sys.p_handle = GFL_ARC_OpenDataHandle( ARCID_CODE_INPUT, HEAPID_CODEIN );	
	GFL_BG_Init( HEAPID_CODEIN );
	GFL_BMPWIN_Init( HEAPID_CODEIN );
	wk->sys.pfd		 = PaletteFadeInit( HEAPID_CODEIN );
	
	wk->sys.fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
							GFL_FONT_LOADTYPE_FILE , TRUE , HEAPID_CODEIN );
	
	PaletteTrans_AutoSet( wk->sys.pfd, TRUE );
	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_MAIN_BG,	 0x200, HEAPID_CODEIN );
//	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_SUB_BG,	 0x200, HEAPID_CODEIN );
//	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_MAIN_OBJ, 0x200, HEAPID_CODEIN );
//	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_SUB_OBJ,	 0x200, HEAPID_CODEIN );	

	///< Vram設定
	CI_VramBankSet();	
	
	///< OAM設定
	CI_pv_disp_CATS_Init( wk , &vramSetTable );
	
	///< 設定初期化
	CI_pv_ParamInit( wk );
	
	///< Touch Panel設定
	{
		CI_pv_ButtonManagerInit( wk );
	}
	
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE,
					  GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2, 15, 7 );
					 
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE,
					   GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
					   GX_BLEND_PLANEMASK_BG3, 7, 8 );
					   
	wk->sys.vBlankTcb = GFUser_VIntr_CreateTCB( CI_VBlank , wk , 16 );
	
	
	
	return GFL_PROC_RES_FINISH;
}


//--------------------------------------------------------------
/**
 * @brief	メイン
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CI_Proc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	BOOL bUpdate;
	
	CODEIN_WORK* wk = mywk;
	
	bUpdate = CI_pv_MainUpdate( wk );
	
	GFL_CLACT_SYS_Main();
	return bUpdate ? GFL_PROC_RES_FINISH : GFL_PROC_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief	終了
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	static PROC_RESULT	
 *
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CI_Proc_Exit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	CODEIN_WORK* wk = mywk;
	
	GFL_TCB_DeleteTask( wk->sys.vBlankTcb );
	
	GFL_FONT_Delete( wk->sys.fontHandle );
	CI_pv_disp_CodeRes_Delete( wk );
	
	GFL_BMPWIN_Exit();
	
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3,  VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_Exit();
	
	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_MAIN_BG );
//	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_SUB_BG );
//	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_MAIN_OBJ );
//	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_SUB_OBJ );
	PaletteFadeFree( wk->sys.pfd );
	
	GFL_ARC_CloseDataHandle( wk->sys.p_handle );
	
//	CATS_ResourceDestructor_S( wk->sys.csp, wk->sys.crp );
//	CATS_FreeMemory( wk->sys.csp );
	
	GFL_BMN_Delete( wk->sys.btn );
	
  if( pwk == NULL )
  {
  	GFL_STR_DeleteBuffer( wk->param.strbuf );
  }
	GFL_PROC_FreeWork( proc );
	
	GFL_HEAP_DeleteHeap( HEAPID_CODEIN );
	
	return GFL_PROC_RES_FINISH;
}






//--------------------------------------------------------------
/**
 * @brief	VramBankSet
 *
 * @param	bgl	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CI_VramBankSet( void )
{
	GFL_DISP_GX_InitVisibleControl();
	
	//VRAM設定
	{
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}
	
		//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME1_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000 ,
				GX_BG_EXTPLTT_01, 0x01, 0, 0, FALSE
			},
			///<FRAME2_M
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x4000 ,
				GX_BG_EXTPLTT_01, 0x02, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x1800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x10000, 0x8000 ,
				GX_BG_EXTPLTT_01, 0x03, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	}
	
	///< サブ画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			{	/// font
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x4000 ,
				GX_BG_EXTPLTT_01 , 0x00, 0, 0, FALSE
			},
			{	/// bg
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x04000, 0x2000 ,
				GX_BG_EXTPLTT_01,0x01, 0, 0, FALSE
			},
			{
				0
			},
			{
				0
			},
		#if 0
			{	/// list
				0, 0, 0x800, 0, GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01,
				CB_BG_PRI_SCR_LIST, 0, 0, FALSE
			},
			{	/// edit
				0, 0, 0x800, 0, GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01,
				CB_BG_PRI_SCR_EDIT, 0, 0, FALSE
			},
		#endif
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_S );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	}
	
	///< SUB画面をメイン画面にするため
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
	
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * @brief	VBlank
 *
 * @param	work	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CI_VBlank( GFL_TCB *tcb, void *work )
{
	CODEIN_WORK* wk = work;

//	DoVramTransferManager();			///< Vram転送マネージャー実行
	
//	CATS_RenderOamTrans();

	PaletteFadeTrans( wk->sys.pfd );
	
//	GF_BGL_VBlankFunc( wk->sys.bgl );
	
	GFL_CLACT_SYS_VBlankFunc();
}
