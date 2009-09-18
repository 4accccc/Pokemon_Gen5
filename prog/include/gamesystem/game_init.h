//============================================================================================
/**
 * @file	game_init.h
 * @date	2008.10.31
 * @author	tamada GAMEFREAK inc.
 * @brief	ゲーム開始時の初期化用定義など
 */
//============================================================================================
#pragma once
#ifdef	__cplusplus
extern "C" {
#endif

extern const GFL_PROC_DATA GameMainProcData;
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
	GAMEINIT_MODE_FIRST,
	GAMEINIT_MODE_CONTINUE,
	GAMEINIT_MODE_DEBUG,
}GAMEINIT_MODE;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMEINIT_MODE mode;
	VecFx32 pos;
	u16 mapid;
	s16 dir;
}GAME_INIT_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern GAME_INIT_WORK * DEBUG_GetGameInitWork(
  GAMEINIT_MODE mode, u16 mapid, VecFx32 *pos, s16 dir );

#ifdef	__cplusplus
}	//extern "C" {
#endif
