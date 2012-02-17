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
#include "poke_tool/monsno_def.h"
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
#include "field/fes_gimmick_npc_dat.h"
#include "field/fes_gimmick_local.h"

#define FES_SHOPPING_PRICE_MAX  (59800)  //買い物ミッションの値段最高額

static GMEVENT* sub_MakeEvent( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u8 id );
static BOOL sub_CheckMMdlPos( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
static BOOL sub_CheckHitAnotherEventForGrid( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const VecFx32* ev_pos );
static BOOL sub_CheckHitAnotherEventForRail( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int data_idx, const RAIL_LOCATION* ev_loc );
static void sub_MMdlDelete( FES_GIMMICK_NPC_WORK* wk, int data_idx );

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
 *  @brief  フェス　NPCミッションギミック　イベントクリアフラグセット
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_FlagSet( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u16 zone_id, u16 idx )
{
  FES_GIMMICK_NPC_FLAG* f_tbl;
  if( wk->point_max == 0 ){
    return;
  }
  f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, zone_id );

  f_tbl->flag[idx] = TRUE;
  
  //非表示設定
  MMDL_SetStatusBitVanish( wk->mmdl_tbl[idx], TRUE );
  //OBJ削除
  sub_MMdlDelete( wk, idx );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック　ミッション終了時処理
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_NPC_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  int i, ret = ( wk->data_max > 0 );
 
  if( wk->point_max > 0 ){
    for(i = 0;i < FES_GIMMICK_MAX;i++){
      sub_MMdlDelete( wk, i );
    }
  }

  //ワーククリア
  MI_CpuClear32( wk, sizeof(FES_GIMMICK_NPC_WORK) );

  return ret;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック トレーナー対戦データ加工
 *
 *  フェス対戦トレーナーのPOKEPARTYを渡すと加工して返す
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_FesTrainerPartySet( FES_GIMMICK* sys_wk, POKEPARTY* party )
{
  int i, ct;
  int score;
  if( sys_wk->fmwk->beacon.base_param != FMISSION_NPC_TYPE_TR_BATTLE ){
    GF_ASSERT(0);
    return;
  }
  ct = PokeParty_GetPokeCount( party );

  score = LINK_FESTIVAL_GetNowScore( sys_wk->fest_sys );
  if( score > 30 ){
    score = 30; //増分上限は30レベル
  }
  for( i = 0;i < ct; i++ ){
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
    int level = PP_Get( pp, ID_PARA_level, NULL )+score;
    u32 exp;

    if( level > PTL_LEVEL_MAX ){
      level = PTL_LEVEL_MAX;  //レベルはMax100
    }
    exp = POKETOOL_GetMinExp( PP_Get(pp, ID_PARA_monsno, NULL), PP_Get(pp, ID_PARA_form_no, NULL), level );

    PP_Put( pp, ID_PARA_exp, exp );
    PP_Renew( pp ); //レベルはRenewの中で経験値から再計算される
    IWASAWA_Printf("FesTrainer = %d\n", PP_Get( pp, ID_PARA_level, NULL ) );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック 交換アイテム取得
 */
//----------------------------------------------------------------------
void FES_GIMMICK_NPC_TradeItemGet( FES_GIMMICK* sys_wk, MMDL* mmdl, u16 my_rank, u16 npc_rank, u16* my_item, u16* npc_item )
{
  const NITEM_TRADE_DATA* tbl;
  if( sys_wk->fmwk->beacon.base_param != FMISSION_NPC_TYPE_ITEM_TRADE ){
    GF_ASSERT(0);
    *my_item = ITEM_KIZUGUSURI;
    *npc_item = ITEM_IIKIZUGUSURI;
    return;
  }

  tbl = &DATA_NpcItemTrade[sys_wk->fmwk->beacon.ex_param];
  *my_item  = tbl->item[MMDL_GetParam( mmdl, NITEM_TRADE_PARAM_MY_CATEGORY )][my_rank];
  *npc_item = tbl->item[MMDL_GetParam( mmdl, NITEM_TRADE_PARAM_NPC_CATEGORY )][npc_rank];
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック 販売アイテムと値段を取得
 */
//----------------------------------------------------------------------
#define PRICE_RANGE_NUM (50-8)  //0.1刻みで0.8～4.9

void FES_GIMMICK_NPC_ShoppingItemGet( FES_GIMMICK* sys_wk, MMDL* mmdl, u16* item, u16* price )
{
  int value;
  u8 type = FES_GIMMICK_NPC_SUB_GetObjType( sys_wk->fmwk );
  
  if( sys_wk->fmwk->beacon.base_param != FMISSION_NPC_TYPE_SHOPPING ){
    GF_ASSERT(0);
    *item = ITEM_KIZUGUSURI;
    *price = 300; //きずぐすりの基本値段
    return;
  }

  *item  = MMDL_GetParam( mmdl, NITEM_SHOPPING_PARAM_ITEM);
  value  = MMDL_GetParam( mmdl, NITEM_SHOPPING_PARAM_PRICE);

  OS_Printf("value=%d, ",value);
  if(type){ //価格に振れ幅が出る(0.8～4.9)
    int range = ( GFUser_GetPublicRand( PRICE_RANGE_NUM )+8 ); //まず10倍で計算 8-49の乱数を取得
    f32 f_range = FX_Div( FX32_CONST(range), FX32_CONST(10) );  //割10して0.8～4.9の乱数に変換
    fx32 f_value = FX_Mul( FX32_CONST( value ), f_range );
    value = FX_Whole( f_value );  //小数点切捨て
    OS_Printf("range=%d, price=%d\n", range, value );
  }
  if( value > FES_SHOPPING_PRICE_MAX ){
    value = FES_SHOPPING_PRICE_MAX;
  }
  *price = value;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェス　NPCミッションギミック 記憶クイズパラメータを生成
 */
//----------------------------------------------------------------------
#define MEMORY_QUIZ_POKE_START  (MONSNO_TUTAAZYA)
#define MEMORY_QUIZ_POKE_END    (MONSNO_URUGAMOSU)
#define MEMORY_QUIZ_POKE_RANGE  (MEMORY_QUIZ_POKE_END-MEMORY_QUIZ_POKE_START+1)
static u16 mquiz_LotPoke( u16 range, u16* log, int log_num );

void FES_GIMMICK_NPC_MemoryQuizParamGet( FES_GIMMICK* sys_wk, u16* q_num, u16* q_idx, u16* a_idx )
{
  int i,answer_max;
  u16 rnd_log[NPC_MEMORY_QUIZ_QUESTION_MAX];
  u8  rnd_idx[NPC_MEMORY_QUIZ_QUESTION_MAX];
  FES_GIMMICK_NPC_WORK* wk = (FES_GIMMICK_NPC_WORK*)sys_wk->wk_buf;
  FES_GIMMICK_NPC_MEMORY_QUIZ_WORK* twk = &(sys_wk->tmp_wk.npc_tmp_wk);

  MI_CpuClear8( twk, sizeof(FES_GIMMICK_NPC_MEMORY_QUIZ_WORK) );

  //出題数を決める
  {
    twk->q_num = LINK_FESTIVAL_GetMyScore( sys_wk->fest_sys );  //現在のマイスコア
    if( twk->q_num < NPC_MEMORY_QUIZ_QUESTION_MIN ){
      twk->q_num = NPC_MEMORY_QUIZ_QUESTION_MIN;
    }else if( twk->q_num > NPC_MEMORY_QUIZ_QUESTION_MAX ){
      twk->q_num = NPC_MEMORY_QUIZ_QUESTION_MAX;
    }
  }

  //出題ポケモンを抽選
  if( twk->q_num < NPC_MEMORY_QUIZ_ANSWER_MAX ){
    answer_max = NPC_MEMORY_QUIZ_ANSWER_MAX;
  }else{
    answer_max = twk->q_num;
  }
  {
    int range = wk->tmp_data_size/sizeof(u16);
    u16* poke_list = (u16*)wk->tmp_data_buf;

    for( i = 0;i < answer_max; i++ ){
      rnd_idx[i] = i;   //連番初期化しておく
      rnd_log[i] = mquiz_LotPoke( range, rnd_log, i );
      twk->q_poke[i] = poke_list[ rnd_log[i] ];
    }
  }
  //出題idxを抽選
  twk->question_idx = GFUser_GetPublicRand( twk->q_num );

  //配列シャッフル
  for( i = 0;i < answer_max; i++ ){
    u8 tmp = rnd_idx[i];
    u8 rnd =  GFUser_GetPublicRand(answer_max);
    rnd_idx[i] = rnd_idx[ rnd ];
    rnd_idx[rnd] = tmp;
  }
  //前5つを回答候補にする
  twk->answer_idx = 0xFF;
  for( i = 0;i < NPC_MEMORY_QUIZ_ANSWER_MAX;i++){
    twk->a_poke[i] = twk->q_poke[rnd_idx[i]];
    if( rnd_idx[i] == twk->question_idx ){
      twk->answer_idx = i;
    }
  }
  //出題idxが回答候補に入っていなかったら、ランダムで上書き
  if( twk->answer_idx == 0xFF ){
    twk->answer_idx = GFUser_GetPublicRand( NPC_MEMORY_QUIZ_ANSWER_MAX );
    twk->a_poke[ twk->answer_idx ] = twk->q_poke[ twk->question_idx ];
  }

  *q_num = twk->q_num;
  *q_idx = twk->question_idx;
  *a_idx = twk->answer_idx;
}

//出題ポケモン
u16 FES_GIMMICK_NPC_MemoryQuizParamGetQuestionPoke( FES_GIMMICK* sys_wk, u8 idx )
{
  FES_GIMMICK_NPC_MEMORY_QUIZ_WORK* twk = &(sys_wk->tmp_wk.npc_tmp_wk);

  return twk->q_poke[idx];
}
//回答ポケモン
u16 FES_GIMMICK_NPC_MemoryQuizParamGetAnswerPoke( FES_GIMMICK* sys_wk, u8 idx )
{
  FES_GIMMICK_NPC_MEMORY_QUIZ_WORK* twk = &(sys_wk->tmp_wk.npc_tmp_wk);

  return twk->a_poke[idx];
}

//----------------------------------------------------------------------
/*
 *  記憶クイズ 出題ポケモン抽選
 */
//----------------------------------------------------------------------

//同じものが抽選されないようにチェック
static BOOL mquiz_LotPokeConflictCheck( u16 poke, u16* log, int log_num )
{
  int i;

  for( i = 0;i < log_num; i++ ){
    if( poke == log[i] ) return TRUE;
  }
  return FALSE;
}
//ポケモンNo抽選
static u16 mquiz_LotPoke( u16 range, u16* log, int log_num )
{
  u16 poke;
  do{
    poke = GFUser_GetPublicRand( range );
  }while( mquiz_LotPokeConflictCheck( poke, log, log_num ));

  return poke;
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
        sub_MMdlDelete( wk, i );
      }
    }else{
      RAIL_LOCATION loc;
      EVENTDATA_GetBGTalkCenterRailLocation( pd, &loc );
      if( sub_CheckHitAnotherEventForRail( sys_wk, wk, i, &loc ) ){
        //他のイベントと重なったので隠す&ゾーンチェンジするまで再チェックしないようにフラグ立てちゃう
        f_tbl->flag[i] = TRUE;
        sub_MMdlDelete( wk, i );
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
  //動作モデルとの判定を取る(フェスNPC自身は無視)
  mmdl = MMDLSYS_SearchGridPosEx2( sys_wk->mmdl_sys,
            SIZE_GRID_FX32(ev_pos->x), SIZE_GRID_FX32(ev_pos->z), ev_pos->y, H_DIFF_COLLISION_FOR_GROUND, FALSE, wk->mmdl_tbl[data_idx] );
  if( mmdl != NULL ){
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
  
  //動作モデルとの判定を取る(フェスNPC自身は無視)
  mmdl = MMDLSYS_SearchRailLocationEx( sys_wk->mmdl_sys, ev_loc, FALSE, wk->mmdl_tbl[data_idx] );
  if( mmdl != NULL ){
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
 *  @brief  NPCミッションギミック　MMDLを削除
 */
//----------------------------------------------------------------------
static void sub_MMdlDelete( FES_GIMMICK_NPC_WORK* wk, int idx )
{
  if( wk->mmdl_tbl[idx] != NULL ){
    //OBJ削除
    MMDL_Delete( wk->mmdl_tbl[idx] );
    wk->mmdl_tbl[idx] = NULL;
  }
}


//=====================================================================
//以下デバッグルーチン
#ifdef PM_DEBUG

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック 配置場所数チェック　
 */
//----------------------------------------------------------------------
int DEB_FES_GIMMICK_NPC_PosNumGet( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk )
{
  return wk->point_max;
}

//----------------------------------------------------------------------
/*
 *  @brief  NPCミッションギミック 配置場所チェック　
 */
//----------------------------------------------------------------------
BOOL DEB_FES_GIMMICK_NPC_PosGet( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, int pos_idx, VecFx32* vec,int* param )
{
  FES_GIMMICK_NPC_FLAG* f_tbl = FES_GIMMICK_NPC_SUB_FlagTableGet( wk, wk->zone_id );

  if( pos_idx < wk->point_max )
  {
    int i;
    BG_TALK_DATA* pd = &wk->data[ wk->idx[pos_idx] ];

    if( pd->pos_type == EVENTDATA_POSTYPE_GRID ){
      EVENTDATA_GetBGTalkCenterPos( pd, vec);
    }else{
      RAIL_LOCATION loc;
      EVENTDATA_GetBGTalkCenterRailLocation( pd, &loc );
      FIELD_RAIL_MAN_GetLocationPosition( sys_wk->railman, &loc, vec );
    }
    for( i = 0;i < FES_GIMMICK_NPC_POINT_PARAM_MAX;i++ ){
      param[i] = wk->param[pos_idx][i];
    }
    param[FES_GIMMICK_NPC_POINT_PARAM_MAX] = wk->idx[pos_idx];
    if( f_tbl->flag[pos_idx] || wk->mmdl_tbl[pos_idx] == NULL ){
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

#endif  //PM_DEBUG

