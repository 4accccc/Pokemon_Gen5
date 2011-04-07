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
#include "system/main.h"  //HEAPID_PROC
#include "message.naix"
#include "tr_tool/ex_trainer_def.h"
#include "msg/script/msg_infinity_dungeon_scr.h"

#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.cdat"

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
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
  u32 tr_pos_type;
  u32 room_type;
  BOOL can_up_move;
  BOOL can_down_move;
  BOOL can_left_move;
  BOOL can_right_move;
  u32  room_id;
}ROOM_DATA;

//-------------------------------------
///	ダンジョン内トレーナーデータ
//=====================================
typedef struct{
  u32 tr_id[INFINITY_DUNGEON_TRID_MAX]; // 全トレーナーランダム配列
}DUNGEON_TRAINER;

//-------------------------------------
///	フロアデータ
//=====================================
typedef struct{
  ROOM_DATA room_data[ROOM_MAX];
  DUNGEON_TRAINER tr_data;
}FLOOR_DATA;

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
//	マップ移動タイプ
//=====================================
enum{
  MAP_CHAGE_TYPE_ROOM,
  MAP_CHAGE_TYPE_FOOR,
  MAP_CHAGE_TYPE_MAX,
};

//-------------------------------------
//	部屋タイプ
//=====================================
enum{
  ROOM_TYPE_ELEVATOR,
  ROOM_TYPE_BOSS,
  ROOM_TYPE_SPECIAL1,
  ROOM_TYPE_SPECIAL2,
  ROOM_TYPE_DEFAULT,
  ROOM_TYPE_RECEPTION,
  ROOM_TYPE_MAX,
};

//-------------------------------------
//	受付のエレベータID
//=====================================
enum{
  RECEPTION_ELEVATOR_NORMAL,
  RECEPTION_ELEVATOR_EX,
};

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
  6 , 0 , 1 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_elevator_pos = {
  6 , 0 , 1 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(受付)
//=====================================
static const GRID_DATA sc_reception_elevator_pos[] = {
  7 , 0 , 1 , // 左
  11, 0 , 1 , // 右
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
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  VecFx32 warp_pos;       // 遷移先でのマップ位置
  s32     zone_id;        // 遷移先ゾーンID
}NEXT_MAP_DATA;

//-------------------------------------
///	マップ復帰用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  GAMEDATA              *p_gdata;
  FIELDMAP_WORK         *p_field_work;
  INFINITYDUNGEON_SAVE  *p_save;
  FLDMAPPER             *p_g3d_mapper;
  FIELD_BMODEL_MAN      *p_bmodel_man;
  MMDLSYS               *p_mmdlsys;
 

  // マップデータ
  FLOOR_DATA            floor_data[FLOOR_MAX];
  const u16             *p_zone_data;

  // フロア
  s32 now_room;
  s32 now_floor;


}MAP_INIT_EVENT_WORK;

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
  HEAPID                heapID;

  // 移動タイプ
  u8                    map_change_type;

  // マップデータ
  FLOOR_DATA            floor_data[FLOOR_MAX];

  // ダンジョン生成用シード
  s32 seed;

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

  // 乱数コンテキスト
  GFL_STD_RandContext rand_context;

}MAP_CHAGE_EVENT_WORK;



//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// 共通
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys);
static u32  GetDungeonColor( MAP_CHAGE_EVENT_WORK *p_wk );
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk );
static void GetFloorData( INFINITYDUNGEON_SAVE  *p_save ,FLOOR_DATA *p_floor_data );

// ゾーン依存
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk );
static u16 GetFieldDir( u16 warp_dir );

// 部屋データ
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk );
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン生成
static void SetDungeonData( FLOOR_DATA *p_floor_data , u32 seed );
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
static int GetRoomTrainerIDPos( FLOOR_DATA *p_floor_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static u32 GetRoomTrainerID( FLOOR_DATA *p_floor_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static void SetTrainerData( FLOOR_DATA *p_floor_data , u32 seed );
static void SetTrainer( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップ位置から次のマップを導き、ワープさせる
 *
 *	@param	* gsys
 *	@param	warp_dir 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys, u16 warp_dir )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK *p_wk;
 	
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_work, p_event );
	p_wk = GMEVENT_GetEventWork( p_event );
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
void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys, int floor_no )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK *p_wk;
 	
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_work, p_event );
	p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->next_floor = floor_no % INFINITYDUNGEON_FLOOR_MAX; //エリア内のフロアに変換

  if( floor_no == 0 )
  {
    p_wk->now_area = 0;
  }
  else
  {
    p_wk->now_area = floor_no / INFINITYDUNGEON_FLOOR_MAX; //エリア内のフロアに変換
  }
  p_wk->map_change_type = MAP_CHAGE_TYPE_FOOR;
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
  p_wk->heapID        = HEAPID_PROC;
  p_wk->p_gsys        = p_gsys;
  p_wk->p_field_work  = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata       = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data   = GetZoneIdTable( p_wk );
  p_wk->now_room      = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save ) % INFINITYDUNGEON_FLOOR_MAX; // エリア内のフロアに変換
  p_wk->p_g3d_mapper  = FIELDMAP_GetFieldG3Dmapper( p_wk->p_field_work );
  p_wk->seed          = INFINITYDUNGEON_SAVE_GetRandomSeed( p_wk->p_save );
  p_wk->p_bmodel_man  = FLDMAPPER_GetBuildModelManager( p_wk->p_g3d_mapper );
  p_wk->p_mmdlsys     = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
  GFL_STD_RandInit( &p_wk->rand_context, p_wk->seed );

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

      // フロアデータの取得
      GetFloorData( p_wk->p_save ,p_wk->floor_data);

      // フェードアウト
      PMSND_PlaySE( SEQ_SE_KAIDAN );
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
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
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
  return &p_wk->floor_data[ p_wk->next_floor ].room_data[p_wk->next_room];
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
  return &p_wk->floor_data[ p_wk->now_floor ].room_data[p_wk->now_room];
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
    ONOUE_Printf("WARP_DIR_UP\n");
    p_wk->next_room -= ROOM_MAX_X;
    break;
  case WARP_DIR_DOWN:
    ONOUE_Printf("WARP_DIR_DOWN\n");
    p_wk->next_room += ROOM_MAX_X;
    break;
  case WARP_DIR_LEFT:
    ONOUE_Printf("WARP_DIR_LEFT\n");
    p_wk->next_room --;
    break;
  case WARP_DIR_RIGHT:
    ONOUE_Printf("WARP_DIR_RIGHT\n");
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
  INFINITYDUNGEON_SAVE_SetFloorID( p_wk->p_save , p_wk->next_floor );
  
  ONOUE_Printf("現在の部屋タイプ:%u\n", p_next_room_data->room_type);
  ONOUE_Printf("現在の部屋番号:%u\n", p_wk->now_room);
  ONOUE_Printf("次の部屋番号:%u\n", p_wk->next_room);

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
static u32 GetDungeonColor( MAP_CHAGE_EVENT_WORK *p_wk )
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
  u32 version = GetDungeonColor( p_wk );

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
  u32 version = GetDungeonColor( p_wk );

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
  u32 version = GetDungeonColor( p_wk );

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
  u32 version = GetDungeonColor( p_wk );

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
 *	@brief  ダンジョンデータをセットする
 *
 *	@param	*p_floor_data 
 *	@param	seed 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonData( FLOOR_DATA *p_floor_data , u32 seed )
{
  int i,j;

  // 適当なデータで初期化
  for( i = 0; i<FLOOR_MAX; i++ )
  {
    for( j = 0; j<ROOM_MAX; j++ )
    {
      p_floor_data[i].room_data[j].room_type = ROOM_TYPE_DEFAULT;
      
      // 全部オープン
      p_floor_data[i].room_data[j].can_up_move  = TRUE;
      p_floor_data[i].room_data[j].can_down_move  = TRUE;
      p_floor_data[i].room_data[j].can_left_move  = TRUE;
      p_floor_data[i].room_data[j].can_right_move = TRUE;

      if( j%5 == 4  )
      {
        // 右端
        p_floor_data[i].room_data[j].can_right_move    = FALSE;
      }
      else if( j%5 == 0 )
      {
        // 左端
        p_floor_data[i].room_data[j].can_left_move    = FALSE;
      }

      // 上の段
      if( j < 5 )
      {
        p_floor_data[i].room_data[j].can_up_move    = FALSE;
      }else{
        p_floor_data[i].room_data[j].can_up_move    = TRUE;
      }
      
      // 下の段
      if( j >= ROOM_MAX-5 )
      {
        p_floor_data[i].room_data[j].can_down_move    = FALSE;
      }else{
        p_floor_data[i].room_data[j].can_down_move    = TRUE;
      }

      // 部屋情報設定
      p_floor_data[i].room_data[j].tr_pos_type = GFUser_GetPublicRand(INF_TR_ROOM_POS_TYPE_MAX);
      p_floor_data[i].room_data[j].room_id = j;

    }
    // エレベータ部屋設置
    p_floor_data[i].room_data[0].room_type = ROOM_TYPE_ELEVATOR;
  }

  p_floor_data[0].room_data[1].room_type = ROOM_TYPE_DEFAULT;
  p_floor_data[0].room_data[2].room_type = ROOM_TYPE_SPECIAL1;
  p_floor_data[0].room_data[3].room_type = ROOM_TYPE_SPECIAL2;

  // ボス部屋設置
  p_floor_data[0].room_data[4].room_type = ROOM_TYPE_BOSS;
}


//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータ
 *
 *	@param	*p_floor_data フロアデータ
 *	@param	seed          生成用シード
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainerData( FLOOR_DATA *p_floor_data , u32 seed )
{
  int i;
  for( i = 0; i<INFINITY_DUNGEON_TRID_MAX; i++)
  {
    p_floor_data->tr_data.tr_id[i] = i;
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
  BMODEL_ID bmodel_id = NARC_output_buildmodel_indoor_mugenobj01_nsbmd;
  GFL_G3D_OBJSTATUS status;

  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      if( !p_room_data->can_up_move )
      {
        const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
        VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
        GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
        SetGridPos(&status.trans,p_grid_data->x,p_grid_data->y,p_grid_data->z);
        FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, bmodel_id, &status );
      }
    }

    // 下
    if( !p_room_data->can_down_move )
    {
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_DOWN );
      VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
      GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
      SetGridPos(&status.trans,p_grid_data->x,p_grid_data->y,p_grid_data->z);
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, bmodel_id, &status );
    }

    // 左
    if( !p_room_data->can_left_move )
    {
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_LEFT );
      VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
      GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
      SetGridPos(&status.trans,p_grid_data->x,p_grid_data->y,p_grid_data->z);
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, bmodel_id, &status );
    }

    // 右
    if( !p_room_data->can_right_move )
    {
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_RIGHT );
      VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
      GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
      SetGridPos(&status.trans,p_grid_data->x,p_grid_data->y,p_grid_data->z);
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, bmodel_id, &status );
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
    if( !p_room_data->can_up_move )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
      AddDummyObj( p_wk , p_grid_data );
    }
  }

  // 下
  if( !p_room_data->can_down_move )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_DOWN );
    AddDummyObj( p_wk , p_grid_data );
  }

  // 左
  if( !p_room_data->can_left_move )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_LEFT );
    AddDummyObj( p_wk , p_grid_data );
  }

  // 右
  if( !p_room_data->can_right_move )
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
 *	@param	*p_scr_work
 *	@param	*p_gsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Recovery( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys )
{
  MAP_CHAGE_EVENT_WORK  wk;
  ROOM_DATA             *p_room_data;
  
  // ワーク初期化
  InitWorkData( &wk ,p_gsys );

  // ダンジョンデータの生成
  SetDungeonData( wk.floor_data ,wk.seed );

  // 配置モデルを配置
  p_room_data = GetNowRoomData( &wk );
  SetBUildModel(&wk,p_room_data);

  OS_Printf("初期化\n");
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
  p_tr_room = &sc_room_tr_param[p_room_data->tr_pos_type];

  for( i= 0; i<p_tr_room->member_num; i++ )
  {
    const INF_ROOM_TR_DATA *p_md = &p_tr_room->tr_move_data[i];
    NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
    const INF_MMDL_PARAM *p_mmdlp = &sc_mmdl_param[p_md->move_type];

    u32 tr_id = GetRoomTrainerID(p_wk->floor_data,p_wk->now_floor, p_room_data->room_id ,i );
    u16 obj_code = FBI_TOOL_GetOBJCode( ARCID_INFINITY_DUGENON_TR , tr_id ,p_wk->heapID);
    MMDL *mmdl = MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys,p_md->g_x, p_md->g_y, p_mmdlp->direction,
        tr_id + TRID_OFFSET_INFINITY_DUNGEON, obj_code, p_mmdlp->move_code,
        p_next_map_data->zone_id );
    MMDL_SetEventType( mmdl , p_mmdlp->ev_type);
    MMDL_SetParam( mmdl, p_mmdlp->param0 , MMDL_PARAM_0 );
    MMDL_SetParam( mmdl, p_mmdlp->param1 , MMDL_PARAM_1 );
    MMDL_SetMoveLimitX( mmdl, p_mmdlp->lim_x );
    MMDL_SetMoveLimitZ( mmdl, 2 );
    MMDL_SetEventID( mmdl, SCRID_INFINITY_DUNGEON_TALK_BATTLE_TRAINER );
    ONOUE_Printf("tr_id;%u\n",tr_id);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定部屋IDのトレーナーID格納位置取得
 *
 *	@param	*p_floor_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetRoomTrainerIDPos( FLOOR_DATA *p_floor_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int i;
  u32 tr_num = 0;

  for( i = 0; i<ROOM_MAX; i++ )
  {
    u32 tr_type = p_floor_data[floor_id].room_data[i].tr_pos_type;
    const INF_ROOM_SET_TR_PARAM *p_tr_room = &sc_room_tr_param[tr_type];

    // 指定の部屋まで移動した場合は終了する
    if( room_id <= i )
    {
      break;
    }
    // 人数加算
    tr_num+=p_tr_room->member_num;
  }

  return tr_num+room_tr_no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フロア内のトレーナー番号を調べる
 *
 *	@param	*p_floor_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetRoomTrainerID( FLOOR_DATA *p_floor_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int id_pos = GetRoomTrainerIDPos( p_floor_data ,floor_id ,room_id , room_tr_no);
  ONOUE_Printf("room_id;%u\n",room_id);
  ONOUE_Printf("id_pos;%d\n",id_pos);

  // オーバーチェック
  if( INFINITY_DUNGEON_TRID_MAX <= id_pos)
  {
    id_pos = INFINITY_DUNGEON_TRID_MAX-1;
    ONOUE_Printf("トレーナー配置数限界を突破している\n");
  }

  return p_floor_data->tr_data.tr_id[id_pos];
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
  GMEVENT * event;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  BSUBWAY_PARTNER_DATA tr_data;
  HEAPID heapID = HEAPID_PROC;
  u16 tr_no;
  u8 pow_rnd;
  GFL_STD_RandContext rand_context;

  // OBJIDからtr_idを求める
  tr_no = INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId( obj_id );

  ONOUE_Printf("tr_no;%u\n",tr_no);

  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,tr_no,NARC_message_infinity_trname_dat ,heapID);

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, 5 );

  // パワー乱数を決定する
  pow_rnd=31;

  //ポケモンデータをセット
  FBI_TOOL_SetBSWayPokemonParam(
      trd, tr_no , tr_data.btpwd,2,
      ARCID_INFINITY_DUGENON_PD,NULL, NULL, NULL, &rand_context, pow_rnd, heapID);

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
        20 , 10 , 0 , &rand_context);

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
 *	@param	*p_floor_data フロアデータセット先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetFloorData( INFINITYDUNGEON_SAVE *p_save ,FLOOR_DATA *p_floor_data )
{
  s32 seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_save );
  // ダンジョンデータの生成
  SetDungeonData( p_floor_data ,seed );

  // トレーナーデータをセット
  SetTrainerData( p_floor_data ,seed );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージを取得する
 *
 *	@param	*p_save 
 *	@param	hint_level 
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetHintMessage( INFINITYDUNGEON_SAVE *p_save ,u8 hint_level ,u16 obj_code)
{
  s32 now_floor;
  s32 boss_room;
  s32 boss_floor;
  FLOOR_DATA floor_data[FLOOR_MAX];
  static const int sub_pattern_num[] = { 0, 0, 0, 0 }; // 各ヒントレベルのパターン数

  // フロアデータの取得
  GetFloorData( p_save ,floor_data);

  for( boss_floor = 0; boss_floor<FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( floor_data[boss_floor].room_data[boss_room].room_type == ROOM_TYPE_BOSS )
      {
        int  talk_pattern;
        int  sub_str_num = 0;
        now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_save ) % INFINITYDUNGEON_FLOOR_MAX; // エリア内のフロアに変換

        // OBJコードから会話パターンを選択
        talk_pattern = 0;

        // ヒントレベルごとのメッセージIDを取得
        switch( hint_level )
        {
        case INFINITY_DUNGEON_HINT_LEVEL_1:
          {
            return msg_hint_level_0_00+(talk_pattern*sub_pattern_num[hint_level]);
          }
          break;
        case INFINITY_DUNGEON_HINT_LEVEL_2:
          {
            if( now_floor != boss_floor )
            {
              // 違う階
              sub_str_num = 1;
            }
            return msg_hint_level_1_00_y+(talk_pattern*sub_pattern_num[hint_level])+sub_str_num;
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
            return msg_hint_level_2_0_0+(talk_pattern*sub_pattern_num[hint_level])+sub_str_num;
          }
          break;
        case INFINITY_DUNGEON_HINT_LEVEL_4:
          {
            return msg_hint_level_3_00+(talk_pattern*sub_pattern_num[hint_level]);
          }
          break;
        }
      }
    }
  }

  GF_ASSERT(0);
  return msg_hint_level_0_00; // とぼけるメッセージ
}

