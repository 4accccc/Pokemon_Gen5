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
#include "system/rtc_tool.h"

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
#include "field/zonedata.h"

#include "enc_pokeset.h"
#include "event_battle.h"
#include "field_encount_local.h"

#include "script_def.h"
#include "debug/debug_flg.h"
#include "sound/wb_sound_data.sadl"

#include "enceffno.h"

//======================================================================
//  define
//======================================================================
#define HEAPID_BTLPARAM (HEAPID_PROC) ///<バトルパラメタ用HEAPID

//--------------------------------------------------------------
/// エンカウントデータ要素
//--------------------------------------------------------------
//======================================================================
//  proto
//======================================================================
static ENCOUNT_LOCATION enc_GetLocation( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type, u8* prob_rev );
static u32 enc_GetLocationPercent( FIELD_ENCOUNT *enc,ENCOUNT_LOCATION location, u8 prob_rev );
static BOOL enc_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_WORK* ewk, const u32 per );
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per );

static void enc_CreateBattleParam( FIELD_ENCOUNT *enc, const ENCPOKE_FLD_PARAM* spa,
    BATTLE_SETUP_PARAM *bsp, HEAPID heapID, const ENC_POKE_PARAM* inPokeTbl );
static void enc_CreateBattleParamMovePoke( FIELD_ENCOUNT *enc, const ENCPOKE_FLD_PARAM* efp,
    BATTLE_SETUP_PARAM *bsp, HEAPID heapID, MPD_PTR mpd );

static void BTL_FIELD_SITUATION_SetFromFieldStatus( BTL_FIELD_SITUATION* sit, GAMEDATA* gdata, FIELDMAP_WORK* fieldWork );

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
 *
 * フィールドエフェクトに依存するので、フィールドエフェクトコントローラ生成後に呼び出すこと
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

  enc->eff_enc = EFFECT_ENC_CreateWork( heapID );
  EFFECT_ENC_Init( enc, enc->eff_enc );

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
  EFFECT_ENC_End( enc, enc->eff_enc );
  EFFECT_ENC_DeleteWork(enc->eff_enc);

  GFL_HEAP_FreeMemory( enc );
}

//======================================================================
//  フィールドエンカウント　チェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック(通常用)
 * @param enc FIELD_ENCOUNT
 * @param enc_mode  ENCOUNT_MODE_???
 * @retval  NULL  エンカウントなし
 * @retval  GMEVENT*  エンカウント成功
 */
//--------------------------------------------------------------
void* FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type )
{
  int enc_eff_no;
  u32 per,enc_num;
  BOOL ret = FALSE,force_f = FALSE;
  BATTLE_SETUP_PARAM* bp;
  ENCOUNT_WORK* ewk;
  ENCOUNT_LOCATION enc_loc;
  ENC_POKE_PARAM poke_tbl[FLD_ENCPOKE_NUM_MAX];
  ENCPOKE_FLD_PARAM fld_spa;
  MPD_PTR mpd = NULL;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );

  ewk = GAMEDATA_GetEncountWork(enc->gdata);
  if( enc_type == ENC_TYPE_FORCE || enc_type == ENC_TYPE_EFFECT ){
    force_f = TRUE;
  }else{
    //最後のエンカウントからのプレイヤーの歩数を加算
    encwork_AddPlayerWalkCount( ewk, fplayer);
  }

#ifdef PM_DEBUG
  //デバッグ強制エンカウントOffルーチン
  if( enc_type != ENC_TYPE_FORCE && DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEncount) ){
    return NULL;
  }
#endif

  //ロケーションチェック
  {
    u8 prob_rev = 0;
    enc_loc = enc_GetLocation( enc, enc_type, &prob_rev );
    per = enc_GetLocationPercent( enc, enc_loc, prob_rev );
  }
  if( per <= 0 ){
    return( NULL ); //確率0
  }

  //ENCPOKE_FLD_PARAM作成
  ENCPOKE_SetEFPStruct( &fld_spa, enc->gdata, enc_loc, enc_type,
      FIELD_WEATHER_GetWeatherNo(FIELDMAP_GetFieldWeather( enc->fwork )) );

  if( !force_f )
  {
    //道具＆特性によるエンカウント率変動
    per = ENCPOKE_EncProbManipulation( &fld_spa, enc->gdata, per);

    if( enc_CheckEncount(enc,ewk,per) == FALSE ){ //エンカウントチェック
      return NULL;
    }
  }

  if( enc_type != ENC_TYPE_EFFECT ){ //移動ポケモンチェック
    mpd = ENCPOKE_GetMovePokeEncountData( enc->encdata,
              &fld_spa, FIELDMAP_GetZoneID( enc->fwork ) ); 
  }
  if( mpd != NULL){
    //バトルパラメータセット
    bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
    enc_CreateBattleParamMovePoke( enc, &fld_spa, bp, HEAPID_BTLPARAM, mpd );

  }else{
    //エンカウントデータ生成
    MI_CpuClear8(poke_tbl,sizeof(ENC_POKE_PARAM)*FLD_ENCPOKE_NUM_MAX);
    enc_num = ENCPOKE_GetNormalEncountPokeData( enc->encdata,
              &fld_spa, FIELDMAP_GetZoneID( enc->fwork ),poke_tbl );

    if( enc_num == 0 ){ //エンカウント失敗
      return NULL;
    }

    //バトルパラメータセット
    bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
    enc_CreateBattleParam( enc, &fld_spa, bp, HEAPID_BTLPARAM, poke_tbl );
  }

  // プレイヤー座標更新＆歩数カウントクリア
  encwork_SetPlayerPos( ewk, fplayer);

  //エンカウントエフェクト抽選
  enc_eff_no = ENCEFFNO_GetWildEncEffNo( poke_tbl[0].monsNo, enc->fwork );

  //エンカウントイベント生成
  return (void*)EVENT_WildPokeBattle( enc->gsys, enc->fwork, bp, FALSE, enc_eff_no );
}

//--------------------------------------------------------------
/**
 * スクリプト　野生イベント戦闘エンカウント
 * @param enc FIELD_ENCOUNT
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
void* FIELD_ENCOUNT_SetWildEncount( FIELD_ENCOUNT *enc, u16 mons_no, u8 mons_lv, u16 flags )
{
  int enc_eff_no;
  BATTLE_SETUP_PARAM* bp;
  ENCOUNT_WORK* ewk;
  ENC_POKE_PARAM poke_tbl[FLD_ENCPOKE_NUM_MAX];

  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  ENCPOKE_FLD_PARAM fld_spa;

  //ENCPOKE_FLD_PARAM作成
  ENCPOKE_SetEFPStruct( &fld_spa, enc->gdata, ENC_LOCATION_NONE, ENC_TYPE_NORMAL,
      FIELD_WEATHER_GetWeatherNo(FIELDMAP_GetFieldWeather( enc->fwork )) );

  //エンカウントポケモンデータ生成
  MI_CpuClear8(poke_tbl,sizeof(ENC_POKE_PARAM)*FLD_ENCPOKE_NUM_MAX);
  {
    ENC_POKE_PARAM* poke = &poke_tbl[0];
    poke->monsNo = mons_no;
    poke->level = mons_lv;

    if( flags & SCR_WILD_BTL_FLAG_RARE ){
      poke->rare = TRUE;
    }
  }

  //バトルパラメータセット
  bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
  enc_CreateBattleParam( enc, &fld_spa, bp, HEAPID_BTLPARAM, poke_tbl );

  //エンカウントエフェクト抽選
  enc_eff_no = ENCEFFNO_GetWildEncEffNo( poke_tbl[0].monsNo, enc->fwork );

  //エンカウントイベント生成
  return (void*)EVENT_WildPokeBattle( enc->gsys, enc->fwork, bp, TRUE, enc_eff_no );
}

//--------------------------------------------------------------
/**
 * エンカウントチェック(釣りエンカウント)
 * @param enc FIELD_ENCOUNT
 * @param enc_mode  ENCOUNT_MODE_???
 * @retval  NULL  エンカウントなし
 * @retval  BATTLE_SETUP_PARAM*  エンカウント成功
 */
//--------------------------------------------------------------
void* FIELD_ENCOUNT_CheckFishingEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type )
{
  u32 per,enc_num;
  BATTLE_SETUP_PARAM* bp;
  ENCOUNT_WORK* ewk;
  ENCOUNT_LOCATION enc_loc;
  ENC_POKE_PARAM poke_tbl[FLD_ENCPOKE_NUM_MAX];

  ENCPOKE_FLD_PARAM fld_spa;

  //ロケーションチェック
  if( enc_type == ENC_TYPE_EFFECT ){
    enc_loc = ENC_LOCATION_FISHING_SP;
  }else{
    enc_loc = ENC_LOCATION_FISHING;
  }
  per = enc_GetLocationPercent( enc, enc_loc, 0 );
  if( per <= 0 ){
    return( NULL ); //確率0
  }

  //ENCPOKE_FLD_PARAM作成
  ENCPOKE_SetEFPStruct( &fld_spa, enc->gdata, enc_loc, enc_type,
      FIELD_WEATHER_GetWeatherNo(FIELDMAP_GetFieldWeather( enc->fwork )) );

  //道具＆特性によるエンカウント率変動(@todo 釣りに影響するの？)
  if( enc_type != ENC_TYPE_EFFECT ){
    per = ENCPOKE_EncProbManipulation( &fld_spa, enc->gdata, per);
    if( enc_GetPercentRand() < per ){
      return( NULL );
    }
  }

  { //移動ポケモンチェック( @todo 移動ポケモンは釣りでエンカウントするの？)

  }

  //エンカウントデータ生成
  MI_CpuClear8(poke_tbl,sizeof(ENC_POKE_PARAM)*FLD_ENCPOKE_NUM_MAX);
  enc_num = ENCPOKE_GetNormalEncountPokeData( enc->encdata, &fld_spa, FIELDMAP_GetZoneID( enc->fwork), poke_tbl );

  if( enc_num == 0 ){ //エンカウント失敗
    return NULL;
  }

  //バトルパラメータセット
  bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
  enc_CreateBattleParam( enc, &fld_spa, bp, HEAPID_BTLPARAM, poke_tbl );

  return bp;
}


//----------------------------------------------------------------------------
/**
 * WFBCエンカウントチェック
 * @param enc           FIELD_ENCOUNT
 * @param cp_wfbcedata  wfbc情報
 * @retval  NULL  エンカウントなし
 * @retval  GMEVENT*  エンカウント成功
 */
//-----------------------------------------------------------------------------
void* FIELD_ENCOUNT_CheckWfbcEncount( FIELD_ENCOUNT *enc, const FIELD_WFBC* cp_wfbcdata )
{
  int enc_eff_no;
  u32 per,enc_num;
  BOOL ret = FALSE,force_f = FALSE;
  BATTLE_SETUP_PARAM* bp;
  ENCOUNT_WORK* ewk;
  ENCOUNT_LOCATION enc_loc;
  ENC_POKE_PARAM poke_tbl[FLD_ENCPOKE_NUM_MAX];
  ENCPOKE_FLD_PARAM fld_spa;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );



  ewk = GAMEDATA_GetEncountWork(enc->gdata);

  //最後のエンカウントからのプレイヤーの歩数を加算
  encwork_AddPlayerWalkCount( ewk, fplayer);

#ifdef PM_DEBUG
  //デバッグ強制エンカウントOffルーチン
  if( !force_f && DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEncount) ){
    return NULL;
  }
#endif

  //ロケーションチェック
  {
    u8 prob_rev = 0;
    enc_loc = enc_GetLocation( enc, ENC_TYPE_WFBC, &prob_rev );
  }

  //ENCPOKE_FLD_PARAM作成
  ENCPOKE_SetEFPStruct( &fld_spa, enc->gdata, enc_loc, ENC_TYPE_WFBC,
      FIELD_WEATHER_GetWeatherNo(FIELDMAP_GetFieldWeather( enc->fwork )) );

#if 0
  if( !force_f )
  {
    //道具＆特性によるエンカウント率変動
    per = ENCPOKE_EncProbManipulation( &fld_spa, enc->gdata, per);

    if( enc_CheckEncount(enc,ewk,per) == FALSE ){ //エンカウントチェック
      return NULL;
    }
  }
#endif

  { //移動ポケモンチェック

  }

  //エンカウントデータ生成
  MI_CpuClear8(poke_tbl,sizeof(ENC_POKE_PARAM)*FLD_ENCPOKE_NUM_MAX);
  enc_num = ENCPOKE_GetWFBCEncountPoke( cp_wfbcdata, &fld_spa, poke_tbl );

  if( enc_num == 0 ){ //エンカウント失敗
    return NULL;
  }

  //バトルパラメータセット
  bp = BATTLE_PARAM_Create( HEAPID_BTLPARAM );
  enc_CreateBattleParam( enc, &fld_spa, bp, HEAPID_BTLPARAM, poke_tbl );

  // プレイヤー座標更新＆歩数カウントクリア
  encwork_SetPlayerPos( ewk, fplayer);

  //エンカウントエフェクト抽選
  enc_eff_no = ENCEFFNO_GetWildEncEffNo( poke_tbl[0].monsNo, enc->fwork );

  //エンカウントイベント生成
  return (void*)EVENT_WildPokeBattle( enc->gsys, enc->fwork, bp, FALSE, enc_eff_no );
}

//======================================================================
//  サブ　エンカウント確率
//======================================================================

//--------------------------------------------------------------
/**
 * アトリビュートとエンカウントタイプからエンカウントローケーション取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
static ENCOUNT_LOCATION enc_GetLocation( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type ,u8* prob_rev)
{
  MAPATTR attr;
  VecFx32 pos;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( enc->fwork );
  MAPATTR_FLAG attr_flag;

  *prob_rev = 0;
  attr = FIELD_PLAYER_GetMapAttr( fplayer );
  if(MAPATTR_IsEnable(attr) == FALSE){
    return ENC_LOCATION_NONE;
  }

  //エンカウントフラグチェック
  attr_flag = MAPATTR_GetAttrFlag(attr);

  //エンカウントチェック
  if( enc_type != ENC_TYPE_EFFECT && (attr_flag & MAPATTR_FLAGBIT_ENCOUNT) == FALSE ){
    return ENC_LOCATION_NONE;
  }

  //エンカウントアトリビュートチェック
  {
    MAPATTR_VALUE attr_value = MAPATTR_GetAttrValue(attr);

    //アトリビュートによるエンカウント率補正取得
    if( MAPATTR_VALUE_CheckLongGrass( attr_value )){
      *prob_rev += 10;
    }
    //水チェック
    if(attr_flag & MAPATTR_FLAGBIT_WATER){
      if( enc_type == ENC_TYPE_EFFECT ) return ENC_LOCATION_WATER_SP;
      return ENC_LOCATION_WATER;
    }
    if( enc_type == ENC_TYPE_EFFECT ) return ENC_LOCATION_GROUND_SP;

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
 * @param location ENCOUNT_LOCATION
 * @param prov_rev アトリビュートによるエンカウント率補正値
 * @retval u32 エンカウント率 0=エンカウント無し
 */
//--------------------------------------------------------------
static u32 enc_GetLocationPercent( FIELD_ENCOUNT *enc,ENCOUNT_LOCATION location, u8 prob_rev )
{
  s32 per = 0;

  if(location >= ENC_LOCATION_MAX){
    return 0;
  }
  per = enc->encdata->prob[location];

  if( per > 0){ //元の確率0は0のまま
    per += prob_rev;
  }
  if( per < 0){
    return 0;
  }
  return per;
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
  return( FALSE );
}

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
  //エネミーパーティ生成
  {
    int i;
    BTL_FIELD_SITUATION sit;
    POKEPARTY* partyEnemy = PokeParty_AllocPartyWork( heapID );
    POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );

    for(i = 0;i < ((u8)(efp->enc_double_f)+1);i++){
      ENCPOKE_PPSetup( pp, efp, &inPokeTbl[i] );
      PokeParty_Add( partyEnemy, pp );
    }

    BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, enc->gdata, enc->fwork );
    BTL_SETUP_Wild( bsp, enc->gdata, partyEnemy, &sit, BTL_RULE_SINGLE+efp->enc_double_f, heapID );

    GFL_HEAP_FreeMemory( pp );
    GFL_HEAP_FreeMemory( partyEnemy );
  }
}

//--------------------------------------------------------------
/**
 * バトルパラム作成(移動ポケモン用)
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_CreateBattleParamMovePoke( FIELD_ENCOUNT *enc, const ENCPOKE_FLD_PARAM* efp,
    BATTLE_SETUP_PARAM *bsp, HEAPID heapID, MPD_PTR mpd )
{
  //エネミーパーティ生成
  BTL_FIELD_SITUATION sit;
  POKEPARTY* partyEnemy = PokeParty_AllocPartyWork( heapID );
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemoryLo( heapID, POKETOOL_GetWorkSize() );

  ENCPOKE_PPSetupMovePoke( pp, efp, mpd );
  PokeParty_Add( partyEnemy, pp );

  BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, enc->gdata, enc->fwork );
  BTL_SETUP_Wild( bsp, enc->gdata, partyEnemy, &sit, BTL_RULE_SINGLE, heapID );

  GFL_HEAP_FreeMemory( pp );
  GFL_HEAP_FreeMemory( partyEnemy );
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
    BATTLE_SETUP_PARAM *bp, const HEAPID heapID,
    TrainerID partner_id, TrainerID tr_id0, TrainerID tr_id1 )
{
  GAMESYS_WORK *gsys = enc->gsys;
  GAMEDATA *gdata = enc->gdata;
  BTL_FIELD_SITUATION sit;

  BATTLE_PARAM_Init(bp);

  BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, enc->gdata, enc->fwork );

  if( partner_id != TRID_NULL )
  { //AIマルチ
    BTL_SETUP_AIMulti_Trainer( bp, gdata, &sit, partner_id, tr_id0, tr_id1, heapID );
  }else if( (tr_id1 != TRID_NULL) && tr_id0 != tr_id1 )
  { //タッグ
    BTL_SETUP_Tag_Trainer( bp, gdata, &sit, tr_id0, tr_id1, heapID );
  }else if( tr_id0 == tr_id1 )
  { //ダブル
    BTL_SETUP_Double_Trainer( bp, gdata, &sit, tr_id0, heapID );
  }else
  { //シングル
    BTL_SETUP_Single_Trainer( bp, gdata, &sit, tr_id0, heapID );
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
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup, int partner_id, int tr_id0, int tr_id1, HEAPID heapID )
{
  KAGAYA_Printf( "トレーナーバトルパラム作成 HEAPID=%d\n", heapID );
  enc_CreateTrainerBattleParam( enc, setup, heapID, partner_id, tr_id0, tr_id1 );
}

static BtlWeather btlparam_GetBattleWeather( FIELDMAP_WORK* fieldWork )
{
  FIELD_WEATHER* fld_weather = FIELDMAP_GetFieldWeather( fieldWork );
  u32 weather = FIELD_WEATHER_GetWeatherNo( fld_weather );

  switch( weather )
  {
  case WEATHER_NO_SUNNY:
    return BTL_WEATHER_SHINE;
  case WEATHER_NO_RAIN:
  case WEATHER_NO_STORM:
  case WEATHER_NO_SPARK:
    return BTL_WEATHER_RAIN;
  case WEATHER_NO_SNOW:
  case WEATHER_NO_SNOWSTORM:
  case WEATHER_NO_ARARE:
    return BTL_WEATHER_SNOW;
  }
  return BTL_WEATHER_NONE;
}

/*
 *  @brief  戦闘背景/天候 など FIELDMAP_WORKを参照して決定されるシチュエーションデータを取得する
 *  @todo 仮処理
 */
static void BTL_FIELD_SITUATION_SetFromFieldStatus( BTL_FIELD_SITUATION* sit, GAMEDATA* gdata, FIELDMAP_WORK* fieldWork )
{
  u16 zone_id = FIELDMAP_GetZoneID( fieldWork );
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( fieldWork );

  //戦闘背景
  sit->bgType = ZONEDATA_GetBattleBGType(zone_id);
  {
    MAPATTR attr = FIELD_PLAYER_GetMapAttr( fplayer );
    sit->bgAttr = FIELD_BATTLE_GetBattleAttrID(MAPATTR_GetAttrValue(attr));
  }
  //タイムゾーン取得
  sit->timeZone = GFL_RTC_GetTimeZone();  //@todo EVTIMEからの取得に変更予定

  //天候
  sit->weather = btlparam_GetBattleWeather( fieldWork );

  sit->season = GAMEDATA_GetSeasonID( gdata );
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
