//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_gimmick.h
 *	@brief  無限ダンジョンギミック
 *	@author	Masayuki Onoue
 *	@date		2011.03.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "infinity_dungeon.h"

// ギミックワークのアサインID
#define INF_GIMMICK_WORK_ASSIGN_ID  (0)

//=============================================================================
//  typedef
//=============================================================================
typedef struct _INF_GIMMICK_WK INF_GIMMICK_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void INFINITY_DUNGEON_Setup( FIELDMAP_WORK *p_fieldmap );
extern void INFINITY_DUNGEON_End( FIELDMAP_WORK *p_fieldmap );
extern void INFINITY_DUNGEON_Move( FIELDMAP_WORK *p_fieldmap );
extern INF_DUNGEON_DATA* INFINITY_DUNGEON_DataGet( FIELDMAP_WORK *p_fieldmap );
extern GMEVENT * INFINITY_DUNGEON_EVENT_ElevatorOpen(GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_fieldmap ,BOOL inOpen ,int elev_no);
extern GMEVENT * INFINITY_DUNGEON_EVENT_ElevatorLiftBan(GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_fieldmap );
extern GMEVENT * INFINITY_DUNGEON_EVENT_GateOpen(GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_fieldmap );
extern void INFINITY_DUNGEON_GMK_SetBModelData( INF_GIMMICK_WK *p_wk, FIELD_BMODEL * bmodel ,u8 dir );

