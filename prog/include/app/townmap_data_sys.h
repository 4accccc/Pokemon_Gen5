//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		townmap_data_sys.h
 *	@brief	タウンマップデータ読み込み
 *	@author	Toru=Nagihashi
 *	@date		2009.07.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "townmap_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	タウンマップ用パラメータ
//=====================================
typedef enum 
{
	TOWNMAP_DATA_PARAM_ZONE_ID,
	TOWNMAP_DATA_PARAM_ZONE_SEARCH, //いらないかも
	TOWNMAP_DATA_PARAM_POS_X,
	TOWNMAP_DATA_PARAM_POS_Y,
	TOWNMAP_DATA_PARAM_CURSOR_X,
	TOWNMAP_DATA_PARAM_CURSOR_Y,
	TOWNMAP_DATA_PARAM_HIT_START_X,
	TOWNMAP_DATA_PARAM_HIT_START_Y,
	TOWNMAP_DATA_PARAM_HIT_END_X,
	TOWNMAP_DATA_PARAM_HIT_END_Y,
	TOWNMAP_DATA_PARAM_HIT_WIDTH,
	TOWNMAP_DATA_PARAM_PLACE_TYPE,
	TOWNMAP_DATA_PARAM_SKY_FLAG,
	TOWNMAP_DATA_PARAM_WARP_X,
	TOWNMAP_DATA_PARAM_WARP_Y,
	TOWNMAP_DATA_PARAM_ARRIVE_FLAG,
	TOWNMAP_DATA_PARAM_GUIDE_MSGID,
	TOWNMAP_DATA_PARAM_PLACE1_MSGID,
	TOWNMAP_DATA_PARAM_PLACE2_MSGID,
	TOWNMAP_DATA_PARAM_PLACE3_MSGID,
	TOWNMAP_DATA_PARAM_PLACE4_MSGID,
	TOWNMAP_DATA_PARAM_PLACE5_MSGID,
	TOWNMAP_DATA_PARAM_PLACE6_MSGID,

	TOWNMAP_DATA_PARAM_MAX,
} TOWNMAP_DATA_PARAM;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	タウンマップデータ
//=====================================
typedef struct _TOWNMAP_DATA TOWNMAP_DATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern TOWNMAP_DATA *TOWNMAP_DATA_Alloc( HEAPID heapID );
extern void TOWNMAP_DATA_Free( TOWNMAP_DATA *p_wk );

extern u16 TOWNMAP_DATA_GetParam( const TOWNMAP_DATA *cp_wk, u16 idx, TOWNMAP_DATA_PARAM param );

//field_townmap.hのFIELD_TOWNMAP_GetRootZoneID関数で得たZONEIDを以下に渡すと
//上記タウンマップデータのidxを返す
extern u16 TOWNMAP_DATA_SearchRootZoneID( const TOWNMAP_DATA *cp_wk, u16 zoneID );
