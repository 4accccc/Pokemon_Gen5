//==============================================================================
/**
 * @file	codein_disp.c
 * @brief	文字入力インターフェース
 * @author	goto
 * @date	2007.07.11(水)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "print/wordset.h"
#include "print/printsys.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"

#include "sound/pm_sndsys.h"

#include "codein_pv.h"

#define USE_FontOam 0

//--------------------------------------------------------------
/**
 * @brief	CATS の 初期化
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CATS_Init( CODEIN_WORK* wk , const GFL_DISP_VRAM* vramBank )
{
	/*
	wk->sys.clwk = CATS_AllocMemory( HEAPID_CODEIN );
	{
		const TCATS_OAM_INIT OamInit = {
			0, 128, 0, 32,
			0, 128, 0, 32,
		};
		const TCATS_CHAR_MANAGER_MAKE Ccmm = {
			48 + 48,			///< ID管理 main + sub
			1024 * 0x40,		///< 64k
			 512 * 0x20,		///< 32k
			GX_OBJVRAMMODE_CHAR_1D_64K,
			GX_OBJVRAMMODE_CHAR_1D_32K
		};
		CATS_SystemInit( wk->sys.csp, &OamInit, &Ccmm, 16 + 16 );
	}
	
	{
		BOOL active;
		const TCATS_RESOURCE_NUM_LIST ResourceList = {
			48 + 48,	///< キャラ登録数 	main + sub
			16 + 16,	///< パレット登録数	main + sub
			64,			///< セル
			64,			///< セルアニメ
			16,			///< マルチセル
			16,			///< マルチセルアニメ
		};
		
		wk->sys.crp = CATS_ResourceCreate( wk->sys.csp );

		active   = CATS_ClactSetInit( wk->sys.csp, wk->sys.crp, 64 + 64 );
		GF_ASSERT( active );
		
		active	  = CATS_ResourceManagerInit( wk->sys.csp, wk->sys.crp, &ResourceList );
		GF_ASSERT( active );
	}	
	*/
	GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
	cellSysInitData.oamst_main = 0x10;	//デバッグメータの分
	cellSysInitData.oamnum_main = 128-0x10;
	GFL_CLACT_SYS_Create( &cellSysInitData , vramBank ,HEAPID_CODEIN );
	
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
	wk->sys.cellUnit  = GFL_CLACT_UNIT_Create( 64 , 0, HEAPID_CODEIN );
	GFL_CLACT_UNIT_SetDefaultRend( wk->sys.cellUnit );
}

//--------------------------------------------------------------
/**
 * @brief	コード用データ読み込み
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Load( CODEIN_WORK* wk )
{
	ARCHANDLE*		hdl;
//	GF_BGL_INI*		bgl;
	GFL_CLUNIT		*cellUnit;
//	CATS_RES_PTR	crp;
	PALETTE_FADE_PTR pfd;

	cellUnit = wk->sys.cellUnit;
//	crp = wk->sys.crp;
	pfd = wk->sys.pfd;	
//	bgl = wk->sys.bgl;
	hdl = wk->sys.p_handle;

	///< bg
//	ArcUtil_HDL_BgCharSet( hdl, NARC_codein_gra_code_bg_NCGR, bgl, GF_BGL_FRAME1_M, 0, 0, 0, HEAPID_CODEIN );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl , NARC_code_input_code_bg_NCGR , GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_CODEIN );
//	ArcUtil_HDL_ScrnSet( hdl, NARC_codein_gra_code_bg_NSCR, bgl, GF_BGL_FRAME1_M, 0, 0, 0, HEAPID_CODEIN );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl , NARC_code_input_code_bg_NSCR, GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_CODEIN );
	//090918 2本に変更しました。 Ariizumi
	PaletteWorkSet_Arc( pfd, ARCID_CODE_INPUT, NARC_code_input_code_bg_NCLR, HEAPID_CODEIN, FADE_MAIN_BG, 0x20 * 2, 0 );
	
	///< bg-top
//	ArcUtil_HDL_BgCharSet( hdl, NARC_codein_gra_code_bg_t_NCGR, bgl, GF_BGL_FRAME1_S, 0, 0, 0, HEAPID_CODEIN );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl , NARC_code_input_code_bg_t_NCGR , GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_CODEIN );
//	ArcUtil_HDL_ScrnSet( hdl, NARC_codein_gra_code_bg_t_NSCR, bgl, GF_BGL_FRAME1_S, 0, 0, 0, HEAPID_CODEIN );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl , NARC_code_input_code_bg_t_NSCR, GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_CODEIN );
	GFL_ARCHDL_UTIL_TransVramPalette( hdl , NARC_code_input_code_bg_t_NCLR , PALTYPE_SUB_BG , 0, 0, HEAPID_CODEIN );
	
	///< code
//	CATS_LoadResourcePlttWorkArcH( pfd, FADE_MAIN_OBJ, csp, crp, hdl, NARC_codein_gra_code_NCLR, FALSE, 1, NNS_G2D_VRAM_TYPE_2DMAIN, eID_CODE_OAM );
//	CATS_LoadResourceCharArcH( csp, crp, hdl, NARC_codein_gra_code_NCGR, FALSE, NNS_G2D_VRAM_TYPE_2DMAIN, eID_CODE_OAM );
//	CATS_LoadResourceCellArcH( csp, crp, hdl, NARC_codein_gra_code_NCER, FALSE, eID_CODE_OAM );
//	CATS_LoadResourceCellAnmArcH( csp, crp, hdl, NARC_codein_gra_code_NANR, FALSE, eID_CODE_OAM );	
	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_NCLR , CLSYS_DRAW_MAIN , 0,  HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_NCER , NARC_code_input_code_NANR , HEAPID_CODEIN );
	
	///< cur
//	CATS_LoadResourcePlttWorkArcH( pfd, FADE_MAIN_OBJ, csp, crp, hdl, NARC_codein_gra_code_cur_NCLR, FALSE, 1, NNS_G2D_VRAM_TYPE_2DMAIN, eID_CUR_OAM );
//	CATS_LoadResourceCharArcH( csp, crp, hdl, NARC_codein_gra_code_cur_NCGR, FALSE, NNS_G2D_VRAM_TYPE_2DMAIN, eID_CUR_OAM );
//	CATS_LoadResourceCellArcH( csp, crp, hdl, NARC_codein_gra_code_cur_NCER, FALSE, eID_CUR_OAM );
//	CATS_LoadResourceCellAnmArcH( csp, crp, hdl, NARC_codein_gra_code_cur_NANR, FALSE, eID_CUR_OAM );	
	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_cur_NCLR , CLSYS_DRAW_MAIN , 0x20,  HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_cur_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_cur_NCER , NARC_code_input_code_cur_NANR , HEAPID_CODEIN );

	///< button
//	CATS_LoadResourcePlttWorkArcH( pfd, FADE_MAIN_OBJ, csp, crp, hdl, NARC_codein_gra_code_button_NCLR, FALSE, 2, NNS_G2D_VRAM_TYPE_2DMAIN, eID_BTN_OAM );
//	CATS_LoadResourceCharArcH( csp, crp, hdl, NARC_codein_gra_code_button_NCGR, FALSE, NNS_G2D_VRAM_TYPE_2DMAIN, eID_BTN_OAM );
//	CATS_LoadResourceCellArcH( csp, crp, hdl, NARC_codein_gra_code_button_NCER, FALSE, eID_BTN_OAM );
//	CATS_LoadResourceCellAnmArcH( csp, crp, hdl, NARC_codein_gra_code_button_NANR, FALSE, eID_BTN_OAM );	
	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_button_NCLR , CLSYS_DRAW_MAIN , 0x40,  HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_button_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_button_NCER , NARC_code_input_code_button_NANR , HEAPID_CODEIN );
	
	///< font
	{
//		int wintype = CONFIG_GetWindowType( wk->cfg );
//		TalkWinGraphicSet( bgl, GF_BGL_FRAME0_S, 1, ePAL_WIN, wintype, HEAPID_CODEIN );
		TalkWinFrame_GraphicSet( GFL_BG_FRAME0_S , 1 , ePAL_FRAME , 0 , HEAPID_CODEIN );
//		PaletteWorkSet_Arc( pfd, ARC_WINFRAME, TalkWinPalArcGet( wintype ), HEAPID_CODEIN, FADE_SUB_BG, 0x20, ePAL_FRAME * 16 );

		///< フォント
//		PaletteWorkSet_Arc( pfd, ARC_FONT, NARC_font_talk_ncrl, HEAPID_CODEIN, FADE_SUB_BG, 0x20, ePAL_FONT * 16 );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , ePAL_FONT*0x20, 16*2, HEAPID_CODEIN );
	}
}


//--------------------------------------------------------------
/**
 * @brief	もろもろ解放
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Delete( CODEIN_WORK* wk )
{
	int i;
	
	for ( i = 0; i < wk->code_max; i++ ){
		GFL_CLACT_WK_Remove( wk->code[ i ].clwk );
	}
	
	for ( i = 0; i < 2; i++ ){
		GFL_CLACT_WK_Remove( wk->bar[ i ].clwk );
		GFL_CLACT_WK_Remove( wk->btn[ i ].clwk );
	}
	
	for ( i = 0; i < 3; i++ ){
		GFL_CLACT_WK_Remove( wk->cur[ i ].clwk );
	}
	
	CI_pv_FontOam_SysDelete( wk );
	
	GFL_BMPWIN_Delete( wk->sys.win );
	
	//リソースの開放
	for( i=0;i<CIO_RES2_MAX;i++ )
	{
		GFL_CLGRP_PLTT_Release( wk->sys.resIdx[CIO_RES1_PLT][i] );
		GFL_CLGRP_CGR_Release( wk->sys.resIdx[CIO_RES1_NCG][i] );
		GFL_CLGRP_CELLANIM_Release( wk->sys.resIdx[CIO_RES1_ANM][i] );
	}
	
	GFL_CLACT_UNIT_Delete( wk->sys.cellUnit );
	GFL_CLACT_SYS_Delete();
	
//	CATS_ResourceDestructor_S( wk->sys.csp, wk->sys.crp );
}

//--------------------------------------------------------------
/**
 * @brief	コードOAM作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeOAM_Create( CODEIN_WORK* wk )
{
	int i;
	int i_c = 0;
	int i_b = 0;
	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;
	PALETTE_FADE_PTR pfd;

	cellUnit = wk->sys.cellUnit;
	pfd = wk->sys.pfd;

	clwkInit.pos_x = 0;
	clwkInit.pos_y = 0;
	clwkInit.anmseq = 0;
	clwkInit.softpri = 10;
	clwkInit.bgpri = 0;
	
	for ( i = 0; i < wk->code_max + BAR_OAM_MAX; i++ ){
		
		if ( i == ( wk->bar[ i_b ].state + i_b + 1 ) ){
			
			clwkInit.pos_x = 72 + ( i * 8 ) + 4;
			clwkInit.pos_y = POS_CODE_Y;
			clwkInit.anmseq = eANM_CODE_BAR;

			wk->bar[ i_b ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] ,
													wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] , 
													wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] ,
													&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
			i_b++;
		}
		else {			
			clwkInit.pos_x = 72 + ( i * 8 ) + 4;
			clwkInit.pos_y = POS_CODE_Y;
			clwkInit.anmseq = CI_pv_disp_CodeAnimeGet( wk->code[ i_c ].state, wk->code[ i_c ].size );
			//TODO Affineにする必要あり？
			wk->code[ i_c ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] ,
													wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] , 
													wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] ,
													&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
			
			//CATS_ObjectAffineSetCap( wk->code[ i_c ].cap, CLACT_AFFINE_DOUBLE );
//			GFL_CLACT_WK_SetAffineParam( wk->code[ i_c ].clwk , CLSYS_AFFINETYPE_DOUBLE );
			//CATS_ObjectUpdateCap( wk->code[ i_c ].cap );
			i_c++;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	カーソルOAMの作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Create( CODEIN_WORK* wk )
{	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;
	PALETTE_FADE_PTR pfd;

	cellUnit = wk->sys.cellUnit;
	pfd = wk->sys.pfd;
	
	clwkInit.pos_x = 0;
	clwkInit.pos_y = 0;
	clwkInit.anmseq = 0;
	clwkInit.softpri = 0;
	clwkInit.bgpri = 0;
	
	wk->cur[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
	wk->cur[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
	wk->cur[ 2 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
	
	CI_pv_disp_CurBar_PosSet( wk, 0 );
//	CATS_ObjectAnimeSeqSetCap( wk->cur[ 0 ].cap, 0 );
//	CATS_ObjectUpdateCap( wk->cur[ 0 ].cap );
	GFL_CLACT_WK_SetAnmSeq( wk->cur[ 0 ].clwk, 0 );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 0 ].clwk, TRUE );
	
	wk->cur[ 1 ].move_wk.pos.x = 0;
	wk->cur[ 1 ].move_wk.pos.y = 0;
	wk->cur[ 1 ].state = eANM_CUR_SQ;
	CI_pv_disp_CurSQ_PosSet( wk, 0 );
//	CATS_ObjectAnimeSeqSetCap( wk->cur[ 1 ].cap, wk->cur[ 1 ].state );
//	CATS_ObjectUpdateCap( wk->cur[ 1 ].cap );
	GFL_CLACT_WK_SetAnmSeq( wk->cur[ 1 ].clwk, wk->cur[ 1 ].state );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 1 ].clwk, TRUE );
//	CATS_ObjectObjModeSetCap( wk->cur[ 1 ].cap, GX_OAM_MODE_XLU );
	GFL_CLACT_WK_SetObjMode( wk->cur[ 1 ].clwk, GX_OAM_MODE_XLU );
	
	wk->cur[ 2 ].move_wk.pos.x = 0;
	wk->cur[ 2 ].move_wk.pos.y = 0;
	wk->cur[ 2 ].state = eANM_CUR_SQ;
	CI_pv_disp_CurSQ_PosSet( wk, 0 );
//	CATS_ObjectAnimeSeqSetCap( wk->cur[ 2 ].cap, wk->cur[ 2 ].state );
//	CATS_ObjectUpdateCap( wk->cur[ 2 ].cap );
	GFL_CLACT_WK_SetAnmSeq( wk->cur[ 2 ].clwk, wk->cur[ 2 ].state );
//	CATS_ObjectObjModeSetCap( wk->cur[ 2 ].cap, GX_OAM_MODE_XLU );
	GFL_CLACT_WK_SetObjMode( wk->cur[ 2 ].clwk, GX_OAM_MODE_XLU );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 2 ].clwk, TRUE );
	
	CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
	CI_pv_disp_CurOAM_Visible( wk, 2, FALSE );
}


//--------------------------------------------------------------
/**
 * @brief	Button OAM 作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BtnOAM_Create( CODEIN_WORK* wk )
{	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;
	PALETTE_FADE_PTR pfd;

	cellUnit = wk->sys.cellUnit;
	pfd = wk->sys.pfd;
	
	clwkInit.pos_x = 0;
	clwkInit.pos_y = 0;
	clwkInit.anmseq = 0;
	clwkInit.softpri = 10;
	clwkInit.bgpri = 0;
	
//	coap.pal		= 0;
	wk->btn[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
											&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
	
//	coap.pal		= 1;
	wk->btn[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
											&clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
	{
		GFL_CLACTPOS pos;
		
		pos.x = ( wk->sys.rht[ eHRT_BACK ].rect.left + wk->sys.rht[ eHRT_BACK ].rect.right ) / 2;
		pos.y = ( wk->sys.rht[ eHRT_BACK ].rect.top  + wk->sys.rht[ eHRT_BACK ].rect.bottom ) / 2;	

//		CATS_ObjectPosSetCap( wk->btn[ 0 ].cap, x, y );
		GFL_CLACT_WK_SetPos( wk->btn[ 0 ].clwk, &pos , CLSYS_DEFREND_MAIN );
//		CATS_ObjectAnimeSeqSetCap( wk->btn[ 0 ].cap, 0 );
//		CATS_ObjectUpdateCap( wk->btn[ 0 ].cap );
		GFL_CLACT_WK_SetAnmSeq( wk->btn[ 0 ].clwk, 0 );

		pos.x = ( wk->sys.rht[ eHRT_END ].rect.left + wk->sys.rht[ eHRT_END ].rect.right ) / 2;
		pos.y = ( wk->sys.rht[ eHRT_END ].rect.top  + wk->sys.rht[ eHRT_END ].rect.bottom ) / 2;	
		
//		CATS_ObjectPosSetCap( wk->btn[ 1 ].cap, x, y );
		GFL_CLACT_WK_SetPos( wk->btn[ 1 ].clwk, &pos , CLSYS_DEFREND_MAIN );
//		CATS_ObjectAnimeSeqSetCap( wk->btn[ 1 ].cap, 0 );
//		CATS_ObjectUpdateCap( wk->btn[ 1 ].cap );
		GFL_CLACT_WK_SetAnmSeq( wk->btn[ 1 ].clwk, 0 );
	}
}


//--------------------------------------------------------------
/**
 * @brief	カーソルOAMのONOFF
 *
 * @param	wk	
 * @param	no	
 * @param	flag	TRUE = ON
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Visible( CODEIN_WORK* wk, int no, BOOL flag )
{
	GFL_CLACT_WK_SetDrawEnable( wk->cur[ no ].clwk , flag );
//	if ( flag == TRUE ){
//		CATS_ObjectEnableCap( wk->cur[ no ].cap, CATS_ENABLE_TRUE );
//	}
//	else {
//		CATS_ObjectEnableCap( wk->cur[ no ].cap, CATS_ENABLE_FALSE );		
//	}
}

//--------------------------------------------------------------
/**
 * @brief	カーソルのONOFF取得
 *
 * @param	wk	
 * @param	no	
 *
 * @retval	BOOL	
 *
 */
//--------------------------------------------------------------
BOOL CI_pv_disp_CurOAM_VisibleGet( CODEIN_WORK* wk, int no )
{
	return 	GFL_CLACT_WK_GetDrawEnable( wk->cur[ no ].clwk );
//	int enable;
//	enable = CATS_ObjectEnableGetCap( wk->cur[ no ].cap );
//	return enable == CATS_ENABLE_TRUE ? TRUE : FALSE;
}


//--------------------------------------------------------------
/**
 * @brief	アニメ切り替え
 *
 * @param	wk	
 * @param	no	
 * @param	anime	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_AnimeChange( CODEIN_WORK* wk, int no, int anime )
{
	GFL_CLACT_WK_SetAnmSeq( wk->cur[ no ].clwk, anime );
//	CATS_ObjectAnimeSeqSetCap( wk->cur[ no ].cap, anime );
}


//--------------------------------------------------------------
/**
 * @brief	カーソルの位置を入力位置に
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurBar_PosSet( CODEIN_WORK* wk, int id )
{
	GFL_CLACTPOS pos;
	GFL_CLWK *clwk;
	
	clwk = wk->code[ id ].clwk;
	
	wk->cur[ 0 ].state = id;
	
	GFL_CLACT_WK_GetPos( clwk, &pos , CLSYS_DEFREND_MAIN );
	pos.y += CUR_BAR_OY;
	GFL_CLACT_WK_SetPos( wk->cur[ 0 ].clwk, &pos , CLSYS_DEFREND_MAIN );
}

//--------------------------------------------------------------
/**
 * @brief	カーソル位置をパネルに
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSet( CODEIN_WORK* wk, int id )
{
	GFL_CLACTPOS pos;
	
	pos.x = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + wk->sys.rht[ id + eHRT_NUM_0 ].rect.right ) / 2;
	pos.y = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.top  + wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom ) / 2;
	
	GFL_CLACT_WK_SetPos( wk->cur[ 1 ].clwk, &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ 1 ].clwk, 0 );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSetEx( CODEIN_WORK* wk, int id, int cur_id )
{
	GFL_CLACTPOS pos;
	
	pos.x = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + wk->sys.rht[ id + eHRT_NUM_0 ].rect.right ) / 2;
	pos.y = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.top  + wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom ) / 2;
	
	GFL_CLACT_WK_SetPos( wk->cur[ cur_id ].clwk, &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ cur_id ].clwk, 0 );

}

//--------------------------------------------------------------
/**
 * @brief	カーソル更新
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurUpdate( CODEIN_WORK* wk )
{
	
	int id = 2;
	int anime;
	BOOL bAnime;
	
	for ( id = 1; id < 3; id++ ){
		
		//anime = CATS_ObjectAnimeSeqGetCap( wk->cur[ id ].cap );
		anime = GFL_CLACT_WK_GetAnmSeq( wk->cur[ id ].clwk );
		
		if ( anime == eANM_CUR_TOUCH ){
			//bAnime = CATS_ObjectAnimeActiveCheckCap( wk->cur[ id ].cap );
			bAnime = GFL_CLACT_WK_CheckAnmActive( wk->cur[ id ].clwk );
			
			if ( bAnime == FALSE ){
				
//				CATS_ObjectAnimeSeqSetCap( wk->cur[ id ].cap, wk->cur[ id ].state );
				GFL_CLACT_WK_SetAnmSeq( wk->cur[ id ].clwk, wk->cur[ id ].state );
		
				if ( wk->sys.touch == TRUE ){
						CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
				}
				else {
						CI_pv_disp_CurOAM_Visible( wk, 1, TRUE );
				}
				CI_pv_disp_CurOAM_Visible( wk, 2, FALSE );
			}
		}
		else {
			
			if ( anime != wk->cur[ id ].state ){
				//CATS_ObjectAnimeSeqSetCap( wk->cur[ id ].cap, wk->cur[ id ].state );
				GFL_CLACT_WK_SetAnmSeq( wk->cur[ id ].clwk, wk->cur[ id ].state );
			}
			{
				//anime = CATS_ObjectAnimeSeqGetCap( wk->cur[ 2 ].cap );
				anime = GFL_CLACT_WK_GetAnmSeq( wk->cur[ id ].clwk );
				if ( anime != eANM_CUR_TOUCH ){
					if ( wk->sys.touch == TRUE ){
						CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
					}
					else {
						CI_pv_disp_CurOAM_Visible( wk, 1, TRUE );
					}	
				}
			}
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	FONTOAM pos set
 *
 * @param	obj	
 * @param	ox	
 * @param	oy	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void FontOamPosSet(BMPOAM_ACT_PTR obj, int ox, int oy)
{
	if (obj != NULL)
	{
		BmpOam_ActorSetPos( obj , ox,oy );
	}
#if USE_FontOam
	int x;
	int y;
	if (obj != NULL){
//		FONTOAM_GetMat(obj, &x, &y);
		FONTOAM_SetMat(obj,  ox,  oy);
	}	
#endif
}

#define BTN_FONT_OAM_OX		( 24 )
#define BTN_FONT_OAM_OY		( 7 )
//--------------------------------------------------------------
/**
 * @brief	ボタンのアニメ
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BtnUpdate( CODEIN_WORK* wk )
{
	int i;
	s16 ox, oy;
	
	for ( i = 0; i < 2; i++ ){
	
		ox = ( wk->sys.rht[ i + eHRT_BACK ].rect.left + wk->sys.rht[ i + eHRT_BACK ].rect.right ) / 2;
		ox -= BTN_FONT_OAM_OX;
		oy = ( wk->sys.rht[ i + eHRT_BACK ].rect.top  + wk->sys.rht[ i + eHRT_BACK ].rect.bottom ) / 2;
		oy -= BTN_FONT_OAM_OY;
		
		switch ( wk->btn[ i ].state ){
		case 0:
			wk->btn[ i ].move_wk.wait = 0;
			break;
			
		case 1:
			wk->btn[ i ].move_wk.wait++;
			if ( wk->btn[ i ].move_wk.wait == 1 ){
				//CATS_ObjectAnimeSeqSetCap( wk->btn[ i ].cap, 1 );
				GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 1 );
				FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy-0);
			}
			else if ( wk->btn[ i ].move_wk.wait == 2 ){
				//CATS_ObjectAnimeSeqSetCap( wk->btn[ i ].cap, 2 );
				GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 2 );
				FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy-0);
			}
			else if ( wk->btn[ i ].move_wk.wait == 10 ){
				//CATS_ObjectAnimeSeqSetCap( wk->btn[ i ].cap, 0 );
				GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 0 );
				FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy+0 );				
				wk->btn[ i ].state++;
			}		
			break;
		
		default:
			//CATS_ObjectAnimeSeqSetCap( wk->btn[ i ].cap, 0 );
			GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 0 );
			FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy );
			wk->btn[ i ].state = 0;
			break;
		}
	}
}


//--------------------------------------------------------------
/**
 * @brief	アニメコードを取得
 *
 * @param	state	
 * @param	size	TRUE = Large
 *
 * @retval	int	
 *
 */
//--------------------------------------------------------------
int CI_pv_disp_CodeAnimeGet( int state, BOOL size )
{
	int anime = eANM_CODE_LN;
	
	if ( size == FALSE ){
		
		 anime = eANM_CODE_SN;
	}
	
	anime += state;
	
	return anime;
}

//--------------------------------------------------------------
/**
 * @brief	サイズフラグの設定
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_SizeFlagSet( CODEIN_WORK* wk )
{
	int i;
	
	for ( i = 0; i < wk->code_max; i++ ){
		
		if ( i >= wk->ls && i < wk->le ){
			
			wk->code[ i ].size = TRUE;
		}
		else {
			
			wk->code[ i ].size = FALSE;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	移動先の位置を設定する
 *
 * @param	wk	
 * @param	mode	0 = set 1 = move_wk set
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_MovePosSet( CODEIN_WORK* wk, int mode )
{
	int i;
	int bp;
	s16 base;
//	s16 x, y;
	
	//OS_Printf( "focus %d\n", wk->focus_now );
	base = wk->x_tbl[ wk->focus_now ];
	bp	 = 0;
	
	for ( i = 0; i < wk->code_max; i++ ){
		GFL_CLACTPOS pos;

		if ( i >= wk->ls && i < wk->le ){
			
			if ( i == wk->ls ){
				base += M_SIZE;
			}
			else {
				base += L_SIZE;
			}
		}
		else {
			if ( i == 0 ){
				base += M_SIZE;
			}
			else {
				base += S_SIZE;
			}			
		}
		
		//CATS_ObjectPosGetCap( wk->code[ i ].cap, &x, &y );
		GFL_CLACT_WK_GetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_MAIN );
		if ( mode == eMPS_SET ){
			pos.x = base;
			//CATS_ObjectPosSetCap( wk->code[ i ].cap, base, y );										
			GFL_CLACT_WK_SetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_MAIN );
		}
		else {
			wk->code[ i ].move_wk.pos.x	= ( base - pos.x ) / MOVE_WAIT;
			wk->code[ i ].move_wk.pos.y	= 0;
			wk->code[ i ].move_wk.wait	= MOVE_WAIT;
			wk->code[ i ].move_wk.scale	= 0;
		}
		
		if ( i == wk->bar[ bp ].state && bp != BAR_OAM_MAX ){
			GFL_CLACTPOS pos;
			
			//CATS_ObjectPosGetCap( wk->bar[ bp ].cap, &x, &y );
			GFL_CLACT_WK_GetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_MAIN );
			
			if ( wk->ls == wk->le ){
				base += S_SIZE;
			}
			else {
				if ( i > wk->ls && i < wk->le ){
					
					base += M_SIZE;
				}
				else {
					
					base += S_SIZE;
				}
			}
			
			if ( mode == eMPS_SET ){
				pos.x = base;
				//CATS_ObjectPosSetCap( wk->bar[ bp ].cap, base, y );
				GFL_CLACT_WK_SetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_MAIN );
			}
			else {
				
				wk->bar[ bp ].move_wk.pos.x	= ( base - pos.x ) / MOVE_WAIT;
				wk->bar[ bp ].move_wk.pos.y	= 0;
				wk->bar[ bp ].move_wk.wait	= MOVE_WAIT;
			}
			bp++;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	当たり判定の作成
 *
 * @param	wk	
 * @param	no	
 * @param	sx	
 * @param	sy	
 *
 * @retval	static inline void	
 *
 */
//--------------------------------------------------------------
static inline void CODE_HitRectSet( CODEIN_WORK* wk, int no, s16 sx, s16 sy )
{
	GFL_CLACTPOS pos;
	CODE_OAM* code;	
	
	code = &wk->code[ no ];
	
	//CATS_ObjectPosGetCap( code->cap, &x, &y );
	GFL_CLACT_WK_GetPos( code->clwk, &pos , CLSYS_DEFREND_MAIN );
	
	code->hit->rect.top		= pos.y - sy;
	code->hit->rect.left	= pos.x - sx;
	code->hit->rect.bottom	= pos.y + sy;
	code->hit->rect.right	= pos.x + sx;
}

//--------------------------------------------------------------
/**
 * @brief	当たり判定の設定
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_HitTableSet( CODEIN_WORK* wk )
{
	int i;
	s16 sx;
	s16 sy;
	
	for ( i = 0; i < wk->code_max; i++ ){
				
		if ( i >= wk->ls && i < wk->le ){
			
			sx = L_SIZE / 2;
			sy = L_SIZE / 2;
		}
		else {
			
			sx = S_SIZE / 2;
			sy = S_SIZE;
		}
		
		CODE_HitRectSet( wk, i, sx, sy );
	}
}


#include "msg/msg_code_input.h"

#define	PRINT_COL_BLACK		(GF_PRINTCOLOR_MAKE( 15, 13, 2))		// フォントカラー：白
#define	PRINT_COL_BUTTON	(PRINTSYS_LSB_Make( 10,  9, 0))		// フォントカラー：黒
#define PRINT_NUM_CLEAR		(0x00)
#define PRINT_NUM_CLEAR_2	(0xEE)

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysInit(CODEIN_WORK* wk)
{
	wk->sys.bmpoam_sys = BmpOam_Init( HEAPID_CODEIN , wk->sys.cellUnit );
	
//	FontProc_LoadFont( FONT_BUTTON, HEAPID_CODEIN );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysDelete(CODEIN_WORK* wk)
{
	GFL_CLGRP_PLTT_Release( wk->sys.bmpOamPltIdx );

	BmpOam_ActorDel( wk->sys.bmp_obj[0] );
	BmpOam_ActorDel( wk->sys.bmp_obj[1] );
	
	BmpOam_Exit( wk->sys.bmpoam_sys );
#if USE_FontOam
	FontProc_UnloadFont( FONT_BUTTON );
	
	FONTOAM_Delete(wk->sys.font_obj[ 0 ]);
	CharVramAreaFree(&wk->sys.font_vram[ 0 ]);
	
	FONTOAM_Delete(wk->sys.font_obj[ 1 ]);
	CharVramAreaFree(&wk->sys.font_vram[ 1 ]);
	
	FONTOAM_SysDelete(wk->sys.fontoam_sys);			
#endif
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_ResourceLoad(CODEIN_WORK* wk)
{
	ARCHANDLE *hdl = GFL_ARC_OpenDataHandle( ARCID_FONT , HEAPID_CODEIN );
	wk->sys.bmpOamPltIdx = GFL_CLGRP_PLTT_RegisterComp( hdl , NARC_font_default_nclr , CLSYS_DRAW_MAIN , 0x20*4,  HEAPID_CODEIN );
	GFL_ARC_CloseDataHandle( hdl );
#if USE_FontOam
	CATS_LoadResourcePlttWorkArc(wk->sys.pfd, FADE_MAIN_OBJ,
								 wk->sys.csp, wk->sys.crp,
								 ARC_FONT, NARC_font_talk_ncrl, FALSE,
								 1, NNS_G2D_VRAM_TYPE_2DMAIN, eID_FNT_OAM );	
#endif
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Add( CODEIN_WORK* wk )
{
	CI_pv_FontOam_ResourceLoad(wk);
	CI_pv_FontOam_Create(wk, 0,  78, 165, 0);
	CI_pv_FontOam_Create(wk, 1, 172, 165, 0);
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	flag	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Enable(CODEIN_WORK* wk, BOOL flag)
{
#if USE_FontOam
	FONTOAM_SetDrawFlag(wk->sys.font_obj[ 0 ], flag);
	FONTOAM_SetDrawFlag(wk->sys.font_obj[ 1 ], flag);
#endif
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	no	
 * @param	x	
 * @param	y	
 * @param	pal_offset	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Create(CODEIN_WORK* wk, int no, int x, int y, int pal_offset)
{
	s16 ox, oy;
	BMPOAM_ACT_DATA	finit;
	STRBUF*			str;
	int				pal_id;
	int				vram_size;
	GFL_MSGDATA*	man;
	GFL_BMP_DATA 	*bmp;
	
	
	
	man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_code_input_dat, HEAPID_CODEIN );
	str	= GFL_MSG_CreateString( man, msg_03 + no );
		

	///< BMP
	{
		///< FONT_BUTTON は 2dotでかい
//		GF_BGL_BmpWinInit(&bmpwin);
		bmp = GFL_BMP_Create(10, 2, GFL_BMP_16_COLOR, HEAPID_CODEIN);
		PRINTSYS_PrintColor( bmp , 0,0,str,wk->sys.fontHandle, PRINTSYS_LSB_Make( 15, 14, 0) );
	}

	pal_id = eID_FNT_OAM;

//	vram_size = FONTOAM_NeedCharSize(&bmpwin, NNS_G2D_VRAM_TYPE_2DMAIN, HEAPID_CODEIN);
//	CharVramAreaAlloc(vram_size, CHARM_CONT_AREACONT, NNS_G2D_VRAM_TYPE_2DMAIN, &wk->sys.font_vram[ no ]);
	
	ox = ( wk->sys.rht[ no + eHRT_BACK ].rect.left + wk->sys.rht[ no + eHRT_BACK ].rect.right ) / 2;
	ox -= BTN_FONT_OAM_OX;
	oy = ( wk->sys.rht[ no + eHRT_BACK ].rect.top  + wk->sys.rht[ no + eHRT_BACK ].rect.bottom ) / 2;
	oy -= BTN_FONT_OAM_OY;

/*
	finit.fontoam_sys	= wk->sys.fontoam_sys;
	finit.bmp			= &bmpwin;
	finit.clact_set		= CATS_GetClactSetPtr(wk->sys.crp);
	finit.pltt			= CATS_PlttProxy(wk->sys.crp, pal_id);
	finit.parent		= NULL;
	finit.char_ofs		= wk->sys.font_vram[ no ].alloc_ofs;
	finit.x				= ox;
	finit.y				= oy;
	finit.bg_pri		= 0;
	finit.soft_pri		= 0;
	finit.draw_area		= NNS_G2D_VRAM_TYPE_2DMAIN;
	finit.heap			= HEAPID_CODEIN;
*/
	finit.bmp = bmp;
	finit.x = ox;
	finit.y = oy;
//	finit.pltt_index = wk->sys.bmpOamPltIdx;
  finit.pltt_index = wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON];
	finit.pal_offset = 0;
	finit.soft_pri = 0;
	finit.bg_pri = 0;
	finit.setSerface = CLSYS_DEFREND_MAIN;
	finit.draw_type = CLSYS_DRAW_MAIN;
	
	OS_Printf("bmpOamPltIdx=%d\n",wk->sys.bmpOamPltIdx );
	OS_Printf("resIdx RES2_BUTTOn=%d\n", wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON]);
	
	wk->sys.bmp_obj[ no ] = BmpOam_ActorAdd( wk->sys.bmpoam_sys , &finit );
	BmpOam_ActorBmpTrans( wk->sys.bmp_obj[ no ] );
//	FONTOAM_SetPaletteOffsetAddTransPlttNo(wk->sys.font_obj[ no ], pal_offset);
//	FONTOAM_SetMat(wk->sys.font_obj[ no ], x, y + );

//	GF_BGL_BmpWinDel(&bmpwin);
	GFL_BMP_Delete( bmp );
	GFL_STR_DeleteBuffer(str);
	GFL_MSG_Delete(man);
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	bgl	
 * @param	win	
 * @param	frm	
 * @param	x	
 * @param	y	
 * @param	sx	
 * @param	sy	
 * @param	ofs	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BMP_WindowAdd( GFL_BMPWIN** win, int frm, int x, int y, int sx, int sy, GFL_FONT *fontHandle )
{
//	GF_BGL_BmpWinInit( win );
//	GF_BGL_BmpWinAdd( bgl, win, frm, x, y, sx, sy, ePAL_FONT, ofs );
	*win = GFL_BMPWIN_Create( frm , x, y, sx, sy, ePAL_FONT, GFL_BMP_CHRAREA_GET_B );
					 
//	BmpTalkWinWrite( win, WINDOW_TRANS_OFF, 1, ePAL_FRAME );
	TalkWinFrame_Write( *win , WINDOW_TRANS_OFF, 1, ePAL_FRAME );
	
	GFL_BMPWIN_MakeScreen( *win );

//	GF_BGL_BmpWinDataFill( win, FBMP_COL_WHITE );
//	GF_BGL_BmpWinOn( win );
	
	CI_pv_BMP_MsgSet( *win, msg_01 , fontHandle );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	win	
 * @param	mes_id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_BMP_MsgSet( GFL_BMPWIN * win, int mes_id , GFL_FONT *fontHandle )
{
	GFL_MSGDATA* man;
	STRBUF* str;
	

	man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_code_input_dat, HEAPID_CODEIN );
	str	= GFL_MSG_CreateString( man, mes_id );
	
//	GF_BGL_BmpWinDataFill( win, FBMP_COL_WHITE );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ) , 15 );
//	GF_STR_PrintSimple( win, FONT_SYSTEM, str, 0, 0, 0, NULL );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( win ) , 0,0,str, fontHandle );
//	GF_BGL_BmpWinOn( win );

	GFL_STR_DeleteBuffer( str );
	GFL_MSG_Delete( man );	
	
	GFL_BMPWIN_TransVramCharacter( win );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}
