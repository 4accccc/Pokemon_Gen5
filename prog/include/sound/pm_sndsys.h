//============================================================================================
/**
 * @file	pm_sndsys.h
 * @brief	ポケモンサウンドシステム
 * @author	
 * @date	
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	サウンドシステム（main.c等から呼び出される）
 *
 */
//============================================================================================
extern void	PMSNDSYS_Init( void );
extern void	PMSNDSYS_Main( void );
extern void	PMSNDSYS_Exit( void );

//============================================================================================
/**
 *
 * @brief	サウンド関数（各appから呼び出される）
 *
 */
//============================================================================================
extern NNSSndHandle* PMSNDSYS_GetBGMhandlePointer( void );

extern BOOL	PMSNDSYS_PlayBGM( u32 soundIdx );
extern void	PMSNDSYS_StopBGM( void );
extern void	PMSNDSYS_PauseBGM( BOOL pauseFlag );
extern void	PMSNDSYS_PushBGM( void );
extern void	PMSNDSYS_PopBGM( void );

extern BOOL	PMSNDSYS_PlaySE( u32 soundNum );
extern BOOL	PMSNDSYS_PlayVoice( u32 pokeNum );
