
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

#define	PLAYER1_READY	(0)

static	void	YT_MainGameAct(GAME_PARAM *gp);
static	void	YT_ExitGame(GAME_PARAM *gp);
static	void	YT_ClactResourceLoad( YT_CLACT_RES *clact_res, u32 heapID );
static	int		YT_ReadyCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps);
static	void	YT_ReadyAct(GAME_PARAM *gp,int player_no);
static	BOOL	YT_FallCheck(GAME_PARAM *gp,YT_PLAYER_STATUS *ps);
static	void	YT_CheckFlag(GFL_TCB *tcb,void *work);

//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム初期化
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_InitGame(GAME_PARAM *gp)
{
    PLAYER_PARAM* pp;

    // セルアクターユニット作成
	gp->clact->p_unit = GFL_CLACT_UnitCreate( YT_CLACT_MAX, gp->heapID );

	//エリアマネージャ初期化
	gp->clact_area=GFL_AREAMAN_Create(YT_CLACT_MAX,gp->heapID);
	
	//BGシステム初期化
	GFL_BG_Init(gp->heapID);

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
			///<FRAME1_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x4000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
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
		GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
		GFL_BG_SetBGControl(GFL_BG_FRAME2_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_S);
		GFL_BG_SetBGControl(GFL_BG_FRAME3_S, &TextBgCntDat[4], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_S);

		// OBJマッピングモード
		GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
		GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );

		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	}

	//画面生成
	GFL_ARC_UTIL_TransVramBgCharacter(0,NARC_yossyegg_game_bg_NCGR,GFL_BG_FRAME2_M,0,0,0,gp->heapID);
	GFL_ARC_UTIL_TransVramScreen(0,NARC_yossyegg_game_bg_NSCR,GFL_BG_FRAME2_M,0,0,0,gp->heapID);
	GFL_ARC_UTIL_TransVramBgCharacter(0,NARC_yossyegg_YT_BG03_NCGR,GFL_BG_FRAME3_M,0,0,0,gp->heapID);
	GFL_ARC_UTIL_TransVramScreen(0,NARC_yossyegg_YT_BG03_NSCR,GFL_BG_FRAME3_M,0,0,0,gp->heapID);
	GFL_ARC_UTIL_TransVramPalette(0,NARC_yossyegg_YT_BG03_NCLR,PALTYPE_MAIN_BG,0,0x100,gp->heapID);

	GFL_DISP_SetDispOn();
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN|GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,16,0,2);

	//セルアクターリソース読み込み
	YT_ClactResourceLoad(&gp->clact->res, gp->heapID);

	//ヨッシーキャラデータ読み込み
	gp->yossy_bmp=GFL_BMP_LoadCharacter(0,NARC_yossyegg_yossy_birth_NCGR,0,gp->heapID);

	//BMP関連初期化
	GFL_BMPWIN_Init(gp->heapID);
	gp->yossy_bmpwin=GFL_BMPWIN_Create(GFL_BG_FRAME1_M,0,0,32,32,2,GFL_BMP_CHRAREA_GET_B);
	GFL_BMPWIN_MakeScreen(gp->yossy_bmpwin);
	GFL_BG_LoadScreenReq(GFL_BG_FRAME1_M);

	//フラグ関連初期
	{
		int	x,y;

		GFL_STD_MemClear(&gp->ps[0],sizeof(YT_PLAYER_STATUS));
		GFL_STD_MemClear(&gp->ps[1],sizeof(YT_PLAYER_STATUS));

		for(x=0;x<YT_LINE_MAX;x++){
			gp->ps[0].falltbl[x]=x;
			gp->ps[1].falltbl[x]=x;
			gp->ps[0].stoptbl[x]=x;
			gp->ps[1].stoptbl[x]=x;
		}
		for(y=0;y<YT_HEIGHT_MAX;y++){
			gp->ps[0].rensa[y]=NULL;
			gp->ps[1].rensa[y]=NULL;
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
		GFL_STD_MtRandInit(0);
	}

	//プレーヤー初期化
	gp->seq_no=0;
	gp->game_seq_no[0]=0;
	gp->game_seq_no[1]=0;

    if(gp->pNetParam){  // 通信にあわせてマリオの初期化を行います
        int	player_no;

        for(player_no=0;player_no<2;player_no++){
            if(YT_NET_IsParent(gp->pNetParam) && (player_no == 1)){
                continue;
            }
            else if(!YT_NET_IsParent(gp->pNetParam) && (player_no == 0)){
                continue;
            }
            pp = YT_InitPlayer(gp,player_no,player_no,TRUE);
            YT_InitPlayerAddTask(gp, pp, player_no);
        }
    }
    else{
        pp = YT_InitPlayer(gp,0,0,FALSE);
        YT_InitPlayerAddTask(gp, pp, 0);
		gp->ps[0].exist_flag=1;
    }

	//ゲームフラグチェックタスクセット
	gp->check_tcb=GFL_TCB_AddTask(gp->tcbsys,YT_CheckFlag,gp,TCB_PRI_PLAYER);

	YT_JobNoSet(gp,YT_MainGameNo);

	GFL_SOUND_PlayBGM(SEQ_MORI);

    GFL_NET_ReloadIcon();
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

	GFL_TCB_Main(gp->tcbsys);

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
static	void	YT_MainGameAct(GAME_PARAM *gp)
{
	int	player_no;

	switch(gp->seq_no){
	case SEQ_GAME_PLAY:
		for(player_no=0;player_no<2;player_no++){
			if(gp->pNetParam){
				if(YT_NET_IsParent(gp->pNetParam) && (player_no == 1)){
					continue;
				}
				else if(!YT_NET_IsParent(gp->pNetParam) && (player_no == 0)){
					continue;
				}
			}
			else if(gp->ps[player_no].exist_flag==0){
				continue;
			}
			if(gp->ps[player_no].status.win_lose_flag){
				gp->seq_no=SEQ_GAME_OVER;
				gp->wait_work=120;
			}
			switch(gp->game_seq_no[player_no]){
			case SEQ_GAME_START_WAIT:
				if(GFL_FADE_CheckFade()==FALSE){
					gp->game_seq_no[player_no]++;
				}
				break;
			case SEQ_GAME_READY_CHECK:
				switch(YT_ReadyCheck(gp,&gp->ps[player_no])){
				case YT_READY_MAKE:
					gp->ps[player_no].fall_count++;
					YT_ReadyAct(gp,player_no);
				case YT_READY_ALREADY:
					gp->ps[player_no].fall_wait=YT_FALL_WAIT;
					gp->game_seq_no[player_no]++;
					break;
				default:
					break;
				}
				break;
			case SEQ_GAME_FALL_CHECK:
				if(YT_FallCheck(gp,&gp->ps[player_no])==TRUE){
					if(--gp->ps[player_no].fall_wait==0){
						gp->game_seq_no[player_no]=SEQ_GAME_READY_CHECK;
					}
				}
				break;
			}
		}
		break;
	case SEQ_GAME_OVER:
		if(gp->wait_work){
			gp->wait_work--;
		}
		else{
			if(GFL_UI_KEY_GetTrg()){
				GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN|GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,0,16,2);
				gp->seq_no=SEQ_GAME_TO_TITLE;
			}
		}
		break;
	case SEQ_GAME_TO_TITLE:
		if(GFL_FADE_CheckFade()==FALSE){
			YT_ExitGame(gp);
		}
		break;
	}
}
	
//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム終了処理
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
static	void	YT_ExitGame(GAME_PARAM *gp)
{
	//ゲームフラグチェックタスク破棄
	GFL_TCB_DeleteTask(gp->check_tcb);

	//BMP関連終了
	GFL_BMPWIN_Delete(gp->yossy_bmpwin);
	GFL_BMPWIN_Exit();

	//ヨッシーキャラデータ破棄
	GFL_HEAP_FreeMemory(gp->yossy_bmp);

	//BGシステム終了
	GFL_BG_Exit();

	//エリアマネージャ破棄
	GFL_AREAMAN_Delete(gp->clact_area);

    // セルアクターユニット破棄
	GFL_CLACT_UnitDelete(gp->clact->p_unit);

	YT_JobNoSet(gp,YT_InitTitleNo);
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
		line=__GFL_STD_MtRand()%4;
		if(gp->ps[player_no].ready[line][0]){
			continue;
		}
		type=__GFL_STD_MtRand()%4;
		//デカキャラは、確率を低くする
		if(type==YT_CHR_TERESA){
			if(__GFL_STD_MtRand()%5==0){
				type=YT_CHR_DEKATERESA;
			}
		}
		else{
			//タマゴの殻発生確率
			if(__GFL_STD_MtRand()%5==0){
				type=YT_CHR_GREEN_EGG_U+__GFL_STD_MtRand()%4;
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
		p_buff = GFL_ARC_LoadDataAlloc( 0,NARC_yossyegg_O_WOODS3_NCGR, heapID );
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
		p_buff = GFL_ARC_LoadDataAlloc( 0,NARC_yossyegg_OBJ_COL_NCLR, heapID );
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
		clact_res->p_cellbuff = GFL_ARC_LoadDataAlloc( 0,NARC_yossyegg_fall_obj_NCER, heapID );
		result = NNS_G2dGetUnpackedCellBank( clact_res->p_cellbuff, &clact_res->p_cell );
		GF_ASSERT( result );
	}

	// セルアニメデータ読み込み
	{
		clact_res->p_cellanmbuff = GFL_ARC_LoadDataAlloc( 0,NARC_yossyegg_fall_obj_NANR, heapID );
		result = NNS_G2dGetUnpackedAnimBank( clact_res->p_cellanmbuff, &clact_res->p_cellanm );
		GF_ASSERT( result );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームフラグチェックタスク
 */
//-----------------------------------------------------------------------------
static	void	YT_CheckFlag(GFL_TCB *tcb,void *work)
{
	GAME_PARAM			*gp=(GAME_PARAM *)work;
	YT_PLAYER_STATUS	*ps;
	int					player_no;

	for(player_no=0;player_no<2;player_no++){
		ps=&gp->ps[player_no];
		//回転処理終了チェック
		if(ps->status.rotate_flag){
			{
				int					i;
				int					left_line_stop;
				int					right_line_stop;
				int					left_line_fall;
				int					right_line_fall;
				FALL_CHR_PARAM		*fcp;

				left_line_stop=ps->stoptbl[ps->status.rotate_flag-1];
				right_line_stop=ps->stoptbl[ps->status.rotate_flag];
				left_line_fall=ps->falltbl[ps->status.rotate_flag-1];
				right_line_fall=ps->falltbl[ps->status.rotate_flag];

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
					ps->status.rotate_flag=0;
				}
			}
		}
		//ひっくり返し処理終了チェック
		if(ps->status.overturn_flag){
			{
				int					x,y,line;
				FALL_CHR_PARAM		*fcp;

				for(x=0;x<YT_LINE_MAX;x++){
					line=ps->stoptbl[x];
					if(ps->status.overturn_flag&(1<<x)){
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
					ps->status.overturn_flag=0;
				}
			}
		}
		//タマゴ生成チェック
		if((ps->status.egg_make_check_flag)&&(ps->status.overturn_flag==0)){
			YT_EggMakeCheck(ps);
		}
		//タマゴ生成中チェック
		if((ps->status.egg_make_flag)&&(ps->egg_make_count==0)){
			ps->status.egg_make_flag=0;
		}
		//連鎖落下中チェック
		if(ps->status.rensa_flag){
			{
				int					y;
				FALL_CHR_PARAM		*fcp;

				for(y=0;y<YT_HEIGHT_MAX;y++){
					fcp=ps->rensa[y];
					if(fcp){
						break;
					}
				}
				if(y==YT_HEIGHT_MAX){
					ps->status.rensa_flag=0;
				}
			}
		}
		//ゲームオーバーチェック
		if(ps->status.no_active_flag==0){
			{
				int				x;
				FALL_CHR_PARAM	*fcp;

				for(x=0;x<YT_LINE_MAX;x++){
					fcp=ps->stop[x][YT_HEIGHT_MAX-1];
					if(fcp){
						ps->status.win_lose_flag|=YT_GAME_LOSE;
						gp->ps[player_no^1].status.win_lose_flag|=YT_GAME_WIN;
					}
				}
			}
		}
		//全消しチェック
		if((ps->status.no_active_flag==0)&&(ps->fall_count>2)){
			{
				int	x,y;

				for(x=0;x<YT_LINE_MAX;x++){
					for(y=0;y<YT_HEIGHT_MAX;y++){
						if(ps->stop[x][y]){
							break;
						}
					}
					if(y<YT_HEIGHT_MAX){
						break;
					}
				}
				if((x==YT_LINE_MAX)&&(y==YT_HEIGHT_MAX)){
					switch(YT_ReadyCheck(gp,ps)){
					case YT_READY_ALREADY:
						for(x=0;x<YT_LINE_MAX;x++){
							for(y=0;y<YT_HEIGHT_MAX;y++){
								if(ps->fall[x][y]){
									break;
								}
							}
							if(y<YT_HEIGHT_MAX){
								break;
							}
						}
						if((x!=YT_LINE_MAX)||(y!=YT_HEIGHT_MAX)){
							break;
						}
					case YT_READY_WAIT:
						ps->status.win_lose_flag|=YT_GAME_WIN;
						gp->ps[player_no^1].status.win_lose_flag|=YT_GAME_LOSE;
					default:
						break;
					}
				}
			}
		}
	}
}

