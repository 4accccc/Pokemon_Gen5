//======================================================================
/**
 * @file  field_encount.c
 * @brief フィールド　エンカウント処理
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fieldmap.h"
#include "map_attr.h"

#include "encount_data.h"
#include "field_encount.h"

#include "battle/battle.h"
#include "gamesystem/btl_setup.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/tokusyu_def.h"
#include "poke_tool/poketype.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_personal.h"
#include "tr_tool/tr_tool.h"
#include "item/itemsym.h"

#include "enc_pokeset.h"
#include "event_battle.h"
#include "field_encount_local.h"

#include "debug/debug_flg.h"
#include "sound/wb_sound_data.sadl"

//======================================================================
//  define
//======================================================================
#define FENCOUNT_PL_NULL ///<PL処理無効

#define DEBUG_WB_FORCE_GROUND //エンカウントデータを地上固定する

#define CALC_SHIFT (8) ///<計算シフト値
#define WALK_COUNT_GLOBAL (8) ///<エンカウントしない歩数
#define WALK_COUNT_MAX (0xffff) ///<歩数カウント最大
#define NEXT_PERCENT (40) ///<エンカウントする基本確率
#define WALK_NEXT_PERCENT (5) ///<歩数カウント失敗で次の処理に進む確率

#define LONG_GRASS_PERCENT (30) ///<長い草の中にいるときの加算確率
#define CYCLE_PERCENT (30) ///<自転車に乗っているときの加算確率

#define HEAPID_BTLPARAM (HEAPID_PROC) ///<バトルパラメタ用HEAPID

//--------------------------------------------------------------
/// エンカウントデータ要素
//--------------------------------------------------------------
#ifdef DEBUG_ONLY_FOR_kagaya
//#define DEBUG_ENCOUNT_CHECKOFF_ATTR
#endif

//======================================================================
//  struct
//======================================================================
struct _TAG_FIELD_ENCOUNT
{
  FIELDMAP_WORK *fwork;
  GAMESYS_WORK *gsys;
  GAMEDATA *gdata;
  ENCOUNT_DATA* encdata;
};


//======================================================================
//  proto
//======================================================================
static ENCOUNT_LOCATION enc_GetAttrLocation( FIELD_ENCOUNT *enc );
static u32 enc_GetLocationPercent( FIELD_ENCOUNT *enc,ENCOUNT_LOCATION location );
static BOOL enc_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_WORK* ewk, const u32 per );
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per );

static void enc_CreateBattleParam( FIELD_ENCOUNT *enc, const ENCPOKE_FLD_PARAM* spa,
    BATTLE_SETUP_PARAM *bsp, HEAPID heapID, const ENC_POKE_PARAM* inPokeTbl );

static u32 enc_GetPercentRand( void );

static int enc_GetWalkCount( FIELD_ENCOUNT *enc );
static void enc_AddWalkCount( FIELD_ENCOUNT *enc );
static void enc_ClearWalkCount( FIELD_ENCOUNT *enc );

static void encwork_SetPlayerPos( ENCOUNT_WORK* ewk, const FIELD_PLAYER* player);
static void encwork_AddPlayerWalkCount( ENCOUNT_WORK* ewk, const FIELD_PLAYER* player);
static u32 encwork_GetPlayerWalkCount( ENCOUNT_WORK* ewk );

//======================================================================
//  フィールドエンカウント
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエンカウント　ワーク作成
 * @param fwork FIELDMAP_WORK
 * @retval FIELD_ENCOUNT*
 */
//--------------------------------------------------------------
FIELD_ENCOUNT * FIELD_ENCOUNT_Create( FIELDMAP_WORK *fwork )
{
  HEAPID heapID;
  FIELD_ENCOUNT *enc;

  heapID = FIELDMAP_GetHeapID( fwork );
  enc = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_ENCOUNT) );

  enc->fwork = fwork;
  enc->gsys = FIELDMAP_GetGameSysWork( enc->fwork );
  enc->gdata = GAMESYSTEM_GetGameData( enc->gsys );
  enc->encdata = EVENTDATA_GetEncountDataTable( GAMEDATA_GetEventData(enc->gdata) );
  return( enc );
}

//--------------------------------------------------------------
/**
 * フィールドエンカウント　削除
 * @param enc FIELD_ENCOUNT*
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_ENCOUNT_Delete( FIELD_ENCOUNT *enc )
{
  GFL_HEAP_FreeMemory( enc );
}

//======================================================================
//  フィールドエンカウント　チェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック
 * @param enc FIELD_ENCOUNT
 * @param enc_mode  ENCOUNT_MODE_???
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
void* FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type )
{
  u32 per,enc_num;
  BOOL ret = FALSE;
  BATTLE_SETUP_PARAM* bp;
  ENCOUNT_WORK* ewk;
  ENCOUNT_LOCATION enc_loc;
  ENC_POKE_PARAM poke_tbl[FLD_ENCPOKE_NUM_MAX];

  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  ENCPOKE_FLD_PARAM fld_spa;

  //最後のエンカウントからのプレイヤーの歩数を加算
  ewk = GAMEDATA_GetEncountWork(enc->gdata);
  encwork_AddPlayerWalkCount( ewk, fplayer);

#ifdef PM_DEBUG
  if( enc_type != ENC_TYPE_FORCE && DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEncount) ){
    return NULL;
  }
#endif

  //ロケーションチェック
  enc_loc = enc_GetAttrLocation( enc );
  per = enc_GetLocationPercent( enc, enc_loc );
  if( per <= 0 ){
    return( NULL ); //確率0
  }

  //ENCPOKE_FLD_PARAM作成
  ENCPOKE_SetEFPStruct( &fld_spa, enc->gdata, enc_loc, enc_type, FALSE );

  if( enc_type != ENC_TYPE_FORCE )
  {
    //道具＆特性によるエンカウント率変動
    per = ENCPOKE_EncProbManipulation( &fld_spa, enc->gdata, per);

    if( enc_CheckEncount(enc,ewk,per) == FALSE ){ //エンカウントチェック
      return NULL;
    }
  }

  { //移動ポケモンチェック

  }

  //エンカウントデータ生成
  enc_num = ENCPOKE_GetNormalEncountPokeData( enc->encdata, &fld_spa, poke_tbl );

  if( enc_num == 0 ){ //エンカウント失敗
    return NULL;
  }

  //バトルパラメータセット
  bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
  enc_CreateBattleParam( enc, &fld_spa, bp, HEAPID_BTLPARAM, poke_tbl );

  // プレイヤー座標更新＆歩数カウントクリア
  encwork_SetPlayerPos( ewk, fplayer);

  //エンカウントイベント生成
  return (void*)EVENT_WildPokeBattle( enc->gsys, enc->fwork, bp );
}

//======================================================================
//  サブ　エンカウント確率
//======================================================================

//--------------------------------------------------------------
/**
 * アトリビュートからエンカウントローケーション取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
static ENCOUNT_LOCATION enc_GetAttrLocation( FIELD_ENCOUNT *enc )
{
  MAPATTR attr;
  VecFx32 pos;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( enc->fwork );
  MAPATTR_FLAG attr_flag;

  attr = FIELD_PLAYER_GetMapAttr( fplayer );
  if(MAPATTR_IsEnable(attr) == FALSE){
    return ENC_LOCATION_ERR;
  }

  //エンカウントフラグチェック
  attr_flag = MAPATTR_GetAttrFlag(attr);

#ifdef DEBUG_ENCOUNT_CHECKOFF_ATTR
  attr_flag |= MAPATTR_FLAGBIT_ENCOUNT;
#endif
  if((attr_flag & MAPATTR_FLAGBIT_ENCOUNT) == FALSE ){
    return ENC_LOCATION_ERR;
  }

  //エンカウントアトリビュートチェック
  {
    MAPATTR_VALUE attr_value = MAPATTR_GetAttrValue(attr);
#ifdef DEBUG_WB_FORCE_GROUND
    attr_flag = 0; //@todo 仮　地上で固定
#endif

    //水チェック
    if(attr_flag & MAPATTR_FLAGBIT_WATER){
      return ENC_LOCATION_WATER;
    }
    //地面ハイレベル
    if( MAPATTR_VALUE_CheckEncountGrassB(attr_value) ){
      return ENC_LOCATION_GROUND_H;
    }
  }
  //@todo その他は今のところ地面ローレベルを当てておく 091002
  return ENC_LOCATION_GROUND_L;
}

//--------------------------------------------------------------
/**
 * エンカウント率をロケーションから取得
 * @param enc FIELD_ENCOUNT*
 * @param attr チェックするMAPATTR
 * @param outEncLocation にエンカウント場所を格納するENCOUNT_LOCATION
 * @retval u32 エンカウント率 0=エンカウント無し
 */
//--------------------------------------------------------------
static u32 enc_GetLocationPercent( FIELD_ENCOUNT *enc,ENCOUNT_LOCATION location )
{
  u32 per = 0;

  if(location >= ENC_LOCATION_MAX){
    return 0;
  }
  return enc->encdata->prob[location];
}

//======================================================================
//  サブ　エンカウントチェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック
 * @param enc FIELD_ENCOUNT
 * @param per エンカウント確率
 * @param attr MAPATTR
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
static BOOL enc_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_WORK* ewk, u32 per )
{
  u32 calc_per,next_per;

  if( per > 100 ){ //100%
    per = 100;
  }
  //移動歩数によるエンカウント率補正
  {
    u32 walk_ct = encwork_GetPlayerWalkCount( ewk );
    if(walk_ct == 0){
      return FALSE;
    }else if(walk_ct == 1){
      per = 2;  //最初の一歩目は強制で2%
    }
  }

  if( enc_GetPercentRand() < per ){
    return( TRUE );
  }
#if 0   //091006時点では歩数によるエンカウント率補正はない
  calc_per = per << CALC_SHIFT;

  if( enc_CheckEncountWalk(enc,calc_per) == FALSE ){
    enc_AddWalkCount( enc ); //歩数カウント

    if( enc_GetPercentRand() >= WALK_NEXT_PERCENT ){ //5%で次の処理へ
      return( FALSE );
    }
  }

  next_per = NEXT_PERCENT; //エンカウント基本確率

  { //日付による確率変動
  }

  if( next_per > 100 ){ //100%超えチェック
    next_per = 100;
  }

  if( enc_GetPercentRand() < next_per ){ //確率チェック
    if( enc_GetPercentRand() >= per ){
      return( TRUE );
    }
  }
#endif
  return( FALSE );
}

#if 0
//--------------------------------------------------------------
/**
 * エンカウントチェック　歩数チェック
 * @param enc FIELD_ENCOUNT
 * @param per エンカウント確率
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per )
{
  per = (per/10) >> CALC_SHIFT;

  if( per > WALK_COUNT_GLOBAL ){
    per = WALK_COUNT_GLOBAL;
  }

  per = WALK_COUNT_GLOBAL - per;

  if( enc_GetWalkCount(enc) >= per ){
    return( TRUE );
  }

  return( FALSE );
}
#endif

//======================================================================
//  バトルパラム
//======================================================================
//--------------------------------------------------------------
/**
 * バトルパラム作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_CreateBattleParam( FIELD_ENCOUNT *enc, const ENCPOKE_FLD_PARAM* efp,
    BATTLE_SETUP_PARAM *bsp, HEAPID heapID, const ENC_POKE_PARAM* inPokeTbl )
{
  GAMEDATA *gdata = enc->gdata;

  //エネミーパーティ生成
  {
    int i;
    POKEPARTY* partyEnemy = PokeParty_AllocPartyWork( heapID );
    POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
    MYSTATUS* myStatus = GAMEDATA_GetMyStatus( gdata );

    for(i = 0;i < ((u8)(efp->enc_double_f)+1);i++){
      ENCPOKE_PPSetup( pp, efp, &inPokeTbl[i] );
      PokeParty_Add( partyEnemy, pp );
    }
    GFL_HEAP_FreeMemory( pp );

    BP_SETUP_Wild( bsp, gdata, heapID, BTL_RULE_SINGLE+efp->enc_double_f,partyEnemy, efp->land_form, efp->weather );
  }

  { //2vs2時の味方AI（不要ならnull）
    bsp->partyPartner = NULL;
  }

  { //2vs2時の２番目敵AI用（不要ならnull）
    bsp->partyEnemy2 = NULL;
  }

  { //BGM設定
    //デフォルト時のBGMナンバー
//  bsp->musicDefault = SEQ_WB_BA_TEST_250KB;
    bsp->musicDefault = SEQ_BGM_VS_NORAPOKE;
    //ピンチ時のBGMナンバー
    bsp->musicPinch = SEQ_BGM_BATTLEPINCH;
  }
}

//--------------------------------------------------------------
/**
 * バトルパラム作成（トレーナー戦）
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_CreateTrainerBattleParam(
    FIELD_ENCOUNT *enc,
    BATTLE_SETUP_PARAM *param, const HEAPID heapID,
    TrainerID tr_id )
{
  GAMESYS_WORK *gsys = enc->gsys;
  GAMEDATA *gdata = enc->gdata;

  BATTLE_PARAM_Init(param);

  BTL_SETUP_Single_Trainer( param, gdata, NULL, BTL_LANDFORM_GRASS, BTL_WEATHER_NONE, tr_id );

  { //対戦相手の手持ちポケモン生成
    param->partyEnemy1 = PokeParty_AllocPartyWork( heapID );
    TT_EncountTrainerDataMake( param, heapID );
  }

  { //BGM設定
    //デフォルト時のBGMナンバー
//  param->musicDefault = SEQ_WB_BA_TEST_250KB;
    param->musicDefault = SEQ_BGM_VS_NORAPOKE;
    //ピンチ時のBGMナンバー
    param->musicPinch = SEQ_BGM_BATTLEPINCH;
  }
}

//--------------------------------------------------------------
/**
 * フィールドエンカウント　トレーナー用BATTLE_SETUP_PARAM*作成
 * @param enc FIELD_ENCOUNT*
 * @param tr_id トレーナーID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_ENCOUNT_SetTrainerBattleParam(
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup, int tr_id, HEAPID heapID )
{
  KAGAYA_Printf( "トレーナーバトルパラム作成 HEAPID=%d\n", heapID );
  enc_CreateTrainerBattleParam( enc, setup, heapID, tr_id );
}

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * ランダム確率取得
 * @retval  u32 確率
 */
//--------------------------------------------------------------
static u32 enc_GetPercentRand( void )
{
  u32 per = (0xffff/100) + 1;
  u32 val = GFUser_GetPublicRand(0xffff) / per;
  return( val );
}

#if 0
//--------------------------------------------------------------
/**
 * 歩数カウント取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
static int enc_GetWalkCount( FIELD_ENCOUNT *enc )
{
  return( GAMEDATA_GetFieldMapWalkCount(enc->gdata) );
}

//--------------------------------------------------------------
/**
 * 歩数カウント増加
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_AddWalkCount( FIELD_ENCOUNT *enc )
{
  int count = GAMEDATA_GetFieldMapWalkCount( enc->gdata );
  if( count < WALK_COUNT_MAX ){ count++; }
  GAMEDATA_SetFieldMapWalkCount( enc->gdata, count );
}

//--------------------------------------------------------------
/**
 * 歩数カウントクリア
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_ClearWalkCount( FIELD_ENCOUNT *enc )
{
  GAMEDATA_SetFieldMapWalkCount( enc->gdata, 0 );
}
#endif

/**
 *  @brief  プレイヤーポジション保存
 */
static void encwork_SetPlayerPos( ENCOUNT_WORK* ewk, const FIELD_PLAYER* player)
{
  MMDL* mmdl = FIELD_PLAYER_GetMMdl( player );

  MI_CpuClear8( &ewk->player, sizeof(EWK_PLAYER) );

  ewk->player.pos_x = MMDL_GetGridPosX( mmdl );
  ewk->player.pos_y = MMDL_GetGridPosY( mmdl );
  ewk->player.pos_z = MMDL_GetGridPosZ( mmdl );
}

/*
 *  @brief  最後のエンカウントからのプレイヤーの歩数を加算
 *  ※最後にエンカウントしたグリッドでの方向転換は歩数に数えない
 */
static void encwork_AddPlayerWalkCount( ENCOUNT_WORK* ewk, const FIELD_PLAYER* player)
{
  MMDL* mmdl = FIELD_PLAYER_GetMMdl( player );

  if(ewk->player.move_f){
    ++ewk->player.walk_ct;
    return;
  }
  if( MMDL_GetGridPosX( mmdl ) == ewk->player.pos_x &&
      MMDL_GetGridPosY( mmdl ) == ewk->player.pos_y &&
      MMDL_GetGridPosZ( mmdl ) == ewk->player.pos_z ){
    return;
  }
  ewk->player.move_f = TRUE;
  ++ewk->player.walk_ct;
}

/*
 *  @brief  プレイヤーが最後のエンカウント後に何歩歩いたか？
 *
 *  ※最後にエンカウントしたグリッドでの方向転換は歩数に数えない
 */
static u32 encwork_GetPlayerWalkCount( ENCOUNT_WORK* ewk )
{
  return ewk->player.walk_ct;
}

/**
 *  @brief  プレイヤーステータスクリア
 */
static void encwork_PlayerStatusClear( ENCOUNT_WORK* ewk )
{
  MI_CpuClear8( &ewk->player, sizeof(EWK_PLAYER) );
}
