//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_tool.c
 *	@brief  無限ダンジョン：ツール
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *	@note   vsプロジェクトでもソースを流用し、テストを行うことを考慮し設計します
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "infinity_dungeon_def.h"
#include "infinity_dungeon_tool.h"
#include "field/field_dir.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
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

// 配置データ
#include "fldmmdl_code.h"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.h"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.cdat"
// トレーナーデータ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_area_data.cdat"
// ダンジョン関連データ
#include "infinity_dungeon_tool.cdat"
#include "../../../resource/infinity_dungeon/data/inf_area_data.cdat"
#include "../../../resource/infinity_dungeon/data/inf_area_tr_cnt.cdat"

#ifndef VS_PROJECT
SDK_COMPILER_ASSERT( INF_TR_AREA_DATA_MAX == (INF_DUNGEON_TR_ID_MAX -INF_TRID_CNT_RECOVERY) );
#endif

//----------------------------------------------------------------------------
/**
 *	@brief 動作モデルパラメータを取得
 *
 *	@param	move_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const INF_MMDL_PARAM* INF_TOOL_GetMMDLParam( u16 move_type )
{
  return &sc_mmdl_param[move_type];
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋配置トレーナーパラメータ取得
 *
 *	@param	tr_pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const INF_ROOM_SET_TR_PARAM* INF_TOOL_GetRoomTrParam( u16 tr_pos_type )
{
  return &sc_room_tr_param[tr_pos_type];
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー抽選パラメータ取得
 *
 *	@param	tr_pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const INF_TR_AREA_DATA* INF_TOOL_GetTrainerParam( u16 tr_index )
{
  return &sc_tr_area_data[tr_index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナーインデックスからトレーナータイプを取得する
 *
 *	@param	tr_index 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetTrainerType( u16 tr_index )
{
  return sc_tr_area_data[tr_index].tr_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置パターンから配置人数を取得する
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetMemberNum( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].member_num;
  }
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置パターンから部屋タイプを取得
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetRoomType( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].room_type;
  }
  return ROOM_TYPE_DEFAULT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋ごとのバージョン条件を取得
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetRoomVersion( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].version;
  }
  return INF_MULTI_DUNGEON;
}

//----------------------------------------------------------------------------
/**
 *	@brief 指定エリアの広さを取得する
 *
 *	@param	area_no 
 *	@param	*x 
 *	@param	*y 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_TOOL_GetFloorSize( u8 area_no ,s32 *fs_x ,s32 *fs_y )
{
  (*fs_x) = sc_floor_size[area_no].fs_x;
  (*fs_y) = sc_floor_size[area_no].fs_y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応したゾーンIDテーブルを取得する
 *
 *	@param	version 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const u16* INF_TOOL_GetZoneIdTable( u8 version )
{
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
 *	@brief  バージョンごとのエレベータ位置を取得
 *
 *	@param	version 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const GRID_DATA* INF_TOOL_GetElevatorPos( u8 version )
{
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
 *	@param	elevator_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const GRID_DATA* INF_TOOL_GetReceptionElevatorPos( int elevator_type )
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
const GRID_DATA* INF_TOOL_GetWarpPosTable( u8 version ,int room_type ,int warp_dir  )
{
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
const GRID_DATA* INF_TOOL_GetBmodelPos( u8 version ,int room_type ,int warp_dir  )
{
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
 *	@brief  指定エリアのレベルデータを取得する
 *
 *	@param	area_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const INF_LEVEL_TABLE_PARAM* INF_TOOL_GetLevelData( u16 area_no )
{
  return &sc_level_table[area_no];
}

//----------------------------------------------------------------------------
/**
 *	@brief  パワー乱数を取得する
 *
 *	@param	area_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetPowRnd( u16 area_no )
{
  return sc_pow_rnd_table[area_no];
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナータイプごとの配置数テーブルを取得
 *
 *	@param	area_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const u8* INF_TOOL_GetTrTypeCntTable( u16 area_no ,u8 version)
{
  if( version == VERSION_B2 )
  {
    return sc_inf_tr_setdata_b[area_no];
  }
  else
  {
    return sc_inf_tr_setdata_w[area_no];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  表示コードに対応した会話パターンを返す
 *
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetTrainerHintType( const INF_DUNGEON_DATA *p_dungeon_data,u16 obj_code ,u8 floor_no, u8 id_pos )
{
  BOOL is_gate = INF_TOOL_DUNGEON_GetIsGateTrainer( p_dungeon_data ,floor_no ,id_pos );
  if( is_gate == FALSE )
  {
    int i;
    for( i = 0; i < INFINITY_DUNGEON_TALK_PATTERN_MAX; i++ )
    {
      if( sc_talk_pattern_table[i].obj_code == obj_code )
      {
        return sc_talk_pattern_table[i].hint_type;
      }
    }
  }
  else
  {
    // ゲートトレーナー専用ヒント
    return INFTR_HINT_TYPE_BOSS_INFO;
  }
  
  GF_ASSERT_MSG(0,"ヒントタイプの登録されていない表示コードです\n");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  表示コードに対応した会話パターンを返す
 *
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INF_TOOL_GetTrainerTalkType( u16 obj_code)
{
  int i;
  for( i = 0; i < INFINITY_DUNGEON_TALK_PATTERN_MAX; i++ )
  {
    if( sc_talk_pattern_table[i].obj_code == obj_code )
    {
      return sc_talk_pattern_table[i].talk_type;
    }
  }

  GF_ASSERT_MSG(0,"会話パターンの登録されていない表示コードです\n");
  return 0;
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
void INF_TOOL_RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context )
{
  int i;
  u32 tmp;
  for( i = 0; i < size ;i++ )
  {
    u32 rnd = GFL_STD_Rand( rand_context,size);
    tmp = p_array[rnd];
    p_array[rnd] = p_array[i];
    p_array[i] = tmp;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  配列内のデータをランダムにならびかえる(u8版)
 *
 *	@param	*p_array 
 *	@param	size 
 *	@param	context 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_TOOL_RndArrayU8( u8 *p_array , int size ,GFL_STD_RandContext *rand_context )
{
  int i;
  u32 tmp;
  for( i = 0; i < size ;i++ )
  {
    u32 rnd = GFL_STD_Rand( rand_context,size);
    tmp = p_array[rnd];
    p_array[rnd] = p_array[i];
    p_array[i] = tmp;
  }
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
void INF_TOOL_SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed )
{
  int i,n;
  GFL_STD_RandContext rand_context;

  // シード生成用シード
  p_seed->master_seed = master_seed;

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, p_seed->master_seed );

  // 子シードを生成
  p_seed->tr_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
  p_seed->dungeon_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);

  // トレーナーシード
  for( i = 0; i < INFINITYDUNGEON_FLOOR_MAX ;i++ )
  {
    for( n = 0; n < INFINITY_DUNGEON_TRID_MAX ;n++ )
    {
      p_seed->tr_seed[i][n] = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フロアデータ配列を取得する
 *
 *	@param	area_no 
 *	@param	version 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const u8* INF_TOOL_GetFloorData( u16 area_no ,u16 floor_no ,u8 version )
{
  if( version == VERSION_B2 )
  {
    return sc_floor_data_b[area_no][floor_no];
  }
  else
  {
    return sc_floor_data_w[area_no][floor_no];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定トレーナーがゲートトレーナーか調べる
 *
 *	@param	INF_DUNGEON_DATA *p_wk 
 *	@param	floor_no
 *	@param	id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL INF_TOOL_DUNGEON_GetIsGateTrainer( const INF_DUNGEON_DATA *p_wk ,u8 floor_no, u8 id_pos )
{
  if( (p_wk->gate_trainer_id_pos == id_pos) &&
      (p_wk->gate_trainer_floor == floor_no) )
  {
    return TRUE;
  }
    
  return FALSE;
}

