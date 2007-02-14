
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
static	void	YT_AnmSeqSet(CLWK *clwk,u16 anm_seq,int clact_no);
static	void	YT_RotateActSet(FALL_CHR_PARAM *fcp);

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
	FALL_CHR_PARAM	*fcp=(FALL_CHR_PARAM *)GFL_HEAP_AllocMemory(gp->heapID,sizeof(FALL_CHR_PARAM));

	fcp->gp=gp;
	fcp->seq_no=0;
	fcp->player_no=player_no;
	fcp->type=type;
	fcp->line_no=line_no;
	fcp->dir=0;
	fcp->clact_no=GFL_AREAMAN_ReserveAuto(gp->clact_area,1);
	fcp->offset_pos.x=0;
	fcp->offset_pos.y=0;

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
			YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_STOP_U+YT_FALL_CHR_ANM_OFS*fcp->type,fcp->clact_no);
			fcp->seq_no++;
		}
		YT_ChrPosSet(fcp);
		break;
	case SEQ_FALL_CHR_FALL_INIT:
		if(ps->fall_wait){
			break;
		}
		YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_FALL+YT_FALL_CHR_ANM_OFS*fcp->type,fcp->clact_no);
		YT_FallStart(fcp,ps);
		fcp->fall_wait=gp->default_fall_wait;
		fcp->fall_wait_tmp=gp->default_fall_wait;
		fcp->seq_no++;
	case SEQ_FALL_CHR_FALL:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			break;
		}
		if((ps->rotate_flag)||(ps->overturn_flag)){
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
				YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_LANDING+YT_FALL_CHR_ANM_OFS*fcp->type,fcp->clact_no);
				GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
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
			YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_STOP_U+YT_FALL_CHR_ANM_OFS*fcp->type+fcp->dir,fcp->clact_no);
		}
		if(fcp->overturn_flag){
			YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_OVERTURN_UD+YT_FALL_CHR_ANM_OFS*fcp->type+fcp->dir,fcp->clact_no);
			YT_ChrPosSet(fcp);
			fcp->seq_no=SEQ_FALL_CHR_OVERTURN;
		}
		break;
	case SEQ_FALL_CHR_ROTATE:
		if(fcp->rotate_wait){
			fcp->rotate_wait--;
		}
		else{
			{
				int	i;
				int	rotate_pos[][4]={{28,52,76,100},{28,52,76,100}};
	
				fcp->now_pos.x+=fcp->rotate_speed;
				if(fcp->rotate_speed>0){
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
							YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_TUBURE_U+YT_FALL_CHR_ANM_OFS*fcp->type+fcp->dir,fcp->clact_no);
						}
					}
				}
			}
		}
		break;
	case SEQ_FALL_CHR_OVERTURN:
		if(GFL_CLACT_WkCheckAnmActive(clwk)==FALSE){
			fcp->dir^=1;
			fcp->overturn_flag=0;
			fcp->offset_pos.y=0;
			YT_ChrPosSet(fcp);
			fcp->seq_no=SEQ_FALL_CHR_STOP;
			{
				int	i;
				for(i=0;i<YT_HEIGHT_MAX;i++){
					if(ps->stop[ps->stoptbl[fcp->line_no]][i]==fcp){
						break;
					}
				}
				if(ps->stop[ps->stoptbl[fcp->line_no]][i+1]){
					YT_AnmSeqSet(clwk,NANR_fall_obj_KURIBO_TUBURE_U+YT_FALL_CHR_ANM_OFS*fcp->type+fcp->dir,fcp->clact_no);
				}
			}
		}
		break;
	case SEQ_FALL_CHR_EGG_MAKE:
		break;
	case SEQ_FALL_CHR_VANISH_INIT:
		YT_AnmSeqSet(clwk,NANR_fall_obj_CHR_KIE,fcp->clact_no);
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
			CLWK			*clwk_under=fcp_under->clwk;
			if(fcp->type==fcp_under->type){
				fcp_under->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				ps->stop[stop_line][height-1]=NULL;
				//さらに下のキャラがいた場合はつぶれを元に戻す
				if(height-1){
					fcp_under=ps->stop[stop_line][height-2];
					clwk_under=fcp_under->clwk;
					YT_AnmSeqSet(clwk_under,NANR_fall_obj_KURIBO_STOP_U+YT_FALL_CHR_ANM_OFS*fcp_under->type+fcp_under->dir,fcp->clact_no);
				}
				return YT_ACT_VANISH;
			}
			else{
				YT_AnmSeqSet(clwk_under,NANR_fall_obj_KURIBO_TUBURE_U+YT_FALL_CHR_ANM_OFS*fcp_under->type+fcp_under->dir,fcp->clact_no);
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

	// リソースデータ代入
	GFL_CLACT_WkSetCellResData( &resdat, 
			&fcp->gp->clact->res.imageproxy, &fcp->gp->clact->res.plttproxy,
			fcp->gp->clact->res.p_cell, fcp->gp->clact->res.p_cellanm );
	// 登録
	data[fcp->player_no][fcp->line_no].anmseq+=YT_FALL_CHR_ANM_OFS*fcp->type;
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
 *	@param	clwk		セルアクターワーク
 *	@param	anm_seq		アニメーションナンバー
 *	@param	clact_no	セルアクターナンバー
 */
//-----------------------------------------------------------------------------
static	void	YT_AnmSeqSet(CLWK *clwk,u16 anm_seq,int clact_no)
{
	GFL_CLACT_WkSetAnmSeq(clwk,anm_seq);

	//通信対戦時には、座標を送信する
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
		fcp->rotate_speed=-YT_ROTATE_SPEED;
	}
	else{
		fcp->rotate_speed=YT_ROTATE_SPEED;
	}
	fcp->push_seq_no=fcp->seq_no;
	fcp->seq_no=SEQ_FALL_CHR_ROTATE;
}

