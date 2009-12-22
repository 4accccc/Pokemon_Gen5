/*
 *  @file   enc_pokeset.c
 *  @brief  エンカウントする野生ポケモンデータ生成
 *  @author Miyuki Iwasawa
 *  @date   09.10.08
 */

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
#include "savedata/encount_sv.h"
#include "move_pokemon_def.h"
#include "move_pokemon.h"
#include "enc_pokeset.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

//======================================================================
//  struct
//======================================================================

///大量発生ポケモンデータテーブル型
typedef struct GENERATE_ENCOUNT{
  u16 zone_id;
  u16 mons_no;
  u8  form;
  u8  lv_min;
  u8  lv_max;
}GENERATE_ENCOUNT;

///ランダムポケモン抽選関数型
typedef u32 (*ENC_PROB_CALC_FUNC)( u32 max );

typedef enum{
  EP_ITEM_RANGE_NORMAL = ITEM_RANGE_NORMAL,
  EP_ITEM_RANGE_HUKUGAN = ITEM_RANGE_HUKUGAN,
}ITEM_RANGE;

#define PPD_ITEM_SLOT_NUM (3) //パーソナルのアイテムスロット数

///@todo アイテムを持たせる確率定義 ビンゴがなくなる場合、変更が必要かも
static u8 DATA_ItemRangeTable[][PPD_ITEM_SLOT_NUM] = {
   {50,5,0},  //通常
   {60,20,0}, //通常ふくがん
   {45,5,5},  //ビンゴ
   {55,20,5}, //ビンゴふくがん
};

//大量発生データ
#include "fieldmap/zone_id.h"
#include "generate_enc_def.h"
#include "generate_enc.cdat"

#define GENERATE_ENC_PROB (40*10)

////////////////////////////////////////////////////////////
//プロトタイプ
static u32 eps_GetPercentRand( void );
static void efp_MonsSpaCheck( ENCPOKE_FLD_PARAM* efp, const WEATHER_NO weather );
static BOOL encpoke_CheckEncountAvoid( const ENCPOKE_FLD_PARAM *efp, u8 level );
static const GENERATE_ENCOUNT* encpoke_GetGenerateData( const GAMEDATA* gdata );
static BOOL eps_CheckGeneratePoke( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* ioEfp, u16 zone_id, ENC_COMMON_DATA* outTable);

static u32 eps_GetEncountTable( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* ioEfp, u16 zone_id, ENC_COMMON_DATA* outTable);

static void eps_SetPokeParam(const ENC_COMMON_DATA* data,ENC_POKE_PARAM* outPoke);
static u8 eps_LotFixTypeEncount(const ENCPOKE_FLD_PARAM* efp,const ENC_COMMON_DATA* enc_data,const u8 type );
static void eps_CheckSpaEncountLevel(const ENCPOKE_FLD_PARAM* efp,const ENC_COMMON_DATA* enc_tbl,ENC_POKE_PARAM* ioPoke );
static void eps_PokeLottery( const ENCPOKE_FLD_PARAM* efp, const ENC_COMMON_DATA* enc_tbl, ENC_POKE_PARAM* outPoke );

static void eps_EncPokeItemSet(POKEMON_PARAM* pp,
    POKEMON_PERSONAL_DATA* personal,ITEM_RANGE item_range,u8 bingo_f,u8 negau_hosei);
static u32 eps_EncPokeCalcPersonalRand(
    const ENCPOKE_FLD_PARAM* efp, const POKEMON_PERSONAL_DATA*  personal,const ENC_POKE_PARAM* poke );

//////////////////////////////////////////////////////
///抽選確率計算関数テーブル
static u32 eps_GetPercentRandGroundPokeTblNo( u32 max );
static u32 eps_GetPercentRandWaterPokeTblNo( u32 max );
static u32 eps_GetPercentRandFishingPokeTblNo( u32 max );
static u32 eps_GetPercentRandNumPokeTblNo( u32 max );

static const ENC_PROB_CALC_FUNC DATA_EncProbCalcFuncTable[ENCPROB_CALCTYPE_MAX] = {
  eps_GetPercentRandGroundPokeTblNo,
  eps_GetPercentRandWaterPokeTblNo,
  eps_GetPercentRandFishingPokeTblNo,
  eps_GetPercentRandNumPokeTblNo,
};

//---------------------------------------------------------------------------
/*
 *  @brief  ポケモンエンカウント時フィールドパラメータ構造体デフォルトセット
 */
//---------------------------------------------------------------------------
void ENCPOKE_SetEFPStruct(ENCPOKE_FLD_PARAM* outEfp, const GAMEDATA* gdata,
    const ENCOUNT_LOCATION location, const ENCOUNT_TYPE enc_type, const WEATHER_NO weather)
{
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork((GAMEDATA*)gdata);
  POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* pp;

  MI_CpuClear8(outEfp,sizeof(ENCPOKE_FLD_PARAM));

  outEfp->location = location;
  outEfp->enc_type = enc_type;

  outEfp->gdata = gdata;
  outEfp->enc_save = EncDataSave_GetSaveDataPtr( save );

  //プレイヤーのIDセット
  outEfp->my = GAMEDATA_GetMyStatus( (GAMEDATA*)gdata );
  outEfp->myID = MyStatus_GetID(outEfp->my);

  //先頭ポケモンのパラメータチェック
  pp = PokeParty_GetMemberPointer( party, 0 );

  outEfp->mons_egg_f = PP_Get( pp, ID_PARA_tamago_flag, NULL );
  if(!outEfp->mons_egg_f)
  { //モンスターNo他取得
    outEfp->mons_no = PP_Get( pp, ID_PARA_monsno, NULL);
    outEfp->mons_item = PP_Get( pp, ID_PARA_item, NULL);
    outEfp->mons_spa = PP_Get( pp, ID_PARA_speabino, NULL);
    outEfp->mons_sex = PP_Get( pp, ID_PARA_sex, NULL);
    outEfp->mons_chr = PP_GetSeikaku( pp );

    //特性効果発生チェック
    efp_MonsSpaCheck( outEfp, WEATHER_NO_SUNNY);
  }
  
  //////////////////////////////
  //フラグチェック
  outEfp->gameclear_f = EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork( (GAMEDATA*)gdata ),SYS_FLAG_GAME_CLEAR); 

  //釣り戦闘かどうか
  if( location == ENC_LOCATION_FISHING || location == ENC_LOCATION_FISHING_SP ){
    outEfp->fishing_f = TRUE;
  }
  //スプレーチェック
  if ( !EncDataSave_CanUseSpray( EncDataSave_GetSaveDataPtr(save) ) ){
    outEfp->spray_f = TRUE;
  }
  //強制エンカウントチェック
  if( outEfp->enc_type == ENC_TYPE_FORCE ){
    outEfp->enc_force_f = TRUE;
    outEfp->enc_type = ENC_TYPE_NORMAL;  //強制フラグを立てたら、後はNormalタイプでエミュレート
  }
  if( outEfp->enc_type == ENC_TYPE_EFFECT ){
    outEfp->enc_force_f = TRUE;
  }

  outEfp->companion_f = FALSE;
  outEfp->enc_double_f = FALSE;

  //ダブルエンカウントチェック
  if( outEfp->location == ENC_LOCATION_GROUND_H ){
    if(eps_GetPercentRand() < 10) {
      outEfp->enc_double_f = TRUE;
    }
  }
  //スプレー及び低レベルエンカウント回避チェックに用いるレベルを取得
  if( outEfp->spa_low_lv_rm || outEfp->spray_f){
    //手持ちの戦える一匹目のレベルを取得
    int idx = PokeParty_GetMemberTopIdxBattleEnable( party );
    pp = PokeParty_GetMemberPointer( party, idx );
    outEfp->spray_lv = PP_Get( pp, ID_PARA_level, NULL);
  }

  //エンカウントさせるポケモン数
  outEfp->enc_poke_num = outEfp->enc_double_f+1;
}

//--------------------------------------------------------------
/**
 * 特性、道具使用による確率変動
 *
 * @param enc       エンカウントシステムポインタ
 * @param efp       エンカウント情報構造体ポインタ
 * @param inProb    計算元の確率
 * @return  変動後の確率
 */
//--------------------------------------------------------------
u32 ENCPOKE_EncProbManipulation( const ENCPOKE_FLD_PARAM* efp, const GAMEDATA* gdata, const u32 inProb)
{
  u32 new_prob = inProb;

  //特性による確率変更
  if( efp->spa_rate_up ){
    new_prob *= 2;
  }else if(efp->spa_rate_down){
    new_prob /= 2;
  }
  //装備アイテム(きよめのおふだ/きよめのおこう)による確率変動
  if(efp->mons_item == ITEM_KIYOMENOOHUDA || efp->mons_item == ITEM_KIYOMENOOKOU){
    new_prob = (inProb * 2) / 3;
  }
  if(new_prob >= 100){
    new_prob = 100;
  }
  return new_prob;
}

//--------------------------------------------------------------
/**
 * @brief エンカウントテーブルとENCPOKE_FLD_PARAMからポケモンパラメータ抽選
 * @param
 * @return  抽選されたポケモン数(0ならエンカウント失敗)
 */
//--------------------------------------------------------------
u32 ENCPOKE_GetEncountPoke( const ENCPOKE_FLD_PARAM *efp, const ENC_COMMON_DATA *enc_tbl, ENC_POKE_PARAM* outPokeTbl )
{
  int i;
  u32 num = 0;

  for(i = 0;i < efp->enc_poke_num;i++){
    eps_PokeLottery( efp, enc_tbl, &outPokeTbl[num] );
    if( encpoke_CheckEncountAvoid( efp, outPokeTbl[num].level)){
      continue;
    }
    num++;
  }
  return( num );
}

//--------------------------------------------------------------
/**
 * ROMのエンカウントデータテーブルからポケモンを抽選
 *
 * @param enc       エンカウントシステムポインタ
 * @param efp       エンカウント情報構造体ポインタ
 * @param inProb    計算元の確率
 * @return  変動後の確率
 */
//--------------------------------------------------------------
int ENCPOKE_GetNormalEncountPokeData( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* efp, u16 zone_id, ENC_POKE_PARAM* outPokeTbl)
{
  u32 num = 0;
  ENC_COMMON_DATA enc_tbl[ENC_MONS_NUM_MAX];

  //ROMからエンカウントデータテーブルを取得
  eps_GetEncountTable( inData, efp, zone_id, enc_tbl);

  //エンカウント抽選
  num = ENCPOKE_GetEncountPoke( efp, enc_tbl, outPokeTbl );

  return num;
}

//--------------------------------------------------------------
/**
 * 移動ポケモンエンカウト抽選
 *
 * @param enc       エンカウントシステムポインタ
 * @param efp       エンカウント情報構造体ポインタ
 * @param inProb    計算元の確率
 * @return  変動後の確率
 */
//--------------------------------------------------------------
MPD_PTR ENCPOKE_GetMovePokeEncountData( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* efp, u16 zone_id )
{
	MPD_PTR temp[MOVE_POKE_MAX];
	u8 i, enc_count;

	//同じ場所にいる移動ポケモンを取得
	enc_count = 0;
	for( i = 0; i < MOVE_POKE_MAX; i++){
    MPD_PTR mpd;
		if( !EncDataSave_IsMovePokeValid( efp->enc_save,i) ){
      continue;
    }
    mpd = EncDataSave_GetMovePokeDataPtr( efp->enc_save, i );
		if( zone_id != EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_ZONE_ID )){
      continue;
		}
    if( encpoke_CheckEncountAvoid( efp,
          EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_LV))){
      continue;
    }
		temp[enc_count++] = mpd;
	}

	if (enc_count == 0 || GFUser_GetPublicRand0(1000) < 500){
		return NULL;
	}
	
  //ランダムで選出
	return temp[ GFUser_GetPublicRand0(enc_count) ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCエンカウントポケモンの取得
 *
 *	@param	cp_wfbc     WFBCワーク
 *	@param  efp         エンカウントの状況
 *	@param	outPokeTbl  出力先
 *
 * @return  抽選されたポケモン数(0ならエンカウント失敗)
 */
//-----------------------------------------------------------------------------
u32 ENCPOKE_GetWFBCEncountPoke( const FIELD_WFBC* cp_wfbc, ENCPOKE_FLD_PARAM* efp, ENC_POKE_PARAM* outPokeTbl )
{
  u32 num = 0;
  int i;
  int tbl_index;
  const FIELD_WFBC_PEOPLE* cp_people;
  const FIELD_WFBC_PEOPLE_DATA* cp_people_data;
  u32 rand;
  
  GF_ASSERT( cp_wfbc );
  GF_ASSERT( outPokeTbl );

  switch( efp->location )
  {
  case ENC_LOCATION_GROUND_L:
    tbl_index=  FIELD_WFBC_PEOPLE_ENC_POKE_GRASS_LOW;
    break;
  case ENC_LOCATION_GROUND_H:
    tbl_index=  FIELD_WFBC_PEOPLE_ENC_POKE_GRASS_HIGH;
    break;
  case ENC_LOCATION_WATER:
    tbl_index=  FIELD_WFBC_PEOPLE_ENC_POKE_WATER;
    break;

  // なしはさようなら
  case ENC_LOCATION_NONE:
    return 0;

  default:
    GF_ASSERT_MSG( 0, "WFBC enc not support enc_location = %d\n", efp->location );
    break;
  }
  

  for( i=0; i<FIELD_WFBC_NPCID_MAX; i++ )
  {

    cp_people  = FIELD_WFBC_GetPeople( cp_wfbc, i );
    if( cp_people )
    {
      cp_people_data = FIELD_WFBC_PEOPLE_GetPeopleData( cp_people );

      rand = GFUser_GetPublicRand( FIELD_WFBC_PEOPLE_ENC_POKE_PERCENT_MAX );

      TOMOYA_Printf( "percent %d\n", cp_people_data->enc_percent[ tbl_index ] );
      // ポケモン抽選
      if( rand < cp_people_data->enc_percent[ tbl_index ] )
      {
        GFL_STD_MemClear( &outPokeTbl[0], sizeof(ENC_POKE_PARAM) );
        
        // ヒット
        outPokeTbl[0].monsNo  = cp_people_data->enc_monsno[ tbl_index ];
        outPokeTbl[0].level   = cp_people_data->enc_lv[ tbl_index ];

        TOMOYA_Printf( "monsNo %d level %d\n", outPokeTbl[0].monsNo, outPokeTbl[0].level );
        return 1;
      }
    }
  }


  return 0;
}

/*
 *  @brief  エンカウントポケモン　POKEPARA生成
 */
POKEMON_PARAM* ENCPOKE_PPCreate(const ENCPOKE_FLD_PARAM* efp, const ENC_POKE_PARAM* poke, int heapID)
{
  POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );

  ENCPOKE_PPSetup( pp, efp, poke );

  return pp;
}

/*
 *  @brief  エンカウントポケモン　POKEPARA生成
 */
void ENCPOKE_PPSetup(POKEMON_PARAM* pp,const ENCPOKE_FLD_PARAM* efp, const ENC_POKE_PARAM* poke )
{
  u8 chr,sex;
  POKEMON_PERSONAL_DATA*  personal;

  PP_Setup( pp,poke->monsNo,poke->level,efp->myID );

  //エンカウントポケモンのパーソナルを取得
  personal = POKE_PERSONAL_OpenHandle( poke->monsNo, poke->form, HEAPID_PROC );

  //フォルムセット
  PP_Put( pp, ID_PARA_form_no, poke->form );

  //アイテムセット
  if(poke->item != ITEM_DUMMY_DATA){
    PP_Put( pp, ID_PARA_item, poke->item );
  }else if(efp->enc_type == ENC_TYPE_BINGO){
    eps_EncPokeItemSet(pp,personal,efp->spa_item_rate_up,TRUE,0); //@todo
  }else{
    eps_EncPokeItemSet(pp,personal,efp->spa_item_rate_up,FALSE,0);
  }

  { //個体乱数セット
    u32 p_rnd = eps_EncPokeCalcPersonalRand( efp, personal, poke );

    PP_SetupEx( pp, poke->monsNo, poke->level, efp->myID, PTL_SETUP_POW_AUTO, p_rnd );
  }
  //親の性別と名前をPut
  PP_Put( pp, ID_PARA_oyasex, MyStatus_GetMySex(efp->my) );
  PP_Put( pp, ID_PARA_oyaname_raw, (u32)MyStatus_GetMyName(efp->my) );

  //性格代入処理
  if( efp->spa_chr_fix )
  {
    PP_Put( pp, ID_PARA_seikaku, efp->mons_chr );
  }

  PP_Renew( pp );
  POKE_PERSONAL_CloseHandle( personal );
}

/*
 *  @brief  エンカウント移動ポケモン　POKEPARA生成
 */
void ENCPOKE_PPSetupMovePoke(POKEMON_PARAM* pp,const ENCPOKE_FLD_PARAM* efp, MPD_PTR mpd )
{
  {
	  u16 monsno = EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_MONSNO);
	  u8 lv = EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_LV);
	  u32 pow_rnd = EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_POW_RND);
	  u32 per_rnd = EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_PER_RND);

    PP_SetupEx( pp, monsno, lv, efp->myID, pow_rnd, per_rnd );
  }
	PP_Put( pp, ID_PARA_condition, EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_COND) );		// コンディション
	PP_Put( pp, ID_PARA_hp, EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_HP) );		// HP
}

/*
 *  @brief  現在大量発生が起きているゾーンIDを返す
 *
 *  @retval 0xFFFF  大量発生が起きていない(クリア前)
 *  @retval それ以外  大量発生が起きているゾーンID
 */
u16 ENCPOKE_GetGenerateZone( const GAMEDATA* gdata )
{
  const GENERATE_ENCOUNT* gene = encpoke_GetGenerateData( gdata );

  if(gene == NULL){
    return 0xFFFF;
  }
  return gene->zone_id;
}

//===============================================================================
//サブ　エンカウトデータ関連
//===============================================================================
/*
 *  @brief  特性効果発生チェック
 */
static void efp_MonsSpaCheck( ENCPOKE_FLD_PARAM* ioEfp, const WEATHER_NO weather )
{
  //発動率100%
  switch(ioEfp->mons_spa)
  {
  ///<エンカウント率2倍(ありじごく他)
  case TOKUSYU_ARIZIGOKU:
  case TOKUSYU_NOOGAADO:
  case TOKUSYU_HAKKOU:
    ioEfp->spa_rate_up = TRUE;
    return;
  ///<エンカウント率1/2(あくしゅう他)
  case TOKUSYU_AKUSYUU:
  case TOKUSYU_SIROIKEMURI:
  case TOKUSYU_HAYAASI:
    ioEfp->spa_rate_down = TRUE;
    return;
  ///<砂嵐エンカウント率1/2（すながくれ他)
  case TOKUSYU_SUNAGAKURE:
    if(weather == WEATHER_NO_STORM){
      ioEfp->spa_rate_down = TRUE;
    }
    return;
  ///<あられエンカウント率1/2（ゆきがくれ他）
  case TOKUSYU_YUKIGAKURE:
    if( weather == WEATHER_NO_SNOW || weather == WEATHER_NO_SNOWSTORM || weather == WEATHER_NO_ARARE ){
      ioEfp->spa_rate_down = TRUE;
    }
    return;
  ///<釣りエンカウント率2倍(ねんちゃく他)
  case TOKUSYU_NENTYAKU:
  case TOKUSYU_KYUUBAN:
    if(ioEfp->fishing_f){
      ioEfp->spa_rate_up = TRUE;
    }
    return;
  ///<アイテムが手に入りやすい(ふくがん他)
  case TOKUSYU_HUKUGAN:
    ioEfp->spa_item_rate_up = TRUE;
    return;
  ///<手持ちと同じ性格(シンクロ他)
  case TOKUSYU_SINKURO:
    ioEfp->spa_chr_fix = TRUE;
    return;
  }

  //発動率2/3
  if((ioEfp->mons_spa == TOKUSYU_MEROMEROBODHI) && (eps_GetPercentRand() < 67)){
    ioEfp->spa_sex_fix = TRUE;      ///<手持ちと異なる性別(メロメロボディ他)
    return;
  }
  //発動率1/2
  if(eps_GetPercentRand() < 50){
    return;
  }
  switch(ioEfp->mons_spa)
  {
  ///<ハガネタイプとのエンカウント率アップ
  case TOKUSYU_ZIRYOKU:
    ioEfp->spa_hagane_up = TRUE;
    return;
  ///<電気タイプとのエンカウント率アップ
  case TOKUSYU_SEIDENKI:
    ioEfp->spa_denki_up = TRUE;
    return;
  ///<レベルの高いポケモンとエンカウントしやすい(やるき他)
  case TOKUSYU_HARIKIRI:
  case TOKUSYU_YARUKI:
  case TOKUSYU_PURESSYAA:
    ioEfp->spa_high_lv_hit = TRUE;
    return;
  ///<レベルの低いポケモンとエンカウントしない(威嚇他)
  case TOKUSYU_IKAKU:
  case TOKUSYU_SURUDOIME:
    ioEfp->spa_low_lv_rm = TRUE;
    return;
  }
}

//--------------------------------------------------------------
/*
 *  @brief  レベル差戦闘回避チェック
 *
 *  @retval NULL  大量発生が起きていない(クリア前)
 *  @retval それ以外  発生している大量発生パラメータ
 */
//--------------------------------------------------------------
static BOOL encpoke_CheckEncountAvoid( const ENCPOKE_FLD_PARAM *efp, u8 level )
{
  if(efp->enc_force_f){
    return FALSE;
  }
  //特性によるレベル差戦闘回避
  if(efp->spa_low_lv_rm && (efp->spray_lv - level) >= 5){
    return TRUE;
  }
  //スプレーチェック
  if(efp->spray_f && (level <= efp->spray_lv)){
    return TRUE;
  } 
  return FALSE;
}

//--------------------------------------------------------------
/*
 *  @brief  現在起きている大量発生データを返す
 *
 *  @retval NULL  大量発生が起きていない(クリア前)
 *  @retval それ以外  発生している大量発生パラメータ
 */
//--------------------------------------------------------------
static const GENERATE_ENCOUNT* encpoke_GetGenerateData( const GAMEDATA* gdata )
{
  const GENERATE_ENCOUNT* gene;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork((GAMEDATA*)gdata);
  BOOL gameclear_f = EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork( (GAMEDATA*)gdata ),SYS_FLAG_GAME_CLEAR ); 
  u8  idx;
 
  if( !gameclear_f ){
    return NULL;
  }
  idx = EncDataSave_GetGenerateZoneIdx( EncDataSave_GetSaveDataPtr( save ) );
 
  if( idx >= GENERATE_ENC_POKE_MAX ){
    idx = 0;
  }
  return &DATA_GenerateEncount[idx];
}

//--------------------------------------------------------------
/**
 * 大量発生チェック
 * @param inData    エンカウントデータ
 * @param location  取得するロケーション
 * @param type      エンカウントタイプ
 * @param outTable  取得したモンスターデータテーブルを格納するバッファポインタ
 * @retuen  テーブル長
 */
//--------------------------------------------------------------
static BOOL eps_CheckGeneratePoke( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* ioEfp, u16 zone_id, ENC_COMMON_DATA* outTable)
{
  const GENERATE_ENCOUNT* gene = encpoke_GetGenerateData( ioEfp->gdata );
  
  if( gene == NULL || gene->zone_id != zone_id ){
    return FALSE;
  }
  if( ioEfp->location != ENC_LOCATION_GROUND_L && ioEfp->location != ENC_LOCATION_GROUND_H ){
    return FALSE;
  }
  if( GFUser_GetPublicRand0( 1000 ) >= GENERATE_ENC_PROB ){
    return FALSE;
  }
  outTable[0].monsNo = gene->mons_no;
  outTable[0].minLevel = gene->lv_min;
  outTable[0].maxLevel = gene->lv_max;
  outTable[0].form = gene->form;
  
  ioEfp->prob_calctype = ENCPROB_CALCTYPE_EQUAL;
  ioEfp->tbl_num = 1;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * フィールドエンカウントデータテーブルをロケーションとタイプから取得
 * @param inData    エンカウントデータ
 * @param location  取得するロケーション
 * @param type      エンカウントタイプ
 * @param outTable  取得したモンスターデータテーブルを格納するバッファポインタ
 * @retuen  テーブル長
 */
//--------------------------------------------------------------
static u32 eps_GetEncountTable( const ENCOUNT_DATA *inData, ENCPOKE_FLD_PARAM* ioEfp, u16 zone_id, ENC_COMMON_DATA* outTable)
{
  u32 num = 0,calctype = ENCPROB_CALCTYPE_NORMAL;
  const ENC_COMMON_DATA* src;

  if(ioEfp->enc_type == ENC_TYPE_BINGO)
  {
    //ここでビンゴエンカウントテーブルを取得
    ioEfp->prob_calctype = ENCPROB_CALCTYPE_EQUAL;
    ioEfp->tbl_num = ENC_MONS_NUM_BINGO;
    return ioEfp->tbl_num;
  }
  //@todo 大量発生チェック
  if( eps_CheckGeneratePoke( inData, ioEfp, zone_id, outTable)){
    return ioEfp->tbl_num;
  }

  //テーブル取得
  if( ioEfp->location >= ENC_LOCATION_MAX ){
    GF_ASSERT_MSG(0,"GetEncountTable location error %d\n",ioEfp->location);
    ioEfp->location = ENC_LOCATION_GROUND_L;
  }
  switch(ioEfp->location)
  {
  case ENC_LOCATION_GROUND_L:
    num = ENC_MONS_NUM_GROUND_L;
    src = inData->groundLmons;
    break;
  case ENC_LOCATION_GROUND_H:
    num = ENC_MONS_NUM_GROUND_H;
    src = inData->groundHmons;
    break;
  case ENC_LOCATION_GROUND_SP:
    num = ENC_MONS_NUM_GROUND_SP;
    src = inData->groundSmons;
    break;
  case ENC_LOCATION_WATER:
    calctype = ENCPROB_CALCTYPE_WATER;
    num = ENC_MONS_NUM_WATER;
    src = inData->waterMons;
    break;
  case ENC_LOCATION_WATER_SP:
    calctype = ENCPROB_CALCTYPE_WATER;
    num = ENC_MONS_NUM_WATER_SP;
    src = inData->waterSpMons;
    break;
  case ENC_LOCATION_FISHING:
    calctype = ENCPROB_CALCTYPE_FISHING;
    num = ENC_MONS_NUM_FISHING;
    src = inData->fishingMons;
    break;
  case ENC_LOCATION_FISHING_SP:
    calctype = ENCPROB_CALCTYPE_FISHING;
    num = ENC_MONS_NUM_FISHING_SP;
    src = inData->fishingSpMons;
    break;
  }
  ioEfp->prob_calctype = calctype;
  ioEfp->tbl_num = num;
  MI_CpuCopy8(src,outTable,sizeof(ENC_COMMON_DATA)*num);
  return num;
}

/**
 *  @brief  エンカウトポケモンパラメータセット
 */
static void eps_SetPokeParam(const ENC_COMMON_DATA* data,ENC_POKE_PARAM* outPoke)
{
  u8 form_max;
  s16 lv_diff = data->maxLevel - data->minLevel;

  MI_CpuClear8(outPoke,sizeof(ENC_POKE_PARAM));

  if(lv_diff < 0){
    lv_diff = 0;
  }
  outPoke->level = data->minLevel + (eps_GetPercentRand() % (lv_diff+1));

  outPoke->monsNo = data->monsNo;

  form_max = POKETOOL_GetPersonalParam( data->monsNo, 0, POKEPER_ID_form_max );
  if(data->form == ENC_MONS_FORM_RND_CODE ){
    outPoke->form = GFUser_GetPublicRand0(form_max);  //全フォルムからランダム
  }else if(data->form >= form_max){
    outPoke->form = 0;
  }else{
    outPoke->form = data->form;
  }
}

/**
 *  @brief  特性による特定タイプの優先エンカウント
 *
 *  テーブル中から指定したタイプのものだけ選んで抽選
 */
static u8 eps_LotFixTypeEncount(const ENCPOKE_FLD_PARAM* efp,const ENC_COMMON_DATA* enc_data,const u8 type )
{
  int i,num = 0;
  u8  type1,type2;
  u8 idx_tbl[ENC_MONS_NUM_MAX];

  for(i = 0;i < efp->tbl_num;i++){
    if( enc_data[i].form == ENC_MONS_FORM_RND_CODE ){
      continue; //フォルムがランダムコードの場合は特性適用しない
    }
    type1 = POKETOOL_GetPersonalParam(enc_data[i].monsNo, enc_data[i].form, POKEPER_ID_type1 );
    type2 = POKETOOL_GetPersonalParam(enc_data[i].monsNo, enc_data[i].form, POKEPER_ID_type2 );
    if((type1 == type)||(type2 == type)){
      idx_tbl[num++] = i;
    }
  }
  if(num  == 0){
    return 0xFF;
  }
  return idx_tbl[eps_GetPercentRand()%num];
}

/*
 *  @brief  隠し特性によるエンカウントレベル補整
 *
 *  @param  ioMons  [io]エンカウントモンスターパラメータ構造体
 */
static void eps_CheckSpaEncountLevel(const ENCPOKE_FLD_PARAM* efp,const ENC_COMMON_DATA* enc_tbl,ENC_POKE_PARAM* ioPoke )
{
  int i;
  u8  max = ioPoke->level,lv_diff = FALSE;

  if( !efp->spa_high_lv_hit ){
    return; //補整無し
  }
  for(i = 0;i < efp->tbl_num;i++){
    //minとmaxの差があるエンカウントタイプか？
    if( (!lv_diff) && (enc_tbl[i].minLevel != enc_tbl[i].maxLevel)){
      lv_diff = TRUE;
    }
    if(enc_tbl[i].monsNo != ioPoke->monsNo){
      continue;
    }
    if(max < enc_tbl[i].maxLevel){
      max = enc_tbl[i].maxLevel;
    }
  }
  //ビンゴorMinMaxに差があるエンカウントタイプの場合+5level。ただし、テーブル内のmaxlevelは超えない
  if((ioPoke->level + 5) <= max){
    if(efp->enc_type == ENC_TYPE_BINGO || lv_diff == TRUE){
      ioPoke->level += 5;
      return;
    }
  }
  ioPoke->level = max;
}

//--------------------------------------------------------------
/*
 *  @brief  渡されたテーブルからポケモンを一体抽選
 */
//--------------------------------------------------------------
static void eps_PokeLottery( const ENCPOKE_FLD_PARAM* efp, const ENC_COMMON_DATA* enc_tbl, ENC_POKE_PARAM* outPoke )
{
  u8 idx = 0xFF;

  if(efp->spa_hagane_up){ //ハガネタイプ優先エンカウト
    idx = eps_LotFixTypeEncount( efp, enc_tbl, POKETYPE_HAGANE );
  }else if(efp->spa_denki_up){  //でんきタイプ優先エンカウント
    idx = eps_LotFixTypeEncount( efp, enc_tbl, POKETYPE_DENKI );
  }
  if(idx == 0xFF){  //タイプ優先エンカウントに失敗したら指定の確率計算
    idx = (DATA_EncProbCalcFuncTable[efp->prob_calctype])(efp->tbl_num);
  }
  //エンカウントモンスターパラメータセット
  eps_SetPokeParam(&enc_tbl[idx],outPoke);

  //隠し特性によるエンカウントレベル補正
  eps_CheckSpaEncountLevel(efp, enc_tbl, outPoke );
}

//===============================================================================
//サブ　POKEMON_PARAM生成
//===============================================================================

/*
 *  @brief  エンカウントポケモンアイテムセット
 */
static void eps_EncPokeItemSet(POKEMON_PARAM* pp,
    POKEMON_PERSONAL_DATA* personal,ITEM_RANGE item_range,u8 bingo_f,u8 negau_hosei)
{
  int i;
  u16 item[PPD_ITEM_SLOT_NUM];
  u8  rnd,tmp,range[PPD_ITEM_SLOT_NUM];

  for(i = 0;i < 3;i++){
    item[i] = POKE_PERSONAL_GetParam(personal,POKEPER_ID_item1+i);
  }
  if(item[0] == item[1] && item[1] == item[2]){
    PP_Put(pp,ID_PARA_item,item[0]);  //3スロットが全部同じアイテムなら必ず持たせる
    return;
  }
  MI_CpuCopy8(range,DATA_ItemRangeTable[item_range+bingo_f*2],sizeof(u8)*PPD_ITEM_SLOT_NUM);

  //ビンゴならレンジ補正
  if(bingo_f){
    //@todo
    range[0] -= negau_hosei;
    range[2] += negau_hosei;
  }
  rnd = eps_GetPercentRand();

  tmp = 0;
  for(i = 0;i < PPD_ITEM_SLOT_NUM;i++){
    tmp += range[i];
    if( rnd < tmp ){
      PP_Put(pp,ID_PARA_item,item[i]);
      break;
    }
  }
}

/*
 *  @brief  個性乱数計算
 *
 *  @todo
 */
static u32 eps_EncPokeCalcPersonalRand(
    const ENCPOKE_FLD_PARAM* efp, const POKEMON_PERSONAL_DATA*  personal,const ENC_POKE_PARAM* poke )
{
  u32 p_rnd;
  u8 sex_vec,sex;

  sex = PTL_SEX_UNKNOWN;
  sex_vec = POKE_PERSONAL_GetParam( (POKEMON_PERSONAL_DATA*)personal, POKEPER_ID_sex );
  if( PokePersonal_SexVecTypeGet( sex_vec ) != POKEPER_SEXTYPE_FIX &&
      efp->spa_sex_fix)
  {
    if(efp->mons_sex == PTL_SEX_MALE){
      sex = PTL_SEX_FEMALE;
    }else if(efp->mons_sex == PTL_SEX_FEMALE){
      sex = PTL_SEX_MALE;
    }
  }

  do{
    if(poke->rare){
      p_rnd = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
      break;
    }else{
      p_rnd = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
    }
    IWASAWA_Printf("PP_Rnd = %08x\n",p_rnd);

    if( sex != PTL_SEX_UNKNOWN )
    {
      if( POKETOOL_GetSex( poke->monsNo, poke->form, p_rnd) == sex ){
        break;  //性別指定一致
      }
    }else
    {
      break;  //指定無し
    }
  }while(1);

  return p_rnd;
}

//===============================================================================
//サブ　確率計算
//===============================================================================
//--------------------------------------------------------------
/**
 * ランダム確率取得
 * @retval  u32 確率
 */
//--------------------------------------------------------------
static u32 eps_GetPercentRand( void )
{
  u32 per = (0xffff/100) + 1;
  u32 val = GFUser_GetPublicRand(0xffff) / per;
  return( val );
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（草むら/砂漠/洞窟）
 * @param nothing
 * @return u8 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 eps_GetPercentRandGroundPokeTblNo( u32 max )
{
  u32 i;

  i = eps_GetPercentRand();
  if( i < 20 )      return  0;    // 20%
  if( i >= 20 && i < 40 ) return  1;    // 20%
  if( i >= 40 && i < 50 ) return  2;    // 10%
  if( i >= 50 && i < 60 ) return  3;    // 10%
  if( i >= 60 && i < 70 ) return  4;    // 10%
  if( i >= 70 && i < 80 ) return  5;    // 10%
  if( i >= 80 && i < 85 ) return  6;    //  5%
  if( i >= 85 && i < 90 ) return  7;    //  5%
  if( i >= 90 && i < 94 ) return  8;    //  4%
  if( i >= 94 && i < 98 ) return  9;    //  4%
  if( i == 98 )     return  10;   //  1%
  return  11;               //  1%
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（水上）
 * @param nothing
 * @return u8 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 eps_GetPercentRandWaterPokeTblNo( u32 max )
{
  u32 i;

  i = eps_GetPercentRand();

  if( i < 60 )      return  0;    // 60%
  if( i >= 60 && i < 90 ) return  1;    // 30%
  if( i >= 90 && i < 95 ) return  2;    //  5%
  if( i >= 95 && i < 99 ) return  3;    //  4%
  return  4;                //  1%
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（釣り）
 * @param inFishingRod    釣竿
 * @return u8 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 eps_GetPercentRandFishingPokeTblNo( u32 max )
{
  u32 i;

  i = eps_GetPercentRand();

  if    ( i < 40 )    return 0; // 40%
  else if ( i < 80 )  return 1; // 40%
  else if ( i < 95 )  return 2; // 15%
  else if ( i < 99 )  return 3; // 4%
  return 4; // 1%
}

//--------------------------------------------------------------
/**
 * ランダムポケモンセット（1/指定数）
 * @return u32 エンカウントデータのテーブル番号
 */
//--------------------------------------------------------------
static u32 eps_GetPercentRandNumPokeTblNo( u32 max )
{
  return eps_GetPercentRand() % max;
}

