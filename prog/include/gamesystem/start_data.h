//============================================================================================
/**
 * @file  start_data.h
 * @brief 「さいしょから」のプレイヤー進行データ初期化
 * @date  2011.09.28
 */
//============================================================================================
#pragma once
#include <gflib.h>
#include "gamesystem/game_data.h"

FS_EXTERN_OVERLAY( start_data );

extern void GAMEINIT_SetCommonStartData( GAMEDATA * gamedata, HEAPID heapID );
extern void GAMEINIT_SetNormalStartData( GAMEDATA * gamedata, HEAPID heapID );

