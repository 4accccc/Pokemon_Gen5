
//============================================================================================
/**
 * @file	multi_boot.h
 * @brief	DS版ヨッシーのたまご	マルチブート親機
 * @date	2007.05.11
 */
//============================================================================================

#ifndef	__MULTI_BOOT_H__
#define	__MULTI_BOOT_H__

#undef GLOBAL
#ifdef __MULTI_BOOT_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

GLOBAL	void	YT_InitMultiBoot(GAME_PARAM *gp);
GLOBAL	void	YT_MainMultiBoot(GAME_PARAM *gp);

#endif	__MULTI_BOOT_H__
