//============================================================================================
/**
 * @file	field_data.h
 */
//============================================================================================
#pragma once
#include "fieldmap_resist.h"
//------------------------------------------------------------------
//------------------------------------------------------------------

typedef struct _DEPEND_FUNCTIONS DEPEND_FUNCTIONS;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELDDATA_SetMapperData(u16 mapid, FLDMAPPER_RESISTDATA * map_res, void * matrix_buf);
extern const FLDMAPPER_RESISTDATA * FIELDDATA_GetMapperData(u16 mapid);
extern const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid);
extern BOOL FIELDDATA_IsMatrixMap(u16 mapid);
extern u16 FIELDDATA_GetMapIDMax(void);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const DEPEND_FUNCTIONS FieldBaseFunctions;
extern const DEPEND_FUNCTIONS FieldNoGridFunctions;
extern const DEPEND_FUNCTIONS FieldGridFunctions;
extern const DEPEND_FUNCTIONS FieldTestC3Functions;

