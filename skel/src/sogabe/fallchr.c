
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

static	void	YT_MainFallChr(GFL_TCB *tcb,void *work);
static	void	YT_FallStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps);
static	int		YT_LandingCheck(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk,NET_PARAM* pNet);
static	int		YT_LandingStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int fall_line,int stop_line,int height,NET_PARAM* pNet);
static	int		YT_LandingCheckRensa(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk,NET_PARAM* pNet);
static	int		YT_LandingStartRensa(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int stop_line,int height,NET_PARAM* pNet);
static	CLWK	*YT_ClactWorkAdd(FALL_CHR_PARAM *fcp);
static	void	YT_AnmSeqSet(FALL_CHR_PARAM *fcp,int flag, NET_PARAM* pNet);
static	void	YT_ChrPosSet(FALL_CHR_PARAM *fcp, NET_PARAM* pNet);
static	void	YT_RotateActSet(FALL_CHR_PARAM *fcp);
static	void	YT_YossyBirth(GAME_PARAM *gp,FALL_CHR_PARAM *fcp);
static	void	YT_YossyBirthAnime(GFL_TCB *tcb,void *work);

typedef struct{
	GAME_PARAM			*gp;
	YT_PLAYER_STATUS	*ps;
	int					seq_no;
	int					pat_no;
	int					wait;
	int					pos_x;
	int					pos_y;
	int					count;
}YOSSY_BIRTH_ANIME;

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
	FALL_CHR_PARAM	*fcp=(FALL_CHR_PARAM *)GFL_HEAP_AllocClearMemory(gp->heapID,sizeof(FALL_CHR_PARAM));
    NET_PARAM* pNet = gp->pNetParam;

	fcp->gp=gp;
	fcp->player_no=player_no;
	fcp->type=type;
	fcp->line_no=line_no;
	fcp->clact_no=GFL_AREAMAN_ReserveAuto(gp->clact_area,1);

	GFL_TCB_AddTask(gp->tcbsys,YT_MainFallChr,fcp,TCB_PRI_FALL_CHR);
	gp->clact->clact_work[fcp->clact_no]=fcp->clwk=YT_ClactWorkAdd(fcp);

	GFL_CLACT_WkGetWldPos(fcp->clwk,&fcp->now_pos);

    //通信対戦時には、作成typeを送信
    if(pNet){
        YT_NET_SendAnmCreate(fcp->clact_no, type, pNet);
    }
    
	return fcp;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Network用落下キャラ生成
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		キャラタイプ
 *	@param	line_no		落下ラインナンバー
 *
 *	@retval CLWK*
 */
//-----------------------------------------------------------------------------
CLWK* YT_InitNetworkFallChr(GAME_PARAM *gp,u8 player_no,u8 type,u8 line_no)
{
	FALL_CHR_PARAM	*fcp=(FALL_CHR_PARAM *)GFL_HEAP_AllocClearMemory(gp->heapID,sizeof(FALL_CHR_PARAM));
    CLWK* pCLWK;
    
	fcp->gp=gp;
	fcp->player_no=player_no;
	fcp->type=type;
	fcp->line_no=line_no;
	fcp->clact_no=GFL_AREAMAN_ReserveAuto(gp->clact_area,1);
    pCLWK=YT_ClactWorkAdd(fcp);
	GFL_CLACT_WkGetWldPos(pCLWK,&fcp->now_pos);
    GFL_HEAP_FreeMemory(fcp);
	return pCLWK;
}

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラ削除
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		キャラタイプ
 *	@param	line_no		落下ラインナンバー
 *
 *	@retval void
 */
//-----------------------------------------------------------------------------
static void YT_DeleteFallChr(CLWK* clwk,FALL_CHR_PARAM *fcp,GFL_TCB *tcb,NET_PARAM* pNet)
{
	//通信対戦時には、消す命令を送信する
    if(pNet){
        YT_NET_DeleteReq(fcp->clact_no, pNet);
    }

    GFL_CLACT_WkDel(clwk);
    GFL_HEAP_FreeMemory(fcp);
    GFL_TCB_DeleteTask(tcb);

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
	SEQ_FALL_CHR_RENSA_FALL_P,
	SEQ_FALL_CHR_RENSA_FALL_C,
	SEQ_FALL_CHR_VANISH_INIT,
	SEQ_FALL_CHR_VANISH,
};

enum{
	YT_ACT_FALL=0,
	YT_ACT_VANISH,
	YT_ACT_LANDING,
};

static	FALL_CHR_PARAM	*fcp_p=NULL;

static	void	YT_MainFallChr(GFL_TCB *tcb,void *work)
{
	FALL_CHR_PARAM		*fcp=(FALL_CHR_PARAM *)work;
	GAME_PARAM			*gp=(GAME_PARAM *)fcp->gp;
    NET_PARAM*          pNet = gp->pNetParam;
	YT_PLAYER_STATUS	*ps=(YT_PLAYER_STATUS *)&gp->ps[fcp->player_no];
	CLWK				*clwk=(CLWK *)fcp->clwk;

    GF_ASSERT(clwk);

	fcp_p=fcp;

	if(gp->seq_no==SEQ_GAME_TO_TITLE){
		GFL_HEAP_FreeMemory(work);
		GFL_TCB_DeleteTask(tcb);
		return;
	}

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
			YT_AnmSeqSet(fcp,YT_ANM_STOP,pNet);
			fcp->seq_no++;
		}
		YT_ChrPosSet(fcp, pNet);
		break;
	case SEQ_FALL_CHR_FALL_INIT:
		if(ps->fall_wait){
			break;
		}
		YT_AnmSeqSet(fcp,YT_ANM_FALL,pNet);
		YT_FallStart(fcp,ps);
		fcp->fall_wait=gp->default_fall_wait;
		fcp->fall_wait_tmp=gp->default_fall_wait;
		fcp->seq_no++;
	case SEQ_FALL_CHR_FALL:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			break;
		}
		if(ps->status.no_active_flag){
			break;
		}
		if((GFL_UI_KEY_GetCont()&PAD_KEY_DOWN)==0){
			GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
		}
		else{ 
			GFL_CLACT_WkSetAutoAnmSpeed(clwk,6*FX32_ONE);
		}
//		GFL_CLACT_WkSetAutoAnmSpeed(clwk,0);
		if((fcp->fall_wait)&&((GFL_UI_KEY_GetCont()&PAD_KEY_DOWN)==0)){
			fcp->fall_wait--;
		}
		else{
			fcp->fall_wait=fcp->fall_wait_tmp;
			switch(YT_LandingCheck(fcp,ps,clwk,pNet)){
			case YT_ACT_VANISH:
				GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
				fcp->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				break;
			case YT_ACT_LANDING:
				YT_AnmSeqSet(fcp,YT_ANM_LANDING,pNet);
				GFL_CLACT_WkSetAutoAnmSpeed(clwk,1*FX32_ONE);
				if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
					ps->status.egg_make_check_flag|=(1<<fcp->line_no);
				}
				fcp->seq_no=SEQ_FALL_CHR_STOP;
				break;
			default:
				if((GFL_UI_KEY_GetCont()&PAD_KEY_DOWN)==0){
					fcp->now_pos.y+=YT_FALL_SPEED;
				}
				else{
					fcp->now_pos.y+=YT_FALL_SPEED*2;
				}
				YT_ChrPosSet(fcp, pNet);
				break;
			}
		}
		break;
	case SEQ_FALL_CHR_STOP:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			YT_AnmSeqSet(fcp,YT_ANM_STOP,pNet);
		}
		if(fcp->overturn_flag){
			YT_AnmSeqSet(fcp,YT_ANM_OVERTURN,pNet);
			YT_ChrPosSet(fcp, pNet);
			fcp->push_seq_no=fcp->seq_no;
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
				int	rotate_pos[][4]={{28,52,76,100},{28+128,52+128,76+128,100+128}};
	
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
				YT_ChrPosSet(fcp, pNet);
				if(fcp->rotate_flag==0){
					fcp->seq_no=fcp->push_seq_no;
					if(fcp->seq_no==SEQ_FALL_CHR_STOP){
						for(i=0;i<YT_HEIGHT_MAX;i++){
							if(ps->stop[ps->stoptbl[fcp->line_no]][i]==fcp){
								break;
							}
						}
						if(ps->stop[ps->stoptbl[fcp->line_no]][i+1]){
							YT_AnmSeqSet(fcp,YT_ANM_TUBURE,pNet);
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
				YT_AnmSeqSet(fcp,YT_ANM_FALL,pNet);
			}
			else{
				YT_AnmSeqSet(fcp,YT_ANM_EGG,pNet);
			}
			fcp->overturn_flag=0;
			fcp->offset_pos.y=0;
			YT_ChrPosSet(fcp, pNet);
			if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
				ps->status.egg_make_check_flag|=(1<<fcp->line_no);
			}
			fcp->seq_no=fcp->push_seq_no;
			{
				int	i;
				for(i=0;i<YT_HEIGHT_MAX;i++){
					if(ps->stop[ps->stoptbl[fcp->line_no]][i]==fcp){
						break;
					}
				}
				if(ps->stop[ps->stoptbl[fcp->line_no]][i+1]){
					YT_AnmSeqSet(fcp,YT_ANM_TUBURE,pNet);
				}
			}
		}
		break;
	case SEQ_FALL_CHR_EGG_MAKE:
		if(fcp->wait_value){
			if(fcp->wait_value&1){
				fcp->now_pos.y+=fcp->speed_value;
				YT_ChrPosSet(fcp, pNet);
			}
			if(--fcp->wait_value==0){
				if(fcp->egg_make_flag){
					if(fcp->type==YT_CHR_GREEN_EGG_U){
						fcp->type=YT_CHR_GREEN_EGG;
						YT_AnmSeqSet(fcp,YT_ANM_EGG,pNet);
						fcp->seq_no=SEQ_FALL_CHR_YOSSY_BIRTH;
						fcp->egg_make_flag=0;
						ps->egg_make_count--;
					}
					else if(fcp->type==YT_CHR_RED_EGG_U){
						fcp->type=YT_CHR_RED_EGG;
						YT_AnmSeqSet(fcp,YT_ANM_EGG,pNet);
						fcp->seq_no=SEQ_FALL_CHR_YOSSY_BIRTH;
						fcp->egg_make_flag=0;
						ps->egg_make_count--;
					}
					else{
						ps->egg_make_count--;
						GFL_AREAMAN_Release(gp->clact_area,fcp->clact_no,1);
                        YT_DeleteFallChr(clwk,fcp,tcb,pNet);
					}
				}
				else{
					if((fcp->type==YT_CHR_GREEN_EGG_U)||(fcp->type==YT_CHR_RED_EGG_U)){
						ps->status.egg_make_check_flag|=(1<<fcp->line_no);
					}
					fcp->seq_no=SEQ_FALL_CHR_STOP;
				}
			}
		}
		break;
	case SEQ_FALL_CHR_YOSSY_BIRTH:
		if(fcp->rotate_flag){
			YT_RotateActSet(fcp);
			YT_AnmSeqSet(fcp,YT_ANM_STOP,pNet);
		}
		if(fcp->overturn_flag){
			YT_AnmSeqSet(fcp,YT_ANM_OVERTURN,pNet);
			YT_ChrPosSet(fcp, pNet);
			fcp->push_seq_no=fcp->seq_no;
			fcp->seq_no=SEQ_FALL_CHR_OVERTURN;
		}
		if(ps->status.no_active_flag){
			break;
		}
		if(fcp->birth_wait){
			if(fcp->birth_wait>YT_BIRTH_SPEED){
				fcp->birth_wait-=YT_BIRTH_SPEED;
			}
			else{
				fcp->birth_wait=0;
			}
		}
		else{
			YT_YossyBirth(gp,fcp);
			GFL_AREAMAN_Release(gp->clact_area,fcp->clact_no,1);
            YT_DeleteFallChr(clwk,fcp,tcb,pNet);
			return;
		}
		{
			u32	wait;

			wait=20*FX32_ONE-fcp->birth_wait;
			GFL_CLACT_WkSetAutoAnmSpeed(clwk,wait);
		}
		break;
	case SEQ_FALL_CHR_RENSA_FALL_P:
		{
			FALL_CHR_PARAM		*fcp_rensa;
			int					y;

			if(ps->status.no_active_flag==YT_RENSA_FLAG){
				switch(YT_LandingCheckRensa(fcp,ps,clwk,pNet)){
				case YT_ACT_VANISH:
					fcp->seq_no=SEQ_FALL_CHR_VANISH_INIT;
					for(y=0;y<YT_HEIGHT_MAX;y++){
						fcp_rensa=ps->rensa[y];
						if((fcp_rensa!=fcp)&&(fcp_rensa)){
							fcp_rensa->seq_no=SEQ_FALL_CHR_RENSA_FALL_P;
							break;
						}
					}
					break;
				case YT_ACT_LANDING:
					fcp->seq_no=SEQ_FALL_CHR_STOP;
					for(y=0;y<YT_HEIGHT_MAX;y++){
						fcp_rensa=ps->rensa[y];
						if((fcp_rensa!=fcp)&&(fcp_rensa)){
							fcp_rensa->seq_no=SEQ_FALL_CHR_RENSA_FALL_P;
							break;
						}
					}
					break;
				default:
					fcp->now_pos.y+=YT_FALL_SPEED*4;
					YT_ChrPosSet(fcp, pNet);
					for(y=0;y<YT_HEIGHT_MAX;y++){
						fcp_rensa=ps->rensa[y];
						if((fcp_rensa!=fcp)&&(fcp_rensa)){
							fcp_rensa->now_pos.y+=YT_FALL_SPEED*4;
							YT_ChrPosSet(fcp_rensa, pNet);
						}
					}
					break;
				}
			}
		}
		break;
	case SEQ_FALL_CHR_RENSA_FALL_C:
		break;
	case SEQ_FALL_CHR_VANISH_INIT:
		YT_AnmSeqSet(fcp,YT_ANM_VANISH,pNet);
		fcp->seq_no++;
	case SEQ_FALL_CHR_VANISH:
		if(GFL_CLACT_WkCheckAnmActive(clwk)==FALSE){
			GFL_AREAMAN_Release(gp->clact_area,fcp->clact_no,1);
            YT_DeleteFallChr(clwk,fcp,tcb,pNet);
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
static	int		YT_LandingCheck(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk,NET_PARAM* pNet)
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
		YT_ChrPosSet(fcp, pNet);
		return YT_LandingStart(fcp,ps,check_fall_line,check_stop_line,height,pNet);
	}

	return YT_ACT_FALL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	着地開始時処理
 */
//-----------------------------------------------------------------------------
static	int		YT_LandingStart(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int fall_line,int stop_line,int height,NET_PARAM* pNet)
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
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				}
				return YT_ACT_VANISH;
			}
			else if((fcp->type==YT_CHR_TERESA)&&(fcp_under->type==YT_CHR_DEKATERESA)){
				fcp_under->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				return YT_ACT_VANISH;
			}
			else if((fcp_under->type==YT_CHR_TERESA)&&(fcp->type==YT_CHR_DEKATERESA)){
				fcp_under->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				ps->stop[stop_line][height-1]=NULL;
				//さらに下のキャラがいた場合はつぶれを元に戻す
				if(height-1){
					fcp_under=ps->stop[stop_line][height-2];
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				}
				ps->fall[fall_line][y]=fcp;
				fcp->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp,YT_ANM_FALL,pNet);
				return YT_ACT_FALL;
			}
			else{
				YT_AnmSeqSet(fcp_under,YT_ANM_TUBURE,pNet);
			}
		}
	}
	//stop配列にセット
	ps->stop[stop_line][height]=fcp;

	return YT_ACT_LANDING;
}

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラ着地チェック（連鎖時専用）
 */
//-----------------------------------------------------------------------------
static	int		YT_LandingCheckRensa(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,CLWK *clwk,NET_PARAM* pNet)
{
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
		YT_ChrPosSet(fcp, pNet);
		return YT_LandingStartRensa(fcp,ps,check_stop_line,height,pNet);
	}

	return YT_ACT_FALL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	着地開始時処理
 */
//-----------------------------------------------------------------------------
static	int		YT_LandingStartRensa(FALL_CHR_PARAM *fcp,YT_PLAYER_STATUS *ps,int stop_line,int height,NET_PARAM* pNet)
{
	int	x,y;

	//rensa配列をクリア
	for(y=0;y<YT_HEIGHT_MAX;y++){
		if(ps->rensa[y]==fcp){
			ps->rensa[y]=NULL;
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
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				}
				return YT_ACT_VANISH;
			}
			else if((fcp->type==YT_CHR_TERESA)&&(fcp_under->type==YT_CHR_DEKATERESA)){
				fcp_under->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				return YT_ACT_VANISH;
			}
			else if((fcp_under->type==YT_CHR_TERESA)&&(fcp->type==YT_CHR_DEKATERESA)){
				fcp_under->seq_no=SEQ_FALL_CHR_VANISH_INIT;
				ps->stop[stop_line][height-1]=NULL;
				//さらに下のキャラがいた場合はつぶれを元に戻す
				if(height-1){
					fcp_under=ps->stop[stop_line][height-2];
					YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
				}
				ps->rensa[y]=fcp;
				fcp->type=YT_CHR_TERESA;
				YT_AnmSeqSet(fcp,YT_ANM_STOP,pNet);
				return YT_ACT_FALL;
			}
			else{
				YT_AnmSeqSet(fcp_under,YT_ANM_TUBURE,pNet);
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
	CLWK_DATA	data={
					28, -8,	//座標(x,y)
					NANR_fall_obj_KURIBO_FALL,		//アニメーションシーケンス
					0,		//優先順位
					0,		//bg優先順位
				};
	s16			pos_data[][YT_LINE_MAX][2]={
					//PLAYER1
					{
						//LINE0
						{
							 28, -8,	//座標(x,y)
						},
						//LINE1
						{
							 52, -8,	//座標(x,y)
						},
						//LINE2
						{
							 76, -8,	//座標(x,y)
						},
						//LINE3
						{
							100, -8,	//座標(x,y)
						},
					},
					//PLAYER2
					{
						//LINE0
						{
							 28+128, -8,	//座標(x,y)
						},
						//LINE1
						{
							 52+128, -8,	//座標(x,y)
						},
						//LINE2
						{
							 76+128, -8,	//座標(x,y)
						},
						//LINE3
						{
							100+128, -8,	//座標(x,y)
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
	u8			bg_pri[]={
					2,		//YT_PRI_KURIBO,
					2,		//YT_PRI_PAKKUN,
					2,		//YT_PRI_GESSO,
					2,		//YT_PRI_TERESA,
					2,		//YT_PRI_DEKATERESA,
					2,		//YT_PRI_GREEN_EGG_U,
					2,		//YT_PRI_GREEN_EGG_D,
					2,		//YT_PRI_RED_EGG_U,
					2,		//YT_PRI_RED_EGG_D,
					2,		//YT_PRI_GREEN_EGG,
					2,		//YT_PRI_RED_EGG,
				};

	// リソースデータ代入
	GFL_CLACT_WkSetCellResData( &resdat, 
			&fcp->gp->clact->res.imageproxy, &fcp->gp->clact->res.plttproxy,
			fcp->gp->clact->res.p_cell, fcp->gp->clact->res.p_cellanm );
	// 登録
	data.pos_x=pos_data[fcp->player_no][fcp->line_no][0];
	data.pos_y=pos_data[fcp->player_no][fcp->line_no][1];
	data.anmseq+=fcp->type;
	data.softpri=soft_pri[fcp->type];
	data.bgpri=bg_pri[fcp->type];
	p_wk = GFL_CLACT_WkAdd( fcp->gp->clact->p_unit, 
			&data, &resdat,
			CLWK_SETSF_NONE,
			fcp->gp->heapID );
    GF_ASSERT(p_wk);

	// オートアニメーション設定
	GFL_CLACT_WkSetAutoAnmFlag( p_wk, TRUE );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクター位置情報セット
 */
//-----------------------------------------------------------------------------
static	void	YT_ChrPosSet(FALL_CHR_PARAM *fcp, NET_PARAM* pNet)
{
	CLSYS_POS	pos;

	pos.x=fcp->now_pos.x+fcp->offset_pos.x;
	pos.y=fcp->now_pos.y+fcp->offset_pos.y;

	GFL_CLACT_WkSetWldPos(fcp->clwk,&pos);

	//通信対戦時には、座標を送信する
    if(pNet){
        YT_NET_SendPosReq(fcp->clact_no, pos.x, pos.y, pNet);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクターアニメセット
 *	
 *	@param	fcp		落下キャラパラメータ
 *	@param	flag	アニメーションフラグ
 */
//-----------------------------------------------------------------------------
static	void	YT_AnmSeqSet(FALL_CHR_PARAM *fcp,int flag,NET_PARAM* pNet)
{
	u16	anm_seq;

	anm_seq=yt_anime_table[flag][fcp->dir];

	switch(flag){
	case YT_ANM_LANDING:
		YT_NET_PlaySE(SE_LANDING,pNet);
	case YT_ANM_STOP:
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
		YT_NET_PlaySE(SE_VANISH,pNet);
		break;
	default:
		//ありえない数値はバグなので止める
		GF_ASSERT(0);
		break;
	}

	GFL_CLACT_WkSetAnmSeq(fcp->clwk,anm_seq);

    //通信対戦時には、アニメーションナンバーを送信する
    if(pNet){
        YT_NET_SendAnmReq(fcp->clact_no, anm_seq, pNet);
    }
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
	int				chr_count;
	int				egg_count;
	FALL_CHR_PARAM	*fcp_top;
	FALL_CHR_PARAM	*fcp_p;

	while(ps->status.egg_make_check_flag){
		if(ps->status.egg_make_check_flag&1){
			egg_line=ps->stoptbl[line_no];
			chr_count=0;
			egg_count=1;
			for(egg_height=0;egg_height<YT_HEIGHT_MAX;egg_height++){
				fcp_top=ps->stop[egg_line][egg_height];
				if(fcp_top){
					if((fcp_top->type==YT_CHR_GREEN_EGG_D)||(fcp_top->type==YT_CHR_RED_EGG_D)){
						break;
					}
				}
			}
			if(egg_height==YT_HEIGHT_MAX){
				egg_height=0;
			}
			for(;egg_height<YT_HEIGHT_MAX;egg_height++){
				fcp_top=ps->stop[egg_line][egg_height];
				if(fcp_top){
					if((fcp_top->type==YT_CHR_GREEN_EGG_U)||(fcp_top->type==YT_CHR_RED_EGG_U)){
						GFL_CLACT_WkSetSoftPri(fcp_top->clwk,YT_PRI_GREEN_EGG_U);
						break;
					}
				}
			}
			//見つからなかったらアサート
			GF_ASSERT(egg_height!=YT_HEIGHT_MAX);

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
					GFL_CLACT_WkSetSoftPri(fcp_p->clwk,YT_PRI_GREEN_EGG_D);
					//タマゴではさんだ個数を計算
					fcp_p=ps->stop[egg_line][egg_height];
					fcp_p->chr_count=chr_count*egg_count;
					fcp_p->birth_wait=YT_BIRTH_WAIT*fcp_p->chr_count;
					if(fcp_p->birth_wait>20*FX32_ONE){
						fcp_p->birth_wait=20*FX32_ONE;
					}
					{
						int	height;
						int	speed_value=0;
						int	wait_value=28;
		
						for(height=egg_search;height<egg_height;height++){
							fcp_p=ps->stop[egg_line][height];
							fcp_p->seq_no=SEQ_FALL_CHR_EGG_MAKE;
							fcp_p->speed_value=speed_value;
							fcp_p->egg_make_flag=1;
							ps->egg_make_count++;
							ps->stop[egg_line][height]=NULL;
							if(chr_count){
								fcp_p->wait_value=wait_value;
								speed_value++;
							}
							else{
								wait_value=14;
								fcp_p->wait_value=wait_value;
								speed_value+=2;
							}
						}
						while((ps->stop[egg_line][height])&&(height<YT_HEIGHT_MAX)){
							fcp_p=ps->stop[egg_line][height];
							ps->stop[egg_line][egg_search]=fcp_p;
							fcp_p->seq_no=SEQ_FALL_CHR_EGG_MAKE;
							fcp_p->speed_value=speed_value;
							fcp_p->wait_value=wait_value;
							ps->stop[egg_line][height]=NULL;
							height++;
							egg_search++;
						}
					}
					fcp_top->egg_make_flag=1;
					ps->status.egg_make_flag=1;
					ps->egg_make_count++;
					egg_search=0;
					break;
				case YT_CHR_GREEN_EGG:
				case YT_CHR_RED_EGG:
					egg_count++;
					chr_count+=fcp_p->chr_count;
					break;
				case YT_CHR_DEKATERESA:
					chr_count+=2;
					break;
				default:
					chr_count++;
					break;
				}
			}
		}
		line_no++;
		ps->status.egg_make_check_flag=ps->status.egg_make_check_flag>>1;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ヨッシー誕生処理
 *	
 *	@param	fcp		落下キャラパラメータ
 */
//-----------------------------------------------------------------------------
static	void	YT_YossyBirth(GAME_PARAM *gp,FALL_CHR_PARAM *fcp)
{
	YT_PLAYER_STATUS	*ps=(YT_PLAYER_STATUS *)&gp->ps[fcp->player_no];
	int					height;
	int					height_tbl[]={144-40,130-40,116-40,102-40,88-40,74-40,60-40,46-40,32-40};

	for(height=0;height<YT_HEIGHT_MAX;height++){
		if(ps->stop[ps->stoptbl[fcp->line_no]][height]==fcp){
			break;
		}
	}
	ps->stop[ps->stoptbl[fcp->line_no]][height]=NULL;
	//さらに下のキャラがいた場合はつぶれを元に戻す
	if(height){
		{
			FALL_CHR_PARAM*	fcp_under;
			NET_PARAM*		pNet=gp->pNetParam;

			fcp_under=ps->stop[ps->stoptbl[fcp->line_no]][height-1];
			YT_AnmSeqSet(fcp_under,YT_ANM_STOP,pNet);
		}
	}
    {
        u8 posx = 16+24*fcp->line_no+128*fcp->player_no;
        u8 posy = height_tbl[height];
        YT_YossyBirthAnimeTaskSet(gp,ps,posx,posy,fcp->chr_count);
        YT_NetSendYossyBirthAnime(posx,posy,fcp->chr_count,gp->pNetParam);
    }
    
	//タマゴの上にキャラがいた場合は、連鎖落下シーケンスをセット
	{
		int				count=0;
		FALL_CHR_PARAM*	fcp_rensa;
		NET_PARAM*		pNet=gp->pNetParam;

		for(;height<YT_HEIGHT_MAX;height++){
			fcp_rensa=ps->stop[ps->stoptbl[fcp->line_no]][height];
			if(fcp_rensa){
				ps->stop[ps->stoptbl[fcp->line_no]][height]=NULL;
				if(count==0){
					fcp_rensa->seq_no=SEQ_FALL_CHR_RENSA_FALL_P;
				}
				else{
					fcp_rensa->seq_no=SEQ_FALL_CHR_RENSA_FALL_C;
				}
				YT_AnmSeqSet(fcp_rensa,YT_ANM_STOP,pNet);
				ps->rensa[count++]=fcp_rensa;
			}
		}
		if(count){
			ps->status.rensa_flag=1;
		}
	}
}

void YT_YossyBirthAnimeTaskSet(GAME_PARAM *gp,YT_PLAYER_STATUS *ps,u8 pos_x,u8 pos_y,u8 count)
{
	YOSSY_BIRTH_ANIME	*yba=GFL_HEAP_AllocMemory(gp->heapID,sizeof(YOSSY_BIRTH_ANIME));

	yba->gp=gp;
	yba->ps=ps;
	yba->seq_no=0;
	yba->pat_no=0;
	yba->wait=0;
	yba->pos_x=pos_x;
	yba->pos_y=pos_y;
	yba->count=count;
	if(ps){
		ps->status.birth_flag=1;
	}
	GFL_TCB_AddTask(gp->tcbsys,YT_YossyBirthAnime,yba,TCB_PRI_PLAYER);
}

static	void	YT_YossyBirthAnime(GFL_TCB *tcb,void *work)
{
	YOSSY_BIRTH_ANIME	*yba=(YOSSY_BIRTH_ANIME *)work;

	switch(yba->seq_no){
	case 0:
		if(yba->wait==0){
			yba->wait=4;
			GFL_BMP_Fill(GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMP_Print(yba->gp->yossy_bmp,GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),
						  0,0+48*yba->pat_no,yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMPWIN_TransVramCharacter(yba->gp->yossy_bmpwin);
			yba->pat_no++;
			if((yba->pat_no>yba->count)||(yba->pat_no==19)){
				yba->pat_no--;
				yba->wait=32;
				yba->seq_no++;
			}
		}
		else{
			yba->wait--;
		}
		break;
	case 1:
	case 2:
		if(yba->wait==0){
			if(yba->seq_no==1){
				GFL_SOUND_PlaySE(SE_YOSSY);
			}
			yba->wait=8;
			GFL_BMP_Fill(GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMP_Print(yba->gp->yossy_bmp,GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),
						  0+32*yba->seq_no,0+48*yba->pat_no,yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMPWIN_TransVramCharacter(yba->gp->yossy_bmpwin);
			yba->seq_no++;
		}
		else{
			yba->wait--;
		}
		break;
	case 3:
	case 4:
		if(yba->wait==0){
			yba->wait=8;
			GFL_BMP_Fill(GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMP_Print(yba->gp->yossy_bmp,GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),
						  0+32*(4-yba->seq_no),0+48*yba->pat_no,yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMPWIN_TransVramCharacter(yba->gp->yossy_bmpwin);
			yba->seq_no++;
		}
		else{
			yba->wait--;
		}
		break;
	case 5:
		if(yba->wait==0){
			GFL_BMP_Fill(GFL_BMPWIN_GetBmp(yba->gp->yossy_bmpwin),yba->pos_x,yba->pos_y,32,48,0);
			GFL_BMPWIN_TransVramCharacter(yba->gp->yossy_bmpwin);
			if(yba->ps){
				yba->ps->status.birth_flag=0;
			}
			GFL_HEAP_FreeMemory(yba);
			GFL_TCB_DeleteTask(tcb);
		}
		else{
			yba->wait--;
		}
		break;
	}
}

