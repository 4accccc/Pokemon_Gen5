//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_pokesearch.h
 *	@brief  ポケモン検索
 *	@author	Toru=Nagihashi
 *	@date		2009.11.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "br_res.h"
#include "savedata/zukan_savedata.h"
#include "system/bmp_oam.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ポケモン検索選択結果
//=====================================
typedef enum
{
  BR_POKESEARCH_SELECT_NONE,
  BR_POKESEARCH_SELECT_CANCEL,
  BR_POKESEARCH_SELECT_DECIDE,
} BR_POKESEARCH_SELECT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ポケモン検索ワーク
//=====================================
typedef struct _BR_POKESEARCH_WORK BR_POKESEARCH_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern BR_POKESEARCH_WORK *BR_POKESEARCH_Init( const ZUKAN_SAVEDATA *cp_zkn, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit,  BMPOAM_SYS_PTR p_bmpoam, HEAPID heapID );
extern void BR_POKESEARCH_Exit( BR_POKESEARCH_WORK *p_wk );
extern void BR_POKESEARCH_Main( BR_POKESEARCH_WORK *p_wk );
extern void BR_POKESEARCH_StartUp( BR_POKESEARCH_WORK *p_wk );
extern void BR_POKESEARCH_CleanUp( BR_POKESEARCH_WORK *p_wk );
extern BR_POKESEARCH_SELECT BR_POKESEARCH_GetSelect( const BR_POKESEARCH_WORK *cp_wk, u32 *p_mons_no );
