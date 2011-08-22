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

#define FES_ITEM_PARAM_NUM (4) //1セット4アイテム
#define LOT_ITEM_PROB_MAG (1000)  ///<乱数精度を高めるため、100(%)に倍率をかけた値で抽選する->1%=1000

typedef struct _FES_ITEM_PARAM{
  u16 item;
  u16 prob;      ///<抽選確率
}FES_ITEM_PARAM;

typedef struct _FES_ITEM_SET{
  FES_ITEM_PARAM prm[FES_ITEM_PARAM_NUM];
}FES_ITEM_SET;

//抽選数の下限と上限
static const u8 DATA_LotTargetNum[][2] = {
  { FES_GIMMICK_ITEM_POINT_TOWN_MIN, FES_GIMMICK_ITEM_POINT_TOWN_MAX }, 
  { FES_GIMMICK_ITEM_POINT_FIELD_MIN, FES_GIMMICK_ITEM_POINT_FIELD_MAX }, 
};

//抽選アイテムセットテーブル
static const FES_ITEM_SET DATA_ItemSetTbl[FMISSION_ITEM_SET_MAX] = {
  { { {ITEM_AKAIKAKERA, 25}, {ITEM_AOIKAKERA, 25}, {ITEM_MIDORINOKAKERA, 25}, {ITEM_KIIROIKAKERA, 25} } },
  { { {ITEM_MONSUTAABOORU, 30}, {ITEM_MONSUTAABOORU, 30}, {ITEM_MONSUTAABOORU, 30}, {ITEM_PUREMIABOORU, 10} } },
  { { {ITEM_MOMONNOMI, 25}, {ITEM_ORENNOMI, 25}, {ITEM_KAGONOMI, 25}, {ITEM_KURABONOMI, 25} } },
  { { {ITEM_OBONNOMI, 33}, {ITEM_KIINOMI, 33}, {ITEM_RAMUNOMI, 33}, {ITEM_NAZONOMI, 1} } },
};


static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, u8 id );
static BOOL sub_SetPosByLot( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, u16 zone_id, GFL_STD_RandContext* rnd_c );
static u16 sub_LotItem( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, GFL_STD_RandContext* rnd_c );
static BOOL sub_SetEffect( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk );

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ITEM_ZoneChangeSetup( FES_GIMMICK* sys_wk, u16 zone_id )
{
  GFL_STD_RandContext rnd_c;
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)&sys_wk->wk_buf;

  //キラオブジェクト表示をリセット
  FLDEFF_FES_KIRA_ObjDelAll( sys_wk->eff_task );

  //ワーククリア
  MI_CpuClear32( wk, sizeof(FES_GIMMICK_ITEM_WORK) );

  //乱数コンテキスト初期化
  FES_GIMMICK_SUB_RndInit( sys_wk->fmwk->key, zone_id, &rnd_c );

  sub_SetPosByLot( sys_wk, wk, zone_id, &rnd_c );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　フィールド復帰時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ITEM_FieldRecoverSetup( FES_GIMMICK* sys_wk )
{
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)&sys_wk->wk_buf;
  sub_SetEffect( sys_wk, wk );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　ミッション終了時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ITEM_Delete( FES_GIMMICK* sys_wk )
{
  GFL_STD_RandContext rnd_c;
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)&sys_wk->wk_buf;

  //キラオブジェクト表示をリセット
  FLDEFF_FES_KIRA_ObjDelAll( sys_wk->eff_task );

  //ワーククリア
  MI_CpuClear32( wk, sizeof(FES_GIMMICK_ITEM_WORK) );
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
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)&sys_wk->wk_buf;
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
  FES_GIMMICK_ITEM_WORK* wk = (FES_GIMMICK_ITEM_WORK*)&sys_wk->wk_buf;
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
  return NULL;
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
  u16 item = FLDEFF_FES_KIRA_ObjItemGet( sys_wk->eff_task, id );

  FLDEFF_FES_KIRA_ObjDel( sys_wk->eff_task, id );

  //イベント生成
  if( item == 0 ){
    //万一アイテム未設定なら、エフェクトだけ消してイベント起こさず帰る
    return NULL;
  } 
  event = SCRIPT_SetEventScript( sys_wk->gsys, SCRID_FMISSION_ITEM_GET, NULL, HEAPID_FIELDMAP );
  sc = SCRIPT_GetScriptWorkFromEvent( event );
  SCRIPT_SetScriptWorkParam( sc, item, 0, 0, 0 );

  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　POS抽選
 */
//----------------------------------------------------------------------
static BOOL sub_SetPosByLot( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, u16 zone_id, GFL_STD_RandContext* rnd_c )
{
  int i, pos_num;
  const POS_EVENT_DATA *pos;

  EVENTDATA_SYSTEM* evdata = GAMEDATA_GetEventData( sys_wk->gdata );

  wk->now_zone = FIELDMAP_GetZoneID( sys_wk->fieldWork );

  pos = EVENTDATA_GetPosEventDataTbl( evdata, &pos_num ); 

  if( pos == NULL ){  //POSデータそのものがない
    return FALSE;
  }

  wk->data_max = 0;
  for(i = 0;i < pos_num;i++, pos++){
    if( pos->check_type != POS_CHECK_TYPE_DUMMY_FES ){
      continue;
    }
    wk->data[wk->data_max++] = *pos;
    if( wk->data_max >= FES_GIMMICK_ITEM_DATA_MAX ){
      break;  //上限超えたのでこれ以上サーチしない
    }
  }

  if( wk->data_max == 0 ){
    return FALSE; //フェス用POSが無い
  }

  //抽選するポイント数を決定
  {
    u8 min = DATA_LotTargetNum[1][0];
    u8 max = DATA_LotTargetNum[1][1];

    wk->point_max = FES_GIMMICK_SUB_RndGet( rnd_c, (max-min)+1, NULL, 0 ) + min;

    if( wk->point_max > wk->data_max ){
      wk->point_max = wk->data_max;
    }
    if( wk->point_max == 0 ){
      return FALSE; //抽選ポイント0
    }
  }

  //ポイントを抽選
  {
    u32 log[ FES_GIMMICK_ITEM_POINT_MAX ];

    for(i = 0;i < wk->point_max;i++){
      wk->idx[i] = FES_GIMMICK_SUB_RndGet( rnd_c, wk->data_max, log, i );
      wk->item[i] = sub_LotItem( sys_wk, wk, rnd_c);
      log[i] = wk->idx[i];  //抽選ナンバーの結果を取っておく
    }
  }
  return TRUE;
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

  for( i = 0;i < wk->point_max; i++)
  {
    POS_EVENT_DATA* pd = &wk->data[ wk->idx[i] ];

    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetPosEventCenterPos( pd, &vec);
    }else{
      RAIL_LOCATION loc;
      EVENTDATA_GetPosEventCenterRailLocation( pd, &loc );
      FIELD_RAIL_MAN_GetLocationPosition( sys_wk->railman, &loc, &vec );
    }

    FLDEFF_FES_KIRA_ObjAdd( sys_wk->eff_task, i, wk->item[i], &vec );
  }
  return TRUE;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　ポイントアイテム抽選
 */
//----------------------------------------------------------------------
static u16 sub_LotItem( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk, GFL_STD_RandContext* rnd_c )
{
  int i,level = 0;
  u32 rnd;
  const FES_ITEM_SET* ps;

  if( sys_wk->fmwk->param >= FMISSION_ITEM_SET_MAX ){
    ps = &DATA_ItemSetTbl[FMISSION_ITEM_SET_KAKERA];
    GF_ASSERT(0);
  }else{
    ps = &DATA_ItemSetTbl[ sys_wk->fmwk->param ];
  }
  rnd = FES_GIMMICK_SUB_RndGet( rnd_c, 100*LOT_ITEM_PROB_MAG, NULL, 0 );

  for( i = 0;i < FES_ITEM_PARAM_NUM;i++){
    level += ps->prm[i].prob*LOT_ITEM_PROB_MAG;

    if( rnd < level ){
      return ps->prm[i].item;
    }
  }
  GF_ASSERT(0);
  //万一抜けてきちゃったら0番目を返す
  return ps->prm[0].item;
}



