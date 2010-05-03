//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui.h
 *	@brief  UI　上書き　デバック機能
 *	@author	tomoya takahashi
 *	@date		2010.04.29
 *
 *	モジュール名：
 *
 *	*拡張メモリに配置されるため、デバッカー上でしか動作しません。
 *	コンソールタイプのチェックをしてから使用してください。
 *	if( (OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER)) ){
 *	}
 *	
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#ifdef PM_DEBUG

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	起動タイプ
//=====================================
typedef enum {
  DEBUG_UI_NONE,            // なし
  DEBUG_UI_AUTO_UPDOWN,     // 自動　上下
  DEBUG_UI_AUTO_LEFTRIGHT,  // 自動　左右
  DEBUG_UI_AUTO_USER_INPUT, // 自動　入力データ再生

  DEBUG_UI_TYPE_MAX,
} DEBUG_UI_TYPE;

//-------------------------------------
///	再生タイプ
//=====================================
typedef enum {
  DEBUG_UI_PLAY_LOOP,       // LOOP再生
  DEBUG_UI_PLAY_ONE,        // 一回再生

  DEBUG_UI_PLAY_MAX,
} DEBUG_UI_PLAY_TYPE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern void DEBUG_UI_SetUp( DEBUG_UI_TYPE type, DEBUG_UI_PLAY_TYPE play );
extern DEBUG_UI_TYPE DEBUG_UI_GetType( void );
extern BOOL DEBUG_UI_IsUpdate( void );


#endif // PM_DEBUG



#ifdef _cplusplus
}	// extern "C"{
#endif



