//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_mapcreate.h
 *	@brief  レゾナンスリゾートランダムマップ作成
 *	@author	Toru=Nagihashi
 *	@date		2011.03.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "map/dp3format.h"
#include "field/field_buildmodel.h"
#include "field/field_g3d_map.h"
#include "field/resonance_resort_data.h"
#include "../field_g3d_mapper.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef enum
{
  RESORT_MAPCREATE_TYPE_FIELD,
  RESORT_MAPCREATE_TYPE_ROOM,
}RESORT_MAPCREATE_TYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _RESORT_MAPCREATE RESORT_MAPCREATE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern RESORT_MAPCREATE * RESORT_MAPCREATE_Init( HEAPID heapID );
extern void RESORT_MAPCREATE_Exit( RESORT_MAPCREATE *p_wk );

extern void RESORT_MAPCREATE_Setup( RESORT_MAPCREATE *p_wk, u16 zoneID, const RESORT_CITY_DATA *cp_city, const RESORT_PEOPLE_ALL_DATA *cp_people_all );

extern int RESORT_MAPCREATE_SetUpBlock( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, int map_index, HEAPID heapID );

extern void RESORT_MAPCREATE_EventDataOverWrite( RESORT_MAPCREATE *p_wk, EVENTDATA_SYSTEM* p_evdata );

//----------------------------------------------------------------------------
/**
 *	@brief リゾート用データ取得 実体はfield_g3d_mapper.c
 */
//-----------------------------------------------------------------------------
extern RESORT_MAPCREATE * FLDMAPPER_GetResortWork( const FLDMAPPER* g3Dmapper);
