//=============================================================================================
/**
 * @file  btl_server_local.h
 * @brief ポケモンWB バトルシステム サーバー処理共有定数など
 * @author  taya
 *
 * @date  2009.02.26  作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_LOCAL_H__
#define __BTL_SERVER_LOCAL_H__

typedef enum {

  SV_WAZAFAIL_NULL = 0,
  SV_WAZAFAIL_PP_ZERO,
  SV_WAZAFAIL_NEMURI,
  SV_WAZAFAIL_MAHI,
  SV_WAZAFAIL_KOORI,
  SV_WAZAFAIL_KONRAN,
  SV_WAZAFAIL_SHRINK,
  SV_WAZAFAIL_MEROMERO,
  SV_WAZAFAIL_KANASIBARI,
  SV_WAZAFAIL_TYOUHATSU,
  SV_WAZAFAIL_ICHAMON,
  SV_WAZAFAIL_FUUIN,
  SV_WAZAFAIL_KAIHUKUHUUJI,
  SV_WAZAFAIL_HPFULL,
  SV_WAZAFAIL_FUMIN,
  SV_WAZAFAIL_NAMAKE,
  SV_WAZAFAIL_WAZALOCK,
  SV_WAZAFAIL_TOKUSEI,
  SV_WAZAFAIL_JURYOKU,
  SV_WAZAFAIL_TO_RECOVER,

  SV_WAZAFAIL_OTHER,

}SV_WazaFailCause;


typedef enum {

  SV_DMGCAUSE_WAZA,         ///< ワザによる直接ダメージ
  SV_DMGCAUSE_WAZA_REACT,   ///< ワザを使ったことによる反動ダメージ
  SV_DMGCAUSE_WAZA_EFFECT,  ///< ワザによって発声した各種効果（やどりぎ、まきびし等）
  SV_DMGCAUSE_CONFUSE,      ///< こんらん自爆によるダメージ
  SV_DMGCAUSE_POKESICK,     ///< ポケモン系状態異常（どく・やけど）によるダメージ
  SV_DMGCAUSE_WEATHER,      ///< 天候（すなあらし、あられ等）によるダメージ
  SV_DMGCAUSE_TOKUSEI,      ///< 各種とくせいによって引き起こされるダメージ（さめはだ、ゆうばく等）
  SV_DMGCAUSE_ITEM,         ///< アイテム効果によって引き起こされるダメージ（どくどくだま等）


}SV_DamageCause;

#endif

