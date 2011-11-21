//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon.h
 *	@brief  無限ダンジョン
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "script.h"
#include "scrcmd_work.h"
#include "field/infinity_dungeon_def.h"
#include "savedata/infinity_dungeon_save.h"
#include "infinity_dungeon_tool.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define INF_TUTORIAL_DATA_CNT (2)                       // チュートリアルデータ数
#define INFINITY_DUNGEON_TR_OBJ_ID_OFS     (0xB4)       // トレーナー用IDオフセット

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys,const INF_DUNGEON_DATA *p_dungeon_data, u16 warp_dir ,u8 fade_in_param ,u8 fade_out_param );
extern void INFINITY_DUNGEON_Recovery( GAMESYS_WORK *p_gsys ,const INF_DUNGEON_DATA  *p_dungeon_data);
extern void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA *p_dungeon_data ,int floor_no );
extern GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u8 obj_id);
extern u32 INFINITY_DUNGEON_GetHintMessage( const INFINITYDUNGEON_SAVE *p_save ,const INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code,u8 id_pos);
extern void INF_MakeDungeonData( INF_DUNGEON_DATA *p_wk,GAMEDATA *p_gdata, HEAPID heapID );
extern void INF_ReCalcDungeonData( INF_DUNGEON_DATA *p_wk,INFINITYDUNGEON_SAVE *p_save );
extern u32 INFINITY_DUNGEON_GetRecoverFlgPos( const INF_DUNGEON_DATA *p_dungeon_data ,u16 tr_id );
extern u32 INFINITY_DUNGEON_GetBossFloor( const INF_DUNGEON_DATA *p_dungeon_data );
extern u32 INF_GetDungeonDataSize();
extern u16 INFINITY_DUNGEON_GetTrIDByIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u8 id_pos );
extern u8 INFINITY_DUNGEON_GetDungeonColor( const INF_DUNGEON_DATA *p_wk );
extern u32 INFINITY_DUNGEON_TrainerCount( const INF_DUNGEON_DATA *p_dungeon_data );
extern BOOL INFINITY_DUNGEON_GetUpPosRoomType( const INF_DUNGEON_DATA *p_dungeon_data ,s32 now_floor ,s32 now_room );
extern u32 INFINITY_DUNGEON_GetGateBeforeBattleMessage( const INFINITYDUNGEON_SAVE *p_save ,const INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code ,BOOL is_before_battle);

