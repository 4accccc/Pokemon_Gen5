//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_field.c
 *	@brief  レゾナンスリゾートフィールドギミック
 *	@author	Toru=Nagihashi
 *	@data		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK
#include "resort_npc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define RESORT_FIELD_ASSIGN_ID  (1)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//レゾナンスリゾート
extern void RESORT_FIELD_Setup( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_FIELD_End( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_FIELD_Move( FIELDMAP_WORK *p_fieldmap );

//スクリプトコマンドに使用
extern RESORT_PEOPLE_SYS * RESORT_FIELD_GetPeopleSys( FIELDMAP_WORK *p_fieldmap );
extern RESORT_DATA_MANAGER * RESORT_FIELD_GetDataManager( FIELDMAP_WORK *p_fieldmap );
extern RESORT_BINARY_SHOP * RESORT_FIELD_GetBinaryShop( FIELDMAP_WORK *p_fieldmap );
extern RESORT_BINARY_SPOT * RESORT_FIELD_GetBinarySpot( FIELDMAP_WORK *p_fieldmap );
extern RESORT_BINARY_CITY * RESORT_FIELD_GetBinaryCity(  FIELDMAP_WORK *p_fieldmap );
extern RESORT_NPC_SYS * RESORT_FIELD_GetNpcSys( FIELDMAP_WORK *p_fieldmap );
