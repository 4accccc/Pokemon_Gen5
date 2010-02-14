//======================================================================
/**
 * @file  field_mystery_gift.h
 * @brief
 * @date  2010.02.12
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================

#pragma once

#include <gflib.h>

#include "savedata/mystery_data.h"

//======================================================================
//======================================================================
//--------------------------------------------------------------
extern u8 FIELD_MYSTERYDATA_GetGiftType( MYSTERY_DATA * fd );
extern GIFT_PACK_DATA * FIELD_MYSTERYDATA_GetGiftData( MYSTERY_DATA * fd, int * index );

