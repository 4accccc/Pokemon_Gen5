
//============================================================================================
/**
 * @file	fallchr.c
 * @brief	DS版ヨッシーのたまご　落下キャラプログラム
 * @author	sogabe
 * @date	2007.02.09
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"

#include "sample_graphic/yossyegg.naix"
#include "fall_obj_anm.h"

#define __PLAYER_H_GLOBAL__
#include "fallchr.h"

static	void	YT_MainFallChr(TCB *tcb,void *work);
static	void	YT_FallStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps);
static	int		YT_LandingCheck(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk);
static	int		YT_LandingStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int fall_line,int stop_line,int height);
static	CLWK	*YT_ClactWorkAdd(FALL_CHR_PARAM *fcp);
static	void	YT_ChrPosSet(FALL_CHR_PARAM *fcp);
static	void	YT_AnmSeqSet(FALL_CHR_PARAM *fcp,int flag);
static	void	YT_RotateActSet(FALL_CHR_PARAM *fcp);
static	void	YT_EggMakeCheckSet(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps);
static	void	YT_EggMakeFlagCheck(TCB *tcb,void *work);
static	void	YT_YossyBirth(FALL_CHR_PARAM *fcp);

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラアニメーションテーブル
 */
//-----------------------------------------------------------------------------
enum{
	YT_ANM_STOP=0,
	YT_ANM_FALL,
	YT_ANM_LANDING,
	YT_ANM_TUBURE,
	YT_ANM_OVERTURN,
	YT_ANM_EGG,
	YT_ANM_VANISH,
};

static	const	u16	yt_anime_table[][2]={
	{NANR_fall_obj_KURIBO_STOP_U,		NANR_fall_obj_KURIBO_STOP_D		},
	{NANR_fall_obj_KURIBO_FALL,			NANR_fall_obj_KURIBO_FALL		},
	{NANR_fall_obj_KURIBO_LANDING,		NANR_fall_obj_KURIBO_LANDING	},
	{NANR_fall_obj_KURIBO_TUBURE_U,		NANR_fall_obj_KURIBO_TUBURE_D	},
	{NANR_fall_obj_KURIBO_OVERTURN_UD,	NANR_fall_obj_KURIBO_OVERTURN_DU},
	{NANR_fall_obj_GREEN_EGG,			NANR_fall_obj_RED_EGG			},
	{NANR_fall_obj_CHR_KIE,				NANR_fall_obj_CHR_KIE			},
};


//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラ初期化
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		キャラタイプ
 *	@param	line_no		落下ラインナンバー
 *
 *	@retval FALL_CHR_PARAM
 */
//-----------------------------------------------------------------------------
FALL_CHR_PARAM	*YT_InitFallChr(GAME_PARAM *gp,u8 player_no,u8 type,u8 line_no)
{
	FALL_CHR_PARAM	*fcp=(FALL_CHR_PARAM *)GFL_HEAP_AllocMemoryClear(gp->heapID,sizeof(FALL_CHR_PARAM));

	fcp->gp=gp;
	fcp->player_no=player_no;
	fcp->type=type;
	fcp->line_no=line_no;
	fcp->clact_no=GFL_AREAMAN_ReserveAuto(gp->clact_area,1);

	GFL_TCB_AddTask(gp->tcbsys,YT_MainFallChr,fcp,TCB_PRI_FALL_CHR);
	gp->clact->clact_work[fcp->clact_no]=fcp->clwk=YT_ClactWorkAdd(fcp);
	GFL_CLACT_WkGetWldPos(fcp->clwk,&fcp->now_pos);

	return fcp;
}

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラメイン処理
 */
//-----------------------------------------------------------------------------
enum{
	SEQ_FALL_CHR_READY_INIT=0,
	SEQ_FALL_CHR_READY,
	SEQ_FALL_CHR_FALL_INIT,
	SEQ_FALL_CHR_FALL,
	SEQ_FALL_CHR_LANDING,
	SEQ_FALL_CHR_STOP,
	SEQ_FALL_CHR_ROTATE,
	SEQ_FALL_CHR_OVERTURN,
	SEQ_FALL_CHR_EGG_MAKE,
	SEQ_FALL_CHR_YOSSY_BIRTH,
	SEQ_FALL_CHR_VANISH_INIT,
	SEQ_FALL_CHR_VANISH,
};

enum{
	YT_ACT_FALL=0,
	YT_ACT_VANISH,
	YT_ACT_LANDING,
};

static	void	YT_MainFallChr(TCB *tcb,void *work)
{
	FALL_CHR_PARAM		*fcp=(FALL_CHR_PARAM *)work;
	GAME_PARAM			*gp=(GAME_PARAM *)fcp->gp;
	YT_PLAYER_STATUS	*ps=(YT_PLAYER_STATUS *)&gp->ps[fcp->player_no];
	CLWK				*clwk=(CLWK *)fcp->clwk;

	switch(fcp->seq_no){
	case SEQ_FALL_CHR_READY_INIT:
		//アニメを最速に
//		GFL_CLACT_WkSetAutoAnmSpeed(clwk,0);
		fcp->seq_no++;
	case SEQ_FALL_CHR_READY:
		if(fcp->now_pos.y<YT_READY_Y_POS){
			fcp->now_pos.y+=YT_READY_FALL_SPEED;
		}
		else{
			fcp->now_pos.y=YT_READY_Y_POS;
			YT_AnmSeqSet(fcp,YT_ANM_STOP);
			fcp->seq_no++;
		}
		YT_ChrPosSet(fcp);
		break;
	case SEQ_FALL_CHR_FALL_INIT:
		if(ps->fall_wait){
			break;
		}
		YT_AnmSeqSet(fcp,YT_ANM_FALL);
		YT_FallStart(fcp,ps);
		fcp->fall_wait=gp->default_fall_wait;
		fcp->fall_wait_tmp=gp->default_fall_wait;
		fcp->seq_no++;
	case SEQ_FALL_CHR_FALL:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			break;
		}
		if((ps->rotate_flag)||(ps->overturn_flag)||(ps->egg_make_check_flag)||(ps->egg_make_flag)){
			break;
		}
		if((GFL_UI_KeyGetCont()&PAD_KEY_DOWN)==0){
			GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
		}
		else{ 
			GFL_CLACT_WkSetAutoAnmSpeed(clwk,6*FX32_ONE);
		}
//		GFL_CLACT_WkSetAutoAnmSpeed(clwk,0);
		if((fcp->fall_wait)&&((GFL_UI_KeyGetCont()&PAD_KEY_DOWN)==0)){
			fcp->fall_wait--;
		}
		else{
			fcp->fall_wait=fcp->fall_wait_tmp;
			switch(YT_LandingCheck(fcp,ps,clwk)){
			case YT_ACT_VANISH:
				GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
				fcp->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				break;
			case YT_ACT_LANDING:
				YT_AnmSeqSet(fcp,YT_ANM_LANDING);
				GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
				if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
					ps->egg_make_check_flag=(1<<fcp->line_no);
				}
				fcp->seq_no=SEQ_FALL_CHR_STOP;
				break;
			default:
				if((GFL_UI_KeyGetCont()&PAD_KEY_DOWN)==0){
					fcp->now_pos.y+=YT_FALL_SPEED;
				}
				else{
					fcp->now_pos.y+=YT_FALL_SPEED*2;
				}
				YT_ChrPosSet(fcp);
				break;
			}
		}
		break;
	case SEQ_FALL_CHR_STOP:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			YT_AnmSeqSet(fcp,YT_ANM_STOP);
		}
		if(fcp->overturn_flag){
			YT_AnmSeqSet(fcp,YT_ANM_OVERTURN);
			YT_ChrPosSet(fcp);
			fcp->seq_no=SEQ_FALL_CHR_OVERTURN;
		}
		break;
	case SEQ_FALL_CHR_ROTATE:
		if(fcp->wait_value){
			fcp->wait_value--;
		}
		else{
			{
				int	i;
				int	rotate_pos[][4]={{28,52,76,100},{28,52,76,100}};
	
				fcp->now_pos.x+=fcp->speed_value;
				if(fcp->speed_value>0){
					if(fcp->now_pos.x>=rotate_pos[fcp->player_no][fcp->rotate_flag]){
						fcp->line_no=fcp->rotate_flag;
						fcp->rotate_flag=0;
					}
				}
				else{
					if(fcp->now_pos.x<=rotate_pos[fcp->player_no][fcp->rotate_flag-1]){
						fcp->line_no=fcp->rotate_flag-1;
						fcp->rotate_flag=0;
					}
				}
				YT_ChrPosSet(fcp);
				if(fcp->rotate_flag==0){
					fcp->seq_no=fcp->push_seq_no;
					if(fcp->seq_no==SEQ_FALL_CHR_STOP){
						for(i=0;i<YT_HEIGHT_MAX;i++){
							if(ps->stop[ps->stoptbl[fcp->line_no]][i]==fcp){
								break;
							}
						}
						if(ps->stop[ps->stoptbl[fcp->line_no]][i+1]){
							YT_AnmSeqSet(fcp,YT_ANM_TUBURE);
						}
					}
				}
			}
		}
		break;
	case SEQ_FALL_CHR_OVERTURN:
		if(GFL_CLACT_WkCheckAnmActive(clwk)==FALSE){
			if(fcp->type<YT_CHR_GREEN_EGG_U){
				fcp->dir^=1;
			}
			else if(fcp->type<YT_CHR_GREEN_EGG){
				fcp->type=((fcp->type-YT_CHR_GREEN_EGG_U)^1)+YT_CHR_GREEN_EGG_U;
				YT_AnmSeqSet(fcp,YT_ANM_FALL);
			}
			else{
				YT_AnmSeqSet(fcp,YT_ANM_EGG);
			}
			fcp->overturn_flag=0;
			fcp->offset_pos.y=0;
			YT_ChrPosSet(fcp);
			if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
				ps->egg_make_check_flag=(1<<fcp->line_no);
			}
			fcp->seq_no=SEQ_FALL_CHR_STOP;
			{
				int	i;
				for(i=0;i<YT_HEIGHT_MAX;i++){
					if(ps->stop[ps->stoptbl[fcp->line_no]][i]==fcp){
						break;
					}
				}
				if(ps->stop[ps->stoptbl[fcp->line_no]][i+1]){
					YT_AnmSeqSet(fcp,YT_ANM_TUBURE);
				}
			}
		}
		break;
	case SEQ_FALL_CHR_EGG_MAKE:
		if(fcp->wait_value){
			if(fcp->wait_value&1){
				fcp->now_pos.y+=fcp->speed_value;
				YT_ChrPosSet(fcp);
			}
			if(--fcp->wait_value==0){
				if(fcp->egg_make_flag){
					if(fcp->type==YT_CHR_GREEN_EGG_U){
						fcp->type=YT_CHR_GREEN_EGG;
						YT_AnmSeqSet(fcp,YT_ANM_EGG);
						fcp->seq_no=SEQ_FALL_CHR_YOSSY_BIRTH;
						fcp->egg_make_flag=0;
					}
					else if(fcp->type==YT_CHR_RED_EGG_U){
						fcp->type=YT_CHR_RED_EGG;
						YT_AnmSeqSet(fcp,YT_ANM_EGG);
						fcp->seq_no=SEQ_FALL_CHR_YOSSY_BIRTH;
						fcp->egg_make_flag=0;
					}
					else{
						GFL_AREAMAN_Release(gp->clact_area,fcp->clact_no,1);
						GFL_CLACT_WkDel(clwk);
						GFL_HEAP_FreeMemory(fcp);
						GFL_TCB_DeleteTask(tcb);
					}
				}
				else{
					if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
						ps->egg_make_check_flag=(1<<fcp->line_no);
					}
					fcp->seq_no=SEQ_FALL_CHR_STOP;
				}
			}
		}
		break;
	case SEQ_FALL_CHR_YOSSY_BIRTH:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			YT_AnmSeqSet(fcp,YT_ANM_STOP);
		}
		if(fcp->overturn_flag){
			YT_AnmSeqSet(fcp,YT_ANM_OVERTURN);
			YT_ChrPosSet(fcp);
			fcp->seq_no=SEQ_FALL_CHR_OVERTURN;
		}
		if((ps->rotate_flag)||(ps->overturn_flag)||(ps->egg_make_check_flag)||(ps->egg_make_flag)){
			break;
		}
		if(fcp->birth_wait){
			fcp->birth_wait--;
		}
		else{
			YT_YossyBirth(fcp);
		}
		break;
	case SEQ_FALL_CHR_VANISH_INIT:
		YT_AnmSeqSet(fcp,YT_ANM_VANISH);
		fcp->seq_no++;
	case SEQ_FALL_CHR_VANISH:
		if(GFL_CLACT_WkCheckAnmActive(clwk)==FALSE){
			GFL_AREAMAN_Release(gp->clact_area,fcp->clact_no,1);
			GFL_CLACT_WkDel(clwk);
			GFL_HEAP_FreeMemory(fcp);
			GFL_TCB_DeleteTask(tcb);
		}
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	落下開始時処理
 */
//-----------------------------------------------------------------------------
static	void	YT_FallStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps)
{
	int	x,y;

	//ready配列をクリア
	for(y=0;y<YT_HEIGHT_MAX;y++){
		if(ps->ready[fcp->line_no][y]==fcp){
			ps->ready[fcp->line_no][y]=NULL;
			break;
		}
	}
	//見つからなければアサート
	GF_ASSERT(y!=YT_HEIGHT_MAX);
	
	//fall配列にセット
	ps->fall[fcp->line_no][y]=fcp;
}

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラ着地チェック
 */
//-----------------------------------------------------------------------------
static	int		YT_LandingCheck(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk)
{
	int			check_fall_line=ps->falltbl[fcp->line_no];
	int			check_stop_line=ps->stoptbl[fcp->line_no];
	int			height;
	int			height_tbl[]={144,130,116,102,88,74,60,46,32};

	for(height=0;height<YT_HEIGHT_MAX;height++){
		if(ps->stop[check_stop_line][height]==0){
			break;
		}
	}

	if(fcp->now_pos.y>=height_tbl[height]){
		fcp->now_pos.y=height_tbl[height];
		YT_ChrPosSet(fcp);
		return YT_LandingStart(fcp,ps,check_fall_line,check_stop_line,height);
	}

	return YT_ACT_FALL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	着地開始時処理
 */
//-----------------------------------------------------------------------------
static	int		YT_LandingStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int fall_line,int stop_line,int height)
{
	int	x,y;

	//fall配列をクリア
	for(y=0;y<YT_HEIGHT_MAX;y++){
		if(ps->fall[fall_line][y]==fcp){
			ps->fall[fall_line][y]=NULL;
			break;
		}
	}
	//見つからなければアサート
	GF_ASSERT(y!=YT_HEIGHT_MAX);
	

	//一番下ではなければ、下のキャラをつぶすか、マッチングチェックを行う
	if(height){
		{
			FALL_CHR_PARAM	*fcp_under=ps->stop[stop_line][height-1];

			if(fcp->type==fcp_under->type){
				fcp_under->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				ps->stop[stop_line][height-1]=NULL;
				//さらに下のキャラがいた場合はつぶれを元に戻す
				if(height-1){
					fcp_under=ps->stop[stop_line][height-2];
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP);
				}
				return YT_ACT_VANISH;
			}
			else if((fcp->type==YT_CHR_TERESA)&&(fcp_under->type==YT_CHR_DEKATERESA)){
				fcp_under->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp_under,YT_ANM_STOP);
				return YT_ACT_VANISH;
			}
			else if((fcp_under->type==YT_CHR_TERESA)&&(fcp->type==YT_CHR_DEKATERESA)){
				fcp_under->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				ps->stop[stop_line][height-1]=NULL;
				//さらに下のキャラがいた場合はつぶれを元に戻す
				if(height-1){
					fcp_under=ps->stop[stop_line][height-2];
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP);
				}
				ps->fall[fall_line][y]=fcp;
				fcp->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp,YT_ANM_FALL);
				return YT_ACT_FALL;
			}
			else{
				YT_AnmSeqSet(fcp_under,YT_ANM_TUBURE);
			}
		}
	}
	//stop配列にセット
	ps->stop[stop_line][height]=fcp;

	return YT_ACT_LANDING;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクター登録
 *	
 *	@param	p_unit		セルアクターユニット
 *	@param	p_res		リソースワーク
 *	@param	cp_data		登録データ
 *	@param	heapID		ヒープ
 *
 *	@return	生成したワーク
 */
//-----------------------------------------------------------------------------
static CLWK* YT_ClactWorkAdd(FALL_CHR_PARAM *fcp)
{
	CLWK_RES	resdat;
	CLWK		*p_wk;
	CLWK_DATA	data[][YT_LINE_MAX]={
					//PLAYER1
					{
						//LINE0
						{
							28, -8,	//座標(x,y)
							NANR_fall_obj_KURIBO_FALL,		//アニメーションシーケンス
							0,		//優先順位
							0,		//bg優先順位
						},
						//LINE1
						{
							52, -8,	//座標(x,y)
							NANR_fall_obj_KURIBO_FALL,		//アニメーションシーケンス
							0,		//優先順位
							0,		//bg優先順位
						},
						//LINE2
						{
							 76, -8,	//座標(x,y)
							NANR_fall_obj_KURIBO_FALL,		//アニメーションシーケンス
							0,		//優先順位
							0,		//bg優先順位
						},
						//LINE3
						{
							100, -8,	//座標(x,y)
							NANR_fall_obj_KURIBO_FALL,		//アニメーションシーケンス
							0,		//優先順位
							0,		//bg優先順位
						},
					},
				};
	u8			soft_pri[]={
					YT_PRI_KURIBO,
					YT_PRI_PAKKUN,
					YT_PRI_GESSO,
					YT_PRI_TERESA,
					YT_PRI_DEKATERESA,
					YT_PRI_GREEN_EGG_U,
					YT_PRI_GREEN_EGG_D,
					YT_PRI_RED_EGG_U,
					YT_PRI_RED_EGG_D,
					YT_PRI_GREEN_EGG,
					YT_PRI_RED_EGG,
				};

	// リソースデータ代入
	GFL_CLACT_WkSetCellResData( &resdat, 
			&fcp->gp->clact->res.imageproxy, &fcp->gp->clact->res.plttproxy,
			fcp->gp->clact->res.p_cell, fcp->gp->clact->res.p_cellanm );
	// 登録
	data[fcp->player_no][fcp->line_no].anmseq+=fcp->type;
	data[fcp->player_no][fcp->line_no].softpri=soft_pri[fcp->type];
	p_wk = GFL_CLACT_WkAdd( fcp->gp->clact->p_unit, 
			&data[fcp->player_no][fcp->line_no], &resdat,
			CLWK_SETSF_NONE,
			fcp->gp->heapID );

	// オートアニメーション設定
	GFL_CLACT_WkSetAutoAnmFlag( p_wk, TRUE );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクター位置情報セット
 */
//-----------------------------------------------------------------------------
static	void	YT_ChrPosSet(FALL_CHR_PARAM *fcp)
{
	CLSYS_POS	pos;

	pos.x=fcp->now_pos.x+fcp->offset_pos.x;
	pos.y=fcp->now_pos.y+fcp->offset_pos.y;

	GFL_CLACT_WkSetWldPos(fcp->clwk,&pos);

	//通信対戦時には、座標を送信する
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクターアニメセット
 *	
 *	@param	fcp		落下キャラパラメータ
 *	@param	flag	アニメーションフラグ
 */
//-----------------------------------------------------------------------------
static	void	YT_AnmSeqSet(FALL_CHR_PARAM *fcp,int flag)
{
	u16	anm_seq;

	anm_seq=yt_anime_table[flag][fcp->dir];

	switch(flag){
	case YT_ANM_STOP:
	case YT_ANM_LANDING:
	case YT_ANM_TUBURE:
		//タマゴの場合はアニメセットしない
		if(fcp->type>=YT_CHR_GREEN_EGG_U){
			return;
		}
	case YT_ANM_FALL:
		anm_seq+=fcp->type;
		break;
	case YT_ANM_OVERTURN:
		//タマゴの場合は専用アニメをセット
		if(fcp->type>=YT_CHR_GREEN_EGG){
			anm_seq=NANR_fall_obj_GREEN_EGG_OVERTURN+(fcp->type-YT_CHR_GREEN_EGG);
		}
		else if(fcp->type>=YT_CHR_GREEN_EGG_U){
			anm_seq=NANR_fall_obj_GREEN_EGG_OVERTURN_UD+(fcp->type-YT_CHR_GREEN_EGG_U);
		}
		else{
			anm_seq+=fcp->type;
		}
		break;
	case YT_ANM_EGG:
		if(fcp->type==YT_CHR_GREEN_EGG){
			anm_seq=yt_anime_table[flag][0];
		}
		else{
			anm_seq=yt_anime_table[flag][1];
		}
		break;
	case YT_ANM_VANISH:
		break;
	default:
		//ありえない数値はバグなので止める
		GF_ASSERT(0);
		break;
	}

	GFL_CLACT_WkSetAnmSeq(fcp->clwk,anm_seq);

	//通信対戦時には、アニメーションナンバーを送信する
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転アクションセット
 *	
 *	@param	clwk		セルアクターワーク
 *	@param	anm_seq		アニメーションナンバー
 *	@param	clact_no	セルアクターナンバー
 */
//-----------------------------------------------------------------------------
static	void	YT_RotateActSet(FALL_CHR_PARAM *fcp)
{
	//左移動
	if(fcp->rotate_flag==fcp->line_no){
		fcp->speed_value=-YT_ROTATE_SPEED;
	}
	else{
		fcp->speed_value=YT_ROTATE_SPEED;
	}
	fcp->push_seq_no=fcp->seq_no;
	fcp->seq_no=SEQ_FALL_CHR_ROTATE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タマゴ生成チェック
 *	
 *	@param	fcp		落下キャラパラメータ
 *	@param	ps		プレーヤーステータス
 *
 *  @retval	FALSE:タマゴ生成なし　TRUE:タマゴ生成あり
 */
//-----------------------------------------------------------------------------
void	YT_EggMakeCheck(YT_PLAYER_STATUS *ps)
{
	int				line_no=0;
	int				egg_line;
	int				egg_height;
	int				egg_search;
	int				chr_count=0;
	int				egg_count=1;
	FALL_CHR_PARAM	*fcp_p;

	while(ps->egg_make_check_flag){
		if(ps->egg_make_check_flag&1){
			egg_line=ps->stoptbl[line_no];

			for(egg_height=0;egg_height<YT_HEIGHT_MAX;egg_height++){
				fcp_p=ps->stop[egg_line][egg_height];
				if((fcp_p->type==YT_CHR_GREEN_EGG_U)||(fcp_p->type==YT_CHR_RED_EGG_U)){
					break;
				}
			}
			//見つからなかったらアサート
			GF_ASSERT(egg_height!=YT_HEIGHT_MAX);

			fcp_p->egg_make_flag=1;
			egg_search=egg_height;
		
			while(egg_search){
				egg_search--;
				fcp_p=ps->stop[egg_line][egg_search];
				switch(fcp_p->type){
				//途中で上殻が見つかった時は多重タマゴなので、先に内側のタマゴ生成をする（見つかることはないけど）
				case YT_CHR_GREEN_EGG_U:
				case YT_CHR_RED_EGG_U:
					egg_search=0;
					break;
				case YT_CHR_GREEN_EGG_D:
				case YT_CHR_RED_EGG_D:
					{
						int	height;
						int	speed_value=0;
						u8	soft_pri[]={
							YT_PRI_KURIBO,
							YT_PRI_PAKKUN,
							YT_PRI_GESSO,
							YT_PRI_TERESA,
							YT_PRI_DEKATERESA,
							YT_PRI_GREEN_EGG_U,
							YT_PRI_GREEN_EGG_D,
							YT_PRI_RED_EGG_U,
							YT_PRI_RED_EGG_D,
							YT_PRI_GREEN_EGG,
							YT_PRI_RED_EGG,
						};
		
						for(height=egg_search;height<egg_height;height++){
							fcp_p=ps->stop[egg_line][height];
							fcp_p->seq_no=SEQ_FALL_CHR_EGG_MAKE;
							fcp_p->speed_value=speed_value;
							fcp_p->egg_make_flag=1;
							fcp_p->wait_value=28;
							GFL_CLACT_WkSetSoftPri(fcp_p->clwk,soft_pri[fcp_p->type]);
							ps->stop[egg_line][height]=NULL;
							speed_value++;
						}
						while(ps->stop[egg_line][height]){
							fcp_p=ps->stop[egg_line][height];
							ps->stop[egg_line][egg_search]=fcp_p;
							fcp_p->seq_no=SEQ_FALL_CHR_EGG_MAKE;
							fcp_p->speed_value=speed_value;
							fcp_p->wait_value=28;
							GFL_CLACT_WkSetSoftPri(fcp_p->clwk,soft_pri[fcp_p->type]);
							ps->stop[egg_line][height]=NULL;
							height++;
							egg_search++;
						}
					}
					fcp_p=ps->stop[egg_line][egg_height];
					fcp_p->chr_count=chr_count*egg_count;
					fcp_p->birth_wait=YT_BIRTH_WAIT*fcp_p->chr_count;
					egg_search=0;
					break;
				case YT_CHR_GREEN_EGG:
				case YT_CHR_RED_EGG:
					egg_count++;
					chr_count+=fcp_p->chr_count;
					break;
				default:
					chr_count++;
					break;
				}
			}
		}
		line_no++;
		ps->egg_make_check_flag=ps->egg_make_check_flag>>1;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ヨッシー誕生処理
 *	
 *	@param	fcp		落下キャラパラメータ
 */
//-----------------------------------------------------------------------------
static	void	YT_YossyBirth(FALL_CHR_PARAM *fcp)
{
}

