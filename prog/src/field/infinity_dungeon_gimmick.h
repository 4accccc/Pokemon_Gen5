//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		infinity_dungeon_gimmick.h
//   @brief   無限ダンジョンギミック
//   @author	Masayuki Onoue
//   @date		2011.03.03
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "infinity_dungeon.h"

// ギミックワークのアサインID
#define INF_GIMMICK_WORK_ASSIGN_ID  (0)

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

