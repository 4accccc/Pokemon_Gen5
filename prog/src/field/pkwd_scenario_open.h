//============================================================================================
/**
 *	@file	pkwd_scenario_open.h
 *	@brief  ポケウッド解放処理関連ヘッダ
 *	@author	mori  GAME FREAK inc.
 *  @date   2011.09.15
 *
 */
//============================================================================================
#pragma once

// シナリオオープン条件の種類
#define PW_OPEN_PROVISION_BEGINING  ( 0 )   // 最初から
#define PW_OPEN_PROVISION_SUCCESS   ( 1 )   // 成功エンド
#define PW_OPEN_PROVISION_GAMECLEAR ( 2 )   // ゲームクリア
#define PW_OPEN_PROVISION_STARRANK  ( 3 )   // スターランク
#define PW_OPEN_PROVISION_PROFIT    ( 4 )   // 興行収入
#define PW_OPEN_PROVISION_ALLGOOD   ( 5 )   // 全作品GOODエンド
#define PW_OPEN_PROVISION_TUTORIAL  ( 6 )   // チュートリアルクリア後
#define PW_OPEN_PROVISION_DUAL      ( 7 )   // 2つの条件を持っている(指定シナリオクリア＆クリア）

// シナリオがオープンするための興行収入額
#define POKEWOOD_PROVISION_PROFIT   ( 5000 )

extern void PKWD_OpenCheckScenario( POKEWOOD_SAVEDATA *ps, GAMEDATA *gamedata, int scenario_no, int result, int profit );
extern int  PKWD_GetScenarioOpenProvision( int scenario_no );
