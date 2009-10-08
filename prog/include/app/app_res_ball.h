//=============================================================================
/**
 * @file	app_res_ball.c
 * @brief	共通素材関係ソース
 * @author	genya_hosaka
 * @data	09/10/07
 *
 * モジュール名：APP_RES_BALL
 */
//=============================================================================
#pragma once

#include "app_res_def.h"

extern const u32 APP_RES_BALL_GetArcId( void );
extern u32 APP_RES_BALL_GetBallPltArcIdx( const BALL_ID ball );
extern u32 APP_RES_BALL_GetBallCharArcIdx( const BALL_ID ball );
extern u32 APP_RES_BALL_GetBallCellArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping );
extern u32 APP_RES_BALL_GetBallAnimeArcIdx( const BALL_ID ball, const APP_RES_MAPPING mapping );

