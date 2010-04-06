/////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  ƒMƒ~ƒbƒN“o˜^ŠÖ”(ƒQ[ƒg)
 * @file   field_gimmick_gate.h
 * @author obata
 * @date   2009.10.21
 *
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fieldmap.h"
#include "gimmick_obj_elboard.h"


//===========================================================================
// ¡ƒMƒ~ƒbƒN“o˜^ŠÖ”
//===========================================================================
extern void GATE_GIMMICK_Setup( FIELDMAP_WORK* fieldmap );
extern void GATE_GIMMICK_End( FIELDMAP_WORK* fieldmap );
extern void GATE_GIMMICK_Move( FIELDMAP_WORK* fieldmap );


//---------------------------------------------------------------------------
/**
 * @brief “dŒõŒf¦”Â‚ÌŒü‚«‚ğæ“¾‚·‚é
 *
 * @param fieldmap
 *
 * @return “dŒõŒf¦”Â‚ÌŒü‚«( DIR_xxxx )
 */
//---------------------------------------------------------------------------
extern u8 GATE_GIMMICK_GetElboardDir( FIELDMAP_WORK* fieldmap );
