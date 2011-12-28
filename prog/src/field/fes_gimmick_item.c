//===============================================================================
/*
 *  @file   fmission_gimmick_item.c
 *  @brief  フェスミッション アイテムミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================

#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "system/time_icon.h"
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_nw.h"
#include "gamesystem/fest_mission_local.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "field/fldeff_fes_kira.h"
#include "field/eventdata_sxy.h"
#include "field/field_rail.h"
#include "field/zonedata.h"
#include "./event_fieldmap_control.h"
#include "./event_fmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_fmission_event.h"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"
#include "../../../resource/fldmapdata/script/fmission_scr_def.h" //SCRID_～

#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"
#include "field/fes_gimmick_local.h"

/*
#define FES_ITEM_PARAM_NUM (4) //1セット4アイテム
#define LOT_ITEM_PROB_MAG (1000)  ///<乱数精度を高めるため、100(%)に倍率をかけた値で抽選する->1%=1000
*/
static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, u8 id );
static BOOL sub_SetEffect( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk );
static BOOL sub_CheckHitAnotherEventForGrid( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, const VecFx32* ev_pos );
static BOOL sub_CheckHitAnotherEventForRail( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, const RAIL_LOCATION* ev_loc );

static FES_GIMMICK_ITEM_FLAG* sub_FlagTableGet( FES_GIMMICK_ITEM_WORK* wk, u16 zone_id );

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　フィールド復帰時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ITEM_FieldRecoverSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk )
{
  //キラオブジェクト表示をリセット
  FLDEFF_FES_KIRA_ObjDelAll( sys_wk->eff_task );

  if( wk->point_max > 0 ){
    sub_SetEffect( sys_wk, wk );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　ミッション終了時処理
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_ITEM_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk )
{
  int ret = ( wk->data_max > 0 );

  //キラオブジェクト表示をリセット
  FLDEFF_FES_KIRA_ObjDelAll( sys_wk->eff_task );

  //ワーククリア
  MI_CpuClear32( wk, sizeof(FES_GIMMICK_ITEM_WORK) );

  return ret;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック ポイント当たり判定&イベント生成
 *
 *  @retval NULL  イベント無し
 *  @retval GMEVENT* POSを踏んだのでイベント発生
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_ITEM_CheckEventForGrid( FES_GIMMICK* sys_wk, const VecFx32* cp_pos )
{
  int i;
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)sys_wk->wk_buf;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );

  for( i = 0;i < wk->point_max; i++){
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];
    
    if( !FLDEFF_FES_KIRA_ObjIsVisible( sys_wk->eff_task, i ) ||
        (pd->pos_type != EVENTDATA_POSTYPE_GRID )){
      continue;
    }
    if( BGTalkData_GPOS_IsHit( pd, cp_pos ) ){
      return sub_MakeEvent( sys_wk, wk, i );
    }
  }
  return NULL;
}
//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック ポイント当たり判定&イベント生成
 *
 *  @retval NULL  イベント無し
 *  @retval GMEVENT* POSを踏んだのでイベント発生
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_ITEM_CheckEventForRail( FES_GIMMICK* sys_wk,  const RAIL_LOCATION* cp_location )
{
  int i;
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)sys_wk->wk_buf;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );

  for( i = 0;i < wk->point_max; i++){
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];
    
    if( !FLDEFF_FES_KIRA_ObjIsVisible( sys_wk->eff_task, i ) ||
        (pd->pos_type != EVENTDATA_POSTYPE_RAIL )){
      continue;
    }
    if( BGTalkData_RPOS_IsHit( pd, cp_location ) ){
      return sub_MakeEvent( sys_wk, wk, i );
    }
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　キラキラエフェクトとの座標当たり判定
 *
 *  @retval NULL  イベント無し
 *  @retval GMEVENT* POSを踏んだのでイベント発生
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_ITEM_CheckHitGridPos( FES_GIMMICK* sys_wk, u16 gx, u16 gz, fx32 height )
{
  int i;
  VecFx32 vec;
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)sys_wk->wk_buf;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );
  FES_GIMMICK_ITEM_FLAG* f_tbl;
  
  if( wk->point_max == 0 ){
    return FALSE;
  }
  f_tbl = sub_FlagTableGet( wk, wk->zone_id );

  for( i = 0;i < wk->point_max; i++)
  {
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];

    if( f_tbl->flag[i] ){
      continue; //すでに取得済み
    }
    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetBGTalkCenterPos( pd, &vec);
    }
    else{
      RAIL_LOCATION loc;
      EVENTDATA_GetBGTalkCenterRailLocation( pd, &loc );
      FIELD_RAIL_MAN_GetLocationPosition( sys_wk->railman, &loc, &vec );
    }

    if( SIZE_GRID_FX32(vec.x) != gx ||
        SIZE_GRID_FX32(vec.z) != gz ||
        (vec.y-H_DIFF_COLLISION_FOR_GROUND) > height ||
        (vec.y+H_DIFF_COLLISION_FOR_GROUND) < height ){
      continue;
    }
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック イベント生成　
 */
//----------------------------------------------------------------------
static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, u8 id )
{
  GMEVENT* event;
  SCRIPT_WORK* sc;
  FES_GIMMICK_ITEM_FLAG* f_tbl = sub_FlagTableGet( wk, wk->zone_id );
  u16 item = FLDEFF_FES_KIRA_ObjItemGet( sys_wk->eff_task, id );

  FLDEFF_FES_KIRA_ObjDel( sys_wk->eff_task, id );
  f_tbl->flag[id] = TRUE; //取得済みフラグ立てる

  //イベント生成
  if( item == 0 ){
    //万一アイテム未設定なら、エフェクトだけ消してイベント起こさず帰る
    GF_ASSERT(0);
    return NULL;
  } 
  event = SCRIPT_SetEventScript( sys_wk->gsys, SCRID_FMISSION_ITEM_GET, NULL, HEAPID_FIELDMAP );
  sc = SCRIPT_GetScriptWorkFromEvent( event );
  SCRIPT_SetScriptWorkParam( sc, item, 0, 0, 0 );
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　エフェクト生成
 */
//----------------------------------------------------------------------
static BOOL sub_SetEffect( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk )
{
  int i;
  VecFx32 vec;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );
  FES_GIMMICK_ITEM_FLAG* f_tbl = sub_FlagTableGet( wk, wk->zone_id );

  for( i = 0;i < wk->point_max; i++)
  {
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];

    if( f_tbl->flag[i] ){
      continue; //すでに取得済み
    }
    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetBGTalkCenterPos( pd, &vec);
      if( sub_CheckHitAnotherEventForGrid( sys_wk, wk, &vec ) ){
        //他のイベントと重なったのでエフェクト出さない&ゾーンチェンジするまで再チェックしないようにフラグ立てちゃう
        f_tbl->flag[i] = TRUE;
        continue;
      }
    }else{
      RAIL_LOCATION loc;
      EVENTDATA_GetBGTalkCenterRailLocation( pd, &loc );
      if( sub_CheckHitAnotherEventForRail( sys_wk, wk, &loc ) ){
        //他のイベントと重なったのでエフェクト出さない&ゾーンチェンジするまで再チェックしないようにフラグ立てちゃう
        f_tbl->flag[i] = TRUE;
        continue;
      }
      FIELD_RAIL_MAN_GetLocationPosition( sys_wk->railman, &loc, &vec );
    }

    FLDEFF_FES_KIRA_ObjAdd( sys_wk->eff_task, i, wk->item[i], &vec );
  }
  return TRUE;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　エフェクト生成時の当たり判定(グリッド)
 */
//----------------------------------------------------------------------
static BOOL sub_CheckHitAnotherEventForGrid( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, const VecFx32* ev_pos )
{
  MMDL* mmdl;
  //動作モデルとの判定を取る
  mmdl = MMDLSYS_SearchGridPosEx( sys_wk->mmdl_sys,
            SIZE_GRID_FX32(ev_pos->x), SIZE_GRID_FX32(ev_pos->z), ev_pos->y, H_DIFF_COLLISION_FOR_GROUND, FALSE );
  if( mmdl != NULL && mmdl != sys_wk->player_mmdl ){  //自機は除外する
    return TRUE;
  }

  //他のPOSイベントとの当たり判定を取る
  if( EVENTDATA_CheckPosEvent( sys_wk->evdata, sys_wk->evwork, ev_pos, DIR_MAX ) != EVENTDATA_ID_NONE ){
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　エフェクト生成時の当たり判定(レール)
 */
//----------------------------------------------------------------------
static BOOL sub_CheckHitAnotherEventForRail( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, const RAIL_LOCATION* ev_loc )
{
  MMDL* mmdl;
#if 0
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );

  //自分の初期位置との当たり判定を取る
  if( MMDL_GetRailLocation( sys_wk->player_mmdl, &location ) )
  {
    if( PosEventData_RPOS_IsHit( pd, &location)){
      return TRUE;
    }
  }
#endif
  //動作モデルとの判定を取る
  mmdl = MMDLSYS_SearchRailLocation( sys_wk->mmdl_sys, ev_loc, FALSE );
  if( mmdl != NULL && mmdl != sys_wk->player_mmdl ){  //自機は除外する
    return TRUE;
  }

  //他のPOSイベントとの当たり判定を取る
  if( EVENTDATA_CheckPosEventRailLocation( sys_wk->evdata, sys_wk->evwork, ev_loc ) != EVENTDATA_ID_NONE ){
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック 指定ゾーンのフラグテーブル取得
 */
//----------------------------------------------------------------------
static FES_GIMMICK_ITEM_FLAG* sub_FlagTableGet( FES_GIMMICK_ITEM_WORK* wk, u16 zone_id )
{
  int i,idx = 0;

  for( i = 0;i < wk->flag_wk.tbl_num;i++){
   if( wk->flag_wk.tbl[i].zone_id == zone_id ){
     return &wk->flag_wk.tbl[i];
   }
  }
  GF_ASSERT(0); //見つからなかった
  return &wk->flag_wk.tbl[0];
}

