//============================================================================================
/**
 * @file	field_data.h
 */
//============================================================================================
#pragma once
#include "gamesystem/pm_season.h"
#include "fieldmap_resist.h"
#include "map_matrix.h"

#include "fieldmap.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELDDATA_SetMapperData(u16 mapid, u8 sid, FLDMAPPER_RESISTDATA * map_res, MAP_MATRIX *matrix_buf );
extern const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const DEPEND_FUNCTIONS FieldBaseFunctions;
extern const DEPEND_FUNCTIONS FieldNoGridFunctions;
extern const DEPEND_FUNCTIONS FieldGridFunctions;
extern const DEPEND_FUNCTIONS FieldTestC3Functions;

