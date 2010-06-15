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
#include "btl_tables.h"

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
 *  ワザ
 */
//--------------------------------------------------------------
typedef struct {
  u16  number;            ///< ワザナンバー
  u8   pp;                ///< PP値
  u8   ppMax;             ///< PP最大値
  u8   ppCnt;             ///< PP増加数
  u8   usedFlag : 4;      ///< 使用したフラグ（死亡・入れ替えなどでクリア）
  u8   usedFlagFix : 4;   ///< 使用したフラグ（死亡・入れ替えなどでも保持）
}BPP_WAZA_CORE;

typedef struct {
  BPP_WAZA_CORE  truth;   ///< 真ワザワーク
  BPP_WAZA_CORE  surface; ///< 仮ワザワーク
  u8             fLinked; ///< 仮=真である状態ならtrue
}BPP_WAZA;

//--------------------------------------------------------------
/**
 *  コアパラメータ  - へんしんしても書き換わらない要素 -
 */
//--------------------------------------------------------------
typedef struct {
  POKEMON_PARAM*  ppSrc;
  const POKEMON_PARAM*  ppFake;

  u32   exp;
  u16   monsno;
  u16   hpMax;        ///< 最大HP
  u16   hp;
  u16   item;
  u16   usedItem;
  u16   defaultTokusei;
  u8    level;
  u8    myID;
  u8    defaultFormNo;
  u8    fHensin     : 1;
  u8    fFakeEnable : 1;
  u8    fBtlIn      : 1;

  BPP_SICK_CONT   sickCont[ WAZASICK_MAX ];
  u8  wazaSickCounter[ WAZASICK_MAX ];

  u8  confrontRecCount;
  u8  confrontRec[ BTL_POKEID_MAX ];

}BPP_CORE_PARAM;

//--------------------------------------------------------------
/**
 *  基本パラメータ
 */
//--------------------------------------------------------------
typedef struct {

  u16 monsno;       ///< ポケモンナンバー

  u16 attack;       ///< こうげき
  u16 defence;      ///< ぼうぎょ
  u16 sp_attack;    ///< とくこう
  u16 sp_defence;   ///< とくぼう
  u16 agility;      ///< すばやさ

//  u8  level;        ///< レベル
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

struct _BTL_POKEPARAM {

  BPP_CORE_PARAM      coreParam;

  BPP_BASE_PARAM      baseParam;
  BPP_VARIABLE_PARAM  varyParam;
  BPP_WAZA            waza[ PTL_WAZA_MAX ];

  u16  tokusei;
  u16  weight;

  u8  wazaCnt;
  u8  formNo;
  u8  criticalRank;
  u8  usedWazaCount;
  u8  prevWazaType;

  u16 turnCount;        ///< 継続して戦闘に出ているカウンタ
  u16 appearedTurn;     ///< 戦闘に出たターンを記録
  u16 prevSelectWazaID;       ///< 直前に選択したワザID（WAZALOCK状態等で強制的に選択されたワザ > クライアントが選択した所持ワザ）
  u16 prevActWazaID;          ///< 直前に実行されたワザID（派生ワザを使った場合にprevSelectWazaIDと異なる）
  u16 wazaContCounter;        ///< 同ワザ連続成功カウンタ（prevActWazaIDのワザを何回連続で成功させたか。直前失敗ならゼロ）
  u16 prevDamagedWaza;        ///< 直前にダメージを受けたワザ
  BtlPokePos  prevTargetPos;  ///< 直前に狙った相手

  u8  turnFlag[ TURNFLG_BUF_SIZE ];
  u8  contFlag[ CONTFLG_BUF_SIZE ];
  u8  counter[ BPP_COUNTER_MAX ];
  BPP_WAZADMG_REC  wazaDamageRec[ WAZADMG_REC_TURN_MAX ][ WAZADMG_REC_MAX ];
  u8               dmgrecCount[ WAZADMG_REC_TURN_MAX ];
  u8               dmgrecTurnPtr;
  u8               dmgrecPtr;

  u16 migawariHP;

  WazaID  combiWazaID;
  u8      combiPokeID;
//  u32 dmy;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void setupBySrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP, BOOL fReflectHP );
static u32 WazaWorkSys_SetupBySrcPP( BTL_POKEPARAM* bpp, const POKEMON_PARAM* pp_src, BOOL fLinkSurface );
static void WazaWorkSys_ReflectToPP( BTL_POKEPARAM* bpp );
static void WazaWorkSys_ReflectFromPP( BTL_POKEPARAM* bpp );
static void WazaWork_ClearUsedFlag( BPP_WAZA* waza );
static void WazaWork_UpdateNumber( BPP_WAZA* waza, WazaID nextNumber, u8 ppMax, BOOL fPermenent );
static void WazaCore_UpdateNumber( BPP_WAZA_CORE* core, WazaID nextID, u8 ppMax );
static BOOL WazaCore_SetupByPP( BPP_WAZA_CORE* core, POKEMON_PARAM* pp, u8 index );
static void setupBySrcDataBase( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP );
static void clearHensin( BTL_POKEPARAM* bpp );
static void reflectWazaPP( BTL_POKEPARAM* bpp );
static void clearUsedWazaFlag( BTL_POKEPARAM* bpp );
static void clearCounter( BTL_POKEPARAM* bpp );
static void Effrank_Init( BPP_VARIABLE_PARAM* rank );
static void Effrank_Reset( BPP_VARIABLE_PARAM* rank );
static BOOL Effrank_Recover( BPP_VARIABLE_PARAM* rank );
static void splitTypeCore( const BTL_POKEPARAM* bpp, PokeType* type1, PokeType* type2 );
static BppValueID ConvertValueID( const BTL_POKEPARAM* bpp, BppValueID vid );
static const s8* getRankVaryStatusConst( const BTL_POKEPARAM* bpp, BppValueID type, s8* min, s8* max );
static s8* getRankVaryStatus( BTL_POKEPARAM* pp, BppValueID type, s8* min, s8* max );
static void cureDependSick( BTL_POKEPARAM* bpp, WazaSick sickID  );
static void clearWazaSickWork( BTL_POKEPARAM* bpp, BOOL fPokeSickInclude );
static void dmgrecClearWork( BTL_POKEPARAM* bpp );
static void dmgrecFwdTurn( BTL_POKEPARAM* bpp );
static inline void flgbuf_clear( u8* buf, u32 size );
static inline void flgbuf_set( u8* buf, u32 flagID );
static inline void flgbuf_reset( u8* buf, u32 flagID );
static inline BOOL flgbuf_get( const u8* buf, u32 flagID );
static void ConfrontRec_Clear( BTL_POKEPARAM* bpp );
static inline BOOL IsMatchTokusei( const BTL_POKEPARAM* bpp, PokeTokusei tokusei );



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
BTL_POKEPARAM*  BTL_POKEPARAM_Create( POKEMON_PARAM* pp, u8 pokeID, HEAPID heapID )
{
  BTL_POKEPARAM* bpp = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_POKEPARAM) );

  bpp->coreParam.myID = pokeID;
  bpp->coreParam.monsno = PP_Get( pp, ID_PARA_monsno, 0 );
  bpp->coreParam.ppSrc = pp;
  bpp->coreParam.hp = PP_Get( pp, ID_PARA_hp, 0 );
  bpp->coreParam.hpMax = PP_Get( pp, ID_PARA_hpmax, 0 );
  bpp->coreParam.item = PP_Get( pp, ID_PARA_item, NULL );
  bpp->coreParam.usedItem = ITEM_DUMMY_DATA;
  bpp->coreParam.fHensin = FALSE;
  bpp->coreParam.ppFake = NULL;
  bpp->coreParam.fFakeEnable = NULL;
  bpp->coreParam.fBtlIn = NULL;
  bpp->coreParam.defaultFormNo = PP_Get( pp, ID_PARA_form_no, NULL );
  bpp->coreParam.defaultTokusei = PP_Get( pp, ID_PARA_speabino, 0 );
  bpp->coreParam.level = PP_Get( pp, ID_PARA_level, 0 );

  setupBySrcData( bpp, pp, TRUE );

  // 所有ワザデータ初期化
  bpp->wazaCnt = WazaWorkSys_SetupBySrcPP( bpp, pp, TRUE );
  bpp->usedWazaCount = 0;

  BTL_Printf("setup pokeID=%d, monsno=%d, ppSrc=%p\n", pokeID, bpp->coreParam.monsno, pp );

  // ランク効果初期化
  Effrank_Init( &bpp->varyParam );

  // 状態異常ワーク初期化
  clearWazaSickWork( bpp, TRUE );

  // この時点でポケモン用状態異常になっていれば引き継ぎ
  {
    PokeSick sick = PP_GetSick( pp );
    if( sick != POKESICK_NULL )
    {
      bpp->coreParam.sickCont[ sick ] = BTL_CALC_MakeDefaultPokeSickCont( sick );
      bpp->coreParam.wazaSickCounter[ sick ] = 0;
    }
  }

  // 各種ワーク領域初期化
  bpp->appearedTurn = TURNCOUNT_NULL;
  bpp->turnCount = 0;
  bpp->migawariHP = 0;
  bpp->prevSelectWazaID = WAZANO_NULL;
  bpp->prevActWazaID    = WAZANO_NULL;
  bpp->prevWazaType     = POKETYPE_NULL;
  bpp->wazaContCounter   = 0;
  bpp->combiWazaID = WAZANO_NULL;
  bpp->combiPokeID = BTL_POKEID_NULL;
  bpp->criticalRank = 0;

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
static void setupBySrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP, BOOL fReflectHP )
{
  u16 monsno = bpp->coreParam.monsno;
  int i;

  // 2010.4.15 HP反映（レベルアップ＆死亡でHPが残ってしまう問題に対処）
  if( fReflectHP ){
    bpp->coreParam.hp = PP_Get( srcPP, ID_PARA_hp, 0 );
    bpp->coreParam.hpMax = PP_Get( srcPP, ID_PARA_hpmax, 0 );
  }
  bpp->coreParam.exp = PP_Get( srcPP, ID_PARA_exp, NULL );

  // 基本パラメタ初期化
  setupBySrcDataBase( bpp, srcPP );

  bpp->tokusei = PP_Get( srcPP, ID_PARA_speabino, 0 );
  bpp->formNo = PP_Get( srcPP, ID_PARA_form_no, 0 );

  bpp->weight = POKETOOL_GetPersonalParam( bpp->coreParam.monsno, bpp->formNo, POKEPER_ID_weight );
  if( bpp->weight < BTL_POKE_WEIGHT_MIN ){
    bpp->weight = BTL_POKE_WEIGHT_MIN;
  }
}
/**
 *  srcPPを元にワザデータを設定
 */
static u32 WazaWorkSys_SetupBySrcPP( BTL_POKEPARAM* bpp, const POKEMON_PARAM* pp_src, BOOL fLinkSurface )
{
  POKEMON_PARAM* pp = (POKEMON_PARAM*)pp_src;
  u8 fastFlag = PP_FastModeOn( pp );
  u32 i, cnt = 0;

  if( fLinkSurface )
  {
    for(i=0; i<PTL_WAZA_MAX; i++)
    {
      bpp->waza[i].truth.usedFlag = FALSE;
      bpp->waza[i].truth.usedFlagFix = FALSE;
    }
  }

  for(i=0; i<PTL_WAZA_MAX; i++)
  {
    if( WazaCore_SetupByPP( &bpp->waza[i].truth, pp, i ) ){
      ++cnt;
    }
  }

  if( fLinkSurface )
  {
    for(i=0; i<PTL_WAZA_MAX; ++i){
      bpp->waza[i].surface = bpp->waza[i].truth;
      bpp->waza[i].fLinked = TRUE;
    }
  }

  PP_FastModeOff( (POKEMON_PARAM*)pp, fastFlag );
  return cnt;
}
/**
 *  srcPPにワザデータを反映させる
 */
static void WazaWorkSys_ReflectToPP( BTL_POKEPARAM* bpp )
{
  POKEMON_PARAM* pp = (POKEMON_PARAM*)(bpp->coreParam.ppSrc);

  u32 i;
  for(i=0; i<PTL_WAZA_MAX; ++i)
  {
    PP_SetWazaPos( pp, bpp->waza[i].truth.number, i );
    PP_Put( pp, ID_PARA_pp1+i, bpp->waza[i].truth.pp );
    PP_Put( pp, ID_PARA_pp_count1+i, bpp->waza[i].truth.ppCnt );
  }
}
/**
 *  srcPP のワザデータを使って再設定（レベルアップ用）
 */
static void WazaWorkSys_ReflectFromPP( BTL_POKEPARAM* bpp )
{
  POKEMON_PARAM* pp = (POKEMON_PARAM*)(bpp->coreParam.ppSrc);
  u32 i;

  bpp->wazaCnt = 0;

  for(i=0; i<PTL_WAZA_MAX; ++i)
  {
    if( WazaCore_SetupByPP( &(bpp->waza[i].truth), pp, i) ){
      bpp->wazaCnt++;
    }
    if( bpp->waza[i].fLinked ){
      bpp->waza[i].surface = bpp->waza[i].truth;
    }
  }
}
static void WazaWorkSys_ClearSurface( BTL_POKEPARAM* bpp )
{
  u32 i;

  bpp->wazaCnt = 0;
  for(i=0; i<PTL_WAZA_MAX; ++i)
  {
    bpp->waza[i].surface = bpp->waza[i].truth;
    if( bpp->waza[i].surface.number != WAZANO_NULL ){
      bpp->wazaCnt++;
    }
    bpp->waza[i].fLinked = TRUE;
  }
}

static void WazaWork_ClearUsedFlag( BPP_WAZA* waza )
{
  waza->surface.usedFlag = FALSE;
  waza->truth.usedFlag = FALSE;
}


static void WazaWork_UpdateNumber( BPP_WAZA* waza, WazaID nextNumber, u8 ppMax, BOOL fPermenent )
{
  // スケッチ用
  if( fPermenent )
  {
    // 真ワザワークを更新、リンク中なら仮ワザワークも連動させる
    WazaCore_UpdateNumber( &waza->truth, nextNumber, ppMax );
    if( waza->fLinked ){
      waza->surface = waza->truth;
    }
  }
  // ものまね・へんしん用
  else
  {
    // 仮ワザワークのみを更新、リンクを切る
    BTL_N_Printf( DBGSTR_BPP_WazaLinkOff, nextNumber );
    WazaCore_UpdateNumber( &waza->surface, nextNumber, ppMax );
    waza->fLinked = FALSE;
  }
}
/**
 *  ppMax  0ならデフォルト値
 */
static void WazaCore_UpdateNumber( BPP_WAZA_CORE* core, WazaID nextID, u8 ppMax )
{
  core->number = nextID;
  core->usedFlag = FALSE;
  core->usedFlagFix = FALSE;

  if( nextID != WAZANO_NULL ){
    core->ppMax = WAZADATA_GetMaxPP( nextID, 0 );
  }
  else{
    core->ppMax = 0;
  }

  if( (ppMax !=0)
  &&  (core->ppMax > ppMax)
  ){
    core->ppMax = ppMax;
  }
  core->pp = core->ppMax;
}
static BOOL WazaCore_SetupByPP( BPP_WAZA_CORE* core, POKEMON_PARAM* pp, u8 index )
{
  BOOL fExist = TRUE;
  WazaID next_number;

  next_number = PP_Get( pp, ID_PARA_waza1+index, NULL );

  if( core->number != next_number )
  {
    TAYA_Printf("使用フラグ再設定によるクリア\n");
    core->usedFlag = FALSE;
    core->usedFlagFix = FALSE;
  }

  core->number = next_number;
  if( core->number != WAZANO_NULL )
  {
    core->pp = PP_Get( pp, ID_PARA_pp1+index, 0 );
    core->ppMax = PP_Get( pp, ID_PARA_pp_max1+index, 0 );
    core->ppCnt = PP_Get( pp, ID_PARA_pp_count1+index, 0 );
  }
  else{
    core->pp = 0;
    core->ppMax = 0;
    core->ppCnt = 0;
    fExist = FALSE;
  }


  return fExist;
}


//----------------------------------------------------------------------------------
/**
 * 元になるポケモンパラメータ構造体からバトルパラメータ部分のみ構築
 *
 * @param   bpp
 * @param   srcPP
 */
//----------------------------------------------------------------------------------
static void setupBySrcDataBase( BTL_POKEPARAM* bpp, const POKEMON_PARAM* srcPP )
{
  bpp->baseParam.type1 = PP_Get( srcPP, ID_PARA_type1, 0 );
  bpp->baseParam.type2 = PP_Get( srcPP, ID_PARA_type2, 0 );
  bpp->baseParam.sex = PP_GetSex( srcPP );
//  bpp->baseParam.level = PP_Get( srcPP, ID_PARA_level, 0 );
  bpp->baseParam.attack = PP_Get( srcPP, ID_PARA_pow, 0 );
  bpp->baseParam.defence = PP_Get( srcPP, ID_PARA_def, 0 );
  bpp->baseParam.sp_attack = PP_Get( srcPP, ID_PARA_spepow, 0 );
  bpp->baseParam.sp_defence = PP_Get( srcPP, ID_PARA_spedef, 0 );
  bpp->baseParam.agility = PP_Get( srcPP, ID_PARA_agi, 0 );
}


//----------------------------------------------------------------------------------
/**
 * へんしん状態のクリア
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void clearHensin( BTL_POKEPARAM* bpp )
{
  if( bpp->coreParam.fHensin )
  {
    POKEMON_PARAM* ppSrc = (POKEMON_PARAM*)(bpp->coreParam.ppSrc);

    setupBySrcData( bpp, ppSrc, FALSE );
    WazaWorkSys_ClearSurface( bpp );

    bpp->coreParam.fHensin = FALSE;
  }
}
//----------------------------------------------------------------------------------
/**
 * へんしん状態でなければワザPP値を ppSrc に反映
 *
 * @param   bpp
 */
//----------------------------------------------------------------------------------
#if 0
static void reflectWazaPP( BTL_POKEPARAM* bpp )
{
  if( !(bpp->coreParam.fHensin) )
  {
    u32 i;
    for(i=0; i<NELEMS(bpp->waza); ++i)
    {
      if( bpp->waza[i].number != WAZANO_NULL )
      {
        PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_pp1+i, bpp->waza[i].pp );
      }
    }
  }
}
#endif

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
    WazaWork_ClearUsedFlag( &bpp->waza[i] );
  }

  bpp->prevSelectWazaID = WAZANO_NULL;
  bpp->prevActWazaID    = WAZANO_NULL;
  bpp->prevWazaType     = POKETYPE_NULL;
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

u8 BPP_GetID( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.myID;
}

u16 BPP_GetMonsNo( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.monsno;
}


/**
 *  ワザ所持数を返す（仮ワザワーク）
 */
u8 BPP_WAZA_GetCount( const BTL_POKEPARAM* bpp )
{
  return bpp->wazaCnt;
}
/**
 *  ワザ所持数を返す（真ワザワーク）
 */
u8 BPP_WAZA_GetCount_Org( const BTL_POKEPARAM* bpp )
{
  u32 cnt, i;
  for(i=0, cnt=0; i<PTL_WAZA_MAX; ++i)
  {
    if( bpp->waza[i].truth.number != WAZANO_NULL ){
      ++cnt;
    }else{
      break;
    }
  }
  return cnt;
}

/**
 *  生きている間に使ったワザの数を返す（死んだらリセットされる）
 */
u8 BPP_WAZA_GetUsedCountInAlive( const BTL_POKEPARAM* bpp )
{
  u8 cnt, i;
  for(i=0, cnt=0; i<bpp->wazaCnt; ++i)
  {
    if( bpp->waza[i].surface.usedFlag ){
      ++cnt;
    }
  }
  return cnt;
}

/**
 *  使ったワザの数を返す（死んでもカウントアップ）
 */
u8 BPP_WAZA_GetUsedCount( const BTL_POKEPARAM* bpp )
{
  return bpp->usedWazaCount;
}

u8 BPP_WAZA_GetUsableCount( const BTL_POKEPARAM* bpp )
{
  u8 cnt, i;
  for(i=0, cnt=0; i<bpp->wazaCnt; ++i)
  {
    if( bpp->waza[i].surface.pp ){
      ++cnt;
    }
  }
  return cnt;
}

WazaID BPP_WAZA_GetID( const BTL_POKEPARAM* bpp, u8 idx )
{
  GF_ASSERT(idx < bpp->wazaCnt);
  return bpp->waza[idx].surface.number;
}
WazaID BPP_WAZA_GetID_Org( const BTL_POKEPARAM* bpp, u8 idx )
{
  return bpp->waza[idx].truth.number;
}
BOOL BPP_WAZA_CheckUsedInAlive( const BTL_POKEPARAM* bpp, u8 idx )
{
  GF_ASSERT(idx < bpp->wazaCnt);
  return bpp->waza[idx].surface.usedFlag;
}

void BPP_WAZA_Copy( const BTL_POKEPARAM* bppSrc, BTL_POKEPARAM* bppDst )
{
  u32 i;
  for(i=0; i<NELEMS(bppDst->waza); ++i){
    bppDst->waza[i] = bppSrc->waza[i];
  }
  bppDst->wazaCnt = bppSrc->wazaCnt;
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

  *PP = bpp->waza[idx].surface.pp;
  *PPMax = bpp->waza[idx].surface.ppMax;

  return bpp->waza[idx].surface.number;
}

//=============================================================================================
/**
 * [ワザパラメータ] PP不足分を取得（仮ワザワーク）
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
  return (bpp->waza[idx].surface.ppMax - bpp->waza[idx].surface.pp);
}
//=============================================================================================
/**
 * [ワザパラメータ] PP不足分を取得（真ワザワーク）
 *
 * @param   bpp
 * @param   idx       対象ワザインデックス
 *
 * @retval  u8        PP不足分
 */
//=============================================================================================
u8 BPP_WAZA_GetPPShort_Org( const BTL_POKEPARAM* bpp, u8 idx )
{
  GF_ASSERT(bpp->waza[idx].truth.number != WAZANO_NULL);
  return (bpp->waza[idx].truth.ppMax - bpp->waza[idx].truth.pp);
}
//=============================================================================================
/**
 * ワザPP値を取得（仮ワザワーク）
 *
 * @param   pp
 * @param   wazaIdx
 *
 */
//=============================================================================================
u16 BPP_WAZA_GetPP( const BTL_POKEPARAM* bpp, u8 wazaIdx )
{
  if( wazaIdx < bpp->wazaCnt ){
    return  bpp->waza[wazaIdx].surface.pp;
  }else{
    GF_ASSERT_MSG(0,"wazaIdx:%d, wazaCnt:%d", wazaIdx, bpp->wazaCnt);
    return 0;
  }
}
//=============================================================================================
/**
 * ワザPP値をワザIDから取得（仮ワザワーク）
 *
 * @param   pp
 * @param   wazaIdx
 *
 */
//=============================================================================================
u16 BPP_WAZA_GetPP_ByNumber( const BTL_POKEPARAM* bpp, WazaID waza )
{
  u32 i;
  for(i=0; i<bpp->wazaCnt; ++i)
  {
    if( bpp->waza[i].surface.number == waza ){
      return bpp->waza[i].surface.pp;
    }
  }
  return 0;
}
//=============================================================================================
/**
 * ワザPP値を取得（真ワザワーク）
 *
 * @param   pp
 * @param   wazaIdx
 *
 */
//=============================================================================================
u16 BPP_WAZA_GetPP_Org( const BTL_POKEPARAM* bpp, u8 wazaIdx )
{
  GF_ASSERT(bpp->waza[wazaIdx].truth.number != WAZANO_NULL);
  return  bpp->waza[wazaIdx].truth.pp;
}

//=============================================================================================
/**
 * ワザPP値に回復の余地があるか判定
 *
 * @param   pp
 * @param   wazaIdx
 * @param   fOrg    TRUEなら真ワザワークの方を調べる
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_WAZA_IsPPFull( const BTL_POKEPARAM* bpp, u8 wazaIdx, BOOL fOrg )
{
  {
    const BPP_WAZA_CORE* core = (fOrg)? &(bpp->waza[wazaIdx].truth) : &(bpp->waza[wazaIdx].surface);

    if( (core->number != WAZANO_NULL)
    &&  (core->pp == core->ppMax)
    ){
      return TRUE;
    }
    return FALSE;
  }
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
void BPP_WAZA_DecrementPP( BTL_POKEPARAM* bpp, u8 wazaIdx, u8 value )
{
  GF_ASSERT(wazaIdx < bpp->wazaCnt);

  if( bpp->waza[wazaIdx].surface.pp >= value )
  {
    bpp->waza[wazaIdx].surface.pp -= value;
  }
  else
  {
    bpp->waza[wazaIdx].surface.pp = 0;
  }

  if( bpp->waza[wazaIdx].fLinked )
  {
    bpp->waza[wazaIdx].truth.pp = bpp->waza[wazaIdx].surface.pp;
    PP_Put( bpp->coreParam.ppSrc, ID_PARA_pp1+wazaIdx, bpp->waza[wazaIdx].truth.pp );
  }
}
//=============================================================================================
/**
 * ワザPP値を減少（ものまね等、一時的に上書きされている場合の上書き前の部分）
 *
 * @param   bpp
 * @param   wazaIdx
 * @param   value
 */
//=============================================================================================
void BPP_WAZA_DecrementPP_Org( BTL_POKEPARAM* bpp, u8 wazaIdx, u8 value )
{
  if( bpp->waza[wazaIdx].fLinked == FALSE )
  {
    int pp =  bpp->waza[wazaIdx].truth.pp;
    pp -= value;
    if( pp < 0 ){
      pp = 0;
    }
    bpp->waza[wazaIdx].truth.pp = pp;
    PP_Put( bpp->coreParam.ppSrc, ID_PARA_pp1+wazaIdx, bpp->waza[wazaIdx].truth.pp );
  }
}
//=============================================================================================
/**
 *
 *
 * @param   bpp
 * @param   wazaIdx
 */
//=============================================================================================
void BPP_WAZA_SetUsedFlag_Org( BTL_POKEPARAM* bpp, u8 wazaIdx )
{
  if( bpp->waza[wazaIdx].fLinked == FALSE )
  {
    BPP_WAZA* pWaza = &bpp->waza[ wazaIdx ];

    if( pWaza->truth.usedFlagFix == FALSE )
    {
      pWaza->truth.usedFlagFix = TRUE;
      bpp->usedWazaCount++;
    }
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
 * @retval  WazaID    回復したワザナンバー
 */
//=============================================================================================
WazaID BPP_WAZA_IncrementPP( BTL_POKEPARAM* bpp, u8 wazaIdx, u8 value )
{
  GF_ASSERT(wazaIdx < PTL_WAZA_MAX);

  {
    BPP_WAZA* wp = &bpp->waza[wazaIdx];

    wp->surface.pp += value;
    if( wp->surface.pp > wp->surface.ppMax )
    {
      wp->surface.pp = wp->surface.ppMax;
    }

    if( wp->fLinked ){
      wp->truth.pp = wp->surface.pp;
    }

    return wp->surface.number;
  }
}
//=============================================================================================
/**
 * ワザPP値を増加（真ワザワーク用）
 *
 * @param   pp
 * @param   wazaIdx
 * @param   value
 *
 * @retval  WazaID    回復したワザナンバー
 */
//=============================================================================================
WazaID BPP_WAZA_IncrementPP_Org( BTL_POKEPARAM* bpp, u8 wazaIdx, u8 value )
{
  GF_ASSERT(wazaIdx < PTL_WAZA_MAX);

  {
    BPP_WAZA* wp = &bpp->waza[wazaIdx];

    wp->truth.pp += value;
    if( wp->truth.pp > wp->truth.ppMax )
    {
      wp->truth.pp = wp->truth.ppMax;
    }

    if( wp->fLinked ){
      wp->surface.pp = wp->truth.pp;
    }

    return wp->truth.number;
  }
}

//=============================================================================================
/**
 * ワザが一時上書き状態にあるか判定
 *
 * @param   bpp
 * @param   wazaIdx   インデックス
 * @param   orgWaza   上書き前のワザID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_WAZA_IsLinkOut( const BTL_POKEPARAM* bpp, u8 wazaIdx )
{
  if( bpp->waza[wazaIdx].fLinked == FALSE )
  {
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
/**
 * 使用したワザフラグを立てる
 *
 * @param   pp
 * @param   wazaIdx
 */
//=============================================================================================
void BPP_WAZA_SetUsedFlag( BTL_POKEPARAM* bpp, u8 wazaIdx )
{
  BPP_WAZA* pWaza = &bpp->waza[ wazaIdx ];

  if( pWaza->surface.usedFlagFix == FALSE )
  {
    pWaza->surface.usedFlagFix = TRUE;
    bpp->usedWazaCount++;
  }
  pWaza->surface.usedFlag = TRUE;

  {
    TAYA_Printf( "wazaIdx=%d, adrs=%p\n", wazaIdx, &(pWaza->surface) );
  }

  if( pWaza->fLinked ){
    pWaza->truth.usedFlag = pWaza->surface.usedFlag;
    pWaza->truth.usedFlagFix = pWaza->surface.usedFlagFix;
  }
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
void BPP_WAZA_UpdateID( BTL_POKEPARAM* bpp, u8 wazaIdx, WazaID waza, u8 ppMax, BOOL fPermenent )
{
  BPP_WAZA* pWaza = &bpp->waza[ wazaIdx ];

  WazaWork_UpdateNumber( &bpp->waza[wazaIdx], waza, ppMax, fPermenent );
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
    if( bpp->waza[i].surface.number == waza ){
      return TRUE;
    }
  }
  return FALSE;
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
      if( pp->waza[i].surface.number == waza ){
        return i;
      }
    }
  }
  return PTL_WAZA_MAX;
}








static void splitTypeCore( const BTL_POKEPARAM* bpp, PokeType* type1, PokeType* type2 )
{
  BOOL fHaneYasume = BPP_CheckSick( bpp, WAZASICK_HANEYASUME );

  // はねやすめで飛行タイプを無効にする
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

  // どちらか無効なら純粋タイプになる／元々純粋タイプならノーマルタイプになる
  if( (*type1) == POKETYPE_NULL )
  {
    if( (*type2) == POKETYPE_NULL ){
      *type2 = POKETYPE_NORMAL;
    }
    *type1 = *type2;
  }
  else if( (*type2) == POKETYPE_NULL )
  {
    *type2 = (*type1);
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
void BPP_ClearViewSrcData( BTL_POKEPARAM* bpp )
{
  bpp->coreParam.ppFake = NULL;
  bpp->coreParam.fFakeEnable = FALSE;
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
 * srcPPの各パラメータ取得（レベルアップ表示用）
 *
 * @param   bpp
 * @param   info
 */
//=============================================================================================
void BPP_GetPPStatus( const BTL_POKEPARAM* bpp, BTL_LEVELUP_INFO* info )
{
  u8 fastFlag = PP_FastModeOn( bpp->coreParam.ppSrc );

  info->hp     = PP_Get( bpp->coreParam.ppSrc, ID_PARA_hpmax, 0 );
  info->atk    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_pow, 0 );
  info->def    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_def, 0 );
  info->sp_atk = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spepow, 0 );
  info->sp_def = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spedef, 0 );
  info->agi    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_agi, 0 );

  PP_FastModeOff( bpp->coreParam.ppSrc, fastFlag );

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

  case BPP_LEVEL:           return bpp->coreParam.level;
  case BPP_SEX:             return bpp->baseParam.sex;

  case BPP_HP:              return bpp->coreParam.hp;
  case BPP_MAX_HP:          return bpp->coreParam.hpMax;

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
 * ワザ（そらをとぶ・あなをほる等）により画面から消えている状態チェック
 *
 * @param   bpp
 *
 * @retval  BppContFlag
 */
//=============================================================================================
BppContFlag  BPP_CONTFLAG_CheckWazaHide( const BTL_POKEPARAM* bpp )
{
  static const BppContFlag  hideState[] = {
    BPP_CONTFLG_SORAWOTOBU, BPP_CONTFLG_ANAWOHORU, BPP_CONTFLG_DIVING, BPP_CONTFLG_SHADOWDIVE,
  };
  u32 i;

  for(i=0; i<NELEMS(hideState); ++i)
  {
    if( BPP_CONTFLAG_Get(bpp, hideState[i]) )
    {
      return hideState[ i ];
    }
  }
  return BPP_CONTFLG_NULL;
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
  double r = (double)(pp->coreParam.hp * 100) / (double)(pp->coreParam.hpMax);
  return FX32_CONST( r );
}

//-----------------------------
static const s8* getRankVaryStatusConst( const BTL_POKEPARAM* bpp, BppValueID type, s8* min, s8* max )
{
  const s8* ptr;

  *min = BPP_RANK_STATUS_MIN;
  *max = BPP_RANK_STATUS_MAX;

  switch( type ) {
  case BPP_ATTACK_RANK:     ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE_RANK:    ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK_RANK:  ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE_RANK: ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY_RANK:    ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:       ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO:     ptr = &bpp->varyParam.avoid; break;

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
  case BPP_ATTACK_RANK:     ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE_RANK:    ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK_RANK:  ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE_RANK: ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY_RANK:    ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:       ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO:     ptr = &bpp->varyParam.avoid; break;

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
  case BPP_ATTACK_RANK:     ptr = &bpp->varyParam.attack; break;
  case BPP_DEFENCE_RANK:    ptr = &bpp->varyParam.defence; break;
  case BPP_SP_ATTACK_RANK:  ptr = &bpp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE_RANK: ptr = &bpp->varyParam.sp_defence; break;
  case BPP_AGILITY_RANK:    ptr = &bpp->varyParam.agility; break;
  case BPP_HIT_RATIO:       ptr = &bpp->varyParam.hit; break;
  case BPP_AVOID_RATIO:     ptr = &bpp->varyParam.avoid; break;

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
  case BPP_ATTACK_RANK:     ptr = &pp->varyParam.attack; break;
  case BPP_DEFENCE_RANK:    ptr = &pp->varyParam.defence; break;
  case BPP_SP_ATTACK_RANK:  ptr = &pp->varyParam.sp_attack; break;
  case BPP_SP_DEFENCE_RANK: ptr = &pp->varyParam.sp_defence; break;
  case BPP_AGILITY_RANK:    ptr = &pp->varyParam.agility; break;
  case BPP_HIT_RATIO:       ptr = &pp->varyParam.hit; break;
  case BPP_AVOID_RATIO:     ptr = &pp->varyParam.avoid; break;

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
  if( pp->coreParam.hp > pp->coreParam.hpMax )
  {
    pp->coreParam.hp = pp->coreParam.hpMax;
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
 * １ターン有効フラグのセット
 *
 * @param   pp
 * @param   flagID
 *
 */
//=============================================================================================
void BPP_TURNFLAG_Set( BTL_POKEPARAM* bpp, BppTurnFlag flagID )
{
  flgbuf_set( bpp->turnFlag, flagID );
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
void BPP_CONTFLAG_Set( BTL_POKEPARAM* bpp, BppContFlag flagID )
{
  flgbuf_set( bpp->contFlag, flagID );
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
    BPP_CurePokeSick( bpp );
  }

  bpp->coreParam.sickCont[ sick ] = contParam;
  bpp->coreParam.wazaSickCounter[sick] = 0;
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
  u32 i = 0;

  while( 1 )
  {
    sick = BTL_TABLES_GetTurnCheckWazaSickByOrder(i++);
    if( sick == WAZASICK_NULL ){
      break;
    }
    // ねむり・こんらんはアクション開始のタイミングで独自のカウンタデクリメント処理
    if( (sick == WAZASICK_NEMURI) || (sick == WAZASICK_KONRAN) ){
      continue;
    }
    if( bpp->coreParam.sickCont[sick].type != WAZASICK_CONT_NONE )
    {
      u32 turnMax = BPP_SICCONT_GetTurnMax( bpp->coreParam.sickCont[sick] );
      BOOL fCure = FALSE;
      BPP_SICK_CONT oldCont;

      oldCont = bpp->coreParam.sickCont[ sick ];

      // こだわりロックは、該当するワザを持っていないなら直る
      if( sick == WAZASICK_KODAWARI )
      {
        WazaID  waza = BPP_SICKCONT_GetParam( oldCont );
        if( !BPP_WAZA_IsUsable(bpp, waza) )
        {
          bpp->coreParam.sickCont[sick] = BPP_SICKCONT_MakeNull();;
          bpp->coreParam.wazaSickCounter[sick] = 0;
          fCure = TRUE;
        }
      }

      // 継続ターン経過チェック
      if( turnMax )
      {
        bpp->coreParam.wazaSickCounter[sick] += 1;

        if( bpp->coreParam.wazaSickCounter[sick] >= turnMax )
        {
          BTL_Printf("経過ターンが最大ターンを越えた\n");
          bpp->coreParam.sickCont[sick] = BPP_SICKCONT_MakeNull();;
          bpp->coreParam.wazaSickCounter[sick] = 0;
          fCure = TRUE;
        }
      }
      // 永続型で最大ターン数が指定されているものはカウンタをインクリメント
      // （現状、この機構を利用しているのは「どくどく」のみ）
      else if( bpp->coreParam.sickCont[sick].type == WAZASICK_CONT_PERMANENT )
      {
        if( (bpp->coreParam.sickCont[sick].permanent.count_max != 0 )
        &&  (bpp->coreParam.wazaSickCounter[sick] < bpp->coreParam.sickCont[sick].permanent.count_max)
        ){
          bpp->coreParam.wazaSickCounter[sick]++;
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
  BPP_SICK_CONT* cont = &(bpp->coreParam.sickCont[ WAZASICK_NEMURI ]);

  if( cont->type != WAZASICK_CONT_NONE )
  {
    u32 turnMax = BPP_SICCONT_GetTurnMax( *cont );
    if( turnMax )
    {
      // とくせい「はやおき」は眠りカウンタ２倍速
      u32 incValue = IsMatchTokusei(bpp, POKETOKUSEI_HAYAOKI)? 2 : 1;

      bpp->coreParam.wazaSickCounter[ WAZASICK_NEMURI ] += incValue;
      BTL_N_Printf( DBGSTR_BPP_NemuriWakeCheck, bpp->coreParam.myID, turnMax, bpp->coreParam.wazaSickCounter[ WAZASICK_NEMURI ] );
      if( bpp->coreParam.wazaSickCounter[ WAZASICK_NEMURI ] >= turnMax )
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
  BPP_SICK_CONT* cont = &(bpp->coreParam.sickCont[ SICK_ID ]);

  if( cont->type != WAZASICK_CONT_NONE )
  {
    u32 turnMax = BPP_SICCONT_GetTurnMax( *cont );
    if( turnMax )
    {
      bpp->coreParam.wazaSickCounter[ SICK_ID ]++;
      if( bpp->coreParam.wazaSickCounter[ SICK_ID ] >= turnMax )
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
    bpp->coreParam.sickCont[ i ] = BPP_SICKCONT_MakeNull();
    bpp->coreParam.wazaSickCounter[ i ] = 0;
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
    bpp->coreParam.sickCont[ WAZASICK_AKUMU ] = BPP_SICKCONT_MakeNull();    // 眠りが治れば“あくむ”も治る
    bpp->coreParam.wazaSickCounter[ WAZASICK_AKUMU ] = 0;
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
    bpp->coreParam.sickCont[ sick ] = BPP_SICKCONT_MakeNull();
    bpp->coreParam.wazaSickCounter[ sick ] = 0;
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
void BPP_CureWazaSickDependPoke( BTL_POKEPARAM* bpp, u8 depend_pokeID )
{
  if( depend_pokeID != BTL_POKEID_NULL )
  {
    BOOL fCure;
    u32 i;

    for(i=0; i<WAZASICK_MAX; ++i)
    {
      if( !BPP_SICKCONT_IsNull(bpp->coreParam.sickCont[i]) )
      {
        TAYA_Printf("状態異常[%d]をチェック。依存ポケ=%d\n", i, BPP_SICKCONT_GetPokeID(bpp->coreParam.sickCont[i]));

        if( BPP_SICKCONT_GetPokeID(bpp->coreParam.sickCont[i]) == depend_pokeID )
        {
         TAYA_Printf("治します\n");
          bpp->coreParam.sickCont[i] = BPP_SICKCONT_MakeNull();
          cureDependSick( bpp, i );
        }
      }
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
    if( pp->coreParam.sickCont[i].type != WAZASICK_CONT_NONE ){
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
  GF_ASSERT(sickType < NELEMS(bpp->coreParam.sickCont));

  if( bpp->coreParam.sickCont[ sickType ].type != WAZASICK_CONT_NONE )
  {
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
  GF_ASSERT(sick < NELEMS(pp->coreParam.sickCont));

  switch(  pp->coreParam.sickCont[sick].type ){
  case WAZASICK_CONT_TURN:
    return pp->coreParam.sickCont[sick].turn.param;

  case WAZASICK_CONT_POKE:
    return pp->coreParam.sickCont[sick].poke.ID;

  case WAZASICK_CONT_POKETURN:
    return pp->coreParam.sickCont[sick].poketurn.pokeID;

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
  GF_ASSERT(sick < NELEMS(bpp->coreParam.sickCont));
  return bpp->coreParam.sickCont[ sick ];
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
  GF_ASSERT(sick < NELEMS(bpp->coreParam.sickCont));
  return bpp->coreParam.wazaSickCounter[ sick ];
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
      if( BPP_SICKCONT_IsMoudokuCont(bpp->coreParam.sickCont[sick]) ){
        return BTL_CALC_QuotMaxHP(bpp, 16) * bpp->coreParam.wazaSickCounter[sick];
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

  for(i=start; i<NELEMS(bpp->coreParam.sickCont); ++i){
    bpp->coreParam.sickCont[i].raw = 0;
    bpp->coreParam.sickCont[i].type = WAZASICK_CONT_NONE;
  }
  GFL_STD_MemClear( bpp->coreParam.wazaSickCounter, sizeof(bpp->coreParam.wazaSickCounter) );
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
  bpp->coreParam.fBtlIn = TRUE;
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

  clearHensin( bpp );
  WazaWorkSys_ClearSurface( bpp );
  clearUsedWazaFlag( bpp );
  clearCounter( bpp );
  BPP_CombiWaza_ClearParam( bpp );
  BPP_FakeDisable( bpp );

  BPP_MIGAWARI_Delete( bpp );
  clearWazaSickWork( bpp, TRUE );
  Effrank_Init( &bpp->varyParam );

  bpp->formNo = bpp->coreParam.defaultFormNo;
  bpp->tokusei = bpp->coreParam.defaultTokusei;
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

  clearHensin( bpp );
  WazaWorkSys_ClearSurface( bpp );
  clearUsedWazaFlag( bpp );
  clearCounter( bpp );
  BPP_CombiWaza_ClearParam( bpp );
  BPP_FakeDisable( bpp );

  if( !BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_BATONTOUCH) )
  {
    BPP_MIGAWARI_Delete( bpp );
    clearWazaSickWork( bpp, FALSE );
    Effrank_Init( &bpp->varyParam );
    flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );
  }
  else{
    TAYA_Printf( "バトンタッチ使用者 pokeID=%d, atk=%d, def=%d\n",
      bpp->coreParam.myID, bpp->varyParam.attack, bpp->varyParam.defence );
  }

  bpp->formNo = bpp->coreParam.defaultFormNo;
  bpp->tokusei = bpp->coreParam.defaultTokusei;

  PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_form_no, bpp->formNo );
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
  setupBySrcData( bpp, bpp->coreParam.ppSrc, FALSE );
  flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

  BPP_MIGAWARI_Delete( bpp );

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
void BPP_BatonTouchParam( BTL_POKEPARAM* target, BTL_POKEPARAM* user )
{
  u32 i;

  target->varyParam = user->varyParam;

  TAYA_Printf("[%d]->[%d]へバトンタッチで引き継がれた:攻撃ランク=%d, 防御ランク=%d\n",
    user->coreParam.myID, target->coreParam.myID,
      target->varyParam.attack, target->varyParam.defence );

  target->migawariHP = user->migawariHP;

  // 特定の状態異常をそのまま引き継ぎ
  for(i=0; i<WAZASICK_MAX; ++i)
  {
    if( (user->coreParam.sickCont[i].type != WAZASICK_CONT_NONE)
    &&  (BTL_SICK_CheckBatonTouch(i))
    ){
      target->coreParam.sickCont[i] = user->coreParam.sickCont[i];
      target->coreParam.wazaSickCounter[i] = user->coreParam.wazaSickCounter[i];
    }
  }

  // 特定の継続フラグを引き継ぎ（今のところパワートリックだけ）
  if( BPP_CONTFLAG_Get(user, BPP_CONTFLG_POWERTRICK) ){
    u16 atk, def;
    BPP_CONTFLAG_Set(target, BPP_CONTFLG_POWERTRICK);

    atk = BPP_GetValue_Base( target, BPP_ATTACK );
    def = BPP_GetValue_Base( target, BPP_DEFENCE );
    BPP_SetBaseStatus( target, BPP_ATTACK, def );
    BPP_SetBaseStatus( target, BPP_DEFENCE, atk );
    BTL_Printf("パワートリック引き継ぎ: Atk(%d) <-> Def(%d)\n", atk, def);
  }

  // 受け継がせたらクリア
  BPP_MIGAWARI_Delete( user );
  clearWazaSickWork( user, FALSE );
  Effrank_Init( &user->varyParam );
  flgbuf_clear( user->contFlag, sizeof(user->contFlag) );
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
 * フォルム変更
 *
 * @param   bpp
 * @param   formNo
 */
//=============================================================================================
void BPP_ChangeForm( BTL_POKEPARAM* bpp, u8 formNo )
{
  bpp->formNo = formNo;

  PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_form_no, formNo );
  PP_Renew( (POKEMON_PARAM*)(bpp->coreParam.ppSrc) );
  setupBySrcDataBase( bpp, bpp->coreParam.ppSrc );

  if( (bpp->coreParam.monsno == MONSNO_SHEIMI) && (formNo == FORMNO_SHEIMI_LAND) ){
    bpp->coreParam.defaultFormNo = FORMNO_SHEIMI_LAND;
  }
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
void BPP_UpdateWazaProcResult( BTL_POKEPARAM* bpp, BtlPokePos actTargetPos, BOOL fActEnable, PokeType actWazaType, WazaID actWaza, WazaID orgWaza )
{
  WazaID  prevActWaza = bpp->prevActWazaID;

  bpp->prevSelectWazaID = orgWaza;
  bpp->prevActWazaID = actWaza;
  bpp->prevTargetPos = actTargetPos;
  bpp->prevWazaType  = actWazaType;

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

  BTL_N_Printf( DBGSTR_BPP_WazaRecord, bpp->coreParam.myID,
    bpp->prevSelectWazaID, bpp->prevActWazaID, bpp->prevTargetPos, bpp->prevWazaType, prevActWaza, bpp->wazaContCounter );

}

//=============================================================================================
/**
 * 同じワザを連続何回成功しているか返す（連続していない場合は0、連続中は1オリジンの値が返る）
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
 * 直近に実行されたワザタイプを返す（場に出てから使ったワザが無い場合は POKETYPE_NULL ）
 *
 * @param   bpp
 *
 * @retval  PokeType
 */
//=============================================================================================
PokeType BPP_GetPrevWazaType( const BTL_POKEPARAM* bpp )
{
  return bpp->prevWazaType;
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
BtlPokePos BPP_GetPrevTargetPos( const BTL_POKEPARAM* bpp )
{
  return bpp->prevTargetPos;
}
//=============================================================================================
/**
 * バトルに一度でも参加したかどうか判定
 *
 * @param   bpp
 *
 * @retval  BOOL    参加していた場合TRUE
 */
//=============================================================================================
BOOL BPP_GetBtlInFlag( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.fBtlIn;
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
  return bpp->weight;
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
  if( bpp->coreParam.level < PTL_LEVEL_MAX )
  {
    u32 expNow, expSum, expBorder;

    expNow = bpp->coreParam.exp;
    expSum = expNow + (*expRest);
    expBorder = POKETOOL_GetMinExp( bpp->coreParam.monsno, bpp->formNo, bpp->coreParam.level+1 );

    if( expSum >= expBorder )
    {
      u32 expAdd = (expBorder - expNow);
      u16 prevHP   = bpp->coreParam.hpMax;
      u16 diffHP;
      u16 atk, def, sp_atk, sp_def, agi;
      u8  fFastMode = PP_FastModeOn( (POKEMON_PARAM*)(bpp->coreParam.ppSrc) );

      // レベルアップ前パラメータ
      atk = PP_Get( bpp->coreParam.ppSrc, ID_PARA_pow, 0 );
      def = PP_Get( bpp->coreParam.ppSrc, ID_PARA_def, 0 );
      agi = PP_Get( bpp->coreParam.ppSrc, ID_PARA_agi, 0 );
      sp_atk = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spepow, 0 );
      sp_def = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spedef, 0 );

      bpp->coreParam.exp = (expNow + expAdd);
      PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_exp, bpp->coreParam.exp );
      PP_Renew( (POKEMON_PARAM*)(bpp->coreParam.ppSrc) );

      // レベルアップ後パラメータ
      bpp->coreParam.hpMax = PP_Get( bpp->coreParam.ppSrc, ID_PARA_hpmax, 0 );
      bpp->coreParam.level = PP_Get( bpp->coreParam.ppSrc, ID_PARA_level, 0 );
      diffHP = bpp->coreParam.hpMax - prevHP;
      info->atk    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_pow, 0 );
      info->def    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_def, 0 );
      info->sp_atk = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spepow, 0 );
      info->sp_def = PP_Get( bpp->coreParam.ppSrc, ID_PARA_spedef, 0 );
      info->agi    = PP_Get( bpp->coreParam.ppSrc, ID_PARA_agi, 0 );

      if( !(bpp->coreParam.fHensin) )
      {
        bpp->baseParam.attack     = info->atk;
        bpp->baseParam.defence    = info->def;
        bpp->baseParam.sp_attack  = info->sp_atk;
        bpp->baseParam.sp_defence = info->sp_def;
        bpp->baseParam.agility    = info->agi;
      }

      info->level  = bpp->coreParam.level;
      info->hp     = diffHP;
      info->atk    -= atk;
      info->def    -= def;
      info->sp_atk -= sp_atk;
      info->sp_def -= sp_def;
      info->agi    -= agi;

      bpp->coreParam.hp += diffHP;
      PP_Put((POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_hp, bpp->coreParam.hp );

      WazaWorkSys_ReflectToPP( bpp );

      PP_FastModeOff( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), fFastMode );

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

  if( info ){
    GFL_STD_MemClear( info, sizeof(*info) );
  }
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
 * Srcポケモンデータに現在のパラメータを反映
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectToPP( BTL_POKEPARAM* bpp, BOOL fDefaultForm )
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
  else{
    PP_SetSick( pp, POKESICK_NULL );
  }

  WazaWorkSys_ReflectToPP( bpp );

  {
    u8 formNo = ( bpp->coreParam.fHensin || fDefaultForm )? bpp->coreParam.defaultFormNo : bpp->formNo;
    PP_Put( pp, ID_PARA_form_no, formNo );
    PP_Renew( pp );
  }

  PP_Put( pp, ID_PARA_item, bpp->coreParam.item );
//  PP_Put( pp, ID_PARA_speabino, bpp->coreParam.defaultTokusei );
}

//=============================================================================================
/**
 * Srcポケモンデータからパラメータを再構築（レベルアップ、ワザ覚え後）
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_ReflectByPP( BTL_POKEPARAM* bpp )
{
  if( !(bpp->coreParam.fHensin) )
  {
    // へんしん中でなければパラメータを全て反映
    setupBySrcData( bpp, bpp->coreParam.ppSrc, TRUE );
    WazaWorkSys_ReflectFromPP( bpp );
  }
  else{
    // へんしん中なら真ワザワークのみ反映
    WazaWorkSys_SetupBySrcPP( bpp, bpp->coreParam.ppSrc, FALSE );
  }
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
  bpp->coreParam.ppFake = NULL;
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
    static BPP_WAZA  wazaWork[ PTL_WAZA_MAX ] = {0};
    u16    migawariHP;
    int i;

    // 不変部分をバックアップ
    GFL_STD_MemCopy( bpp->waza, wazaWork, sizeof(wazaWork) );
    core = bpp->coreParam;
    migawariHP = bpp->migawariHP;

    // 丸ごとコピー
    *bpp = *target;

    // 不変部分書き戻し
    bpp->coreParam = core;
    bpp->migawariHP = migawariHP;
    GFL_STD_MemCopy( wazaWork, bpp->waza, sizeof(wazaWork) );


    // 仮ワザワークを全書き換え（MAXPPは最大5）
    for(i=0; i<PTL_WAZA_MAX; ++i)
    {
      WazaWork_UpdateNumber( &bpp->waza[i], target->waza[i].surface.number, 5, FALSE );
    }

    flgbuf_clear( bpp->turnFlag, sizeof(bpp->turnFlag) );
    flgbuf_clear( bpp->contFlag, sizeof(bpp->contFlag) );

    bpp->appearedTurn = TURNCOUNT_NULL;
    bpp->turnCount = 0;
    bpp->prevSelectWazaID = WAZANO_NULL;
    bpp->prevActWazaID = WAZANO_NULL;
    bpp->prevWazaType = POKETYPE_NULL;
    bpp->wazaContCounter = 0;

    bpp->coreParam.fHensin = TRUE;

    return TRUE;
  }

  return FALSE;
}

BOOL BPP_HENSIN_Check( const BTL_POKEPARAM* bpp )
{
  return bpp->coreParam.fHensin;
}

//---------------------------------------------------------------------------------------------
// みがわり関連
//---------------------------------------------------------------------------------------------

void BPP_MIGAWARI_Create( BTL_POKEPARAM* bpp, u16 migawariHP )
{
  GF_ASSERT(bpp->migawariHP==0);
  bpp->migawariHP = migawariHP;
  BPP_CureWazaSick( bpp, WAZASICK_BIND );
}
void BPP_MIGAWARI_Delete( BTL_POKEPARAM* bpp )
{
  bpp->migawariHP = 0;
}
BOOL BPP_MIGAWARI_IsExist( const BTL_POKEPARAM* bpp )
{
  return bpp->migawariHP != 0;
}

/**
 *  ダメージ加算
 *
 *  @param  [io] damage  与えるダメージ値 -> 与えたダメージ値
 *
 *  @retval  破壊されたらTRUEが返る
 */
BOOL BPP_MIGAWARI_AddDamage( BTL_POKEPARAM* bpp, u16* damage )
{
  GF_ASSERT(bpp->migawariHP!=0);

  if( bpp->migawariHP <= (*damage) ){
    (*damage) = bpp->migawariHP;
    bpp->migawariHP = 0;
    return TRUE;
  }else{
    bpp->migawariHP -= (*damage);
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
  bpp->coreParam.confrontRecCount = 0;
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
  for(i=0; i<bpp->coreParam.confrontRecCount; ++i)
  {
    if( bpp->coreParam.confrontRec[i] == pokeID ){
      return;
    }
  }
  if( i < NELEMS(bpp->coreParam.confrontRec) ){
    BTL_Printf("ポケ[%d]が[%d]と対面した\n", bpp->coreParam.myID, pokeID);
    bpp->coreParam.confrontRec[i] = pokeID;
    bpp->coreParam.confrontRecCount++;
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
  return bpp->coreParam.confrontRecCount;
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
  if( idx < bpp->coreParam.confrontRecCount ){
    return bpp->coreParam.confrontRec[ idx ];
  }
  return BTL_POKEID_NULL;
}
//=============================================================================================
/**
 * 捕獲された時のボールアイテムIDをセットする
 *
 * @param   bpp
 * @param   ballItemID
 */
//=============================================================================================
void BPP_SetCaptureBallID( const BTL_POKEPARAM* bpp, u16 ballItemID )
{
  BALL_ID  ballID = ITEM_GetBallID( ballItemID );
  if( ballID == BALLID_NULL ){
    ballID = BALLID_MONSUTAABOORU;
  }
  PP_Put( (POKEMON_PARAM*)(bpp->coreParam.ppSrc), ID_PARA_get_ball, ballID );
}


//---------------------------------------------------------------------------------------------
// 合体ワザ関連
//---------------------------------------------------------------------------------------------

//=============================================================================================
/**
 * 合体ワザ対象（後発）としてのパラメータセット
 *
 * @param   bpp
 * @param   combiPokeID     先発ポケモンID
 * @param   combiUsedWaza   先発ポケモンが使ったワザID
 */
//=============================================================================================
void BPP_CombiWaza_SetParam( BTL_POKEPARAM* bpp, u8 combiPokeID, WazaID combiUsedWaza )
{
  bpp->combiPokeID = combiPokeID;
  bpp->combiWazaID = combiUsedWaza;
}
//=============================================================================================
/**
 * 合体ワザパラメータ取得
 *
 * @param   bpp
 * @param   combiPokeID     [out]
 * @param   combiUsedWaza   [out]
 *
 * @retval  BOOL    セットされたパラメータがある場合、TRUE
 */
//=============================================================================================
BOOL BPP_CombiWaza_GetParam( const BTL_POKEPARAM* bpp, u8* combiPokeID, WazaID* combiUsedWaza )
{
  if( bpp->combiPokeID != BTL_POKEID_NULL )
  {
    *combiPokeID = bpp->combiPokeID;
    *combiUsedWaza = bpp->combiWazaID;
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * 合体ワザパラメータが設定されているかチェック
 *
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BPP_CombiWaza_IsSetParam( const BTL_POKEPARAM* bpp )
{
  return bpp->combiPokeID != BTL_POKEID_NULL;
}
//=============================================================================================
/**
 * 合体ワザパラメータクリア
 *
 * @param   bpp
 */
//=============================================================================================
void BPP_CombiWaza_ClearParam( BTL_POKEPARAM* bpp )
{
  if( bpp->combiPokeID!= BTL_POKEID_NULL)
  {
    bpp->combiPokeID = BTL_POKEID_NULL;
    bpp->combiWazaID = WAZANO_NULL;
  }
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

