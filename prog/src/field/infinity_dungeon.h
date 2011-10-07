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
//  typedef
//=============================================================================
typedef struct _INF_DUNGEON_DATA INF_DUNGEON_DATA;

//=============================================================================
//
//					定数宣言
//
//=============================================================================
#define ROOM_MAX_X            (4)                       // 部屋X最大数
#define ROOM_MAX_Y            (4)                       // 部屋Y最大数
#define ROOM_MAX              (ROOM_MAX_X*ROOM_MAX_Y)   // 部屋最大数
#define INF_MAX_SEARC_CNT     (100)                     // 乱数生成時最高思考数
#define INF_TUTORIAL_DATA_CNT (2)                       // チュートリアルデータ数
#define INFINITY_DUNGEON_TR_OBJ_ID_OFS     (0xB4)       // トレーナー用IDオフセット

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys,INF_DUNGEON_DATA *p_dungeon_data, u16 warp_dir );
extern void INFINITY_DUNGEON_Recovery( GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA  *p_dungeon_data);
extern void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA *p_dungeon_data ,int floor_no );
extern GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u8 obj_id);
extern u32 INFINITY_DUNGEON_GetHintMessage( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code);
extern void INF_MakeDungeonData( INF_DUNGEON_DATA *p_wk,GAMEDATA *p_gdata, HEAPID heapID );
extern void INF_ReCalcDungeonData( INF_DUNGEON_DATA *p_wk,INFINITYDUNGEON_SAVE *p_save );
extern u32 INFINITY_DUNGEON_GetRecoverFlgPos( INF_DUNGEON_DATA *p_dungeon_data ,u16 tr_id );
extern u32 INFINITY_DUNGEON_GetBossFloor( INF_DUNGEON_DATA *p_dungeon_data );
extern u32 INF_GetDungeonDataSize();
extern u16 INFINITY_DUNGEON_GetTrIDByIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u8 id_pos );
extern u8 INFINITY_DUNGEON_GetDungeonColor( INF_DUNGEON_DATA *p_wk );
extern u32 INFINITY_DUNGEON_TrainerCount( INF_DUNGEON_DATA *p_dungeon_data );
extern BOOL INFINITY_DUNGEON_GetUpPosRoomType( INF_DUNGEON_DATA *p_dungeon_data ,s32 now_floor ,s32 now_room );

