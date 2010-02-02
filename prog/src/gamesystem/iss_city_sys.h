////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_city_sys.h
 * @brief  äXISSÉVÉXÉeÉÄ
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/playerwork.h"

typedef struct _ISS_CITY_SYS ISS_CITY_SYS;



//===========================================================================================
// Å°ê∂ê¨/îjä¸
//===========================================================================================
extern ISS_CITY_SYS* ISS_CITY_SYS_Create( PLAYER_WORK* player, HEAPID heapID );
extern void          ISS_CITY_SYS_Delete( ISS_CITY_SYS* system );


//===========================================================================================
// Å°ìÆçÏ/êßå‰
//===========================================================================================
extern void ISS_CITY_SYS_Main      ( ISS_CITY_SYS* system );
extern void ISS_CITY_SYS_On        ( ISS_CITY_SYS* system );
extern void ISS_CITY_SYS_Off       ( ISS_CITY_SYS* system );
extern void ISS_CITY_SYS_ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID );
