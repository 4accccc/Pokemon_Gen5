//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		tpoke_data.h
 *	@brief  連れ歩きポケモン情報  OBJCODE 大きさ
 *	@author	tomoya takahashi
 *	@date		2010.03.23
 *
 *	モジュール名：TPOKE_DATA
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gamesystem/game_data.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	連れ歩きポケモン情報
//=====================================
typedef struct _TPOKE_DATA TPOKE_DATA;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// データ読み込み・破棄
extern TPOKE_DATA* TPOKE_DATA_Create( HEAPID heapID );
extern void TPOKE_DATA_Delete( TPOKE_DATA* p_wk );

// オブジェコード取得
extern u16 TPOKE_DATA_GetObjCode( const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno );
// 多きさ取得
extern BOOL TPOKE_DATA_IsSizeBig( const GAMEDATA* cp_gdata, const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno );
extern BOOL TPOKE_DATA_IsSizeNormal( const GAMEDATA* cp_gdata, const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno );


#ifdef _cplusplus
}	// extern "C"{
#endif



