//==============================================================================
/**
 * @file	game_start.h
 * @brief	「さいしょから」or「つづきから」を選択後の処理
 * @author	matsuda
 * @date	2009.01.07(水)
 */
//==============================================================================
#ifndef __GAME_START_H__
#define __GAME_START_H__

//==============================================================================
//	外部関数宣言
//==============================================================================
extern void GameStart_Beginning(void);
extern void GameStart_Continue(void);
extern void GameStart_ContinueNetOff(void);
extern void GameStart_ContinueNet(void);
extern void GameStart_Debug(void);
extern void GameStart_DebugNet(void);
extern void GameStart_DebugNetOff(void);
extern void GameStart_Debug_SelectName(void);


#endif	//__GAME_START_H__
