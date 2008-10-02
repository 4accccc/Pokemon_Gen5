//============================================================================================
/**
 * @file	field_player.h
 * @date	2008.9.29
 * @brief	（仮）フィールドプレイヤー制御
 */
//============================================================================================
#pragma once

#include "field_common.h"

typedef struct _PC_ACTCONT PC_ACTCONT;

extern PC_ACTCONT*		CreatePlayerAct( FIELD_SETUP* gs, HEAPID heapID );
extern void				DeletePlayerAct( PC_ACTCONT* pcActCont );
extern void				MainPlayerAct( PC_ACTCONT* pcActCont, int key );
extern void				MainPlayerAct_NoGrid( PC_ACTCONT* pcActCont, int key);
//static void				MainFriendPlayerAct( PC_ACTCONT* pcActCont );
//static void				SetPlayerActAnm( PC_ACTCONT* pcActCont, int anmSetID );
extern void				SetPlayerActTrans( PC_ACTCONT* pcActCont, const VecFx32* trans );
extern void				GetPlayerActTrans( PC_ACTCONT* pcActCont, VecFx32* trans );
extern void				SetPlayerActDirection( PC_ACTCONT* pcActCont, const u16* direction );
extern void				GetPlayerActDirection( PC_ACTCONT* pcActCont, u16* direction );

#include "field_player_grid.h"
