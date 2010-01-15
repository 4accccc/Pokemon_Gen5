//============================================================================================
/**
 * @file  waza_tool.c
 * @bfief 技パラメータツール群（WB改訂版）
 * @author  HisashiSogabe,  Masao Taya
 * @date  08.11.13
 */
//============================================================================================
#include  <gflib.h>

#include  "arc_def.h"

#include  "waza_tool/wazano_def.h"
#include  "waza_tool/wazadata.h"

#include  "waza_tool_def.h"

/*------------------------------------------------------------------------------------*/
/*  consts                                                                            */
/*------------------------------------------------------------------------------------*/
enum {
  RANK_STORE_MAX = 3,
};
/*------------------------------------------------------------------------------------*/
/*  structures                                                                        */
/*------------------------------------------------------------------------------------*/
struct _WAZA_DATA {

  u8  type;         ///< タイプ（みず、くさ、ほのお…）
  u8  category;     ///< カテゴリ（ enum WazaCategory )
  u8  damageType;   ///< ダメージタイプ（ enum WazaDamageType )
  u8  power;        ///< 威力
  u8  hitPer;       ///< 命中率
  u8  basePP;       ///< 基本PPMax
  s8  priority;     ///< ワザ優先度
  u8  hitCountMin : 4;  ///< 最小ヒット回数
  u8  hitCountMax : 4;  ///< 最大ヒット回数
  u16 sickID;       ///< 状態異常コード（enum WazaSick）
  u8  sickPer;      ///< 状態異常の発生率
  u8  sickCont;     ///< 状態異常継続パターン（enum WazaSickCont）
  u8  sickTurnMin;  ///< 状態異常の継続ターン最小
  u8  sickTurnMax;  ///< 状態異常の継続ターン最大
  u8  criticalRank; ///< クリティカルランク
  u8  shrinkPer;    ///< ひるみ確率
  u8  rankEffType[WAZA_RANKEFF_NUM_MAX]; ///< ステータスランク効果
  s8  rankEffValue[WAZA_RANKEFF_NUM_MAX];///< ステータスランク増減値
  u8  rankEffPer[WAZA_RANKEFF_NUM_MAX];  ///< ステータスランク発生率
  s8  damageRecoverRatio; ///< ダメージ回復率
  s8  HPRecoverRatio;     ///< HP回復率
  u8  target;       ///< ワザ効果範囲( enum WazaTarget )
  u32 flags;        ///< 各種フラグ

};

/*------------------------------------------------------------------------------------*/
/*  consts                                                                            */
/*------------------------------------------------------------------------------------*/
enum {
  HITRATIO_MUST = 101,    ///< hitPer（命中率）がこの値なら、必中ワザ
  CRITICAL_MUST = 6,      ///< criticalRank がこの値なら、必ずクリティカル
};

/*------------------------------------------------------------------------------------*/
/*  globals                                                                           */
/*------------------------------------------------------------------------------------*/

/**
 *  簡易版パラメータ取得処理で、ワザパラメータを読み込むために利用するテンポラリワーク
 */
static WAZA_DATA  gWazaData = {0};

static inline WAZA_DATA* loadWazaDataTmp( WazaID id )
{
  GFL_ARC_LoadData( &gWazaData, ARCID_WAZA_TBL, id );
  return &gWazaData;
}

//=============================================================================================
/**
 * ワザパラメータ取得（簡易版）
 *
 * @param   id
 * @param   param
 *
 * @retval  int
 */
//=============================================================================================
int WAZADATA_GetParam( WazaID wazaID, WazaDataParam param )
{
  GF_ASSERT(wazaID < WAZANO_MAX);
  GF_ASSERT(wazaID != WAZANO_NULL);
  GF_ASSERT(param < WAZAPARAM_MAX);

  {
    WAZA_DATA* wp = loadWazaDataTmp( wazaID );
    int result = WAZADATA_PTR_GetParam( wp, param );
    return result;
  }
}
//=============================================================================================
/**
 * ワザパラメータ取得（ポインタ版）
 *
 * @param   id
 * @param   param
 *
 * @retval  int
 */
//=============================================================================================
int WAZADATA_PTR_GetParam( const WAZA_DATA* wazaData, WazaDataParam param )
{
  switch( param ){
  case WAZAPARAM_TYPE:                ///< タイプ（みず、くさ、ほのお…）
    return wazaData->type;
  case WAZAPARAM_CATEGORY:            ///< カテゴリ（ enum WazaCategory )
    return wazaData->category;
  case WAZAPARAM_DAMAGE_TYPE:         ///< ダメージタイプ（ enum WazaDamageType )
    return wazaData->damageType;
  case WAZAPARAM_POWER:               ///< 威力
    return wazaData->power;
  case WAZAPARAM_HITPER:              ///< 命中率
    if( wazaData->hitPer != HITRATIO_MUST ){
      return wazaData->hitPer;
    }else{
      return 100;
    }
  case WAZAPARAM_BASE_PP:             ///< 基本PPMax
    return wazaData->basePP;
  case WAZAPARAM_PRIORITY:            ///< ワザ優先度
    return wazaData->priority;
  case WAZAPARAM_CRITICAL_RANK:       ///< クリティカルランク
    if( wazaData->criticalRank != CRITICAL_MUST ){
      return wazaData->criticalRank;
    }else{
      return 0;
    }
  case WAZAPARAM_HITCOUNT_MAX:        ///< 最大ヒット回数
    return wazaData->hitCountMax;
  case WAZAPARAM_HITCOUNT_MIN:        ///< 最小ヒット回数
    return wazaData->hitCountMin;
  case WAZAPARAM_SHRINK_PER:          ///< ひるみ確率
    return wazaData->shrinkPer;
  case WAZAPARAM_SICK:                ///< 状態異常コード（enum WazaSick）
    return wazaData->sickID;
  case WAZAPARAM_SICK_PER:            ///< 状態異常の発生率
    return wazaData->sickPer;
  case WAZAPARAM_SICK_CONT:           ///< 状態異常継続パターン（enum WazaSickCont）
    return wazaData->sickCont;
  case WAZAPARAM_SICK_TURN_MIN:       ///< 状態異常の継続ターン最小
    return wazaData->sickTurnMin;
  case WAZAPARAM_SICK_TURN_MAX:       ///< 状態異常の継続ターン最大
    return wazaData->sickTurnMax;

  case WAZAPARAM_RANKTYPE_1:          ///< ステータスランク効果１
  case WAZAPARAM_RANKTYPE_2:          ///< ステータスランク効果２
  case WAZAPARAM_RANKTYPE_3:          ///< ステータスランク効果３
    return wazaData->rankEffType[ param - WAZAPARAM_RANKTYPE_1 ];

  case WAZAPARAM_RANKVALUE_1:         ///< ステータスランク効果１増減値
  case WAZAPARAM_RANKVALUE_2:         ///< ステータスランク効果２増減値
  case WAZAPARAM_RANKVALUE_3:         ///< ステータスランク効果３増減値
    return wazaData->rankEffValue[ param - WAZAPARAM_RANKVALUE_1 ];

  case WAZAPARAM_RANKPER_1:           ///< ステータスランク効果１の発生率
  case WAZAPARAM_RANKPER_2:           ///< ステータスランク効果２の発生率
  case WAZAPARAM_RANKPER_3:           ///< ステータスランク効果３の発生率
    return wazaData->rankEffPer[ param - WAZAPARAM_RANKPER_1 ];


  case WAZAPARAM_DAMAGE_RECOVER_RATIO:///< ダメージ回復率
    if( wazaData->damageRecoverRatio > 0 ){
      return wazaData->damageRecoverRatio;
    }else{
      return 0;
    }
  case WAZAPARAM_DAMAGE_REACTION_RATIO:
    if( wazaData->damageRecoverRatio < 0 ){
      return wazaData->damageRecoverRatio * -1;
    }else{
      return 0;
    }
  case WAZAPARAM_HP_RECOVER_RATIO:    ///< HP回復率
    if( wazaData->HPRecoverRatio > 0 ){
      return wazaData->HPRecoverRatio;
    }else{
      return 0;
    }
  case WAZAPARAM_HP_REACTION_RATIO:
    if( wazaData->HPRecoverRatio < 0 ){
      return wazaData->HPRecoverRatio * -1;
    }else{
      return 0;
    }
  case WAZAPARAM_TARGET:             ///< ワザ効果範囲( enum WazaTarget )

    return wazaData->target;

  default:
    GF_ASSERT(0);
    break;
  }
  return 0;
}
//=============================================================================================
/**
 * ワザフラグ取得（簡易版）
 *
 * @param   id
 * @param   flag
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL WAZADATA_GetFlag( WazaID wazaID, WazaFlag flag )
{
  GF_ASSERT(wazaID<WAZANO_MAX);
  {
    WAZA_DATA* wp = loadWazaDataTmp( wazaID );
    return WAZADATA_PTR_GetFlag( wp, flag );
  }
}
//=============================================================================================
/**
 * ワザフラグ取得（ポインタ版）
 *
 * @param   wazaData
 * @param   flag
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL WAZADATA_PTR_GetFlag( const WAZA_DATA* wazaData, WazaFlag flag )
{
  GF_ASSERT(flag<WAZAFLAG_MAX);
  TAYA_Printf("FlagID=%d, bits=%08x\n", flag, wazaData->flags);
  return (wazaData->flags & (1<<flag)) != 0;
}

//=============================================================================================
/**
 * ワザによって発生する状態異常の継続パラメータを取得
 *
 * @param   id
 *
 * @retval  WAZA_SICKCONT_PARAM
 */
//=============================================================================================
WAZA_SICKCONT_PARAM WAZADATA_GetSickCont( WazaID id )
{
  WAZA_DATA* wp = loadWazaDataTmp( id );

  WAZA_SICKCONT_PARAM  param;

  if( wp->sickCont )
  {
    param.type = wp->sickCont;
    param.turnMin = wp->sickTurnMin;
    param.turnMax = wp->sickTurnMax;
  }
  return param;
}
//=============================================================================================
/**
 * ランク効果の種類数を返す
 *
 * @param   id
 *
 * @retval  u8
 */
//=============================================================================================
u8 WAZADATA_GetRankEffectCount( WazaID id )
{
  WAZA_DATA* wp = loadWazaDataTmp( id );
  u32 i, cnt = 0;
  for(i=0; i<RANK_STORE_MAX; ++i){
    if( wp->rankEffType[i] != WAZA_RANKEFF_NULL ){
      ++cnt;
    }
  }
  return cnt;
}
//=============================================================================================
/**
 * ランク効果を取得
 *
 * @param   id        [in] ワザID
 * @param   idx       [in] 何番目のランク効果か？（0～）
 * @param   volume    [out] ランク効果の程度（+ならアップ, -ならダウン, 戻り値==WAZA_RANKEFF_NULLの時のみ0）
 *
 * @retval  WazaRankEffect    ランク効果ID
 */
//=============================================================================================
WazaRankEffect WAZADATA_GetRankEffect( WazaID id, u32 idx, int* volume )
{
  GF_ASSERT(idx < WAZA_RANKEFF_NUM_MAX);
  {
    WAZA_DATA* wp = loadWazaDataTmp( id );
    WazaRankEffect  eff;

    eff = wp->rankEffType[ idx ];
    if( eff != WAZA_RANKEFF_NULL ){
      *volume = wp->rankEffValue[ idx ];
    }
    else{
      *volume = 0;
    }
    return eff;
  }
}
//============================================================================================
/**
 *  PPMaxを取得
 *
 * @param[in] id        取得する技ナンバー
 * @param[in] maxupcnt  マックスアップを使用した回数
 *
 * @retval  u8    PPMax
 */
//============================================================================================
u8 WAZADATA_GetMaxPP( WazaID id, u8 maxupcnt )
{
  u8  ppmax;

  if( maxupcnt > 3 ){
    maxupcnt = 3;
  }

  ppmax = WAZADATA_GetParam( id, WAZAPARAM_BASE_PP );
  ppmax += ( ppmax * 20 * maxupcnt ) / 100;

  return  ppmax;
}
//=============================================================================================
/**
 * 天候効果を返す
 *
 * @param   id
 *
 * @retval  BtlWeather
 */
//=============================================================================================
WazaWeather WAZADATA_GetWeather( WazaID id )
{
  switch( id ){
  case WAZANO_AMAGOI:     return WAZA_WEATHER_RAIN;
  case WAZANO_NIHONBARE:  return WAZA_WEATHER_SHINE;
  case WAZANO_SUNAARASI:  return WAZA_WEATHER_SAND;
  case WAZANO_ARARE:      return WAZA_WEATHER_SNOW;

  default:
    return WAZA_WEATHER_NONE;
  }
}
//------------------------------------------------
/**
 *  ワザカテゴリ取得
 */
//------------------------------------------------
WazaCategory  WAZADATA_GetCategory( WazaID id )
{
  return WAZADATA_GetParam( id, WAZAPARAM_CATEGORY );
}
//------------------------------------------------
/**
 *  ワザタイプ取得
 */
//------------------------------------------------
PokeType WAZADATA_GetType( WazaID id )
{
  return WAZADATA_GetParam( id, WAZAPARAM_TYPE );
}
//------------------------------------------------
/**
 *  ワザ威力値取得
 */
//------------------------------------------------
u16 WAZADATA_GetPower( WazaID id )
{
  return WAZADATA_GetParam( id, WAZAPARAM_POWER );
}
//------------------------------------------------
/**
 *  ワザダメージ分類取得
 */
//------------------------------------------------
WazaDamageType WAZADATA_GetDamageType( WazaID id )
{
  return WAZADATA_GetParam( id, WAZAPARAM_DAMAGE_TYPE );
}
//------------------------------------------------
/**
 *  必中フラグ取得（つばめがえしなどがTRUEを返す）
 */
//------------------------------------------------
BOOL WAZADATA_IsAlwaysHit( WazaID id )
{
  WAZA_DATA* wp = loadWazaDataTmp( id );
  return (wp->hitPer == HITRATIO_MUST);
}
//------------------------------------------------
/**
 *  必クリティカルフラグ取得
 */
//------------------------------------------------
BOOL WAZADATA_IsMustCritical( WazaID id )
{
  WAZA_DATA* wp = loadWazaDataTmp( id );
  return (wp->criticalRank == CRITICAL_MUST);
}
//------------------------------------------------
/**
 *  ダメージワザかどうか判定
 */
//------------------------------------------------
BOOL WAZADATA_IsDamage( WazaID id )
{
  return WAZADATA_GetParam(id, WAZAPARAM_POWER) != 0;
}


