//======================================================================
/**
 * @file  field_encount.c
 * @brief	フィールド　エンカウント処理
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fieldmap.h"
#include "map_attr.h"

#include "field_encount.h"

#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "tr_tool/tr_tool.h"

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
#define	WALK_NEXT_PERCENT	(5) ///<歩数カウント失敗で次の処理に進む確率

#define LONG_GRASS_PERCENT (30) ///<長い草の中にいるときの加算確率
#define CYCLE_PERCENT (30) ///<自転車に乗っているときの加算確率

#define HEAPID_BTLPARAM (HEAPID_PROC) ///<バトルパラメタ用HEAPID

//--------------------------------------------------------------
/// エンカウントデータ要素
//--------------------------------------------------------------
#if 0
#define ENC_MONS_NUM_NORMAL (12)
#else //7月ROM、新ポケを一通り見せる為、一時的に増やす
#define ENC_MONS_NUM_NORMAL (24)
#endif
#define ENC_MONS_NUM_GENERATE (2)
#define ENC_MONS_NUM_NOON (2)
#define ENC_MONS_NUM_NIGHT (2)
#define ENC_MONS_NUM_SWAY_GRASS (4)
#define ENC_FORM_PROB_NUM (5)
#define ENC_MONS_NUM_AGB (2)
#define ENC_MONS_NUM_SEA (5)
#define ENC_MONS_NUM_ROCK (5)
#define ENC_MONS_NUM_FISH (5)

#define ENC_MONS_NUM_MAX ENC_MONS_NUM_NORMAL ///<エンカウントポケモン最大

#ifdef DEBUG_ONLY_FOR_kagaya
//#define DEBUG_ENCOUNT_CHECKOFF_ATTR
#endif

//--------------------------------------------------------------
//  ENCOUNT_LOCATION
//--------------------------------------------------------------
typedef enum
{
  ENCOUNT_LOCATION_GROUND = 0,
  ENCOUNT_LOCATION_WATER,
  ENCOUNT_LOCATION_FISHING,
}ENCOUNT_LOCATION;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_ENCOUNT
//--------------------------------------------------------------
struct _TAG_FIELD_ENCOUNT
{
  FIELDMAP_WORK *fwork;
  GAMESYS_WORK *gsys;
  GAMEDATA *gdata;
  BATTLE_SETUP_PARAM btl_setup_param;
};

//--------------------------------------------------------------
/// ENC_COMMON_DATA
//--------------------------------------------------------------
typedef struct _TAG_ENC_COMMON_DATA
{
	int MonsNo;
	u16 LvMax;
	u16 LvMin;
}ENC_COMMON_DATA;

//--------------------------------------------------------------
/// GROUND_ENC_MONSTER_DAT
//--------------------------------------------------------------
typedef struct _TAG_ENC_MONSTER_DAT
{
	char Level;
	int MonsterNo;
}GROUND_ENC_MONSTER_DAT;

//--------------------------------------------------------------
/// NON_GROUND_ENC_MONSTER_DAT
//--------------------------------------------------------------
typedef struct _TAG_NON_GROUND_ENC_MONSTER_DAT
{
	char MaxLevel;
	char MinLevel;
	int MonsterNo;
}NON_GROUND_ENC_MONSTER_DAT;

//--------------------------------------------------------------
/// ENCOUNT_DATA
//--------------------------------------------------------------
typedef struct _TAG_ENCOUNT_DATA
{
  int wb_normal_encount_max; //kari
	int EncProbGround;
	GROUND_ENC_MONSTER_DAT NormalEnc[ENC_MONS_NUM_NORMAL];
	int GenerateEnc[ENC_MONS_NUM_GENERATE]; //大量発生
	int NoonEnc[ENC_MONS_NUM_NOON]; //昼エンカウント
	int NightEnc[ENC_MONS_NUM_NIGHT]; //夜エンカウント
	int SwayEnc[ENC_MONS_NUM_SWAY_GRASS]; //揺れ草エンカウント
  
  //インデックス　0:シーウシ　1：シードルゴ　2～4：未使用
	int FormProb[ENC_FORM_PROB_NUM];
	
  //アンノーン出現テーブル（0：アンノーン戦闘無し　1～8：テーブル番号）
  int AnnoonTable;
  
	int RubyEnc[ENC_MONS_NUM_AGB];
	int SapphireEnc[ENC_MONS_NUM_AGB];
	int EmeraldEnc[ENC_MONS_NUM_AGB];
	int FireEnc[ENC_MONS_NUM_AGB];
	int LeafEnc[ENC_MONS_NUM_AGB];
	int EncProbSea;
	NON_GROUND_ENC_MONSTER_DAT	EncSea[ENC_MONS_NUM_SEA];
	int EncProbRock;
	NON_GROUND_ENC_MONSTER_DAT	EncRock[ENC_MONS_NUM_ROCK];
	int EncProbFish1;
	NON_GROUND_ENC_MONSTER_DAT EncFish1[ENC_MONS_NUM_FISH];
	int EncProbFish2;
	NON_GROUND_ENC_MONSTER_DAT EncFish2[ENC_MONS_NUM_FISH];
	int EncProbFish3;
	NON_GROUND_ENC_MONSTER_DAT EncFish3[ENC_MONS_NUM_FISH];
}ENCOUNT_DATA;

//--------------------------------------------------------------
/// ENC_FLD_SPA
//--------------------------------------------------------------
typedef struct _TAG_ENC_FLD_SPA
{
	u32 TrainerID; //トレーナーＩＤ
	BOOL SprayCheck; //スプレーチェックするかのフラグ	TRUE:チェックする
	BOOL EncCancelSpInvalid;//戦闘回避特性無効	TRUE:無効	FALSE:有効		(現状では、あまいかおり・あまいミツ用)
	u8 SpMyLv; //スプレーチェックに使うレベル
	u8 Egg; //タマゴフラグ
	u8 Spa; //特性
	u8 FormProb[2]; //シーウシ・シードルゴ用フォルム変更確率　0：シーウシ　1：シードルゴ
	u8 AnnoonTblType; //アンノーンテーブルタイプ
}ENC_FLD_SPA;

//======================================================================
//  proto
//======================================================================
static u32 enc_GetAttrPercent( FIELD_ENCOUNT *enc,
    MAPATTR attr, ENCOUNT_LOCATION *outEncLocation );

static BOOL enc_CheckEncount(
    FIELD_ENCOUNT *enc, const u32 per, const MAPATTR attr );
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per );

static void enc_AddPartyPokemon(
    POKEPARTY *party, u32 monsno, u32 level, u32 id, HEAPID heapID );
static const void enc_FreeBattleSetupParam( BATTLE_SETUP_PARAM *param );

static BOOL enc_SetEncountData(
    FIELD_ENCOUNT *enc,
    BATTLE_SETUP_PARAM *param,
    const ENC_FLD_SPA *inFldSpa,
    const ENC_COMMON_DATA *enc_data, u32 location, HEAPID heapID,
    int tbl_max );

static u32 enc_GetPercentRand( void );
static const ENCOUNT_DATA * enc_GetEncountData(
    FIELD_ENCOUNT *enc, MAPATTR attr );

static void enc_SetSpaStruct(
    FIELD_ENCOUNT *enc,
    const POKEPARTY *party,
    const ENCOUNT_DATA* inData, ENC_FLD_SPA *outSpa );

static int enc_GetWalkCount( FIELD_ENCOUNT *enc );
static void enc_AddWalkCount( FIELD_ENCOUNT *enc );
static void enc_ClearWalkCount( FIELD_ENCOUNT *enc );

static const ENCOUNT_DATA dataTestEncountData;
static const ENCOUNT_DATA dataTestEncountData01;

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

//--------------------------------------------------------------
/**
 * フィールドエンカウント　BATTLE_SETUP_PARAM取得
 * @param enc FIELD_ENCOUNT*
 * @param setup コピー先BATTLE_SETUP_PARAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_ENCOUNT_GetBattleSetupParam(
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup )
{
  *setup = enc->btl_setup_param;
}

//======================================================================
//  フィールドエンカウント　チェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック
 * @param enc FIELD_ENCOUNT
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
BOOL FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc )
{
  u32 per;
  BOOL ret = FALSE;
  BOOL companion;
  int gx,gz;
  VecFx32 pos;
  
  MAPATTR attr;
  
  ENCOUNT_LOCATION enc_loc;
  
  const ENCOUNT_DATA *data;

	ENC_COMMON_DATA enc_data[ENC_MONS_NUM_MAX];
  
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( enc->fwork );
  
  BATTLE_SETUP_PARAM *setup;
	ENC_FLD_SPA fld_spa;
  
  FIELD_PLAYER_GetPos( fplayer, &pos );
  attr = MAPATTR_GetAttribute( mapper, &pos );
  
  if( attr == MAPATTR_ERROR ){
    return( FALSE );
  }
  
  data = enc_GetEncountData( enc, attr );

  //090713 ROM用 水アトリビュートエンカウント無効
  #if 1 
  {
    MAPATTR_FLAG attr_flag;
    attr_flag = MAPATTR_GetAttrFlag( attr );
    if( (attr_flag & MAPATTR_FLAGBIT_WATER) ){ //水
      return( FALSE );
    }
  }
  #endif
  
  //エンカウント確率取得
  per = enc_GetAttrPercent( enc, attr, &enc_loc );
  
  if( per == 0 ){
    return( FALSE ); //確率0
  }
  
  { //手持ちポケモン取得
  }
  
  { //ENC_FLD_SPA作成
    enc_SetSpaStruct(
        enc, GAMEDATA_GetMyPokemon(enc->gdata), data, &fld_spa );
  }

  { //スプレーチェック
  }
  
  { //特性による確率変更
  }
  
  { //ビードロによる確率変更
  }
  
  { //装備アイテム（きよめのおふだ）による確率変動
  }
  
  if( enc_CheckEncount(enc,per,attr) == TRUE ){ //エンカウントチェック
    ret = TRUE;
  }
  
  { //揺れ草エンカウントチェック
  }
  
  if( ret == FALSE ){
    return( FALSE );
  }
  
  { //連れ歩き判定
    companion = FALSE;
  }

  { //移動ポケモンチェック
  }
  
  { //バトルパラメタの作成
  }
  
  { //手持ちポケモンなどセット
  }
  
  ret = FALSE;
  setup = &enc->btl_setup_param;
  
  if( enc_loc == ENCOUNT_LOCATION_GROUND ) //陸
  { //陸エンカウントデータ作成
    int i; 
    BOOL book_get;
    
		for( i = 0; i < data->wb_normal_encount_max; i++ ){
			enc_data[i].MonsNo = data->NormalEnc[i].MonsterNo;
			enc_data[i].LvMax = data->NormalEnc[i].Level;
			enc_data[i].LvMin = data->NormalEnc[i].Level;
    }
    
    //全国図鑑フラグ
    book_get = FALSE;
     
    //昼夜テーブル書き換え
    //大量発生テーブル書き換え
		//裏山テーブル書き換え
		//AGBスロットテーブル書き換え
    
    if( companion != FALSE ){ //連れ歩き
      //まだ
    }else{ //通常対戦
      ret = enc_SetEncountData(
          enc, setup, &fld_spa, enc_data, enc_loc, HEAPID_BTLPARAM,
          data->wb_normal_encount_max );
    }
  }
  else if( enc_loc == ENCOUNT_LOCATION_WATER ) //水
  { //水エンカウントデータ作成
    int i;
    for( i = 0; i < ENC_MONS_NUM_SEA; i++ ){
			enc_data[i].MonsNo = data->EncSea[i].MonsterNo;
			enc_data[i].LvMax = data->EncSea[i].MaxLevel;
			enc_data[i].LvMin = data->EncSea[i].MinLevel;
    }
     
    ret = enc_SetEncountData(
          enc, setup, &fld_spa, enc_data, enc_loc, HEAPID_BTLPARAM,
          ENC_MONS_NUM_SEA );
  }
  else //エラー
  {
    GF_ASSERT( 0 && "ENCOUNT LOCATION ERROR" );
    return( FALSE );
  }
  
  if( ret == FALSE ){ //エンカウント無し
    enc_FreeBattleSetupParam( setup );
  }
  
  // 歩数カウントクリア
  enc_ClearWalkCount( enc );
  
  return( ret );
}

//======================================================================
//  サブ　エンカウント確率
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウント率をアトリビュートから取得
 * @param enc FIELD_ENCOUNT*
 * @param attr チェックするMAPATTR
 * @param outEncLocation にエンカウント場所を格納するENCOUNT_LOCATION
 * @retval u32 エンカウント率 0=エンカウント無し
 */
//--------------------------------------------------------------
static u32 enc_GetAttrPercent( FIELD_ENCOUNT *enc,
    MAPATTR attr, ENCOUNT_LOCATION *outEncLocation )
{
  u32 per = 0;
  MAPATTR_FLAG attr_flag;
  
  attr_flag = MAPATTR_GetAttrFlag( attr );
  
#ifdef DEBUG_ENCOUNT_CHECKOFF_ATTR
  attr_flag |= MAPATTR_FLAGBIT_ENCOUNT;
#endif
  
  if( (attr_flag & MAPATTR_FLAGBIT_ENCOUNT) ){ //エンカウントフラグ
    const ENCOUNT_DATA *data = enc_GetEncountData( enc, attr );

#ifdef DEBUG_WB_FORCE_GROUND    
    attr_flag = 0; //wb 仮 地上で固定
#endif

    if( (attr_flag & MAPATTR_FLAGBIT_WATER) ){ //水
      *outEncLocation = ENCOUNT_LOCATION_WATER;
      per = data->EncProbSea;
    }else{ //陸
      *outEncLocation = ENCOUNT_LOCATION_GROUND;
      per = data->EncProbGround;
    }
  }
   
  return( per );
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
static BOOL enc_CheckEncount(
    FIELD_ENCOUNT *enc, u32 per, const MAPATTR attr )
{
  u32 calc_per,next_per;
  
  if( per > 100 ){ //100%
    per = 100;
  }
  
  calc_per = per << CALC_SHIFT;
  
  if( enc_CheckEncountWalk(enc,calc_per) == FALSE ){
    enc_AddWalkCount( enc ); //歩数カウント
    
    if( enc_GetPercentRand() >= WALK_NEXT_PERCENT ){ //5%で次の処理へ
      return( FALSE );
    }
  }
  
  next_per = NEXT_PERCENT; //エンカウント基本確率
  
  { //確率変動
    MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
    
    if( MAPATTR_VALUE_CheckLongGrass(val) == TRUE ){
      next_per += LONG_GRASS_PERCENT; //長い草の場合は+30%
    }else{
      #ifndef FENCOUNT_PL_NULL
		  if ( Player_FormGet(fsys->player) == HERO_FORM_CYCLE ){
			    next_per += CYCLE_PERCENT; //自転車の場合は+30%
      }
      #endif
    }
  }
    
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
  
  return( FALSE );
}

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
static void enc_CreateBattleParam(
    FIELD_ENCOUNT *enc,
    BATTLE_SETUP_PARAM *param, HEAPID heapID,
    u32 monsNo, u32 lv, u32 id )
{
  GAMESYS_WORK *gsys = enc->gsys;
  GAMEDATA *gdata = enc->gdata;
  
  { //各設定
    param->engine = BTL_ENGINE_ALONE;
    param->rule = BTL_RULE_SINGLE;
    param->competitor = BTL_COMPETITOR_WILD;
    param->netHandle = NULL;
    param->commMode = BTL_COMM_NONE;
    param->netID = 0;
  }

  { //プレイヤーパーティ追加
    param->partyPlayer = PokeParty_AllocPartyWork( heapID );
    PokeParty_Copy( GAMEDATA_GetMyPokemon(gdata), param->partyPlayer );
  }
  
  { //一体目のポケモン追加
    param->partyEnemy1 = PokeParty_AllocPartyWork( heapID );
    
    //MonsNo 0 チェック
    if( monsNo == 0 ){
#ifdef DEBUG_ONLY_FOR_kagaya
      GF_ASSERT( 0 );
#endif
      monsNo = 513;
    }
    
    enc_AddPartyPokemon( param->partyEnemy1, monsNo, lv, id, heapID );
  }
  
  { //2vs2時の味方AI（不要ならnull）
    param->partyPartner = NULL;
  }

  { //2vs2時の２番目敵AI用（不要ならnull）
    param->partyEnemy2 = NULL;
  }
  
  { //プレイヤーステータス
    param->statusPlayer = SaveData_GetMyStatus( SaveControl_GetPointer() );
  }
  
  { //BGM設定
    //デフォルト時のBGMナンバー
//  param->musicDefault = SEQ_WB_BA_TEST_250KB;
    param->musicDefault = SEQ_VS_NORAPOKE;
    //ピンチ時のBGMナンバー
    param->musicPinch = SEQ_WB_BA_PINCH_TEST_150KB;
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
  
  { //各設定
    param->engine = BTL_ENGINE_ALONE;
    param->rule = BTL_RULE_SINGLE;
    param->competitor = BTL_COMPETITOR_TRAINER;
    param->netHandle = NULL;
    param->commMode = BTL_COMM_NONE;
    param->netID = 0;
  }

  { //プレイヤーパーティ追加
    KAGAYA_Printf( "バトルパラム作成 HEAPID=%d\n", heapID );
    param->partyPlayer = PokeParty_AllocPartyWork( heapID );
    PokeParty_Copy( GAMEDATA_GetMyPokemon(gdata), param->partyPlayer );
  }
  
  { //対戦相手の手持ちポケモン生成
    param->partyEnemy1 = PokeParty_AllocPartyWork( heapID );

    param->trID = tr_id;
    
    TT_EncountTrainerDataMake( param, heapID );
  }
  
  { //2vs2時の味方AI（不要ならnull）
    param->partyPartner = NULL;
  }

  { //2vs2時の２番目敵AI用（不要ならnull）
    param->partyEnemy2 = NULL;
  }
  
  { //プレイヤーステータス
    param->statusPlayer = SaveData_GetMyStatus( SaveControl_GetPointer() );
  }
  
  { //BGM設定
    //デフォルト時のBGMナンバー
//  param->musicDefault = SEQ_WB_BA_TEST_250KB;
    param->musicDefault = SEQ_VS_NORAPOKE;
    //ピンチ時のBGMナンバー
    param->musicPinch = SEQ_WB_BA_PINCH_TEST_150KB;
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
void FIELD_ENCOUNT_SetTrainerBattleSetupParam(
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup, int tr_id, HEAPID heapID )
{
  KAGAYA_Printf( "トレーナーバトルパラム作成 HEAPID=%d\n", heapID );
  enc_CreateTrainerBattleParam( enc, setup, heapID, tr_id );
}

//--------------------------------------------------------------
/**
 * PP追加
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void enc_AddPartyPokemon(
    POKEPARTY *party, u32 monsNo, u32 lv, u32 id, HEAPID heapID ) 
{
  {
    //monsNo
    KAGAYA_Printf( "PP追加 %d\n", monsNo );
  }

  {
    POKEMON_PARAM *pp = PP_Create( monsNo, lv, id, heapID );
    PokeParty_Add( party, pp );
    GFL_HEAP_FreeMemory( pp );
  }
}

//--------------------------------------------------------------
/**
 * バトルセットアップ開放
 * @param
 * @retval
 */
//--------------------------------------------------------------
static const void enc_FreeBattleSetupParam( BATTLE_SETUP_PARAM *param)
{
  if (param->partyPlayer)
  {
    GFL_HEAP_FreeMemory(param->partyPlayer);
  }
  if (param->partyPartner)
  {
    GFL_HEAP_FreeMemory(param->partyPartner);
  }
  if (param->partyEnemy1)
  {
    GFL_HEAP_FreeMemory(param->partyEnemy1);
  }
  if (param->partyEnemy2)
  {
    GFL_HEAP_FreeMemory(param->partyEnemy2);
  }
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（草むら/砂漠/洞窟）
 * @param nothing
 * @return u8 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 enc_GetRandWildGroundPokeTblNo( int max )
{
	u32	i;
  
	i = enc_GetPercentRand();
#if 0
	if( i < 20 )			return	0;		// 20%
	if( i >= 20 && i < 40 )	return	1;		// 20%
	if( i >= 40 && i < 50 )	return	2;		// 10%
	if( i >= 50 && i < 60 )	return	3;		// 10%
	if( i >= 60 && i < 70 )	return	4;		// 10%
	if( i >= 70 && i < 80 )	return	5;		// 10%
	if( i >= 80 && i < 85 )	return	6;		//  5%
	if( i >= 85 && i < 90 )	return	7;		//  5%
	if( i >= 90 && i < 94 )	return	8;		//  4%
	if( i >= 94 && i < 98 )	return	9;		//  4%
	if( i == 98 )			return	10;		//  1%
	return	11;								//  1%
#else
  //現状 新ポケを全て見せる為に無作為
  i %= max;
  return( i );
#endif
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（水上）
 * @param nothing
 * @return u8 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 enc_GetRandWildWaterPokeTblNo( void )
{
	u32	i;
  
	i = enc_GetPercentRand();

	if( i < 60 )			return	0;		// 60%
	if( i >= 60 && i < 90 )	return	1;		// 30%
	if( i >= 90 && i < 95 )	return	2;		//  5%
	if( i >= 95 && i < 99 )	return	3;		//  4%
	return	4;								//  1%
}

typedef enum
{
	FISHINGROD_BAD,			///<ぼろのつりざお
	FISHINGROD_GOOD,		///<いいつりざお
	FISHINGROD_GREAT,		///<すごいつりざお
}FISHINGROD_TYPE;

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（釣り）
 * @param	inFishingRod		釣竿
 * @return u8 エンカウントデータのテーブル番号		
 */
//--------------------------------------------------------------
static u8 enc_GetRandFishingPokeTblNo( const FISHINGROD_TYPE inFishingRod )
{
	u8	i;
	u8	p = 0;

	i = enc_GetPercentRand();
   
	switch( inFishingRod ){
	case FISHINGROD_BAD:		// ボロの釣竿
		if		( i < 60 )	p = 0;	// 60%
		else if ( i < 90 )	p = 1;	// 30%
		else if ( i < 95 )	p = 2;	// 5%
		else if ( i < 99 )	p = 3;	// 4%	
		else				p = 4;	// 1%
		break;
	case FISHINGROD_GOOD:		// 良い釣竿
		if		( i < 40 )	p = 0;	// 40%
		else if ( i < 80 )	p = 1;	// 40%
		else if ( i < 95 )	p = 2;	// 15%
		else if ( i < 99 )	p = 3;	// 4%
		else				p = 4;	// 1%
		break;
	case FISHINGROD_GREAT:		// 凄い釣竿
		if		( i < 40 )	p = 0;	// 40%
		else if ( i < 80 )	p = 1;	// 40%
		else if ( i < 95 )	p = 2;	// 15%
		else if ( i < 99 )	p = 3;	// 4%
		else				p = 4;	// 1%
		break;
	default:
		GF_ASSERT(0&&"unknown fishing rod");
	}

	return	p;
}

//--------------------------------------------------------------
/**
 * レベル設定
 * @param	inData  共通エンカウントデータテーブル
 * @param	inFldSpa  特性チェック・フォルム変更用構造体へのポインタ
 * @return	u8  レベル
 */
//--------------------------------------------------------------
static u8 enc_GetEncountPokeLv(
    const ENC_COMMON_DATA *inData, const ENC_FLD_SPA *inFldSpa )
{
	u8	size;
	u8	plus;
	u8	min, max;
	u8	spa;

	// Max Lv が Min Lv より小さい場合の修正処理
	if( inData->LvMax >= inData->LvMin ){
		min = inData->LvMin;
		max = inData->LvMax;
	}else{
		min = inData->LvMax;
		max = inData->LvMin;
	}
  
	size = max - min + 1;		// 補正サイズ
	plus = GFUser_GetPublicRand( size );	// 補正値
  
	//「やるき」「プレッシャー」「はりきり」の特性発動チェック
  //レベルの高いポケモンと５割でエンカウント
	// 先頭のポケモンがタマゴでない
#if 0
	if( inFldSpa->Egg == 0 ){
		if( (inFldSpa->Spa == TOKUSYU_HARIKIRI)||
			(inFldSpa->Spa == TOKUSYU_YARUKI)||
			(inFldSpa->Spa == TOKUSYU_PURESSYAA) ){
			// 1/2で特性発動
			if( gf_p_rand(2) == 0 ){
				return 	( min + plus );		// 特性未発動
			}
			return	( max );	//特性発動。レベルをmaxにする
		}
	}
#endif
	return	( min + plus );		// エンカウントレベル
}

//--------------------------------------------------------------
/**
 * 
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
static u8 enc_GetMaxLvMonsTblNo(
    const ENC_COMMON_DATA *inEncCommonData,
    const ENC_FLD_SPA *inFldSpa, const u8 inTblNo)
{

	if( inFldSpa->Egg == 0 ){
		//やるき、はりきり、プレッシャー
    //特性発動チェックレベルの高いポケモンと５割でエンカウント
#if 0
		if( (inFldSpa->Spa == TOKUSYU_YARUKI) ||
        (inFldSpa->Spa == TOKUSYU_HARIKIRI) ||
        (inFldSpa->Spa == TOKUSYU_PURESSYAA) )
    {
			if( (enc_GetPercentRand() % 50) ) //5割で発動
      {
        u8 i;
        u8 tbl_no = inTblNo; //テーブル番号保存
       
	  		//同じモンスターナンバーのテーブルを検索
  			for( i = 0; i < ENC_MONS_NUM_NORMAL; i++ )
        {
	  			if( inEncCommonData[i].MonsNo ==  //同じモンスターナンバーか？
              inEncCommonData[tbl_no].MonsNo ){
	  				if( inEncCommonData[i].LvMax >  //レベル比較
                inEncCommonData[tbl_no].LvMax ){
				  		tbl_no = i; //テーブル番号の更新
			  		}
			  	}
			  }
			  return tbl_no;
      }
		}
#endif
	}
  
	return inTblNo;
}

//--------------------------------------------------------------
/**
 * エンカウントデータセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL enc_SetEncountData(
    FIELD_ENCOUNT *enc,
    BATTLE_SETUP_PARAM *param,
    const ENC_FLD_SPA *inFldSpa,
    const ENC_COMMON_DATA *enc_data, u32 location, HEAPID heapID,
    int tbl_max )
{
  BOOL result = FALSE;
  u32 lv = 0, no = 0;
  
  switch( location )
  {
  case ENCOUNT_LOCATION_GROUND:
    { //じりょく特性による、はがねタイプのエンカウント率の向上
    }
    
    { //せいでんき特性による、でんきタイプのエンカウント率の向上
    }
    
    if( result == FALSE ){
      no = enc_GetRandWildGroundPokeTblNo( tbl_max );
    }
    
		//隠し特性による最大レベルポケモン選出(地上)
    no = enc_GetMaxLvMonsTblNo( enc_data, inFldSpa, no );
    lv = enc_data[no].LvMax;
    break;
  case ENCOUNT_LOCATION_WATER:
    { //じりょく特性による、はがねタイプのエンカウント率の向上
    }

    { //じりょく特性による、はがねタイプのエンカウント率の向上
    }

    if( result == FALSE ){
			//特性による指定タイプとのエンカウントに失敗。通常のエンカウント
      no = enc_GetRandWildWaterPokeTblNo();
    }
    
    lv = enc_GetEncountPokeLv( &enc_data[no], inFldSpa );
    break;
  case ENCOUNT_LOCATION_FISHING:
    { //じりょく特性による、はがねタイプのエンカウント率の向上
    }

    { //じりょく特性による、はがねタイプのエンカウント率の向上
    }
    
    if( result == FALSE ){
			//特性による指定タイプとのエンカウントに失敗。通常のエンカウント
      no = enc_GetRandWildWaterPokeTblNo();
    }
    
    lv = enc_GetEncountPokeLv( &enc_data[no], inFldSpa );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  { //特性によるレベル差戦闘回避
  }
  
  { //スプレーチェック
  }
  
  enc_CreateBattleParam(
      enc, param, heapID, enc_data[no].MonsNo, lv, inFldSpa->TrainerID );
  return( TRUE );
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

//--------------------------------------------------------------
/**
 * エンカウントデータ取得 仮
 * @param
 * @retval
 */
//--------------------------------------------------------------
static const ENCOUNT_DATA * enc_GetEncountData(
    FIELD_ENCOUNT *enc, MAPATTR attr )
{
  MAPATTR_VALUE attr_value = MAPATTR_GetAttrValue( attr );

  if( MAPATTR_VALUE_CheckEncountGrassB(attr_value) ){
    return( &dataTestEncountData01 );
  }
  
  return( &dataTestEncountData );
}

//--------------------------------------------------------------
/**
 * 特性、スプレーチェックなどの情報を集計して構造体にセット
 * @param	fsys			フィールドシステムポインタ
 * @param	inPokeParam		自分のポケモンパラム
 * @param	inData			エンカウントデータ
 * @param	outSpa			情報構造体ポインタ
 * @return	none
 */
//--------------------------------------------------------------
static void enc_SetSpaStruct(
    FIELD_ENCOUNT *enc,
    const POKEPARTY *party,
    const ENCOUNT_DATA* inData, ENC_FLD_SPA *outSpa  )
{
	//先頭ポケモンのタマゴチェック
#if 0
	if( PokeParaGet( inPokeParam, ID_PARA_tamago_flag, NULL ) == 0 ){
		// 先頭のポケモンがタマゴでない
		outSpa->Egg = 0;
    // 特殊能力取得
		outSpa->Spa = PokeParaGet(inPokeParam, ID_PARA_speabino, NULL);
	}else{
		// 先頭のポケモンがタマゴ
		outSpa->Egg = 1;
		outSpa->Spa = TOKUSYU_MAX;
	}
#else
  {
    outSpa->Egg = 0; //タマゴではない
		outSpa->Spa = 0; //特性無し
  }
#endif
  
	outSpa->SpMyLv = 0;
	outSpa->SprayCheck = FALSE;
	outSpa->EncCancelSpInvalid = FALSE;
	
	if( inData != NULL ){
		//シーウシ・シードルゴフォルム変更確率セット
		outSpa->FormProb[0] = inData->FormProb[0];	//シーウシ
		outSpa->FormProb[1] = inData->FormProb[1];	//シードルゴ
		
    //アンノーン
		if (inData->AnnoonTable != 0){
			GF_ASSERT( inData->AnnoonTable <= 8 && "annoon_table_over" );
			//アンノーンテーブルタイプセット
			outSpa->AnnoonTblType = inData->AnnoonTable-1;
		}else{
			outSpa->AnnoonTblType = 0;
		}
	}
  
	//トレーナーＩＤのセット
#if 0
	outSpa->TrainerID = MyStatus_GetID( SaveData_GetMyStatus(fsys->savedata) );
#else
  outSpa->TrainerID = 3539; //kari
#endif
}

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

//======================================================================
//  data
//======================================================================
#include "poke_tool/monsno_def.h"

#if 0 //0905ROM用
//--------------------------------------------------------------
//  仮エンカウントデータ  
//--------------------------------------------------------------
static const ENCOUNT_DATA dataTestEncountData =
{
  30, //エンカウント率
  { //通常エンカウントポケモン
    {
      5,  //レベル
      494,//ピンボー
    },
    {
      5,  //レベル
      495,//モグリュー
    },
    {
      5,  //レベル
      496,//ゴリダルマ
    },
    {
      5,  //レベル
      497,//ワニメガネ
    },
    {
      5,  //レベル
      498,//ワニグラス
    },
/*
    {
      5,  //レベル
      499,//ワニバーン
    },
*/
    {
      5,  //レベル
      500,//ハゴロムシ
    },
    {
      5,  //レベル
      501,//ハキシード
    },
/*
    {
      5,  //レベル
      502,//オノックス
    },
*/
    {
      5,  //レベル
      503,//カーメント
    },
/*
    {
      5,  //レベル
      504,//バンビーナ
    },
*/
    {
      5,  //レベル
      505,//バトロード
    },
    {
      5,  //レベル
      506,//プルンス
    },
    {
      5,  //レベル
      507,//コバト
    },
    {
      5,  //レベル
      508,//ライブラ
    },
    {
      5,  //レベル
      509,//バクパク
    },
    {
      5,  //レベル
      510,//ユリコネ
    },
    {
      5,  //レベル
      511,//オーバト
    },
    {
      5,  //レベル
      512,//パンチーヌ
    },
    {
      5,  //レベル
      513,//モコウモリ
    },
  },

  { //大量発生
    5, 5,
  },
  { //昼エンカウント
    5, 5,
  },
  { //夜エンカウント
    5, 5,
  },
  { //揺れ草
    0, 0, 0, 0,
  },
  
  { //フォルム変更確率
    0,0,0,0,0,
  },
  
  0, //アンノーン出現テーブル
  
  { //AGBスロット ルビーエンカウントデータ
    0, 0,
  },
  { //AGBスロット サファイアエンカウントデータ
    0, 0, 
  },
  { //AGBスロット エメラルドエンカウントデータ
    0, 0,
  },
  { //AGBスロット 赤エンカウントデータ
    0, 0,
  },
  { //AGBスロット 緑エンカウントデータ
    0, 0,
  },
  
  30, //海エンカウント率
  { //海エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //岩砕きエンカウント率
  { //岩砕きエンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //ボロ釣竿エンカウント率
  { //ボロ釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //いい釣竿エンカウント率
  { //いい釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //凄い釣竿エンカウント率
  { //凄い釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
};
#endif //0905ROM用

#if 0
494	ピンボー
495	モグリュー
497	ワニグラス
498	サンダイル
500	ハゴモリ
504	バンビーナ
506	プルンス
507	マメバト
508	ライブラ
510	チュリネ
511	ハトーボー
513	モコウモリ
514	モコット
516	サボッテン
#define	MONSNO_SABOTTEN	(516)
517	ハガクレ
#define	MONSNO_HAGAKURE	(517)
518	グロッケン
#define	MONSNO_GUROKKEN	(518)
519	カーオケン
#define	MONSNO_KAAOKEN	(519)
521	ヒダルマ
#define	MONSNO_HIDARUMA	(521)
522	ゾロア
#define	MONSNO_ZOROA	(522)
524	シロベア
#define	MONSNO_SIROBEA	(524)
526	チラッチ
#define	MONSNO_TIRATTI	(526)
530	チャーク
#define	MONSNO_TYAAKU	(530)
532	ナスカッチ
#define	MONSNO_NASUKATTI	(532)
533	ムジャジャ
#define	MONSNO_MUZYAZYA	(533)

496	ゴリダルマ
#define	MONSNO_GORIDARUMA	(496)
499	レイバーン
#define	MONSNO_REIBAAN	(499)
501	ハキシード
#define	MONSNO_HAKISIIDO	(501)
502	オノックス
#define	MONSNO_ONOKKUSU	(502)
503	カーメント
#define	MONSNO_KAAMENTO	(503)
505	ケンホロウ
#define	MONSNO_KENHOROU	(505)
509	ユメバクラ
#define	MONSNO_YUMEBAKURA	(509)
512	ドレディア
#define	MONSNO_DOREDHIA	(512)
515	メリコット
#define	MONSNO_MERIKOTTO	(515)
520	シキジカ
#define	MONSNO_SIKIZIKA	(520)
523	ゾロアーク
#define	MONSNO_ZOROAAKU	(523)
525	シャーベア
#define	MONSNO_SYAABEA	(525)
527	チラニー
#define	MONSNO_TIRATTI	(526)
528	ドゴウモリ
#define	MONSNO_DOGOUMORI	(528)
529	プルキング
#define	MONSNO_PURUKINGU	(529)
531	ドチャック
#define	MONSNO_DOTYAKKU	(531)
#endif

//090731ROM用 GRASS A
static const ENCOUNT_DATA dataTestEncountData =
{
  24, //通常エンカウントポケモン数
  30, //エンカウント率
  { //通常エンカウントポケモン
    {
      5,  //レベル
      MONSNO_PINBOO,
    },
    {
      5,  //レベル
      MONSNO_MOGURYUU,
    },
    {
      5,  //レベル
      MONSNO_WANIGURASU,
    },
    {
      5,  //レベル
      MONSNO_SANDAIRU,
    },
    {
      5,  //レベル
      MONSNO_HAGOMORI,
    },
    {
      5,  //レベル
      MONSNO_BANBIINA,
    },
    {
      5,  //レベル
      MONSNO_PURUNSU,
    },
    {
      5,  //レベル
      MONSNO_MAMEBATO,
    },
    {
      5,  //レベル
      MONSNO_RAIBURA,
    },
    {
      5,  //レベル
      MONSNO_TYURINE,
    },
    {
      5,  //レベル
      MONSNO_HATOOBOO,
    },
    {
      5,  //レベル
      MONSNO_MOKOUMORI,
    },
    {
      5,  //レベル
      MONSNO_MOKOTTO,
    },
    {
      5,  //レベル
      MONSNO_SABOTTEN,
    },
    {
      5,  //レベル
      MONSNO_HAGAKURE,
    },
    {
      5,  //レベル
      MONSNO_GUROKKEN,
    },
    {
      5,  //レベル
      MONSNO_KAAOKEN,
    },
    {
      5,  //レベル
      MONSNO_HIDARUMA,
    },
    {
      5,  //レベル
      MONSNO_ZOROA,
    },
    {
      5,  //レベル
      MONSNO_SIROBEA,
    },
    {
      5,  //レベル
      MONSNO_TIRATTI,
    },
    {
      5,  //レベル
      MONSNO_TYAAKU,
    },
    {
      5,  //レベル
      MONSNO_NASUKATTI,
    },
    {
      5,  //レベル
      MONSNO_MUZYAZYA,
    },
  },

  { //大量発生
    5, 5,
  },
  { //昼エンカウント
    5, 5,
  },
  { //夜エンカウント
    5, 5,
  },
  { //揺れ草
    0, 0, 0, 0,
  },
  
  { //フォルム変更確率
    0,0,0,0,0,
  },
  
  0, //アンノーン出現テーブル
  
  { //AGBスロット ルビーエンカウントデータ
    0, 0,
  },
  { //AGBスロット サファイアエンカウントデータ
    0, 0, 
  },
  { //AGBスロット エメラルドエンカウントデータ
    0, 0,
  },
  { //AGBスロット 赤エンカウントデータ
    0, 0,
  },
  { //AGBスロット 緑エンカウントデータ
    0, 0,
  },
  
  30, //海エンカウント率
  { //海エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //岩砕きエンカウント率
  { //岩砕きエンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //ボロ釣竿エンカウント率
  { //ボロ釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //いい釣竿エンカウント率
  { //いい釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //凄い釣竿エンカウント率
  { //凄い釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
};

static const ENCOUNT_DATA dataTestEncountData01 =
{
  16, //通常エンカウントポケモン数
  30, //エンカウント率
  { //通常エンカウントポケモン
    {
      15,  //レベル
      MONSNO_GORIDARUMA,
    },
    {
      15,  //レベル
      MONSNO_REIBAAN,
    },
    {
      15,  //レベル
      MONSNO_HAKISIIDO,
    },
    {
      15,  //レベル
      MONSNO_ONOKKUSU,
    },
    {
      15,  //レベル
      MONSNO_KAAMENTO,
    },
    {
      15,  //レベル
      MONSNO_KENHOROU,
    },
    {
      15,  //レベル
      MONSNO_YUMEBAKURA,
    },
    {
      15,  //レベル
      MONSNO_DOREDHIA,
    },
    {
      15,  //レベル
      MONSNO_MERIKOTTO,
    },
    {
      15,  //レベル
      MONSNO_SIKIZIKA,
    },
    {
      15,  //レベル
      MONSNO_ZOROAAKU,
    },
    {
      15,  //レベル
      MONSNO_SYAABEA,
    },
    {
      15,  //レベル
      MONSNO_TIRATTI,
    },
    {
      15,  //レベル
      MONSNO_DOGOUMORI,
    },
    {
      15,  //レベル
      MONSNO_PURUKINGU,
    },
    {
      15,  //レベル
      MONSNO_DOTYAKKU,
    },
  },

  { //大量発生
    5, 5,
  },
  { //昼エンカウント
    5, 5,
  },
  { //夜エンカウント
    5, 5,
  },
  { //揺れ草
    0, 0, 0, 0,
  },
  
  { //フォルム変更確率
    0,0,0,0,0,
  },
  
  0, //アンノーン出現テーブル
  
  { //AGBスロット ルビーエンカウントデータ
    0, 0,
  },
  { //AGBスロット サファイアエンカウントデータ
    0, 0, 
  },
  { //AGBスロット エメラルドエンカウントデータ
    0, 0,
  },
  { //AGBスロット 赤エンカウントデータ
    0, 0,
  },
  { //AGBスロット 緑エンカウントデータ
    0, 0,
  },
  
  30, //海エンカウント率
  { //海エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //岩砕きエンカウント率
  { //岩砕きエンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //ボロ釣竿エンカウント率
  { //ボロ釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //いい釣竿エンカウント率
  { //いい釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
  
  30, //凄い釣竿エンカウント率
  { //凄い釣竿エンカウントポケモン
    {
      10,
      5,
      494,//ピンボー
    },
    {
      10,
      5,
      495,//モグリュー
    },
    {
      10,
      5,
      496,//ゴリダルマ
    },
    {
      10,
      5,
      497,//ワニメガネ
    },
    {
      10,
      5,
      498,//ワニグラス
    },
  },
};
