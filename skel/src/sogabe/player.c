
//============================================================================================
/**
 * @file	player.c
 * @brief	DS版ヨッシーのたまご　プレーヤープログラム
 * @author	sogabe
 * @date	2007.02.08
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"
#include "fallchr.h"

#include "sample_graphic/yossyegg.naix"

#define __PLAYER_H_GLOBAL__
#include "player.h"

#define	PAD_BUTTON_ROTATE (PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_X|PAD_BUTTON_Y)

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーパラメータ構造体
 */
//-----------------------------------------------------------------------------
struct _PLAYER_PARAM {
	GAME_PARAM			*gp;
	u8					seq_no;			//シーケンスナンバー
	u8					player_no;		//プレーヤーナンバー
	u8					type;			//プレーヤーの種類
	u8					line_no;		//現在いるライン
	u8					dir;			//キャラクタの向き
	u8					anm_no;
	u8					pat_no;
	u8					anm_wait;
	void				*chr_data;
	NNSG2dCharacterData *nns_char_data;
    NET_PARAM           *pNetParam;   //通信用構造体
};

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー描画用データテーブル
 */
//-----------------------------------------------------------------------------
//スクリーンアドレス
static	const	u16	PlayerScreenAdrs[][3]={
	//Player1
	{(0x13*0x20+0x03),(0x13*0x20+0x06),(0x13*0x20+0x09)},
	//Player2
	{(0x13*0x20+0x13),(0x13*0x20+0x16),(0x13*0x20+0x19)},
};

static const	u16	PlayerClearScreenData[]={
	0x2002,0x2003,0x2001,0x2002,
	0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,
};

static	const	u16	PlayerChrTransAdrs[]={
	0x20,0x30,
};

#define	YT_ANIME_STOP		(0xffff)
#define	YT_ANIME_LOOP		(0xfffe)
#define	YT_PLAYER_CHR_SIZE	(0x0200)

#define	YT_PLAYER_CF_0			(YT_PLAYER_CHR_SIZE*0x00)
#define	YT_PLAYER_CF2B_0		(YT_PLAYER_CHR_SIZE*0x01)
#define	YT_PLAYER_CF2B_1		(YT_PLAYER_CHR_SIZE*0x02)
#define	YT_PLAYER_CF2B_2		(YT_PLAYER_CHR_SIZE*0x03)
#define	YT_PLAYER_CB_0			(YT_PLAYER_CHR_SIZE*0x04)
#define	YT_PLAYER_CB2F_0		(YT_PLAYER_CHR_SIZE*0x05)
#define	YT_PLAYER_CB2F_1		(YT_PLAYER_CHR_SIZE*0x06)
#define	YT_PLAYER_CB2F_2		(YT_PLAYER_CHR_SIZE*0x07)

#define	YT_PLAYER_RF_0			(YT_PLAYER_CHR_SIZE*0x08)
#define	YT_PLAYER_RF2B_0		(YT_PLAYER_CHR_SIZE*0x09)
#define	YT_PLAYER_RF2B_1		(YT_PLAYER_CHR_SIZE*0x0a)
#define	YT_PLAYER_RF2B_2		(YT_PLAYER_CHR_SIZE*0x0b)
#define	YT_PLAYER_RB_0			(YT_PLAYER_CHR_SIZE*0x0c)
#define	YT_PLAYER_RB2F_0		(YT_PLAYER_CHR_SIZE*0x0d)
#define	YT_PLAYER_RB2F_1		(YT_PLAYER_CHR_SIZE*0x0e)
#define	YT_PLAYER_RB2F_2		(YT_PLAYER_CHR_SIZE*0x0f)

#define	YT_PLAYER_LF_0			(YT_PLAYER_CHR_SIZE*0x10)
#define	YT_PLAYER_LF2B_0		(YT_PLAYER_CHR_SIZE*0x11)
#define	YT_PLAYER_LF2B_1		(YT_PLAYER_CHR_SIZE*0x12)
#define	YT_PLAYER_LF2B_2		(YT_PLAYER_CHR_SIZE*0x13)
#define	YT_PLAYER_LB_0			(YT_PLAYER_CHR_SIZE*0x14)
#define	YT_PLAYER_LB2F_0		(YT_PLAYER_CHR_SIZE*0x15)
#define	YT_PLAYER_LB2F_1		(YT_PLAYER_CHR_SIZE*0x16)
#define	YT_PLAYER_LB2F_2		(YT_PLAYER_CHR_SIZE*0x17)

#define	YT_PLAYER_CF_1			(YT_PLAYER_CHR_SIZE*0x18)
#define	YT_PLAYER_CF_2			(YT_PLAYER_CHR_SIZE*0x19)
#define	YT_PLAYER_CB_1			(YT_PLAYER_CHR_SIZE*0x1a)
#define	YT_PLAYER_CB_2			(YT_PLAYER_CHR_SIZE*0x1b)
#define	YT_PLAYER_RF_1			(YT_PLAYER_CHR_SIZE*0x1c)
#define	YT_PLAYER_RF_2			(YT_PLAYER_CHR_SIZE*0x1d)
#define	YT_PLAYER_RB_1			(YT_PLAYER_CHR_SIZE*0x1e)
#define	YT_PLAYER_RB_2			(YT_PLAYER_CHR_SIZE*0x1f)
#define	YT_PLAYER_LF_1			(YT_PLAYER_CHR_SIZE*0x20)
#define	YT_PLAYER_LF_2			(YT_PLAYER_CHR_SIZE*0x21)
#define	YT_PLAYER_LB_1			(YT_PLAYER_CHR_SIZE*0x22)
#define	YT_PLAYER_LB_2			(YT_PLAYER_CHR_SIZE*0x23)

#define	YT_PLAYER_CF_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x24)
#define	YT_PLAYER_CF_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x25)
#define	YT_PLAYER_CF_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x26)
#define	YT_PLAYER_CB_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x27)
#define	YT_PLAYER_CB_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x28)
#define	YT_PLAYER_CB_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x29)
#define	YT_PLAYER_RF_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x2a)
#define	YT_PLAYER_RF_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x2b)
#define	YT_PLAYER_RF_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x2c)
#define	YT_PLAYER_RB_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x2d)
#define	YT_PLAYER_RB_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x2e)
#define	YT_PLAYER_RB_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x2f)
#define	YT_PLAYER_LF_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x30)
#define	YT_PLAYER_LF_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x31)
#define	YT_PLAYER_LF_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x32)
#define	YT_PLAYER_LB_PUNCH_C	(YT_PLAYER_CHR_SIZE*0x33)
#define	YT_PLAYER_LB_PUNCH_R	(YT_PLAYER_CHR_SIZE*0x34)
#define	YT_PLAYER_LB_PUNCH_L	(YT_PLAYER_CHR_SIZE*0x35)

#define	YT_PLAYER_LOSE_0		(YT_PLAYER_CHR_SIZE*0x36)
#define	YT_PLAYER_LOSE_1		(YT_PLAYER_CHR_SIZE*0x37)
#define	YT_PLAYER_WIN_0			(YT_PLAYER_CHR_SIZE*0x38)
#define	YT_PLAYER_WIN_1			(YT_PLAYER_CHR_SIZE*0x39)

typedef struct{
	u16	pat_adrs;
	u16	wait;
}YT_ANIME_TABLE;

enum{
	YT_PLAYER_ANM_LF=0,
	YT_PLAYER_ANM_CF,
	YT_PLAYER_ANM_RF,
	YT_PLAYER_ANM_LB,
	YT_PLAYER_ANM_CB,
	YT_PLAYER_ANM_RB,
	YT_PLAYER_ANM_LF2B,
	YT_PLAYER_ANM_CF2B,
	YT_PLAYER_ANM_RF2B,
	YT_PLAYER_ANM_LB2F,
	YT_PLAYER_ANM_CB2F,
	YT_PLAYER_ANM_RB2F,
	YT_PLAYER_ANM_LF_PUNCH_L,
	YT_PLAYER_ANM_LF_PUNCH_C,
	YT_PLAYER_ANM_LF_PUNCH_R,
	YT_PLAYER_ANM_CF_PUNCH_L,
	YT_PLAYER_ANM_CF_PUNCH_C,
	YT_PLAYER_ANM_CF_PUNCH_R,
	YT_PLAYER_ANM_RF_PUNCH_L,
	YT_PLAYER_ANM_RF_PUNCH_C,
	YT_PLAYER_ANM_RF_PUNCH_R,
	YT_PLAYER_ANM_LB_PUNCH_L,
	YT_PLAYER_ANM_LB_PUNCH_C,
	YT_PLAYER_ANM_LB_PUNCH_R,
	YT_PLAYER_ANM_CB_PUNCH_L,
	YT_PLAYER_ANM_CB_PUNCH_C,
	YT_PLAYER_ANM_CB_PUNCH_R,
	YT_PLAYER_ANM_RB_PUNCH_L,
	YT_PLAYER_ANM_RB_PUNCH_C,
	YT_PLAYER_ANM_RB_PUNCH_R,
	YT_PLAYER_ANM_WIN,
	YT_PLAYER_ANM_LOSE,
	YT_PLAYER_ANM_MAX
};

#define	YT_ANIME_LOOP_WAIT	(16)	//繰り返し系のWAIT
#define	YT_ANIME_TURN_WAIT	(2)		//回る系のWAIT
#define	YT_ANIME_PUNCH_WAIT	(4)		//パンチ系のWAIT

//	YT_PLAYER_ANM_LF
static	const	YT_ANIME_TABLE	yt_player_anm_lf[]={
	{YT_PLAYER_LF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LF_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LF_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_CF
static	const	YT_ANIME_TABLE	yt_player_anm_cf[]={
	{YT_PLAYER_CF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CF_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CF_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_RF
static	const	YT_ANIME_TABLE	yt_player_anm_rf[]={
	{YT_PLAYER_RF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RF_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RF_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RF_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_LB
static	const	YT_ANIME_TABLE	yt_player_anm_lb[]={
	{YT_PLAYER_LB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LB_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_LB_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_CB
static	const	YT_ANIME_TABLE	yt_player_anm_cb[]={
	{YT_PLAYER_CB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CB_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_CB_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_RB
static	const	YT_ANIME_TABLE	yt_player_anm_rb[]={
	{YT_PLAYER_RB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RB_1,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RB_0,YT_ANIME_LOOP_WAIT},
	{YT_PLAYER_RB_2,YT_ANIME_LOOP_WAIT},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_LF2B
static	const	YT_ANIME_TABLE	yt_player_anm_lf2b[]={
	{YT_PLAYER_LF2B_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_LF2B_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_LF2B_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CF2B
static	const	YT_ANIME_TABLE	yt_player_anm_cf2b[]={
	{YT_PLAYER_CF2B_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_CF2B_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_CF2B_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RF2B
static	const	YT_ANIME_TABLE	yt_player_anm_rf2b[]={
	{YT_PLAYER_RF2B_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_RF2B_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_RF2B_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LB2F
static	const	YT_ANIME_TABLE	yt_player_anm_lb2f[]={
	{YT_PLAYER_LB2F_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_LB2F_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_LB2F_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CB2F
static	const	YT_ANIME_TABLE	yt_player_anm_cb2f[]={
	{YT_PLAYER_CB2F_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_CB2F_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_CB2F_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RB2F
static	const	YT_ANIME_TABLE	yt_player_anm_rb2f[]={
	{YT_PLAYER_RB2F_0,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_RB2F_1,YT_ANIME_TURN_WAIT},
	{YT_PLAYER_RB2F_2,YT_ANIME_TURN_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LF_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_lf_punch_l[]={
	{YT_PLAYER_LF_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LF_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_lf_punch_c[]={
	{YT_PLAYER_LF_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LF_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_lf_punch_r[]={
	{YT_PLAYER_LF_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CF_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_cf_punch_l[]={
	{YT_PLAYER_CF_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CF_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_cf_punch_c[]={
	{YT_PLAYER_CF_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CF_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_cf_punch_r[]={
	{YT_PLAYER_CF_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RF_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_rf_punch_l[]={
	{YT_PLAYER_RF_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RF_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_rf_punch_c[]={
	{YT_PLAYER_RF_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RF_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_rf_punch_r[]={
	{YT_PLAYER_RF_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LB_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_lb_punch_l[]={
	{YT_PLAYER_LB_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LB_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_lb_punch_c[]={
	{YT_PLAYER_LB_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_LB_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_lb_punch_r[]={
	{YT_PLAYER_LB_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CB_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_cb_punch_l[]={
	{YT_PLAYER_CB_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CB_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_cb_punch_c[]={
	{YT_PLAYER_CB_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_CB_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_cb_punch_r[]={
	{YT_PLAYER_CB_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RB_PUNCH_L
static	const	YT_ANIME_TABLE	yt_player_anm_rb_punch_l[]={
	{YT_PLAYER_CB_PUNCH_L,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RB_PUNCH_C
static	const	YT_ANIME_TABLE	yt_player_anm_rb_punch_c[]={
	{YT_PLAYER_RB_PUNCH_C,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_RB_PUNCH_R
static	const	YT_ANIME_TABLE	yt_player_anm_rb_punch_r[]={
	{YT_PLAYER_RB_PUNCH_R,YT_ANIME_PUNCH_WAIT},
	{0,YT_ANIME_STOP},
};
//	YT_PLAYER_ANM_WIN
static	const	YT_ANIME_TABLE	yt_player_anm_win[]={
	{YT_PLAYER_WIN_1,12},
	{YT_PLAYER_WIN_0,12},
	{YT_PLAYER_WIN_1,12},
	{YT_PLAYER_WIN_0,12},
	{YT_PLAYER_WIN_1,12},
	{YT_PLAYER_WIN_0,32},
	{0,YT_ANIME_LOOP},
};
//	YT_PLAYER_ANM_LOSE
static	const	YT_ANIME_TABLE	yt_player_anm_lose[]={
	{YT_PLAYER_LOSE_0,8},
	{YT_PLAYER_LOSE_1,8},
	{YT_PLAYER_LOSE_0,8},
	{YT_PLAYER_LOSE_1,8},
	{YT_PLAYER_LOSE_0,8},
	{YT_PLAYER_LOSE_1,32},
	{0,YT_ANIME_LOOP},
};

static	const	YT_ANIME_TABLE	* const yt_anime_table[]={
//	YT_PLAYER_ANM_LF
	yt_player_anm_lf,
//	YT_PLAYER_ANM_CF
	yt_player_anm_cf,
//	YT_PLAYER_ANM_RF
	yt_player_anm_rf,
//	YT_PLAYER_ANM_LB
	yt_player_anm_lb,
//	YT_PLAYER_ANM_CB
	yt_player_anm_cb,
//	YT_PLAYER_ANM_RB
	yt_player_anm_rb,
//	YT_PLAYER_ANM_LF2B
	yt_player_anm_lf2b,
//	YT_PLAYER_ANM_CF2B
	yt_player_anm_cf2b,
//	YT_PLAYER_ANM_RF2B
	yt_player_anm_rf2b,
//	YT_PLAYER_ANM_LB2F
	yt_player_anm_lb2f,
//	YT_PLAYER_ANM_CB2F
	yt_player_anm_cb2f,
//	YT_PLAYER_ANM_RB2F
	yt_player_anm_rb2f,
//	YT_PLAYER_ANM_LF_PUNCH_L
	yt_player_anm_lf_punch_l,
//	YT_PLAYER_ANM_LF_PUNCH_C
	yt_player_anm_lf_punch_c,
//	YT_PLAYER_ANM_LF_PUNCH_R
	yt_player_anm_lf_punch_r,
//	YT_PLAYER_ANM_CF_PUNCH_L
	yt_player_anm_cf_punch_l,
//	YT_PLAYER_ANM_CF_PUNCH_C
	yt_player_anm_cf_punch_c,
//	YT_PLAYER_ANM_CF_PUNCH_R
	yt_player_anm_cf_punch_r,
//	YT_PLAYER_ANM_RF_PUNCH_L
	yt_player_anm_rf_punch_l,
//	YT_PLAYER_ANM_RF_PUNCH_C
	yt_player_anm_rf_punch_c,
//	YT_PLAYER_ANM_RF_PUNCH_R
	yt_player_anm_rf_punch_r,
//	YT_PLAYER_ANM_LB_PUNCH_L
	yt_player_anm_lb_punch_l,
//	YT_PLAYER_ANM_LB_PUNCH_C
	yt_player_anm_lb_punch_c,
//	YT_PLAYER_ANM_LB_PUNCH_R
	yt_player_anm_lb_punch_r,
//	YT_PLAYER_ANM_CB_PUNCH_L
	yt_player_anm_cb_punch_l,
//	YT_PLAYER_ANM_CB_PUNCH_C
	yt_player_anm_cb_punch_c,
//	YT_PLAYER_ANM_CB_PUNCH_R
	yt_player_anm_cb_punch_r,
//	YT_PLAYER_ANM_RB_PUNCH_L
	yt_player_anm_rb_punch_l,
//	YT_PLAYER_ANM_RB_PUNCH_C
	yt_player_anm_rb_punch_c,
//	YT_PLAYER_ANM_RB_PUNCH_R
	yt_player_anm_rb_punch_r,
//	YT_PLAYER_ANM_WIN
	yt_player_anm_win,
//	YT_PLAYER_ANM_LOSE
	yt_player_anm_lose,
};

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーアクションナンバー
 */
//-----------------------------------------------------------------------------
enum{
	YT_PLAYER_ACT_MOVE_L=0,
	YT_PLAYER_ACT_MOVE_R,
	YT_PLAYER_ACT_ROTATE,
	YT_PLAYER_ACT_OVERTURN_L,
	YT_PLAYER_ACT_OVERTURN_R,
	YT_PLAYER_ACT_OVERTURN_C,
	YT_PLAYER_ACT_MAX
};

static	void	YT_MainPlayer(GFL_TCB *tcb,void *work);
static	u8		YT_PlayerActGet(PLAYER_PARAM *pp);
static	void	YT_PlayerAnimeSet(PLAYER_PARAM *pp,int anm_no);
static	void	YT_PlayerAnimeMain(PLAYER_PARAM *pp);
static	BOOL	YT_PlayerAnimeCheck(PLAYER_PARAM *pp);
static	void	YT_PlayerChrTrans(PLAYER_PARAM *pp);
static	void	YT_PlayerScreenMake(PLAYER_PARAM *pp,u8 old_line_no,u8 new_line_no);
static	void	YT_PlayerRotateScreenMake(PLAYER_PARAM *pp,u8 flag);
static	void	YT_PlayerRotateAct(PLAYER_PARAM *pp,YT_PLAYER_STATUS *ps);
static	void	YT_PlayerOverTurnAct(PLAYER_PARAM *pp,YT_PLAYER_STATUS *ps,int flag);

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー初期化
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		プレーヤータイプ
 *  @retval PLAYER_PARAM
 */
//-----------------------------------------------------------------------------
PLAYER_PARAM* YT_InitPlayer(GAME_PARAM *gp,u8 player_no,u8 type,u8 bNetSend)
{
	PLAYER_PARAM *pp=(PLAYER_PARAM *)GFL_HEAP_AllocMemory(gp->heapID,sizeof(PLAYER_PARAM));

	pp->gp=gp;
	pp->seq_no=0;
	pp->player_no=player_no;
	pp->type=type;
	pp->line_no=1;
	pp->dir=0;
    pp->pNetParam = (bNetSend==TRUE) ? gp->pNetParam : NULL;

	//キャラクタデータ展開
	pp->chr_data=GFL_ARC_UTIL_LoadCharacter(0,NARC_yossyegg_YT_MARIO_NCGR,0,&pp->nns_char_data,gp->heapID);

	YT_PlayerScreenMake(pp,pp->line_no,pp->line_no);

	YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no);
    return pp;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーをタスクに登録
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	type		プレーヤータイプ
 *	@param	player_no	1P or 2P
 *  @retval void
 */
//-----------------------------------------------------------------------------
void YT_InitPlayerAddTask(GAME_PARAM *gp, PLAYER_PARAM* pp,u8 player_no)
{
    gp->ps[player_no].tcb_player = GFL_TCB_AddTask(gp->tcbsys,
                                                   YT_MainPlayer,pp,
                                                   TCB_PRI_PLAYER);
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー初期化
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		プレーヤータイプ
 */
//-----------------------------------------------------------------------------
enum{
	SEQ_PLAYER_ACT_CHECK=0,		//プレーヤーの動作チェック
	SEQ_PLAYER_ROTATE,
	SEQ_PLAYER_OVERTURN,
	SEQ_PLAYER_WIN_INIT,
	SEQ_PLAYER_LOSE_INIT,
	SEQ_PLAYER_WIN_LOSE,

	OVER_TURN_L=1,
	OVER_TURN_R=2,
};
static	void	YT_MainPlayer(GFL_TCB *tcb,void *work)
{
	PLAYER_PARAM		*pp=(PLAYER_PARAM *)work;
	YT_PLAYER_STATUS	*ps=&pp->gp->ps[pp->player_no];
    int actno=0;

	YT_PlayerAnimeMain(pp);

	if(pp->seq_no!=SEQ_PLAYER_WIN_LOSE){
		switch(ps->status.win_lose_flag){
		case YT_GAME_WIN:
			pp->seq_no=SEQ_PLAYER_WIN_INIT;
			GFL_SOUND_PlayBGM(SEQ_CLEAR);
			break;
		case YT_GAME_LOSE:
		case YT_GAME_DRAW:
			pp->seq_no=SEQ_PLAYER_LOSE_INIT;
			GFL_SOUND_PlayBGM(SEQ_GAMEOVER);
			break;
		}
	}

	switch(pp->seq_no){
	case SEQ_PLAYER_ACT_CHECK:
        actno = YT_PlayerActGet(pp);
		switch(actno){
		case YT_PLAYER_ACT_MOVE_L:
			if(pp->line_no){
				YT_PlayerScreenMake(pp,pp->line_no,pp->line_no-1);
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
			}
			break;
		case YT_PLAYER_ACT_MOVE_R:
			if(pp->line_no<2){
				YT_PlayerScreenMake(pp,pp->line_no,pp->line_no+1);
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
			}
			break;
		case YT_PLAYER_ACT_ROTATE:
			if(ps->status.no_active_flag==0){
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF2B+pp->line_no+3*pp->dir);
				YT_PlayerRotateScreenMake(pp,0);
				pp->seq_no=SEQ_PLAYER_ROTATE;
				YT_NET_PlaySE(SE_ROTATE,pp->pNetParam);
				YT_PlayerRotateAct(pp,ps);
			}
			break;
		case YT_PLAYER_ACT_OVERTURN_L:
			if(ps->status.no_active_flag==0){
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF_PUNCH_L+3*pp->line_no+9*pp->dir);
				pp->seq_no=SEQ_PLAYER_OVERTURN;
				YT_NET_PlaySE(SE_OVERTURN,pp->pNetParam);
				YT_PlayerOverTurnAct(pp,ps,OVER_TURN_L);
			}
			break;
		case YT_PLAYER_ACT_OVERTURN_R:
			if(ps->status.no_active_flag==0){
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF_PUNCH_R+3*pp->line_no+9*pp->dir);
				pp->seq_no=SEQ_PLAYER_OVERTURN;
				YT_NET_PlaySE(SE_OVERTURN,pp->pNetParam);
				YT_PlayerOverTurnAct(pp,ps,OVER_TURN_R);
			}
			break;
		case YT_PLAYER_ACT_OVERTURN_C:
			if(ps->status.no_active_flag==0){
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF_PUNCH_C+3*pp->line_no+9*pp->dir);
				pp->seq_no=SEQ_PLAYER_OVERTURN;
				YT_NET_PlaySE(SE_OVERTURN,pp->pNetParam);
				YT_PlayerOverTurnAct(pp,ps,OVER_TURN_L|OVER_TURN_R);
			}
			break;
		default:
			break;
		}
		break;
	case SEQ_PLAYER_ROTATE:
		switch(YT_PlayerActGet(pp)){
		case YT_PLAYER_ACT_MOVE_L:
			if(pp->line_no){
				pp->dir^=1;
				YT_PlayerRotateScreenMake(pp,1);
				YT_PlayerScreenMake(pp,pp->line_no,pp->line_no-1);
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
				pp->seq_no=SEQ_PLAYER_ACT_CHECK;
			}
			break;
		case YT_PLAYER_ACT_MOVE_R:
			if(pp->line_no<2){
				pp->dir^=1;
				YT_PlayerRotateScreenMake(pp,1);
				YT_PlayerScreenMake(pp,pp->line_no,pp->line_no+1);
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
				pp->seq_no=SEQ_PLAYER_ACT_CHECK;
			}
			break;
		default:
			if(YT_PlayerAnimeCheck(pp)==FALSE){
				pp->dir^=1;
				YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
				YT_PlayerRotateScreenMake(pp,1);
				pp->seq_no=SEQ_PLAYER_ACT_CHECK;
			}
		}
		break;
	case SEQ_PLAYER_OVERTURN:
		if(YT_PlayerAnimeCheck(pp)==FALSE){
			YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LF+pp->line_no+3*pp->dir);
			pp->seq_no=SEQ_PLAYER_ACT_CHECK;
		}
		break;
	case SEQ_PLAYER_WIN_INIT:
			YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_WIN);
			pp->seq_no=SEQ_PLAYER_WIN_LOSE;
		break;
	case SEQ_PLAYER_LOSE_INIT:
			YT_PlayerAnimeSet(pp,YT_PLAYER_ANM_LOSE);
			pp->seq_no=SEQ_PLAYER_WIN_LOSE;
		break;
	case SEQ_PLAYER_WIN_LOSE:
		if(pp->gp->seq_no==SEQ_GAME_TO_TITLE){
			GFL_HEAP_FreeMemory(pp->chr_data);
			GFL_HEAP_FreeMemory(work);
			GFL_TCB_DeleteTask(tcb);
		}
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー動作情報を取得
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 */
//-----------------------------------------------------------------------------
static	u8		YT_PlayerActGet(PLAYER_PARAM *pp)
{
	//キーから動作を選択
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		return YT_PLAYER_ACT_MOVE_L;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		return YT_PLAYER_ACT_MOVE_R;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_ROTATE ){
		return YT_PLAYER_ACT_ROTATE;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		return YT_PLAYER_ACT_OVERTURN_L;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		return YT_PLAYER_ACT_OVERTURN_R;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		return YT_PLAYER_ACT_OVERTURN_C;
	}
	return YT_PLAYER_ACT_MAX;
}



//----------------------------------------------------------------------------
/**
 *	@brief	通信できたプレーヤーアニメをセット
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	anm_no	セットするアニメナンバー
 */
//-----------------------------------------------------------------------------
void YT_NetPlayerChrTrans(GAME_PARAM *gp,PLAYER_PARAM *pp,int player_no,int anm_no,int line_no,int rot,int pat_no)
{
	const	YT_ANIME_TABLE	* yat=yt_anime_table[anm_no];

	pp->anm_no=anm_no;
	pp->pat_no=pat_no;
	pp->anm_wait=yat[0].wait;
    pp->gp=gp;
    pp->seq_no=0;
    pp->player_no = player_no;
    pp->type = player_no;
    pp->line_no = line_no;
    pp->dir = rot;
	YT_PlayerChrTrans(pp);
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーアニメをセット
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	anm_no	セットするアニメナンバー
 */
//-----------------------------------------------------------------------------
static void	YT_PlayerAnimeSet(PLAYER_PARAM *pp,int anm_no)
{
	const	YT_ANIME_TABLE	* yat=yt_anime_table[anm_no];

	pp->anm_no=anm_no;
	pp->pat_no=0;
	pp->anm_wait=yat[0].wait;

	YT_PlayerChrTrans(pp);
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーアニメメイン処理
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 */
//-----------------------------------------------------------------------------
static	void	YT_PlayerAnimeMain(PLAYER_PARAM *pp)
{
	if(pp->anm_no<YT_PLAYER_ANM_MAX){
		if(pp->anm_wait){
			pp->anm_wait--;
		}
		else{
			{
				const	YT_ANIME_TABLE	* yat=yt_anime_table[pp->anm_no];

				pp->pat_no++;
				switch(yat[pp->pat_no].wait){
				//アニメ終了
				case YT_ANIME_STOP:
					pp->anm_no=YT_PLAYER_ANM_MAX;
					break;
				//アニメループ
				case YT_ANIME_LOOP:
					pp->pat_no=0;
				default:
					pp->anm_wait=yat[pp->pat_no].wait;
					YT_PlayerChrTrans(pp);
					break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーアニメが動作しているかチェック
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 *
 *	@retval	TRUE:動作中　FALSE:終了
 */
//-----------------------------------------------------------------------------
static	BOOL	YT_PlayerAnimeCheck(PLAYER_PARAM *pp)
{
	return (pp->anm_no<YT_PLAYER_ANM_MAX);
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤーキャラ転送
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 *
 *	@retval	TRUE:動作中　FALSE:終了
 */
//-----------------------------------------------------------------------------
static	void	YT_PlayerChrTrans(PLAYER_PARAM *pp)
{
	const	YT_ANIME_TABLE	* yat=yt_anime_table[pp->anm_no];
	u8		*buf=pp->nns_char_data->pRawData;
	
	buf+=yat[pp->pat_no].pat_adrs;

	GFL_BG_LoadCharacter( YT_PLAYER_FRAME, buf, YT_PLAYER_CHR_SIZE, PlayerChrTransAdrs[pp->player_no] );

    // 通信のときは場所を送る
    if(pp->pNetParam){
        YT_NET_SendPlayerAnmReq(pp->player_no, pp->anm_no, pp->pat_no, pp->line_no, pp->dir, pp->pNetParam);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー表示用スクリーン生成
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 *	@param	old_line_no	現在のプレーヤーのライン位置
 *	@param	new_line_no	新たなプレーヤーのライン位置
 */
//-----------------------------------------------------------------------------
static void	YT_PlayerScreenMake(PLAYER_PARAM *pp,u8 old_line_no,u8 new_line_no)
{
	int	x,y;
	u16	chr_no=0;
	u16	*scr_adrs=(u16 *)GFL_BG_GetScreenBufferAdrs( YT_PLAYER_FRAME );

	if(old_line_no!=new_line_no){
		//現在位置のスクリーンデータを消去
		for(y=0;y<4;y++){
			for(x=0;x<4;x++){
				scr_adrs[PlayerScreenAdrs[pp->player_no][old_line_no]+y*0x20+x]=PlayerClearScreenData[chr_no];
				chr_no++;
			}
		}
	}

	chr_no=(YT_PLAYER_CHRNO+0x10*pp->player_no)|YT_PLAYER_PALNO;

	//スクリーンデータを生成
	for(y=0;y<4;y++){
		for(x=0;x<4;x++){
			scr_adrs[PlayerScreenAdrs[pp->player_no][new_line_no]+y*0x20+x]=chr_no;
			chr_no++;
		}
	}

	GFL_BG_LoadScreenV_Req( YT_PLAYER_FRAME );

	pp->line_no=new_line_no;

    // 通信の時には送る
    if(pp->pNetParam){
        YT_NET_SendPlayerScreenMake(pp->player_no, old_line_no, new_line_no, pp->pNetParam);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	通信用 プレーヤー表示用スクリーン生成
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 *	@param	old_line_no	現在のプレーヤーのライン位置
 *	@param	new_line_no	新たなプレーヤーのライン位置
 */
//-----------------------------------------------------------------------------

void YT_PlayerScreenMakeNetFunc(PLAYER_PARAM *pp, u8 player_no, u8 old_line_no,u8 new_line_no)
{
    pp->player_no = player_no;
    YT_PlayerScreenMake(pp, old_line_no, new_line_no);
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー表示用スクリーン生成（ターンするとき）
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	flag	0:ターン開始時　1:ターン終了時
 */
//-----------------------------------------------------------------------------
static	void	YT_PlayerRotateScreenMake(PLAYER_PARAM *pp,u8 flag)
{
	u16	*scr_adrs=(u16 *)GFL_BG_GetScreenBufferAdrs( YT_PLAYER_FRAME );

	if(flag){
		scr_adrs[PlayerScreenAdrs[pp->player_no][pp->line_no]-1]=0x2001;
		scr_adrs[PlayerScreenAdrs[pp->player_no][pp->line_no]+4]=0x2003;
	}
	else{
		scr_adrs[PlayerScreenAdrs[pp->player_no][pp->line_no]-1]=0;
		scr_adrs[PlayerScreenAdrs[pp->player_no][pp->line_no]+4]=0;
	}

	GFL_BG_LoadScreenV_Req( YT_PLAYER_FRAME );

    // 通信の時には送る
    if(pp->pNetParam){
        YT_NET_SendPlayerRotateScreenMake(pp->player_no, pp->line_no,flag, pp->pNetParam);
    }

}


//----------------------------------------------------------------------------
/**
 *	@brief	通信用 プレーヤー表示用スクリーン生成（ターンするとき）
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	flag	0:ターン開始時　1:ターン終了時
 */
//-----------------------------------------------------------------------------
void	YT_PlayerRotateScreenMakeNetFunc(PLAYER_PARAM *pp, u8 player_no, u8 line_no, u8 flag)
{
    pp->player_no = player_no;
    pp->line_no = line_no;

    YT_PlayerRotateScreenMake(pp,flag);
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転処理
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	ps		プレーヤーステータス
 */
//-----------------------------------------------------------------------------
static	void	YT_PlayerRotateAct(PLAYER_PARAM *pp,YT_PLAYER_STATUS *ps)
{
	int				i;
	int				left_line;
	int				right_line;
	int				left_height=0;
	int				right_height=0;
	FALL_CHR_PARAM	*fcp;
	int				height_tbl[]={144,130,116,102,88,74,60,46,32};
	GFL_CLACTPOS	pos;

	ps->status.rotate_flag=pp->line_no+1;

	left_line=ps->stoptbl[pp->line_no];
	right_line=ps->stoptbl[pp->line_no+1];
	ps->stoptbl[pp->line_no+1]=left_line;
	ps->stoptbl[pp->line_no]=right_line;

	for(i=0;i<YT_HEIGHT_MAX;i++){
		fcp=ps->stop[left_line][i];
		if(fcp){
			fcp->rotate_flag=ps->status.rotate_flag;
			fcp->wait_value=i;
			left_height++;
		}
		fcp=ps->stop[right_line][i];
		if(fcp){
			fcp->rotate_flag=ps->status.rotate_flag;
			fcp->wait_value=i;
			right_height++;
		}
	}

	left_line=ps->falltbl[pp->line_no];
	right_line=ps->falltbl[pp->line_no+1];
	if(left_height!=right_height){
		if(left_height>right_height){
			for(i=0;i<YT_HEIGHT_MAX;i++){
				fcp=ps->fall[right_line][i];
				if(fcp){
					GFL_CLACT_WK_GetWldPos(fcp->clwk,&pos);
					if(pos.y>height_tbl[left_height]){
						fcp->rotate_flag=ps->status.rotate_flag;
						ps->falltbl[pp->line_no+1]=left_line;
						ps->falltbl[pp->line_no]=right_line;
						break;
					}
				}
			}
		}
		else{
			for(i=0;i<YT_HEIGHT_MAX;i++){
				fcp=ps->fall[left_line][i];
				if(fcp){
					GFL_CLACT_WK_GetWldPos(fcp->clwk,&pos);
					if(pos.y>height_tbl[right_height]){
						fcp->rotate_flag=ps->status.rotate_flag;
						ps->falltbl[pp->line_no+1]=left_line;
						ps->falltbl[pp->line_no]=right_line;
						break;
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ひっくり返し処理
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	ps		プレーヤーステータス
 *	@param	flag	ひっくり返すラインフラグ
 */
//-----------------------------------------------------------------------------
static	void	YT_PlayerOverTurnAct(PLAYER_PARAM *pp,YT_PLAYER_STATUS *ps,int flag)
{
	int				i;
	int				left_line;
	int				right_line;
	FALL_CHR_PARAM	*fcp;
	int				height_tbl[]={144,130,116,102,88,74,60,46,32};

	left_line=ps->stoptbl[pp->line_no];
	right_line=ps->stoptbl[pp->line_no+1];

	if(flag&OVER_TURN_L){
		ps->status.overturn_flag|=(1<<pp->line_no);
	}
	if(flag&OVER_TURN_R){
		ps->status.overturn_flag|=(1<<(pp->line_no+1));
	}

	for(i=0;i<YT_HEIGHT_MAX;i++){
		if(flag&OVER_TURN_L){
			fcp=ps->stop[left_line][i];
			if(fcp){
				fcp->overturn_flag=1;
				fcp->offset_pos.y=-1;
			}
		}
		if(flag&OVER_TURN_R){
			fcp=ps->stop[right_line][i];
			if(fcp){
				fcp->overturn_flag=1;
				fcp->offset_pos.y=-1;
			}
		}
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	勝ち負けフラグを立てる
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	bWin	勝った場合にTRUE
 */
//-----------------------------------------------------------------------------

void YT_PlayerSetWinLoseFlag(YT_PLAYER_STATUS *ps, int bWin)
{
    if(bWin){
        ps->status.win_lose_flag|=YT_GAME_WIN;
    }
    else{
        ps->status.win_lose_flag|=YT_GAME_LOSE;
    }
}

