//======================================================================
/**
 * @file bsubway_tr.c
 * @brief バトルサブウェイ　トレーナー関連
 * @author  Miyuki Iwasawa
 * @date  06.04.17
 * @note GSより移植 kaga
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "savedata/battle_rec.h"

#include "gamesystem/btl_setup.h"
#include "poke_tool/poke_tool.h"
#include "tr_tool/tr_tool.h"
#include "battle/battle.h"
#include "pm_define.h"
#include "buflen.h"
#include "sound/pm_sndsys.h"

#include "fldmmdl.h"
#include "field_encount.h"
#include "message.naix"

#include "bsubway_scr_def.h"
#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "savedata/bsubway_savedata.h"

//#include "../mystery/fntequ.h"

#define __BSUBWAY_FLD_H_GLOBAL
#include "bsubway_tr.h"
#include "event_bsubway.h"

//正式データが出来るまでインクルード
#include "item/itemsym.h"
#include "poke_tool/monsno_def.h"
#include "waza_tool/wazano_def.h"
//#include "battle/battle_common.h"
#include "tr_tool/trtype_def.h"
#include "battle/bsubway_battle_data.h"
#include "tr_tool/trno_def.h"
//#include "b_tower_fld.dat"

#include "fld_btl_inst_tool.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static u16 get_Rand( BSUBWAY_SCRWORK *wk );

//======================================================================
//  バトルサブウェイ関連
//======================================================================
//--------------------------------------------------------------
/**
 * BATTLE_SETUP_PARAM作成
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param gsys GAMESYS_WORK
 * @retval BATTLE_SETUP_PARAM
 */
//--------------------------------------------------------------
BATTLE_SETUP_PARAM * BSUBWAY_SCRWORK_CreateBattleParam(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
  HEAPID heapID = HEAPID_PROC;
  BATTLE_SETUP_PARAM *dst;
  POKEPARTY *party;

  party = PokeParty_AllocPartyWork( GFL_HEAP_LOWID(heapID) );
   
  {
    int i;
	  POKEMON_PARAM * pp;
    const POKEPARTY *myparty = BSUBWAY_SCRWORK_GetPokePartyUse( wk );
	  PokeParty_Init( party, wk->member_num );
    for( i=0; i<wk->member_num; i++ ){
      pp = PokeParty_GetMemberPointer( myparty, wk->member[i] );
      PokeParty_Add( party, pp );
      
      KAGAYA_Printf( "BSW BTL POKE MENBER No.%d Pos %d\n",
          i, wk->member[i] );
    }
	}

#ifdef PM_DEBUG
  if( wk->play_mode == BSWAY_MODE_MULTI ||
      wk->play_mode == BSWAY_MODE_S_MULTI ){
      KAGAYA_Printf(
          "セットするAIパートナーのトレーナータイプ=%d\n",
          wk->five_data[wk->partner].bt_trd.tr_type );
  }
#endif

  dst = FBI_TOOL_CreateBattleParam(
          gsys, party, wk->play_mode,
          wk->tr_data,
          &wk->five_data[wk->partner],
          wk->member_num );

  GFL_HEAP_FreeMemory(party);
  
  return dst;
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
COMM_BTL_DEMO_PARAM * BSUBWAY_SCRWORK_CreateBattleDemoParam(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys )
{
  COMM_BTL_DEMO_PARAM *demo;
  demo = &bsw_scr->comm_btl_demo_param;
  return( demo );
}

//======================================================================
//  トレーナーデータ
//======================================================================
//--------------------------------------------------------------
//  トレーナー番号テーブル　ノーマル １～３週目のラウンド１～６
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable_Normal1_6[3][2] =
{
  {  1-1,  50-1}, //stage 1 round 1-6
  { 31-1,  70-1}, //stage 2 round 1-6
  { 51-1, 110-1}, //stage 3 round 1-6
};

//--------------------------------------------------------------
//  トレーナー番号テーブル　ノーマル １～２週目のラウンド７
//  ３週目は専用の番号になる。
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable_Normal_Round7[2][2] =
{
  {  51-1,  70-1}, //stage 1 round 7
  { 71-1,  90-1}, //stage 2 round 7
};

//--------------------------------------------------------------
//  トレーナー番号テーブル　スーパー　１～４週目のラウンド１～６
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable_Super1_6[4][2]=
{
  {111-1,160-1},
  {141-1,180-1},
  {161-1,200-1},
  {181-1,220-1},
};

//--------------------------------------------------------------
//  トレーナー番号テーブル　スーパー　１～４週目のラウンド７
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable_Super7[4][2]=
{
  {161-1,180-1},
  {181-1,200-1},
  {201-1,220-1},
  {221-1,240-1},
};

//--------------------------------------------------------------
//  トレーナー番号テーブル　スーパー　５週目以降の１～７
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable_Super_Stage5[1][2]=
{
  {201-1,300-1}, //全部同じ
};

//--------------------------------------------------------------
/**
 * @brief  トレーナーナンバー抽選
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param stage ステージ数
 * @param round 何人目か
 * @param play_mode プレイモード
 * @retval u16 トレーナーナンバー
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetTrainerNo(
    BSUBWAY_SCRWORK* wk, u16 stage, u8 round, int play_mode, u8 pair_no )
{
  u16 no = 0;
  u16 super = 0;
  s16 boss_no = -1;
  const u16 *tbl;
  
#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "BSW GET TRAINER NO stage = %d round = %d\n", stage, round );
#endif

  switch( play_mode ) //スーパーモードか？
  {
  case BSWAY_MODE_S_SINGLE:
  case BSWAY_MODE_S_DOUBLE:
  case BSWAY_MODE_S_MULTI:
  case BSWAY_MODE_S_COMM_MULTI:
    super = 1;
  }
  
  if( round == 6 ) //専用ボスか？
  {
    if( super == 0 )  //ノーマル
    {
      if( stage == 2 )
      {
        boss_no = 0;  //ノーマルボス
      }
    }
    else if( stage == 6 ) //スーパーでステージ6
    {
      boss_no = 1;  //スーパーボス
    }
  }
  
  if( boss_no != -1 ) //専用ボス
  {
    switch( play_mode )
    {
    case BSWAY_MODE_SINGLE:
    case BSWAY_MODE_S_SINGLE:
      if( boss_no == 0 )
      {
        no = BSW_TR_DATANO_SINGLE;
      }
      else if( boss_no == 1 )
      {
        no = BSW_TR_DATANO_S_SINGLE;
      }
      break;
    case BSWAY_MODE_DOUBLE:
    case BSWAY_MODE_S_DOUBLE:
      if( boss_no == 0 )
      {
        no = BSW_TR_DATANO_DOUBLE;
      }
      else if( boss_no == 1 )
      {
        no = BSW_TR_DATANO_S_DOUBLE;
      }
      break;
    case BSWAY_MODE_MULTI:
    case BSWAY_MODE_COMM_MULTI:
    case BSWAY_MODE_S_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      if( boss_no == 0 )
      {
#if 0
        if( pair_no == 0 )
        {
          no = BSW_TR_DATANO_MULTI0; //ノボリ
        }
        else
        {
          no = BSW_TR_DATANO_MULTI1; //クダリ
        }
#else //メッセージ演出に合わせる
        if( pair_no == 0 )
        {
          no = BSW_TR_DATANO_MULTI1; //クダリ
        }
        else
        {
          no = BSW_TR_DATANO_MULTI0; //ノボリ
        }
#endif
      }
      else if( boss_no == 1 )
      {
#if 0
        if( pair_no == 0 )
        {
          no = BSW_TR_DATANO_S_MULTI0;
        }
        else
        {
          no = BSW_TR_DATANO_S_MULTI1;
        }
#else //メッセージ演出に合わせる
        if( pair_no == 0 )
        {
          no = BSW_TR_DATANO_S_MULTI1;
        }
        else
        {
          no = BSW_TR_DATANO_S_MULTI0;
        }
#endif
      }
      break;
    default:
      GF_ASSERT( 0 );
    }
  }
  else if( super == 0 ) //ノーマルモード
  {
    if( stage < 3 ) //ステージ２まで
    {
      if( round < 6 ) //ラウンド１～６
      {
        tbl = TrainerNoRangeTable_Normal1_6[stage];
        no = tbl[1] - tbl[0] + 1;
        no = tbl[0] + (get_Rand(wk) % no);
      }
      else  //ラウンド７
      {
        if( stage < 2 )
        {
          tbl = TrainerNoRangeTable_Normal_Round7[stage];
          no = tbl[1] - tbl[0] + 1;
          no = tbl[0] + (get_Rand(wk) % no);
        }
        else //ステージ２以上はありえない
        {
          GF_ASSERT( 0 );
        }
      }
    }
    else //ステージ３以上はありえない
    {
      GF_ASSERT( 0 );
    }
  }
  else //スーパーモード
  {
    if( stage < 4 ) //ステージ４まで
    {
      if( round < 6 ) //ラウンド１～６
      {
        tbl = TrainerNoRangeTable_Super1_6[stage];
        no = tbl[1] - tbl[0] + 1;
        no = tbl[0] + (get_Rand(wk) % no);
      }
      else //ラウンド７
      {
        tbl = TrainerNoRangeTable_Super7[stage];
        no = tbl[1] - tbl[0] + 1;
        no = tbl[0] + (get_Rand(wk) % no);
      }
    }
    else //スーパーステージ５以降
    {
      tbl = TrainerNoRangeTable_Super_Stage5[0];
      no = tbl[1] - tbl[0] + 1;
      no = tbl[0] + (get_Rand(wk) % no);
    }
  }
#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "BSW GET TRAINER NO GET NUM = %d\n", no );
#endif
  return no;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーデータ生成
 * （ロムデータをBSUBWAY_PARTNER_DATA構造体に展開）
 * @param tr_data  [in/out]生成するBSUBWAY_PARTNAER_DATA構造体
 * @param tr_no [in]生成元になるトレーナーID
 * @param cnt [in]トレーナーに持たせるポケモンの数
 * @param set_poke_no [in]ペアトレーナーのポケモン（NULL チェックなし）
 * @param set_item_no [in]ペアのポケモンの装備アイテム（NULL チェックなし）
 * @param poke [in/out] 抽選ポケモン二体のパラメータ格納先(NULLだとなし）
 * @param heapID  [in]ヒープID
 * @retval TRUE 抽選ループが50回超えた FALSE 抽選ループが50回以内で終わった
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理tower_tool.c FSRomBattleTowerTrainerDataMake
 * TOWER_AI_MULTI_ONLY サロンで、5人衆の手持ちポケモンを生成している
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_MakeRomTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no,u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID)
{
  BOOL      ret = 0;

#if 0     //100424 del saito  
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  //トレーナーデータセット
  trd = alloc_TrainerRomData(tr_data,tr_no,heapID);

  //ポケモンデータをセット
  ret = set_BSWayPokemonParam(
      bsw_scr,trd,tr_no,&tr_data->btpwd[0],cnt,
      set_poke_no,set_item_no,poke,heapID);
  GFL_HEAP_FreeMemory( trd );
#else
  ret = FBI_TOOL_MakeRomTrainerData(
      tr_data, tr_no, cnt, set_poke_no, set_item_no, poke, heapID);
#endif  
  return ret;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーの持ちポケモンのパワー乱数を決定する
 * @param  tr_no  トレーナーナンバー
 * @return  パワー乱数
 * b_tower_fld.c→b_tower_evに移動
 */
//--------------------------------------------------------------
static u8 get_PowerRnd(u16 tr_no)
{
  u8  pow_rnd;

  if(tr_no<100){
    pow_rnd=(0x1f/8)*1;
  }
  else if(tr_no<120){
    pow_rnd=(0x1f/8)*2;
  }
  else if(tr_no<140){
    pow_rnd=(0x1f/8)*3;
  }
  else if(tr_no<160){
    pow_rnd=(0x1f/8)*4;
  }
  else if(tr_no<180){
    pow_rnd=(0x1f/8)*5;
  }
  else if(tr_no<200){
    pow_rnd=(0x1f/8)*6;
  }
  else if(tr_no<220){
    pow_rnd=(0x1f/8)*7;
  }
  else{
    pow_rnd=0x1f;
  }
  return pow_rnd;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ ペアトレーナーデータ再生成
 * （セーブされたAIマルチパートナーのデータを
 * BSUBWAY_PARTNER_DATA構造体に展開）
 * @param tr_data [in/out] 生成するBSUBWAY_PARTNAER_DATA構造体
 * @param tr_no [in]  生成元になるトレーナーID
 * @param fixitem [in] TUREなら固定アイテム、FALSEならromアイテムをセット
 * @param poke  [in] ポケモンデータ再生成に必要な構造体型データへのポインタ
 * @param heapID [in] ヒープID
 * @retval nothing
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY AIマルチで休むの後、再開した時に呼ばれる
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_MakePartnerRomData(
    BSUBWAY_SCRWORK *wk, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no,BOOL itemfix,
    const BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID)
{
  int  i;
  u8  pow_rnd = 0;
  BSUBWAY_TRAINER_ROM_DATA  *trd;

  //トレーナーデータセット
//  trd = alloc_TrainerRomData(tr_data,tr_no,heapID); 100425 del saito
  trd = FBI_TOOL_AllocTrainerRomData( tr_data,tr_no,heapID );

  //ポケモンデータをセット
  pow_rnd=get_PowerRnd(tr_no);

  for(i = 0;i < BSUBWAY_STOCK_PAREPOKE_MAX;i++){
#if 0   //100425 del saito    
    make_PokemonParam(
      wk,&(tr_data->btpwd[i]),
      poke->poke_no[i],poke->poke_tr_id,
      poke->poke_rnd[i],pow_rnd,i,itemfix,heapID);
#endif
    FBI_TOOL_MakePokemonParam(
        &(tr_data->btpwd[i]),
        poke->poke_no[i],poke->poke_tr_id,
        poke->poke_rnd[i],pow_rnd,i,itemfix,heapID );
  }
  GFL_HEAP_FreeMemory( trd );
}

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 乱数取得
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval u16 乱数
 * @note 乱数固定の名残
 */
//--------------------------------------------------------------
static u16 get_Rand( BSUBWAY_SCRWORK *wk )
{
  return( GFUser_GetPublicRand(0xffffffff)/65535 );
}

//======================================================================
//  wb null
//======================================================================
#if 0
static void btwoer_TrainerDataMake(
    u8 play_mode,BSUBWAY_TRAINER* src,BSP_TRAINER_DATA * dest, HEAPID heapID )
{
  MI_CpuClear8(dest,sizeof(BSP_TRAINER_DATA));

  dest->tr_type = src->tr_type;  //トレーナータイプ
//dest->tr_gra = 0;//src->tr_type;  //トレーナータイプ
  dest->ai_bit = 0xFFFFFFFF;  //最強

  //トレーナー名
  dest->name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE,

//  GFL_STR_CopyBuffer(
  PM_strcpy(dest->name,src->name);

  //勝ち負けメッセージ
  MI_CpuCopy8(src->win_word,&dest->win_word,sizeof(PMS_DATA));
  MI_CpuCopy8(src->lose_word,&dest->lose_word,sizeof(PMS_DATA));

  switch(play_mode){
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
    dest->fight_type = BTL_RULE_SINGLE;
    break;
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_WIFI_MULTI:
    dest->fight_type = BTL_RULE_DOUBLE;
    break;
  }
}
#endif

/**
 *  @brief  トレーナー対戦データ生成
 */
#if 0
static void btltower_SetTrainerData(BSUBWAY_SCRWORK* wk,BATTLE_PARAM* bp,
    BSUBWAY_PARTNER_DATA* tr,u8 tr_id,u8 tr_client,u8 poke_client )
{
  int i;
  POKEMON_PARAM *pp;

  //トレーナーデータをセット
  btower_TrainerDataMake(
      wk->play_mode,&(tr->bt_trd),&(bp->trainer_data[tr_client]));

  //トレーナーID入力
  bp->trainer_id[tr_client] = tr_id;//wk->tr_data[0].bt_trd.tr_type;

  //対戦相手のポケモンデータをセット
  pp = GFL_HEAP_AllocClearMemory( wk->heapID, POKETOOL_GetWorkSize() );

  PokeParty_Init(bp->poke_party[poke_client],wk->member_num);

  for(i = 0;i < wk->member_num;i++){
    make_PokePara(&(tr->btpwd[i]),pp);
    #if 0
    BattleParam_AddPokemon(bp,pp,poke_client);
    #else
    {
      int result;
      result = PokeParty_Add(bp->poke_party[client_no], pp);
    }
    #endif
  }
  GFL_HEAP_FreeMemory( pp );
}
#endif

