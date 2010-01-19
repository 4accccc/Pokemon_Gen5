//============================================================================================
/**
 * @file		cdemo_seq.h
 * @brief		コマンドデモ画面 シーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		09.05.07
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "system/wipe.h"

#include "cdemo_main.h"
#include "cdemo_seq.h"
#include "cdemo_comm.h"
#include "op_demo.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	CDEMO_SKIP_KEY		(PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_START)		// スキップボタン



//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( CDEMO_WORK * wk );
static int MainSeq_Release( CDEMO_WORK * wk );
static int MainSeq_Fade( CDEMO_WORK * wk );
static int MainSeq_AlphaBlend( CDEMO_WORK * wk );
static int MainSeq_BgScrnAnm( CDEMO_WORK * wk );
static int MainSeq_Main( CDEMO_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pCommDemoFunc MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Fade,
	MainSeq_AlphaBlend,
	MainSeq_BgScrnAnm,

	MainSeq_Main,
};




//--------------------------------------------------------------------------------------------
/**
 * @brief		メイン処理
 *
 * @param		wk		デモワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//-------------------------------------------------------------------------------------------
BOOL CDEMOSEQ_Main( CDEMO_WORK * wk, int * seq )
{
	if( wk->dat->skip != COMMANDDEMO_SKIP_OFF ){
		if( GFL_UI_KEY_GetTrg() & CDEMO_SKIP_KEY ){
			wk->main_seq = CDEMOSEQ_MAIN_RELEASE;
			wk->dat->ret = COMMANDDEMO_RET_SKIP;
		}
	}
#ifdef PM_DEBUG
	if( wk->dat->skip == COMMANDDEMO_SKIP_DEBUG ){
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			wk->main_seq = CDEMOSEQ_MAIN_RELEASE;
			wk->dat->ret = COMMANDDEMO_RET_SKIP_DEBUG;
		}
	}
#endif

	wk->main_seq = MainSeq[wk->main_seq]( wk );

	if( wk->main_seq == CDEMOSEQ_MAIN_END ){
		return FALSE;
	}

/*
	LDA_WORK * awk;

	wk->main_seq = MainSeq[wk->main_seq]( wk );

	if( wk->main_seq == MAINSEQ_END ){
		return 0;
	}

	awk = wk->work;

//	ObjAnmMain( awk );
//	CATS_Draw( awk->crp );
*/

	return TRUE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk		デモワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( CDEMO_WORK * wk )
{
/*
	LDA_WORK * awk;

	// 割り込み停止
	sys_VBlankFuncChange( NULL, NULL );
	sys_HBlankIntrStop();
	// 表示初期化
	GF_Disp_GX_VisibleControlDirect( 0 );
	GF_Disp_GXS_VisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 上画面をメインに
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	wk->work = sys_AllocMemory( HEAPID_LEGEND_DEMO, sizeof(LDA_WORK) );
	MI_CpuClear8( wk->work, sizeof(LDA_WORK) );

	awk = wk->work;

	awk->comm = CommandData;
	awk->ah   = ArchiveDataHandleOpen( ARC_LD_ARCEUS, HEAPID_LEGEND_DEMO );

#ifdef	PM_DEBUG
	awk->debug_count = 0;
	awk->debug_count_frm = 0;
#endif	// PM_DEBUG

	VramBankSet();
	BgInit( awk );
//	ObjInit( wk );

	sys_VBlankFuncChange( VBlankFunc, wk->work );
*/
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	CDEMOMAIN_VramBankSet();
	CDEMOMAIN_BgInit( wk );

	wk->gra_ah = GFL_ARC_OpenDataHandle( ARCID_CDEMO_DATA, HEAPID_COMMAND_DEMO ); 

	// データロード
	CDEMOMAIN_CommDataLoad( wk );

//	WIPE_Reset( WIPE_DISP_SUB );

	CDEMOMAIN_InitVBlank( wk );

//	return CDEMOSEQ_MAIN_MAIN;
	CDEMOMAIN_FadeInSet( wk, 1, 1, CDEMOSEQ_MAIN_MAIN );
	return CDEMOSEQ_MAIN_FADE;



/*
	FS_EXTERN_OVERLAY(ui_common);

	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 割り込み停止
//	sys_VBlankFuncChange( NULL, NULL );
//	sys_HBlankIntrStop();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	// アプリヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_APP, 0x80000 );
	syswk->app = GFL_HEAP_AllocClearMemory( HEAPID_BOX_APP, sizeof(BOX2_APP_WORK) );

//	syswk->app->ppd_ah      = GFL_ARC_OpenDataHandle( ARCID_PERSONAL, HEAPID_BOX_APP );
	syswk->app->pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_BOX_APP );

	BOX2MAIN_VramBankSet();
	BOX2MAIN_BgInit( syswk );
	BOX2MAIN_PaletteFadeInit( syswk );

	BOX2MAIN_BgGraphicLoad( syswk );

	BOX2MAIN_MsgInit( syswk );

	BOX2BMP_Init( syswk );

	BOX2OBJ_Init( syswk );
	BOX2OBJ_TrayPokeIconChange( syswk );
	BOX2OBJ_PartyPokeIconChange( syswk );

	BOX2MAIN_WallPaperSet(
		syswk, BOX2MAIN_GetWallPaperNumber( syswk, syswk->tray ), BOX2MAIN_TRAY_SCROLL_NONE );

	BOX2BGWFRM_Init( syswk );

	if( syswk->dat->callMode != BOX_MODE_TURETEIKU && syswk->dat->callMode != BOX_MODE_AZUKERU ){
		BOX2BGWFRM_ButtonPutTemochi( syswk );
		BOX2BGWFRM_ButtonPutIdou( syswk );
	}
	BOX2BGWFRM_ButtonPutModoru( syswk );

	BOX2BMP_TitlePut( syswk );
	BOX2BMP_DefStrPut( syswk );

	BOX2OBJ_TrayIconCgxMakeAll( syswk );

	BOX2UI_CursorMoveInit( syswk );

	BOX2MAIN_YesNoWinInit( syswk );

//	syswk->app->vtask = VIntrTCB_Add( BOX2MAIN_VBlank, syswk, 0 );

	BOX2MAIN_SetBlendAlpha( TRUE );

	BOX2MAIN_InitVBlank( syswk );

	return syswk->next_seq;
*/

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param	wk		デモワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( CDEMO_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == FALSE ){
		return CDEMOSEQ_MAIN_RELEASE;
	}

	CDEMOMAIN_ExitVBlank( wk );
	GFL_ARC_CloseDataHandle( wk->gra_ah );	
	CDEMOMAIN_CommDataDelete( wk );
	CDEMOMAIN_BgExit();

/*
	LDA_WORK * awk = wk->work;

	sys_VBlankFuncChange( NULL, NULL );

//	ObjExit( awk );
	BgExit( awk );

	ArchiveDataHandleClose( awk->ah );

//	Snd_BgmStop( SEQ_GS_E_ARCEUS, 0 );
//	Snd_BgmPlay( awk->bgm );

	sys_FreeMemoryEz( wk->work );
*/

	return CDEMOSEQ_MAIN_END;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：フェード
 *
 * @param	wk		伝説デモワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Fade( CDEMO_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->next_seq;
	}
	return CDEMOSEQ_MAIN_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：アルファブレンディング
 *
 * @param		wk		デモワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_AlphaBlend( CDEMO_WORK * wk )
{
/*
	LDA_WORK * awk;
	int	ev1, ev2;

	awk = wk->work;

	if( wk->cnt == awk->alpha_mv_frm ){
		ev1 = awk->alpha_end_ev;
		ev2 = abs( 15 - ev1 );
		G2_SetBlendAlpha( awk->alpha_plane1, awk->alpha_plane2, ev1, ev2 );
		wk->cnt = 0;
		return wk->next_seq;
	}

	ev1 = ( ( abs(awk->alpha_ev1-awk->alpha_end_ev) << 8 ) / awk->alpha_mv_frm * wk->cnt ) >> 8;
	if( awk->alpha_ev1 > awk->alpha_end_ev ){
		ev1 = awk->alpha_ev1 - ev1;
	}else{
		ev1 = awk->alpha_ev1 + ev1;
	}
	ev2 = abs( 15 - ev1 );

	G2_SetBlendAlpha( awk->alpha_plane1, awk->alpha_plane2, ev1, ev2 );

	wk->cnt++;
*/

	return CDEMOSEQ_MAIN_ALPHA_BLEND;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン
 *
 * @param		wk		デモワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Main( CDEMO_WORK * wk )
{
/*
	LDA_WORK * awk = wk->work;

	while(1){
		int	ret;
		
		if( awk->comm[0] == COMM_END ){
			return MAINSEQ_RELEASE;
		}
		
		ret = CommFunc[awk->comm[0]]( wk, awk->comm );

		if( ret == COMM_RET_TRUE ){
			break;
		}else if( ret == COMM_RET_FALSE ){
			awk->comm = &awk->comm[ CommandSize[awk->comm[0]] ];
//			awk->d_cnt++;
//			OS_Printf( "d_cnt++ %d\n", awk->d_cnt );
		}else if( ret == COMM_RET_NEXT ){
			awk->comm = &awk->comm[ CommandSize[awk->comm[0]] ];
//			awk->d_cnt++;
//			OS_Printf( "d_cnt++ %d\n", awk->d_cnt );
			break;
		}
	}
*/
/*
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
		return CDEMOSEQ_MAIN_RELEASE;
	}
	return CDEMOSEQ_MAIN_MAIN;
*/


	while(1){
		int	ret;
		
		if( wk->commPos[0] == CDEMOCOMM_END ){
			wk->dat->ret = COMMANDDEMO_RET_NORMAL;
			return CDEMOSEQ_MAIN_RELEASE;
		}

		ret = CDEMOCOMM_Main( wk );
		
		if( ret != CDEMOCOMM_RET_FALSE ){
			break;
		}
	}

	return wk->main_seq;
}


static int MainSeq_BgScrnAnm( CDEMO_WORK * wk )
{
/*
	switch( wk->bgsa_seq ){
	case 0:
		{
			u32	i;
			for( i=0; i<3; i++ ){
				if( wk->bgsa_max > i ){
					CDEMOMAIN_LoadBgGraphic( wk, wk->bgsa_chr, wk->bgsa_pal, wk->bgsa_scr, GFL_BG_FRAME0_M+i );
					GFL_BG_SetPriority( GFL_BG_FRAME0_M+i, i );
					wk->bgsa_chr++;
					wk->bgsa_pal++;
					wk->bgsa_scr++;
					wk->bgsa_load++;
				}else{
					break;
				}
			}
		}
		wk->bgsa_num++;
		wk->bgsa_seq = 1;
//		break;

	case 1:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 2;
			break;
		}
		wk->bgsa_cnt++;
		break;

	case 2:
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+(wk->bgsa_num%3), 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+((wk->bgsa_num+1)%3), 1 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+((wk->bgsa_num+2)%3), 2 );
		wk->bgsa_num++;
		wk->bgsa_seq = 3;
//		break;

	case 3:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 4;
			break;
		}
		wk->bgsa_cnt++;
		break;

	case 4:
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+(wk->bgsa_num%3), 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+((wk->bgsa_num+1)%3), 1 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M+((wk->bgsa_num+2)%3), 2 );
		if( wk->bgsa_load < wk->bgsa_max ){
			CDEMOMAIN_LoadBgGraphic(
				wk, wk->bgsa_chr, wk->bgsa_pal, wk->bgsa_scr, GFL_BG_FRAME0_M+((wk->bgsa_num+1)%3) );
			wk->bgsa_chr++;
			wk->bgsa_pal++;
			wk->bgsa_scr++;
			wk->bgsa_load++;
		}
		wk->bgsa_num++;
		wk->bgsa_seq = 3;
		break;
	}
*/

/*
	switch( wk->bgsa_seq ){
	case 0:
		GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );
		wk->bgsa_seq = 1;

	case 1:
		{
			void * buf;
			buf = GFL_ARCHDL_UTIL_Load( wk->gra_ah, NARC_ld_arceus_ds_2_00000_LZ_bin+wk->bgsa_num, TRUE, HEAPID_COMMAND_DEMO );
//			DC_FlushRange( buf, 256*192*2 );
			if( ( wk->bgsa_load & 1 ) == 0 ){
		    GX_LoadBG2Bmp( buf, 0, 256*192*2 );
				GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );
				GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );
			}else{
		    GX_LoadBG3Bmp( buf, 0, 256*192*2 );
				GFL_BG_SetPriority( GFL_BG_FRAME2_M, 1 );
				GFL_BG_SetPriority( GFL_BG_FRAME3_M, 0 );
			}
			GFL_HEAP_FreeMemory( buf );
		}
		wk->bgsa_load++;
		wk->bgsa_num += 2;
		wk->bgsa_seq = 2;

	case 2:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 1;
			break;
		}
		wk->bgsa_cnt++;
		break;
	}
*/

/*
	switch( wk->bgsa_seq ){
	case 0:
		GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );
		wk->bgsa_seq = 1;

	case 1:
		{
			void * buf;
			buf = GFL_ARCHDL_UTIL_Load( wk->gra_ah, NARC_ld_arceus_ds_2_00000_LZ_bin+wk->bgsa_num, TRUE, HEAPID_COMMAND_DEMO );
//			DC_FlushRange( buf, 256*192*2 );
			if( ( wk->bgsa_load & 1 ) == 0 ){
		    GX_LoadBG2Bmp( buf, 0, 256*192*2 );
//		    MI_CpuCopy32( buf, (void*)G2_GetBG2ScrPtr(), 256*192*2 );
				GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );
				GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );
			}else{
		    GX_LoadBG3Bmp( buf, 0, 256*192*2 );
//		    MI_CpuCopy32( buf, (void*)G2_GetBG3ScrPtr(), 256*192*2 );
				GFL_BG_SetPriority( GFL_BG_FRAME2_M, 1 );
				GFL_BG_SetPriority( GFL_BG_FRAME3_M, 0 );
			}
//	    MI_CpuCopyFast( buf, (void*)G2_GetBG2ScrPtr(), 256*192*2 );
			GFL_HEAP_FreeMemory( buf );
		}
		wk->bgsa_load++;
		wk->bgsa_num += 2;
		wk->bgsa_seq = 2;

	case 2:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 1;
			break;
		}
		wk->bgsa_cnt++;
		break;
	}
*/

	switch( wk->bgsa_seq ){
	case 0:
		{
			void * buf;
			buf = GFL_ARCHDL_UTIL_Load( wk->gra_ah, NARC_op_demo_op001_LZ_bin, TRUE, HEAPID_COMMAND_DEMO );
			DC_FlushRange( buf, 256*170*2 );
	    GX_LoadBG2Bmp( buf, 256*11*2, 256*170*2 );
			GFL_HEAP_FreeMemory( buf );
			wk->bgsa_num += 1;
			buf = GFL_ARCHDL_UTIL_Load( wk->gra_ah, NARC_op_demo_op001_LZ_bin+wk->bgsa_num, TRUE, HEAPID_COMMAND_DEMO );
			DC_FlushRange( buf, 256*170*2 );
	    GX_LoadBG3Bmp( buf, 256*11*2, 256*170*2 );
			GFL_HEAP_FreeMemory( buf );
			wk->bgsa_num += 1;
		}
		GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );
		wk->bgsa_load++;
		wk->bgsa_seq = 1;

	case 1:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 2;
			break;
		}
		wk->bgsa_cnt++;
		break;

	case 2:
		GFL_BG_SetPriority( GFL_BG_FRAME2_M+(wk->bgsa_load&1), 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME2_M+((wk->bgsa_load+1)&1), 1 );
		if( wk->bgsa_num < wk->bgsa_max ){
			void * buf;
			buf = GFL_ARCHDL_UTIL_Load( wk->gra_ah, NARC_op_demo_op001_LZ_bin+wk->bgsa_num, TRUE, HEAPID_COMMAND_DEMO );
			DC_FlushRange( buf, 256*170*2 );
			if( ((wk->bgsa_load+1)&1) == 0 ){
		    GX_LoadBG2Bmp( buf, 256*11*2, 256*170*2 );
			}else{
		    GX_LoadBG3Bmp( buf, 256*11*2, 256*170*2 );
			}
			GFL_HEAP_FreeMemory( buf );
			wk->bgsa_num += 1;
		}
		wk->bgsa_load++;
		wk->bgsa_seq = 3;

	case 3:
		if( wk->bgsa_cnt >= wk->bgsa_wait ){
			if( wk->bgsa_num >= wk->bgsa_max ){
				return CDEMOSEQ_MAIN_MAIN;
			}
			wk->bgsa_cnt = 0;
			wk->bgsa_seq = 2;
			break;
		}
		wk->bgsa_cnt++;
		break;
	}




	return CDEMOSEQ_MAIN_BG_SCRN_ANM;
}

