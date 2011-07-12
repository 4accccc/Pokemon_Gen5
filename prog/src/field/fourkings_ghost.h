//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_ghost.h
 *	@brief  四天王部屋:ゴースト
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void FOURKINGS_GHOST_Setup(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_GHOST_End(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_GHOST_Move(FIELDMAP_WORK *p_fieldmap);

// スクリプトコマンドで呼ばれる関数
extern GMEVENT* FOURKINGS_GHOST_MistAnm( GAMESYS_WORK *gsys );
extern void FOURKINGS_GHOST_FireAnm( GAMESYS_WORK *gsys, int id );
extern GMEVENT* FOURKINGS_GHOST_BookFall( GAMESYS_WORK *gsys, int wait );
extern GMEVENT* FOURKINGS_GHOST_LampAnm( GAMESYS_WORK *gsys );

