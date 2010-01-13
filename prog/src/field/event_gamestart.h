//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_gamestart.h
 *	@brief  イベント　ゲーム開始
 *	@author	tomoya takahashi
 *	@date		2010.01.12
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "field/fieldmap.h"


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief ゲーム開始イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_GameStart(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);

//------------------------------------------------------------------
/**
 * @brief デバッグ用：ゲーム終了
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_GameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);

//--------------------------------------------------------------
/**
 * @brief 全滅時のマップ遷移処理（フィールド非生成時）
 */
//--------------------------------------------------------------
extern GMEVENT * EVENT_GameOver( GAMESYS_WORK * gsys );


//--------------------------------------------------------------
/**
 * @brief FIELD_INITオーバーレイの読み込み・破棄
 */
//--------------------------------------------------------------
extern void GAMESTART_OVERLAY_FIELD_INIT_Load( void );
extern void GAMESTART_OVERLAY_FIELD_INIT_UnLoad( void );


#ifdef _cplusplus
}	// extern "C"{
#endif



