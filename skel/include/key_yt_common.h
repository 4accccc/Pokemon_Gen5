
//============================================================================================
/**
 * @file	yt_common.h
 * @brief	DS版ヨッシーのたまごcommonヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__KEY_YT_COMMON_H__
#define	__KEY_YT_COMMON_H__

#define GLOBAL extern

#include "sample_sound/yossy_egg.sadl"

//ジョブナンバー定義
enum{
	KEY_YT_InitTitleNo=0,
	KEY_YT_MainTitleNo,
	KEY_YT_InitGameNo,
	KEY_YT_MainGameNo,
	KEY_YT_InitMultiBootNo,
	KEY_YT_MainMultiBootNo,
	KEY_YT_Init3DTestNo,
	KEY_YT_Main3DTestNo,
	KEY_YT_InitBlockOutNo,
	KEY_YT_MainBlockOutNo,
};

//-------------------------------------
///	キャラクタナンバー
//=====================================
enum{
	KEY_YT_CHR_KURIBO=0,
	KEY_YT_CHR_PAKKUN,
	KEY_YT_CHR_GESSO,
	KEY_YT_CHR_TERESA,
	KEY_YT_CHR_DEKATERESA,
	KEY_YT_CHR_GREEN_EGG_U,
	KEY_YT_CHR_GREEN_EGG_D,
	KEY_YT_CHR_RED_EGG_U,
	KEY_YT_CHR_RED_EGG_D,
	KEY_YT_CHR_GREEN_EGG,
	KEY_YT_CHR_RED_EGG,
};

//-------------------------------------
///	キャラクタ毎の表示プライオリティー
//=====================================
enum{
	KEY_YT_PRI_KURIBO=40,
	KEY_YT_PRI_PAKKUN=40,
	KEY_YT_PRI_GESSO=40,
	KEY_YT_PRI_TERESA=40,
	KEY_YT_PRI_DEKATERESA=30,
	KEY_YT_PRI_GREEN_EGG_U=0,
	KEY_YT_PRI_GREEN_EGG_D=10,
	KEY_YT_PRI_RED_EGG_U=0,
	KEY_YT_PRI_RED_EGG_D=10,
	KEY_YT_PRI_GREEN_EGG=20,
	KEY_YT_PRI_RED_EGG=20,
};

//-------------------------------------
///	サウンド階層レベル
//=====================================
enum{
	MUS_LEVEL_SE=0,
	MUS_LEVEL_JINGLE,
	MUS_LEVEL_BGM,
	MUS_LEVEL_MAX,
};

//-------------------------------------
///	no_active_flagの内訳
//=====================================
#define	KEY_YT_ROTATE_FLAG			(0x0003)		//回転フラグ
#define	KEY_YT_OVERTURN_FLAG		(0x003c)		//ひっくり返しフラグ
#define	KEY_YT_EGG_MAKE_CHECK_FLAG	(0x03c0)		//タマゴ作成チェックフラグ
#define	KEY_YT_EGG_MAKE_FLAG		(0x0400)		//タマゴ作成フラグ
#define	KEY_YT_BIRTH_FLAG			(0x0800)		//ヨッシー生まれるフラグ
#define	KEY_YT_RENSA_FLAG			(0x1000)		//連鎖フラグ

//-------------------------------------
///	セルアクタープロセスワーク
//=====================================

#define	KEY_YT_CLACT_MAX	(128)
#define	KEY_YT_LINE_MAX		(4)
#define	KEY_YT_HEIGHT_MAX	(9)

typedef struct {
	void* p_cellbuff;
	void* p_cellanmbuff;
	NNSG2dCellDataBank*		p_cell;
	NNSG2dCellAnimBankData*	p_cellanm;
	NNSG2dImageProxy		imageproxy;
	NNSG2dImagePaletteProxy	plttproxy;
} KEY_YT_CLACT_RES;

typedef struct {
	GFL_CLUNIT		*p_unit;
	KEY_YT_CLACT_RES	res;
	GFL_CLWK		*clact_work[KEY_YT_CLACT_MAX];
} KEY_YT_CLACT;

typedef struct	fall_chr_param	FALL_CHR_PARAM;
//通信用構造体
typedef	struct  _NET_PARAM NET_PARAM;


typedef struct{
	GFL_TCB			*tcb_player;
	FALL_CHR_PARAM	*ready[KEY_YT_LINE_MAX][KEY_YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*fall[KEY_YT_LINE_MAX][KEY_YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*stop[KEY_YT_LINE_MAX][KEY_YT_HEIGHT_MAX];
	FALL_CHR_PARAM	*rensa[KEY_YT_HEIGHT_MAX];
	u8				falltbl[KEY_YT_LINE_MAX];
	u8				stoptbl[KEY_YT_LINE_MAX];
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
}KEY_YT_PLAYER_STATUS;

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
	KEY_YT_CLACT			*clact;
	GFL_AREAMAN			*clact_area;
	KEY_YT_PLAYER_STATUS	ps[2];
	u8					default_fall_wait;
    NET_PARAM*          pNetParam;   //通信用構造体
	GFL_TCB				*check_tcb;
	GFL_BMP_DATA		*yossy_bmp;
	GFL_BMPWIN			*yossy_bmpwin;
	void				*job_work;		//ジョブ毎に使用するワークのポインタを格納
	int					mus_level[MUS_LEVEL_MAX];	//サウンド階層レベルの保持
	int					raster_flag;
}GAME_PARAM;

//TCBプライオリティ定義
#define	TCB_PRI_GAME_MAIN	(1)
#define	TCB_PRI_PLAYER		(10)
#define	TCB_PRI_FALL_CHR	(100)

//プレーヤー設定
#define	KEY_YT_PLAYER_FRAME		(GFL_BG_FRAME2_M)	//プレーヤーを描画するフレーム
#define	KEY_YT_PLAYER_CHRNO		(0x20)				//プレーヤーキャラを転送するキャラ位置
#define	KEY_YT_PLAYER_PALNO		(0x02<<12)			//プレーヤーキャラのパレットナンバー

#define	KEY_YT_READY_NEXT_Y_POS		(48)
#define	KEY_YT_READY_FALL_SPEED		(4)
#define	KEY_YT_READY_Y_POS			(32)

#define	KEY_YT_DEFAULT_FALL_WAIT	(20)
#define	KEY_YT_FALL_WAIT			(20)
#define	KEY_YT_FALL_SPEED			(1)

#define	KEY_YT_ROTATE_SPEED			(2)

#define	KEY_YT_BIRTH_WAIT			(2*FX32_HALF)		//はさんだキャラ1個当たりの生まれるまでのウエイト
#define	KEY_YT_BIRTH_SPEED			(0x80)

//勝敗フラグ
#define	KEY_YT_GAME_WIN				(1)
#define	KEY_YT_GAME_LOSE			(2)
#define	KEY_YT_GAME_DRAW			(3)

//落下キャラ調整をする総落下キャラ数
#define	KEY_YT_ADJ_FALL_COUNT		(100)

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
	KEY_YT_READY_WAIT=0,
	KEY_YT_READY_MAKE,
	KEY_YT_READY_ALREADY,
};

GLOBAL	void	KEY_YT_JobNoSet(GAME_PARAM *gp,int job_no);

#include "key_yt_net.h"

#endif	__KEY_YT_COMMON_H__
