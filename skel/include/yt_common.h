
//============================================================================================
/**
 * @file	yt_common.h
 * @brief	DS版ヨッシーのたまごcommonヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__YT_COMMON_H__
#define	__YT_COMMON_H__

#undef GLOBAL
#ifdef __TITLE_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

#include "sample_sound/yossy_egg.sadl"

//ジョブナンバー定義
enum{
	YT_InitTitleNo=0,
	YT_MainTitleNo,
	YT_InitGameNo,
	YT_MainGameNo,
	YT_InitMultiBootNo,
	YT_MainMultiBootNo,
	YT_Init3DTestNo,
	YT_Main3DTestNo,
	YT_InitBlockOutNo,
	YT_MainBlockOutNo,
};

//-------------------------------------
///	キャラクタナンバー
//=====================================
enum{
	YT_CHR_KURIBO=0,
	YT_CHR_PAKKUN,
	YT_CHR_GESSO,
	YT_CHR_TERESA,
	YT_CHR_DEKATERESA,
	YT_CHR_GREEN_EGG_U,
	YT_CHR_GREEN_EGG_D,
	YT_CHR_RED_EGG_U,
	YT_CHR_RED_EGG_D,
	YT_CHR_GREEN_EGG,
	YT_CHR_RED_EGG,
};

//-------------------------------------
///	キャラクタ毎の表示プライオリティー
//=====================================
enum{
	YT_PRI_KURIBO=40,
	YT_PRI_PAKKUN=40,
	YT_PRI_GESSO=40,
	YT_PRI_TERESA=40,
	YT_PRI_DEKATERESA=30,
	YT_PRI_GREEN_EGG_U=0,
	YT_PRI_GREEN_EGG_D=10,
	YT_PRI_RED_EGG_U=0,
	YT_PRI_RED_EGG_D=10,
	YT_PRI_GREEN_EGG=20,
	YT_PRI_RED_EGG=20,
};

//-------------------------------------
///	no_active_flagの内訳
//=====================================
#define	YT_ROTATE_FLAG			(0x0003)		//回転フラグ
#define	YT_OVERTURN_FLAG		(0x003c)		//ひっくり返しフラグ
#define	YT_EGG_MAKE_CHECK_FLAG	(0x03c0)		//タマゴ作成チェックフラグ
#define	YT_EGG_MAKE_FLAG		(0x0400)		//タマゴ作成フラグ
#define	YT_BIRTH_FLAG			(0x0800)		//ヨッシー生まれるフラグ
#define	YT_RENSA_FLAG			(0x1000)		//連鎖フラグ

//-------------------------------------
///	セルアクタープロセスワーク
//=====================================

#define	YT_CLACT_MAX	(128)
#define	YT_LINE_MAX		(4)
#define	YT_HEIGHT_MAX	(9)

typedef struct {
	void* p_cellbuff;
	void* p_cellanmbuff;
	NNSG2dCellDataBank*		p_cell;
	NNSG2dCellAnimBankData*	p_cellanm;
	NNSG2dImageProxy		imageproxy;
	NNSG2dImagePaletteProxy	plttproxy;
} YT_CLACT_RES;

typedef struct {
	GFL_CLUNIT		*p_unit;
	YT_CLACT_RES	res;
	GFL_CLWK		*clact_work[YT_CLACT_MAX];
} YT_CLACT;

typedef struct	fall_chr_param	FALL_CHR_PARAM;
//通信用構造体
typedef	struct  _NET_PARAM NET_PARAM;


typedef struct{
	GFL_TCB			*tcb_player;
	FALL_CHR_PARAM	*ready[YT_LINE_MAX][YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*fall[YT_LINE_MAX][YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*stop[YT_LINE_MAX][YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*rensa[YT_HEIGHT_MAX];
	u8				falltbl[YT_LINE_MAX];
	u8				stoptbl[YT_LINE_MAX];
	union{
		u16			no_active_flag;
		struct{
			u16		rotate_flag			:2;		//回転フラグ
			u16		overturn_flag		:4;		//ひっくり返しフラグ
			u16		egg_make_check_flag	:4;		//タマゴ作成チェックフラグ
			u16		egg_make_flag		:1;		//タマゴ作成フラグ
			u16		birth_flag			:1;		//ヨッシー生まれるフラグ
			u16		rensa_flag			:1;		//連鎖フラグ
			u16		win_lose_flag		:2;		//勝敗フラグ
			u16							:3;
		};
	}status;
	u8				fall_wait;
	u8				exist_flag			:1;		//プレイヤー存在フラグ
	u8									:7;
	int				egg_make_count;
	int				fall_count;
}YT_PLAYER_STATUS;

//ゲームパラメータ構造体宣言
typedef	struct
{
	int					heapID;
	int					job_no;			//ジョブナンバー
	int					seq_no;			//シーケンスナンバー
	int					game_seq_no[2];	//シーケンスナンバー
	int					wait_work;
	GFL_TCBSYS			*tcbsys;
	void				*tcb_work;
	YT_CLACT			*clact;
	GFL_AREAMAN			*clact_area;
	YT_PLAYER_STATUS	ps[2];
	u8					default_fall_wait;
    NET_PARAM*          pNetParam;   //通信用構造体
	GFL_TCB				*check_tcb;
	GFL_BMP_DATA		*yossy_bmp;
	GFL_BMPWIN			*yossy_bmpwin;
	void				*job_work;		//ジョブ毎に使用するワークのポインタを格納
}GAME_PARAM;

//TCBプライオリティ定義
#define	TCB_PRI_GAME_MAIN	(1)
#define	TCB_PRI_PLAYER		(10)
#define	TCB_PRI_FALL_CHR	(100)

//プレーヤー設定
#define	YT_PLAYER_FRAME		(GFL_BG_FRAME2_M)	//プレーヤーを描画するフレーム
#define	YT_PLAYER_CHRNO		(0x20)				//プレーヤーキャラを転送するキャラ位置
#define	YT_PLAYER_PALNO		(0x02<<12)			//プレーヤーキャラのパレットナンバー

#define	YT_READY_NEXT_Y_POS		(48)
#define	YT_READY_FALL_SPEED		(4)
#define	YT_READY_Y_POS			(32)

#define	YT_DEFAULT_FALL_WAIT	(20)
#define	YT_FALL_WAIT			(20)
#define	YT_FALL_SPEED			(1)

#define	YT_ROTATE_SPEED			(2)

#define	YT_BIRTH_WAIT			(2*FX32_HALF)		//はさんだキャラ1個当たりの生まれるまでのウエイト
#define	YT_BIRTH_SPEED			(0x80)

//勝敗フラグ
#define	YT_GAME_WIN				(1)
#define	YT_GAME_LOSE			(2)
#define	YT_GAME_DRAW			(3)

//落下キャラ調整をする総落下キャラ数
#define	YT_ADJ_FALL_COUNT		(100)

enum{
	SEQ_GAME_PLAY=0,
	SEQ_GAME_OVER,
	SEQ_GAME_TO_TITLE,
};

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

GLOBAL	void	YT_JobNoSet(GAME_PARAM *gp,int job_no);

#include "yt_net.h"

#endif	__YT_COMMON_H__
