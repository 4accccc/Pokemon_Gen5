
//============================================================================================
/**
 * @file	title.h
 * @brief	DS版ヨッシーのたまごタイトルヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__TITLE_H__
#define	__TITLE_H__

#undef GLOBAL
#ifdef __TITLE_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

GLOBAL	void	YT_InitTitle(GAME_PARAM *gp);
GLOBAL	void	YT_MainTitle(GAME_PARAM *gp);

#endif	__TITLE_H__
