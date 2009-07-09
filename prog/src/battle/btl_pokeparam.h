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
  BPP_CRITICAL_RATIO,
  //--- ここまでランクアップ／ダウン効果の引数としても用いる ---

  BPP_HP,
  BPP_MAX_HP,
  BPP_LEVEL,
  BPP_TOKUSEI,
  BPP_SEX,
  BPP_FORM,
  BPP_ATTACK_RANK,
  BPP_DEFENCE_RANK,
  BPP_SP_ATTACK_RANK,
  BPP_SP_DEFENCE_RANK,
  BPP_AGILITY_RANK,

  BPP_RANKTYPE_MAX = BPP_AVOID_RATIO+1,

}BppValueID;

//--------------------------------------------------------------
/**
 *  ターン終了ごとにクリアされるフラグセット
 */
//--------------------------------------------------------------
typedef enum {

  BPP_TURNFLG_ACTION_DONE,  ///< 行動した
  BPP_TURNFLG_DAMAGED,      ///< ダメージ喰らった
  BPP_TURNFLG_WAZA_EXE,     ///< ワザを出した
  BPP_TURNFLG_SHRINK,       ///< ひるまされた
  BPP_TURNFLG_MAMORU,       ///< “まもる”発動
  BPP_TURNFLG_FLYING,       ///< 宙に浮いてる

  BPP_TURNFLG_MAX,

}BppTurnFlag;

//--------------------------------------------------------------
/**
 *  アクション選択ごとにクリアされるフラグセット
 */
//--------------------------------------------------------------
typedef enum {

  BPP_ACTFLG_CANT_ACTION,   ///< アクション選択不可

  BPP_ACTFLG_MAX,

}BppActFlag;


//--------------------------------------------------------------
/**
 *  永続的に保持されるフラグセット
 */
//--------------------------------------------------------------
typedef enum {

  BPP_CONTFLG_MEMBERIN_EFFECT,  ///< 入場直後
  BPP_CONTFLG_KODAWARI_LOCK,    ///< 最初に使ったワザしか出せなくなる
  BPP_CONTFLG_CANT_CHANGE,      ///< にげる・入れ替えが出来なくなる
  BPP_CONTFLG_TAME,
  BPP_CONTFLG_SORAWOTOBU,
  BPP_CONTFLG_DIVING,
  BPP_CONTFLG_ANAWOHORU,
  BPP_CONTFLG_MARUKUNARU,
  BPP_CONTFLG_TIISAKUNARU,

  BPP_CONTFLG_MAX,

}BppContFlag;

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
 *  関数プロトタイプ
 */
//--------------------------------------------------------------
extern BTL_POKEPARAM*  BTL_POKEPARAM_Create( const POKEMON_PARAM* pp, u8 id, HEAPID heapID );
extern void BTL_POKEPARAM_Delete( BTL_POKEPARAM* bpp );
extern void BTL_POKEPARAM_Copy( BTL_POKEPARAM* dst, const BTL_POKEPARAM* src );

extern u8 BTL_POKEPARAM_GetID( const BTL_POKEPARAM* pp );
extern u16 BTL_POKEPARAM_GetMonsNo( const BTL_POKEPARAM* pp );
extern PokeTypePair BTL_POKEPARAM_GetPokeType( const BTL_POKEPARAM* pp );
extern u32 BTL_POKEPARAM_GetItem( const BTL_POKEPARAM* pp );
extern BOOL BTL_POKEPARAM_IsMatchType( const BTL_POKEPARAM* pp, PokeType type );
extern const POKEMON_PARAM* BTL_POKEPARAM_GetSrcData( const BTL_POKEPARAM* bpp );

extern u8 BTL_POKEPARAM_GetWazaCount( const BTL_POKEPARAM* pp );
extern u8 BTL_POKEPARAM_GetUsedWazaCount( const BTL_POKEPARAM* pp );
extern WazaID BTL_POKEPARAM_GetWazaNumber( const BTL_POKEPARAM* pp, u8 idx );
extern WazaID BTL_POKEPARAM_GetWazaParticular( const BTL_POKEPARAM* pp, u8 idx, u8* PP, u8* PPMax );
extern u8 BTL_POKEPARAM_GetWazaIdx( const BTL_POKEPARAM* pp, WazaID waza );
extern u16 BTL_POKEPARAM_GetPP( const BTL_POKEPARAM* pp, u8 wazaIdx );

extern int BTL_POKEPARAM_GetValue( const BTL_POKEPARAM* pp, BppValueID vid );
extern int BTL_POKEPARAM_GetValue_Base( const BTL_POKEPARAM* pp, BppValueID vid );
extern int BTL_POKEPARAM_GetValue_Critical( const BTL_POKEPARAM* pp, BppValueID vid );

extern BOOL BTL_POKEPARAM_IsDead( const BTL_POKEPARAM* pp );
extern BOOL BTL_POKEPARAM_IsHPFull( const BTL_POKEPARAM* pp );
extern BOOL BTL_POKEPARAM_IsPPFull( const BTL_POKEPARAM* pp, u8 wazaIdx );

extern BOOL BTL_POKEPARAM_GetTurnFlag( const BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern BOOL BTL_POKEPARAM_GetActFlag( const BTL_POKEPARAM* pp, BppActFlag flagID );
extern BOOL BTL_POKEPARAM_GetContFlag( const BTL_POKEPARAM* pp, BppContFlag flagID );
extern u16 BTL_POKEPARAM_GetTurnCount( const BTL_POKEPARAM* pp );
extern u16 BTL_POKEPARAM_GetAppearTurn( const BTL_POKEPARAM* pp );

extern WazaID  BTL_POKEPARAM_GetPrevWazaNumber( const BTL_POKEPARAM* pp );
extern BtlPokePos  BTL_POKEPARAM_GetPrevTargetPos( const BTL_POKEPARAM* pp );
extern u32  BTL_POKEPARAM_GetSameWazaUsedCounter( const BTL_POKEPARAM* pp );
extern fx32  BTL_POKEPARAM_GetHPRatio( const BTL_POKEPARAM* pp );

extern void BTL_POKEPARAM_SetActionAgility( BTL_POKEPARAM* bpp, u16 actionAgility );
extern u16 BTL_POKEPARAM_GetActionAgility( const BTL_POKEPARAM* bpp );


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
extern BppHpBorder BTL_POKEPARAM_CheckHPBorder( const BTL_POKEPARAM* pp, u32 hp );

//=============================================================================================
/**
 * 現在のHP残量のめやす（普通・半減・ピンチとか）を返す
 *
 * @param   pp
 *
 * @retval  BppHpBorder
 */
//=============================================================================================
extern BppHpBorder BTL_POKEPARAM_GetHPBorder( const BTL_POKEPARAM* pp );


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
extern BOOL BTL_POKEPARAM_IsRankEffectValid( const BTL_POKEPARAM* pp, BppValueID rankType, int volume );

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
extern int BTL_POKEPARAM_RankEffectUpLimit( const BTL_POKEPARAM* pp, BppValueID rankType );

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
extern int BTL_POKEPARAM_RankEffectDownLimit( const BTL_POKEPARAM* pp, BppValueID rankType );

//-------------------------
typedef void (*BtlSickTurnCheckFunc)( BTL_POKEPARAM* bpp, WazaSick sick, BOOL fCure, void* work );


extern void BTL_POKEPARAM_SetWazaSick( BTL_POKEPARAM* pp, WazaSick sick, BPP_SICK_CONT contParam );
extern void BTL_POKEPARAM_CurePokeSick( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_CureWazaSick( BTL_POKEPARAM* pp, WazaSick sick );
extern void BTL_POKEPARAM_CureWazaSickDependPoke( BTL_POKEPARAM* pp, u8 depend_pokeID );
extern void BTL_POKEPARAM_WazaSick_TurnCheck( BTL_POKEPARAM* bpp, BtlSickTurnCheckFunc callbackFunc, void* callbackWork );

extern BOOL BTL_POKEPARAM_CheckSick( const BTL_POKEPARAM* pp, WazaSick sickType );
extern PokeSick BTL_POKEPARAM_GetPokeSick( const BTL_POKEPARAM* pp );
extern u8 BTL_POKEPARAM_GetSickParam( const BTL_POKEPARAM* pp, WazaSick sick );
extern int  BTL_POKEPARAM_CalcSickDamage( const BTL_POKEPARAM* pp, WazaSick sickID );

//-------------------------
extern u8 BTL_POKEPARAM_RankUp( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern u8 BTL_POKEPARAM_RankDown( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern void BTL_POKEPARAM_RankSet( BTL_POKEPARAM* pp, BppValueID rankType, u8 value );
extern void BTL_POKEPARAM_HpMinus( BTL_POKEPARAM* pp, u16 value );
extern void BTL_POKEPARAM_HpPlus( BTL_POKEPARAM* pp, u16 value );
extern void BTL_POKEPARAM_HpZero( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_PPMinus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );
extern void BTL_POKEPARAM_PPPlus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );
extern BOOL BTL_POKEPARAM_Nemuri_CheckWake( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_SetShrink( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_SetTurnFlag( BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern void BTL_POKEPARAM_ForceOffTurnFlag( BTL_POKEPARAM* pp, BppTurnFlag flagID );
extern void BTL_POKEPARAM_TurnCheck( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_SetActFlag( BTL_POKEPARAM* pp, BppActFlag flagID );
extern void BTL_POKEPARAM_ClearActFlag( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_SetContFlag( BTL_POKEPARAM* pp, BppContFlag flagID );
extern void BTL_POKEPARAM_ResetContFlag( BTL_POKEPARAM* pp, BppContFlag flagID );
extern void BTL_POKEPARAM_ChangeTokusei( BTL_POKEPARAM* pp, PokeTokusei tok );
extern void BTL_POKEPARAM_SetAppearTurn( BTL_POKEPARAM* pp, u16 turn );
extern void BTL_POKEPARAM_ChangePokeType( BTL_POKEPARAM* pp, PokeTypePair type );
extern void BTL_POKEPARAM_ChangeForm( BTL_POKEPARAM* pp, u8 formNo );
extern void BTL_POKEPARAM_RemoveItem( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_UpdateUsedWazaNumber( BTL_POKEPARAM* pp, WazaID waza, BtlPokePos targetPos );
extern void BTL_POKEPARAM_ResetUsedWazaNumber( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_RankRecover( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_RankReset( BTL_POKEPARAM* pp );
extern void BTL_POKEPARM_DeadClear( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_OutClear( BTL_POKEPARAM* bpp );
extern void BTL_POKEPARAM_SetItem( BTL_POKEPARAM* pp, u16 itemID );
extern void BTL_POKEPARAM_SetWazaUsed( BTL_POKEPARAM* pp, u8 wazaIdx );
extern void BTL_POKEPARAM_UpdateWazaNumber( BTL_POKEPARAM* pp, u8 wazaIdx, WazaID waza, u8 ppMax, BOOL fPermenent );

extern BOOL BTL_POKEPARAM_HENSIN_Set( BTL_POKEPARAM* bpp, const BTL_POKEPARAM* target );
extern void BTL_POKEPARAM_HENSIN_Reset( BTL_POKEPARAM* bpp );
extern BOOL BTL_POKEPARAM_HENSIN_Check( const BTL_POKEPARAM* bpp );

extern void BTL_POKEPARAM_WAZADMG_REC_Add( BTL_POKEPARAM* pp, const BPP_WAZADMG_REC* rec );
extern u8   BTL_POKEPARAM_WAZADMG_REC_GetCount( const BTL_POKEPARAM* pp, u8 turn_ridx );
extern BOOL BTL_POKEPARAM_WAZADMG_REC_Get( const BTL_POKEPARAM* pp, u8 turn_ridx, u8 rec_ridx, BPP_WAZADMG_REC* dst );


#endif
