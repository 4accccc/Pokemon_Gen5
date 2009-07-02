//======================================================================
/**
 * @file	plist_sys.h
 * @brief	ポケモンリスト メイン処理
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PSTATUS
 */
//======================================================================
#pragma once

#include "p_sta_local_def.h"

extern const BOOL PSTATUS_InitPokeStatus( PSTATUS_WORK *work );
extern const BOOL PSTATUS_TermPokeStatus( PSTATUS_WORK *work );
extern const BOOL PSTATUS_UpdatePokeStatus( PSTATUS_WORK *work );
extern const POKEMON_PASO_PARAM* PSTATUS_UTIL_GetCurrentPPP( PSTATUS_WORK *work );

