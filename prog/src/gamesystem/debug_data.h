//============================================================================================
/**
 * @file	debug_data.h
 * @brief	ゲーム開始時のデバッグデータ追加処理
 * @author	tamada GAME FREAK Inc.
 * @date	09.10.24
 *
 */
//============================================================================================
#pragma once
#include <gflib.h>
#include "gamesystem/game_data.h"

extern void DEBUG_MyPokeAdd(GAMEDATA * gamedata, HEAPID heapID);
extern void DEBUG_MYITEM_MakeBag(GAMEDATA * gamedata, int heapID);

