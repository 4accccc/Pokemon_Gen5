//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_reception.h
 *	@brief  レゾナンスリゾート迎賓館ギミック
 *	@author	Toru=Nagihashi
 *	@date		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK
#include "resort_people.h"
#include "resort_sys.h"
#include "resort_npc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
#define RESORT_RECEPTION_ASSIGN_ID  (0) //resort_fieldのアサインIDと異なるようにすること

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//レゾナンスリゾート迎賓館
extern void RESORT_RECEPTION_Setup( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_RECEPTION_End( FIELDMAP_WORK *p_fieldmap );
extern void RESORT_RECEPTION_Move( FIELDMAP_WORK *p_fieldmap );

//スクリプトコマンドに使用
extern RESORT_PEOPLE_SYS * RESORT_RECEPTION_GetPeopleSys( FIELDMAP_WORK *p_fieldmap );
extern RESORT_SYSTEM * RESORT_RECEPTION_GetResortSys( FIELDMAP_WORK *p_fieldmap );
extern RESORT_NPC_SYS * RESORT_RECEPTION_GetNpcSys( FIELDMAP_WORK *p_fieldmap );
