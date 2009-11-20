/**
 *  @file   effect_encount.h
 *  @brief  エンカウントエフェクト
 *  @author Miyuki Iwasawa
 *  @date   09.11.17
 */

#pragma once

////////////////////////////////////////////////////////////////////////////
//
typedef struct _TAG_EFFECT_ENCOUNT EFFECT_ENCOUNT;

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
 *  @brief  エフェクトエンカウント　イベント侵入起動チェック
 */
extern GMEVENT* EFFECT_ENC_CheckEventApproch( FIELD_ENCOUNT* enc );

