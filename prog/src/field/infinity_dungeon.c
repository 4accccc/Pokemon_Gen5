//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon.c
 *	@brief  無限ダンジョン
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fldmmdl.h"
#include "infinity_dungeon.h"
#include "infinity_dungeon_local_def.h"
#include "system/gfl_use.h"
#include "scrcmd_work.h"
#include "fieldmap/zone_id.h"
#include "event_mapchange.h"
#include "event_field_fade.h"
#include "field/field_const.h"
#include "infinity_dungeon_warp_dir.h"
#include "savedata/infinity_dungeon_save.h"
#include "../../../resource/fldmapdata/build_model/output/buildmodel_indoor.naix"
#include "../../../resource/fldmapdata/script/infinity_dungeon_scr_def.h"
#include "../../../resource/fldmapdata/script/trainer_def.h"
#include "field/infinity_dungeon_def.h"
#include "fld_btl_inst_tool.h"
#include "event_battle.h"
#include "system/main.h"  //HEAPID_FIELDMAP
#include "message.naix"
#include "tr_tool/ex_trainer_def.h"
#include "msg/script/msg_infinity_dungeon_scr.h"
#include "infinity_dungeon_gimmick.h"
#include "gmk_tmp_wk.h"

// グローバル変数
#ifdef PM_DEBUG
extern int g_debug_inf_tr_pos_type;
extern BOOL g_debug_inf_data_change;
extern u32  g_debug_inf_master_seed;
#endif

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	部屋位置データ
//=====================================
typedef struct {
  int x;
  int y;
}DIG_POS;

//-------------------------------------
///	迷路生成用ワーク
//=====================================
typedef struct {
  int data_index;
  DIG_POS now_pos;
  DIG_POS dig_pos_list[INF_MAX_SEARC_CNT];
}INF_DIG_DATA;

//-------------------------------------
// グリッド座標データ
//=====================================
typedef struct {
  s32 x;
  s32 y;
  s32 z;
}GRID_DATA;

//-------------------------------------
///	ルームデータ
//=====================================
typedef struct{
  u8  tr_pos_type;
  u8 room_type;
  BOOL can_move[4];
  u8  room_id;
}ROOM_DATA;

//-------------------------------------
///	ダンジョン内トレーナーデータ
//=====================================
typedef struct{
  u16 tr_id[INFINITY_DUNGEON_TRID_MAX]; // 全トレーナーランダム配列
}DUNGEON_TRAINER;

//-------------------------------------
///	無限ダンジョン用シード
//=====================================
typedef struct 
{
  s32 master_seed;
  s32 tr_data_seed;
  s32 dungeon_data_seed;
  u16 tr_seed[FLOOR_MAX][INFINITY_DUNGEON_TRID_MAX]; // トレーナーシード
}INF_DUNGEON_SEED;

//-------------------------------------
///	無限ダンジョンデータ
//=====================================
struct _INF_DUNGEON_DATA{
  INF_DUNGEON_SEED  seed;
  ROOM_DATA         room_data[FLOOR_MAX][ROOM_MAX];
  DUNGEON_TRAINER   tr_data[FLOOR_MAX];
  u32               recover_data[INFINITY_DUNGEON_RECOVERTR_MAX];
};

//-------------------------------------
///	レベルデータ
//=====================================
typedef struct {
  u16 level_max;
  u16 level_min;
}INF_LEVEL_TABLE_PARAM;

//=============================================================================
/**
 *					データ定義
*/
//=============================================================================

//-------------------------------------
// ゾーンテーブル(ブラック)
//=====================================
static const u16 sc_black_zone_table[] = {
  ZONE_ID_D31R0401, // エレベーター
  ZONE_ID_D31R0601, // ボス
  ZONE_ID_D31R0901, // フラッシュ
  ZONE_ID_D31R0701, // 波乗り
  ZONE_ID_D31R1201, // デフォルト
  ZONE_ID_D31R0201, // 受付
};

//-------------------------------------
// ゾーンテーブル(ホワイト)
//=====================================
static const u16 sc_white_zone_table[] = {
  ZONE_ID_D31R0301, // エレベーター
  ZONE_ID_D31R0501, // ボス
  ZONE_ID_D31R0801, // かいりき
  ZONE_ID_D31R1001, // いあいぎり
  ZONE_ID_D31R1101, // デフォルト
  ZONE_ID_D31R0101, // 受付
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ブラック)
//=====================================
static const GRID_DATA sc_black_elevator_pos = {
  6 , 0 , 0 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_elevator_pos = {
  6 , 0 , 0 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(受付)
//=====================================
static const GRID_DATA sc_reception_elevator_pos[] = {
  7 , 0 , 2 , // 左
  11, 0 , 2 , // 右
};

//-------------------------------------
// マップタイプごとの移動先テーブル(ブラック)
//=====================================
static const GRID_DATA sc_black_warp_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // ボス部屋
  {
    { 6 , 0 , 14},// 下
    { 6 , 0 , 14},// 上
    { 6 , 0 , 14},// 右
    { 6 , 0 , 14},// 左
  },
  // 特殊部屋１
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // 特殊部屋２
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // 通常部屋
  {
     { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
};

//-------------------------------------
// マップタイプごとの移動先テーブル(ホワイト)
//=====================================
static const GRID_DATA sc_white_warp_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // ボス部屋
  {
    { 8 , 0 , 18}, // 下
    { 8 , 0 , 18}, // 上
    { 8 , 0 , 18}, // 右
    { 8 , 0 , 18}, // 左
  },
  // 特殊部屋１
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // 特殊部屋２
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // 通常部屋
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
};

//-------------------------------------
// 配置モデル配置位置(ブラック)
//=====================================
static const GRID_DATA sc_black_bmodel_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 6 , 0 , 1}, // 上
    { 6 , 0 , 13}, // 下
    { 0 , 0 , 7}, // 左
    { 12 , 0 , 7}, // 右
  },
  // ボス部屋
  {
    { 6 , 0 , 14},// 上
    { 6 , 0 , 14},// 下
    { 6 , 0 , 14},// 左
    { 6 , 0 , 14},// 右
  },
  // 特殊部屋１
  {
    { 6 , 0 , 1}, // 上
    { 6 , 0 , 13}, // 下
    { 0 , 0 , 7}, // 左
    { 12 , 0 , 7}, // 右
  },
  // 特殊部屋２
  {
    { 6 , 0 , 1}, // 上
    { 6 , 0 , 13}, // 下
    { 0 , 0 , 7}, // 左
    { 12 , 0 , 7}, // 右
  },
  // 通常部屋
  {
    { 6 , 0 , 1}, // 上
    { 6 , 0 , 13}, // 下
    { 0 , 0 , 7}, // 左
    { 12 , 0 , 7}, // 右
  },
};
//-------------------------------------
// 配置モデル配置位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_bmodel_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // ボス部屋
  {
    { 8 , 0 , 18}, // 上
    { 8 , 0 , 18}, // 下
    { 8 , 0 , 18}, // 左
    { 8 , 0 , 18}, // 右
  },
  // 特殊部屋１
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // 特殊部屋２
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // 通常部屋
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
};

//-------------------------------------
// エリアごとのエリアテーブル
//=====================================
static const INF_LEVEL_TABLE_PARAM sc_level_table[] = {
  { 40 , 45 },   // エリア 1
  { 45 , 50 },   // エリア 2
  { 50 , 55 },   // エリア 3
  { 55 , 60 },   // エリア 4
  { 60 , 65 },   // エリア 5
  { 65 , 70 },   // エリア 6
  { 70 , 75 },   // エリア 7
  { 75 , 80 },   // エリア 8
  { 80 , 85 },   // エリア 9
  { 85 , 90 },   // エリア 10
};

//-------------------------------------
// エリアごとのエリアテーブル
//=====================================
static const u8 sc_pow_rnd_table[] = {
  3*1, // エリア 1
  3*2, // エリア 2
  3*3, // エリア 3
  3*4, // エリア 4
  3*5, // エリア 5
  3*6, // エリア 6
  3*7, // エリア 7
  3*8, // エリア 8
  3*9, // エリア 9
  31, // エリア 10
};

// 配置データ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_setup_data.cdat"

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  VecFx32 warp_pos;       // 遷移先でのマップ位置
  s32     zone_id;        // 遷移先ゾーンID
}NEXT_MAP_DATA;

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  GAMEDATA              *p_gdata;
  FIELDMAP_WORK         *p_field_work;
  INFINITYDUNGEON_SAVE  *p_save;
  FLDMAPPER             *p_g3d_mapper;
  FIELD_BMODEL_MAN      *p_bmodel_man;
  MMDLSYS               *p_mmdlsys;
  INF_DUNGEON_DATA      *p_dungeon_data;
  HEAPID                heapID;

  // 移動タイプ
  u8                    map_change_type;

  // 移動データ
  u16                   warp_dir;
  NEXT_MAP_DATA         next_map_data;
  const u16             *p_zone_data;

  // フロア
  s32 now_room;
  s32 now_floor;
  s32 next_room;
  s32 next_floor;

  // エリア(フロアから算出)
  u16 now_area;

}MAP_CHAGE_EVENT_WORK;



//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// 共通
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys);
static u32 GetDungeonColor();
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk );
static void GetFloorData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static void SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed );

// ゾーン依存
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk );
static u16 GetFieldDir( u16 warp_dir );

// 部屋データ
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk );
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン生成
static void SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y);
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);
static const GRID_DATA* GetBmodelPos( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  );
static void AddDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data);
static void SetDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);

// グリッド
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );

// マップワープイベント
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk );
static const GRID_DATA* GetWarpPosTable( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  );

// フロア移動
static const GRID_DATA* GetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );
static const GRID_DATA* GetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk ,int elevator_type );
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン内トレーナー
static int GetRoomTrainerIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static u32 GetRoomTrainerID( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data , u32 seed );
static void SetTrainer( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);
static u32 GetTrSeed( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos );

// フロア生成 
static BOOL CheckCloseAllDoor(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data);
static int CheckCanMove(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode);
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data);
static ROOM_DATA* GetDigPosRoomData( DIG_POS *p_dig_pos,ROOM_DATA* p_room_data);
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,DIG_POS *p_dig_pos);
static void pop_dig_pos( INF_DIG_DATA *p_dig_data );
static void RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context );
static BOOL CheckDigEnd( ROOM_DATA *p_room_data ,int fs_x ,int fs_y);
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context );

#ifdef PM_DEBUG
static void INF_DEBUG_OutPutFloorData( ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save );
static void INF_DEBUG_OverwritRoomData( ROOM_DATA *p_room_data );
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを生成する
 *
 *	@param	**pp_wk
 *	@param	*p_save 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_MakeDungeonData( INF_DUNGEON_DATA **pp_wk,INFINITYDUNGEON_SAVE *p_save ,HEAPID heapID)
{
  u32 master_seed;

  // ワーク確保
  *pp_wk = GFL_HEAP_AllocMemoryLo( heapID ,sizeof( INF_DUNGEON_DATA ) );
 
  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_save );
  SetDungeonSeed(&(*pp_wk)->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_save , *pp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを再計算する
 *
 *	@param	**pp_wk
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_ReCalcDungeonData( INF_DUNGEON_DATA *p_wk,INFINITYDUNGEON_SAVE *p_save )
{
  u32 master_seed;

  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_save );
  SetDungeonSeed(&p_wk->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_save , p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータの開放
 *
 *	@param	**pp_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_FreeDungeonData(INF_DUNGEON_DATA **pp_wk)
{
  // ワーク解放
  GFL_HEAP_FreeMemory( *pp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップ位置から次のマップを導き、ワープさせる
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	warp_dir 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys,INF_DUNGEON_DATA *p_dungeon_data, u16 warp_dir )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK *p_wk;
 	
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_work, p_event );
	p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_dungeon_data = p_dungeon_data;
  p_wk->warp_dir = warp_dir;
  p_wk->map_change_type = MAP_CHAGE_TYPE_ROOM;
}

//----------------------------------------------------------------------------
/**
 *	@brief  次のフロアにワープ
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	floor_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
extern void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA *p_dungeon_data ,int floor_no )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK  *p_wk;
  SAVE_CONTROL_WORK     *p_save_data;
  GAMEDATA              *p_gdata;
  INFINITYDUNGEON_SAVE  *p_save;

  // イベント呼び出し
  {
    p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
    SCRIPT_CallEvent( p_scr_work, p_event );
    p_wk = GMEVENT_GetEventWork( p_event );
    p_wk->next_floor = floor_no%INFINITYDUNGEON_FLOOR_MAX;
    p_wk->map_change_type = MAP_CHAGE_TYPE_FOOR;
    p_wk->p_dungeon_data = p_dungeon_data;
    p_wk->warp_dir = WARP_DIR_DOWN;
  }
  
  // データを生成するためにフロアを設定する
  {
    p_gdata       = GAMESYSTEM_GetGameData( p_gsys );
    p_save_data         = GAMEDATA_GetSaveControlWork( p_gdata );
    p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
    INFINITYDUNGEON_SAVE_SetFloorID( p_save , floor_no );
  
    // 再計算
    INF_ReCalcDungeonData(p_dungeon_data,p_save); 
  }

  OS_TFPrintf(1,"p_wk->next_floor:%u\n",p_wk->next_floor);
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントデータから必要データを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys)
{
  SAVE_CONTROL_WORK* p_save_data;
  p_wk->heapID        = HEAPID_FIELDMAP;
  p_wk->p_gsys        = p_gsys;
  p_wk->p_field_work  = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata       = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data   = GetZoneIdTable( p_wk );
  p_wk->now_room      = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  p_wk->p_g3d_mapper  = FIELDMAP_GetFieldG3Dmapper( p_wk->p_field_work );
  p_wk->p_bmodel_man  = FLDMAPPER_GetBuildModelManager( p_wk->p_g3d_mapper );
  p_wk->p_mmdlsys     = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
  p_wk->now_area      = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_wk->p_save );

  OS_TFPrintf(1,"p_wk->now_floor:%u\n",p_wk->now_floor);
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップチェンジイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_CHAGE_EVENT_WORK *p_wk = wk;
  ROOM_DATA *p_next_room;

  switch( *seq )
  {
  case 0:
    {
      // データ取得
      GAMESYS_WORK *p_gsys  = GMEVENT_GetGameSysWork( event );
      InitWorkData( p_wk ,p_gsys );

//      // フロアデータの取得
//      GetFloorData( p_wk->p_save ,&p_wk->dungeon_data );

      // フェードアウト
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 1:
    {
      // マップ移動
      NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
      SetNextZoneData( p_wk );
      GMEVENT_CallEvent(event,
          EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work, p_next_map_data->zone_id, &p_next_map_data->warp_pos,GetFieldDir( p_wk->warp_dir ) ));
      (*seq)++;
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }

      ONOUE_Printf("p_wk->warp_dir:%u\n",p_wk->warp_dir);

      // 受付の場合は配置モデルを置かない
      if( p_wk->warp_dir != WARP_INFORMATION )
      {
        // 次の部屋データを取得する
        p_next_room = GetNextRoomData( p_wk );

        if( (p_next_room->room_type != ROOM_TYPE_BOSS) )
        {
          // 配置モデルを配置
          SetBUildModel(p_wk,p_next_room);

          // 封鎖用OBJ配置
          SetDummyObj(p_wk,p_next_room);

          // トレーナー配置
          SetTrainer(p_wk,p_next_room);
        }
      }

      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
      
#ifdef PM_DEBUG      
      INF_DEBUG_OutPutFloorData(p_wk->p_dungeon_data->room_data[p_wk->now_floor] ,p_wk->p_save );
#endif

      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン用シードを生成する
 *
 *	@param	*p_seed 
 *	@param	master_seed 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed )
{
  int i,n;
  GFL_STD_RandContext rand_context;
 
  // シード生成用シード
#ifdef PM_DEBUG
  if( g_debug_inf_master_seed == 0 )
  {
    p_seed->master_seed = master_seed;
  }
  else
  {
    p_seed->master_seed = g_debug_inf_master_seed;
  }
#else
  p_seed->master_seed = master_seed;
#endif

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, p_seed->master_seed );

  // 子シードを生成
  p_seed->tr_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
  p_seed->dungeon_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);

  // トレーナーシード
  for( i = 0; i < FLOOR_MAX ;i++ )
  {
    for( n = 0; n < INFINITY_DUNGEON_TRID_MAX ;n++ )
    {
      p_seed->tr_seed[i][n] = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
    }
  }

  OS_TFPrintf(1,"シード番号:%u\n",p_seed->master_seed);
}

//----------------------------------------------------------------------------
/**
 *	@brief  次のゾーンデータを設定
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk)
{
  if( p_wk->map_change_type == MAP_CHAGE_TYPE_ROOM )
  {
    if( p_wk->warp_dir != WARP_INFORMATION )
    {
      // 次のゾーンID設定
      SetNextZoneID( p_wk );

      // 次の場所設定
      SetNextPos( p_wk );
    }
    else
    {
      // 受付データを設定する
      SetReceptionWarpData( p_wk );

      // 受け付けエレベータの場所をセット
      SetReceptionElevatorPos( p_wk );
    }
  }
  else if( p_wk->map_change_type == MAP_CHAGE_TYPE_FOOR )
  {
    // 次のフロア設定
    SetNextFloor( p_wk );

    // 次の場所設定
    SetElevatorPos( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  次ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->next_floor ][p_wk->next_room];
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動前ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->now_floor ][p_wk->now_room];
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワープ後のゾーンIDを取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  ROOM_DATA     *p_next_room_data;
  u16 warp_dir    = p_wk->warp_dir;
  u32 room_type;
  p_wk->next_room   = p_wk->now_room;
  p_wk->next_floor  = p_wk->now_floor;  // フロアは移動しない

  switch( warp_dir )
  {
  case WARP_DIR_UP:
    p_wk->next_room -= ROOM_MAX_X;
    break;
  case WARP_DIR_DOWN:
    p_wk->next_room += ROOM_MAX_X;
    break;
  case WARP_DIR_LEFT:
    p_wk->next_room --;
    break;
  case WARP_DIR_RIGHT:
    p_wk->next_room ++;
    break;
  defautl:
    GF_ASSERT(0);
    break;
  }

  // 範囲外の場合はとりあえず今の部屋に飛ぶ
  if( p_wk->next_room < 0 )
  {
    p_wk->next_room = p_wk->now_room;
  }
  else if( p_wk->next_room >= ROOM_MAX )
  {
    p_wk->next_room = p_wk->now_room;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  room_type = p_next_room_data->room_type;
  p_next_map_data->zone_id = p_wk->p_zone_data[ room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );

  ONOUE_Printf("現在の部屋タイプ:%u\n", room_type);
  ONOUE_Printf("現在の部屋番号:%u\n", p_wk->now_room);
  ONOUE_Printf("次の部屋番号:%u\n", p_wk->next_room);
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フロアのエレベーター部屋に移動
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk )
{
  int i;
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  ROOM_DATA     *p_next_room_data;

  // 現在の部屋位置を初期化
  p_wk->next_room = 0;

  // エレベータ部屋を探す
  for( i = 0 ; i < ROOM_MAX ; i++ )
  {
    // 部屋タイプの取得
    ROOM_DATA  *p_room_data;
    p_room_data = GetNextRoomData( p_wk );

    // エレベータ部屋が見つかるまでつづける
    if( p_room_data->room_type == ROOM_TYPE_ELEVATOR )
    {
      break;
    }

    // 次の部屋へ
    p_wk->next_room++;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  p_next_map_data->zone_id = p_wk->p_zone_data[ p_next_room_data->room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );

  // 次のフロアを設定する
  p_wk->now_floor = p_wk->next_floor;
  
  ONOUE_Printf("現在の部屋タイプ:%u\n", p_next_room_data->room_type);
  ONOUE_Printf("現在の部屋番号:%d\n", p_wk->now_room);
  ONOUE_Printf("次の部屋番号:%d\n", p_wk->next_room);
  ONOUE_Printf("次のフロア:%d\n", p_wk->next_floor);

  // エレベータ部屋が無かったら警告
  GF_ASSERT( p_next_room_data->room_type == ROOM_TYPE_ELEVATOR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動方向に対応した場所を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  const GRID_DATA   *p_pos;
  ROOM_DATA         *p_room_data;
  u32               room_type;
  
  // 部屋タイプの取得
  p_room_data = GetNextRoomData( p_wk );
  room_type = p_room_data->room_type;

  p_pos = GetWarpPosTable( p_wk ,room_type ,p_wk->warp_dir  );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付に移動するためのデータを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  // ループタイプに合わせたゾーンを設定する
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  p_next_map_data->zone_id = p_wk->p_zone_data[ ROOM_TYPE_RECEPTION ];
  SetElevatorPos( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンごとのエレベーターの位置を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

  const GRID_DATA *p_pos = GetElevatorPos( p_wk );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付用エレベータの場所をセット
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

  const GRID_DATA *p_pos = GetReceptionElevatorPos( p_wk ,RECEPTION_ELEVATOR_NORMAL);

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標をFx32に変換してセットする
 *
 *	@param	*vec 
 *	@param	x     Xグリッド座標
 *	@param	y     Yグリッド座標
 *	@param	z     Xグリッド座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z )
{
  vec->x = (x * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->y = (y * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->z = (z * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のダンジョンバージョンを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetDungeonColor()
{
  return GET_VERSION();
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応したゾーンIDテーブルを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk )
{
  u32 version = GetDungeonColor();

  if( version == VERSION_B2 )
  {
    return sc_black_zone_table;
  }
  else
  {
    return sc_white_zone_table;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROMバージョンごとのエレベータ位置を取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  u32 version = GetDungeonColor();

  if( version == VERSION_B2 )
  {
    return &sc_black_elevator_pos;
  }
  else
  {
    return &sc_white_elevator_pos;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付のエレベータの場所を取得
 *
 *	@param	*p_wk 
 *	@param	elevator_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk ,int elevator_type )
{
  return &sc_reception_elevator_pos[elevator_type];
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応したワープ後場所テーブルを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetWarpPosTable( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  )
{
  u32 version = GetDungeonColor();

  if( version == VERSION_B2 )
  {
    return &sc_black_warp_pos_table[room_type][warp_dir];
  }
  else
  {
    return &sc_white_warp_pos_table[room_type][warp_dir];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応した配置モデル配置位置取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetBmodelPos( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  )
{
  u32 version = GetDungeonColor();

  if( version == VERSION_B2 )
  {
    return &sc_black_bmodel_pos_table[room_type][warp_dir];
  }
  else
  {
    return &sc_white_bmodel_pos_table[room_type][warp_dir];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定位置の部屋データを取得する
 *
 *	@param	*p_dig_pos
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetDigPosRoomData( DIG_POS *p_dig_pos,ROOM_DATA* p_room_data)
{
  return (&p_room_data[p_dig_pos->x+p_dig_pos->y*ROOM_MAX_Y]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  穴を掘る
 *
 *	@param	dir 
 *	@param	*p_dig_data 
 *	@param	*p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data)
{
  ROOM_DATA *p_now_room_data;
  ROOM_DATA *p_next_room_data;

  // 指定の方向に進む
  switch( dir )
  {
  case INF_MOVE_UP:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_UP] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    break;
  case INF_MOVE_DOWN:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_UP] = TRUE;
    break;
  case INF_MOVE_LEFT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    break;
  case INF_MOVE_RIGHT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,DIG_POS *p_dig_pos)
{
  p_dig_data->data_index++;
  p_dig_data->dig_pos_list[p_dig_data->data_index] = *p_dig_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pop_dig_pos( INF_DIG_DATA *p_dig_data )
{
  if( p_dig_data->data_index >= 0 )
  {
    p_dig_data->now_pos = p_dig_data->dig_pos_list[p_dig_data->data_index];
    p_dig_data->dig_pos_list[p_dig_data->data_index].x = 0;
    p_dig_data->dig_pos_list[p_dig_data->data_index].y = 0;
    p_dig_data->data_index--;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋の扉が全て閉じているかを調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *
 *	@retval TRUE=全て閉じている
 */
//-----------------------------------------------------------------------------
static BOOL CheckCloseAllDoor(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data)
{
  int i;
  ROOM_DATA *p_rd = GetDigPosRoomData( p_dig_pos ,p_room_data);
  for( i = 0; i<4 ;i++ )
  {
    if( p_rd->can_move[i] == TRUE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動可能な方向があるか調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *	@param	*p_array      移動可能先格納変数
 *	@param	mode          
 *
 *	@retval 移動可能な方向数
 */
//-----------------------------------------------------------------------------
static int CheckCanMove(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode)
{
  int i;
  int ret_cnt = 0;
  ROOM_DATA *p_rd;

  // 移動方向のクリア
  GFL_STD_MemFill(p_array, INF_MOVE_MAX, sizeof(int)*INF_MOVE_MAX);

  // 4方向の部屋で全てドアが閉まっている部屋を調べる
  {
    DIG_POS pos = *p_dig_pos;
    pos.x+=1;
    if( pos.x < fs_x )
    {
      // 移動方向の部屋がボス部屋の場合は掘り進まない
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          // 全てのドアが閉まっているかチェックする
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_RIGHT;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_RIGHT;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.x-=1;
    if( pos.x >= 0 )
    {
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
          }
        }
        else
        {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.y+=1;
    if( pos.y < fs_y )
    {
      p_rd= GetDigPosRoomData( &pos ,p_room_data);

      // 移動方向がボス部屋、エレベータ部屋の場合はしたには掘り進まない
      if( (p_rd->room_type != ROOM_TYPE_BOSS) && (p_rd->room_type != ROOM_TYPE_ELEVATOR) )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_DOWN;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_DOWN;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    p_rd= GetDigPosRoomData( &pos ,p_room_data);
    if( p_rd->room_type != ROOM_TYPE_ELEVATOR )   // エレベータ部屋は上に移動することを禁止
    {
      pos.y-=1;
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( pos.y >= 0 )
        {
          if( mode == INF_CHECKMODE_FULL_CLOSE )
          {

            if( CheckCloseAllDoor( &pos ,p_room_data ) )
            {
              p_array[ret_cnt] = INF_MOVE_UP;
              ret_cnt++;
            }
          }
          else
          {
            p_array[ret_cnt] = INF_MOVE_UP;
            ret_cnt++;
          }
        }
      }
    }
  }

  return ret_cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  配列内のデータをランダムにならびかえる
 *
 *	@param	*p_array 
 *	@param	size 
 *	@param	context 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context )
{
  int i;
  u32 tmp;
  for( i = 0; i < size-1 ;i++ )
  {
    u32 rnd = GFL_STD_Rand( rand_context,size);
    tmp = p_array[rnd];
    p_array[rnd] = p_array[i];
    p_array[i] = tmp;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータの設置可能配列を取得する
 *
 *	@param	*p_room_data 
 *	@param	*p_index_array 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval 配列に格納した数
 */
//-----------------------------------------------------------------------------
static int GetElevatorSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y)
{
  DIG_POS pos;
  int cnt = 0;
  int x,y;
  ROOM_DATA *p_rd;
  GFL_STD_MemFill(p_index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);

  // ボス部屋を調べ、配置出る場所リストを作成する
  for( y = 0; y<fs_y; y++ )
  {
    for( x = 0; x<fs_x; x++ )
    {
      // 現在地がボス部屋
      {
        pos.x = x;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 上
      if( y - 1 >= 0 )
      {
        pos.x = x;
        pos.y = y-1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 下
      if( y + 1  < fs_y )
      {
        pos.x = x;
        pos.y = y+1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 左
      if( x - 1 >= 0 )
      {
        pos.x = x-1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x+1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 右
      if( x + 1 < fs_x  )
      {
        pos.x = x+1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x-1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 設置可能位置を保存する
      {
        p_index_array[cnt] = x + y*ROOM_MAX_Y;
        cnt++;
      }
    }
  }
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全て掘り終わったかチェック
 *
 *	@param	*p_room_data 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval TRUE=掘り終わり
 */
//-----------------------------------------------------------------------------
static BOOL CheckDigEnd( ROOM_DATA *p_room_data ,int fs_x ,int fs_y)
{
  int x,y;
  ROOM_DATA *p_rd;
  DIG_POS pos;
  for( y=0; y < fs_y; y++ )
  {
    for( x=0; x < fs_x; x++ )
    {
      pos.x = x;
      pos.y = y;
      p_rd= GetDigPosRoomData( &pos ,p_room_data);

      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if(CheckCloseAllDoor( &pos ,p_room_data ) == TRUE)
        {
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンデータをセットする
 *
 *	@param	*p_dungeon_data 
 *	@param	seed 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y)
{
  int i,j,k;
  DIG_POS elv_pos[FLOOR_MAX];
  DIG_POS boss_pos;
  s32 boss_floor;
  GFL_STD_RandContext rand_context;
  
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );
 
  // 部屋タイプを初期化
  {
    const INF_ROOM_SETUP_PARAM *sc_room_setup = &sc_room_setup_param[area_no];
    for( i = 0; i<FLOOR_MAX; i++ )
    {
      for( j = 0; j<ROOM_MAX; j++ )
      {
        // 初期化
        p_dungeon_data->room_data[i][j].room_id = j;
        p_dungeon_data->room_data[i][j].room_type = ROOM_TYPE_DEFAULT;
        p_dungeon_data->room_data[i][j].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

        // 扉を閉じる
        for( k = 0 ; k < 4 ; k++ )
        {
          p_dungeon_data->room_data[i][j].can_move[k]  = FALSE;
        }

        // 部屋タイプを抽選する
        for( k = 0 ; k < sc_room_setup->member_num; k++ )
        {
          const INF_ROOM_DATA *p_rs_data = &sc_room_setup->room_setup_data[k];
          if( GFL_STD_Rand( &rand_context ,100 ) < p_rs_data->probability)
          {
            p_dungeon_data->room_data[i][j].tr_pos_type = p_rs_data->tr_pos_type;
            p_dungeon_data->room_data[i][j].room_type = sc_room_tr_param[p_rs_data->tr_pos_type].room_type;
            break;
          }
        }
      }
    }
  }
 

  // ボス部屋の設置
  {
    ROOM_DATA *p_rd;
    ROOM_DATA *p_room_data;
    int floor_no;
    floor_no = GFL_STD_Rand( &rand_context ,INFINITYDUNGEON_FLOOR_MAX);
    p_room_data = p_dungeon_data->room_data[floor_no];
    boss_pos.x = GFL_STD_Rand( &rand_context ,fs_x);
    boss_pos.y = GFL_STD_Rand( &rand_context ,fs_y-1); // 一番下の部屋には置けない
    p_rd= GetDigPosRoomData( &boss_pos ,p_room_data);
    p_rd->room_type = ROOM_TYPE_BOSS;
    p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    boss_floor = floor_no;
  }

  // エレベータ設置配列を取得
  {
    for( i = 0; i<FLOOR_MAX; i++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[i];
      int index_array[ROOM_MAX];
      u32 rnd;
      int room_index;
      int max = GetElevatorSetArray(p_room_data,index_array,fs_x,fs_y);
      
      GF_ASSERT_MSG((max > 0),"エレベータを置く場所がありません\n");
      room_index = GFL_STD_Rand( &rand_context ,max);
      p_room_data[index_array[room_index]].room_type    = ROOM_TYPE_ELEVATOR;
      p_room_data[index_array[room_index]].tr_pos_type  = INF_TR_ROOM_POS_TYPE_NONE;

      // エレベーター位置を格納（穴掘りスタート位置に使用）
      elv_pos[i].x = index_array[room_index]%ROOM_MAX_X;
      elv_pos[i].y = index_array[room_index]/ROOM_MAX_Y;
    
      OS_TFPrintf(2,"エレベータ:no=%d\n",index_array[room_index]);
    }
  }

  //ランダム
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[floor_no];
      INF_DIG_DATA dig_data;
      BOOL flg;
      u8   dir_index;
      u8   dir;
      int dir_array[INF_MOVE_MAX];
      dig_data.now_pos = elv_pos[floor_no];
      dig_data.data_index = -1;

      OS_TFPrintf(2,"スタート地点:x=%d,y=%d\n",dig_data.now_pos.x,dig_data.now_pos.y);


      for( i = 0; i< INF_MAX_SEARC_CNT; i++ )
      {
        int move_cnt;
        move_cnt = CheckCanMove( &dig_data.now_pos ,p_room_data,dir_array
            ,fs_x,fs_y,INF_CHECKMODE_FULL_CLOSE);
        OS_TFPrintf(2,"dig_pos:x=%d,y=%d\n",dig_data.now_pos.x,dig_data.now_pos.y);
        if( move_cnt > 0 )
        {
          dir_index = GFL_STD_Rand( &rand_context,move_cnt);
          dir = dir_array[dir_index];
          MoveDigPos(dir ,&dig_data ,p_room_data );
        }
        else
        {
          // 一歩戻る
          OS_TFPrintf(2,"pop\n");
          pop_dig_pos( &dig_data );

          // 終了
          if( CheckDigEnd( p_room_data ,fs_x ,fs_y ) == TRUE )
          {
            OS_TFPrintf(2,"終了\n");
            break;
          }
        }
      }

#ifdef PM_DEBUG
      if( i == INF_MAX_SEARC_CNT )
      {
        GF_ASSERT_MSG(0,"迷路が最後まで生成できませんでした");
      }
#endif

      // ボス部屋への道を空ける
      if( boss_floor == floor_no )
      {
        dig_data.now_pos= boss_pos;
        MoveDigPos(INF_MOVE_DOWN ,&dig_data ,p_room_data );
      }

      // ランダムオープン
      {
        DIG_POS pos;
        int dir_array[INF_MOVE_MAX];
        int move_cnt;
        u8   dir_index;
        INF_DIG_DATA dig_data;
        ROOM_DATA *p_rd;
        dig_data.data_index = -1;

        for( pos.y = 0; pos.y < fs_y ; pos.y++ )
        {
          for( pos.x = 0; pos.x < fs_x ; pos.x++ )
          {
            dig_data.now_pos = pos;
            p_rd= GetDigPosRoomData( &pos ,p_room_data);

            // エレベータ部屋とボス部屋からは新たな道を作らない
            if( (p_rd->room_type != ROOM_TYPE_ELEVATOR) && (p_rd->room_type != ROOM_TYPE_BOSS) )
            {
              move_cnt = CheckCanMove( &pos ,p_room_data,dir_array ,fs_x,fs_y,INF_CHECKMODE_MOVE);
              dir_index = GFL_STD_Rand( &rand_context,move_cnt);
              dir = dir_array[dir_index];
              MoveDigPos(dir ,&dig_data ,p_room_data );
            }
          }
        }
      }

#ifdef PM_DEBUG
      if( g_debug_inf_data_change == TRUE )
      {
        // 部屋データをデバッグ出力
        //INF_DEBUG_OutPutFloorData( p_dungeon_data[floor_no].room_data );

        // 部屋タイプを上書き
        INF_DEBUG_OverwritRoomData(p_dungeon_data->room_data[floor_no]);
      }
#endif
    }
  }

  // 配置人数が32人以上の場合、ランダムで適当な人数まで減らす
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
    {
      int member_num = 0;
      for( i = 0; i< ROOM_MAX; i++ )
      {
        u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
        if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
        {
          if( sc_room_tr_param[pos_type].member_num > 0 )
          {
            member_num += sc_room_tr_param[pos_type].member_num;
          }
        }
      }
      //OS_Printf("RM%2d:%d人\n",floor_no,member_num);

      if( member_num > INFINITY_DUNGEON_TRID_MAX )
      {
        int rmv_num = member_num -= INFINITY_DUNGEON_TRID_MAX;
        RemoveTrRoom( p_dungeon_data , floor_no , rmv_num , &rand_context);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定数トレーナーが減るように部屋タイプを入れ替える
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context )
{
  u8  pos_type;
  int cnt = 0;
  int i;
  u32 index_array[ROOM_MAX];

  GFL_STD_MemFill(index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);
  for( i = 0; i< ROOM_MAX; i++ )
  {
    u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
    if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
    {
      index_array[cnt] = i;
      cnt++;
    }
  }

  // ランダム抽選
  RndArray( index_array ,cnt ,rand_context);

  // 部屋を削ってゆく
  for( i = 0; i< cnt; i++ )
  {
    pos_type = p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type;
    rmv_num -= sc_room_tr_param[pos_type].member_num;
    p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

    if( rmv_num <= 0 )
    {
      OS_Printf("%d部屋削りました\n",i+1);
      return;
    }
  }

  GF_ASSERT_MSG(0,"トレーナーが配置されている部屋がありません\n");
}

static void GetSetBlacTrainerIDTable( u32 *p_trid_array );
static void GetSetWhiteTrainerIDTable( u32 *p_trid_array );

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータ
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	seed          生成用シード
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data , u32 seed )
{
  int i,n;
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  GFL_STD_RandContext rand_context;
  
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );

  // トレーナーIDテーブル取得
  if( GetDungeonColor() == VERSION_B2 )
  {
    GetSetBlacTrainerIDTable( trid_array );
  }
  else
  {
    GetSetWhiteTrainerIDTable( trid_array );
  }

  // トレーナーIDのランダムデータを生成
  for( n = 0; n<FLOOR_MAX; n++)
  {
    // トレーナーデータ配列をランダム化
    RndArray( trid_array ,INF_DUNGEON_TR_DATA_MAX ,&rand_context);

    // 必要な数だけトレーナーIDを保存
    for( i = 0; i<INFINITY_DUNGEON_TRID_MAX; i++)
    {
      p_dungeon_data->tr_data[n].tr_id[i] = trid_array[i];
    }
  }

  // 回復トレーナー配置位置を決める
  {
    int id_pos = 0;
    int room_id_array[ROOM_MAX];
    u16 tr_id;
    u32 room_id_index;
    u32 member_index;
    u32 member_num[ROOM_MAX];
    u32 recover_index = 0;

    for( n = 0; n<FLOOR_MAX; n++)
    {
      int cnt = 0;
      for( i = 0; i<ROOM_MAX; i++)
      {
        u8 pos_type = p_dungeon_data->room_data[n][i].tr_pos_type;
        if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
        {
          if( sc_room_tr_param[pos_type].member_num > 0 )
          {
            room_id_array[cnt] = i;
            member_num[cnt] = sc_room_tr_param[pos_type].member_num;
            cnt++;
          }
        }
      }
      GF_ASSERT_MSG(cnt!=0,"回復トレーナーが置ける場所がありません\n");
      room_id_index = GFL_STD_Rand( &rand_context ,cnt);
      member_index = GFL_STD_Rand( &rand_context ,member_num[room_id_index]);
      id_pos = GetRoomTrainerIDPos( p_dungeon_data ,n ,room_id_array[room_id_index] ,member_index);

      tr_id = INF_TRID_START_RECOVERY + GFL_STD_Rand( &rand_context ,INF_TRID_CNT_RECOVERY);
      p_dungeon_data->tr_data[n].tr_id[id_pos] = tr_id;
      p_dungeon_data->recover_data[recover_index] = tr_id;
      recover_index++;
      OS_Printf("回復トレーナー部屋NO：%u\n",room_id_array[room_id_index]);
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ブラック用トレーナーIDテーブルを取得
 *
 *	@param	*p_trid_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetSetBlacTrainerIDTable( u32 *p_trid_array )
{
  int i;
  u16 tr_id = 0;
  int cnt = 0;
  // ブラックトレーナー
  for( i = 0 ; i < INF_TRID_CNT_BLACK; i++ )
  {
    p_trid_array[cnt] = INF_TRID_START_BLACK + i;
    cnt++;
  }

  // お金トレーナー
  for( i = 0; i < INF_TRID_CNT_MONEY; i++ )
  {
    p_trid_array[cnt] = INF_TRID_START_MONEY+i;
    cnt++;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ホワイト用トレーナーIDテーブルを取得
 *
 *	@param	*p_trid_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetSetWhiteTrainerIDTable( u32 *p_trid_array )
{
  int i;
  u16 tr_id = 0;
  int cnt = 0;
  // ホワイトトレーナー
  for( i = 0 ; i < INF_TRID_CNT_WHITE; i++ )
  {
    p_trid_array[cnt] = INF_TRID_START_WHITE + i;
    cnt++;
  }

  // お金トレーナー
  for( i = 0; i < INF_TRID_CNT_MONEY; i++ )
  {
    p_trid_array[cnt] = INF_TRID_START_MONEY+i;
    cnt++;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  配置モデルで通行禁止の場所をふさぐ
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  //-------------------------------------
  // 封鎖用モデルOBJ(黒)
  //=====================================
  static const BMODEL_ID sc_black_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj01_nsbmd, // 上
    NARC_output_buildmodel_indoor_mugenobj05_nsbmd, // 下
    NARC_output_buildmodel_indoor_mugenobj04_nsbmd, // 右
    NARC_output_buildmodel_indoor_mugenobj06_nsbmd, // 左
  };
  //-------------------------------------
  // 封鎖用モデルOBJ(白)
  //=====================================
  static const BMODEL_ID sc_white_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj02_nsbmd,
    NARC_output_buildmodel_indoor_mugenobj08_nsbmd,
    NARC_output_buildmodel_indoor_mugenobj09_nsbmd,
    NARC_output_buildmodel_indoor_mugenobj07_nsbmd,
  };

  GFL_G3D_OBJSTATUS status;
  const BMODEL_ID *p_bmodel_id_table;
  u32 version = GetDungeonColor();

  // 表示座標を初期化
  status.trans.x = 0;
  status.trans.y = 0;
  status.trans.z = 0;
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );

  // モデルテーブル差し替え
  if( version == VERSION_B2 )
  {
    p_bmodel_id_table = sc_black_bmodel_table;
  }
  else
  {
    p_bmodel_id_table = sc_white_bmodel_table;
  }
  
  // 配置
  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      if( !p_room_data->can_move[INF_MOVE_UP] )
      {
        FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_UP], &status );
      }
    }

    // 下
    if( !p_room_data->can_move[INF_MOVE_DOWN] )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_DOWN], &status );
    }

    // 左
    if( !p_room_data->can_move[INF_MOVE_LEFT]  )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_LEFT], &status );
    }

    // 右
    if( !p_room_data->can_move[INF_MOVE_RIGHT] )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_RIGHT], &status );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトを追加する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  // 上(エレベータ部屋には上封鎖オブジェクトを置くことがない)
  if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
  {
    if( !p_room_data->can_move[INF_MOVE_UP] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
      AddDummyObj( p_wk , p_grid_data );
    }
  }

  // 下
  if( !p_room_data->can_move[INF_MOVE_DOWN] )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_DOWN );
    AddDummyObj( p_wk , p_grid_data );
  }

  // 左
  if( !p_room_data->can_move[INF_MOVE_LEFT] )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_LEFT );
    AddDummyObj( p_wk , p_grid_data );
  }

  // 右
  if( !p_room_data->can_move[INF_MOVE_RIGHT] )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_RIGHT );
    AddDummyObj( p_wk , p_grid_data );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトの追加
 *
 *	@param	*p_wk 
 *	@param	x 
 *	@param	y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void AddDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data)
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys, p_grid_data->x, p_grid_data->z, DIR_UP,
      MMDL_ID_EVENTOBJ_00, NONDRAW, MV_DMY, p_next_map_data->zone_id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内復帰処理
 *
 *	@param	*p_gsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Recovery( GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA  *p_dungeon_data)
{
  MAP_CHAGE_EVENT_WORK  *p_wk;
  ROOM_DATA             *p_room_data;

  // ワーク確保
  p_wk = GFL_HEAP_AllocMemoryLo( HEAPID_FIELDMAP, sizeof( MAP_CHAGE_EVENT_WORK ) );

  // ワーク初期化
  InitWorkData( p_wk ,p_gsys );

  // ダンジョンデータの設定
  p_wk->p_dungeon_data = p_dungeon_data;

  // 配置モデルを配置
  p_room_data = GetNowRoomData( p_wk );
  SetBUildModel(p_wk,p_room_data);
 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド指定用方向定義に変換する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 GetFieldDir( u16 warp_dir )
{
  // 変換
  switch( warp_dir )
  {
  case WARP_DIR_UP:
    return DIR_UP;
  case WARP_DIR_DOWN:
    return DIR_DOWN;
  case WARP_DIR_LEFT:
    return DIR_LEFT;
  case WARP_DIR_RIGHT:
    return DIR_RIGHT;
  case WARP_INFORMATION:
    return WARP_DIR_DOWN;
  default:
    return WARP_DIR_DOWN;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーを配置する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainer( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  int i;
  const INF_ROOM_SET_TR_PARAM *p_tr_room;

  if( p_room_data->tr_pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    p_tr_room = &sc_room_tr_param[p_room_data->tr_pos_type];

    for( i= 0; i<p_tr_room->member_num; i++ )
    {
      const INF_ROOM_TR_DATA *p_md = &p_tr_room->tr_move_data[i];
      NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
      const INF_MMDL_PARAM *p_mmdlp = &sc_mmdl_param[p_md->move_type];
      u16 tr_id;

      tr_id = GetRoomTrainerID(p_wk->p_dungeon_data,p_wk->now_floor, p_room_data->room_id ,i );
      ONOUE_Printf("配置TRID=%u\n",tr_id);
      if( tr_id != INF_DUNGEON_TR_ID_MAX )
      {
        u16 obj_code = FBI_TOOL_GetOBJCode( ARCID_INFINITY_DUGENON_TR , tr_id ,p_wk->heapID);
        MMDL *mmdl = MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys,p_md->g_x, p_md->g_y, p_mmdlp->direction,
            tr_id + TRID_OFFSET_INFINITY_DUNGEON, obj_code, p_mmdlp->move_code,
            p_next_map_data->zone_id );
        MMDL_SetEventType( mmdl , p_mmdlp->ev_type);
        MMDL_SetParam( mmdl, p_mmdlp->param0 , MMDL_PARAM_0 );
        MMDL_SetParam( mmdl, p_mmdlp->param1 , MMDL_PARAM_1 );
        MMDL_SetMoveLimitX( mmdl, p_mmdlp->lim_x );
        MMDL_SetMoveLimitZ( mmdl, p_mmdlp->lim_z );
        MMDL_SetEventID( mmdl, SCRID_INFINITY_DUNGEON_TALK_BATTLE_TRAINER );
        ONOUE_Printf("tr_id;%u\n",tr_id);
      }
    }
  }
  
#ifdef PM_DEBUG
  if( g_debug_inf_data_change != TRUE )
  {
    OS_Printf("トレーナー配置パターン番号：%d\n",p_room_data->tr_pos_type);
  }
  else
  {
    OS_Printf("上書き->レーナー配置パターン番号：%d\n",g_debug_inf_tr_pos_type);
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定部屋IDのトレーナーID格納位置取得
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetRoomTrainerIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int i;
  u32 tr_num = 0;

  for( i = 0; i<ROOM_MAX; i++ )
  {
    u32 tr_type = p_dungeon_data->room_data[floor_id][i].tr_pos_type;
    if( tr_type != INF_TR_ROOM_POS_TYPE_NONE )
    {
      const INF_ROOM_SET_TR_PARAM *p_tr_room = &sc_room_tr_param[tr_type];

      // 指定の部屋まで移動した場合は終了する
      if( room_id <= i )
      {
        break;
      }
      // 人数加算
      tr_num+=p_tr_room->member_num;
    }
  }

  return tr_num+room_tr_no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フロア内のトレーナー番号を調べる
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetRoomTrainerID( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int id_pos = GetRoomTrainerIDPos( p_dungeon_data ,floor_id ,room_id , room_tr_no);
  ONOUE_Printf("room_id;%u\n",room_id);
  ONOUE_Printf("id_pos;%d\n",id_pos);

  // オーバーチェック
  if( INFINITY_DUNGEON_TRID_MAX <= id_pos)
  {
    GF_ASSERT_MSG(0,"トレーナー配置数限界を突破している\n");
    return INF_DUNGEON_TR_ID_MAX;
  }

  return p_dungeon_data->tr_data[floor_id].tr_id[id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナー戦闘イベント
 *
 *	@param	*gsys
 *	@param	*fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u16 obj_id)
{
  GMEVENT   *event;
  GAMEDATA  *p_gdata;
  SAVE_CONTROL_WORK     *p_save_data;
  INFINITYDUNGEON_SAVE  *p_save;
  u32 area_no;

  BSUBWAY_TRAINER_ROM_DATA  *trd;
  BSUBWAY_PARTNER_DATA tr_data;
  HEAPID heapID = HEAPID_FIELDMAP;
  u16 tr_no;
  GFL_STD_RandContext rand_context;

  INF_DUNGEON_DATA *p_dungeon_data;
  u32 id_pos;
  u32 tr_seed;
  u32 floor_id;

  // セーブデータの取得
  p_gdata       = GAMESYSTEM_GetGameData( gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
   
  // 現在のエリア取得
  area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  // OBJIDからtr_noを求める
  tr_no = INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId( obj_id );

  // トレーナーシード
  p_dungeon_data  = INFINITY_DUNGEON_DataGet(fieldmap);
  floor_id        = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  id_pos          = INFINITY_DUNGEON_GetTrIDPos( p_dungeon_data ,floor_id ,obj_id ); // 内部でtr_noに変換している
  tr_seed         = GetTrSeed( p_dungeon_data ,floor_id ,tr_no );

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, tr_seed );
  
  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,tr_no,NARC_message_infinity_trname_dat ,heapID);

  //ポケモンデータをセット
  FBI_TOOL_SetBSWayPokemonParam(
      trd, tr_no , tr_data.btpwd,2,
      ARCID_INFINITY_DUGENON_PD,NULL, NULL, NULL, &rand_context, sc_pow_rnd_table[area_no], heapID);

  GFL_HEAP_FreeMemory( trd );

  {
    // 手持ちパーティー生成
    BATTLE_SETUP_PARAM *dst;
    int i;
    POKEPARTY *party;
	  POKEMON_PARAM * pp;
    const POKEPARTY *myparty;
    u8 member_num;

    myparty = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) ); 
    member_num = PokeParty_GetPokeCount(myparty);
    party = PokeParty_AllocPartyWork( GFL_HEAP_LOWID(heapID) );
	  PokeParty_Init( party, member_num );
    for( i=0; i<member_num; i++ ){
      pp = PokeParty_GetMemberPointer( myparty, i );
      PokeParty_Add( party, pp );
    }

    // バトルパラメータの作成
    dst = FBI_TOOL_INFINITY_DUNGEON_CreateBattleParam(
        gsys, party, 
        member_num , 0 , 2,
        &tr_data,
        NULL ,TRID_OFFSET_INFINITY_DUNGEON ,
        sc_level_table[area_no].level_max ,
        sc_level_table[area_no].level_min , 0 , &rand_context);

    GFL_HEAP_FreeMemory(party);

    event = EVENT_InfinityDungeonTrainerBattle( gsys, fieldmap, dst );
  }
  return( event );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのフロアデータを取得
 *
 *	@param	*p_save       無限ダンジョンセーブ
 *	@param	*p_dungeon_data フロアデータセット先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetFloorData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  int i;
  u32 area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
  INF_DUNGEON_SEED *p_seed = &p_dungeon_data->seed;

  // ダンジョンデータを生成
  SetDungeonData( p_dungeon_data ,area_no ,p_seed->dungeon_data_seed ,5 ,5);
 
  // トレーナーデータをセット
  SetTrainerData( p_dungeon_data ,p_seed->tr_data_seed );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージを取得する
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *	@param	hint_level 
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetHintMessage( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data ,u8 hint_level ,u16 obj_code)
{
  s32 now_floor;
  s32 boss_room;
  s32 boss_floor;
  static const int sub_pattern_num[] = { 0, 0, 0, 0 }; // 各ヒントレベルのパターン数
  
  for( boss_floor = 0; boss_floor<FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( p_dungeon_data->room_data[boss_floor][boss_room].room_type == ROOM_TYPE_BOSS )
      {
        // ボス部屋検索
        break;
      }
    }
  }
  {
    int   talk_pattern;
    int   sub_str_num = 0;
    u32   ret_msg_id;
    BOOL  is_get_msg_id = FALSE;
    now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_save );

    // OBJコードから会話パターンを選択
    talk_pattern = 0;

    // ヒントレベルごとのメッセージIDを取得
    switch( hint_level )
    {
    case INFINITY_DUNGEON_HINT_LEVEL_1:
      {
        is_get_msg_id = TRUE;
        ret_msg_id = msg_hint_level_0_00+(talk_pattern*sub_pattern_num[hint_level]);
      }
      break;
    case INFINITY_DUNGEON_HINT_LEVEL_2:
      {
        if( now_floor != boss_floor )
        {
          // 違う階
          sub_str_num = 1;
        }
        is_get_msg_id = TRUE;
        ret_msg_id = msg_hint_level_1_00_y+(talk_pattern*sub_pattern_num[hint_level])+sub_str_num;
      }
      break;
    case INFINITY_DUNGEON_HINT_LEVEL_3:
      {
        if( now_floor < boss_floor )
        {
          // 上の階
          sub_str_num = 1;
        }
        else if( now_floor > boss_floor )
        {
          // 下の階
          sub_str_num = 2;
        }
        is_get_msg_id = TRUE;
        ret_msg_id = msg_hint_level_2_0_0+(talk_pattern*sub_pattern_num[hint_level])+sub_str_num;
        break;
      }
      break;
    case INFINITY_DUNGEON_HINT_LEVEL_4:
      {
        is_get_msg_id = TRUE;
        ret_msg_id = msg_hint_level_3_00+(talk_pattern*sub_pattern_num[hint_level]);
        break;
      }
      break;
    }
    // メッセージIDを取得できた場合は終了
    if( is_get_msg_id == TRUE )
    {
      return ret_msg_id;
    }
  }

  GF_ASSERT(0);
  return msg_hint_level_0_00; // とぼけるメッセージ
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータの格納場所を取得
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	obj_id            内部でトレーナーIDに変換します
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetTrIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u16 obj_id )
{
  u32 i;
  u16 *p_trid;
  u32 master_seed;

  u16 tr_id = INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId(obj_id);

  // トレーナーIDリストを取得
  p_trid = p_dungeon_data->tr_data[floor_id].tr_id;

  // 検索
  for( i= 0; i<INFINITY_DUNGEON_TRID_MAX; i++ )
  {
    if( p_trid[i] == tr_id )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0,"ID位置が発見できませんでした");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーごとのシードを取得する
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	tr_id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetTrSeed( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos )
{
  return p_dungeon_data->seed.tr_seed[floor_id][tr_id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナーの回復フラグを取得
 *
 *	@param	*p_dungeon_data 
 *	@param	tr_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetRecoverFlgPos( INF_DUNGEON_DATA *p_dungeon_data ,u16 tr_id )
{
  u32 i;
  u32 master_seed;
  u32 *p_trid;

  tr_id = INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId(tr_id);

  // トレーナーIDリストを取得
  p_trid = p_dungeon_data->recover_data;

  // 検索
  for( i= 0; i<INFINITY_DUNGEON_RECOVERTR_MAX; i++ )
  {
    if( p_trid[i] == tr_id )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0,"ID位置が発見できませんでした");
  return 0;
}

#ifdef PM_DEBUG

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置タイプ上書き
 *
 *	@param	*p_room_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INF_DEBUG_OverwritRoomData( ROOM_DATA *p_room_data )
{
  int i;
  // 全ての部屋タイプを固定する
  for( i = 0 ; i < ROOM_MAX ; i++ )
  {
    if( INF_TR_ROOM_POS_TYPE_NONE > g_debug_inf_tr_pos_type )
    {
      // 部屋タイプの取得
      ROOM_DATA  *p_rd = &p_room_data[i];
      p_rd->tr_pos_type = g_debug_inf_tr_pos_type;
      p_rd->room_type = sc_room_tr_param[g_debug_inf_tr_pos_type].room_type;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フロアデータのデバッグ出力
 *
 *	@param	*p_room_data 
 *	@param	p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INF_DEBUG_OutPutFloorData( ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save )
{
  int i,j,k;
  int draw;

  for( i = 0; i<ROOM_MAX_Y; i++ )
  {
    for( draw = 0 ; draw <= 4; draw++ )
    {
      for( j = 0; j<ROOM_MAX_X; j++ )
      {
        int room_id = i*ROOM_MAX_Y + j;
        switch( draw )
        {
          // 上の列
        case 0:
          {
            OS_TFPrintf( 3,"■■");
            if( p_room_data[room_id].can_move[INF_MOVE_UP] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            OS_TFPrintf( 3,"■■");
          }
          break;
        case 1:
          {
            if( p_room_data[room_id].room_type == ROOM_TYPE_BOSS)
            {
              OS_TFPrintf( 3,"■Ｂ　　■");
            }
            else if( p_room_data[room_id].room_type == ROOM_TYPE_ELEVATOR)
            {
              OS_TFPrintf( 3,"■Ｅ　　■");
            }
            else
            {
              OS_TFPrintf( 3,"■　　　■");
            }
          }
          break;
          // 真ん中の列
        case 2:
          {
            u8 now_room_id = INFINITYDUNGEON_SAVE_GetRoomID( p_save );
            if( p_room_data[room_id].can_move[INF_MOVE_LEFT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            // 現在地表示
            if( room_id == now_room_id )
            {
              OS_TFPrintf( 3,"　主　"); //部屋の中心
            }
            else
            {
              OS_TFPrintf( 3,"　　　"); //部屋の中心
            }

            if( p_room_data[room_id].can_move[INF_MOVE_RIGHT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }
          }
          break;
        case 3:
          {
            OS_TFPrintf( 3,"■%2d　　■",room_id);
          }
          break;
          // 下の列
        case 4:
          {
            OS_TFPrintf( 3,"■■");
            if( p_room_data[room_id].can_move[INF_MOVE_DOWN] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            OS_TFPrintf( 3,"■■");
          }
          break;
        }
      }
      OS_TFPrintf( 3,"\n");
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン生成テスト
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void DEBUG_INFINITY_DUNGEON_MakeDungeonData()
{
  s32 seed;
  s32 area;
  INF_DUNGEON_DATA dungeon_data;
  seed = GFUser_GetPublicRand( GFL_STD_RAND_MAX );
  area = GFUser_GetPublicRand( INFINITY_DUNGEON_AREA_ID_MAX );
  OS_Printf("Seed = %u--------------\n",seed);
  OS_TFPrintf(2,"Seed = %u--------------\n",seed);
  OS_TFPrintf(3,"Seed = %u--------------\n",seed);
  // ダンジョンデータの生成
  OS_TFPrintf(3,"3x3--------------------\n");
  SetDungeonData( &dungeon_data ,area ,seed ,3 ,3);
  OS_TFPrintf(3,"4x4--------------------\n");
  SetDungeonData( &dungeon_data ,area ,seed ,4 ,4);
  OS_TFPrintf(3,"5x3--------------------\n");
  SetDungeonData( &dungeon_data ,area ,seed ,5 ,5);
  OS_Printf("end--------------------\n");
  OS_TFPrintf(2,"end--------------------\n");
  OS_TFPrintf(3,"end--------------------\n");
}
#endif
