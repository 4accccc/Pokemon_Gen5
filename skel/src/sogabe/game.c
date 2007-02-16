
//============================================================================================
/**
 * @file	game.c
 * @brief	DS版ヨッシーのたまご　ゲームプログラム
 * @author	sogabe
 * @date	2007.02.08
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"
#include "player.h"
#include "fallchr.h"

#include "sample_graphic/yossyegg.naix"

#define __GAME_H_GLOBAL__
#include "game.h"

static	void	YT_MainGameAct(GAME_PARAM *gp);
static	void	YT_ClactResourceLoad( YT_CLACT_RES *clact_res, u32 heapID );
static	int		YT_ReadyCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps);
static	void	YT_ReadyAct(GAME_PARAM *gp,int player_no);
static	BOOL	YT_FallCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps);
static	void	YT_CheckFlag(TCB *tcb,void *work);

//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム初期化
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_InitGame(GAME_PARAM *gp)
{
	// セルアクターユニット作成
	gp->clact->p_unit = GFL_CLACT_UnitCreate( YT_CLACT_MAX, gp->heapID );

	//エリアマネージャ初期化
	gp->clact_area=GFL_AREAMAN_Create(YT_CLACT_MAX,gp->heapID);
	
	//BGシステム初期化
	GFL_BG_sysInit(gp->heapID);

	//VRAM設定
	{
		GFL_BG_DISPVRAM vramSetTable = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0_F				// テクスチャパレットスロット
		};
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}

	// BG SYSTEM
	{
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_InitBG( &BGsys_data );
	}

	//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME2_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_BGControlSet(GFL_BG_FRAME2_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ScrClear(GFL_BG_FRAME2_M );
		GFL_BG_BGControlSet(GFL_BG_FRAME3_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ScrClear(GFL_BG_FRAME3_M );
		GFL_BG_BGControlSet(GFL_BG_FRAME2_S, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ScrClear(GFL_BG_FRAME2_S );
		GFL_BG_BGControlSet(GFL_BG_FRAME3_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ScrClear(GFL_BG_FRAME3_S );

		GFL_DISP_GX_VisibleControl(GX_PLANEMASK_BG0, VISIBLE_ON );

		// OBJマッピングモード
		GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
		GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );

		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	}

	//画面生成
	GFL_ARC_UtilBgCharSet(0,NARC_yossyegg_game_bg_NCGR,GFL_BG_FRAME2_M,0,0,0,gp->heapID);
	GFL_ARC_UtilScrnSet(0,NARC_yossyegg_game_bg_NSCR,GFL_BG_FRAME2_M,0,0,0,gp->heapID);
	GFL_ARC_UtilBgCharSet(0,NARC_yossyegg_YT_BG03_NCGR,GFL_BG_FRAME3_M,0,0,0,gp->heapID);
	GFL_ARC_UtilScrnSet(0,NARC_yossyegg_YT_BG03_NSCR,GFL_BG_FRAME3_M,0,0,0,gp->heapID);
	GFL_ARC_UtilPalSet(0,NARC_yossyegg_YT_BG03_NCLR,PALTYPE_MAIN_BG,0,0x100,gp->heapID);

	GFL_DISP_DispOn();
	GFL_DISP_DispSelect( GFL_DISP_3D_TO_SUB );

	GFL_FADE_MasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN|GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,16,0,2);

	//セルアクターリソース読み込み
	YT_ClactResourceLoad(&gp->clact->res, gp->heapID);

	//フラグ関連初期
	{
		int	x,y;

		for(x=0;x<YT_LINE_MAX;x++){
			gp->ps[0].falltbl[x]=x;
			gp->ps[1].falltbl[x]=x;
			gp->ps[0].stoptbl[x]=x;
			gp->ps[1].stoptbl[x]=x;
		}
		for(y=0;y<YT_HEIGHT_MAX;y++){
			for(x=0;x<YT_LINE_MAX;x++){
				gp->ps[0].ready[x][y]=NULL;
				gp->ps[1].ready[x][y]=NULL;
				gp->ps[0].fall[x][y]=NULL;
				gp->ps[1].fall[x][y]=NULL;
				gp->ps[0].stop[x][y]=NULL;
				gp->ps[1].stop[x][y]=NULL;
			}
		}
		gp->default_fall_wait=YT_DEFAULT_FALL_WAIT;
	}

	//乱数初期化
	{
		GFL_STD_MTRandInit(0);
	}

	//プレーヤー初期化
	YT_InitPlayer(gp,0,0);

	//ゲームフラグチェックタスクセット
	gp->check_tcb=GFL_TCB_AddTask(gp->tcbsys,YT_CheckFlag,gp,TCB_PRI_PLAYER);

	YT_JobNoSet(gp,YT_MainGameNo);

}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームメイン処理
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_MainGame(GAME_PARAM *gp)
{
	//ゲームシーケンス処理
	YT_MainGameAct(gp);

	GFL_TCB_SysMain(gp->tcbsys);

	// セルアクターユニット描画処理
	GFL_CLACT_UnitDraw( gp->clact->p_unit );

	// セルアクターシステムメイン処理
	// 全ユニット描画が完了してから行う必要があります。
	GFL_CLACT_SysMain();
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームシーケンス処理
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
enum{
	SEQ_GAME_START_WAIT=0,
	SEQ_GAME_READY_CHECK,
	SEQ_GAME_FALL_CHECK,
};

enum{
	YT_READY_WAIT=0,
	YT_READY_MAKE,
	YT_READY_ALREADY,
};

static	void	YT_MainGameAct(GAME_PARAM *gp)
{
	switch(gp->seq_no){
	case SEQ_GAME_START_WAIT:
		if(GFL_FADE_FadeCheck()==FALSE){
			gp->seq_no++;
		}
		break;
	case SEQ_GAME_READY_CHECK:
		switch(YT_ReadyCheck(gp,&gp->ps[0])){
		case YT_READY_MAKE:
			YT_ReadyAct(gp,0);
		case YT_READY_ALREADY:
			gp->ps[0].fall_wait=YT_FALL_WAIT;
			gp->seq_no++;
			break;
		default:
			break;
		}
		break;
	case SEQ_GAME_FALL_CHECK:
		if(YT_FallCheck(gp,&gp->ps[0])==TRUE){
			if(--gp->ps[0].fall_wait==0){
				gp->seq_no=SEQ_GAME_READY_CHECK;
			}
		}
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	待機キャラを生成してもよいかチェック
 *	
 *	@param	gp		ゲームパラメータポインタ
 *	@param	ps		プレイヤーステータス
 */
//-----------------------------------------------------------------------------
static	int	YT_ReadyCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps)
{
	int	x,y;

	for(x=0;x<4;x++){
		//すでにREADY状態なら
		if(ps->ready[x][0]){
			return YT_READY_ALREADY;
		}
	}

	for(x=0;x<4;x++){
		for(y=0;y<8;y++){
			if(ps->fall[x][y]){
				{
					CLSYS_POS	pos;
					GFL_CLACT_WkGetWldPos(gp->clact->clact_work[ps->fall[x][y]->clact_no],&pos);
					if(pos.y<YT_READY_NEXT_Y_POS){
						return YT_READY_WAIT;
					}
				}
				break;
			}
		}
	}
	return YT_READY_MAKE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	待機キャラ生成処理
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
static	void	YT_ReadyAct(GAME_PARAM *gp,int player_no)
{
	int	i=2;
	int	line,type;

	while(i){
		line=__GFL_STD_MTRand()%4;
		if(gp->ps[player_no].ready[line][0]){
			continue;
		}
		type=__GFL_STD_MTRand()%4;
		//デカキャラは、確率を低くする
		if(type==YT_CHR_TERESA){
			if(__GFL_STD_MTRand()%5==0){
				type=YT_CHR_DEKATERESA;
			}
		}
		else{
			//タマゴの殻発生確率
			if(__GFL_STD_MTRand()%5==0){
				type=YT_CHR_GREEN_EGG_U+__GFL_STD_MTRand()%4;
			}
		}
		gp->ps[player_no].ready[line][0]=YT_InitFallChr(gp,player_no,type,line);
		i--;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	待機キャラを落下させてもよいかチェック
 *	
 *	@param	gp		ゲームパラメータポインタ
 *	@param	ps		プレイヤーステータス
 */
//-----------------------------------------------------------------------------
static	BOOL	YT_FallCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps)
{
	int	x,y;

	for(x=0;x<YT_LINE_MAX;x++){
		for(y=0;y<YT_HEIGHT_MAX;y++){
			if(ps->fall[x][y]){
				return FALSE;
			}
		}
	}
	for(x=0;x<YT_LINE_MAX;x++){
		ps->falltbl[x]=x;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターリソース読み込み
 *
 *	@param	clact_res	ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void YT_ClactResourceLoad( YT_CLACT_RES *clact_res, u32 heapID )
{
	BOOL result;
	void* p_buff;
	NNSG2dCharacterData* p_char;
	NNSG2dPaletteData* p_pltt;
	
	// キャラクタデータ読み込み＆転送
	{
		p_buff = GFL_ARC_DataLoadMalloc( 0,NARC_yossyegg_O_WOODS3_NCGR, heapID );
		result = NNS_G2dGetUnpackedCharacterData( p_buff, &p_char );
		GF_ASSERT( result );
		NNS_G2dInitImageProxy( &clact_res->imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&clact_res->imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&clact_res->imageproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// パレットデータ読み込み＆転送
	{
		p_buff = GFL_ARC_DataLoadMalloc( 0,NARC_yossyegg_OBJ_COL_NCLR, heapID );
		result = NNS_G2dGetUnpackedPaletteData( p_buff, &p_pltt );
		GF_ASSERT( result );
		NNS_G2dInitImagePaletteProxy( &clact_res->plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&clact_res->plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&clact_res->plttproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// セルデータ読み込み
	{
		clact_res->p_cellbuff = GFL_ARC_DataLoadMalloc( 0,NARC_yossyegg_fall_obj_NCER, heapID );
		result = NNS_G2dGetUnpackedCellBank( clact_res->p_cellbuff, &clact_res->p_cell );
		GF_ASSERT( result );
	}

	// セルアニメデータ読み込み
	{
		clact_res->p_cellanmbuff = GFL_ARC_DataLoadMalloc( 0,NARC_yossyegg_fall_obj_NANR, heapID );
		result = NNS_G2dGetUnpackedAnimBank( clact_res->p_cellanmbuff, &clact_res->p_cellanm );
		GF_ASSERT( result );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームフラグチェックタスク
 */
//-----------------------------------------------------------------------------
static	void	YT_CheckFlag(TCB *tcb,void *work)
{
	GAME_PARAM			*gp=(GAME_PARAM *)work;
	YT_PLAYER_STATUS	*ps;
	int					player_no;

	for(player_no=0;player_no<2;player_no++){
		ps=&gp->ps[player_no];
		//回転処理終了チェック
		if(ps->rotate_flag){
			{
				int					i;
				int					left_line_stop;
				int					right_line_stop;
				int					left_line_fall;
				int					right_line_fall;
				FALL_CHR_PARAM		*fcp;

				left_line_stop=ps->stoptbl[ps->rotate_flag-1];
				right_line_stop=ps->stoptbl[ps->rotate_flag];
				left_line_fall=ps->falltbl[ps->rotate_flag-1];
				right_line_fall=ps->falltbl[ps->rotate_flag];

				for(i=0;i<YT_HEIGHT_MAX;i++){
					fcp=ps->stop[left_line_stop][i];
					if(fcp){
						if(fcp->rotate_flag){
							break;
						}
					}
					fcp=ps->stop[right_line_stop][i];
					if(fcp){
						if(fcp->rotate_flag){
							break;
						}
					}
					fcp=ps->fall[left_line_fall][i];
					if(fcp){
						if(fcp->rotate_flag){
							break;
						}
					}
					fcp=ps->fall[right_line_fall][i];
					if(fcp){
						if(fcp->rotate_flag){
							break;
						}
					}
				}
				if(i==YT_HEIGHT_MAX){
					ps->rotate_flag=0;
				}
			}
		}
		//ひっくり返し処理終了チェック
		if(ps->overturn_flag){
			{
				int					x,y,line;
				FALL_CHR_PARAM		*fcp;

				for(x=0;x<YT_LINE_MAX;x++){
					line=ps->stoptbl[x];
					if(ps->overturn_flag&(1<<x)){
						for(y=0;y<YT_HEIGHT_MAX;y++){
							fcp=ps->stop[line][y];
							if(fcp){
								if(fcp->overturn_flag){
									break;
								}
							}
						}
					}
				}
				if((x==YT_LINE_MAX)&&(y==YT_HEIGHT_MAX)){
					ps->overturn_flag=0;
				}
			}
		}
		//タマゴ生成チェック
		if(ps->egg_make_check_flag){
			YT_EggMakeCheck(ps);
		}
		//タマゴ生成中チェック
		if(ps->egg_make_flag){
			{
				YT_PLAYER_STATUS	*ps=(YT_PLAYER_STATUS *)work;
				int					x,y;
				FALL_CHR_PARAM		*fcp;

				for(x=0;x<YT_LINE_MAX;x++){
					for(y=0;y<YT_HEIGHT_MAX;y++){
						fcp=ps->stop[x][y];
						if(fcp){
							if(fcp->egg_make_flag){
								return;
							}
						}
					}
				}
				ps->egg_make_flag=0;
			}
		}
	}
}

