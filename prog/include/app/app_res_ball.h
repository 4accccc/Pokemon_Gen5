//=============================================================================
/**
 * @file	  app_res_ball.h
 * @brief	  ボールリソース アクセサ
 * @author  genya_hosaka
 * @data	  09/10/07
 *
 * @note    2*2キャラのアイコンです。大きい方はアイテムアイコンから取得してください
 *
 * モジュール名：APP_RES_BALL
 */
//=============================================================================
#pragma once

#include "app_res_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//ボールアイコンのパレットの本数
#define APP_RES_BALL_PLT_NUM (1)

//=============================================================================
/**
 *							  EXTERN
 */
//=============================================================================
extern const u32 APP_RES_BALL_GetArcId( void );
extern u32 APP_RES_BALL_GetBallPltArcIdx( const BALL_ID ball );
extern u32 APP_RES_BALL_GetBallCharArcIdx( const BALL_ID ball );
extern u32 APP_RES_BALL_GetBallCellArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping );
extern u32 APP_RES_BALL_GetBallAnimeArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping );

