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
#include "field/infinity_dungeon_def.h"
#include "savedata/infinity_dungeon_save.h"
#include "savedata/save_tbl.h"
#include "tr_tool/ex_trainer_def.h"

// グローバル変数
#ifdef PM_DEBUG
extern BOOL g_debug_inf_is_presentation;
#endif

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	無限ダンジョンセーブデータ構造体
//=====================================
struct _INFINITYDUNGEON_SAVE{
  u8  dungeon_status;
  u8  ex_dungeon_status;
  u8  ex_mission_count;
  u8  memento_status;       // 未使用

  u32 rand_seed;
  u32 trainer_flag[INFINITYDUNGEON_FLOOR_MAX];
  u32 recover_flag;
  u8  map_status;
  u8  floor_id;
  u8  room_id;
  u8  partner_id;
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
  p_save->partner_id    = 0;
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
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetDungeonStatus( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->dungeon_status;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンステータスを取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetExDungeonStatus( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->ex_dungeon_status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  EXダンジョン開放ミッションクリア回数を取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetExMissionCount( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->ex_mission_count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  記念品取得状態を取得
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetMementoFlag( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->memento_status;
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
  return (p_save->floor_id % INFINITYDUNGEON_FLOOR_MAX); // エリア内のフロアに変換
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
 *	@brief  現在のパートナーIDを取得する
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITYDUNGEON_SAVE_GetPartnerID( const INFINITYDUNGEON_SAVE *p_save )
{
  return p_save->partner_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン状態を設定する
 *
 *	@param	*p_save 
 *	@param	dungeon_status
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetDungeonStatus( INFINITYDUNGEON_SAVE *p_save ,u8 dungeon_status)
{
  p_save->dungeon_status = dungeon_status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  EXダンジョン状態を設定する
 *
 *	@param	*p_save 
 *	@param	ex_dungeon_status
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetExDungeonStatus( INFINITYDUNGEON_SAVE *p_save ,u8 ex_dungeon_status)
{
  p_save->ex_dungeon_status = ex_dungeon_status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  EXダンジョン用ミッションクリア回数を加算
 *
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_AddExMissionCount( INFINITYDUNGEON_SAVE *p_save )
{
  if( p_save->ex_mission_count < 255 )
  {
    p_save->ex_mission_count = p_save->ex_mission_count + 1;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  景品取得状態を設定する
 *
 *	@param	*p_save 
 *	@param	memento_status
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetMementoFlag( INFINITYDUNGEON_SAVE *p_save ,u8 memento_status)
{
  p_save->memento_status = memento_status;
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
 *	@brief  現在のパートナーIDを設定する
 *
 *	@param	*p_save 
 *	@param	partner_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITYDUNGEON_SAVE_SetPartnerID( INFINITYDUNGEON_SAVE *p_save ,u8 partner_id)
{
  p_save->partner_id = partner_id;
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
  if( p_save->floor_id == 0 )
  {
    return INFINITY_DUNGEON_AREA_ID_1;
  }
  else
  {
    return p_save->floor_id/INFINITYDUNGEON_FLOOR_MAX;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントレベル取得
 *
 *	@param	obj_id 
 *
 *	@retval ヒントレベル
 */
//-----------------------------------------------------------------------------
u8 INFINITY_DUNGEON_SAVE_GetHintLevel( const INFINITYDUNGEON_SAVE *p_save )
{
  s32 floor_id;
  s32 tr_id_pos;
  s32 win_num = 0;

  for( floor_id = 0; floor_id<INFINITYDUNGEON_FLOOR_MAX; floor_id++ )
  {
    for( tr_id_pos = 0; tr_id_pos<INFINITY_DUNGEON_TRID_MAX; tr_id_pos++ )
    {
      BOOL ret = INFINITYDUNGEON_SAVE_GetTrainerFlag( p_save ,floor_id ,tr_id_pos );
      if( ret == TRUE )
      {
        win_num++;
      }
    }
  }

#ifdef PM_DEBUG
  // プレゼン用マップ差し替え
  if( g_debug_inf_is_presentation == TRUE )
  {
    if( 4 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_4;
    }

    if( 3 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_3;
    }

    if( 2 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_2;
    }

    return INFINITY_DUNGEON_HINT_LEVEL_1;
  }
  else
  {
    if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_4 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_4;
    }

    if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_3 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_3;
    }

    if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_2 <= win_num )
    {
      return INFINITY_DUNGEON_HINT_LEVEL_2;
    }
    return INFINITY_DUNGEON_HINT_LEVEL_1;
  }
#else
  if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_4 <= win_num )
  {
    return INFINITY_DUNGEON_HINT_LEVEL_4;
  }

  if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_3 <= win_num )
  {
    return INFINITY_DUNGEON_HINT_LEVEL_3;
  }

  if( INFINITY_DUNGEON_HINT_LEVEL_WIN_NUM_2 <= win_num )
  {
    return INFINITY_DUNGEON_HINT_LEVEL_2;
  }

  return INFINITY_DUNGEON_HINT_LEVEL_1;
#endif
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

