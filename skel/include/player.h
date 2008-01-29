
//============================================================================================
/**
 * @file	player.h
 * @brief	DS版ヨッシーのたまごタイトルヘッダー
 * @date	2007.02.08
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
 *	@param	bNetSend	送信データを送る必要があるときTRUE
 */
//-----------------------------------------------------------------------------
GLOBAL PLAYER_PARAM*	YT_InitPlayer(GAME_PARAM *gp,u8 player_no,u8 type,u8 bNetSend);

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
GLOBAL void	YT_NetPlayerChrTrans(GAME_PARAM *gp,PLAYER_PARAM *pp,int player_no,int anm_no,int line_no,int rot,int pat_no);

//----------------------------------------------------------------------------
/**
 *	@brief	プレーヤー表示用スクリーン生成
 *	
 *	@param	pp			プレーヤーパラメータポインタ
 *	@param	old_line_no	現在のプレーヤーのライン位置
 *	@param	new_line_no	新たなプレーヤーのライン位置
 */
//-----------------------------------------------------------------------------
GLOBAL void YT_PlayerScreenMakeNetFunc(PLAYER_PARAM *pp, u8 player_no, u8 old_line_no,u8 new_line_no);

//----------------------------------------------------------------------------
/**
 *	@brief	通信用 プレーヤー表示用スクリーン生成（ターンするとき）
 *	
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	flag	0:ターン開始時　1:ターン終了時
 */
//-----------------------------------------------------------------------------

GLOBAL void	YT_PlayerRotateScreenMakeNetFunc(PLAYER_PARAM *pp, u8 player_no, u8 line_no, u8 flag);

//----------------------------------------------------------------------------
/**
 *	@brief	勝ち負けフラグを立てる
 *	@param	pp		プレーヤーパラメータポインタ
 *	@param	bWin	勝った場合にTRUE
 */
//-----------------------------------------------------------------------------
GLOBAL void YT_PlayerSetWinLoseFlag(YT_PLAYER_STATUS *ps, int bWin);

#endif	__PLAYER_H__
