//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fes_gimmick_event.c
 *  @brief  フェスミッション イベントミッションギミック
 *	@author	Masayuki Onoue
 *	@date		2011.11.10
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
#include "animal_trail_def.h"

#include "savedata/etc_save.h"
#include "field/zonedata.h"

// フェスミッションワークを超えていないかチェック
// 獣道の最大数分確保できるかをチェック
SDK_COMPILER_ASSERT((sizeof(FES_ANIMAL_TRAIL_DATA)*ANIMAL_TRAIL_NUM) <= FMISSION_WORK_AREA_LEN );
// ワークチェック
SDK_COMPILER_ASSERT(sizeof(FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK) <= FMISSION_WORK_AREA_LEN );

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
static BOOL ExAnimalTrailObjDelete( FES_GIMMICK* sys_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  フェス用隠し穴データを取得
 *
 *	@param	sys_wk 
 *	@param	trail_id  
 *
 *	@retval フェス専用隠し穴データ
 */
//-----------------------------------------------------------------------------
const FES_ANIMAL_TRAIL_DATA* FES_GIMMICK_EVENT_GetExAnimalTrailData( FES_GIMMICK* sys_wk ,u8 trail_id )
{
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( sys_wk->fest_sys );
  u8 index = FES_GIMMICK_EVENT_GetExAnimalTrailDataID( sys_wk ,trail_id );
  return &p_twk->replace_data[index];
}


//----------------------------------------------------------------------------
/**
 *	@brief  指定隠し穴のモンスターの有効フラグを落として、次の場所を抽選する
 *
 *	@param	wk 
 *	@param	end_trail_id  無効にする隠し穴の場所
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FES_GIMMICK_EVENT_RefreshExtraAnimalTrailData( FES_GIMMICK* wk ,u8 end_trail_id )
{ 
  int i;
  u8 new_index = 0;
  u8 index = FES_GIMMICK_EVENT_GetExAnimalTrailDataID( wk ,end_trail_id );
  u8 index_table[FES_ATRAIL_REPLACE_NUM-1]; // 配置数から自分の場所を除いた数分
  u8 data_pos = 0;
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );

  // 置き置き換え対象かチェック
  if( index != FES_ATRAIL_REPLACE_NUM )
  {
    // 自分のテーブルインデックス以外を取得
    for( i = 0; i < FES_ATRAIL_REPLACE_NUM ; i++ )
    {
      if( index != i )
      {
        index_table[data_pos] = i;
        data_pos++;
      }
    }

    // 個数チェック
    GF_ASSERT( data_pos == (FES_ATRAIL_REPLACE_NUM-1) );

    // 自分の場ををふさぐ
    p_twk->replace_data[index].is_enable = FALSE;

    // インデックステーブルのどれかを選択
    new_index = index_table[GFUser_GetPublicRand( FES_ATRAIL_REPLACE_NUM-1 )];
    
    // 指定場所を有効に
    p_twk->replace_data[new_index].is_enable = TRUE;

    // 自分の場所は塗りつぶさない
    GF_ASSERT( new_index != index );

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
  ONOUE_Printf("隠し穴調査再配置\n");
#endif

  }
  else
  {
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  フェス　EVENTミッションギミック　ミッション終了時処理
 *
 *	@param	sys_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL FES_GIMMICK_EVENT_Delete( FES_GIMMICK* sys_wk )
{
  // 隠し穴大捜査EXPARAM版終了処理
  if( FES_GIMMICK_EVENT_IsExAnimalTrailMission( sys_wk ) )
  {
     ExAnimalTrailObjDelete( sys_wk );
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴OBJが存在する場合は、OBJを削除する
 *
 *	@param	sys_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ExAnimalTrailObjDelete( FES_GIMMICK* sys_wk )
{
  u16 area_id = ZONEDATA_GetAreaID( FIELDMAP_GetZoneID(sys_wk->fieldWork) );

  // フェス終了時獣道にいるときのみOBJを削除する
  if ( AREADATA_IsAnimalTrailArea(area_id) == TRUE )
  {
    u8 trail_id;
    ETC_SAVE_WORK *p_save;
    SAVE_CONTROL_WORK*      p_save_control;
    
    // 現在の獣道IDを取得
    p_save_control  = GAMEDATA_GetSaveControlWork( sys_wk->gdata );
    p_save          = SaveData_GetEtc( p_save_control );
    trail_id = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( p_save );

    // フェスデータが配置されているか調べる
    if( FES_GIMMICK_EVENT_IsSetExAnimalTrailData( sys_wk ,trail_id ) == TRUE)
    {
      // モデル削除
      MMDL *p_mmdl = MMDLSYS_SearchOBJID( sys_wk->mmdl_sys ,
                                                    MMDL_ID_ANIMAL_TRAIL_OBJ );
      if( p_mmdl != NULL )
      {
        MMDL_Delete( p_mmdl );
        return TRUE;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴大調査ミッション用データを配置するか調べる
 *
 *	@param	sys_wk 
 *	@param	trail_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL FES_GIMMICK_EVENT_IsSetExAnimalTrailData( FES_GIMMICK* sys_wk ,u8 trail_id )
{
  int i;
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( sys_wk->fest_sys );
  
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    // 有効でかつ、入り口を通っているもののみ置き換える
    if( ( p_twk->replace_data[i].trail_id == trail_id ) &&
        ( p_twk->replace_data[i].is_enable == TRUE ) &&
        ( p_twk->replace_data[i].is_through_door ) )
    {
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴大調査ドア通過フラグをセットする
 *
 *	@param	sys_wk 
 *	@param	trail_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FES_GIMMICK_EVENT_SetExAnimalTrailDoorFlag( FES_GIMMICK* sys_wk ,u8 trail_id )
{
  int i;
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( sys_wk->fest_sys );

  // フラグをセットするデータを検索
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    if( p_twk->replace_data[i].trail_id == trail_id )
    {
      // ドア通過フラグをセット
      p_twk->replace_data[i].is_through_door = TRUE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  置換え対象獣道データインデックスを取得する
 *
 *	@param	sys_wk  
 *	@param	trail_id 獣道ID
 *
 *	@retval 置き換え対象でない場合はFES_ATRAIL_REPLACE_NUMを返す
 */
//-----------------------------------------------------------------------------
u8 FES_GIMMICK_EVENT_GetExAnimalTrailDataID( FES_GIMMICK* sys_wk ,u8 trail_id )
{
  int i;
  FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK *p_twk = LINK_FESTIVAL_GetMissionGimmickWork( sys_wk->fest_sys );
  
  for( i = 0; i < FES_ATRAIL_REPLACE_NUM; i++ )
  {
    // 置換え対象獣道を探索
    if( p_twk->replace_data[i].trail_id == trail_id )
    {
      return i;
    }
  }

  return FES_ATRAIL_REPLACE_NUM;
}

