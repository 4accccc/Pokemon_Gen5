/**
 *  @file   effect_encount.h
 *  @brief  エンカウントエフェクト
 *  @author Miyuki Iwasawa
 *  @date   09.11.17
 */

#pragma once

#include "gamesystem/gamedata_def.h"
#include "gamesystem/gamesystem.h"
#include "field_encount.h"

////////////////////////////////////////////////////////////////////////////
//
typedef struct _TAG_EFFECT_ENCOUNT EFFECT_ENCOUNT;

#define EFFENC_DEFAULT_INTERVAL (50)  //エフェクト抽選デフォルトインターバル
#define EFFENC_DEFAULT_PROB (20)  //エフェクト抽選確率デフォルト

/**
 *  @brief  エンカウントエフェクトワーク生成
 */
extern EFFECT_ENCOUNT* EFFECT_ENC_CreateWork( HEAPID heapID );

/*
 *  @brief  エンカウントエフェクトワーク破棄
 */
extern void EFFECT_ENC_DeleteWork( EFFECT_ENCOUNT* eff_wk );

/*
 *  @brief  エフェクトエンカウント　システム終了処理
 */
extern void EFFECT_ENC_End( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

/*
 *  @brief  エフェクトエンカウント システム初期化
 */
extern void EFFECT_ENC_Init( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );



/*
 *  @brief  エンカウントエフェクト起動チェック
 */
extern void EFFECT_ENC_CheckEffectEncountStart( FIELD_ENCOUNT* enc );

/*
 *  @brief  エフェクトエンカウント  強制破棄　
 */
extern void EFFECT_ENC_EffectDelete( FIELD_ENCOUNT* enc );

/*
 *  @brief  エフェクトエンカウント　OBJとの接触によるエフェクト破棄チェック
 */
extern void EFFECT_ENC_CheckObjHit( FIELD_ENCOUNT* enc );

/*
 *  @brief  エフェクトエンカウント　座標チェック
 */
extern BOOL EFFECT_ENC_CheckEffectPos( const FIELD_ENCOUNT* enc, MMDL_GRIDPOS* pos );

/*
 *  @brief  エフェクトエンカウント　イベント侵入起動チェック
 */
extern GMEVENT* EFFECT_ENC_CheckEventApproch( FIELD_ENCOUNT* enc );

/*
 *  @brief  エフェクトエンカウント　自機位置との距離を返す
 *
 *  @retval エフェクトがないときはFALSEを返す
 */
extern BOOL EFFECT_ENC_GetDistanceToPlayer( FIELD_ENCOUNT* enc, u16* o_distance );

/*
 *  @brief  エフェクトエンカウント　フィールド生成時エフェクト復帰強制キャンセル
 */
extern void EFFECT_ENC_EffectRecoverCancel( FIELD_ENCOUNT* enc );

/*
 *  @brief  エフェクトエンカウント　アニメ再生ポーズ
 */
extern void EFFECT_ENC_EffectAnmPauseSet( FIELD_ENCOUNT* enc, BOOL pause_f );

/**
 * @brief   エフェクトエンカウトアイテム取得イベントで手に入れるItemNoを取得する
 * @retval itemno
 */
extern u16 EFFECT_ENC_GetEffectEncountItem( FIELD_ENCOUNT* enc );


/////////////////////////////////////////////////////////////////////////////////////
//デバッグ用ルーチン
#ifdef PM_DEBUG
enum{
  EFFENC_DNI_INTERVAL,
  EFFENC_DNI_PROB,
  EFFENC_DNI_OFSX,
  EFFENC_DNI_OFSZ,
};

extern u32 EFFENC_DEB_NumInputParamGet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param );
extern void EFFENC_DEB_NumInputParamSet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value );

#endif  //PM_DEBUG





