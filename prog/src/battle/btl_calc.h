#ifndef __BTL_CALC_H__
#define __BTL_CALC_H__

#include "item\item.h"
#include "waza_tool\wazano_def.h"

#include "battle\battle.h"
#include "btl_main.h"
#include "btl_util.h"
#include "btl_pokeparam.h"
#include "btl_sideeff.h"
#include "btl_const.h"


/**
 *  要素数->bitフラグフィールドに必要なバッファサイズ（バイト）計算
 */
#define BTL_CALC_BITFLAG_BUFSIZE(max)   (((max)/8)+(((max)%8)!=0))



//--------------------------------------------------------------------
/**
 *  タイプ相性（ 0, 25, 50, 100, 200, 400% の６段階）
 */
//--------------------------------------------------------------------
typedef enum {

  BTL_TYPEAFF_0 = 0,  ///< 無効
  BTL_TYPEAFF_25,
  BTL_TYPEAFF_50,
  BTL_TYPEAFF_100,
  BTL_TYPEAFF_200,
  BTL_TYPEAFF_400,

  BTL_TYPEAFF_MAX,
  BTL_TYPEAFF_NULL = BTL_TYPEAFF_MAX,

}BtlTypeAff;
//--------------------------------------------------------------------
/**
 *  簡易タイプ相性（こうかなし・ふつう・ばつぐん・もうひとつの４段階）
 */
//--------------------------------------------------------------------
typedef enum {

  BTL_TYPEAFF_ABOUT_NONE = 0,
  BTL_TYPEAFF_ABOUT_NORMAL,
  BTL_TYPEAFF_ABOUT_ADVANTAGE,
  BTL_TYPEAFF_ABOUT_DISADVANTAGE,

}BtlTypeAffAbout;

//--------------------------------------------------------------------
/**
 *  タイプ相性 -> 簡易タイプ相性変換
 */
//--------------------------------------------------------------------
extern BtlTypeAffAbout BTL_CALC_TypeAffAbout( BtlTypeAff aff );


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
extern void BTL_CALC_InitSys( const GFL_STD_RandContext* randContext, HEAPID heapID );
extern void BTL_CALC_ResetSys( const GFL_STD_RandContext* randContext );
extern void BTL_CALC_QuitSys( void );
extern u32 BTL_CALC_GetRand( u32 range );

extern u32 BTL_CALC_DamageBase( u32 wazaPower, u32 atkPower, u32 atkLevel, u32 defGuard );

extern u16 BTL_CALC_StatusRank( u16 defaultVal, u8 rank );
extern u8 BTL_CALC_HitPer( u8 defPer, u8 rank );
extern BOOL BTL_CALC_CheckCritical( u8 rank );

extern BtlTypeAff BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType );
extern BtlTypeAff BTL_CALC_TypeAffPair( PokeType wazaType, PokeTypePair pokeType );
extern BtlTypeAff BTL_CALC_TypeAffMul( BtlTypeAff aff1, BtlTypeAff aff2 );

extern u32 BTL_CALC_AffDamage( u32 rawDamage, BtlTypeAff aff );

extern u8 BTL_CALC_HitCountStd( u8 numHitMax );
extern u16 BTL_CALC_RecvWeatherDamage( const BTL_POKEPARAM* bpp, BtlWeather weather );
extern fx32 BTL_CALC_GetWeatherDmgRatio( BtlWeather weather, PokeType wazaType );

extern u8 BTL_CALC_GetResistTypes( PokeType type, PokeType* dst );

extern BOOL BTL_CALC_IsTrtypeGymLeader( u16 trType );
extern BOOL BTL_CALC_IsTrtypeBig4( u16 trType );
extern BOOL BTL_CALC_IsTrTypeBoss( u16 trType );

extern u32 BTL_CALC_PokeIDx6_Pack32bit( const u8* pokeIDList );
extern void BTL_CALC_PokeIDx6_Unpack32bit( u32 pack, u8* pokeIDList );


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
extern void BTL_CALC_WazaSickContToBppSickCont( WAZA_SICKCONT_PARAM wazaSickCont, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* sickCont );

extern BPP_SICK_CONT BTL_CALC_MakeDefaultPokeSickCont( PokeSick sick );
extern BOOL BTL_CALC_IsBasicSickID( WazaSick sickID );
extern void BTL_CALC_MakeDefaultWazaSickCont( WazaSick sick, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* cont );
extern BPP_SICK_CONT BTL_CALC_MakeWazaSickCont_Turn( u8 turn_count );
extern BPP_SICK_CONT BTL_CALC_MakeWazaSickCont_Poke( u8 pokeID );

extern BtlSideEffect  BTL_CALC_WazaIDtoSideEffectID( WazaID waza );
extern WazaID  BTL_CALC_SideEffectIDtoWazaID( BtlSideEffect sideEffect );



extern WazaSick BTL_CALC_CheckMentalSick( const BTL_POKEPARAM* bpp );




// 勝ったとき・負けた時のおこづかい計算
extern u32 BTL_CALC_WinMoney( const BATTLE_SETUP_PARAM* sp );
extern u32 BTL_CALC_LoseMoney( u8 badgeCount, const BTL_PARTY* party );



extern void BTL_CALC_ITEM_InitSystem( HEAPID heapID );
extern void BTL_CALC_ITEM_QuitSystem(void);
extern s32  BTL_CALC_ITEM_GetParam( u16 item, u16 paramID );

extern WazaID BTL_CALC_RandWaza( const u16* excludeWazaTbl, u16 tblElems );
extern u32 BTL_CALC_CalcBaseExp( const BTL_POKEPARAM* bpp );


//=============================================================================================
//=============================================================================================
extern u8 BTL_RULE_GetNumFrontPos( BtlRule rule );
extern BOOL BTL_RULE_IsNeedSelectTarget( BtlRule rule );
extern u8 BTL_RULE_HandPokeIndex( BtlRule rule, u8 numCoverPos );
extern WazaTarget BTL_CALC_GetNoroiTargetType( const BTL_POKEPARAM* attacker );


//--------------------------------------------------------------
/**
 *  逃げたクライアント情報
 */
//--------------------------------------------------------------
typedef struct {
  u32 count;
  u8  clientID[ BTL_CLIENT_MAX ];
}BTL_ESCAPEINFO;

extern void BTL_ESCAPEINFO_Clear( BTL_ESCAPEINFO* info );
extern void BTL_ESCAPEINFO_Add( BTL_ESCAPEINFO* info, u8 clientID );
extern u32 BTL_ESCAPEINFO_GetCount( const BTL_ESCAPEINFO* info );
extern BtlResult BTL_ESCAPEINFO_CheckWinner( const BTL_ESCAPEINFO* info, u8 myClientID, BtlCompetitor competitor );


//=============================================================================================
/**
 * ワザターゲットの自動決定（ランダム）
 */
//=============================================================================================
extern BtlPokePos BTL_CALC_DecideWazaTargetAuto( const BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokeCon, const BTL_POKEPARAM* bpp, WazaID waza );

extern u32 BTL_CALC_MulRatio( u32 value, fx32 ratio );
extern u32 BTL_CALC_MulRatio_OverZero( u32 value, fx32 ratio );
extern u32 BTL_CALC_MulRatioInt( u32 value, u32 ratio );
extern u32 BTL_CALC_MulRatioInt_OverZero( u32 value, u32 ratio );


/**
 *  確率事象のチェック
 */
extern u32 BTL_CALC_IsOccurPer( u32 per );

/**
 *  数値切り上げ
 */
extern int BTL_CALC_Roundup( int value, int min );

/**
 *  ポケモンの最大HP * 1/N を計算
 */
extern u32 BTL_CALC_QuotMaxHP_Zero( const BTL_POKEPARAM* bpp, u32 denom );

/**
 *  ポケモンの最大HP * 1/N を計算（最低１になるように補正）
 */
extern u32 BTL_CALC_QuotMaxHP( const BTL_POKEPARAM* bpp, u32 denom );



/**
 *  min以上 ～ max以下の範囲内で乱数取得
 */
extern  u32 BTL_CALC_RandRange( u32 min, u32 max );

static inline void BTL_CALC_BITFLG_Construction( u8* flags, u8 bufsize )
{
  u32 i;
  flags[0] = bufsize;
  for(i=1; i<bufsize; ++i){
    flags[i] = 0;
  }
}
static inline void BTL_CALC_BITFLG_Set( u8* flags, u32 index )
{
  u8 byte = 1 + index / 8;
  u8 bit = index % 8;

  if( byte < flags[0] ){
    flags[ byte ] |= (1 << bit);
  }
}
static inline BOOL BTL_CALC_BITFLG_Check( const u8* flags, u32 index )
{
  u8 byte = 1 + index / 8;
  u8 bit = index % 8;

  if( byte < flags[0] ){
    return (flags[ byte ] & (1 << bit)) != 0;
  }
  return 0;
}
static inline void BTL_CALC_BITFLG_Off( u8* flags, u32 index )
{
  u8 byte = 1 + index / 8;
  u8 bit = index & 8;
  if( byte < flags[0] ){
    flags[ byte ] &= (~((u8)(1 << bit)));
  }
}

static inline u32 BTL_CALC_ABS( int value )
{
  if( value < 0 ){
    value *= -1;
  }
  return value;
}

#endif

