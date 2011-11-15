//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fes_gimmick_event_nw.c
 *  @brief  フェスミッション EVENTミッションギミック(NOTWIFI)
 *	@author	Masayuki Onoue
 *	@date		2011.11.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "field/fieldmap.h"


#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"
#include "field/fes_gimmick_local.h"

#include "field/fes_gimmick_event.h"

#include "fes_gimmick_trail.cdat"
#include "animal_trail_def.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static u16 ExAnimalTrailMonsNoGet( const FES_GIMMICK* sys_wk );
static u8 ExAnimalTrailFormNoGet( const FES_GIMMICK* sys_wk );
static void RndArray( u8 *p_array , int size ,GFL_STD_RandContext *rand_context );

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴調査（特殊ポケモン配置版）用ワークを初期化する
 *
 *	@param	sys_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FES_GIMMICK_EVENT_ExAnimalTrailMissionWkCreate( FES_GIMMICK* sys_wk )
{
  int i;
  const FEST_MISSION *p_feswk = LINK_FESTIVAL_GetMissionWork( sys_wk->fest_sys );
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( sys_wk->fest_sys );
  GFL_STD_RandContext rnd_c;
  u8 id_array[ANIMAL_TRAIL_NUM];
  
  
  // ワーククリア
  MI_CpuClear8( p_twk, sizeof(FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK) );

  //乱数コンテキスト初期化
  FES_GIMMICK_SUB_RndInit( p_feswk->key, 0, &rnd_c );
    
  // ========================================
  // 配置場所抽選
  // IDの初期化
  for( i = 0; i < ANIMAL_TRAIL_NUM; i++ )
  {
    id_array[i] = i;
  }
  // 配列内をランダムに並び替え
  RndArray( id_array ,ANIMAL_TRAIL_NUM ,&rnd_c );
  
  
  // 先頭から指定数再配置リストに入れる
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    p_twk->replace_data[i].trail_id       = id_array[i];
    p_twk->replace_data[i].is_female      = FALSE;
    p_twk->replace_data[i].is_enable      = TRUE;
    p_twk->replace_data[i].is_through_door= FALSE;
    
    // 同一ミッションの場合はモンスター固定だが、変更時にはここを変更
    p_twk->replace_data[i].monsno  = ExAnimalTrailMonsNoGet( sys_wk );
    p_twk->replace_data[i].formno  = ExAnimalTrailFormNoGet( sys_wk );
  }
  
  // 再度配列内をランダムに並び替え
  RndArray( id_array ,ANIMAL_TRAIL_NUM ,&rnd_c );
 
  // ========================================
  // メス抽選
  // インデックスで初期化
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    id_array[i] = i;
  }
  // 配列内をランダムに並び替え
  RndArray( id_array ,FES_ATRAIL_REPLACE_NUM ,&rnd_c );
   
  // 指定数メス配置に置き換える
  for( i = 0; i < FES_ATRAIL_FEMALE_SET_NUM; i++ )
  {
    p_twk->replace_data[id_array[i]].is_female = TRUE;
  }

#ifdef PM_DEBUG 
  OS_Printf("============================================\n");
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    FES_ANIMAL_TRAIL_DATA *p_rp = &p_twk->replace_data[i];
    OS_Printf( "獣道ID：%u\n",p_rp->trail_id);
    OS_Printf( "メスフラグ：%u\n",p_rp->is_female);
    OS_Printf( "有効フラグ：%u\n",p_rp->is_enable);
    OS_Printf("--------------------------------------------\n");
  }
  OS_Printf("============================================\n");
  ONOUE_Printf("隠し穴調査（特殊ポケモン配置版）用ワーク初期化\n");
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴配置データからモンスター番号を取得する（進行中ミッション）
 *
 *	@param	sys_wk 
 *
 *	@retval モンスター番号
 */
//-----------------------------------------------------------------------------
static u16 ExAnimalTrailMonsNoGet( const FES_GIMMICK* sys_wk )
{
  const FEST_MISSION *p_feswk = LINK_FESTIVAL_GetMissionWork( sys_wk->fest_sys );
  GF_ASSERT( p_feswk->beacon.ex_param < FMISSION_EXP_TRAIL_MONS_MAX );
  return DATA_TrailMonsTbl[ p_feswk->beacon.ex_param ].monsno;
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴配置データからフォルム番号を取得する（進行中ミッション）
 *
 *	@param	sys_wk 
 *
 *	@retval フォルム番号
 */
//-----------------------------------------------------------------------------
static u8 ExAnimalTrailFormNoGet( const FES_GIMMICK* sys_wk )
{
  const FEST_MISSION *p_feswk = LINK_FESTIVAL_GetMissionWork( sys_wk->fest_sys );
  GF_ASSERT( p_feswk->beacon.ex_param < FMISSION_EXP_TRAIL_MONS_MAX );
  return DATA_TrailMonsTbl[ p_feswk->beacon.ex_param ].formno;
}

//----------------------------------------------------------------------------
/**
 *	@brief  配列内のデータをランダムにならびかえる
 *
 *	@param	*p_array 
 *	@param	size 
 *	@param	*rand_context 
 *
 *	@retval
 *	@note INF_TOOL_RndArray関数を参考に製作しました
 */
//-----------------------------------------------------------------------------
static void RndArray( u8 *p_array , int size ,GFL_STD_RandContext *rand_context )
{
  int i;
  u32 tmp;
  for( i = 0; i < size ;i++ )
  {
    u32 rnd = GFL_STD_Rand( rand_context, size );
    tmp = p_array[rnd];
    p_array[rnd] = p_array[i];
    p_array[i] = tmp;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴調査（特殊ポケモン配置版）ミッションか調べる
 *
 *	@param	sys_wk 
 *
 *	@retval TRUEのとき隠し穴（特殊ポケモン配置版）
 */
//-----------------------------------------------------------------------------
BOOL FES_GIMMICK_EVENT_IsExAnimalTrailMission( FES_GIMMICK* sys_wk )
{
  const FEST_MISSION *p_feswk = LINK_FESTIVAL_GetMissionWork( sys_wk->fest_sys );
  
  // 獣道調査かつExパラメータが存在するときのみ
  if( (p_feswk->beacon.base_param == FMISSION_EVID_ANIMAL_TRAIL) &&
      (p_feswk->beacon.ex_param != FMISSION_EXP_TRAIL_DUMMY ) )
  {
    return TRUE;
  }

  return FALSE;
}

