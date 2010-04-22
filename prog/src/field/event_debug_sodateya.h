///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『育て屋』
 * @file   event_debug_sodateya.h
 * @author obata
 * @date   2010.04.22
 */
///////////////////////////////////////////////////////////////////////////////
#ifdef PM_DEBUG
#pragma once

FS_EXTERN_OVERLAY( debug_sodateya );

//-----------------------------------------------------------------------------
/**
 * @brief 育て屋デバッグメニューイベントを生成する
 *
 * @parma gameSystem
 * @param wk         FIELDMAP_WORKへのポインタ
 */
//-----------------------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_DebugMenuSodateya( GAMESYS_WORK* gameSystem, void* wk );


#endif // PM_DEBUG
