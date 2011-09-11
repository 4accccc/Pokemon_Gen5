//===============================================================================
/*
 *  @file   fmission_gimmick_npc.c
 *  @brief  フェスミッション NPCミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.09.09
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
#include "field/fes_gimmick_npc.h"
#include "field/fes_gimmick_local.h"

static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u8 id );
static BOOL sub_CheckMMdlPos( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
static BOOL sub_CheckHitAnotherEventForGrid( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const VecFx32* ev_pos );
static BOOL sub_CheckHitAnotherEventForRail( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const RAIL_LOCATION* ev_loc );

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　MMDL初期化処理
 *
 *  フェスNPC用リソースをフィールドに常駐させる
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_MMdlSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  MMDL_BLACTCONT_AddResourceTexCode( sys_wk->mmdl_sys, FES_GIMMICK_NPC_SUB_GetObjCode( wk, sys_wk->fmwk ) );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　フィールド復帰時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_FieldRecoverSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  if( wk->point_max > 0 ){
    sub_CheckMMdlPos( sys_wk, wk );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　ミッション終了時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  int i;

  if( wk->point_max > 0 ){
    for(i = 0;i < FES_GIMMICK_MAX;i++){
      if( wk->mmdl_tbl[i] != NULL ){
        MMDL_Delete( wk->mmdl_tbl[i] );
      }
    }
  }

  //ワーククリア
  MI_CpuClear32( wk, sizeof(FES_GIMMICK_NPC_WORK) );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック ポイント話しかけ判定&イベント生成
 *
 *  @retval NULL  イベント無し
 *  @retval GMEVENT* POSを踏んだのでイベント発生
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_NPC_CheckEventForGrid( FES_GIMMICK* sys_wk, const VecFx32* cp_pos )
{
  int i;
  FES_GIMMICK_NPC_WORK* wk = (FES_GIMMICK_NPC_WORK*)sys_wk->wk_buf;

#if 0
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );

  for( i = 0;i < wk->point_max; i++){
    POS_EVENT_DATA* pd = &wk->data[ wk->idx[i] ];
    
    if( !FLDEFF_FES_KIRA_ObjIsVisible( sys_wk->eff_task, i ) ||
        (pd->pos_type != EVENTDATA_POSTYPE_GRID )){
      continue;
    }
    if( PosEventData_GPOS_IsHit( pd, cp_pos ) ){
      return sub_MakeEvent( sys_wk, wk, i );
    }
  }
#endif
  return NULL;
}
//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック ポイント当たり判定&イベント生成
 *
 *  @retval NULL  イベント無し
 *  @retval GMEVENT* POSを踏んだのでイベント発生
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_NPC_CheckEventForRail( FES_GIMMICK* sys_wk,  const RAIL_LOCATION* cp_location )
{
  int i;
  FES_GIMMICK_NPC_WORK* wk = (FES_GIMMICK_NPC_WORK*)sys_wk->wk_buf;
#if 0
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );

  for( i = 0;i < wk->point_max; i++){
    POS_EVENT_DATA* pd = &wk->data[ wk->idx[i] ];
    
    if( !FLDEFF_FES_KIRA_ObjIsVisible( sys_wk->eff_task, i ) ||
        (pd->pos_type != EVENTDATA_POSTYPE_RAIL )){
      continue;
    }
    if( PosEventData_RPOS_IsHit( pd, cp_location ) ){
      return sub_MakeEvent( sys_wk, wk, i );
    }
  }
#endif
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
BOOL FES_GIMMICK_NPC_CheckHitGridPos( FES_GIMMICK* sys_wk, u16 gx, u16 gz, fx32 height )
{
#if 0
  int i;
  VecFx32 vec;
  FES_GIMMICK_NPC_WORK* wk = (FES_GIMMICK_NPC_WORK*)sys_wk->wk_buf;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );
  FES_GIMMICK_NPC_FLAG* f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, wk->zone_id );

  for( i = 0;i < wk->point_max; i++)
  {
    POS_EVENT_DATA* pd = &wk->data[ wk->idx[i] ];

    if( f_tbl->flag[i] ){
      continue; //すでに取得済み
    }
    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetPosEventCenterPos( pd, &vec);
    }
    else{
      RAIL_LOCATION loc;
      EVENTDATA_GetPosEventCenterRailLocation( pd, &loc );
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
#endif
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック イベント生成　
 */
//----------------------------------------------------------------------
static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u8 id )
{
  GMEVENT* event = NULL;
#if 0
  SCRIPT_WORK* sc;
  FES_GIMMICK_NPC_FLAG* f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, wk->zone_id );
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
 
  /*
   * GFBTS1967関連対処
   * 8番道路の一本橋から落ちてすぐ、エフェクトエンカウントが起動した場合
   * 自機の特殊落下中ステータスがクリアされないため、明示的にクリアする
   */
  if( FIELD_PLAYER_CheckOzeFallOut( sys_wk->field_player ) ){
    FIELD_PLAYER_FinishOzeFallOut( sys_wk->field_player );
  }
#endif
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック 配置場所チェック　
 *
 *  他のイベントや動作モデルに重なっちゃう奴は非表示にする
 */
//----------------------------------------------------------------------
static BOOL sub_CheckMMdlPos( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  int i;
  VecFx32 vec;
  FLDMAP_BASESYS_TYPE ftype = FIELDMAP_GetBaseSystemType( sys_wk->fieldWork );
  FES_GIMMICK_NPC_FLAG* f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, wk->zone_id );

  for( i = 0;i < wk->point_max; i++)
  {
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];

    if( f_tbl->flag[i] || wk->mmdl_tbl[i] == NULL ){
      continue; //すでに取得済みなのでチェックしなくていい
    }
    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetBGTalkCenterPos( pd, &vec);
      if( sub_CheckHitAnotherEventForGrid( sys_wk, wk, i, &vec ) ){
        //他のイベントと重なったので隠す&ゾーンチェンジするまで再チェックしないようにフラグ立てちゃう
        f_tbl->flag[i] = TRUE;
        MMDL_Delete( wk->mmdl_tbl[i] );
      }
    }else{
      RAIL_LOCATION loc;
      EVENTDATA_GetBGTalkCenterRailLocation( pd, &loc );
      if( sub_CheckHitAnotherEventForRail( sys_wk, wk, i, &loc ) ){
        //他のイベントと重なったので隠す&ゾーンチェンジするまで再チェックしないようにフラグ立てちゃう
        f_tbl->flag[i] = TRUE;
        MMDL_Delete( wk->mmdl_tbl[i] );
      }
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　生成時の当たり判定(グリッド)
 */
//----------------------------------------------------------------------
static BOOL sub_CheckHitAnotherEventForGrid( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const VecFx32* ev_pos )
{
  MMDL* mmdl;
  //動作モデルとの判定を取る
  mmdl = MMDLSYS_SearchGridPosEx( sys_wk->mmdl_sys,
            SIZE_GRID_FX32(ev_pos->x), SIZE_GRID_FX32(ev_pos->z), ev_pos->y, H_DIFF_COLLISION_FOR_GROUND, FALSE );
  if( mmdl != NULL && mmdl != wk->mmdl_tbl[data_idx] ){ //フェスNPC自身は無視
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
 *  @brief  NPCミッションギミック　生成時の当たり判定(レール)
 */
//----------------------------------------------------------------------
static BOOL sub_CheckHitAnotherEventForRail( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const RAIL_LOCATION* ev_loc )
{
  MMDL* mmdl;
  
  //動作モデルとの判定を取る
  mmdl = MMDLSYS_SearchRailLocation( sys_wk->mmdl_sys, ev_loc, FALSE );
//  if( mmdl != NULL && mmdl != sys_wk->player_mmdl ){  //自機は除外する
  if( mmdl != NULL && mmdl != wk->mmdl_tbl[data_idx] ){ //フェスNPC自身は無視
    return TRUE;
  }

  //他のPOSイベントとの当たり判定を取る
  if( EVENTDATA_CheckPosEventRailLocation( sys_wk->evdata, sys_wk->evwork, ev_loc ) != EVENTDATA_ID_NONE ){
    return TRUE;
  }
  return FALSE;
}



