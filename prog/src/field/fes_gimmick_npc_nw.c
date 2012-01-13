//===============================================================================
/*
 *  @file   fmission_gimmick_npc.c
 *  @brief  フェスミッション NPCミッションギミック(NOTWIFI)
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
#include "field/fes_gimmick_npc.h"
#include "field/fes_gimmick_local.h"

#include "field/fes_gimmick_npc_dat.h"
#include "arc/fmission_data.naix"

#ifdef PM_DEBUG
 #include "debug/debug_flg.h"
#endif

#define LOT_PROB_MAG (1000)  ///<乱数精度を高めるため、100(%)に倍率をかけた値で抽選する->1%=1000
#define NPC_MISSION_EXPARAM_MAX (3) //拡張パラメータ許容値
#define NPC_MISSION_POINT_NUM_VALUE (8)  //ポイント数バリエーション

//FES_NPCオブジェコードテーブル
#define FES_NPC_PARAM_FIXED  (1) //パラメータ1つで固定

typedef struct FES_NPC_OBJ_PARAM{
  u16 obj_code;
  u16 type;
}FES_NPC_OBJ_PARAM;

typedef struct FES_NPC_PARAM{
  u32 param_num;
  const FES_NPC_OBJ_PARAM* p_tbl;
}FES_NPC_PARAM;

static const FES_NPC_OBJ_PARAM DATA_FesNpcLostChildren[FES_NPC_PARAM_FIXED] = {
  { BABYBOY1, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcPresentItem[FES_NPC_PARAM_FIXED] = {
  { BOY2, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcItemTrade[FMISSION_EXP_ITEM_TRADE_SET_MAX] = {
  { GIRL4, 1 },
  { BACKPACKERM, 0 },
  { BACKPACKERM, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcShopping[FMISSION_EXP_SHOPPING_SET_MAX] = {
  { SHOPM1, 0 },
  { SUNGLASSES, 1 },
  { SUNGLASSES, 1 },
  { SUNGLASSES, 1 },
  { SUNGLASSES, 1 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcRPS[FES_NPC_PARAM_FIXED] = {
  { BABYGIRL2, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcMemoryQuiz[FES_NPC_PARAM_FIXED] = {
  { CLOWN, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcTrBattle[FMISSION_EXP_TRAINER_SET_MAX] = {
  { FIGHTERM, 0 },
  { TRAINERW, 1 },
  { VETERANM, 2 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcGetAPresent[FES_NPC_PARAM_FIXED] = {
  { PCWOMAN1, 0 },
};
static const FES_NPC_OBJ_PARAM DATA_FesNpcPokeQuiz[FES_NPC_PARAM_FIXED] = {
  { ASSISTANTM, 0 },
};

static const FES_NPC_PARAM DATA_FesNpcParam[FMISSION_NPC_TYPE_MAX] = {
  { FES_NPC_PARAM_FIXED, DATA_FesNpcLostChildren },//FES_NPC_TYPE_LOST_CHILDREN 迷子を捜せ
  { FES_NPC_PARAM_FIXED, DATA_FesNpcPresentItem },//FES_NPC_TYPE_PRESENT_ITEM アイテムをプレゼントする
  { FMISSION_EXP_ITEM_TRADE_SET_MAX, DATA_FesNpcItemTrade },//FES_NPC_TYPE_ITEM_TRADE アイテム交換
  { FMISSION_EXP_SHOPPING_SET_MAX, DATA_FesNpcShopping },//FES_NPC_TYPE_SHOPPING アイテム購入
  { FES_NPC_PARAM_FIXED, DATA_FesNpcRPS },//FES_NPC_TYPE_RPS じゃんけん
  { FES_NPC_PARAM_FIXED, DATA_FesNpcMemoryQuiz },//FES_NPC_TYPE_MEMORY_QUIZ 記憶クイズ
  { FMISSION_EXP_TRAINER_SET_MAX, DATA_FesNpcTrBattle },//FES_NPC_TYPE_TR_BATTLE バトル
  { FES_NPC_PARAM_FIXED, DATA_FesNpcGetAPresent },//FES_NPC_TYPE_GET_A_PRESENT アイテムをもらう
  { FES_NPC_PARAM_FIXED, DATA_FesNpcPokeQuiz },//FES_NPC_TYPE_POKE_QUIZ ポケモン図鑑クイズ
};

/////////////////////////////////////////////////////////////////////
//ポイント出現パターンテーブル
static const u8 DATA_FesNpcPointNumTbl[FMISSION_POINT_PAT_MAX][NPC_MISSION_POINT_NUM_VALUE] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 }, ///<FMISSION_POINT_PAT_NONE
  { 0, 0, 0, 0, 0, 0, 1, 1 }, ///<FMISSION_POINT_PAT_RARE
  { 0, 0, 1, 1, 1, 1, 1, 2 }, ///<FMISSION_POINT_PAT_LOW
  { 1, 1, 1, 1, 2, 2, 2, 2 }, ///<FMISSION_POINT_PAT_MIDDLE
  { 1, 1, 2, 2, 2, 3, 3, 3 }, ///<FMISSION_POINT_PAT_HIGH
  { 1, 2, 2, 3, 3, 3, 4, 4 }, ///<FMISSION_POINT_PAT_MAXIMUM
  { 0, 0, 0, 0, 0, 0, 4, 4 }, ///<FMISSION_POINT_PAT_LUNK
};

#include "field/fes_gimmick_npc.cdat"

static void sub_AddMMdl( GAMESYS_WORK* gsys, FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u16 zone_id );
static BOOL sub_SetPosByLot( GAMESYS_WORK* gsys, FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u16 zone_id, GFL_STD_RandContext* rnd_c );
static void sub_LotParam( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log );
static void sub_LotParamPresentItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c );
static void sub_LotParamTradeItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c );
static void sub_LotParamShoppingItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c );
static void sub_LotParamTrBattle( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log );
static void sub_LotParamGetAPresentItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c );
static void sub_LotParamPokeQuiz( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log );

static void sub_FlagTableUpdate( FES_GIMMICK_NPC_WORK* wk, u16 zone_id );

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_ZoneChangeSetup( FES_GIMMICK_NPC_WORK* wk, GAMESYS_WORK* gsys, LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, u16 zone_id )
{
  GFL_STD_RandContext rnd_c;

  //ワーククリア
  wk->fest_sys = fest_sys;
  wk->zone_id = zone_id;
  wk->data_max = 0;
  wk->point_max = 0;
  MI_CpuClear8( wk->data, sizeof(BG_TALK_DATA)*FES_GIMMICK_NPC_DATA_MAX );
  MI_CpuClear8( wk->idx, sizeof(u8)*FES_GIMMICK_NPC_POINT_MAX );
  MI_CpuClear8( wk->param, sizeof(u16)*FES_GIMMICK_NPC_POINT_MAX*FES_GIMMICK_NPC_POINT_PARAM_MAX );
  MI_CpuClear8( wk->mmdl_tbl, sizeof(MMDL*)*FES_GIMMICK_NPC_POINT_MAX );

  //乱数コンテキスト初期化
  FES_GIMMICK_SUB_RndInit( fmwk->key, FES_GIMMICK_SUB_GetRndInitZone(zone_id), &rnd_c );

  sub_SetPosByLot( gsys, wk, fmwk, zone_id, &rnd_c );
  
  //フラグ領域セット
  sub_FlagTableUpdate( wk, zone_id );

  //動作モデル初期化
  if( wk->point_max ){
    sub_AddMMdl( gsys, wk, fmwk, zone_id );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　ミッション開始時１回のみの初期化処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_MissionInitSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  ARCHANDLE* handle;
  int id;
  switch( sys_wk->fmwk->beacon.base_param ){
  case FMISSION_NPC_TYPE_MEMORY_QUIZ:
    id = NARC_fmission_data_fmission_quiz_pokelist00_bin+FMISSION_EXP_POKE_QUIZ_LOCAL; //固定で地方図鑑データ
    break;
  case FMISSION_NPC_TYPE_POKE_QUIZ:
    id = NARC_fmission_data_fmission_quiz_pokelist00_bin+sys_wk->fmwk->beacon.ex_param;
    break;
  default:
    return;
  }
  handle = LINK_FESTIVAL_GetFMissionDataHandle( sys_wk->fest_sys );

  wk->tmp_data_size = GFL_ARC_GetDataSizeByHandle( handle, id );
  if( wk->tmp_data_size > FES_GIMMICK_NPC_TMPDATA_BUF_SIZE ){
    GF_ASSERT(0);
    wk->tmp_data_size = FES_GIMMICK_NPC_TMPDATA_BUF_SIZE;
  }
  GFL_ARC_LoadDataOfsByHandle( handle, id, 0, wk->tmp_data_size, wk->tmp_data_buf );
}

static const FES_NPC_OBJ_PARAM* sub_GetObjParam( const FEST_MISSION* fmwk )
{
  const FES_NPC_PARAM* param = &DATA_FesNpcParam[fmwk->beacon.base_param];
  int idx = fmwk->beacon.ex_param;

  IWASAWA_Printf("%d, %d\n",fmwk->beacon.base_param, fmwk->beacon.ex_param);
  if( param->param_num == FES_NPC_PARAM_FIXED ){
    idx = 0;
  }else if( fmwk->beacon.ex_param >= param->param_num ){
    GF_ASSERT(0);
    idx = 0;
  }
  return &param->p_tbl[idx];

}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック ミッションパラメータからオブジェタイプを求める
 */
//----------------------------------------------------------------------
u8 FES_GIMMICK_NPC_SUB_GetObjType( const FEST_MISSION* fmwk )
{
  const FES_NPC_OBJ_PARAM* param = sub_GetObjParam( fmwk );
  return param->type;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック ミッションパラメータからOBJコードを決める
 */
//----------------------------------------------------------------------
u16 FES_GIMMICK_NPC_SUB_GetObjCode( FES_GIMMICK_NPC_WORK* wk, const FEST_MISSION* fmwk )
{
  const FES_NPC_OBJ_PARAM* param = sub_GetObjParam( fmwk );
  return param->obj_code;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック 指定ゾーンのフラグテーブル取得
 */
//----------------------------------------------------------------------
FES_GIMMICK_NPC_FLAG* FES_GIMMICK_NPC_SUB_FlagTableGet( FES_GIMMICK_NPC_WORK* wk, u16 zone_id )
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

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　MMDL追加
 */
//----------------------------------------------------------------------
static void sub_AddMMdl( GAMESYS_WORK* gsys, FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u16 zone_id )
{
  int i;
  MMDL_HEADER head;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  FES_GIMMICK_NPC_FLAG* f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, zone_id );

  MI_CpuClear8(&head,sizeof(MMDL_HEADER));
  
  head.obj_code = FES_GIMMICK_NPC_SUB_GetObjCode( wk, fmwk );
  head.dir = DIR_DOWN;
  head.event_id = SCRID_FMISSION_NPC_LOST_CHILDREN+fmwk->beacon.base_param;

  for( i = 0;i < wk->point_max; i++)
  {
    MMDL* mmdl;
    MMDLSYS* mmdlsys = GAMEDATA_GetMMdlSys( gdata );
    BG_TALK_DATA* pd = &wk->data[ wk->idx[i] ];

    if( f_tbl->flag[i] ){
      continue; //すでに終了
    }

    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      const BG_TALK_DATA_GPOS* cp_pos = (const BG_TALK_DATA_GPOS*)pd->pos_buf;
 
      MMDLHEADER_SetGridPos( &head, cp_pos->gx, cp_pos->gz, cp_pos->height );
      head.move_code = MV_DIR_RND;
    }else{
      const BG_TALK_DATA_RPOS * cp_pos = (const BG_TALK_DATA_RPOS *)pd->pos_buf;

      MMDLHEADER_SetRailPos( &head, cp_pos->rail_index, cp_pos->front_grid, cp_pos->side_grid );
      head.move_code = MV_RAIL_DIR_RND_ALL;
    }
    head.id = MMDL_ID_FMISSION_OBJ_01+i;
    head.param0 = wk->param[i][0];
    head.param1 = wk->param[i][1];
    head.param2 = wk->param[i][2];

    wk->mmdl_tbl[i] = MMDLSYS_AddMMdl( mmdlsys, &head, zone_id );
    MMDL_OnMoveBit( wk->mmdl_tbl[i], MMDL_MOVEBIT_NOT_SAVE );  //セーブ対象にしない
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　POS抽選
 */
//----------------------------------------------------------------------
static BOOL sub_SetPosByLot( GAMESYS_WORK* gsys, FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u16 zone_id, GFL_STD_RandContext* rnd_c )
{
  int i, pos_num;
  const BG_TALK_DATA *pos;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM* evdata = GAMEDATA_GetEventData( gdata );

  pos = EVENTDATA_GetTalkBGEventTbl( evdata, &pos_num ); 

  if( pos == NULL ){  //BGデータそのものがない
    return FALSE;
  }

  wk->data_max = 0;
  for(i = 0;i < pos_num;i++, pos++){
    if( pos->type != BG_TALK_TYPE_DUMMY_FES_NPC &&
        pos->type != BG_TALK_TYPE_DUMMY_FES ){
      continue;
    }
    wk->data[wk->data_max++] = *pos;
    if( wk->data_max >= FES_GIMMICK_NPC_DATA_MAX ){
      break;  //上限超えたのでこれ以上サーチしない
    }
  }

  if( wk->data_max == 0 ){
    return FALSE; //フェス用BGが無い
  }

  //抽選するポイント数を決定
  {
    FMISSION_MAP_ATTR map_attr = ZONEDATA_GetFesMapAttr( wk->zone_id );
    int bit_ofs, rnd;
    u8 pat_idx;
    
    if( map_attr >= FMISSION_MAP_ATTR_MAX ){
      wk->point_max = 0;
      return FALSE;
    }

    bit_ofs = map_attr*FMISSION_POINT_PATTERN_BIT_NUM;
    pat_idx = (fmwk->beacon.pattern >> bit_ofs ) & FMISSION_POINT_PATTERN_BIT_MASK;
    rnd = FES_GIMMICK_SUB_RndGet( rnd_c, NPC_MISSION_POINT_NUM_VALUE, NULL, 0 );

    wk->point_max = DATA_FesNpcPointNumTbl[pat_idx][rnd];

#ifdef PM_DEBUG
    if( DEBUG_FLG_GetFlg(DEBUG_FLG_FMissionEvPosMax) ){
      //常に最大数抽選
      wk->point_max = FES_GIMMICK_NPC_POINT_MAX; 
    }
#endif  //PM_DEBUG

#if defined(DEBUG_ONLY_FOR_iwasawa) || defined(DEBUG_ONLY_FOR_masafumi_saitou)
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"zone-%d mattr=%d, pat=%08x, ofs=%d, idx=%d, rnd=%d, point=%d\n", 
        wk->zone_id, map_attr, fmwk->beacon.pattern, bit_ofs, pat_idx, rnd, wk->point_max );
#endif

    if( wk->point_max > wk->data_max ){
      wk->point_max = wk->data_max;
    }
    if( wk->point_max == 0 ){
      return FALSE; //抽選ポイント0
    }
  }

  //ポイントを抽選
  {
    u32 idx_log[ FES_GIMMICK_NPC_POINT_MAX ];
    u32 param_log[ FES_GIMMICK_NPC_POINT_MAX ];
    
    for(i = 0;i < wk->point_max;i++){
      wk->idx[i] = FES_GIMMICK_SUB_RndGet( rnd_c, wk->data_max, idx_log, i );
      sub_LotParam( wk, fmwk, i, rnd_c, param_log );
      idx_log[i] = wk->idx[i];  //抽選ナンバーの結果を取っておく
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　ポイントパラメータ抽選
 */
//----------------------------------------------------------------------
static void sub_LotParam( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log )
{
  switch( fmwk->beacon.base_param ){
  case FMISSION_NPC_TYPE_PRESENT_ITEM:
    sub_LotParamPresentItem( wk, fmwk, idx, rnd_c );
    break;
  case FMISSION_NPC_TYPE_ITEM_TRADE:
    sub_LotParamTradeItem( wk, fmwk, idx, rnd_c );
    break;
  case FMISSION_NPC_TYPE_SHOPPING:
    sub_LotParamShoppingItem( wk, fmwk, idx, rnd_c );
    break;
  case FMISSION_NPC_TYPE_TR_BATTLE:
    sub_LotParamTrBattle( wk, fmwk, idx, rnd_c, log );
    break;
  case FMISSION_NPC_TYPE_GET_A_PRESENT:
    sub_LotParamGetAPresentItem( wk, fmwk, idx, rnd_c );
    break;
  case FMISSION_NPC_TYPE_POKE_QUIZ:
    sub_LotParamPokeQuiz( wk, fmwk, idx, rnd_c, log );
    break;
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 道具プレゼント
 */
//----------------------------------------------------------------------
static void sub_LotParamPresentItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c )
{
  u32 rnd = FES_GIMMICK_SUB_RndGet( rnd_c, FES_NITEM_PRESENT_DATA_MAX, NULL, 0 );

  wk->param[idx][NITEM_PRESENT_PARAM_ITEM] = DATA_NpcItemPresent[rnd];
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 道具交換 
 */
//----------------------------------------------------------------------
static void sub_LotParamTradeItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c )
{
  u32 rnd, range;
  u8 tbl_no = fmwk->beacon.ex_param;

  if( tbl_no >= FES_NITEM_TRADE_DATA_MAX ){
    GF_ASSERT(0);
    tbl_no = 0;
  }
  wk->param[idx][NITEM_TRADE_PARAM_TBLNO] = tbl_no;
 
  range = (FES_NITEM_TRADE_CATEGORY_MAX-1);
  rnd = FES_GIMMICK_SUB_RndGet( rnd_c, range, NULL, 0 )+1;

  { //まずカテゴリA-C内で抽選
    int i,swap;

    swap = FES_GIMMICK_SUB_RndGet( rnd_c, 2, NULL, 0 );
    for( i = 0;i < 2;i++){
      wk->param[idx][(i+swap)%2] = (rnd+i)%range;
    }
  }
  //5%の確率でレアテーブルに片方を差し替え
  rnd = FES_GIMMICK_SUB_RndGet( rnd_c, 100*LOT_PROB_MAG, NULL, 0 );
  if( rnd >= (LOT_PROB_MAG*5) ){
    return;
  }
  wk->param[idx][FES_GIMMICK_SUB_RndGet( rnd_c, 2, NULL, 0 )] = range;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 道具販売 
 */
//----------------------------------------------------------------------
static void sub_LotParamShoppingItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c )
{
  u8 no = 0;
  u8 tbl_no = fmwk->beacon.ex_param;

  {
    int i,key = 0;
    u32 rnd = FES_GIMMICK_SUB_RndGet( rnd_c, 100*LOT_PROB_MAG, NULL, 0 );

    for( i = 0; i < FES_NITEM_SHOPPING_LIST_MAX; i++ ){
      key += DATA_NpcItemShoppingProb[i]*LOT_PROB_MAG;

      if( rnd < key ){
        no = i;
        break;
      } 
    }
  }
  wk->param[idx][NITEM_SHOPPING_PARAM_ITEM] = DATA_NpcItemShopping[tbl_no].list[no].item;
  wk->param[idx][NITEM_SHOPPING_PARAM_PRICE] = DATA_NpcItemShopping[tbl_no].list[no].price;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 トレーナーID 
 */
//----------------------------------------------------------------------
static void sub_LotParamTrBattle( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log )
{
  u32 rnd;
  u8 tbl_no = fmwk->beacon.ex_param;
  
  if( tbl_no >= FES_NPC_TRAINER_DATA_MAX ){
    GF_ASSERT(0);
    tbl_no = 0;
  }
  rnd = FES_GIMMICK_SUB_RndGet( rnd_c, FES_NPC_TRAINER_SET_NUM, log, idx );
  wk->param[idx][NPC_TRAINER_PARAM_TRID] = DATA_NpcTrainer[tbl_no][rnd];
  log[idx] = rnd;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 プレゼントをもらう
 */
//----------------------------------------------------------------------
static void sub_LotParamGetAPresentItem( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c )
{
  int no = 0;
  int i,key = 0;
  const NITEM_GET_A_PRESENT_PARAM* tbl = DATA_NpcItemGetAPresent[fmwk->beacon.ex_param].list;
  u32 rnd = FES_GIMMICK_SUB_RndGet( rnd_c, 100*LOT_PROB_MAG, NULL, 0 );

  for( i = 0; i < FES_NITEM_SHOPPING_LIST_MAX; i++ ){
    key += (tbl[i].prob*LOT_PROB_MAG);

    if( rnd < key ){
      no = i;
      break;
    } 
  }
  wk->param[idx][NITEM_GPRESENT_PARAM_ITEM] = tbl[no].item;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック　パラメータ抽選 図鑑クイズ出題抽選
 */
//----------------------------------------------------------------------
static void sub_LotParamPokeQuiz( FES_GIMMICK_NPC_WORK* wk, FEST_MISSION* fmwk, u8 idx, GFL_STD_RandContext* rnd_c, u32* log )
{
  u16* list = (u16*)wk->tmp_data_buf;

  u32 rnd = FES_GIMMICK_SUB_RndGet( rnd_c, wk->tmp_data_size/sizeof(u16), log, idx );
  wk->param[idx][NPC_POKE_QUIZ_PARAM_MONSNO] = list[rnd];
  log[idx] = rnd;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック 指定テーブルのフラグが1つでも立っているかチェック
 *
 *  @retval TRUE  1つ以上フラグが立っている
 *  @retval FALSE 1つもフラグが立っていない
 */
//----------------------------------------------------------------------
static BOOL sub_FlagTableIsChange( FES_GIMMICK_NPC_WORK* wk )
{
  int i, idx;
  
  if( wk->flag_wk.tbl_num == 0 ){
    return TRUE;  //最初の一個は必ず登録する
  }else{
    idx = wk->flag_wk.tbl_num-1;
  }
  //直近テーブルのゾーンのフラグが1つでも立っているかチェック
  for(i = 0;i < FES_GIMMICK_NPC_POINT_MAX; i++ ){
    if( wk->flag_wk.tbl[idx].flag[i] ){
      return TRUE;  //立っているフラグを見つけた
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック ゾーンチェンジ時のフラグ初期化
 */
//----------------------------------------------------------------------
static void sub_FlagTableUpdate( FES_GIMMICK_NPC_WORK* wk, u16 zone_id )
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
    if( wk->flag_wk.tbl_num < FES_GIMMICK_NPC_FLAG_WORK_NUM ){  //新しい場所に登録
      idx = wk->flag_wk.tbl_num++;
    }else{  //一番古いテーブルを押し出し
      idx = FES_GIMMICK_NPC_FLAG_WORK_END;
      MI_CpuCopy8( &wk->flag_wk.tbl[1], &(wk->flag_wk.tbl[0]), sizeof(FES_GIMMICK_NPC_FLAG)*(FES_GIMMICK_NPC_FLAG_WORK_END)); 
    }
  }else{  //上書き登録
    if( wk->flag_wk.tbl_num == 0 ){
      idx  = 0;
    }else{
      idx = wk->flag_wk.tbl_num-1;
    }
  }
  MI_CpuClear8( &wk->flag_wk.tbl[ idx ], sizeof(FES_GIMMICK_NPC_FLAG));
  wk->flag_wk.tbl[ idx ].zone_id = zone_id;

#if defined(DEBUG_ONLY_FOR_iwasawa) || defined(DEBUG_ONLY_FOR_masafumi_saitou)
  {
    int i;
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"zone_flag_tbl = %d -> ", wk->flag_wk.tbl_num);
    for( i = 0; i < wk->flag_wk.tbl_num; i++ ){
      OS_FPrintf(OSPRINT_LINKFEST_PAGE,"%d, ", wk->flag_wk.tbl[i].zone_id );
    }
    OS_FPrintf(OSPRINT_LINKFEST_PAGE,"\n");
  }
#endif
}

