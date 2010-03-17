//======================================================================
/**
 * @file    pdw_postman.h
 * @date    2010.03.17
 * @brief   PDW”z’BˆõŠÖ˜A
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "fieldmap.h"
#include "savedata/dreamworld_data.h"

//======================================================================
//======================================================================

extern GMEVENT * DEBUG_EVENT_PDW_PostmanWindow( GAMESYS_WORK * gsys );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern u16 PDW_POSTMAN_searchNGItem(
    GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws, u32 ng_count );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern u32 PDW_POSTMAN_GetNGCount(
    GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws );


//--------------------------------------------------------------
//--------------------------------------------------------------
extern void PDW_POSTMAN_ReceiveItems(
    GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern GMEVENT * PDW_POSTMAN_CreateInfoEvent( GAMESYS_WORK * gsys );
