//=============================================================================================
/**
 * @file  wazadata.h
 * @brief ポケモンWB  ワザデータアクセサ
 * @author  taya
 *
 * @date  2008.10.14  作成
 */
//=============================================================================================
#ifndef __WAZADATA_H__
#define __WAZADATA_H__

#include "poke_tool/poketype.h"
#include "battle/battle.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef  u16  WazaID;


//------------------------------------------------------------------------------
/**
 *  ワザのカテゴリ最上位分類
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZADATA_CATEGORY_SIMPLE_DAMAGE,
  WAZADATA_CATEGORY_SIMPLE_SICK,
  WAZADATA_CATEGORY_SIMPLE_EFFECT,
  WAZADATA_CATEGORY_SIMPLE_RECOVER,
  WAZADATA_CATEGORY_DAMAGE_SICK,
  WAZADATA_CATEGORY_EFFECT_SICK,
  WAZADATA_CATEGORY_DAMAGE_EFFECT,
  WAZADATA_CATEGORY_DAMAGE_EFFECT_USER,
  WAZADATA_CATEGORY_DRAIN,
  WAZADATA_CATEGORY_ICHIGEKI,
  WAZADATA_CATEGORY_GUARD,
  WAZADATA_CATEGORY_WEATHER,
  WAZADATA_CATEGORY_FIELD_EFFECT,
  WAZADATA_CATEGORY_SIDE_EFFECT,
  WAZADATA_CATEGORY_EXPLOSION,
  WAZADATA_CATEGORY_PUSHOUT,
  WAZADATA_CATEGORY_OTHERS,

  WAZADATA_CATEGORY_MAX,

}WazaCategory;

//------------------------------------------------------------------------------
/**
 *  ワザのダメージ種類
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZADATA_DMG_NONE = 0,
  WAZADATA_DMG_PHYSIC,  ///< 物理ダメージ
  WAZADATA_DMG_SPECIAL, ///< 特殊ダメージ

}WazaDamageType;

//------------------------------------------------------------------------------
/**
 *  ワザによって引き起こされる状態異常。
 *  PokeSickをバトル用に拡張したもの。
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZASICK_ORIGIN = POKESICK_ORIGIN,

  WAZASICK_DOKU = POKESICK_DOKU,
  WAZASICK_MAHI = POKESICK_MAHI,
  WAZASICK_NEMURI = POKESICK_NEMURI,
  WAZASICK_KOORI = POKESICK_KOORI,
  WAZASICK_YAKEDO = POKESICK_YAKEDO,

  WAZASICK_KONRAN = POKESICK_MAX,
  WAZASICK_MEROMERO,
  WAZASICK_BIND,
  WAZASICK_AKUMU,
  WAZASICK_NOROI,
  WAZASICK_TYOUHATSU,
  WAZASICK_ICHAMON,
  WAZASICK_KANASIBARI,
  WAZASICK_AKUBI,
  WAZASICK_KAIHUKUHUUJI,
  WAZASICK_IEKI,            ///< とくせい無効化
  WAZASICK_MIYABURU,        ///< みやぶられて相性ゼロでも等倍で殴られる＆回避率上昇が無効
  WAZASICK_YADORIGI,        ///< やどりぎのたね
  WAZASICK_SASIOSAE,        ///< さしおさえ
  WAZASICK_HOROBINOUTA,     ///< ほろびのうた
  WAZASICK_NEWOHARU,        ///< ねをはる
  WAZASICK_TOOSENBOU,       ///< にげる・いれかえが出来ない
  WAZASICK_ENCORE,          ///< 直前に使ったワザしか出せない
  WAZASICK_WAZALOCK,        ///< 直前に使ったワザしか出せない上、行動も選択できない
  WAZASICK_MUSTHIT,         ///< ワザ必中状態
  WAZASICK_MUSTHIT_TARGET,  ///< 特定相手に対してのみ必中状態
  WAZASICK_FLYING,          ///< でんじふゆう
  WAZASICK_POWERTRICK,      ///< パワートリック

  WAZASICK_MAX,
  WAZASICK_NULL = POKESICK_NULL,

}WazaSick;

//------------------------------------------------------------------------------
/**
 *  状態異常の継続タイプ分類
 */
//------------------------------------------------------------------------------
typedef enum {
  WAZASICK_CONT_NONE = 0,

  WAZASICK_CONT_PERMANENT,  ///< ずっと継続
  WAZASICK_CONT_TURN,       ///< ターン数継続
  WAZASICK_CONT_POKE,       ///< ワザをかけたポケモンが居る間、継続
  WAZASICK_CONT_POKETURN,   ///< ターン数経過か＆ポケモンが場から消えるまでを両方チェック

}WazaSickCont;

//------------------------------------------------------
/**
 *  状態異常の継続パラメータ
 */
//------------------------------------------------------
typedef struct {

  union {
    u16 raw;
    struct {
      u16  type    : 4;   ///< 継続タイプ（ WazaSickCont ）
      u16  turnMin : 6;   ///< 継続ターン最小
      u16  turnMax : 6;   ///< 継続ターン最大
    };
  };

}WAZA_SICKCONT_PARAM;


//------------------------------------------------------------------------------
/**
 *  ワザによって発生する追加効果（攻撃等のランクアップ・ダウン効果）
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZA_RANKEFF_NULL = 0,
  WAZA_RANKEFF_ORIGIN = 1,

  WAZA_RANKEFF_ATTACK = WAZA_RANKEFF_ORIGIN,
  WAZA_RANKEFF_DEFENCE,
  WAZA_RANKEFF_SP_ATTACK,
  WAZA_RANKEFF_SP_DEFENCE,
  WAZA_RANKEFF_AGILITY,
  WAZA_RANKEFF_HIT,
  WAZA_RANKEFF_AVOID,

  WAZA_RANKEFF_MAX,
  WAZA_RANKEFF_NUMS = WAZA_RANKEFF_MAX - WAZA_RANKEFF_ORIGIN,

  WAZA_RANKEFF_CRITICAL_RATIO = WAZA_RANKEFF_AVOID+1,

}WazaRankEffect;

//------------------------------------------------------------------------------
/**
 *  ワザの効果範囲
 */
//------------------------------------------------------------------------------
typedef enum {

  // ポケモン対象
  WAZA_TARGET_OTHER_SELECT,       ///< 通常ポケ（１体選択）
  WAZA_TARGET_FRIEND_USER_SELECT, ///< 自分を含む味方ポケ（１体選択）
  WAZA_TARGET_FRIEND_SELECT,      ///< 自分以外の味方ポケ（１体選択）
  WAZA_TARGET_ENEMY_SELECT,       ///< 相手側ポケ（１体選択）
  WAZA_TARGET_OTHER_ALL,          ///< 自分以外の全ポケ
  WAZA_TARGET_ENEMY_ALL,          ///< 相手側全ポケ
  WAZA_TARGET_FRIEND_ALL,         ///< 味方側全ポケ
  WAZA_TARGET_USER,               ///< 自分のみ
  WAZA_TARGET_ALL,                ///< 場に出ている全ポケ
  WAZA_TARGET_ENEMY_RANDOM,       ///< 相手ポケ１体ランダム

  // ポケモン以外対象
  WAZA_TARGET_FIELD,              ///< 場全体（天候など）
  WAZA_TARGET_SIDE_ENEMY,         ///< 敵側陣営
  WAZA_TARGET_SIDE_FRIEND,        ///< 自分側陣営

  // 特殊
  WAZA_TARGET_UNKNOWN,            ///< ゆびをふるなど特殊型

}WazaTarget;

//------------------------------------------------------------------------------
/**
 *  ワザ優先度
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZAPRI_MAX = 5,
  WAZAPRI_MIN = -7,

  WAZAPRI_RANGE = (WAZAPRI_MAX - WAZAPRI_MIN + 1),

}WazaPriority;

//------------------------------------------------------------------------------
/**
 *  ワザイメージ分類
 */
//------------------------------------------------------------------------------
typedef enum {
  WAZA_IMG_NULL = 0,    ///< イメージなし

  WAZA_IMG_PUNCH,       ///< パンチ（てつのこぶし対象）
  WAZA_IMG_SOUND,       ///< サウンド（ぼうおん対象）
  WAZA_IMG_HEAT,        ///< 高熱（こおり解除）
  WAZA_IMG_FLY,         ///< 上空に飛ぶ（じゅうりょく時、使用不可）

  WAZA_IMG_MAX,
}WazaImage;

//------------------------------------------------------------------------------
/**
 *  各種フラグ
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZAFLAG_MAMORU = 0,    ///< 「まもる」でガード対象となるワザ
  WAZAFLAG_TAME,          ///< １ターン溜めワザ

  WAZAFLAG_MAX,

}WazaFlag;

//------------------------------------------------------------------------------
/**
 *  ワザ関連定数
 */
//------------------------------------------------------------------------------
enum {
  WAZA_RANKEFF_NUM_MAX = 2,   ///< １つのワザに割り当てられるランク効果の種類数最大値
};

extern u16  WAZADATA_GetPower( WazaID id );
extern s8   WAZADATA_GetPriority( WazaID id );


// MaxPP値
extern u8 WAZADATA_GetMaxPP( WazaID id, u8 upCnt );

// カテゴリ
extern WazaCategory  WAZADATA_GetCategory( WazaID id );

// タイプ
extern PokeType WAZADATA_GetType( WazaID id );

// 命中率
extern u8  WAZADATA_GetHitRatio( WazaID id );

// 複数回連続攻撃ワザの最大回数。
extern u16  WAZADATA_GetContiniusCount( WazaID id );

// 必中フラグ。つばめがえしなどがTRUEを返す。
extern BOOL WAZADATA_IsAlwaysHit( WazaID id );

// ダメージワザかどうか
extern BOOL WAZADATA_IsDamage( WazaID id );

// 接触ワザかどうか
extern BOOL WAZADATA_IsTouch( WazaID id );

// ダメージタイプ
extern WazaDamageType  WAZADATA_GetDamageType( WazaID id );

// クリティカルランク
extern u8 WAZADATA_GetCriticalRank( WazaID id );

// 最大ヒット回数
extern u8 WAZADATA_GetMaxHitCount( WazaID id );

// 効果範囲
extern WazaTarget WAZADATA_GetTarget( WazaID id );

// 追加効果でひるむ確率
extern u32 WAZADATA_GetShrinkPer( WazaID id );

// 天候効果
extern BtlWeather WAZADATA_GetWeather( WazaID id );

// 状態異常効果
extern WazaSick WAZADATA_GetSick( WazaID id );

// 状態異常継続パラメータ
extern WAZA_SICKCONT_PARAM WAZADATA_GetSickCont( WazaID id );

// 追加効果で状態異常になる確率
extern u32 WAZADATA_GetSickPer( WazaID id );

// 疲れて行動不可になるワザか？
extern BOOL WAZADATA_IsTire( WazaID id );

// ワザフラグ取得
extern BOOL WAZADATA_GetFlag( WazaID id, WazaFlag flag );

//=============================================================================================
/**
 * ランク効果の種類数を取得
 *
 * @param   id        [in] ワザID
 * @param   volume    [out] ランク効果の程度（+ならアップ, -ならダウン, 戻り値==WAZA_RANKEFF_NULLの時のみ0）
 *
 * @retval  WazaRankEffect    ランク効果ID
 */
//=============================================================================================
extern u8 WAZADATA_GetRankEffectCount( WazaID id );

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
extern WazaRankEffect  WAZADATA_GetRankEffect( WazaID id, u32 idx, int* volume );

// ランク効果の発生確率（0-100）
extern u32 WAZADATA_GetRankEffPer( WazaID id );

//=============================================================================================
/**
 * ワザイメージチェック
 *
 * @param   id    ワザID
 * @param   img   イメージID
 *
 * @retval  BOOL    ワザが指定されたイメージに属する場合はTRUE
 */
//=============================================================================================
extern BOOL WAZADATA_IsImage( WazaID id, WazaImage img );

//=============================================================================================
/**
 * 反動率（相手に与えたダメージの何％を反動として自分が受けるか）を返す
 *
 * @param   id
 *
 * @retval  u8    反動率（反動なしなら0, max100）
 */
//=============================================================================================
extern u8 WAZADATA_GetReactionRatio( WazaID id );

//=============================================================================================
/**
 * ドレイン技のダメージ値->回復HP還元率
 *
 * @param   id
 *
 * @retval  u8
 */
//=============================================================================================
extern u8 WAZADATA_GetDrainRatio( WazaID id );

extern u8 WAZADATA_GetRecoverHPRatio( WazaID id );


/*-----------------------------------------------------------------


使用できるか？
必ず失敗するか？
威力は？
ダメージは？
当たったか？
回復は？
追加効果は？


-----------------------------------------------------------------*/

#ifdef PM_DEBUG
extern void WAZADATA_PrintDebug( void );
#else
#define WAZADATA_PrintDebug() /* */
#endif


#endif
