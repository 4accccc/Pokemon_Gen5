//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		\home\swan\prog\src\savedata\infinity_dungeon_save.c
 *	@brief  無限ダンジョンセーブデータ
 *	@author	Masayuki Onoue
 *	@date		2011.02.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "savedata/save_control.h"	        //SAVE_CONTROL_WORK参照のため
#include "../../../resource/infinity_dungeon/data/inf_floor_num.h"
#include "field/infinity_dungeon_def.h"
#include "field/field_wfbc_data.h" 
#include "savedata/infinity_dungeon_save.h"
#include "savedata/save_tbl.h"
#include "tr_tool/ex_trainer_def.h"

#define TRAINER_FLAG_MAX (32) // 1階に存在するトレーナーの最大数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	無限ダンジョンセーブデータ構造体
//=====================================
struct _INFINITYDUNGEON_SAVE{

  // ダンジョン用データ（白黒共通）
  u32 rand_seed;
  u32 trainer_flag[INFINITYDUNGEON_FLOOR_MAX];
  u32 recover_flag;
  u8  map_status;
  u8  floor_id;
  u8  room_id;

  // ダンジョンの状態
  u8  dungeon_status_w;   // 白バージョン用セーブデータ
  u8  dungeon_status_b;   // 黒バージョン用セーブデータ

  u32 dungeon_start_gold; // ダンジョン開始時の所持金

  u8  key_system_area[KEY_SYSTEM_SAVE_WORK_SIZE];
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  構造体のサイズ取得
 *
 *	@param	none
 *
 *	@retval 無限ダンジョン用セーブデータサイズ
 */
//-----------------------------------------------------------------------------
int INFINITYDUNGEON_SAVE_GetWorkSize(void)
{
  return sizeof( INFINITYDUNGEON_SAVE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンセーブ初期化
 *
 *	@param	*p_save セーブデータポインタ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_Init(INFINITYDUNGEON_SAVE *p_save)
{
  GFL_STD_MemFill(p_save, 0, sizeof(INFINITYDUNGEON_SAVE));

  {
    KEY_SYSTEM_SAVE *p_key = INFINITYDUNGEON_SAVE_GetKeySystem( p_save );
    KEY_SYSTEM_SAVE_Init( p_key );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンデータの初期化
 *
 *	@param	*p_save セーブデータポインタ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_InitDungeonData(INFINITYDUNGEON_SAVE *p_save)
{
  // トレーナー戦闘フラグ
  GFL_STD_MemFill(p_save->trainer_flag, 0, sizeof(u32)*INFINITYDUNGEON_FLOOR_MAX);
  
  // ダンジョン内データ
  p_save->recover_flag  = 0;
  p_save->map_status    = 0;
  p_save->floor_id      = 0;
  p_save->room_id       = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンセーブへポインタを取得
 *
 *	@param	* p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INFINITYDUNGEON_SAVE* SaveData_GetINFINITYDUNGEON( SAVE_CONTROL_WORK *p_save )
{
	return SaveControl_DataPtrGet(p_save, GMDATA_ID_INFINITYDUNGEON);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンステータスを取得
 *
 *	@param	*p_save 
 *	@param	wfbc_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetDungeonStatus( const INFINITYDUNGEON_SAVE *p_save ,FIELD_WFBC_CORE_TYPE wfbc_type )
{
  if( wfbc_type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    return p_save->dungeon_status_b;
  }
  return p_save->dungeon_status_w;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン生成用乱数の種を取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITYDUNGEON_SAVE_GetRandomSeed( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->rand_seed;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー戦闘状態取得
 *
 *	@param	*p_save 
 *	@param	floor_id 
 *	@param	trainer_id_pos 
 *
 *	@retval TRUE = 戦闘済み
 */
//-----------------------------------------------------------------------------
BOOL INFINITYDUNGEON_SAVE_GetTrainerFlag( const INFINITYDUNGEON_SAVE *p_save ,int floor_id ,int trainer_id_pos )
{
  u32 bit_mask = 1;
  bit_mask = bit_mask << trainer_id_pos;
  if( p_save->trainer_flag[floor_id] & bit_mask )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  戦闘勝利済みトレーナーの数を取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 INFINITYDUNGEON_SAVE_GetWinTrainerCount( const INFINITYDUNGEON_SAVE *p_save )
{
  u32 i,n;
  u32 cnt = 0;
  for( i = 0; i < INFINITYDUNGEON_FLOOR_MAX; i++ )
  {
    for( n = 0; n < TRAINER_FLAG_MAX; n++ )
    {
      if( INFINITYDUNGEON_SAVE_GetTrainerFlag( p_save ,i ,n ) == TRUE )
      {
        cnt++;
      }
    }
  }

  ONOUE_Printf("無限ダンジョン内勝利済みトレーナーの数を取得：%u\n",cnt);
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ状態を取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetMapStatus( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->map_status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のフロアIDを取得する
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetFloorID( const INFINITYDUNGEON_SAVE *p_save )
{
  u8 area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
  s32 start_floor = INFINITYDUNGEON_SAVE_GetAreaStartFloor( area_id );
  return (p_save->floor_id - start_floor); // エリア内のフロアに変換
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の部屋IDを取得する
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetRoomID( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->room_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン状態を設定する
 *
 *	@param	*p_save 
 *	@param	dungeon_status 
 *	@param	wfbc_type
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetDungeonStatus( INFINITYDUNGEON_SAVE *p_save ,u8 dungeon_status ,FIELD_WFBC_CORE_TYPE wfbc_type)
{
  if( wfbc_type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    p_save->dungeon_status_b = dungeon_status;
  }
  else
  {
    p_save->dungeon_status_w = dungeon_status;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン生成用乱数シードを設定
 *
 *	@param	*p_save 
 *	@param	rand_seed
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetRandomSeed( INFINITYDUNGEON_SAVE *p_save ,u32 rand_seed)
{
  p_save->rand_seed = rand_seed;
}
//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー戦闘状況を設定する
 *
 *	@param	*p_save 
 *	@param	floor_id 
 *	@param	trainer_id 
 *	@param	status      FALSE = 未戦闘 , TRUE = 戦闘済み
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetTrainerFlag( INFINITYDUNGEON_SAVE *p_save ,int floor_id ,int trainer_id_pos , BOOL status)
{
  if( status == FALSE )
  {
    // ビットを下げる
    u32 bit_mask = 0xffffffff;
    u32 del_bit  = 1;

    del_bit   = del_bit << trainer_id_pos;
    bit_mask  = bit_mask ^ del_bit;
    p_save->trainer_flag[floor_id]  = p_save->trainer_flag[floor_id] & bit_mask;
  }
  else
  {
    // ビットを上げる
    s32 bit_mask = 1;
    bit_mask = bit_mask << trainer_id_pos;
    p_save->trainer_flag[floor_id]  = p_save->trainer_flag[floor_id] | bit_mask;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ状態を設定する
 *
 *	@param	*p_save 
 *	@param	map_status
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetMapStatus( INFINITYDUNGEON_SAVE *p_save ,u8 map_status)
{
  p_save->map_status = map_status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のフロア番号を設定する
 *
 *	@param	*p_save 
 *	@param	floor_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetFloorID( INFINITYDUNGEON_SAVE *p_save ,u8 floor_id)
{
  p_save->floor_id = floor_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の部屋番号を設定する
 *
 *	@param	*p_save 
 *	@param	room_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetRoomID( INFINITYDUNGEON_SAVE *p_save ,u8 room_id)
{
  p_save->room_id = room_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナー状態取得
 *
 *	@param	*p_save 
 *	@param	recover_flg_pos ビット位置
 *
 *	@retval TRUE = 回復済み
 */
//-----------------------------------------------------------------------------
BOOL INFINITYDUNGEON_SAVE_GetRecoverFlag( const INFINITYDUNGEON_SAVE *p_save ,int recover_flg_pos )
{
  u32 bit_mask = 1;
  bit_mask = bit_mask << recover_flg_pos;
  if( p_save->recover_flag & bit_mask )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナー状態設定
 *
 *	@param	*p_save 
 *	@param	recover_flg_pos ビット位置
 *	@param	status          TRUE = 回復済み
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetRecoverFlag( INFINITYDUNGEON_SAVE *p_save ,int recover_flg_pos ,BOOL status)
{
  if( status == FALSE )
  {
    // ビットを下げる
    u32 bit_mask = 0xffffffff;
    u32 del_bit  = 1;

    del_bit   = del_bit << recover_flg_pos;
    bit_mask  = bit_mask ^ del_bit;
    p_save->recover_flag = p_save->recover_flag & bit_mask;
  }
  else
  {
    // ビットを上げる
    s32 bit_mask = 1;
    bit_mask = bit_mask << recover_flg_pos;
    p_save->recover_flag = p_save->recover_flag | bit_mask;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のエリアIDを取得する
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetDungeonAreaID( const INFINITYDUNGEON_SAVE *p_save )
{
  u8 area_id;

  // エリア2のスタートエリアから、それより下の階にあるかを調べていく
  for( area_id  = INFINITY_DUNGEON_AREA_ID_1 + 1; area_id < INFINITY_DUNGEON_AREA_ID_MAX; area_id++ )
  {
    s32 start_floor = INFINITYDUNGEON_SAVE_GetAreaStartFloor( area_id );
    if( start_floor > p_save->floor_id )
    {
      // Ex:エリア2以下の場合は、エリア１
      return area_id - 1;
    }
  }

  // エリア9より上の階の場合は、エリア10
  return INFINITY_DUNGEON_AREA_ID_10;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアごとのスタートフロアを取得
 *
 *	@param	area_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetAreaStartFloor( u8 area_id )
{
  s32 i;
  s32 start_floor = 0;

  // 指定の階数まで加算
  for( i = 0; i < area_id; i++ )
  {
    start_floor += INFINITYDUNGEON_SAVE_GetFloorNum(i);
  }
  return start_floor;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フロアが何回あるかを返す
 *
 *	@param	now_area 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetFloorNum( u8 area_id )
{
  // フロアごとのスタート回数
  const u8 floor_num_table[ INFINITY_DUNGEON_AREA_ID_MAX ] =
  {
    INFINITYDUNGEON_FLOOR_AREA1_MAX,
    INFINITYDUNGEON_FLOOR_AREA2_MAX,
    INFINITYDUNGEON_FLOOR_AREA3_MAX,
    INFINITYDUNGEON_FLOOR_AREA4_MAX,
    INFINITYDUNGEON_FLOOR_AREA5_MAX,
    INFINITYDUNGEON_FLOOR_AREA6_MAX,
    INFINITYDUNGEON_FLOOR_AREA7_MAX,
    INFINITYDUNGEON_FLOOR_AREA8_MAX,
    INFINITYDUNGEON_FLOOR_AREA9_MAX,
    INFINITYDUNGEON_FLOOR_AREA10_MAX,
  };

  return floor_num_table[ area_id ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン開始時の所持金を取得する
 *
 *	@param	INFINITYDUNGEON_SAVE *p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITYDUNGEON_SAVE_GetDungeonStartGold( const INFINITYDUNGEON_SAVE *p_save )
{
  // ゲーム開始時の所持金
  return p_save->dungeon_start_gold;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン開始時の所持金を設定する
 *
 *	@param	*p_save 
 *	@param	gold 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetDungeonStartGold( INFINITYDUNGEON_SAVE *p_save ,u32 gold )
{
  p_save->dungeon_start_gold = gold;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブを取得
 *
 *	@param	INFINITYDUNGEON_SAVE *p_save  セーブ
 *
 *	@return キーシステムセーブ
 */
//-----------------------------------------------------------------------------
KEY_SYSTEM_SAVE * INFINITYDUNGEON_SAVE_GetKeySystem( INFINITYDUNGEON_SAVE *p_save )
{
  return (KEY_SYSTEM_SAVE*)p_save->key_system_area;
}
