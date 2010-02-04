//=============================================================================================
/**
 * @file  btl_pokeparam.h
 * @brief ポケモンWB バトルシステム  参加ポケモン戦闘用データ
 * @author  taya
 *
 * @date  2008.10.08  作成
 */
//=============================================================================================
#ifndef __BTL_POKEPARAM_H__
#define __BTL_POKEPARAM_H__

#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_personal.h"
#include "waza_tool/wazadata.h"


#include "btl_util.h"

//--------------------------------------------------------------
/**
 *  ハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_POKEPARAM BTL_POKEPARAM;

//--------------------------------------------------------------
/**
 *  取得できる能力値
 */
//--------------------------------------------------------------
typedef enum {

  BPP_VALUE_NULL = 0,

  BPP_ATTACK,
  BPP_DEFENCE,
  BPP_SP_ATTACK,
  BPP_SP_DEFENCE,
  BPP_AGILITY,
  BPP_HIT_RATIO,
  BPP_AVOID_RATIO,
  //--- ここまで能力ランクアップ／ダウン効果の引数としても用いる ---

  BPP_HP,
  BPP_MAX_HP,
  BPP_LEVEL,
  BPP_TOKUSEI,                ///< とくせい
  BPP_TOKUSEI_EFFECTIVE,      ///< 効果のあるとくせい（いえき中ならPOKETOKUSEI_NULL）
  BPP_SEX,
  BPP_FORM,
  BPP_ATTACK_RANK,
  BPP_DEFENCE_RANK,
  BPP_SP_ATTACK_RANK,
  BPP_SP_DEFENCE_RANK,
  BPP_AGILITY_RANK,
  BPP_EXP,

  BPP_RANKPARAM_START = BPP_ATTACK,
  BPP_RANKPARAM_END = BPP_AGILITY,
  BPP_RANKPARAM_RANGE = (BPP_RANKPARAM_END - BPP_RANKPARAM_START) + 1,

  BPP_RANKVALUE_START = BPP_ATTACK_RANK,
  BPP_RANKVALUE_END = BPP_AGILITY_RANK,
  BPP_RANKVALUE_RANGE = (BPP_RANKVALUE_END - BPP_RANKVALUE_START) + 1,

}BppValueID;

//--------------------------------------------------------------
/**
 *  ランク効果値の上限・下限・デフォルト値
 */
//--------------------------------------------------------------
enum {
  BPP_RANK_STATUS_MIN = 0,
  BPP_RANK_STATUS_MAX = 12,
  BPP_RANK_STATUS_DEFAULT = 6,
};

//--------------------------------------------------------------
/**
 *  ターン終了ごとにクリアされるフラグセット
 */
//--------------------------------------------------------------
typedef enum {

  BPP_TURNFLG_ACTION_DONE,   ///< 行動した
  BPP_TURNFLG_DAMAGED,       ///< ダメージ喰らった
  BPP_TURNFLG_WAZAPROC_DONE, ///< ワザ処理を修了した
  BPP_TURNFLG_SHRINK,        ///< ひるまされた
  BPP_TURNFLG_MUST_SHRINK,   ///< ダメージで必ずひるむ
  BPP_TURNFLG_MAMORU,        ///< “まもる”発動
  BPP_TURNFLG_FLYING,        ///< 宙に浮いてる
  BPP_TURNFLG_ITEM_CONSUMED, ///< アイテムを使用して無くなった

  BPP_TURNFLG_MAX,

}BppTurnFlag;

//--------------------------------------------------------------
/**
 *  永続的に保持されるフラグセット
 */
//--------------------------------------------------------------
typedef enum {

  BPP_CONTFLG_ACTION_DONE,      ///< 行動した
  BPP_CONTFLG_KODAWARI_LOCK,    ///< 最初に使ったワザしか出せなくなる
  BPP_CONTFLG_CANT_CHANGE,      ///< にげる・入れ替えが出来なくなる
  BPP_CONTFLG_TAME,
  BPP_CONTFLG_SORAWOTOBU,
  BPP_CONTFLG_DIVING,
  BPP_CONTFLG_ANAWOHORU,
  BPP_CONTFLG_MARUKUNARU,
  BPP_CONTFLG_TIISAKUNARU,
  BPP_CONTFLG_KIAIDAME,
  BPP_CONTFLG_POWERTRICK,
  BPP_CONTFLG_MIKURUNOMI,
  BPP_CONTFLG_CANT_ACTION,      ///< 反動で動けない

  BPP_CONTFLG_MAX,

}BppContFlag;

//--------------------------------------------------------------
/**
 *  各種カウンタ値
 */
//--------------------------------------------------------------
typedef enum {

  BPP_COUNTER_TAKUWAERU,          ///< たくわえる段階数
  BPP_COUNTER_TAKUWAERU_DEF,      ///< たくわえるによって上がった防御ランク
  BPP_COUNTER_TAKUWAERU_SPDEF,    ///< たくわえるによって上がった特防ランク
  BPP_COUNTER_MAMORU,             ///< まもる・みきりカウンタ

  BPP_COUNTER_MAX,
}BppCounter;

//--------------------------------------------------------------
/**
 *  HP残量めやす
 */
//--------------------------------------------------------------
typedef enum {

  BPP_HPBORDER_GREEN = 0,   ///< 通常
  BPP_HPBORDER_YELLOW,      ///< 1/3以下
  BPP_HPBORDER_RED,         ///< 1/8以下

}BppHpBorder;

//--------------------------------------------------------------
/**
 *  「こらえる（ワザダメージでHPが最低１残る）」ケース
 */
//--------------------------------------------------------------
typedef enum {

  BPP_KORAE_NONE = 0,       ///< こらえない
  BPP_KORAE_WAZA_DEFENDER,  ///< 防御側のワザによる（「こらえる」を使用など）
  BPP_KORAE_WAZA_ATTACKER,  ///< 攻撃側のワザによる（「みねうち」を使用など）
  BPP_KORAE_ITEM,           ///< 防御側の装備アイテム効果（きあいのタスキなど）
  BPP_KORAE_TOKUSEI_DEFENDER,  ///< 防御側のとくせいによる

}BppKoraeruCause;

//--------------------------------------------------------------
/**
 *  ワザダメージレコード関連定数
 */
//--------------------------------------------------------------
enum {
  BPP_WAZADMG_REC_TURN_MAX = 3,   ///< 何ターン分の記録を取るか？
  BPP_WAZADMG_REC_MAX = 6,        ///< １ターンにつき、何件分まで記録するか？
};
//--------------------------------------------------------------
/**
 *  ワザダメージレコード構造体
 */
//--------------------------------------------------------------
typedef struct {
  u16      wazaID;    ///< ワザID
  u16      damage;    ///< ダメージ量
  u8       wazaType;  ///< ほのお、みず等のタイプ
  u8       pokeID;    ///< 攻撃したポケモンID
}BPP_WAZADMG_REC;

static inline void BPP_WAZADMG_REC_Setup( BPP_WAZADMG_REC* rec, u8 pokeID, u16 wazaID, u8 wazaType, u16 damage )
{
  rec->wazaID = wazaID;
  rec->damage = damage;
  rec->wazaType = wazaType;
  rec->pokeID = pokeID;
}

//--------------------------------------------------------------
/**
 *  レベルアップ時の各種パラメータ上昇値
 */
//--------------------------------------------------------------
typedef struct {
  u8  level;
  u8  hp;
  u8  atk;
  u8  def;
  u8  sp_atk;
  u8  sp_def;
  u8  agi;
}BTL_LEVELUP_INFO;

extern BOOL BPP_AddExp( BTL_POKEPARAM* bpp, u32* exp, BTL_LEVELUP_INFO* info );
extern u32 BPP_GetExpMargin( const BTL_POKEPARAM* bpp );


//--------------------------------------------------------------
/**
 *  関数プロトタイプ
 */
//--------------------------------------------------------------
extern BTL_POKEPARAM*  BTL_POKEPARAM_Create( const POKEMON_PARAM* pp, u8 id, HEAPID heapID );
extern void BTL_POKEPARAM_Delete( BTL_POKEPARAM* bpp );
extern void BTL_POKEPARAM_Copy( BTL_POKEPARAM* dst, const BTL_POKEPARAM* src );

extern u8 BPP_GetID( const BTL_POKEPARAM* pp );
extern u16 BPP_GetMonsNo( const BTL_POKEPARAM* pp );
extern PokeTypePair BPP_GetPokeType( const BTL_POKEPARAM* pp );
extern BOOL BPP_IsMatchType( const BTL_POKEPARAM* pp, PokeType type );
extern const POKEMON_PARAM* BPP_GetSrcData( const BTL_POKEPARAM* bpp );
extern void BPP_SetViewSrcData( BTL_POKEPARAM* bpp, const POKEMON_PARAM* fakePP );
extern const POKEMON_PARAM* BPP_GetViewSrcData( const BTL_POKEPARAM* bpp );

extern u8 BPP_WAZA_GetCount( const BTL_POKEPARAM* pp );
extern u8 BPP_WAZA_GetUsedCount( const BTL_POKEPARAM* pp );
extern WazaID BPP_WAZA_GetID( const BTL_POKEPARAM* pp, u8 idx );
extern WazaID BPP_WAZA_GetParticular( const BTL_POKEPARAM* pp, u8 idx, u8* PP, u8* PPMax );
extern u8 BPP_WAZA_SearchIdx( const BTL_POKEPARAM* pp, WazaID waza );
extern u16 BPP_WAZA_GetPP( const BTL_POKEPARAM* pp, u8 wazaIdx );
extern u8 BPP_WAZA_GetPPShort( const BTL_POKEPARAM* bpp, u8 idx );

extern int BPP_GetValue( const BTL_POKEPARAM* pp, BppValueID vid );
extern int BPP_GetValue_Base( const BTL_POKEPARAM* pp, BppValueID vid );
extern int BPP_GetValue_Critical( const BTL_POKEPARAM* pp, BppValueID vid );

extern BOOL BPP_IsDead( const BTL_POKEPARAM* pp );
extern BOOL BPP_IsFightEnable( const BTL_POKEPARAM* bpp );
extern BOOL BPP_IsHPFull( const BTL_POKEPARAM* pp );
extern BOOL BPP_IsPPFull( const BTL_POKEPARAM* pp, u8 wazaIdx );

extern BOOL BPP_TURNFLAG_Get( const BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern BOOL BPP_CONTFLAG_Get( const BTL_POKEPARAM* pp, BppContFlag flagID );
extern u16 BPP_GetTurnCount( const BTL_POKEPARAM* pp );
extern u16 BPP_GetAppearTurn( const BTL_POKEPARAM* pp );
extern fx32  BPP_GetHPRatio( const BTL_POKEPARAM* pp );


extern void BPP_COUNTER_Set( BTL_POKEPARAM* bpp, BppCounter cnt, u8 value );
extern u8 BPP_COUNTER_Get( const BTL_POKEPARAM* bpp, BppCounter cnt );


//=============================================================================================
/**
 * 指定HPの値から、HP残量のめやす（普通・半減・ピンチとか）を返す
 *
 * @param   pp
 * @param   hp
 *
 * @retval  BppHpBorder
 */
//=============================================================================================
extern BppHpBorder BPP_CheckHPBorder( const BTL_POKEPARAM* pp, u32 hp );

//=============================================================================================
/**
 * 現在のHP残量のめやす（普通・半減・ピンチとか）を返す
 *
 * @param   pp
 *
 * @retval  BppHpBorder
 */
//=============================================================================================
extern BppHpBorder BPP_GetHPBorder( const BTL_POKEPARAM* pp );


//=============================================================================================
/**
 * ランク増減効果が効く（上限・下限に達していない）かどうか判定
 *
 * @param   pp
 * @param   rankType
 * @param   volume
 *
 * @retval  BOOL    効く場合TRUE
 */
//=============================================================================================
extern BOOL BPP_IsRankEffectValid( const BTL_POKEPARAM* pp, BppValueID rankType, int volume );

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
extern int BPP_RankEffectUpLimit( const BTL_POKEPARAM* pp, BppValueID rankType );

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
extern int BPP_RankEffectDownLimit( const BTL_POKEPARAM* pp, BppValueID rankType );

extern BOOL BPP_IsRankEffectDowned( const BTL_POKEPARAM* bpp );


//-------------------------
typedef void (*BtlSickTurnCheckFunc)( BTL_POKEPARAM* bpp, WazaSick sick, BPP_SICK_CONT oldCont, BOOL fCure, void* work );
typedef void( *BppCureWazaSickDependPokeCallback)( void* arg, BTL_POKEPARAM* bpp, WazaSick sickID, u8 dependPokeID );


extern void BPP_SetWazaSick( BTL_POKEPARAM* pp, WazaSick sick, BPP_SICK_CONT contParam );
extern void BPP_CurePokeSick( BTL_POKEPARAM* pp );
extern void BPP_CureWazaSick( BTL_POKEPARAM* pp, WazaSick sick );
extern void BPP_CureWazaSickDependPoke( BTL_POKEPARAM* pp, u8 depend_pokeID, BppCureWazaSickDependPokeCallback callBackFunc, void* callbackArg );
extern void BPP_WazaSick_TurnCheck( BTL_POKEPARAM* bpp, BtlSickTurnCheckFunc callbackFunc, void* callbackWork );
extern BOOL BPP_CheckNemuriWakeUp( BTL_POKEPARAM* bpp );
extern BOOL BPP_CheckKonranWakeUp( BTL_POKEPARAM* bpp );
extern void BPP_ReflectLevelup( BTL_POKEPARAM* bpp, u8 nextLevel, u8 hpMax, u8 atk, u8 def, u8 spAtk, u8 spDef, u8 agi );
extern void BPP_ReflectExpAdd( BTL_POKEPARAM* bpp );

extern BOOL BPP_CheckSick( const BTL_POKEPARAM* pp, WazaSick sickType );
extern PokeSick BPP_GetPokeSick( const BTL_POKEPARAM* pp );
extern u8 BPP_GetSickParam( const BTL_POKEPARAM* pp, WazaSick sick );
extern int  BPP_CalcSickDamage( const BTL_POKEPARAM* pp, WazaSick sickID );
extern BPP_SICK_CONT BPP_GetSickCont( const BTL_POKEPARAM* bpp, WazaSick sick );
extern u8 BPP_GetSickTurnCount( const BTL_POKEPARAM* bpp, WazaSick sick );

//-------------------------
extern u8 BPP_RankUp( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern u8 BPP_RankDown( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern void BPP_RankSet( BTL_POKEPARAM* pp, BppValueID rankType, u8 value );
extern void BPP_HpMinus( BTL_POKEPARAM* pp, u16 value );
extern void BPP_HpPlus( BTL_POKEPARAM* pp, u16 value );
extern void BPP_HpZero( BTL_POKEPARAM* pp );
extern void BPP_PPMinus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );
extern void BPP_PPPlus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );
extern void BPP_TURNFLAG_Set( BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern void BPP_TURNFLAG_ForceOff( BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern void BPP_TurnCheck( BTL_POKEPARAM* pp );
extern void BPP_CONTFLAG_Set( BTL_POKEPARAM* pp, BppContFlag flagID );
extern void BPP_CONTFLAG_Clear( BTL_POKEPARAM* pp, BppContFlag flagID );
extern void BPP_ChangeTokusei( BTL_POKEPARAM* pp, PokeTokusei tok );
extern void BPP_SetAppearTurn( BTL_POKEPARAM* pp, u16 turn );
extern void BPP_ChangePokeType( BTL_POKEPARAM* pp, PokeTypePair type );
extern void BPP_ChangeForm( BTL_POKEPARAM* pp, u8 formNo );

extern u32 BPP_GetItem( const BTL_POKEPARAM* pp );
extern void BPP_RemoveItem( BTL_POKEPARAM* pp );
extern void BPP_ConsumeItem( BTL_POKEPARAM* pp );
extern void BPP_ClearConsumedItem( BTL_POKEPARAM* bpp );
extern u16 BPP_GetConsumedItem( const BTL_POKEPARAM* bpp );

extern void BPP_UpdateWazaProcResult( BTL_POKEPARAM* bpp, BtlPokePos actTargetPos, BOOL fActEnable, WazaID actWaza, WazaID orgWaza );

extern WazaID  BPP_GetPrevWazaID( const BTL_POKEPARAM* bpp );
extern WazaID  BPP_GetPrevOrgWazaID( const BTL_POKEPARAM* bpp );
extern BtlPokePos  BPP_GetPrevTargetPos( const BTL_POKEPARAM* bpp );
extern u32  BPP_GetWazaContCounter( const BTL_POKEPARAM* bpp );


extern BOOL BPP_RankRecover( BTL_POKEPARAM* pp );
extern void BPP_RankReset( BTL_POKEPARAM* pp );
extern void BPP_Clear_ForDead( BTL_POKEPARAM* pp );
extern void BPP_Clear_ForOut( BTL_POKEPARAM* bpp );
extern void BPP_Clear_ForIn( BTL_POKEPARAM* bpp );
extern void BPP_SetItem( BTL_POKEPARAM* pp, u16 itemID );
extern void BPP_WAZA_SetUsedFlag( BTL_POKEPARAM* pp, u8 wazaIdx );
extern void BPP_WAZA_UpdateID( BTL_POKEPARAM* pp, u8 wazaIdx, WazaID waza, u8 ppMax, BOOL fPermenent );
extern BOOL BPP_WAZA_IsUsable( const BTL_POKEPARAM* bpp, WazaID waza );
extern void BPP_BatonTouchParam( BTL_POKEPARAM* target, const BTL_POKEPARAM* user );
extern void BPP_ReflectToPP( BTL_POKEPARAM* bpp );
extern void BPP_ReflectByPP( BTL_POKEPARAM* bpp );
extern void BPP_SetSrcPP( BTL_POKEPARAM* bpp, POKEMON_PARAM* pp );
extern BOOL BPP_IsFakeEnable( const BTL_POKEPARAM* bpp );
extern void BPP_FakeDisable( BTL_POKEPARAM* bpp );


extern u8 BPP_GetCriticalRank( const BTL_POKEPARAM* bpp );
extern BOOL BPP_AddCriticalRank( BTL_POKEPARAM* bpp, int value );
extern void BPP_SetBaseStatus( BTL_POKEPARAM* bpp, BppValueID vid, u16 value );
extern void BPP_SetWeight( BTL_POKEPARAM* bpp, u16 weight );
extern u16 BPP_GetWeight( const BTL_POKEPARAM* bpp );


extern BOOL BPP_HENSIN_Set( BTL_POKEPARAM* bpp, const BTL_POKEPARAM* target );
extern void BPP_HENSIN_Reset( BTL_POKEPARAM* bpp );
extern BOOL BPP_HENSIN_Check( const BTL_POKEPARAM* bpp );

extern void BPP_WAZADMGREC_Add( BTL_POKEPARAM* pp, const BPP_WAZADMG_REC* rec );
extern u8   BPP_WAZADMGREC_GetCount( const BTL_POKEPARAM* pp, u8 turn_ridx );
extern BOOL BPP_WAZADMGREC_Get( const BTL_POKEPARAM* pp, u8 turn_ridx, u8 rec_ridx, BPP_WAZADMG_REC* dst );

extern void BPP_MIGAWARI_Create( BTL_POKEPARAM* bpp, u16 migawariHP );
extern void BPP_MIGAWARI_Delete( BTL_POKEPARAM* bpp );
extern BOOL BPP_MIGAWARI_IsExist( const BTL_POKEPARAM* bpp );
extern BOOL BPP_MIGAWARI_AddDamage( BTL_POKEPARAM* bpp, u16 damage );

extern void BPP_CONFRONT_REC_Set( BTL_POKEPARAM* bpp, u8 pokeID );
extern u8 BPP_CONFRONT_REC_GetCount( const BTL_POKEPARAM* bpp );
extern u8 BPP_CONFRONT_REC_GetPokeID( const BTL_POKEPARAM* bpp, u8 idx );



#endif
