//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_evil.h
 *	@brief  四天王部屋:悪
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

extern void FOURKINGS_EVIL_Setup(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_EVIL_End(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_EVIL_Move(FIELDMAP_WORK *p_fieldmap);

// スクリプトコマンドで呼ばれる関数
extern void FOURKINGS_EVIL_FireAnm( GAMESYS_WORK *gsys, int id );
extern void FOURKINGS_EVIL_ClawAnm( GAMESYS_WORK *gsys, int id );

