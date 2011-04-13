//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		infinity_dungeon.h
//   @brief   無限ダンジョン
//   @author	Masayuki Onoue
//   @date		2011.02.25
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "script.h"
#include "scrcmd_work.h"
#include "field/infinity_dungeon_def.h"
#include "savedata/infinity_dungeon_save.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define ROOM_MAX_X  (5)    // 部屋X最大数
#define ROOM_MAX_Y  (5)    // 部屋Y最大数
#define FLOOR_MAX   (10)   // フロア最大数
#define ROOM_MAX    (ROOM_MAX_X*ROOM_MAX_Y)   // 部屋最大数
#define INF_MAX_SEARC_CNT (100) // 乱数生成時最高思考数

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys, u16 warp_dir );
extern void INFINITY_DUNGEON_Recovery( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys );
extern void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys, int floor_no );
extern GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u16 obj_id);
extern u32 INFINITY_DUNGEON_GetHintMessage( INFINITYDUNGEON_SAVE *p_save ,u8 hint_level ,u16 obj_code);
extern u32 INFINITY_DUNGEON_GetTrIDPos( INFINITYDUNGEON_SAVE *p_save ,u8 floor_id ,u32 tr_id );
#ifdef PM_DEBUG
extern void DEBUG_INFINITY_DUNGEON_MakeDungeonData();
#endif

