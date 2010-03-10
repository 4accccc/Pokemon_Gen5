//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_movepoke.h
 *	@brief  移動ポケモン　イベント
 *	@author	tomoya takahashi
 *	@date		2010.03.09
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gflib.h"
#include "gamesystem/gamesystem.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ポケモン動作アニメーションタイプ
//  src/field/script_def.hの定義と一致している必要がある。
//=====================================
typedef enum {
  EV_MOVEPOKE_ANIME_RAI_KAZA_INSIDE,
  EV_MOVEPOKE_ANIME_RAI_KAZA_OUTSIDE,

  EV_MOVEPOKE_ANIME_TUCHI_INSIDE,
  EV_MOVEPOKE_ANIME_TUCHI_OUTSIDE,

  EV_MOVEPOKE_ANIME_MAX,
    
} EV_MOVEPOKE_ANIME_TYPE;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	移動ポケモン　動作ワーク
//=====================================
typedef struct _EV_MOVEPOKE_WORK EV_MOVEPOKE_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// 各ポケモン動作イベント
extern EV_MOVEPOKE_WORK* EVENT_MOVEPOKE_Create( FIELDMAP_WORK* p_fieldmap, u16 id, const VecFx32* cp_pos, HEAPID heapID );
extern void EVENT_MOVEPOKE_Delete( EV_MOVEPOKE_WORK* p_wk );

extern void EVENT_MOVEPOKE_StartAnime( EV_MOVEPOKE_WORK* p_wk, EV_MOVEPOKE_ANIME_TYPE anime_no );
extern BOOL EVENT_MOVEPOKE_IsAnimeEnd( const EV_MOVEPOKE_WORK* cp_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif



