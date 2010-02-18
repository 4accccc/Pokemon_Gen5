//============================================================================================
/**
 * @file    event_gpower.h
 * @brief	  イベント：Gパワー関連イベント
 * @author  iwasawa	
 * @date	  2009.02.17
 */
//============================================================================================

#pragma once

/*
 *  @brief  Gパワー発動イベント
 *
 *  @param  g_power 発動するGパワーID
 *  @param  mine    自分のGパワーかどうか？
 */
extern GMEVENT* EVENT_GPowerEffectStart(GAMESYS_WORK * gsys, GPOWER_ID g_power, BOOL mine );

/*
 *  @brief  Gパワー効果終了イベント
 */
extern GMEVENT* EVENT_GPowerEffectEnd( GAMESYS_WORK * gsys );

/*
 *  @brief  Gパワー効果発動エフェクト
 */
extern GMEVENT* EVENT_GPowerUseEffect( GAMESYS_WORK * gsys );



