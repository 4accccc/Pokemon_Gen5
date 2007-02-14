
//============================================================================================
/**
 * @file	fallchr.h
 * @brief	DS版ヨッシーのたまご落下キャラヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__FALL_CHR_H__
#define	__FALL_CHR_H__

#undef GLOBAL
#ifdef __FALL_CHR_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラクタパラメータ構造体
 */
//-----------------------------------------------------------------------------
struct fall_chr_param{
	GAME_PARAM	*gp;
	CLWK		*clwk;
	u8			seq_no;			//シーケンスナンバー
	u8			push_seq_no;	//シーケンスナンバー
	u8			player_no;		//プレーヤーナンバー
	u8			type;			//キャラの種類

	u8			line_no;		//現在いるライン
	u8			dir;			//キャラクタの向き
	u8			clact_no;		//セルアクターナンバー
	u8			fall_wait;

	int			rotate_speed;

	u8			fall_wait_tmp;
	u8			rotate_flag		:2;		//回転フラグ
	u8			overturn_flag	:1;		//ひっくり返しフラグ
	u8							:5;
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
GLOBAL	FALL_CHR_PARAM	*YT_InitFallChr(GAME_PARAM *gp,u8 player_no,u8 type,u8 line_no);

#endif	__FALL_CHR_H__
