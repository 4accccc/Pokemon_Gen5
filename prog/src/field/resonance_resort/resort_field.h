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
#include "resort_people.h"
#include "resort_npc.h"
#include "resort_sys.h"

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
extern RESORT_NPC_SYS * RESORT_FIELD_GetNpcSys( FIELDMAP_WORK *p_fieldmap );
extern RESORT_SYSTEM * RESORT_FIELD_GetResortSys( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_FIELD_DeleteCrowdPeople( FIELDMAP_WORK *p_fieldmap, void* p_people );
extern void RESORT_FIELD_StartEffect( FIELDMAP_WORK *p_fieldmap, u32 id );
extern BOOL RESORT_FIELD_WaitEffect( FIELDMAP_WORK *p_fieldmap, u32 id );
extern void RESORT_FIELD_EndEffect( FIELDMAP_WORK *p_fieldmap, u32 id );

extern void RESORT_FIELD_StartPeople( FIELDMAP_WORK *p_fieldmap );

extern void RESORT_FIELD_AddCrowdTutorial( FIELDMAP_WORK *p_fieldmap );

extern void RESORT_FIELD_SetParade( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_FIELD_SetNoiseReq( FIELDMAP_WORK *p_fieldmap, BOOL is_visible );

#ifdef PM_DEBUG
extern void DEBUG_RESORT_FIELD_SetCheckMMDL( FIELDMAP_WORK *p_fieldmap, MMDL* p_mmdl );
#endif //PM_DEBUG
