//=============================================================================
/**
 * @file	comm_save.h
 * @brief	通信同期セーブ処理
 * @author	Akito Mori
 * @date    2006.05.12
 */
//=============================================================================

#pragma once

#include "savedata/savedata.h"

extern void CommSyncronizeSaveInit( int *seq );
extern int CommSyncronizeSave( SAVEDATA *savedata, int BlockID, int *seq );



