
//============================================================================================
/**
 * @file	player.h
 * @brief	DS版ヨッシーのたまご プレーヤーヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__PLAYER_H__
#define	__PLAYER_H__

#undef GLOBAL
#ifdef __PLAYER_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

/// プレーヤー構造体
typedef struct _PLAYER_PARAM PLAYER_PARAM;


//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー初期化
 *	
 *	@param	gp			ゲームパラメータポインタ
 *	@param	player_no	1P or 2P
 *	@param	type		プレーヤータイプ
 */
//-----------------------------------------------------------------------------
GLOBAL PLAYER_PARAM*	YT_InitPlayer(GAME_PARAM *gp,u8 player_no,u8 type);

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
GLOBAL void YT_InitPlayerAddTask(GAME_PARAM *gp, PLAYER_PARAM* pp,u8 player_no);

//----------------------------------------------------------------------------
/**
 *	@brief	通信できたプレーヤーアニメをセット
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	anm_no	セットするアニメナンバー
 */
//-----------------------------------------------------------------------------
GLOBAL void	YT_PlayerAnimeNetSet(GAME_PARAM *gp,PLAYER_PARAM *pp,int player_no,int anm_no,int line_no,int rot,int actno);

#endif	//__PLAYER_H__

