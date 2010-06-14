//=============================================================================================
/**
 * @file  btl_calc.c
 * @brief ポケモンWB バトル -システム-  各種、数値の計算ルーチン
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#include <gflib.h>

#include "tr_tool\trtype_def.h"
#include "tr_tool\tr_tool.h"

#include "btl_common.h"
#include "btl_tables.h"
#include "btl_util.h"

#include "btl_calc.h"


//--------------------------------------------------------------
/**
 *  Global
 */
//--------------------------------------------------------------
static GFL_STD_RandContext gRandContext = {0};
static u16* gWazaStoreWork = NULL;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BOOL is_include( const u16* tbl, u32 tblElems, u16 wazaID );
static u32 calcWinMoney_Sub( const BSP_TRAINER_DATA* trData, const POKEPARTY * party );



//--------------------------------------------------------------
/**
 *  ランダムコンテキスト内容出力処理
 */
//--------------------------------------------------------------
#ifdef PM_DEBUG
//#define DEBUG_RANDOM_PRINT  // define されていれば処理有効
#endif

#ifdef DEBUG_RANDOM_PRINT

static void _printRandom( BOOL fCounterReset, u32 range )
{
  static u32 counter = 0;

  if( fCounterReset ){
    counter = 0;
  }

  OS_SetPrintOutput_Arm9( 2 );
  OS_Printf("---- print random %d times, range=%d ----\n", counter++, range);
  OS_Printf("x:%0x, mul:%0x, add:%0x\n", gRandContext.x, gRandContext.mul, gRandContext.add);
  OS_SetPrintOutput_Arm9( 0 );
}
#else
#define _printRandom(resetFlag, range)  /* */
#endif





//=============================================================================================
/**
 * システム初期化
 *
 * @param   randContext   [in] 乱数コンテキスト
 * @param   heapID        [in] ヒープID
 */
//=============================================================================================
void BTL_CALC_InitSys( const GFL_STD_RandContext* randContext, HEAPID heapID )
{
  gWazaStoreWork = GFL_HEAP_AllocClearMemory( heapID, WAZANO_MAX * sizeof(u16) );
  gRandContext = *randContext;
  _printRandom( TRUE, 0 );
}
//=============================================================================================
/**
 *
 *
 * @param   randContext
 */
//=============================================================================================
void BTL_CALC_ResetSys( const GFL_STD_RandContext* randContext )
{
  gRandContext = *randContext;
  _printRandom( TRUE, 0 );
}

//=============================================================================================
/**
 * システム終了
 */
//=============================================================================================
void BTL_CALC_QuitSys( void )
{
  if( gWazaStoreWork )
  {
    GFL_HEAP_FreeMemory( gWazaStoreWork );
    gWazaStoreWork = NULL;
  }
}

//=============================================================================================
/**
 * 乱数値を返す
 *  ※返す値は 0～(pattern-1) の範囲。pattern==0だと、32bit全範囲。
 *
 * @param   pattern   乱数の取りうる値の範囲
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_GetRand( u32 range )
{
  u32 result;
  result = GFL_STD_Rand( &gRandContext, range );
  _printRandom( FALSE, range );
  return result;
}

//--------------------------------------------------------------
/**
 *  ステータスランク補正テーブル
 */
//--------------------------------------------------------------
static const struct {
  u8  num;
  u8  denom;
}StatusRankTable[] = {
  { 2, 8 }, // x(2/8) = 25%
  { 2, 7 }, // x(2/7) = 29%
  { 2, 6 }, // x(2/6) = 33%
  { 2, 5 }, // x(2/5) = 40%
  { 2, 4 }, // x(2/4) = 50%
  { 2, 3 }, // x(2/3) = 67%
  { 2, 2 }, // x(2/2) = 100%
  { 3, 2 }, // x(3/2) = 150%
  { 4, 2 }, // x(4/2) = 200%
  { 5, 2 }, // x(5/2) = 250%
  { 6, 2 }, // x(6/2) = 300%
  { 7, 2 }, // x(7/2) = 350%
  { 8, 2 }, // x(8/2) = 400%
};

u16 BTL_CALC_StatusRank( u16 defaultVal, u8 rank )
{
  GF_ASSERT(rank < NELEMS(StatusRankTable));

  {
    defaultVal = defaultVal * StatusRankTable[rank].num;
    defaultVal /= StatusRankTable[rank].denom;
    return defaultVal;
  }
}


//--------------------------------------------------------------
/**
 *  的中率補正テーブル
 */
//--------------------------------------------------------------
static const struct {
  u8  num;
  u8  denom;
}HitRankTable[] = {
  {   6,  18 },
  {   6,  16 },
  {   6,  14 },
  {   6,  12 },
  {   6,  10 },
  {   6,   8 },
  {   6,   6 },
  {   8,   6 },
  {  10,   6 },
  {  12,   6 },
  {  14,   6 },
  {  16,   6 },
  {  18,   6 },
};

u8 BTL_CALC_HitPer( u8 defPer, u8 rank )
{
  GF_ASSERT(rank < NELEMS(HitRankTable));

  {
    u32 per = defPer * HitRankTable[rank].num / HitRankTable[rank].denom;
    if( per > 100 )
    {
      per = 100;
    }
    return per;
  }
}

//--------------------------------------------------------------
/**
 *  クリティカル率テーブル
 */
//--------------------------------------------------------------
BOOL BTL_CALC_CheckCritical( u8 rank )
{
  static const u8 CriticalRankTable[] = {
    16, 8, 4, 3, 2,
  };

  GF_ASSERT(rank < NELEMS(CriticalRankTable));

  {
    u8 rp = CriticalRankTable[ rank ];
    u8 ret = BTL_CALC_GetRand( rp );

    return (ret == 0);
  }
}

//--------------------------------------------------------------
/**
 *  タイプ相性値
 */
//--------------------------------------------------------------
typedef enum {
    x0 = 0,
    xH = 2,
    x1 = 4,
    x2 = 8,
}TypeAffValue;

//--------------------------------------------------------------
/**
 *  タイプ相性テーブル
 */
//--------------------------------------------------------------
static const u8 TypeAffTbl[ POKETYPE_NUMS ][ POKETYPE_NUMS ] = {
//          ノ,  格,  飛,  毒,  地,  岩,  虫,  幽,  鋼,  炎,  水,  草,  電,  超,  氷,  竜,  悪,
/* ノ */ {  x1,  x1,  x1,  x1,  x1,  xH,  x1,  x0,  xH,  x1,  x1,  x1,  x1,  x1,  x1,  x1,  x1, },
/* 格 */ {  x2,  x1,  xH,  xH,  x1,  x2,  xH,  x0,  x2,  x1,  x1,  x1,  x1,  xH,  x2,  x1,  x2, },
/* 飛 */ {  x1,  x2,  x1,  x1,  x1,  xH,  x2,  x1,  xH,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1, },
/* 毒 */ {  x1,  x1,  x1,  xH,  xH,  xH,  x1,  xH,  x0,  x1,  x1,  x2,  x1,  x1,  x1,  x1,  x1, },
/* 地 */ {  x1,  x1,  x0,  x2,  x1,  x2,  xH,  x1,  x2,  x2,  x1,  xH,  x2,  x1,  x1,  x1,  x1, },
/* 岩 */ {  x1,  xH,  x2,  x1,  xH,  x1,  x2,  x1,  xH,  x2,  x1,  x1,  x1,  x1,  x2,  x1,  x1, },
/* 虫 */ {  x1,  xH,  xH,  xH,  x1,  x1,  x1,  xH,  xH,  xH,  x1,  x2,  x1,  x2,  x1,  x1,  x2, },
/* 幽 */ {  x0,  x1,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH, },
/* 鋼 */ {  x1,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH,  xH,  xH,  x1,  xH,  x1,  x2,  x1,  x1, },
/* 炎 */ {  x1,  x1,  x1,  x1,  x1,  xH,  x2,  x1,  x2,  xH,  xH,  x2,  x1,  x1,  x2,  xH,  x1, },
/* 水 */ {  x1,  x1,  x1,  x1,  x2,  x2,  x1,  x1,  x1,  x2,  xH,  xH,  x1,  x1,  x1,  xH,  x1, },
/* 草 */ {  x1,  x1,  xH,  xH,  x2,  x2,  xH,  x1,  xH,  xH,  x2,  xH,  x1,  x1,  x1,  xH,  x1, },
/* 電 */ {  x1,  x1,  x2,  x1,  x0,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  xH,  x1,  x1,  xH,  x1, },
/* 超 */ {  x1,  x2,  x1,  x2,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x0, },
/* 氷 */ {  x1,  x1,  x2,  x1,  x2,  x1,  x1,  x1,  xH,  xH,  xH,  x2,  x1,  x1,  xH,  x2,  x1, },
/* 竜 */ {  x1,  x1,  x1,  x1,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x1,  x1,  x1,  x1,  x2,  x1, },
/* 悪 */ {  x1,  xH,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH, },
};

//--------------------------------------------------------------
/**
 *  タイプ相性計算
 */
//--------------------------------------------------------------
BtlTypeAff BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType )
{
  if( (wazaType == POKETYPE_NULL) || (pokeType == POKETYPE_NULL) ){
    return BTL_TYPEAFF_100;
  }
  else
  {
    u8 affinity = TypeAffTbl[ wazaType ][ pokeType ];

    switch( affinity ){
    case x0:    return BTL_TYPEAFF_0;
    case xH:    return BTL_TYPEAFF_50;
    case x1:    return BTL_TYPEAFF_100;
    case x2:    return BTL_TYPEAFF_200;
    }
  }

  GF_ASSERT(0);
  return BTL_TYPEAFF_0;
}
//--------------------------------------------------------------
/**
 *  タイプ相性計算（ポケモンタイプそのまま使用可能）
 */
//--------------------------------------------------------------
BtlTypeAff BTL_CALC_TypeAffPair( PokeType wazaType, PokeTypePair pokeType )
{
  PokeType type1, type2;

  PokeTypePair_Split( pokeType, &type1, &type2 );

  if( PokeTypePair_IsPure(pokeType) )
  {
    return BTL_CALC_TypeAff( wazaType, type1 );
  }
  else
  {
    BtlTypeAff  aff1, aff2;
    aff1 = BTL_CALC_TypeAff( wazaType, type1 );
    aff2 = BTL_CALC_TypeAff( wazaType, type2 );
    return BTL_CALC_TypeAffMul( aff1, aff2 );
  }
}
//=============================================================================================
/**
 * 相性値の掛け合わせ（２タイプあるポケモン対応）
 *
 * @param   aff1
 * @param   aff2
 *
 * @retval  BtlTypeAff    掛け合わせ後の相性値
 */
//=============================================================================================
BtlTypeAff BTL_CALC_TypeAffMul( BtlTypeAff aff1, BtlTypeAff aff2 )
{
  enum {
    VALUE_0   = 0,
    VALUE_25  = 1,
    VALUE_50  = 2,
    VALUE_100 = 4,
    VALUE_200 = 8,
    VALUE_400 = 16,
  };

  static const u8 valueTable[ BTL_TYPEAFF_MAX ] = {
    VALUE_0,  VALUE_25, VALUE_50, VALUE_100, VALUE_200, VALUE_400
  };
  u32 mulValue;

  GF_ASSERT(aff1<BTL_TYPEAFF_MAX);
  GF_ASSERT(aff2<BTL_TYPEAFF_MAX);

  mulValue = (valueTable[ aff1 ] * valueTable[ aff2 ]) / VALUE_100;

  switch( mulValue ){
  case VALUE_0:    return BTL_TYPEAFF_0;
  case VALUE_25:   return BTL_TYPEAFF_25;
  case VALUE_50:   return BTL_TYPEAFF_50;
  case VALUE_100:  return BTL_TYPEAFF_100;
  case VALUE_200:  return BTL_TYPEAFF_200;
  case VALUE_400:  return BTL_TYPEAFF_400;
  default:
    GF_ASSERT(0);
    return BTL_TYPEAFF_0;
  }
}


//=============================================================================================
/**
 * 基本ダメージ計算
 *
 * @param   wazaPow
 * @param   atkPower
 * @param   atkLevel
 * @param   defGuard
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_DamageBase( u32 wazaPower, u32 atkPower, u32 atkLevel, u32 defGuard )
{
  u32 damage;

  damage = (wazaPower * atkPower * (atkLevel*2/5+2));
  damage = damage / defGuard / 50;
  damage += 2;
  return damage;
}

//--------------------------------------------------------------
/**
 *  相性ダメージ計算
 */
//--------------------------------------------------------------
u32 BTL_CALC_AffDamage( u32 rawDamage, BtlTypeAff aff )
{
  switch( aff ){
  case BTL_TYPEAFF_0:   return 0;
  case BTL_TYPEAFF_25:  return (rawDamage / 4);
  case BTL_TYPEAFF_50:  return (rawDamage / 2);
  case BTL_TYPEAFF_100: return rawDamage;
  case BTL_TYPEAFF_200: return rawDamage * 2;
  case BTL_TYPEAFF_400: return rawDamage * 4;
  default:
    GF_ASSERT(0);
    return rawDamage;
  }
}
//=============================================================================================
/**
 *  抵抗相性を全て取得
 *
 * @param   type    タイプ
 * @param   dst     [out] 抵抗相性を持つタイプを格納する配列
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_CALC_GetResistTypes( PokeType type, PokeType* dst )
{
  u8  cnt, i;

  for(i=0, cnt=0; i<POKETYPE_NUMS; ++i)
  {
    if( (TypeAffTbl[type][i] == x0) || (TypeAffTbl[type][i] == xH) ){
      dst[cnt++] = i;
    }
  }

  return cnt;
}
//=============================================================================================
/**
 * タイプ相性（厳密）->タイプ相性（おおまか）に変更
 *
 * @param   aff
 *
 * @retval  BtlTypeAffAbout
 */
//=============================================================================================
BtlTypeAffAbout BTL_CALC_TypeAffAbout( BtlTypeAff aff )
{
  if( aff > BTL_TYPEAFF_100 )
  {
    return BTL_TYPEAFF_ABOUT_ADVANTAGE;
  }
  if( aff == BTL_TYPEAFF_100 )
  {
    return BTL_TYPEAFF_ABOUT_NORMAL;
  }
  if( aff != BTL_TYPEAFF_0 )
  {
    return BTL_TYPEAFF_ABOUT_DISADVANTAGE;
  }
  return BTL_TYPEAFF_ABOUT_NONE;
}
//=============================================================================================
/**
 * 確率事象チェック
 *
 * @param   per
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_IsOccurPer( u32 per )
{
  return (BTL_CALC_GetRand(100) < per);
}
//=============================================================================================
/**
 * 数値切り上げ
 *
 * @param   value
 * @param   min
 *
 * @retval  int
 */
//=============================================================================================
int BTL_CALC_Roundup( int value, int min )
{
  if( value < min ){ value = min; }
  return value;
}
//=============================================================================================
/**
 *
 *
 * @param   value
 * @param   ratio   倍率（％）
 *
 * @retval  extern u32
 */
//=============================================================================================
u32 BTL_CALC_MulRatio( u32 value, fx32 ratio )
{
  u32 decimal;

  value *= ratio;
  decimal = value & ( (1 << FX32_SHIFT) -1 );
  value >>= FX32_SHIFT;
  if( decimal > (1 << (FX32_SHIFT-1)) ){
    ++value;
  }

  return value;
}
//=============================================================================================
/**
 * 倍率計算（整数x固定小数）※ゼロになった場合、最低１に切り上げ
 *
 * @param   value
 * @param   ratio   倍率（％）
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_MulRatio_OverZero( u32 value, fx32 ratio )
{
  value = BTL_CALC_MulRatio( value, ratio );
  if( value == 0 ){
    value = 1;
  }
  return value;
}
//=============================================================================================
/**
 * 倍率計算（整数x整数）
 *
 * @param   value
 * @param   ratio   倍率（％）
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_MulRatioInt( u32 value, u32 ratio )
{
  u32  rem;

  value *= ratio;
  rem = value % 100;
  value /= 100;
  if( rem >= 50 ){
    ++value;
  }
  return value;
}
//=============================================================================================
/**
 * 倍率計算（整数x整数）※ゼロになった場合、１に切り上げ
 *
 * @param   value
 * @param   ratio
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_MulRatioInt_OverZero( u32 value, u32 ratio )
{
  value = BTL_CALC_MulRatioInt( value, ratio );
  if( value == 0 ){
    value = 1;
  }
  return value;
}
//=============================================================================================
/**
 * ポケモンの最大HP * 1/n を計算
 *
 * @param   bpp
 * @param   denom
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_QuotMaxHP_Zero( const BTL_POKEPARAM* bpp, u32 denom )
{
  u32 ret = BPP_GetValue( bpp, BPP_MAX_HP ) / denom;
  return ret;
}
//=============================================================================================
/**
 * ポケモンの最大HP * 1/n を計算（最低１になるように補正）
 *
 * @param   bpp
 * @param   denom
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_QuotMaxHP( const BTL_POKEPARAM* bpp, u32 denom )
{
  u32 ret = BPP_GetValue( bpp, BPP_MAX_HP ) / denom;
  if( ret == 0 ){ ret = 1; }
  return ret;
}
//=============================================================================================
/**
 *  min以上 ～ max以下の範囲内で乱数取得
 *
 * @param   min
 * @param   max
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_RandRange( u32 min, u32 max )
{
  if( min > max ){
    u32 tmp = min;
    min = max;
    max = tmp;
  }
  {
    u32 range = 1 + (max-min);
    return min + BTL_CALC_GetRand( range );
  }
}




//=============================================================================================
/**
 * 最大ヒット回数を受け取り、実際にヒットする回数（ランダム補正）を返す
 *
 * @param   numHitMax
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_CALC_HitCountStd( u8 numHitMax )
{
  if( numHitMax == 5 )
  {
    static const u8 perTbl[] = {
      0, 0, 35, 70, 85, 100,
    };
    u8 rnd = BTL_CALC_GetRand( 100 );
    u8 i;

    for(i=0; i<NELEMS(perTbl); ++i)
    {
      if( rnd < perTbl[i] ){
        return i;
      }
    }
    return numHitMax;
  }
  else{
    return numHitMax;
  }
}

//=============================================================================================
/**
 * 該当の天候によるターンチェックダメージを計算
 *
 * @param   bpp
 * @param   weather
 *
 * @retval  u16     ダメージ値
 */
//=============================================================================================
u16 BTL_CALC_RecvWeatherDamage( const BTL_POKEPARAM* bpp, BtlWeather weather )
{
  switch( weather ){
  case BTL_WEATHER_SAND:
    if( BPP_IsMatchType(bpp, POKETYPE_IWA) ){ return 0; }
    if( BPP_IsMatchType(bpp, POKETYPE_HAGANE) ){ return 0; }
    if( BPP_IsMatchType(bpp, POKETYPE_JIMEN) ){ return 0; }
    break;

  case BTL_WEATHER_SNOW:
    if( BPP_IsMatchType(bpp, POKETYPE_KOORI) ){ return 0; }
    break;

  default:
    return 0;
  }

  {
    u16 dmg = BPP_GetValue( bpp, BPP_MAX_HP ) / 16;
    if( dmg == 0 ){ dmg = 1; }
    return dmg;
  }
}
//=============================================================================================
/**
 * 天候・ワザタイプ相性による威力倍率補正
 *
 * @param   weather
 * @param   wazaType
 *
 * @retval  fx32
 */
//=============================================================================================
fx32 BTL_CALC_GetWeatherDmgRatio( BtlWeather weather, PokeType wazaType )
{
  switch( weather ){
  case BTL_WEATHER_SHINE:
    if( wazaType == POKETYPE_HONOO ){
      return BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE;
    }
    if( wazaType == POKETYPE_MIZU ){
      return BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE;
     }
    break;

  case BTL_WEATHER_RAIN:
    if( wazaType == POKETYPE_HONOO ){
      return BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE;
    }
    if( wazaType == POKETYPE_MIZU ){
      return BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE;
    }
    break;

  default:
    break;
  }
  return BTL_CALC_DMG_WEATHER_RATIO_NONE;
}


//=============================================================================================
/**
 * ワザデータの状態異常継続パラメータ値から、バトルで使う状態異常継続パラメータ値へ変換
 *
 * @param   wazaSickCont
 * @param   attacker    ワザを使ったポケモン
 * @param   sickCont    [out]
 *
 */
//=============================================================================================
void BTL_CALC_WazaSickContToBppSickCont( WAZA_SICKCONT_PARAM wazaSickCont, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* sickCont )
{
  switch( wazaSickCont.type ){
  case WAZASICK_CONT_POKE:
    GF_ASSERT(attacker!=NULL);
    {
      *sickCont = BPP_SICKCONT_MakePoke( BPP_GetID(attacker) );
    }
    break;

  case WAZASICK_CONT_TURN:
    {
      u8 turn = BTL_CALC_RandRange( wazaSickCont.turnMin, wazaSickCont.turnMax );
      *sickCont = BPP_SICKCONT_MakeTurn( turn );
    }
    break;

  case WAZASICK_CONT_PERMANENT:
    *sickCont = BPP_SICKCONT_MakePermanentIncParam( wazaSickCont.turnMax, wazaSickCont.turnMin );
    break;

  case WAZASICK_CONT_POKETURN:
    GF_ASSERT(attacker!=NULL);
    {
      u8 pokeID = BPP_GetID( attacker );
      u8 turn = BTL_CALC_RandRange( wazaSickCont.turnMin, wazaSickCont.turnMax );
      *sickCont = BPP_SICKCONT_MakePokeTurn( pokeID, turn );
    }
    break;
  }
}
//=============================================================================================
/**
 * ポケモン系状態異常の、デフォルトの継続パラメータを作成
 *
 * @param   sick
 * @param   cont    [out]
 *
 */
//=============================================================================================
BPP_SICK_CONT BTL_CALC_MakeDefaultPokeSickCont( PokeSick sick )
{
  BPP_SICK_CONT  cont;

  cont.raw = 0;
  switch( sick ){
  case POKESICK_DOKU:
  case POKESICK_YAKEDO:
  case POKESICK_MAHI:
  case POKESICK_KOORI:
    cont.type = WAZASICK_CONT_PERMANENT;
    break;
  case POKESICK_NEMURI:
    cont.type = WAZASICK_CONT_TURN;
    cont.turn.count = BTL_CALC_RandRange( BTL_NEMURI_TURN_MIN, BTL_NEMURI_TURN_MAX );
    break;
  default:
    GF_ASSERT_MSG(0, "illegal sick ID(%d)\n", sick);
    cont.type = WAZASICK_CONT_NONE;
    break;
  }

  return cont;
}

//=============================================================================================
/**
 * 基本状態異常か
 *
 * @param   sickID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_CALC_IsBasicSickID( WazaSick sickID )
{
  return sickID < POKESICK_MAX;
}

//=============================================================================================
/**
 * ワザ系状態異常の継続パラメータ（ターン数型）を作成
 *
 * @param   turn_count
 */
//=============================================================================================
BPP_SICK_CONT BTL_CALC_MakeWazaSickCont_Turn( u8 turn_count )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_TURN;
  cont.turn.count = turn_count;
  return cont;
}
//=============================================================================================
/**
 * ワザ系状態異常の継続パラメータ（ポケモン依存型）を作成
 *
 * @param   pokeID
 */
//=============================================================================================
BPP_SICK_CONT BTL_CALC_MakeWazaSickCont_Poke( u8 pokeID )
{
  BPP_SICK_CONT cont;
  cont.raw = 0;
  cont.type = WAZASICK_CONT_POKE;
  cont.poke.ID = pokeID;
  return cont;
}


//=============================================================================================
/**
 * ワザ系状態異常の、デフォルトの継続パラメータを作成
 *
 * @param   sick
 * @param   cont    [out]
 *
 */
//=============================================================================================
void BTL_CALC_MakeDefaultWazaSickCont( WazaSick sick, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* cont )
{
  if( sick < POKESICK_MAX ){
    *cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
    return;
  }

  cont->raw = 0;

  switch( sick ){
  case WAZASICK_MEROMERO:
    {
      u8 pokeID = BPP_GetID( attacker );
      *cont = BPP_SICKCONT_MakePoke( pokeID );
    }
    break;

  case WAZASICK_KONRAN:
    {
      u8 turns = BTL_CALC_RandRange( BTL_CONF_TURN_MIN, BTL_CONF_TURN_MAX );
      *cont = BPP_SICKCONT_MakeTurn( turns );
    }
    break;

  default:
    *cont = BPP_SICKCONT_MakePermanent();
    break;
  }
}
//=============================================================================================
/**
 * 「メンタルハーブ」で治る対象の状態異常チェック
 *
 * @param   bpp
 *
 * @retval  WazaSick
 */
//=============================================================================================
WazaSick BTL_CALC_CheckMentalSick( const BTL_POKEPARAM* bpp )
{
  WazaSick result = WAZASICK_NULL;
  u32 i = 0;
  while( 1 )
  {
    result = BTL_TABLES_GetMentalSickID( i++ );
    if( result == WAZASICK_NULL ){
      break;
    }
    if( BPP_CheckSick(bpp, result) ){
      return result;
    }
  }
  return result;
}


//=============================================================================================
// サイドエフェクト関連
//=============================================================================================
static const struct {
  u16  waza:11; //wazanoが2048,SIDEEFFの定義値が32を越えたら拡張の必要アリ
  u16  side:5;
}WazaSideDispatchTbl[] = {
  { WAZANO_RIHUREKUTAA,    BTL_SIDEEFF_REFRECTOR },      ///< 物理攻撃を半減
  { WAZANO_HIKARINOKABE,   BTL_SIDEEFF_HIKARINOKABE },   ///< 特殊攻撃を半減
  { WAZANO_SINPINOMAMORI,  BTL_SIDEEFF_SINPINOMAMORI },  ///< ポケ系状態異常にならない
  { WAZANO_SIROIKIRI,      BTL_SIDEEFF_SIROIKIRI },      ///< ランクダウン効果を受けない
  { WAZANO_OIKAZE,         BTL_SIDEEFF_OIKAZE },         ///< すばやさ２倍
  { WAZANO_OMAZINAI,       BTL_SIDEEFF_OMAJINAI },       ///< 攻撃が急所に当たらない
  { WAZANO_MAKIBISI,       BTL_SIDEEFF_MAKIBISI },       ///< 入れ替えて出てきたポケモンにダメージ（３段階）
  { WAZANO_DOKUBISI,       BTL_SIDEEFF_DOKUBISI },       ///< 入れ替えて出てきたポケモンに毒（２段階）
  { WAZANO_SUTERUSUROKKU,  BTL_SIDEEFF_STEALTHROCK },    ///< 入れ替えて出てきたポケモンにダメージ（相性計算あり）
//  { WAZANO_WAIDOGAADO,     BTL_SIDEEFF_WIDEGUARD },      ///< 「敵全体」「自分以外全部」の攻撃ワザを防ぐ
};


BtlSideEffect  BTL_CALC_WazaIDtoSideEffectID( WazaID waza )
{
  u32 i;
  for(i=0; i<NELEMS(WazaSideDispatchTbl); ++i){
    if( WazaSideDispatchTbl[i].waza == waza ){
      return WazaSideDispatchTbl[i].side;
    }
  }
  return BTL_SIDEEFF_NULL;
}
WazaID  BTL_CALC_SideEffectIDtoWazaID( BtlSideEffect sideEffect )
{
  u32 i;
  for(i=0; i<NELEMS(WazaSideDispatchTbl); ++i){
    if( WazaSideDispatchTbl[i].side == sideEffect ){
      return WazaSideDispatchTbl[i].waza;
    }
  }
  return WAZANO_NULL;
}
//=============================================================================================
// ランダム技選択
//=============================================================================================


//=============================================================================================
/**
 * 特定のワザを除外した中からランダムでワザを決定（ゆびをふる専用？）
 *
 * @param   excludeWazaTbl    除外するワザIDテーブル
 * @param   tblElems          除外するワザIDテーブルの要素数
 *
 * @retval  WazaID    決定したワザID
 */
//=============================================================================================
WazaID BTL_CALC_RandWaza( const u16* wazaTbl, u16 tblElems )
{
  u16 waza, cnt, i;

  for(cnt=0, waza=1; waza<WAZANO_MAX; ++waza)
  {
    if( !is_include(wazaTbl, tblElems, waza) ){
      gWazaStoreWork[ cnt++ ] = waza;
    }
  }

  i = BTL_CALC_GetRand( cnt );
  return gWazaStoreWork[ i ];
}
static BOOL is_include( const u16* tbl, u32 tblElems, u16 wazaID )
{
  u32 i;
  for(i=0; i<tblElems; ++i){
    if( tbl[i] == wazaID ){
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================================
/**
 * ポケモンを倒した時のベース経験値を計算
 *
 * @param   bpp   倒されたポケモン
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_CalcBaseExp( const BTL_POKEPARAM* bpp )
{
  u32 monsno = BPP_GetMonsNo( bpp );
  u16 formNo = BPP_GetValue( bpp, BPP_FORM );
  u16 level  = BPP_GetValue( bpp, BPP_LEVEL );

  u32 exp = POKETOOL_GetPersonalParam( monsno, formNo, POKEPER_ID_give_exp );
  exp = exp * level / 5;

  return exp;
}

//=============================================================================================
// ルールごとに変わる要素
//=============================================================================================
//=============================================================================================
/**
 * ワザの対象を選択する必要があるか？
 */
//=============================================================================================
BOOL BTL_RULE_IsNeedSelectTarget( BtlRule rule )
{
#ifdef ROTATION_NEW_SYSTEM
  return ((rule != BTL_RULE_SINGLE) && (rule != BTL_RULE_ROTATION));
#else
  return (rule != BTL_RULE_SINGLE);
#endif
}
//=============================================================================================
/**
 * お互いの陣営に存在する位置の数
 */
//=============================================================================================
u8 BTL_RULE_GetNumFrontPos( BtlRule rule )
{
  switch( rule ){
  case BTL_RULE_SINGLE:
    return 1;
  case BTL_RULE_DOUBLE:
    return 2;
  case BTL_RULE_TRIPLE:
    return 3;

  case BTL_RULE_ROTATION:
    return BTL_ROTATION_FRONTPOS_NUM;
  }
  GF_ASSERT(0);
  return 1;
}
//=============================================================================================
/**
 * 手持ちポケモンの開始インデックス
 *
 * @param   rule
 * @param   numCoverPos   クライアントの担当位置数（ダブル時、通常は２，マルチなら１になる）
 */
//=============================================================================================
u8 BTL_RULE_HandPokeIndex( BtlRule rule, u8 numCoverPos )
{
  if( rule == BTL_RULE_ROTATION )
  {
    // ローテーションの場合、３体目まで戦闘に出ているとみなす
    return BTL_ROTATE_NUM;
  }
  return numCoverPos;
}

//=============================================================================================
// トレーナータイプ判別処理
//=============================================================================================
/**
 *  トレーナータイプがジムリーダーに該当するか判定
 */
BOOL BTL_CALC_IsTrtypeGymLeader( u16 trType )
{
  return ( TT_TrainerTypeGrpGet( trType ) == TRTYPE_GRP_LEADER);
}
/**
 *  トレーナータイプが四天王に該当するか判定
 */
BOOL BTL_CALC_IsTrtypeBig4( u16 trType )
{
  return ( TT_TrainerTypeGrpGet( trType ) == TRTYPE_GRP_BIGFOUR);
}
/**
 *  トレーナータイプがボス戦（なつき度上昇）に該当するか判定
 */
BOOL BTL_CALC_IsTrTypeBoss( u16 trType )
{
  if( BTL_CALC_IsTrtypeGymLeader(trType) ){
    return TRUE;
  }
  if( BTL_CALC_IsTrtypeBig4(trType) ){
    return TRUE;
  }
  if( trType == TRTYPE_CHAMPION ){
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
// 賞金計算
//=============================================================================================

//=============================================================================================
/**
 * 勝ったときにもらえるおこづかい金額の計算
 *
 * @param   sp
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_WinMoney( const BATTLE_SETUP_PARAM* sp )
{
  u32 money = 0;
  if( sp->competitor == BTL_COMPETITOR_TRAINER )
  {
    u8 clientID = BTL_CLIENT_ENEMY1;
    if( (sp->tr_data[clientID] != NULL) && (sp->party[clientID] != NULL) ){
      money += calcWinMoney_Sub( sp->tr_data[clientID], sp->party[clientID] );
    }

    clientID = BTL_CLIENT_ENEMY2;
    if( (sp->tr_data[clientID] != NULL) && (sp->party[clientID] != NULL) ){
      money += calcWinMoney_Sub( sp->tr_data[clientID], sp->party[clientID] );
    }
  }
  return money;
}

static u32 calcWinMoney_Sub( const BSP_TRAINER_DATA* trData, const POKEPARTY * party )
{
  if( party )
  {
    u8 poke_cnt = PokeParty_GetPokeCount( party );
    if( poke_cnt )
    {
      const POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, poke_cnt-1 );
      u32 tr_money_ratio = TT_TrainerDataParaGet( trData->tr_id, ID_TD_gold );

      return (PP_Get(pp, ID_PARA_level, NULL) * tr_money_ratio * 4);
    }
  }
  return 0;
}



//=============================================================================================
/**
 * 負けた時に支払う金額計算
 *
 * @param   badgeCount    プレイヤーバッジ数
 * @param   party         パーティデータ
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CALC_LoseMoney( u8 badgeCount, const BTL_PARTY* party )
{
  static const u8 ratio[] = {
    2, 4, 6, 9, 12, 16, 20, 25, 30,
  };
  u32 maxLevel, pokeCnt, i;

  pokeCnt = BTL_PARTY_GetMemberCount( party );
  maxLevel = 1;
  for(i=0; i<pokeCnt; ++i)
  {
    const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( party, i );
    u32 level = BPP_GetValue( bpp, BPP_LEVEL );
    if( level > maxLevel ){
      maxLevel = level;
    }
  }

  // 有り得ないけど一応
  if( badgeCount >= NELEMS(ratio) ){
    badgeCount = NELEMS(ratio) - 1;
  }

  return maxLevel * 4 * ratio[ badgeCount ];
}


//=============================================================================================
// アイテムデータ取得関連
//=============================================================================================
static HEAPID Heap_ItemParam;

//=============================================================================================
/**
 *
 *
 * @param   heapID
 */
//=============================================================================================
void BTL_CALC_ITEM_InitSystem( HEAPID heapID )
{
  Heap_ItemParam = heapID;
}
void BTL_CALC_ITEM_QuitSystem( void )
{
  //
}
//=============================================================================================
/**
 * アイテムパラメータ取得
 *
 * @param   item
 * @param   paramID
 *
 * @retval  s32
 */
//=============================================================================================
s32 BTL_CALC_ITEM_GetParam( u16 item, u16 paramID )
{
  return ITEM_GetParam( item, paramID, Heap_ItemParam );
}



//=============================================================================================
/**
 * 「のろい」の選択対象タイプを返す
 *
 * @param   attacker
 *
 * @retval  WazaTarget
 */
//=============================================================================================
WazaTarget BTL_CALC_GetNoroiTargetType( const BTL_POKEPARAM* attacker )
{
  return BPP_IsMatchType(attacker, POKETYPE_GHOST)? WAZA_TARGET_OTHER_SELECT : WAZA_TARGET_USER;
}

//=============================================================================================
/**
 * ワザターゲットをランダムで自動決定
 *
 * @param   mainModule
 * @param   bpp
 * @param   waza
 *
 * @retval  BtlTargetPos
 */
//=============================================================================================
BtlPokePos BTL_CALC_DecideWazaTargetAuto( const BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokeCon, const BTL_POKEPARAM* bpp, WazaID waza )
{
  BtlRule rule = BTL_MAIN_GetRule( mainModule );
  BtlPokePos targetPos = BTL_POS_NULL;
  BtlPokePos myPos = BTL_MAIN_PokeIDtoPokePos( mainModule, pokeCon, BPP_GetID(bpp) );
  WazaTarget  targetType = WAZADATA_GetParam( waza, WAZAPARAM_TARGET );

  if( waza == WAZANO_NOROI ){
    targetType = BTL_CALC_GetNoroiTargetType( bpp );
  }

  // シングル
  if( rule == BTL_RULE_SINGLE )
  {
    switch( targetType ){
    case WAZA_TARGET_OTHER_SELECT:  ///< 自分以外の１体（選択）
    case WAZA_TARGET_ENEMY_SELECT:  ///< 敵１体（選択）
    case WAZA_TARGET_ENEMY_RANDOM:  ///< 敵ランダム
    case WAZA_TARGET_ENEMY_ALL:     ///< 敵側２体
    case WAZA_TARGET_OTHER_ALL:     ///< 自分以外全部
      targetPos = BTL_MAIN_GetOpponentPokePos( mainModule, myPos, 0 );
      return targetPos;

    case WAZA_TARGET_USER:                ///< 自分１体のみ
    case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方１体
      targetPos = myPos;
      return targetPos;

    case WAZA_TARGET_ALL:
    case WAZA_TARGET_UNKNOWN:
    default:
      break;
    }
    return BTL_POS_NULL;
  }
  // ダブル、トリプル、ローテ共通
  else
  {
    BtlExPos exPos;
    u8 pokeID[ BTL_POS_MAX ];
    u8 pokeCnt = 0;

    switch( targetType ){
    case WAZA_TARGET_OTHER_SELECT:       ///< 通常ポケ（１体選択）
    case WAZA_TARGET_ENEMY_SELECT:       ///< 相手側ポケ（１体選択）
    case WAZA_TARGET_ENEMY_RANDOM:       ///< 相手ポケ１体ランダム
      exPos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );
      break;

    case WAZA_TARGET_FRIEND_USER_SELECT: ///< 自分を含む味方ポケ（１体選択）
      exPos = EXPOS_MAKE( BTL_EXPOS_AREA_MYTEAM, myPos );
      break;

    case WAZA_TARGET_FRIEND_SELECT:      ///< 自分以外の味方ポケ（１体選択）
      exPos = EXPOS_MAKE( BTL_EXPOS_AREA_FRIENDS, myPos );
      break;

    case WAZA_TARGET_USER:               ///< 自分のみ
      return myPos;

    default:
      return BTL_POS_NULL;
    }

    // 生きてるポケモンがいればその中からランダム
    pokeCnt = BTL_MAIN_ExpandExistPokeID( mainModule, pokeCon, exPos, pokeID );
    if( pokeCnt )
    {
      u8 rnd = BTL_CALC_GetRand( pokeCnt );
      return BTL_MAIN_PokeIDtoPokePos( mainModule, pokeCon, pokeID[rnd] );
    }

    // いなければ該当位置からランダム
    pokeCnt = BTL_MAIN_ExpandBtlPos( mainModule, exPos, pokeID );
    if( pokeCnt )
    {
      u8 rnd = BTL_CALC_GetRand( pokeCnt );
      return pokeID[ rnd ];
    }

    return BTL_POS_NULL;
  }
}


//=============================================================================================
/**
 *
 *
 * @param   info
 */
//=============================================================================================
void BTL_ESCAPEINFO_Clear( BTL_ESCAPEINFO* info )
{
  u32 i;
  for(i=0; i<NELEMS(info->clientID); ++i){
    info->clientID[i] = BTL_CLIENTID_NULL;
  }
  info->count = 0;
}
void BTL_ESCAPEINFO_Add( BTL_ESCAPEINFO* info, u8 clientID )
{
  u32 i;
  for(i=0; i<NELEMS(info->clientID); ++i)
  {
    if( info->clientID[i] == clientID ){ break; }
    if( info->clientID[i] == BTL_CLIENTID_NULL )
    {
      info->clientID[i] = clientID;
      info->count++;
//      TAYA_Printf( , info->clientID[i], info->count);
      BTL_N_Printf( DBGSTR_CALC_EscInfoAdd, info->clientID[i], info->count);
      break;
    }
  }
}
u32 BTL_ESCAPEINFO_GetCount( const BTL_ESCAPEINFO* info )
{
  return info->count;
}

BtlResult BTL_ESCAPEINFO_CheckWinner( const BTL_ESCAPEINFO* info, u8 myClientID, BtlCompetitor competitor )
{
  BOOL fMyTeamEscaped = FALSE;
  BOOL fOtherTeamEscaped = FALSE;
  u32 i;


  for(i=0; i<info->count; ++i)
  {
    if( BTL_MAINUTIL_IsFriendClientID( info->clientID[i], myClientID ) ){
      BTL_N_Printf( DBGSTR_CALC_EscCheckMyTeam, info->clientID[i]);
      fMyTeamEscaped = TRUE;
    }else{
      BTL_N_Printf( DBGSTR_CALC_EscCheckEnemy, info->clientID[i]);
      fOtherTeamEscaped = TRUE;
    }
  }

  if( fMyTeamEscaped && fOtherTeamEscaped )
  {
    if( (competitor == BTL_COMPETITOR_WILD) || (competitor == BTL_COMPETITOR_TRAINER) ){
      BTL_N_Printf( DBGSTR_CALC_EscResultBoth1 );
      return BTL_RESULT_RUN;
    }
    BTL_N_Printf( DBGSTR_CALC_EscResultBoth2 );
    return BTL_RESULT_DRAW;
  }
  if( fMyTeamEscaped ){
    BTL_N_Printf( DBGSTR_CALC_EscResultLose );
    return BTL_RESULT_RUN;
  }
  BTL_N_Printf( DBGSTR_CALC_EscResultWin );
  return BTL_RESULT_RUN_ENEMY;
}

