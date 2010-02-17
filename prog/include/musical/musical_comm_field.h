//======================================================================
/**
 * @file	musical_comm_field.h
 * @brief	ミュージカル用 フィールドスクリプトから呼ぶ通信処理
 * @author	ariizumi
 * @data	10/02/16
 */
//======================================================================
#pragma once

#include "gamesystem/game_data.h"

typedef struct _MUS_COMM_WORK MUS_COMM_WORK;

//ワークの作成・削除
extern MUS_COMM_WORK* MUS_COMM_InitField( HEAPID heapId , GAMEDATA *gameData );
extern void MUS_COMM_ExitField( MUS_COMM_WORK *work );
