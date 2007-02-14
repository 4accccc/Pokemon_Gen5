
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

GLOBAL	void	YT_InitPlayer(GAME_PARAM *gp,u8 player_no,u8 type);

#endif	__PLAYER_H__
