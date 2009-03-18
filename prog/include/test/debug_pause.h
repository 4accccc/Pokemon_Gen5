//======================================================================
/**
 * @file	debug_pause.h
 * @brief	コマ送り機能
 * @author	ariizumi
 * @data	09/03/18
 */
//======================================================================

#ifndef DEBUG_PAUSE_H__
#define DEBUG_PAUSE_H__

#ifdef PM_DEBUG

void DEBUG_PAUSE_Init( void );
//戻り値がFALSEならフレーム処理を飛ばす
const BOOL DEBUG_PAUSE_Update( void );

//有効の設定・取得
void DEBUG_PAUSE_SetEnable( const BOOL isEnable );
const BOOL DEBUG_PAUSE_GetEnable( void );

#endif //PM_DEBUG

#endif //DEBUG_PAUSE_H__
