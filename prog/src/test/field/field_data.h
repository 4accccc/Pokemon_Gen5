//============================================================================================
/**
 * @file	field_data.h
 */
//============================================================================================
#pragma once
#include "field_main.h"
//------------------------------------------------------------------
//------------------------------------------------------------------

typedef struct _DEPEND_FUNCTIONS DEPEND_FUNCTIONS;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const FLD_G3D_MAPPER_RESIST * FIELDDATA_GetMapperData(u16 mapid);
extern const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid);
extern const VecFx32 * FIELDDATA_GetStartPosition(u16 mapid);
extern u16 FIELDDATA_GetMapIDMax(void);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const DEPEND_FUNCTIONS FieldBaseFunctions;
extern const DEPEND_FUNCTIONS FieldNoGridFunctions;
extern const DEPEND_FUNCTIONS FieldGridFunctions;

