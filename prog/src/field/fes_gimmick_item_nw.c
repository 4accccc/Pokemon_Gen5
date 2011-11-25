//===============================================================================
/*
 *  @file   fmission_gimmick_item.c
 *  @brief  フェスミッション アイテムミッションギミック(NOTWIFI)
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
#include "field/fes_gimmick_item.h"
#include "field/fes_gimmick_local.h"

#include "field/fes_gimmick_item_dat.h"

#define LOT_ITEM_PROB_MAG (1000)  ///<乱数精度を高めるため、100(%)に倍率をかけた値で抽選する->1%=1000

typedef struct _FES_ITEM_PARAM{
  u16 item;
  u16 prob;      ///<抽選確率
}FES_ITEM_PARAM;

typedef struct _FES_ITEM_SET{
  FES_ITEM_PARAM prm[FES_KIRA_ITEM_PARAM_NUM];
}FES_ITEM_SET;

//抽選数の下限と上限
static const u8 DATA_LotTargetNum[][2] = {
  { FES_GIMMICK_ITEM_POINT_LOW_MIN, FES_GIMMICK_ITEM_POINT_LOW_MAX }, 
  { FES_GIMMICK_ITEM_POINT_MIDDLE_MIN, FES_GIMMICK_ITEM_POINT_MIDDLE_MAX }, 
  { FES_GIMMICK_ITEM_POINT_HIGH_MIN, FES_GIMMICK_ITEM_POINT_HIGH_MAX }, 
};
#define LOT_TARGET_NUM_MAX  NELEMS(DATA_LotTargetNum)

//抽選アイテムセットテーブル
#include "fes_gimmick_item.cdat"

static BOOL sub_SetPosByLot( GAMESYS_WORK* gsys, FES_GIMMICK_ITEM_WORK* wk, FEST_MISSION* fmwk, u16 zone_id, GFL_STD_RandContext* rnd_c );
static u16 sub_LotItem( FES_GIMMICK_ITEM_WORK* wk, FEST_MISSION* fmwk, GFL_STD_RandContext* rnd_c );
static void sub_FlagTableUpdate( FES_GIMMICK_ITEM_WORK* wk, u16 zone_id );

//----------------------------------------------------------------------
/*
 *  @brief  フェス　アイテムミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ITEM_ZoneChangeSetup( FES_GIMMICK_ITEM_WORK* wk, GAMESYS_WORK* gsys, LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, u16 zone_id )
{
  GFL_STD_RandContext rnd_c;

  //ワーククリア
  wk->zone_id = zone_id;
  wk->data_max = 0;
  wk->point_max = 0;
  MI_CpuClear8( wk->data, sizeof(BG_TALK_DATA)*FES_GIMMICK_ITEM_DATA_MAX );
  MI_CpuClear8( wk->idx, sizeof(u8)*FES_GIMMICK_ITEM_POINT_MAX );
  MI_CpuClear8( wk->item, sizeof(u16)*FES_GIMMICK_ITEM_POINT_MAX );
  
  //乱数コンテキスト初期化
  FES_GIMMICK_SUB_RndInit( fmwk->key, FES_GIMMICK_SUB_GetRndInitZone(zone_id), &rnd_c );

  sub_SetPosByLot( gsys, wk, fmwk, zone_id, &rnd_c );
  IWASAWA_Printf( "zone_id = %d\n", zone_id ); 
  //フラグ領域セット
  sub_FlagTableUpdate( wk, zone_id );
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　POS抽選
 */
//----------------------------------------------------------------------
static BOOL sub_SetPosByLot( GAMESYS_WORK* gsys, FES_GIMMICK_ITEM_WORK* wk, FEST_MISSION* fmwk, u16 zone_id, GFL_STD_RandContext* rnd_c )
{
  int i, pos_num;
  const BG_TALK_DATA *pos;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM* evdata = GAMEDATA_GetEventData( gdata );

  pos = EVENTDATA_GetTalkBGEventTbl( evdata, &pos_num ); 

  if( pos == NULL ){  //POSデータそのものがない
    return FALSE;
  }

  wk->data_max = 0;
  for(i = 0;i < pos_num;i++, pos++){
    if( pos->type != BG_TALK_TYPE_DUMMY_FES ){
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
    u8 min,max; 
    u8 param = ZONEDATA_GetFesParam( wk->zone_id );
    param = 2;
    if( param == 0 ){
      return FALSE; //ポイント抽選なし
    }
    if( param > LOT_TARGET_NUM_MAX ){
      GF_ASSERT(0);
      return FALSE; //パラメータが不正なので何もしない
    }
    min = DATA_LotTargetNum[--param][0];
    max = DATA_LotTargetNum[param][1];

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
      wk->item[i] = sub_LotItem( wk, fmwk, rnd_c);
      log[i] = wk->idx[i];  //抽選ナンバーの結果を取っておく
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック　ポイントアイテム抽選
 */
//----------------------------------------------------------------------
static u16 sub_LotItem( FES_GIMMICK_ITEM_WORK* wk, FEST_MISSION* fmwk, GFL_STD_RandContext* rnd_c )
{
  int i,level = 0;
  u32 rnd;
  const FES_ITEM_SET* ps;

  if( fmwk->beacon.base_param >= FMISSION_ITEM_SET_MAX ){
    ps = &DATA_ItemSetTbl[FMISSION_ITEM_SET_KAKERA1];
    GF_ASSERT(0);
  }else{
    ps = &DATA_ItemSetTbl[ fmwk->beacon.base_param ];
  }
  rnd = FES_GIMMICK_SUB_RndGet( rnd_c, 100*LOT_ITEM_PROB_MAG, NULL, 0 );

  for( i = 0;i < FES_KIRA_ITEM_PARAM_NUM;i++){
    level += ps->prm[i].prob*LOT_ITEM_PROB_MAG;

    if( rnd < level ){
      return ps->prm[i].item;
    }
  }
  GF_ASSERT(0);
  //万一抜けてきちゃったら0番目を返す
  return ps->prm[0].item;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック 直近のテーブルのフラグ状態をチェックし、
 *          新規テーブルを登録するか、上書きするか決める
 *
 *  @retval TRUE  1つ以上フラグが立っている
 *  @retval FALSE 1つもフラグが立っていない
 */
//----------------------------------------------------------------------
static BOOL sub_FlagTableIsChange( FES_GIMMICK_ITEM_WORK* wk )
{
  int i, idx;
  
  if( wk->flag_wk.tbl_num == 0 ){
    return TRUE;  //最初の一個は必ず登録する
  }else{
    idx = wk->flag_wk.tbl_num-1;
  }
  //直近テーブルのゾーンのフラグが1つでも立っているかチェック
  for(i = 0;i < FES_GIMMICK_ITEM_POINT_MAX; i++ ){
    if( wk->flag_wk.tbl[idx].flag[i] ){
      return TRUE;  //立っているフラグを見つけた
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムミッションギミック ゾーンチェンジ時のフラグ初期化
 */
//----------------------------------------------------------------------
static void sub_FlagTableUpdate( FES_GIMMICK_ITEM_WORK* wk, u16 zone_id )
{
  int i,idx = 0;
  BOOL new_entry = FALSE;

  //データの無いゾーンなら何もしない
  if( wk->data_max == 0 ){
    return;
  }
  //すでにテーブルに登録済みなら何もしない
  for( i = 0;i < wk->flag_wk.tbl_num;i++){
    if( wk->flag_wk.tbl[i].zone_id == zone_id ){
     return;
    }
  }
  //直近のテーブルのフラグが更新されていなければ、領域は上書きされる
  new_entry = sub_FlagTableIsChange( wk );

  if( new_entry ){  //新規登録
    if( wk->flag_wk.tbl_num < FES_GIMMICK_ITEM_FLAG_WORK_NUM ){  //新しい場所に登録
      idx = wk->flag_wk.tbl_num++;
    }else{  //一番古いテーブルを押し出し
      idx = FES_GIMMICK_ITEM_FLAG_WORK_END;
      MI_CpuCopy8( &wk->flag_wk.tbl[1], &(wk->flag_wk.tbl[0]), sizeof(FES_GIMMICK_ITEM_FLAG)*FES_GIMMICK_ITEM_FLAG_WORK_END); 
    }
  }else{  //上書き登録
    if( wk->flag_wk.tbl_num == 0 ){
      idx  = 0;
    }else{
      idx = wk->flag_wk.tbl_num-1;
    }
  }
  MI_CpuClear8( &wk->flag_wk.tbl[ idx ], sizeof(FES_GIMMICK_ITEM_FLAG));
  wk->flag_wk.tbl[ idx ].zone_id = zone_id;

#if defined(DEBUG_ONLY_FOR_iwasawa) || defined(DEBUG_ONLY_FOR_masafumi_saitou)
  {
    int i;
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"zone_flag_tbl = %d -> ", wk->flag_wk.tbl_num );
    for( i = 0; i < wk->flag_wk.tbl_num; i++ ){
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"%d, ", wk->flag_wk.tbl[i].zone_id );
    }
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"\n");
  }
#endif
}

