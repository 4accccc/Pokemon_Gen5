
//============================================================================================
/**
 * @file	fallchr.h
 * @brief	DS版ヨッシーのたまご落下キャラヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__FALL_CHR_H__
#define	__FALL_CHR_H__

#define GLOBAL extern

//----------------------------------------------------------------------------
/**
 *	@brief	落下キャラクタパラメータ構造体
 */
//-----------------------------------------------------------------------------
struct fall_chr_param{
	GAME_PARAM		*gp;
	GFL_CLWK		*clwk;
	GFL_CLACTPOS	now_pos;
	GFL_CLACTPOS	offset_pos;

	u8			seq_no;			//シーケンスナンバー
	u8			push_seq_no;	//シーケンスナンバー
	u8			player_no;		//プレーヤーナンバー
	u8			type;			//キャラの種類

	u8			line_no;		//現在いるライン
	u8			dir;			//キャラクタの向き
	u8			clact_no;		//セルアクターナンバー
	u8			fall_wait;

	int			speed_value;	//移動スピード格納ワーク（ROTATE、EGG_MAKEなど）

	u8			wait_value;		//ウエイト系の格納ワーク
	u8			fall_wait_tmp;
	u8			rotate_flag		:2;		//回転フラグ
	u8			overturn_flag	:1;		//ひっくり返しフラグ
	u8			egg_make_flag	:1;
	u8							:4;		//連鎖落下中フラグ
	u8			chr_count;		//タマゴキャラの時のはさんでいるタマゴの数
	int			birth_wait;		//生まれるまでのウエイト
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
GLOBAL	FALL_CHR_PARAM	*KEY_YT_InitFallChr(GAME_PARAM *gp,u8 player_no,u8 type,u8 line_no);

//----------------------------------------------------------------------------
/**
 *	@brief	Network用落下キャラ生成
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		キャラタイプ
 *	@param	line_no		落下ラインナンバー
 *
 *	@retval GFL_CLWK*
 */
//-----------------------------------------------------------------------------
GLOBAL GFL_CLWK* KEY_YT_InitNetworkFallChr(GAME_PARAM *gp,u8 player_no,u8 type,u8 line_no);

//----------------------------------------------------------------------------
/**
 *	@brief	タマゴ生成チェック
 *	
 *	@param	ps		プレーヤーステータス
 */
//-----------------------------------------------------------------------------
GLOBAL	void	KEY_YT_EggMakeCheck(KEY_YT_PLAYER_STATUS *ps);


//----------------------------------------------------------------------------
/**
 *	@brief	ヨッシーの生まれるアニメーションタスクをセット
 *	
 *	@param	gs		ゲームパラメータポインタ
 *	@param	ps		プレーヤーステータス
 *	@param	pos_x		
 *	@param	pos_y		
 *	@param	count		
 */
//-----------------------------------------------------------------------------
GLOBAL void KEY_YT_YossyBirthAnimeTaskSet(GAME_PARAM *gp,KEY_YT_PLAYER_STATUS *ps,u8 pos_x,u8 pos_y,u8 count);


#endif	__FALL_CHR_H__
