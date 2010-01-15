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

#include <heapsys.h>

#include "poke_tool/poketype.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef  u16  WazaID;
typedef  struct _WAZA_DATA  WAZA_DATA;

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
  WAZADATA_CATEGORY_FIELD_EFFECT,
  WAZADATA_CATEGORY_SIDE_EFFECT,
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

  WAZASICK_MAHI = POKESICK_MAHI,
  WAZASICK_NEMURI = POKESICK_NEMURI,
  WAZASICK_KOORI = POKESICK_KOORI,
  WAZASICK_YAKEDO = POKESICK_YAKEDO,
  WAZASICK_DOKU = POKESICK_DOKU,

  WAZASICK_DOKUDOKU = POKESICK_MAX,
  WAZASICK_KONRAN,
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
  WAZASICK_FLYING_CANCEL,   ///< ふゆう状態になれない
  WAZASICK_TELEKINESIS,     ///< テレキネシス
  WAZASICK_FREEFALL,        ///< フリーフォール
  WAZASICK_HITRATIO_UP,     ///< 命中率上昇

  WAZASICK_MAX,
  WAZASICK_NULL = POKESICK_NULL,
  WAZASICK_STD_MAX = WAZASICK_MEROMERO+1, ///< 標準状態異常の数（メロメロまで）

  WAZASICK_SPECIAL_CODE = 0xffff, ///< 特殊処理コード

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

  WAZA_RANKEFF_SP = WAZA_RANKEFF_MAX,
  WAZA_RANKEFF_CRITICAL_RATIO,

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

  WAZA_TARGET_MAX,
  WAZA_TARGET_LONG_SELECT = WAZA_TARGET_MAX,  ///<対象選択表示用に定義
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
 *  ワザで発生する天候効果
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZA_WEATHER_NONE = 0,   ///< 天候なし

  WAZA_WEATHER_SHINE,    ///< はれ
  WAZA_WEATHER_RAIN,     ///< あめ
  WAZA_WEATHER_SNOW,     ///< あられ
  WAZA_WEATHER_SAND,     ///< すなあらし
  WAZA_WEATHER_MIST,     ///< きり

  WAZA_WEATHER_MAX,


}WazaWeather;

//------------------------------------------------------------------------------
/**
 *  各種フラグ
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZAFLAG_Touch = 0,     ///< 接触ワザ
  WAZAFLAG_Tame,          ///< １ターン溜めワザ
  WAZAFLAG_Tire,          ///< １ターン反動ワザ
  WAZAFLAG_Mamoru,        ///< 「まもる」でガード対象となるワザ
  WAZAFLAG_MagicCoat,     ///< 「マジックコート」対象
  WAZAFLAG_Yokodori,      ///< 「よこどり」対象
  WAZAFLAG_Oumu,          ///< 「おうむがえし」対象
  WAZAFLAG_Punch,         ///< 「てつのこぶし」対象（パンチのイメージ）
  WAZAFLAG_Sound,         ///< 「ぼうおん」対象（音による攻撃・効果のイメージ）
  WAZAFLAG_Flying,        ///< 重力時に失敗する（飛んだり浮いたりイメージ）
  WAZAFLAG_KooriMelt,     ///< 自分が凍りでも解除してワザを出す（高温イメージ）
  WAZAFLAG_TripleFar,     ///< トリプルバトルで、離れた相手も選択出来る

  WAZAFLAG_MAX,

}WazaFlag;

//------------------------------------------------------------------------------
/**
 *  ワザ関連定数
 */
//------------------------------------------------------------------------------
enum {
  WAZA_RANKEFF_NUM_MAX = 3,   ///< １つのワザに割り当てられるランク効果の種類数最大値
};


//------------------------------------------------------------------------------
/**
 *  ワザパラメータ種類
 */
//------------------------------------------------------------------------------
typedef enum {

  WAZAPARAM_TYPE,                ///< タイプ（みず、くさ、ほのお…）
  WAZAPARAM_CATEGORY,            ///< カテゴリ（ enum WazaCategory )
  WAZAPARAM_DAMAGE_TYPE,         ///< ダメージタイプ（ enum WazaDamageType )
  WAZAPARAM_POWER,               ///< 威力
  WAZAPARAM_HITPER,              ///< 命中率
  WAZAPARAM_BASE_PP,             ///< 基本PPMax
  WAZAPARAM_PRIORITY,            ///< ワザ優先度
  WAZAPARAM_CRITICAL_RANK,       ///< クリティカルランク
  WAZAPARAM_HITCOUNT_MAX,        ///< 最大ヒット回数
  WAZAPARAM_HITCOUNT_MIN,        ///< 最小ヒット回数
  WAZAPARAM_SHRINK_PER,          ///< ひるみ確率
  WAZAPARAM_SICK,                ///< 状態異常コード（enum WazaSick）
  WAZAPARAM_SICK_PER,            ///< 状態異常の発生率
  WAZAPARAM_SICK_CONT,           ///< 状態異常継続パターン（enum WazaSickCont）
  WAZAPARAM_SICK_TURN_MIN,       ///< 状態異常の継続ターン最小
  WAZAPARAM_SICK_TURN_MAX,       ///< 状態異常の継続ターン最大
  WAZAPARAM_RANKTYPE_1,          ///< ステータスランク効果１
  WAZAPARAM_RANKVALUE_1,         ///< ステータスランク効果１増減値
  WAZAPARAM_RANKPER_1,           ///< ステータスランク効果１の発生率
  WAZAPARAM_RANKTYPE_2,          ///< ステータスランク効果２
  WAZAPARAM_RANKVALUE_2,         ///< ステータスランク効果２増減値
  WAZAPARAM_RANKPER_2,           ///< ステータスランク効果２の発生率
  WAZAPARAM_RANKTYPE_3,          ///< ステータスランク効果３
  WAZAPARAM_RANKVALUE_3,         ///< ステータスランク効果３増減値
  WAZAPARAM_RANKPER_3,           ///< ステータスランク効果３の発生率
  WAZAPARAM_DAMAGE_RECOVER_RATIO,///< ダメージ回復率
  WAZAPARAM_HP_RECOVER_RATIO,    ///< HP回復率
  WAZAPARAM_TARGET,              ///< ワザ効果範囲( enum WazaTarget )

  WAZAPARAM_WEATHER,             ///< 天候
  WAZAPARAM_DAMAGE_REACTION_RATIO,  ///< ダメージ反動率
  WAZAPARAM_HP_REACTION_RATIO,      ///< HP反動率

  WAZAPARAM_MAX,

}WazaDataParam;



extern int WAZADATA_GetParam( WazaID id, WazaDataParam param );

extern const WAZA_DATA*  WAZADATA_Alloc( WazaID id, HEAPID heapID );
extern int WAZADATA_PTR_GetParam( const WAZA_DATA* wazaData, WazaDataParam param );

extern BOOL WAZADATA_GetFlag( WazaID id, WazaFlag flag );
extern BOOL WAZADATA_PTR_GetFlag( const WAZA_DATA* wazaData, WazaFlag flag );



// MaxPP値
extern u8 WAZADATA_GetMaxPP( WazaID id, u8 upCnt );

// 威力値
extern u16  WAZADATA_GetPower( WazaID id );

// タイプ
extern PokeType WAZADATA_GetType( WazaID id );

// ダメージタイプ
extern WazaDamageType  WAZADATA_GetDamageType( WazaID id );


// カテゴリ
extern WazaCategory  WAZADATA_GetCategory( WazaID id );

// 必中フラグ。つばめがえしなどがTRUEを返す。
extern BOOL WAZADATA_IsAlwaysHit( WazaID id );

// 必ずクリティカルフラグ
extern BOOL WAZADATA_IsMustCritical( WazaID id );

// ダメージワザかどうか
extern BOOL WAZADATA_IsDamage( WazaID id );

// クリティカルランク
extern u8 WAZADATA_GetCriticalRank( WazaID id );

// 天候効果
extern WazaWeather WAZADATA_GetWeather( WazaID id );

// 状態異常効果
extern WazaSick WAZADATA_GetSick( WazaID id );



//=============================================================================================
/**
 * 状態異常継続パラメータ取得
 *
 * @param   id
 *
 * @retval  extern WAZA_SICKCONT_PARAM
 */
//=============================================================================================
extern WAZA_SICKCONT_PARAM WAZADATA_GetSickCont( WazaID id );

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



#endif
