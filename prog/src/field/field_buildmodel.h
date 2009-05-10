//============================================================================================
/**
 * @file	field_buildmodel.h
 * @brief		îzíuÉÇÉfÉãÇÃêßå‰
 * @author	tamada GAMEFREAK inc.
 * @date	2009.04.24
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "field_g3d_mapper.h"
#include "fieldmap_resist.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef u16 BMODEL_ID;
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FIELD_BMODEL_MAN FIELD_BMODEL_MAN;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_BMODEL_MAN * FIELD_BMODEL_MAN_Create(HEAPID heapID);

extern void FIELD_BMODEL_MAN_Delete(FIELD_BMODEL_MAN * man);

extern void FIELD_BMODEL_MAN_Load(FIELD_BMODEL_MAN * man, u16 zoneid);

extern const FLDMAPPER_RESISTDATA_OBJTBL * FIELD_BMODEL_MAN_GetOBJTBL(const FIELD_BMODEL_MAN * man);
//------------------------------------------------------------------
//------------------------------------------------------------------
extern u16 FIELD_BMODEL_MAN_GetNarcIndex(const FIELD_BMODEL_MAN * man, BMODEL_ID id);

extern u16 FIELD_BMODEL_MAN_GetEntryIndex(BMODEL_ID id);

extern FIELD_BMODEL_MAN * FLDMAPPER_GetBuildModelManager( FLDMAPPER* g3Dmapper);

