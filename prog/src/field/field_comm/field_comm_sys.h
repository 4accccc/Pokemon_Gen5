//==============================================================================
/**
 * @file  field_comm_sys.h
 * @brief
 * @author  matsuda
 * @date  2009.04.30(ñÿ)
 */
//==============================================================================
#pragma once

#include "field/field_comm/field_comm_def.h"


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern COMM_FIELD_SYS_PTR FIELD_COMM_SYS_Alloc(HEAPID commHeapID);
extern void FIELD_COMM_SYS_Free(COMM_FIELD_SYS_PTR comm_field);
extern FIELD_COMM_FUNC* FIELD_COMM_SYS_GetCommFuncWork( COMM_FIELD_SYS_PTR commField );
extern FIELD_COMM_DATA* FIELD_COMM_SYS_GetCommDataWork( COMM_FIELD_SYS_PTR commField );

