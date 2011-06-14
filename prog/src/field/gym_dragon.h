//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		gym_dragon.h
//   @brief   ドラゴンジム
//   @author	Masayuki Onoue
//   @date		2011.06.09
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void GYM_DRAGON_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_DRAGON_End(FIELDMAP_WORK *fieldWork);
extern void GYM_DRAGON_Move(FIELDMAP_WORK *fieldWork);

// スクリプトコマンド用
extern  GMEVENT * GYM_DRAGON_CreateButtonEvent( GAMESYS_WORK *p_gamesys, u16 event_seq );

