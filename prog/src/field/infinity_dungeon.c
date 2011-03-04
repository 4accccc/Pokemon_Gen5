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
#include "field/infinity_dungeon_def.h"

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
  ZONE_ID_D31R0801, // 特殊部屋2
  ZONE_ID_D31R1001, // 特殊部屋1
  ZONE_ID_D31R1201, // デフォルト
  ZONE_ID_D31R0201, // 受付
};

//-------------------------------------
// ゾーンテーブル(ホワイト)
//=====================================
static const u16 sc_white_zone_table[] = {
  ZONE_ID_D31R0301, // エレベーター
  ZONE_ID_D31R0501, // ボス
  ZONE_ID_D31R0701, // 特殊部屋2
  ZONE_ID_D31R0901, // 特殊部屋1
  ZONE_ID_D31R1101, // デフォルト
  ZONE_ID_D31R0101, // 受付
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ブラック)
//=====================================
static const GRID_DATA sc_black_elevator_pos = {
  6 , 0 , 8 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_elevator_pos = {
  6 , 0 , 8 ,
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
///	ルームデータ
//=====================================
typedef struct {
  u32 room_type;
  BOOL can_up_move;
  BOOL can_down_move;
  BOOL can_left_move;
  BOOL can_right_move;
}ROOM_DATA;

//-------------------------------------
///	フロアデータ
//=====================================
typedef struct {
  ROOM_DATA room_data[ROOM_MAX];
}FLOOR_DATA;

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

  // 移動タイプ
  u8                    map_change_type;

  // マップデータ
  FLOOR_DATA            floor_data[FLOOR_MAX];

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
static u32  GetDungeonColor( MAP_CHAGE_EVENT_WORK *p_wk );
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk );

// ゾーン依存
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk );
static u16 GetFieldDir( u16 warp_dir );

// 部屋データ
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk );
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン生成
static void SetDungeonData( MAP_CHAGE_EVENT_WORK *p_wk );
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
 *	@brief  指定のフロアにワープする
 *
 *	@param	* gsys
 *	@param	warp_dir 
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
  p_wk->p_gsys        = p_gsys;
  p_wk->p_field_work  = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata       = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data   = GetZoneIdTable( p_wk );
  p_wk->now_room      = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save ) % INFINITYDUNGEON_FLOOR_MAX; // エリア内のフロアに変換
  p_wk->p_g3d_mapper  = FIELDMAP_GetFieldG3Dmapper( p_wk->p_field_work );
  p_wk->p_bmodel_man  = FLDMAPPER_GetBuildModelManager( p_wk->p_g3d_mapper );
  p_wk->p_mmdlsys     = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
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

      // ダンジョンデータの生成
      SetDungeonData( p_wk );

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

      // 受付の場合は配置モデルを置かない
      if( p_wk->warp_dir != WARP_INFORMATION )
      {
        // 次の部屋データを取得する
        p_next_room = GetNextRoomData( p_wk );

        // 配置モデルを配置
        SetBUildModel(p_wk,p_next_room);

        // 封鎖用OBJ配置
        SetDummyObj(p_wk,p_next_room);
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
  p_room_data = GetNowRoomData( p_wk );
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
  p_wk->warp_dir = WARP_DIR_DOWN;
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
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  int i,j;

  // 適当なデータで初期化
  for( i = 0; i<FLOOR_MAX; i++ )
  {
    for( j = 0; j<ROOM_MAX; j++ )
    {
      p_wk->floor_data[i].room_data[j].room_type = ROOM_TYPE_DEFAULT;
      p_wk->floor_data[i].room_data[j].can_up_move    = TRUE;
      p_wk->floor_data[i].room_data[j].can_down_move  = TRUE;
      p_wk->floor_data[i].room_data[j].can_left_move  = TRUE;
      p_wk->floor_data[i].room_data[j].can_right_move = TRUE;
    }
    
    // エレベータ部屋設置
    p_wk->floor_data[i].room_data[i+3].room_type = ROOM_TYPE_ELEVATOR;
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
  BMODEL_ID bmodel_id = NARC_output_buildmodel_indoor_mball01_nsbmd;
  GFL_G3D_OBJSTATUS status;

  // 上
  if( !p_room_data->can_up_move )
  {
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
    SetGridPos(&status.trans,p_grid_data->x,p_grid_data->y,p_grid_data->z);
    FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, bmodel_id, &status );
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
    // 上
  if( !p_room_data->can_up_move )
  {
    // ダミーオブジェクトで封鎖
    const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
    AddDummyObj( p_wk , p_grid_data );
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
  SetDungeonData( &wk );

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

