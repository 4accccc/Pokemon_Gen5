//=============================================================================================
/**
 * @file  btl_pokeparam.c
 * @brief ポケモンWB バトルシステム  参加ポケモン戦闘用データ
 * @author  taya
 *
 * @date  2008.10.08  作成
 */
//=============================================================================================
#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "waza_tool/wazadata.h"
#include "item/itemsym.h"

#include "btl_common.h"
#include "btl_calc.h"
#include "btl_sick.h"

#include "btl_pokeparam.h"



/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {

  RANK_CRITICAL_MIN = 0,
  RANK_CRITICAL_MAX = 4,
  RANK_CRITICAL_DEF = 0,

  TURNFLG_BUF_SIZE = (BPP_TURNFLG_MAX/8)+(BPP_TURNFLG_MAX%8!=0),
  CONTFLG_BUF_SIZE = (BPP_CONTFLG_MAX/8)+(BPP_CONTFLG_MAX%8!=0),

  TURNCOUNT_NULL = BTL_TURNCOUNT_MAX+1,

  WAZADMG_REC_TURN_MAX = BPP_WAZADMG_REC_TURN_MAX,  ///< ワザダメージレコード：何ターン分の記録を取るか？
  WAZADMG_REC_MAX = BPP_WAZADMG_REC_MAX,            ///< ワザダメージレコード：１ターンにつき、何件分まで記録するか？
};


//--------------------------------------------------------------
/**
 *  コアパラメータ  - へんしんしても書き換わらない要素 -
 */
//--------------------------------------------------------------
typedef struct {
  const POKEMON_PARAM*  ppSrc;
  const POKEMON_PARAM*  ppFake;
  u32   exp;
  u16   monsno;
  u16   hp;
  u16   item;
  u16   usedItem;
  u8    myID;
  u8    fHensin;
  u8    fFakeEnable;
}BPP_CORE_PARAM;

//--------------------------------------------------------------
/**
 *  基本パラメータ
 */
//--------------------------------------------------------------
typedef struct {

  u16 monsno;       ///< ポケモンナンバー
  u16 hpMax;        ///< 最大HP

  u16 attack;       ///< こうげき
  u16 defence;      ///< ぼうぎょ
  u16 sp_attack;    ///< とくこう
  u16 sp_defence;   ///< とくぼう
  u16 agility;      ///< すばやさ

  u8  level;        ///< レベル
  u8  type1;        ///< タイプ１
  u8  type2;        ///< タイプ２
  u8  sex;          ///< 性別

}BPP_BASE_PARAM;

//--------------------------------------------------------------
/**
 *  ランク効果
 */
//--------------------------------------------------------------
typedef struct {

  s8  attack;     ///< こうげき
  s8  defence;    ///< ぼうぎょ
  s8  sp_attack;  ///< とくこう
  s8  sp_defence; ///< とくぼう
  s8  agility;    ///< すばやさ
  s8  hit;        ///< 命中率
  s8  avoid;      ///< 回避率

}BPP_VARIABLE_PARAM;

//--------------------------------------------------------------
/**
 *  ワザ
 */
//--------------------------------------------------------------
typedef struct {
  u16  number;            ///< ワザナンバー
  u16  recoverNumber;     ///< バトル中にワザが書き換わった時に、巻き戻し用に前のワザを保存
  u8   pp;                ///< PP値
  u8   ppMax;             ///< PP最大値
  u8   usedFlag;          ///< 使用したフラグ
}BPP_WAZA;


struct _BTL_POKEPARAM {

  BPP_CORE_PARAM      coreParam;

  BPP_BASE_PARAM      baseParam;
  BPP_VARIABLE_PARAM  varyParam;
  BPP_WAZA            waza[ PTL_WAZA_MAX ];

  u16  tokusei;
  u16  weight;

  BPP_SICK_CONT   sickCont[ WAZASICK_MAX ];
  u8  wazaSickCounter[ WAZASICK_MAX ];
  u8  wazaCnt;
  u8  formNo;
  u8  criticalRank;

  u16 turnCount;        ///< 継続して戦闘に出ているカウンタ
  u16 appearedTurn;     ///< 戦闘に出たターンを記録
  u16 prevSelectWazaID;       ///< 直前に選択したワザID（WAZALOCK状態等で強制的に選択されたワザ > クライアントが選択した所持ワザ）
  u16 prevActWazaID;          ///< 直前に実行されたワザID（派生ワザを使った場合にprevSelectWazaIDと異なる）
  u16 wazaContCounter;        ///< 同ワザ連続成功カウンタ（prevActWazaIDのワザを何回連続で成功させたか。直前失敗ならゼロ）
  BtlPokePos  prevTargetPos;  ///< 直前に狙った相手

  u8  turnFlag[ TURNFLG_BUF_SIZE ];
  u8  contFlag[ CONTFLG_BUF_SIZE ];
  u8  counter[ BPP_COUNTER_MAX ];
  BPP_WAZADMG_REC  wazaDamageRec[ WAZADMG_REC_TURN_MAX ][ WAZADMG_REC_MAX ];
  u8               dmgrecCount[ WAZADMG_REC_TURN_MAX ];
  u8               dmgrecTurnPtr;
  u8               dmgrecPtr;

  u8  confrontRecCount;
  u8  confrontRec[ BTL_POKEID_MAX ];

  u16 migawariHP;
//  u32 dmy;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void setupBySrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP );
static void clearUsedWazaFlag( BTL_POKEPARAM* bpp );
static void clearCounter( BTL_POKEPARAM* bpp );
static void Effrank_Init( BPP_VARIABLE_PARAM* rank );
static void Effrank_Reset( BPP_VARIABLE_PARAM* rank );
static BOOL Effrank_Recover( BPP_VARIABLE_PARAM* rank );
static BppValueID ConvertValueID( const BTL_POKEPARAM* bpp, BppValueID vid );
static const s8* getRankVaryStatusConst( const BTL_POKEPARAM* pp, BppValueID type, s8* min, s8* max );
static s8* getRankVaryStatus( BTL_POKEPARAM* pp, BppValueID type, s8* min, s8* max );
static void cureDependSick( BTL_POKEPARAM* bpp, WazaSick sickID );
static void clearWazaSickWork( BTL_POKEPARAM* bpp, BOOL fPokeSickInclude );
static void dmgrecClearWork( BTL_POKEPARAM* bpp );
static void dmgrecFwdTurn( BTL_POKEPARAM* bpp );
static inline void flgbuf_clear( u8* buf, u32 size );
static inline void flgbuf_set( u8* buf, u32 flagID );
static inline void flgbuf_reset( u8* buf, u32 flagID );
static inline BOOL flgbuf_get( const u8* buf, u32 flagID );
static void ConfrontRec_Clear( BTL_POKEPARAM* bpp );
static inline BOOL IsMatchTokusei( const BTL_POKEPARAM* bpp, PokeTokusei tokusei );
static inline BOOL IsMatchItem( const BTL_POKEPARAM* bpp, u16 itemID );



//=============================================================================================
/**
 * バトル用ポケモンパラメータ生成
 *
 * @param   pp      ゲーム内汎用ポケモンパラメータ
 * @param   pokeID    バトル参加ポケID（バトルに参加しているポケモンに割り振る一意の数値）
 * @param   heapID
 *
 * @retval  BTL_POKEPARAM*
 */
//=============================================================================================
BTL_POKEPARAM*  BTL_POKEPARAM_Create( const POKEMON_PARAM* pp, u8 pokeID, HEAPID heapID )
{
  BTL_POKEPARAM* bpp = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_POKEPARAM) );

  bpp->coreParam.myID = pokeID;
  bpp->coreParam.monsno = PP_Get( pp, ID_PARA_monsno, 0 );
  bpp->coreParam.ppSrc = pp;
  bpp->coreParam.hp = PP_Get( pp, ID_PARA_hp, 0 );
  bpp->coreParam.item = PP_Get( pp, ID_PARA_item, NULL );
  bpp->coreParam.usedItem = ITEM_DUMMY_DATA;
  bpp->coreParam.fHensin = FALSE;
  bpp->coreParam.ppFake = NULL;
  bpp->coreParam.fFakeEnable = NULL;

  setupBySrcData( bpp, pp );

  BTL_Printf("setup pokeID=%d, monsno=%d, ppSrc=%p\n", pokeID, bpp->coreParam.monsno, pp );

  // ランク効果初期化
  Effrank_Init( &bpp->varyParam );

  // 状態異常ワーク初期化
  clearWazaSickWork( bpp, TRUE );

  // この時点でポケモン用状態異常になっていれば引き継ぎ
  {
    PokeSick sick = PP_GetSick( pp );
    if( sick == POKESICK_NEMURI )
    {
      u32 turns = BTL_CALC_RandRange( BTL_NEMURI_TURN_MIN, BTL_NEMURI_TURN_MAX );
      bpp->sickCont[WAZASICK_NEMURI] = BPP_SICKCONT_MakeTurn( turns );
      bpp->wazaSickCounter[WAZASICK_NEMURI] = 0;
    }
  }

  // 各種ワーク領域初期化
  bpp->appearedTurn = TURNCOUNT_NULL;
  bpp->turnCount = 0;
  bpp->migawariHP = 0;
  bpp->prevSelectWazaID = WAZANO_NULL;
  bpp->prevActWazaID    = WAZANO_NULL;
  bpp->wazaContCounter   = 0;

  flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );
  flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

  dmgrecClearWork( bpp );
  ConfrontRec_Clear( bpp );

  return bpp;
}
//----------------------------------------------------------------------------------
/**
 * 元になるポケモンパラメータ構造体から必要部分を構築
 *
 * @param   bpp
 * @param   srcPP
 */
//----------------------------------------------------------------------------------
static void setupBySrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP )
{
  u16 monsno = bpp->coreParam.monsno;
  int i;

  // 基本パラメタ初期化
  bpp->baseParam.type1 = PP_Get( srcPP, ID_PARA_type1, 0 );
  bpp->baseParam.type2 = PP_Get( srcPP, ID_PARA_type2, 0 );
  bpp->baseParam.sex = PP_GetSex( srcPP );
  bpp->baseParam.level = PP_Get( srcPP, ID_PARA_level, 0 );
  bpp->baseParam.hpMax = PP_Get( srcPP, ID_PARA_hpmax, 0 );
  bpp->baseParam.attack = PP_Get( srcPP, ID_PARA_pow, 0 );
  bpp->baseParam.defence = PP_Get( srcPP, ID_PARA_def, 0 );
  bpp->baseParam.sp_attack = PP_Get( srcPP, ID_PARA_spepow, 0 );
  bpp->baseParam.sp_defence = PP_Get( srcPP, ID_PARA_spedef, 0 );
  bpp->baseParam.agility = PP_Get( srcPP, ID_PARA_agi, 0 );


  // 所有ワザデータ初期化
  bpp->wazaCnt = 0;
  for(i=0; i<PTL_WAZA_MAX; i++)
  {
    bpp->waza[i].number = PP_Get( srcPP, ID_PARA_waza1+i, 0 );
    if( bpp->waza[i].number != WAZANO_NULL )
    {
      bpp->waza[i].pp = PP_Get( srcPP, ID_PARA_pp1+i, 0 );
      bpp->waza[i].ppMax = PP_Get( srcPP, ID_PARA_pp_max1+i, 0 );
      bpp->wazaCnt++;
    }
    else
    {
      bpp->waza[i].pp = 0;
      bpp->waza[i].ppMax = 0;
    }
    bpp->waza[i].usedFlag = FALSE;
    bpp->waza[i].recoverNumber = bpp->waza[i].number;
  }

  bpp->tokusei = PP_Get( srcPP, ID_PARA_speabino, 0 );
  bpp->formNo = PP_Get( srcPP, ID_PARA_form_no, 0 );
  bpp->coreParam.exp = PP_Get( srcPP, ID_PARA_exp, NULL );
  bpp->weight = POKETOOL_GetPersonalParam( bpp->coreParam.monsno, bpp->formNo, POKEPER_ID_weight ) / 10;
  if( bpp->weight < BTL_POKE_WEIGHT_MIN ){
    bpp->weight = BTL_POKE_WEIGHT_MIN;
  }
}
//----------------------------------------------------------------------------------
/**
 * ワザ使用フラグのクリア
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void clearUsedWazaFlag( BTL_POKEPARAM* bpp )
{
  u32 i;
  for(i=0; i<NELEMS(bpp->waza); ++i){
    bpp->waza[i].usedFlag = FALSE;
  }

  bpp->prevSelectWazaID = WAZANO_NULL;
  bpp->prevActWazaID    = WAZANO_NULL;
  bpp->wazaContCounter  = 0;
}
//----------------------------------------------------------------------------------
/**
 * 各種カウンタ値をクリア
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void clearCounter( BTL_POKEPARAM* bpp )
{
  u32 i;
  for(i=0; i<NELEMS(bpp->counter); ++i){
    bpp->counter[i] = 0;
  }
}

//=============================================================================================
/**
 * バトル用ポケモンパラメータ削除
 *
 * @param   bpp
 *
 */
//=============================================================================================
void BTL_POKEPARAM_Delete( BTL_POKEPARAM* bpp )
{
  GFL_HEAP_FreeMemory( bpp );
}

//=============================================================================================
/**
 * データコピー
 *
 * @param   dst
 * @param   src
 *
 */
//=============================================================================================
void BTL_POKEPARAM_Copy( BTL_POKEPARAM* dst, const BTL_POKEPARAM* src )
{
  *dst = *src;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

static void Effrank_Init( BPP_VARIABLE_PARAM* rank )
{
  Effrank_Reset( rank );
}
static void Effrank_Reset( BPP_VARIABLE_PARAM* rank )
{
  rank->attack = BPP_RANK_STATUS_DEFAULT;
  rank->defence = BPP_RANK_STATUS_DEFAULT;
  rank->sp_attack = BPP_RANK_STATUS_DEFAULT;
  rank->sp_defence = BPP_RANK_STATUS_DEFAULT;
  rank->agility = BPP_RANK_STATUS_DEFAULT;
  rank->hit = BPP_RANK_STATUS_DEFAULT;
  rank->avoid = BPP_RANK_STATUS_DEFAULT;
}
static BOOL Effrank_Recover( BPP_VARIABLE_PARAM* rank )
{
  BOOL fEffective = FALSE;

  if( rank->attack < BPP_RANK_STATUS_DEFAULT ){
    rank->attack = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  };
  if( rank->defence < BPP_RANK_STATUS_DEFAULT ){
    rank->defence = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }
  if( rank->sp_attack < BPP_RANK_STATUS_DEFAULT ){
    rank->sp_attack = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }
  if( rank->sp_defence < BPP_RANK_STATUS_DEFAULT ){
    rank->sp_defence = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }
  if( rank->agility < BPP_RANK_STATUS_DEFAULT ){
    rank->agility = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }
  if( rank->hit < BPP_RANK_STATUS_DEFAULT ){
    rank->hit = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }
  if( rank->avoid < BPP_RANK_STATUS_DEFAULT ){
    rank->avoid = BPP_RANK_STATUS_DEFAULT;
    fEffective = TRUE;
  }

  return fEffective;
}



//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

u8 BPP_GetID( const BTL_POKEPARAM* pp )
{
  return pp->coreParam.myID;
}

u16 BPP_GetMonsNo( const BTL_POKEPARAM* pp )
{
  return pp->coreParam.monsno;
}

u8 BPP_WAZA_GetCount( const BTL_POKEPARAM* pp )
{
  return pp->wazaCnt;
}

u8 BPP_WAZA_GetUsedCount( const BTL_POKEPARAM* pp )
{
  u8 cnt, i;
  for(i=0, cnt=0; i<pp->wazaCnt; ++i)
  {
    if( pp->waza[i].usedFlag ){
      ++cnt;
    }
  }
  return cnt;
}

WazaID BPP_WAZA_GetID( const BTL_POKEPARAM* bpp, u8 idx )
{
  GF_ASSERT(idx < bpp->wazaCnt);
  return bpp->waza[idx].number;
}

//=============================================================================================
/**
 * [ワザパラメータ] ID, 現PP, 最大PPを一度に取得
 *
 * @param   bpp
 * @param   idx     対象ワザインデックス
 * @param   PP      [out] 現PPを取得する変数アドレス
 * @param   PPMax   [out] 最大PPを取得する変数アドレス
 *
 * @retval  WazaID  ワザID
 */
//=============================================================================================
WazaID BPP_WAZA_GetParticular( const BTL_POKEPARAM* bpp, u8 idx, u8* PP, u8* PPMax )
{
  GF_ASSERT(idx < bpp->wazaCnt);
  *PP = bpp->waza[idx].pp;
  *PPMax = bpp->waza[idx].ppMax;

  return bpp->waza[idx].number;
}

//=============================================================================================
/**
 * [ワザパラメータ] PP不足分を取得
 *
 * @param   bpp
 * @param   idx       対象ワザインデックス
 *
 * @retval  u8        PP不足分
 */
//=============================================================================================
u8 BPP_WAZA_GetPPShort( const BTL_POKEPARAM* bpp, u8 idx )
{
  GF_ASSERT(idx < bpp->wazaCnt);
  return (bpp->waza[idx].ppMax - bpp->waza[idx].pp);
}

static void splitTypeCore( const BTL_POKEPARAM* bpp, PokeType* type1, PokeType* type2 )
{
  BOOL fHaneYasume = BPP_CheckSick( bpp, WAZASICK_HANEYASUME );

  if( (bpp->baseParam.type1 == POKETYPE_HIKOU) && (fHaneYasume) ){
    *type1 = POKETYPE_NULL;
  }else{
    *type1 = bpp->baseParam.type1;
  }

  if( (bpp->baseParam.type2 == POKETYPE_HIKOU) && (fHaneYasume) ){
    *type2 = POKETYPE_NULL;
  }else{
    *type2 = bpp->baseParam.type2;
  }
}

PokeTypePair BPP_GetPokeType( const BTL_POKEPARAM* bpp )
{
  PokeTypePair  typePair;
  PokeType  type1, type2;

  splitTypeCore( bpp, &type1, &type2 );
  typePair = PokeTypePair_Make( type1, type2 );

  return typePair;
}

//=============================================================================================
/**
 * ポケモンが持つ２タイプに、指定タイプが含まれているか判定
 *
 * @param   bpp
 * @param   type
 *
 * @retval  BOOL    含まれていればTRUE
 */
//=============================================================================================
BOOL BPP_IsMatchType( const BTL_POKEPARAM* bpp, PokeType type )
{
  if( type != POKETYPE_NULL )
  {
    PokeType  type1, type2;
    splitTypeCore( bpp, &type1, &type2 );
    if( (type1 == type) || (type2 == type) ){
      return TRUE;
    }
  }
  return FALSE;
}

const POKEMON_PARAM* BPP_GetSrcData( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.ppSrc;
}
void BPP_SetViewSrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* fakePP )
{
  bpp->coreParam.ppFake =  fakePP;
  bpp->coreParam.fFakeEnable = TRUE;
}
const POKEMON_PARAM* BPP_GetViewSrcData( const BTL_POKEPARAM* bpp )
{
  if( (bpp->coreParam.ppFake != NULL)
  &&  (bpp->coreParam.fFakeEnable)
  ){
    return bpp->coreParam.ppFake;
  }
  return bpp->coreParam.ppSrc;
}

//----------------------------------------------------------------------------------
/**
 * 状態異常等の条件により、パラメータ取得用のIDを変更する
 * ※現状、ワンダールーム（全ポケモンの防御・特防を入れ替える）にのみ利用している。
 *
 * @param   bpp
 * @param   BppValueID    パラメータ取得用にリクエストされたパラメータID
 *
 * @retval  BppValueID    実際に適用するパラメータID
 */
//----------------------------------------------------------------------------------
static BppValueID ConvertValueID( const BTL_POKEPARAM* bpp, BppValueID vid )
{
  switch( vid ){
  case BPP_DEFENCE:
    if( BTL_FIELD_CheckEffect(BTL_FLDEFF_WONDERROOM) ){
      vid = BPP_SP_DEFENCE;
    }
    break;

  case BPP_SP_DEFENCE:
    if( BTL_FIELD_CheckEffect(BTL_FLDEFF_WONDERROOM) ){
      vid = BPP_DEFENCE;
    }
    break;

  }
  return vid;
}
//=============================================================================================
/**
 * ランク補正フラットな状態のステータス取得
 *
 * @param   bpp
 * @param   vid   必要なパラメータID
 *
 * @retval  int
 */
//=============================================================================================
int BPP_GetValue_Base( const BTL_POKEPARAM* bpp, BppValueID vid )
{
  vid = ConvertValueID( bpp, vid );

  switch( vid ){
  case BPP_ATTACK:            return bpp->baseParam.attack;
  case BPP_DEFENCE:           return bpp->baseParam.defence;
  case BPP_SP_ATTACK:         return bpp->baseParam.sp_attack;
  case BPP_SP_DEFENCE:        return bpp->baseParam.sp_defence;
  case BPP_AGILITY:           return bpp->baseParam.agility;

  case BPP_HIT_RATIO:         return BPP_RANK_STATUS_DEFAULT;
  case BPP_AVOID_RATIO:       return BPP_RANK_STATUS_DEFAULT;

  default:
    return BPP_GetValue( bpp, vid );
  };
}
//=============================================================================================
/**
 * 能力値を書き換え
 *
 * @param   bpp
 * @param   vid
 * @param   value
 */
//=============================================================================================
void BPP_SetBaseStatus( BTL_POKEPARAM* bpp, BppValueID vid, u16 value )
{
  vid = ConvertValueID( bpp, vid );

  switch( vid ){
  case BPP_ATTACK:      bpp->baseParam.attack = value; break;
  case BPP_DEFENCE:     bpp->baseParam.defence = value; break;
  case BPP_SP_ATTACK:   bpp->baseParam.sp_attack = value; break;
  case BPP_SP_DEFENCE:  bpp->baseParam.sp_defence = value; break;
  case BPP_AGILITY:     bpp->baseParam.agility = value; break;

  default:
    GF_ASSERT(0);
    break;
  }

  BTL_Printf("ポケ[%d]の能力(%d)を%dに書き換えた\n", bpp->coreParam.myID, vid, value);
}

//=============================================================================================
/**
 * 各種パラメータ取得
 *
 * @param   pp
 * @param   vid   必要なパラメータID
 *
 * @retval  int
 */
//=============================================================================================
int BPP_GetValue( const BTL_POKEPARAM* bpp, BppValueID vid )
{
  vid = ConvertValueID( bpp, vid );

  switch( vid ){
  case BPP_ATTACK:          return BTL_CALC_StatusRank( BPP_GetValue_Base(bpp, vid), bpp->varyParam.attack );
  case BPP_DEFENCE:         return BTL_CALC_StatusRank( BPP_GetValue_Base(bpp, vid), bpp->varyParam.defence );
  case BPP_SP_ATTACK:       return BTL_CALC_StatusRank( BPP_GetValue_Base(bpp, vid), bpp->varyParam.sp_attack );
  case BPP_SP_DEFENCE:      return BTL_CALC_StatusRank( BPP_GetValue_Base(bpp, vid), bpp->varyParam.sp_defence );
  case BPP_AGILITY:         return BTL_CALC_StatusRank( BPP_GetValue_Base(bpp, vid), bpp->varyParam.agility );

  case BPP_ATTACK_RANK:     return bpp->varyParam.attack;
  case BPP_DEFENCE_RANK:    return bpp->varyParam.defence;
  case BPP_SP_ATTACK_RANK:  return bpp->varyParam.sp_attack;
  case BPP_SP_DEFENCE_RANK: return bpp->varyParam.sp_defence;
  case BPP_AGILITY_RANK:    return bpp->varyParam.agility;

  case BPP_HIT_RATIO:       return bpp->varyParam.hit;
  case BPP_AVOID_RATIO:     return bpp->varyParam.avoid;

  case BPP_LEVEL:           return bpp->baseParam.level;
  case BPP_HP:              return bpp->coreParam.hp;
  case BPP_MAX_HP:          return bpp->baseParam.hpMax;
  case BPP_SEX:             return bpp->baseParam.sex;

  case BPP_TOKUSEI_EFFECTIVE:
    if( BPP_CheckSick(bpp, WAZASICK_IEKI) ){
      return POKETOKUSEI_NULL;
    }
    /* fallthru */
  case BPP_TOKUSEI:           return bpp->tokusei;

  case BPP_FORM:            return bpp->formNo;
  case BPP_EXP:             return bpp->coreParam.exp;

  default:
    GF_ASSERT(0);
    return 0;
  };
}
//=============================================================================================
/**
 * クリティカルヒット時のパラメータ取得（攻撃側に不利なランク補正をフラットにする）
 *
 * @param   pp
 * @param   vid
 *
 * @retval  int
 */
//=============================================================================================
int BPP_GetValue_Critical( const BTL_POKEPARAM* bpp, BppValueID vid )
{
  BOOL fFlatParam = FALSE;
  BppValueID  vid_org = vid;

  // @todo これだと BPP_GetValue_Base を呼び出しているので再変換が起こりまずい
  vid = ConvertValueID( bpp, vid_org );

  switch( vid ){
  case BPP_ATTACK:     fFlatParam = (bpp->varyParam.attack < BPP_RANK_STATUS_DEFAULT); break;
  case BPP_SP_ATTACK:  fFlatParam = (bpp->varyParam.sp_attack < BPP_RANK_STATUS_DEFAULT); break;
  case BPP_DEFENCE:    fFlatParam = (bpp->varyParam.defence > BPP_RANK_STATUS_DEFAULT); break;
  case BPP_SP_DEFENCE: fFlatParam = (bpp->varyParam.sp_defence > BPP_RANK_STATUS_DEFAULT); break;

  default:
    break;
  }

  if( fFlatParam ){
    return BPP_GetValue_Base( bpp, vid_org );
  }else{
    return BPP_GetValue( bpp, vid_org );
  }
}
//=============================================================================================
/**
 * 所有アイテムIDを返す
 *
 * @param   pp
 *
 * @retval  u32
 */
//=============================================================================================
u32 BPP_GetItem( const BTL_POKEPARAM* pp )
{
  return pp->coreParam.item;
}
void BPP_SetItem( BTL_POKEPARAM* pp, u16 itemID )
{
  pp->coreParam.item = itemID;
}

//=============================================================================================
/**
 * HP満タンかチェック
 *
 * @param   pp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_IsHPFull( const BTL_POKEPARAM* pp )
{
  return BPP_GetValue(pp, BPP_HP) == BPP_GetValue(pp, BPP_MAX_HP);
}
//=============================================================================================
/**
 * ひんし状態かチェック
 *
 * @param   pp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_IsDead( const BTL_POKEPARAM* pp )
{
  return BPP_GetValue( pp, BPP_HP ) == 0;
}
BOOL BPP_IsFightEnable( const BTL_POKEPARAM* bpp )
{
  if( PP_Get(bpp->coreParam.ppSrc, ID_PARA_tamago_flag, NULL)){
    return FALSE;
  }
  if( BPP_IsDead(bpp) ){
    return FALSE;
  }
  return TRUE;
}

//=============================================================================================
/**
 * ワザPP値を取得
 *
 * @param   pp
 * @param   wazaIdx
 *
 */
//=============================================================================================
u16 BPP_WAZA_GetPP( const BTL_POKEPARAM* pp, u8 wazaIdx )
{
  if( wazaIdx < pp->wazaCnt ){
    return  pp->waza[wazaIdx].pp;
  }else{
    GF_ASSERT_MSG(0,"wazaIdx:%d, wazaCnt:%d", wazaIdx, pp->wazaCnt);
    return 0;
  }
}
//=============================================================================================
/**
 * ワザPP値が満タンかどうかチェック
 *
 * @param   pp
 * @param   wazaIdx
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_IsPPFull( const BTL_POKEPARAM* pp, u8 wazaIdx )
{
  GF_ASSERT(wazaIdx < pp->wazaCnt);
  return  pp->waza[wazaIdx].pp == pp->waza[wazaIdx].ppMax;
}
//=============================================================================================
/**
 * 継続して場に出ているターン数を返す
 *
 * @param   pp
 *
 * @retval  u16
 */
//=============================================================================================
u16 BPP_GetTurnCount( const BTL_POKEPARAM* pp )
{
  return pp->turnCount;
}
//=============================================================================================
/**
 * 場に出た時のターン数を返す
 *
 * @param   pp
 *
 * @retval  u16
 */
//=============================================================================================
u16 BPP_GetAppearTurn( const BTL_POKEPARAM* pp )
{
  return pp->appearedTurn;
}
//=============================================================================================
/**
 * ターンフラグ値取得
 *
 * @param   pp
 * @param   flagID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_TURNFLAG_Get( const BTL_POKEPARAM* pp, BppTurnFlag flagID )
{
  return flgbuf_get( pp->turnFlag, flagID );
}
//=============================================================================================
/**
 * 永続フラグ値取得
 *
 * @param   pp
 * @param   flagID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_CONTFLAG_Get( const BTL_POKEPARAM* pp, BppContFlag flagID )
{
  return flgbuf_get( pp->contFlag, flagID );
}
//=============================================================================================
/**
 * HP残量のめやす（普通・半減・ピンチとか）を返す
 *
 * @param   pp
 *
 * @retval  BppHpBorder
 */
//=============================================================================================
BppHpBorder BPP_CheckHPBorder( const BTL_POKEPARAM* pp, u32 hp )
{
  if( hp <= (pp->baseParam.hpMax / 8) )
  {
    return BPP_HPBORDER_RED;
  }
  if( hp <= (pp->baseParam.hpMax / 3) )
  {
    return BPP_HPBORDER_YELLOW;
  }
  return BPP_HPBORDER_GREEN;
}
//=============================================================================================
/**
 * HP残量のめやす（普通・半減・ピンチとか）を返す
 *
 * @param   pp
 *
 * @retval  BppHpBorder
 */
//=============================================================================================
BppHpBorder BPP_GetHPBorder( const BTL_POKEPARAM* pp )
{
  return BPP_CheckHPBorder( pp, pp->coreParam.hp );
}

//=============================================================================================
/**
 * 残りHP/MaxHP 比率（パーセンテージ）取得
 *
 * @param   pp
 *
 * @retval  fx32
 */
//=============================================================================================
fx32 BPP_GetHPRatio( const BTL_POKEPARAM* pp )
{
  double r = (double)(pp->coreParam.hp * 100) / (double)(pp->baseParam.hpMax);
  return FX32_CONST( r );
}
//=============================================================================================
/**
 * 指定ワザのindexを返す（自分が持っていないワザならPTL_WAZA_MAXを返す)
  *
 * @param   pp
 * @param   waza
 *
 * @retval  u8
 */
//=============================================================================================
u8 BPP_WAZA_SearchIdx( const BTL_POKEPARAM* pp, WazaID waza )
{
  if( waza != WAZANO_NULL )
  {
    u32 i;
    for(i=0; i<PTL_WAZA_MAX; ++i){
      if( pp->waza[i].number == waza ){
        return i;
      }
    }
  }
  return PTL_WAZA_MAX;
}

//-----------------------------
static const s8* getRankVaryStatusConst( const BTL_POKEPARAM* bpp, BppValueID type, s8* min, s8* max )
{
  const s8* ptr;

  *min = BPP_RANK_STATUS_MIN;
  *max = BPP_RANK_STATUS_MAX;

  switch( type ) {
  case BPP_ATTACK:      ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE:     ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK:   ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE:  ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY:     ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:   ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO: ptr = &bpp->varyParam.avoid; break;

  default:
    GF_ASSERT_MSG(0, "illegal rank Type ->%d", type);
    return NULL;
  }
  return ptr;
}
static s8* getRankVaryStatus( BTL_POKEPARAM* pp, BppValueID type, s8* min, s8* max )
{
  return (s8*) getRankVaryStatusConst( pp, type, min, max );
}

//=============================================================================================
/**
 * ランク増減効果が有効か？
 *
 * @param   bpp
 * @param   rankType
 * @param   volume
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_IsRankEffectValid( const BTL_POKEPARAM* bpp, BppValueID rankType, int volume )
{
  const s8* ptr;
  s8  min, max, next_value;

  ptr = getRankVaryStatusConst( bpp, rankType, &min, &max );
  BTL_Printf("  現状=%d, 最小=%d, 最大=%d, 効果値=%d\n", *ptr, min, max, volume);
  if( volume > 0 ){
    return ((*ptr) < max);
  }else{
    return ((*ptr) > min);
  }
}
//=============================================================================================
/**
 * ランク増加効果があと何段階効くか判定
 *
 * @param   pp
 * @param   rankType
 *
 * @retval  int   段階数
 */
//=============================================================================================
int BPP_RankEffectUpLimit( const BTL_POKEPARAM* pp, BppValueID rankType )
{
  const s8* ptr;
  s8  min, max;

  ptr = getRankVaryStatusConst( pp, rankType, &min, &max );
  BTL_Printf("ポケ[%d]のランク効果(%d) ... 現段階=%d, 上昇限界=%d\n", pp->coreParam.myID, rankType, *ptr, (max-(*ptr)));
  return max - (*ptr);
}
//=============================================================================================
/**
 * ランク減少効果があと何段階効くか判定
 *
 * @param   pp
 * @param   rankType
 *
 * @retval  int   段階数（マイナス）
 */
//=============================================================================================
int BPP_RankEffectDownLimit( const BTL_POKEPARAM* bpp, BppValueID rankType )
{
  const s8* ptr;
  s8  min, max;

  ptr = getRankVaryStatusConst( bpp, rankType, &min, &max );
  return (*ptr) - min;
}
//=============================================================================================
/**
 * フラットよりランクが下がっている能力が１つ以上あるかチェック
 *
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_IsRankEffectDowned( const BTL_POKEPARAM* bpp )
{
  if( bpp->varyParam.attack < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.defence  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.sp_attack  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.sp_defence  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.agility  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.hit  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  if( bpp->varyParam.avoid  < BPP_RANK_STATUS_DEFAULT ){ return TRUE; }
  return FALSE;
}

//=============================================================================================
/**
 * ランクアップ効果
 *
 * @param   pp
 * @param   rankType
 * @param   volume
 *
 * @retval  u8 result   実際に上がった段階数
 */
//=============================================================================================
u8 BPP_RankUp( BTL_POKEPARAM* bpp, BppValueID rankType, u8 volume )
{
  s8 *ptr;
  s8 max = BPP_RANK_STATUS_MAX;

  switch( rankType ){
  case BPP_ATTACK:      ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE:     ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK:   ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE:  ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY:     ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:   ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO: ptr = &bpp->varyParam.avoid; break;

  default:
    GF_ASSERT(0);
    return FALSE;
  }

  if( *ptr < max )
  {
    if( (*ptr + volume) > max ){
      volume = max - (*ptr);
    }
    *ptr += volume;
    return volume;
  }

  return 0;
}
//=============================================================================================
/**
 * ランクダウン効果
 *
 * @param   pp
 * @param   rankType
 * @param   volume
 *
 * @retval  u8    実際に下がった段階数
 */
//=============================================================================================
u8 BPP_RankDown( BTL_POKEPARAM* bpp, BppValueID rankType, u8 volume )
{
  s8 *ptr;
  s8 min = BPP_RANK_STATUS_MIN;

  switch( rankType ){
  case BPP_ATTACK:       ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE:      ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK:    ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE:   ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY:      ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:    ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO:  ptr = &bpp->varyParam.avoid; break;

  default:
    GF_ASSERT(0);
    return FALSE;
  }

  if( *ptr > min )
  {
    if( (*ptr - volume) < min ){
      volume = (*ptr) - min;
    }
    *ptr -= volume;
    return volume;
  }
  return 0;
}
//=============================================================================================
/**
 * ランク値強制セット
 *
 * @param   pp
 * @param   rankType
 * @param   volume
 *
 */
//=============================================================================================
void BPP_RankSet( BTL_POKEPARAM* pp, BppValueID rankType, u8 value )
{
  s8 *ptr;
  u8 min = BPP_RANK_STATUS_MIN;
  u8 max = BPP_RANK_STATUS_MAX;

  switch( rankType ){
  case BPP_ATTACK:        ptr = &pp->varyParam.attack; break;
  case BPP_DEFENCE:       ptr = &pp->varyParam.defence; break;
  case BPP_SP_ATTACK:     ptr = &pp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE:    ptr = &pp->varyParam.sp_defence; break;
  case BPP_AGILITY:       ptr = &pp->varyParam.agility; break;
  case BPP_HIT_RATIO:     ptr = &pp->varyParam.hit; break;
  case BPP_AVOID_RATIO:   ptr = &pp->varyParam.avoid; break;

  default:
    GF_ASSERT(0);
    return;
  }

  if( (min <= value) && (value <=max) ){
    *ptr = value;
  }else{
    GF_ASSERT_MSG(0, "value=%d", value);
  }
}
//=============================================================================================
/**
 * ランク効果（７種）の内、下がっているものをフラットに戻す
 *
 * @param   bpp
 *
 * @retval  BOOL    有効だった（ランクが下がっている能力が１つ以上あった）場合、TRUE
 */
//=============================================================================================
BOOL BPP_RankRecover( BTL_POKEPARAM* bpp )
{
  return Effrank_Recover( &bpp->varyParam );
}
//=============================================================================================
/**
 * ランク効果（７種）をフラットに戻す
 *
 * @param   pp
 */
//=============================================================================================
void BPP_RankReset( BTL_POKEPARAM* bpp )
{
  Effrank_Reset( &bpp->varyParam );
}

//=============================================================================================
/**
 * クリティカルランク取得
 *
 * @param   bpp
 *
 * @retval  u8
 */
//=============================================================================================
u8 BPP_GetCriticalRank( const BTL_POKEPARAM* bpp )
{
  u8 result = bpp->criticalRank;
  if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_KIAIDAME) ){
    result += 2;
    if( result > RANK_CRITICAL_MAX ){
      result = RANK_CRITICAL_MAX;
    }
  }
  return result;
}
//=============================================================================================
/**
 * クリティカルランク変動
 *
 * @param   bpp
 * @param   value
 *
 * @retval  BOOL    変動した場合TRUE／既に上限（下限）で変動できない場合FALSE
 */
//=============================================================================================
BOOL BPP_AddCriticalRank( BTL_POKEPARAM* bpp, int value )
{
  if( value > 0 )
  {
    if( bpp->criticalRank < RANK_CRITICAL_MAX )
    {
      bpp->criticalRank += value;
      if( bpp->criticalRank > RANK_CRITICAL_MAX ){
        bpp->criticalRank = RANK_CRITICAL_MAX;
      }
      return TRUE;
    }
  }
  else
  {
    value *= -1;
    if( bpp->criticalRank )
    {
      if( bpp->criticalRank > value ){
        bpp->criticalRank -= value;
      }else{
        bpp->criticalRank = 0;
      }
      return TRUE;
    }
  }
  return FALSE;
}



//=============================================================================================
/**
 * HP値を減少
 *
 * @param   pp
 * @param   value
 *
 */
//=============================================================================================
void BPP_HpMinus( BTL_POKEPARAM* pp, u16 value )
{
  if( pp->coreParam.hp > value )
  {
    pp->coreParam.hp -= value;
  }
  else
  {
    pp->coreParam.hp = 0;
  }
}
//=============================================================================================
/**
 * HP値を増加
 *
 * @param   pp
 * @param   value
 *
 */
//=============================================================================================
void BPP_HpPlus( BTL_POKEPARAM* pp, u16 value )
{
  pp->coreParam.hp += value;
  if( pp->coreParam.hp > pp->baseParam.hpMax )
  {
    pp->coreParam.hp = pp->baseParam.hpMax;
  }
}
//=============================================================================================
/**
 * HP値をゼロにする
 *
 * @param   pp
 *
 */
//=============================================================================================
void BPP_HpZero( BTL_POKEPARAM* pp )
{
  pp->coreParam.hp = 0;
}
//=============================================================================================
/**
 * ワザPP値を減少
 *
 * @param   pp
 * @param   wazaIdx
 * @param   value
 *
 */
//=============================================================================================
void BPP_PPMinus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value )
{
  GF_ASSERT(wazaIdx < pp->wazaCnt);

  if( pp->waza[wazaIdx].pp >= value )
  {
    pp->waza[wazaIdx].pp -= value;
  }
  else
  {
    pp->waza[wazaIdx].pp = 0;
  }
}
//=============================================================================================
/**
 * ワザPP値を増加
 *
 * @param   pp
 * @param   wazaIdx
 * @param   value
 *
 */
//=============================================================================================
void BPP_PPPlus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value )
{
  GF_ASSERT(wazaIdx < pp->wazaCnt);

  pp->waza[wazaIdx].pp += value;
  if( pp->waza[wazaIdx].pp > pp->waza[wazaIdx].ppMax )
  {
    pp->waza[wazaIdx].pp = pp->waza[wazaIdx].ppMax;
  }
}
//=============================================================================================
/**
 * 使用したワザフラグを立てる
 *
 * @param   pp
 * @param   wazaIdx
 */
//=============================================================================================
void BPP_WAZA_SetUsedFlag( BTL_POKEPARAM* pp, u8 wazaIdx )
{
  pp->waza[ wazaIdx ].usedFlag = TRUE;
}
//=============================================================================================
/**
 * ワザナンバー上書き
 *
 * @param   pp
 * @param   wazaIdx       何番目のワザ[0-3]？
 * @param   waza          上書き後ワザナンバー
 * @param   ppMax         PP最大値の上限（最大でもデフォルト値まで。0ならデフォルト値になる）
 * @param   fPermenent    永続フラグ（TRUEならバトル後まで引き継ぐ／FALSEなら瀕死・入れかえで元に戻る）
 */
//=============================================================================================
void BPP_WAZA_UpdateID( BTL_POKEPARAM* pp, u8 wazaIdx, WazaID waza, u8 ppMax, BOOL fPermenent )
{
  BPP_WAZA* pWaza = &pp->waza[ wazaIdx ];

  if( !fPermenent ){
    pWaza->recoverNumber = pWaza->number;
  }else{
    pWaza->recoverNumber = waza;
  }
  pWaza->number = waza;
  pWaza->usedFlag = FALSE;
  pWaza->ppMax = WAZADATA_GetMaxPP( waza, 0 );

  if( (ppMax != 0)
  &&  (pWaza->ppMax > ppMax)
  ){
    pWaza->ppMax = ppMax;
  }
  pWaza->pp = pWaza->ppMax;
//  OS_TPrintf("ワザ上書き: ppMax指定=%d, 実値:%d\n", ppMax, pWaza->ppMax);
}
//=============================================================================================
/**
 * 指定ワザを覚えているかチェック
 *
 * @param   bpp
 * @param   waza
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_WAZA_IsUsable( const BTL_POKEPARAM* bpp, WazaID waza )
{
  int i;
  for(i=0; i<NELEMS(bpp->waza); ++i)
  {
    if( bpp->waza[i].number == waza ){
      return TRUE;
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * １ターン有効フラグのセット
 *
 * @param   pp
 * @param   flagID
 *
 */
//=============================================================================================
void BPP_TURNFLAG_Set( BTL_POKEPARAM* pp, BppTurnFlag flagID )
{
  flgbuf_set( pp->turnFlag, flagID );
}
//=============================================================================================
/**
 * 永続フラグのセット
 *
 * @param   pp
 * @param   flagID
 *
 */
//=============================================================================================
void BPP_CONTFLAG_Set( BTL_POKEPARAM* pp, BppContFlag flagID )
{
  if( flagID == BPP_CONTFLG_POWERTRICK ){
    BTL_Printf("ポケ[%d]にパワートリックフラグ\n", pp->coreParam.myID);
  }
  flgbuf_set( pp->contFlag, flagID );
}
//=============================================================================================
/**
 * 永続フラグのリセット
 *
 * @param   pp
 * @param   flagID
 *
 */
//=============================================================================================
void BPP_CONTFLAG_Clear( BTL_POKEPARAM* pp, BppContFlag flagID )
{
  flgbuf_reset( pp->contFlag, flagID );
}

//==================================================================================================
// 状態異常関連
//==================================================================================================


//=============================================================================================
/**
 * 状態異常を設定
 *
 * @param   pp
 * @param   sick
 * @param   contParam   継続パラメータ
 *
 */
//=============================================================================================
void BPP_SetWazaSick( BTL_POKEPARAM* bpp, WazaSick sick, BPP_SICK_CONT contParam )
{
  if( BTL_CALC_IsBasicSickID(sick) )
  {
    PokeSick pokeSick = BPP_GetPokeSick( bpp );
    GF_ASSERT(pokeSick == POKESICK_NULL);
  }

  bpp->sickCont[ sick ] = contParam;
  bpp->wazaSickCounter[sick] = 0;
  if( sick == WAZASICK_NEMURI ){
  }

}


//=============================================================================================
/**
 * 状態異常のターンチェック処理
 *
 * @param   bpp
 * @param   callbackFunc    かかっている状態異常ごとに呼び出されるコールバック関数
 * @param   callbackArg     コールバックに引き渡す任意引数
 */
//=============================================================================================
void BPP_WazaSick_TurnCheck( BTL_POKEPARAM* bpp, BtlSickTurnCheckFunc callbackFunc, void* callbackWork )
{
  WazaSick  sick;


  for(sick=0; sick<NELEMS(bpp->sickCont); ++sick)
  {
    // ねむり・こんらんはアクション開始のタイミングで独自のカウンタデクリメント処理
    if( (sick == WAZASICK_NEMURI) || (sick == WAZASICK_KONRAN) ){
      continue;
    }
    if( bpp->sickCont[sick].type != WAZASICK_CONT_NONE )
    {
      u32 turnMax = BPP_SICCONT_GetTurnMax( bpp->sickCont[sick] );
      BOOL fCure = FALSE;
      BPP_SICK_CONT oldCont;

      oldCont = bpp->sickCont[ sick ];

      // こだわりロックは、該当するワザを持っていないなら直る
      if( sick == WAZASICK_KODAWARI )
      {
        WazaID  waza = BPP_SICKCONT_GetParam( oldCont );
        if( !BPP_WAZA_IsUsable(bpp, waza) )
        {
          bpp->sickCont[sick] = BPP_SICKCONT_MakeNull();;
          bpp->wazaSickCounter[sick] = 0;
          fCure = TRUE;
        }
      }

      // 継続ターン経過チェック
      if( turnMax )
      {
        bpp->wazaSickCounter[sick] += 1;

        if( bpp->wazaSickCounter[sick] >= turnMax )
        {
          BTL_Printf("経過ターンが最大ターンを越えた\n");
          bpp->sickCont[sick] = BPP_SICKCONT_MakeNull();;
          bpp->wazaSickCounter[sick] = 0;
          fCure = TRUE;
        }
      }
      // 永続型で最大ターン数が指定されているものはカウンタをインクリメント
      // （現状、この機構を利用しているのは「どくどく」のみ）
      else if( bpp->sickCont[sick].type == WAZASICK_CONT_PERMANENT )
      {
        if( (bpp->sickCont[sick].permanent.count_max != 0 )
        &&  (bpp->wazaSickCounter[sick] < bpp->sickCont[sick].permanent.count_max)
        ){
          bpp->wazaSickCounter[sick]++;
        }
      }
      if( callbackFunc != NULL ){
        callbackFunc( bpp, sick, oldCont, fCure, callbackWork );
      }
    }
  }
}

//=============================================================================================
/**
 * 「ねむり」独自のターンチェック処理
 * 状態異常カウンタをインクリメントし、起きる／起きないのチェックを行う
 *
 * @param   bpp
 *
 * @retval  BOOL    今回の呼び出しによって起きた場合はTRUE／それ以外はFALSE
 */
//=============================================================================================
BOOL BPP_CheckNemuriWakeUp( BTL_POKEPARAM* bpp )
{
  BPP_SICK_CONT* cont = &(bpp->sickCont[ WAZASICK_NEMURI ]);

  if( cont->type != WAZASICK_CONT_NONE )
  {
    u32 turnMax = BPP_SICCONT_GetTurnMax( *cont );
    if( turnMax )
    {
      // とくせい「はやおき」は眠りカウンタ２倍速
      u32 incValue = IsMatchTokusei(bpp, POKETOKUSEI_HAYAOKI)? 2 : 1;

      bpp->wazaSickCounter[ WAZASICK_NEMURI ] += incValue;
      BTL_N_Printf( DBGSTR_BPP_NemuriWakeCheck, bpp->coreParam.myID, turnMax, bpp->wazaSickCounter[ WAZASICK_NEMURI ] );
      if( bpp->wazaSickCounter[ WAZASICK_NEMURI ] >= turnMax )
      {
        *cont = BPP_SICKCONT_MakeNull();;
        BPP_CureWazaSick( bpp, WAZASICK_NEMURI );
        return TRUE;
      }
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * 「こんらん」独自のターンチェック処理
 * 状態異常カウンタをインクリメントし、さめる／さめないのチェックを行う
 *
 * @param   bpp
 *
 * @retval  BOOL    今回の呼び出しによって覚めた場合はTRUE／それ以外はFALSE
 */
//=============================================================================================
BOOL BPP_CheckKonranWakeUp( BTL_POKEPARAM* bpp )
{
  enum {
    SICK_ID = WAZASICK_KONRAN,
  };
  BPP_SICK_CONT* cont = &(bpp->sickCont[ SICK_ID ]);

  if( cont->type != WAZASICK_CONT_NONE )
  {
    u32 turnMax = BPP_SICCONT_GetTurnMax( *cont );
    if( turnMax )
    {
      bpp->wazaSickCounter[ SICK_ID ]++;
      if( bpp->wazaSickCounter[ SICK_ID ] >= turnMax )
      {
        *cont = BPP_SICKCONT_MakeNull();;
        BPP_CureWazaSick( bpp, SICK_ID );
        return TRUE;
      }
    }
  }
  return FALSE;
}

//=============================================================================================
/**
 * ポケモン系状態異常を回復させる
 *
 * @param   pp
 *
 */
//=============================================================================================
void BPP_CurePokeSick( BTL_POKEPARAM* bpp )
{
  u32 i;
  for(i=POKESICK_ORIGIN; i<POKESICK_MAX; ++i)
  {
    bpp->sickCont[ i ] = BPP_SICKCONT_MakeNull();
    bpp->wazaSickCounter[ i ] = 0;
    cureDependSick( bpp, i );
  }
}
//----------------------------------------------------------------------------------
/**
 * ある状態異常が治った時に同時に治る状態の処理
 *
 * @param   bpp
 * @param   sickID
 */
//----------------------------------------------------------------------------------
static void cureDependSick( BTL_POKEPARAM* bpp, WazaSick sickID  )
{
  switch( sickID ){
  case WAZASICK_NEMURI:
    bpp->sickCont[ WAZASICK_AKUMU ] = BPP_SICKCONT_MakeNull();    // 眠りが治れば“あくむ”も治る
    bpp->wazaSickCounter[ WAZASICK_AKUMU ] = 0;
    break;
  }
}
//=============================================================================================
/**
 * ワザ系状態異常を回復させる
 *
 * @param   pp
 * @param   sick
 *
 */
//=============================================================================================
void BPP_CureWazaSick( BTL_POKEPARAM* bpp, WazaSick sick )
{
  if( BTL_CALC_IsBasicSickID(sick) )
  {
    BPP_CurePokeSick( bpp );
  }
  else
  {
    bpp->sickCont[ sick ] = BPP_SICKCONT_MakeNull();
    bpp->wazaSickCounter[ sick ] = 0;
  }
}
//=============================================================================================
/**
 * 特定ポケモンに依存している状態異常を回復させる
 *
 * @param   pp
 * @param   depend_pokeID
 */
//=============================================================================================
void BPP_CureWazaSickDependPoke( BTL_POKEPARAM* bpp, u8 depend_pokeID, BppCureWazaSickDependPokeCallback callbackFunc, void* callbackArg )
{
  u32 i;
  u8 fCure;
  for(i=0; i<WAZASICK_MAX; ++i)
  {
    switch( bpp->sickCont[i].type ){
    case WAZASICK_CONT_POKE:
      fCure = ( bpp->sickCont[i].poke.ID == depend_pokeID );
      break;
    case WAZASICK_CONT_POKETURN:
      fCure = ( bpp->sickCont[i].poketurn.pokeID == depend_pokeID );
      break;
    default:
      fCure = FALSE;
    }
    if( fCure ){
      bpp->sickCont[i] = BPP_SICKCONT_MakeNull();
      cureDependSick( bpp, i );
      callbackFunc( callbackArg, bpp, i, depend_pokeID );
    }
  }
}
//=============================================================================================
/**
 * ポケモン状態異常にかかっているかチェック
 *
 * @param   pp
 *
 * @retval  PokeSick    かかっている状態異常の識別子（かかっていない場合 POKESICK_NULL）
 */
//=============================================================================================
PokeSick BPP_GetPokeSick( const BTL_POKEPARAM* pp )
{
  u32 i;
  for(i=POKESICK_ORIGIN; i<POKESICK_MAX; ++i)
  {
    if( pp->sickCont[i].type != WAZASICK_CONT_NONE ){
      return i;
    }
  }
  return POKESICK_NULL;
}
//=============================================================================================
/**
 * 特定の状態異常にかかっているかチェック
 *
 * @param   pp
 * @param   sickType
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_CheckSick( const BTL_POKEPARAM* bpp, WazaSick sickType )
{
  GF_ASSERT(sickType < NELEMS(bpp->sickCont));

  if( bpp->sickCont[ sickType ].type != WAZASICK_CONT_NONE )
  {
    if( (sickType == WAZASICK_WAZALOCK) || (sickType == WAZASICK_ENCORE) ){
      BTL_Printf("該当異常になってます pokeID=%d, sick=%d\n", bpp->coreParam.myID, sickType);
    }
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * 特定状態異常に設定されたパラメータ値（8bit）を取得
 *
 * @param   pp
 * @param   sick
 *
 * @retval  u8
 */
//=============================================================================================
u16 BPP_GetSickParam( const BTL_POKEPARAM* pp, WazaSick sick )
{
  GF_ASSERT(sick < NELEMS(pp->sickCont));

  switch(  pp->sickCont[sick].type ){
  case WAZASICK_CONT_TURN:
    return pp->sickCont[sick].turn.param;

  case WAZASICK_CONT_POKE:
    return pp->sickCont[sick].poke.ID;

  case WAZASICK_CONT_POKETURN:
    return pp->sickCont[sick].poketurn.pokeID;

  }

  GF_ASSERT(0); // パラメ無いのに呼び出された
  return 0;
}
//=============================================================================================
/**
 * 特定状態異常の継続パラメータ取得
 *
 * @param   bpp
 * @param   sick
 *
 * @retval  BPP_SICK_CONT
 */
//=============================================================================================
BPP_SICK_CONT BPP_GetSickCont( const BTL_POKEPARAM* bpp, WazaSick sick )
{
  GF_ASSERT(sick < NELEMS(bpp->sickCont));
  return bpp->sickCont[ sick ];
}
//=============================================================================================
/**
 * 特定状態異常の継続ターンカウンタを取得
 *
 * @param   bpp
 * @param   sick
 *
 * @retval  u8
 */
//=============================================================================================
u8 BPP_GetSickTurnCount( const BTL_POKEPARAM* bpp, WazaSick sick )
{
  GF_ASSERT(sick < NELEMS(bpp->sickCont));
  return bpp->wazaSickCounter[ sick ];
}
//=============================================================================================
/**
 * 状態異常のターンチェックで減るHPの量を計算
 *
 * @param   pp
 *
 * @retval  int
 */
//=============================================================================================
int BPP_CalcSickDamage( const BTL_POKEPARAM* bpp, WazaSick sick )
{
  if( BPP_CheckSick(bpp, sick) )
  {
    switch( sick ){
    case WAZASICK_DOKU:
      // 「どくどく」状態ならターン数でダメージ増加
      if( BPP_SICKCONT_IsMoudokuCont(bpp->sickCont[sick]) ){
        return (bpp->baseParam.hpMax / 16) * bpp->wazaSickCounter[sick];
      }else{
        return BTL_CALC_QuotMaxHP( bpp, 8 );
      }
      break;

    case WAZASICK_YAKEDO:
      return BTL_CALC_QuotMaxHP( bpp, 8 );

    case WAZASICK_AKUMU:
      if( BPP_CheckSick(bpp, WAZASICK_NEMURI) ){
        return BTL_CALC_QuotMaxHP( bpp, 4 );
      }
      break;

    case WAZASICK_NOROI:
      return BTL_CALC_QuotMaxHP( bpp, 4 );

    default:
      return 0;
    }
  }
  return 0;
}
//----------------------------------------------------------------------------------
/**
 * ワザ系状態異常のカウンタクリア
 *
 * @param   pp
 */
//----------------------------------------------------------------------------------
static void clearWazaSickWork( BTL_POKEPARAM* bpp, BOOL fPokeSickInclude )
{
  u32 i, start;

  start = (fPokeSickInclude)? 0 : POKESICK_MAX;

  for(i=start; i<NELEMS(bpp->sickCont); ++i){
    bpp->sickCont[i].raw = 0;
    bpp->sickCont[i].type = WAZASICK_CONT_NONE;
  }
  GFL_STD_MemClear( bpp->wazaSickCounter, sizeof(bpp->wazaSickCounter) );
}

//=============================================================================================
/**
 * 場に入場した時のターンナンバーをセット
 *
 * @param   pp
 * @param   turn
 *
 */
//=============================================================================================
void BPP_SetAppearTurn( BTL_POKEPARAM* bpp, u16 turn )
{
  GF_ASSERT(turn < BTL_TURNCOUNT_MAX);
  bpp->appearedTurn = turn;
  bpp->turnCount = 0;
  dmgrecClearWork( bpp );
}
//=============================================================================================
/**
 * ターン終了処理
 *
 * @param   pp
 *
 */
//=============================================================================================
void BPP_TurnCheck( BTL_POKEPARAM* bpp )
{
  flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );

  if( bpp->turnCount < BTL_TURNCOUNT_MAX )
  {
    bpp->turnCount++;
  }

  dmgrecFwdTurn( bpp );
}
//=============================================================================================
/**
 * ターンフラグの個別途中クリア
 *
 * @param   pp
 * @param   flagID
 *
 */
//=============================================================================================
void BPP_TURNFLAG_ForceOff( BTL_POKEPARAM* bpp, BppTurnFlag flagID )
{
  flgbuf_reset( bpp->turnFlag, flagID );
}
//=============================================================================================
/**
 * 死亡による各種状態クリア
 *
 * @param   pp
 */
//=============================================================================================
void BPP_Clear_ForDead( BTL_POKEPARAM* bpp )
{
  flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );
  flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

  clearUsedWazaFlag( bpp );
  clearCounter( bpp );
  BPP_MIGAWARI_Delete( bpp );

  clearWazaSickWork( bpp, TRUE );
//  ConfrontRec_Clear( bpp );
  Effrank_Init( &bpp->varyParam );

  if( bpp->coreParam.ppFake ){
    bpp->coreParam.fFakeEnable = TRUE;
  }
}
//=============================================================================================
/**
 * 退場による各種状態クリア
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_Clear_ForOut( BTL_POKEPARAM* bpp )
{
  flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );
  /*
   * ※バトンタッチで引き継ぐ情報を記録しているため、
   * 退場時に継続フラグクリアはさせてはいけない
   */

  clearUsedWazaFlag( bpp );
  clearCounter( bpp );
//  ConfrontRec_Clear( bpp );
  if( bpp->coreParam.ppFake ){
    bpp->coreParam.fFakeEnable = TRUE;
  }
}
//=============================================================================================
/**
 * 再入場による各種状態初期化
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_Clear_ForIn( BTL_POKEPARAM* bpp )
{
  setupBySrcData( bpp, bpp->coreParam.ppSrc );
  flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

  clearWazaSickWork( bpp, FALSE );
  Effrank_Init( &bpp->varyParam );
}
//=============================================================================================
/**
 * バトンタッチによるパラメータ受け継ぎ
 *
 * @param   target    受け継ぐ側
 * @param   user      受け継がせる側
 */
//=============================================================================================
void BPP_BatonTouchParam( BTL_POKEPARAM* target, const BTL_POKEPARAM* user )
{
  u32 i;

  target->varyParam = user->varyParam;

  BTL_Printf("[%d]->[%d]へバトンタッチで引き継がれた:防御ランク=%d, 特防ランク=%d\n",
    user->coreParam.myID, target->coreParam.myID,
    target->varyParam.defence, target->varyParam.sp_defence);

  // 特定の状態異常をそのまま引き継ぎ
  for(i=0; i<WAZASICK_MAX; ++i)
  {
    if( (user->sickCont[i].type != WAZASICK_CONT_NONE)
    &&  (BTL_SICK_CheckBatonTouch(i))
    ){
      target->sickCont[i] = user->sickCont[i];
      target->wazaSickCounter[i] = user->wazaSickCounter[i];
    }
  }

  // 特定の継続フラグを引き継ぎ（今のところパワートリックだけ）
  if( BPP_CONTFLAG_Get(user, BPP_CONTFLG_POWERTRICK) ){
    u8 atk, def;
    BPP_CONTFLAG_Set(target, BPP_CONTFLG_POWERTRICK);
    atk = BPP_GetValue_Base( target, BPP_ATTACK );
    def = BPP_GetValue_Base( target, BPP_DEFENCE );
    BPP_SetBaseStatus( target, BPP_ATTACK, def );
    BPP_SetBaseStatus( target, BPP_DEFENCE, atk );
    BTL_Printf("パワートリック引き継ぎ: Atk(%d) <-> Def(%d)\n", atk, def);
  }
}

//=============================================================================================
/**
 * タイプかきかえ
 *
 * @param   pp
 * @param   type
 */
//=============================================================================================
void BPP_ChangePokeType( BTL_POKEPARAM* bpp, PokeTypePair type )
{
  bpp->baseParam.type1 = PokeTypePair_GetType1( type );
  bpp->baseParam.type2 = PokeTypePair_GetType2( type );
}
//=============================================================================================
/**
 * とくせい書き換え
 *
 * @param   pp
 * @param   tok
 */
//=============================================================================================
void BPP_ChangeTokusei( BTL_POKEPARAM* bpp, PokeTokusei tok )
{
  bpp->tokusei = tok;
}
//=============================================================================================
/**
 * フォルム変更（サーバ処理用：ソースデータは書き換えない）
 *
 * @param   pp
 * @param   formNo
 */
//=============================================================================================
void BPP_ChangeForm( BTL_POKEPARAM* bpp, u8 formNo )
{
  bpp->formNo = formNo;
}
//=============================================================================================
/**
 * フォルム変更（サーバ処理用：ソースデータは書き換えない）
 *
 * @param   bpp
 * @param   formNo
 */
//=============================================================================================
void BPP_ChangeFormPutSrcData( BTL_POKEPARAM* bpp, u8 formNo )
{
  bpp->formNo = formNo;
  PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_form_no, formNo );
}



//=============================================================================================
/**
 * 所有アイテム削除（所有アイテムが無くなるだけ）
 *
 * @param   pp
 */
//=============================================================================================
void BPP_RemoveItem( BTL_POKEPARAM* bpp )
{
  bpp->coreParam.usedItem = bpp->coreParam.item;
  bpp->coreParam.item = ITEM_DUMMY_DATA;
}
//=============================================================================================
/**
 * 所有アイテム消費（所有アイテムが無くなり、消費バッファに記憶する）
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ConsumeItem( BTL_POKEPARAM* bpp )
{
  bpp->coreParam.usedItem = bpp->coreParam.item;
  bpp->coreParam.item = ITEM_DUMMY_DATA;
}
//=============================================================================================
/**
 * アイテム消費情報のクリア
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ClearConsumedItem( BTL_POKEPARAM* bpp )
{
  bpp->coreParam.usedItem = ITEM_DUMMY_DATA;
}
//=============================================================================================
/**
 * 消費したアイテムナンバーを返す
 *
 * @param   bpp
 *
 * @retval  u16   消費したアイテムナンバー（消費していなければITEM_DUMMY_DATA）
 */
//=============================================================================================
u16 BPP_GetConsumedItem( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.usedItem;
}


//=============================================================================================
/**
 * ワザプロセス終了通知
 *
 * @param   bpp
 * @param   actTargetPos  actWaza の対象位置
 * @param   fActEnable    actWaza が効果があったか（ワザエフェクトが出るか）
 * @param   actWaza       実行されたワザ（
 * @param   orgWaza       クライアントが選択したワザ（ゆびをふる等、派生ワザの場合のみ actWaza と異なる）
 */
//=============================================================================================
void BPP_UpdateWazaProcResult( BTL_POKEPARAM* bpp, BtlPokePos actTargetPos, BOOL fActEnable, WazaID actWaza, WazaID orgWaza )
{
  WazaID  prevActWaza = bpp->prevActWazaID;

  bpp->prevSelectWazaID = orgWaza;
  bpp->prevActWazaID = actWaza;
  bpp->prevTargetPos = actTargetPos;

  if( prevActWaza == actWaza )
  {
    if( fActEnable ){
      bpp->wazaContCounter++;
    }else{
      bpp->wazaContCounter = 0;
    }
  }
  else{
    bpp->wazaContCounter = ( fActEnable )? 1 : 0;
  }
}

//=============================================================================================
/**
 * 同じワザを連続何回出しているか返す（連続していない場合は0、連続中は1オリジンの値が返る）
 *
 * @param   pp
 *
 * @retval  u32
 */
//=============================================================================================
u32 BPP_GetWazaContCounter( const BTL_POKEPARAM* bpp )
{
  return bpp->wazaContCounter;
}
//=============================================================================================
/**
 * 直近に実行されたワザナンバーを返す（場に出てから使ったワザが無い場合は WAZANO_NULL ）
 *
 * @param   pp
 *
 * @retval  WazaID
 */
//=============================================================================================
WazaID BPP_GetPrevWazaID( const BTL_POKEPARAM* bpp )
{
  return bpp->prevActWazaID;
}
//=============================================================================================
/**
 * 直近に選択されたワザナンバーを返す（派生ワザを選択した場合など、BPP_GetPrevWazaIDと値が異なる場合がある）
 *
 * @param   bpp
 *
 * @retval  WazaID
 */
//=============================================================================================
WazaID BPP_GetPrevOrgWazaID( const BTL_POKEPARAM* bpp )
{
  return bpp->prevSelectWazaID;
}

//=============================================================================================
/**
 * 直前に狙った相手の位置を返す
 *
 * @param   pp
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BPP_GetPrevTargetPos( const BTL_POKEPARAM* pp )
{
  return pp->prevTargetPos;
}


//=============================================================================================
/**
 * 体重を設定
 *
 * @param   bpp
 * @param   weight
 */
//=============================================================================================
void BPP_SetWeight( BTL_POKEPARAM* bpp, u16 weight )
{
  if( weight < BTL_POKE_WEIGHT_MIN ){
    weight = BTL_POKE_WEIGHT_MIN;
  }
  bpp->weight = weight;
}
//=============================================================================================
/**
 * 体重を取得
 *
 * @param   bpp
 *
 * @retval  u16
 */
//=============================================================================================
u16 BPP_GetWeight( const BTL_POKEPARAM* bpp )
{
  u16 weight = bpp->weight;

  if( IsMatchTokusei(bpp, POKETOKUSEI_HEVIMETARU) ){
    weight *= 2;
  }else if( IsMatchTokusei(bpp, POKETOKUSEI_RAITOMETARU) ){
    weight /= 2;
  }

  if( IsMatchItem(bpp, ITEM_KARUISI) ){
    weight /= 2;
  }

  if( weight < BTL_POKE_WEIGHT_MIN ){
    weight = BTL_POKE_WEIGHT_MIN;
  }

  return weight;
}
//----------------------------------------------------------------------------------
/**
 * ワザダメージレコード：ワーク初期化
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void dmgrecClearWork( BTL_POKEPARAM* bpp )
{
  GFL_STD_MemClear( bpp->wazaDamageRec, sizeof(bpp->wazaDamageRec) );
  GFL_STD_MemClear( bpp->dmgrecCount, sizeof(bpp->dmgrecCount) );
  bpp->dmgrecTurnPtr = 0;
  bpp->dmgrecPtr = 0;
}
//----------------------------------------------------------------------------------
/**
 * ワザダメージレコード：ターンチェック
 *
 * @param   pp
 */
//----------------------------------------------------------------------------------
static void dmgrecFwdTurn( BTL_POKEPARAM* bpp )
{
  if( ++(bpp->dmgrecTurnPtr) >= WAZADMG_REC_TURN_MAX ){
    bpp->dmgrecTurnPtr = 0;
  }
  bpp->dmgrecCount[ bpp->dmgrecTurnPtr ] = 0;
}
//=============================================================================================
/**
 * ワザダメージレコード：１件記録
 *
 * @param   pp
 * @param   rec
 */
//=============================================================================================
void BPP_WAZADMGREC_Add( BTL_POKEPARAM* bpp, const BPP_WAZADMG_REC* rec )
{
  u8 turn, idx;

  turn = bpp->dmgrecTurnPtr;
  idx = bpp->dmgrecCount[ turn ];

  // 最大記録数を超えていたら古いものを破棄する
  if( idx == WAZADMG_REC_MAX )
  {
    u32 i;
    for(i=0; i<(WAZADMG_REC_MAX-1); ++i){
      bpp->wazaDamageRec[ turn ][ i ] = bpp->wazaDamageRec[ turn ][ i+1 ];
    }
    --idx;
  }

  bpp->wazaDamageRec[ turn ][ idx ] = *rec;

  if( bpp->dmgrecCount[ turn ] < WAZADMG_REC_MAX ){
    bpp->dmgrecCount[ turn ]++;
  }
}
//=============================================================================================
/**
 * ワザダメージレコード：指定ターンの記録件数を取得
 *
 * @param   pp
 * @param   turn_ridx   遡るターン数（0なら当該ターンの記録）
 */
//=============================================================================================
u8 BPP_WAZADMGREC_GetCount( const BTL_POKEPARAM* bpp, u8 turn_ridx )
{
  if( turn_ridx < WAZADMG_REC_TURN_MAX )
  {
    int turn = bpp->dmgrecTurnPtr - turn_ridx;
    if( turn < 0 ){
      turn += WAZADMG_REC_TURN_MAX;
    }
    return bpp->dmgrecCount[ turn ];
  }
  else
  {
    GF_ASSERT( 0 );
    return 0;
  }
}
//=============================================================================================
/**
 * ワザダメージレコード：レコードを取得
 *
 * @param   bpp
 * @param   turn_ridx   遡るターン数（0なら当該ターンの記録）
 * @param   rec_ridx    何件目のレコードか？（0=最新）
 * @param   dst         [out] 取得先ワークアドレス
 *
 * @retval  BOOL        レコードを取得できたらTRUE／それ以上の記録が無い場合などFALSE
 */
//=============================================================================================
BOOL BPP_WAZADMGREC_Get( const BTL_POKEPARAM* bpp, u8 turn_ridx, u8 rec_ridx, BPP_WAZADMG_REC* dst )
{
  if( turn_ridx < WAZADMG_REC_TURN_MAX )
  {
    int turn = bpp->dmgrecTurnPtr - turn_ridx;
    if( turn < 0 ){
      turn += WAZADMG_REC_TURN_MAX;
    }
    if( rec_ridx < bpp->dmgrecCount[ turn ] )
    {
      u8 idx = bpp->dmgrecCount[ turn ] - rec_ridx - 1;
      *dst = bpp->wazaDamageRec[ turn ][ idx ];
      return TRUE;
    }
  }
  return FALSE;
}

//=============================================================================================
/**
 * カウンタ値をセット
 *
 * @param   bpp
 * @param   cnt     カウンタ用途
 * @param   value   カウンタ値
 */
//=============================================================================================
void BPP_COUNTER_Set( BTL_POKEPARAM* bpp, BppCounter cnt, u8 value )
{
  GF_ASSERT(cnt < BPP_COUNTER_MAX);
  bpp->counter[ cnt ] = value;
}
//=============================================================================================
/**
 * カウンタ値を取得
 *
 * @param   bpp
 * @param   cnt   カウンタ用途
 *
 * @retval  u8
 */
//=============================================================================================
u8 BPP_COUNTER_Get( const BTL_POKEPARAM* bpp, BppCounter cnt )
{
  GF_ASSERT(cnt < BPP_COUNTER_MAX);
  return bpp->counter[ cnt ];
}

//=============================================================================================
/**
 * 経験値加算（サーバ用：Srcポケモンデータへの反映は行わない）
 *
 * @param   bpp
 * @param   expRest  [io] 加算する経験値／レベルアップ時は残りの加算経験値
 * @param   info     [out] レベルアップ時に各種パラメータの上昇値を格納する
 *
 * @retval  BOOL    レベルアップしたらTRUE
 */
//=============================================================================================
BOOL BPP_AddExp( BTL_POKEPARAM* bpp, u32* expRest, BTL_LEVELUP_INFO* info )
{
  if( bpp->baseParam.level < PTL_LEVEL_MAX )
  {
    u32 expNow, expSum, expBorder;

    expNow = bpp->coreParam.exp;
    expSum = expNow + (*expRest);
    expBorder = POKETOOL_GetMinExp( bpp->coreParam.monsno, bpp->formNo, bpp->baseParam.level+1 );

    if( expSum >= expBorder )
    {
      u32 expAdd = (expBorder - expNow);
      u16 prevHP   = bpp->baseParam.hpMax;
      info->atk    = bpp->baseParam.attack;
      info->def    = bpp->baseParam.defence;
      info->sp_atk = bpp->baseParam.sp_attack;
      info->sp_def = bpp->baseParam.sp_defence;
      info->agi    = bpp->baseParam.agility;

      bpp->coreParam.exp = (expNow + expAdd);
      PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_exp, bpp->coreParam.exp );
      PP_Renew( (POKEMON_PARAM*)(bpp->coreParam.ppSrc) );

      bpp->baseParam.level = PP_Get( bpp->coreParam.ppSrc, ID_PARA_level, 0 );
      bpp->baseParam.hpMax = PP_Get( bpp->coreParam.ppSrc, ID_PARA_hpmax, 0 );
      bpp->baseParam.attack = PP_Get( bpp->coreParam.ppSrc, ID_PARA_pow, 0 );
      bpp->baseParam.defence = PP_Get( bpp->coreParam.ppSrc, ID_PARA_def, 0 );
      bpp->baseParam.sp_attack = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spepow, 0 );
      bpp->baseParam.sp_defence = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spedef, 0 );
      bpp->baseParam.agility = PP_Get( bpp->coreParam.ppSrc, ID_PARA_agi, 0 );

      info->level  = bpp->baseParam.level;
      info->hp     = bpp->baseParam.hpMax - prevHP;
      info->atk    = bpp->baseParam.attack - info->atk;
      info->def    = bpp->baseParam.defence - info->def;
      info->sp_atk = bpp->baseParam.sp_attack - info->sp_atk;
      info->sp_def = bpp->baseParam.sp_defence - info->sp_def;
      info->agi    = bpp->baseParam.agility - info->agi;

      bpp->coreParam.hp += info->hp;
      PP_Put((POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_hp, bpp->coreParam.hp );

//      PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_exp, bpp->exp );
//      PP_Renew( (POKEMON_PARAM*)(bpp->coreParam.ppSrc) );

      *expRest -= expAdd;
      return TRUE;
    }
    else
    {
      BTL_Printf("pp[%p]に経験値 %d->%d\n", bpp->coreParam.ppSrc, bpp->coreParam.exp, expSum);
      bpp->coreParam.exp = expSum;
      PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_exp, bpp->coreParam.exp );
      BTL_Printf("  ... exp=%d=%d\n", PP_Get(bpp->coreParam.ppSrc, ID_PARA_exp, NULL),bpp->coreParam.exp);
    }
  }

  GFL_STD_MemClear( info, sizeof(*info) );
  return FALSE;
}
//=============================================================================================
/**
 * レベル100になるまでに必要な経験値を取得
 *
 * @param   bpp
 *
 * @retval  u32
 */
//=============================================================================================
u32 BPP_GetExpMargin( const BTL_POKEPARAM* bpp )
{
  return POKETOOL_GetMinExp( bpp->coreParam.monsno, bpp->formNo, 100 );
}

//=============================================================================================
/**
 * レベルアップパラメータを反映させる（クライアント用）
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectLevelup( BTL_POKEPARAM* bpp, u8 nextLevel, u8 hpMax, u8 atk, u8 def, u8 spAtk, u8 spDef, u8 agi )
{
  bpp->coreParam.exp = POKETOOL_GetMinExp( bpp->coreParam.monsno, bpp->formNo, nextLevel );
  bpp->coreParam.hp += hpMax;
  bpp->baseParam.level = nextLevel;
  bpp->baseParam.hpMax      += hpMax;
  bpp->baseParam.attack     += atk;
  bpp->baseParam.defence    += def;
  bpp->baseParam.sp_attack  += spAtk;
  bpp->baseParam.sp_defence += spDef;
  bpp->baseParam.agility    += agi;
}
//=============================================================================================
/**
 * 経験値アップを反映させる（クライアント用）
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectExpAdd( BTL_POKEPARAM* bpp )
{
  POKEMON_PARAM* pp = (POKEMON_PARAM*)(bpp->coreParam.ppSrc);
  bpp->coreParam.exp = PP_Get( pp, ID_PARA_exp, NULL );
}

//=============================================================================================
/**
 * Srcポケモンデータに現在のパラメータを反映
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectToPP( BTL_POKEPARAM* bpp )
{
  POKEMON_PARAM* pp = (POKEMON_PARAM*)(bpp->coreParam.ppSrc);
  u32 i;

  PP_Put( pp, ID_PARA_exp, bpp->coreParam.exp );
  PP_Put( pp, ID_PARA_hp, bpp->coreParam.hp );
  if( bpp->coreParam.hp )
  {
    PokeSick  sick = BPP_GetPokeSick( bpp );
    PP_SetSick( pp, sick );
  }

  for(i=0; i<PTL_WAZA_MAX; ++i)
  {
    PP_SetWazaPos( pp, bpp->waza[i].number, i );
    PP_Put( pp, ID_PARA_pp1+i, bpp->waza[i].pp );
  }

  PP_Put( pp, ID_PARA_item, bpp->coreParam.item );
  PP_Put( pp, ID_PARA_form_no, bpp->formNo );
}
//=============================================================================================
/**
 * Srcポケモンデータからパラメータを再構築
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectByPP( BTL_POKEPARAM* bpp )
{
  setupBySrcData( bpp, bpp->coreParam.ppSrc );
}
//=============================================================================================
/**
 * フェイクのPPが有効な状態かどうか判定
 *
 * @param   bpp
 */
//=============================================================================================
BOOL BPP_IsFakeEnable( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.fFakeEnable;
}
//=============================================================================================
/**
 * フェイクのPPを無効にする
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_FakeDisable( BTL_POKEPARAM* bpp )
{
  bpp->coreParam.fFakeEnable = FALSE;
}


//=============================================================================================
/**
 * Srcポケモンデータポインタを差し替える
 *
 * @param   bpp
 * @param   pp
 */
//=============================================================================================
void BPP_SetSrcPP( BTL_POKEPARAM* bpp, POKEMON_PARAM* pp )
{
  {
    u16 monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    GF_ASSERT_MSG( (monsno == bpp->coreParam.monsno), "bppMonsNo=%d, ppMonsNo=%d", bpp->coreParam.monsno, monsno );
  }
  bpp->coreParam.ppSrc = pp;
}

//---------------------------------------------------------------------------------------------
// bitフラグバッファ処理
//---------------------------------------------------------------------------------------------
static inline void flgbuf_clear( u8* buf, u32 size )
{
  GFL_STD_MemClear( buf, size );
}
static inline void flgbuf_set( u8* buf, u32 flagID )
{
  {
    u8 byte = flagID / 8;
    u8 bit = (1 << (flagID%8));
    buf[ byte ] |= bit;
  }
}
static inline void flgbuf_reset( u8* buf, u32 flagID )
{
  {
    u8 byte = flagID / 8;
    u8 bit = (1 << (flagID%8));
    buf[ byte ] &= (~bit);
  }
}
static inline BOOL flgbuf_get( const u8* buf, u32 flagID )
{
//  GF_ASSERT(flagID<BPP_TURNFLG_MAX);
  {
    u8 byte = flagID / 8;
    u8 bit = (1 << (flagID%8));
    return (buf[ byte ] & bit) != 0;
  }
}

//---------------------------------------------------------------------------------------------
// 変身用ワーク処理
//---------------------------------------------------------------------------------------------

BOOL BPP_HENSIN_Set( BTL_POKEPARAM* bpp, const BTL_POKEPARAM* target )
{
  if( (bpp->coreParam.fHensin == FALSE)
  &&  (target->coreParam.fHensin == FALSE)
  &&  (target->migawariHP == 0)
  ){
    static BPP_CORE_PARAM  core = {0};
    int i;

    core = bpp->coreParam;

    *bpp = *target;
    bpp->coreParam = core;

    // ワザのMAXPPは最大5に揃える
    for(i=0; i<PTL_WAZA_MAX; ++i)
    {
      if( bpp->waza[i].number != WAZANO_NULL )
      {
        if( bpp->waza[i].ppMax > 5 ){
          bpp->waza[i].ppMax = 5;
        }
        bpp->waza[i].pp = bpp->waza[i].ppMax;
        bpp->waza[i].usedFlag = FALSE;
      }
    }

    clearWazaSickWork( bpp, TRUE );
    flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );
    flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

    bpp->appearedTurn = TURNCOUNT_NULL;
    bpp->turnCount = 0;
    bpp->prevSelectWazaID = WAZANO_NULL;
    bpp->prevActWazaID = WAZANO_NULL;
    bpp->wazaContCounter = 0;

    bpp->coreParam.fHensin = TRUE;

    return TRUE;
  }

  return FALSE;
}

//---------------------------------------------------------------------------------------------
// みがわり関連
//---------------------------------------------------------------------------------------------

void BPP_MIGAWARI_Create( BTL_POKEPARAM* bpp, u16 migawariHP )
{
  GF_ASSERT(bpp->migawariHP==0);
  bpp->migawariHP = migawariHP;
}
void BPP_MIGAWARI_Delete( BTL_POKEPARAM* bpp )
{
  bpp->migawariHP = 0;
}
BOOL BPP_MIGAWARI_IsExist( const BTL_POKEPARAM* bpp )
{
  return bpp->migawariHP != 0;
}

BOOL BPP_MIGAWARI_AddDamage( BTL_POKEPARAM* bpp, u16 damage )
{
  GF_ASSERT(bpp->migawariHP!=0);

  if( bpp->migawariHP <= damage ){
    bpp->migawariHP = 0;
    return TRUE;
  }else{
    bpp->migawariHP -= damage;
    return FALSE;
  }
}

//---------------------------------------------------------------------------------------------
// 対面記録関連
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/**
 * 初期化
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void ConfrontRec_Clear( BTL_POKEPARAM* bpp )
{
  bpp->confrontRecCount = 0;
}
//=============================================================================================
/**
 * 対面記録セット
 *
 * @param   bpp
 * @param   pokeID    対面したポケモンID
 */
//=============================================================================================
void BPP_CONFRONT_REC_Set( BTL_POKEPARAM* bpp, u8 pokeID )
{
  u32 i;
  for(i=0; i<bpp->confrontRecCount; ++i)
  {
    if( bpp->confrontRec[i] == pokeID ){
      return;
    }
  }
  if( i < NELEMS(bpp->confrontRec) ){
    BTL_Printf("ポケ[%d]が[%d]と対面した\n", bpp->coreParam.myID, pokeID);
    bpp->confrontRec[i] = pokeID;
    bpp->confrontRecCount++;
  }
}
//=============================================================================================
/**
 * 対面記録の件数取得
 *
 * @param   bpp
 *
 * @retval  u8    記録件数
 */
//=============================================================================================
u8 BPP_CONFRONT_REC_GetCount( const BTL_POKEPARAM* bpp )
{
  return bpp->confrontRecCount;
}
//=============================================================================================
/**
 * 対面記録されているポケモンIDを取得
 *
 * @param   bpp
 * @param   idx   インデックス（0 <= idx < BPP_CONFRONT_REC_GetCount() )
 *
 * @retval  u8    ポケモンID
 */
//=============================================================================================
u8 BPP_CONFRONT_REC_GetPokeID( const BTL_POKEPARAM* bpp, u8 idx )
{
  if( idx < bpp->confrontRecCount ){
    return bpp->confrontRec[ idx ];
  }
  return BTL_POKEID_NULL;
}


//---------------------------------------------------------------------------------------------
// とくせい・アイテム参照（いえき、さしおさえ等の影響を考慮して、直接参照しないようにするため）
//---------------------------------------------------------------------------------------------
static inline BOOL IsMatchTokusei( const BTL_POKEPARAM* bpp, PokeTokusei tokusei )
{
  if( BPP_CheckSick(bpp, WAZASICK_IEKI) ){
    return FALSE;
  }
  return (bpp->tokusei == tokusei);
}
static inline BOOL IsMatchItem( const BTL_POKEPARAM* bpp, u16 itemID )
{
  return (bpp->coreParam.item == itemID);
}

