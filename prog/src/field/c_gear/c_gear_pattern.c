//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		c_gear_pattern.c
 *	@brief  C Gear 初期　パターン　管理
 *	@author	tomoya takahashi
 *	@date		2010.04.05
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "c_gear.naix"

#include "c_gear_pattern.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

#define CGEAR_PATTERN_DATA_SIZE ( sizeof(u8) * (C_GEAR_PANEL_WIDTH * C_GEAR_PANEL_HEIGHT))

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  patternの状態に変更
 *
 *	@param	gamedata    ゲームデータ
 *	@param	pattern     pattern
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void CGEAR_PATTERN_SetUp( CGEAR_SAVEDATA* pCGSV, ARCHANDLE* handle, u32 pattern, HEAPID heapID )
{
  GF_ASSERT( pattern < CGEAR_PATTERN_MAX );

  GFL_ARC_LoadDataByHandle( handle, pattern+NARC_c_gear_pattern01_bin, CGEAR_SV_GetPanelTypeBuff( pCGSV ) );
}

