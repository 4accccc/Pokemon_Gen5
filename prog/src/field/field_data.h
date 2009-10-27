//============================================================================================
/**
 * @file	field_data.h
 */
//============================================================================================
#pragma once
#include "gamesystem/pm_season.h"
#include "field/map_matrix.h"

#include "field/areadata.h"
#include "fieldmap.h"
#include "fieldmap_resist.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELDDATA_SetMapperData(u16 mapid, const AREADATA * areadata,
    FLDMAPPER_RESISTDATA * map_res, MAP_MATRIX *matrix_buf );
extern const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid);
extern FLDMAP_CTRLTYPE FIELDDATA_GetFieldCtrlType(u16 mapid);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const DEPEND_FUNCTIONS FieldBaseFunctions;
extern const DEPEND_FUNCTIONS FieldNoGridFunctions;
extern const DEPEND_FUNCTIONS FieldGridFunctions;

