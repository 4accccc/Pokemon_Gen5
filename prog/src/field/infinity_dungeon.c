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
//	部屋タイプ
//=====================================
enum{
  //ROOM_TYPE_RECEPTION,
  ROOM_TYPE_ELEVATOR,
  ROOM_TYPE_BOSS,
  ROOM_TYPE_SPECIAL1,
  ROOM_TYPE_SPECIAL2,
  ROOM_TYPE_DEFAULT,
  ROOM_TYPE_MAX,
};

//-------------------------------------
// ゾーンテーブル(ブラック)
//=====================================
static const u16 sc_black_zone_table[] = {
  //ZONE_ID_D31R0201, // 受付
  ZONE_ID_D31R0401, // エレベーター
  ZONE_ID_D31R0601, // ボス
  ZONE_ID_D31R0801, // 特殊部屋2
  ZONE_ID_D31R1001, // 特殊部屋1
  ZONE_ID_D31R1201, // デフォルト
};

//-------------------------------------
// ゾーンテーブル(ホワイト)
//=====================================
static const u16 sc_white_zone_table[] = {
  //ZONE_ID_D31R0101, // 受付
  ZONE_ID_D31R0301, // エレベーター
  ZONE_ID_D31R0501, // ボス
  ZONE_ID_D31R0701, // 特殊部屋2
  ZONE_ID_D31R0901, // 特殊部屋1
  ZONE_ID_D31R1101, // デフォルト
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
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  GAMEDATA              *p_gdata;
  FIELDMAP_WORK         *p_fieldWork;
  INFINITYDUNGEON_SAVE  *p_save;

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

}MAP_CHAGE_EVENT_WORK;



//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void InitWorkData( GMEVENT *event, MAP_CHAGE_EVENT_WORK *p_wk );
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk );
static u32  GetDungeonColor( MAP_CHAGE_EVENT_WORK *p_wk );
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetDungeonData( MAP_CHAGE_EVENT_WORK *p_wk );
static const GRID_DATA* GetWarpPosTable( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  );
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk);


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
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントデータから必要データを取得する
 *
 *	@param	*event
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitWorkData( GMEVENT *event, MAP_CHAGE_EVENT_WORK *p_wk )
{
  SAVE_CONTROL_WORK* p_save_data;
  p_wk->p_gsys = GMEVENT_GetGameSysWork( event );
  p_wk->p_fieldWork = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data  = GetZoneIdTable( p_wk );
  p_wk->now_room    = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor   = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
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

  switch( *seq )
  {
  case 0:
    {
      // データ取得
      InitWorkData( event, p_wk);

      // ダンジョンデータの生成
      SetDungeonData( p_wk );

      // フェードアウト
      PMSND_PlaySE( SEQ_SE_KAIDAN );
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_fieldWork ) );
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_PlayerDir( p_wk->p_gsys, p_wk->p_fieldWork ) );
      (*seq)++;
      break;
    }
  case 1:
    {
      // マップ移動
      NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
      SetNextZoneData( p_wk );
      GMEVENT_CallEvent(event,
          EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_fieldWork, p_next_map_data->zone_id, &p_next_map_data->warp_pos, DIR_UP));
      (*seq)++;
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_fieldWork );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        SetBUildModel(p_wk);
        break;
      }
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_fieldWork ) );
      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_fieldWork));

      ONOUE_Printf("ワープ完了\n");
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
  // 次のゾーンID設定
  SetNextZoneID( p_wk );

  // 次の場所設定
  SetNextPos( p_wk );
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
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  u16 warp_dir    = p_wk->warp_dir;
  u32 room_type;
  p_wk->next_room   = p_wk->now_room;

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

  // ループタイプに合わせたゾーンを設定する
  room_type = p_wk->floor_data[ p_wk->now_floor ].room_data[p_wk->next_room].room_type;
  p_next_map_data->zone_id = p_wk->p_zone_data[ room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );

  ONOUE_Printf("現在の部屋タイプ:%u\n", room_type);
  ONOUE_Printf("現在の部屋番号:%u\n", p_wk->now_room);
  ONOUE_Printf("次の部屋番号:%u\n", p_wk->next_room);
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
  u32           room_type;
  
  // 部屋タイプの取得
  room_type = p_wk->floor_data[ p_wk->now_floor ].room_data[p_wk->now_room].room_type;

  p_pos = GetWarpPosTable( p_wk ,room_type ,p_wk->warp_dir  );


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
 *	@brief  ダンジョンデータをセットする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  // ダミーデータを入れる
  p_wk->floor_data[0].room_data[0].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[1].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[2].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[3].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[4].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[5].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[6].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[7].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[8].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[9].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[10].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[11].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[12].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[13].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[14].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[15].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[16].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[17].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[18].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[19].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[20].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[21].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[22].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[23].room_type = ROOM_TYPE_DEFAULT;
  p_wk->floor_data[0].room_data[24].room_type = ROOM_TYPE_DEFAULT;
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
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk)
{
//  // 配置モデルテスト
//  BMODEL_ID bmodel_id = NARC_output_buildmodel_indoor_mball01_nsbmd;
//  FLDMAPPER*   g3d_mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldWork );
//  FIELD_BMODEL_MAN* bmMan  = FLDMAPPER_GetBuildModelManager( g3d_mapper );
//  GFL_G3D_OBJSTATUS status;
//  VecFx32 pos;
//  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
//  GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
//  SetGridPos(&status.trans,6,0,8);
//  FIELD_BMODEL_Create_Direct( bmMan, bmodel_id, &status );
}

