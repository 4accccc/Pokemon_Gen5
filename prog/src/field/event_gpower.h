//============================================================================================
/**
 * @file    event_gpower.h
 * @brief	  イベント：Gパワー関連イベント
 * @author  iwasawa	
 * @date	  2009.02.17
 */
//============================================================================================

#pragma once

#define SE_POWER_USE  (SEQ_SE_W295_01)

typedef struct _GPOWER_EFFECT_PARAM{
  GPOWER_ID g_power;
  BOOL      mine_f;
}GPOWER_EFFECT_PARAM;

FS_EXTERN_OVERLAY( event_gpower );

/*
 *  @brief  Gパワー発動イベント
 *
 *  @param  g_power 発動するGパワーID
 *  @param  mine    自分のGパワーかどうか？
 */
extern GMEVENT* EVENT_GPowerEffectStart(GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  Gパワー効果終了イベント
 */
extern GMEVENT* EVENT_GPowerEffectEnd( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  Gパワー効果発動エフェクト
 */
extern GMEVENT* EVENT_GPowerUseEffect( GAMESYS_WORK * gsys );

/*
 *  @brief  発動中のGパワー確認イベント
 */
extern GMEVENT* EVENT_GPowerEnableListCheck( GAMESYS_WORK * gsys, void* work );


