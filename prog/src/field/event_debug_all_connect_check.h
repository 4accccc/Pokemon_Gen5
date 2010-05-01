//////////////////////////////////////////////////////////////////////////
/**
 * @brief  全接続チェックイベント
 * @file   event_debug_all_connect_check.c
 * @author obata
 * @date   2010.04.21
 */
//////////////////////////////////////////////////////////////////////////
#ifdef PM_DEBUG
#pragma once

FS_EXTERN_OVERLAY( debug_connect_check );

//------------------------------------------------------------------------
/**
 * @brief 全接続チェックイベントを生成する
 *
 * @param gameSystem
 * @param wk
 *
 * @return 生成したイベント
 */
//------------------------------------------------------------------------
extern GMEVENT* EVENT_DEBUG_AllConnectCheck( GAMESYS_WORK* gameSystem, void* wk );

//------------------------------------------------------------------------
/**
 * @brief 現マップの接続チェックイベントを生成する
 *
 * @param gameSystem
 * @param wk
 *
 * @return 生成したイベント
 */
//------------------------------------------------------------------------
extern GMEVENT* EVENT_DEBUG_NowConnectCheck( GAMESYS_WORK* gameSystem, void* wk );

#endif
