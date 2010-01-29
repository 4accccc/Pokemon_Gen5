//////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_road_sys.h
 * @brief  道路ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.29
 */
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/playerwork.h"


// 道路ISSシステムの不完全型の宣言
typedef struct _ISS_ROAD_SYS ISS_ROAD_SYS;


//============================================================================
// ■システム
//============================================================================
// 作成/破棄
extern ISS_ROAD_SYS* ISS_ROAD_SYS_Create( PLAYER_WORK* player, HEAPID heapID );
extern void          ISS_ROAD_SYS_Delete( ISS_ROAD_SYS* system );

// 動作
extern void ISS_ROAD_SYS_Update( ISS_ROAD_SYS* system );

// 制御
extern void ISS_ROAD_SYS_On ( ISS_ROAD_SYS* system );
extern void ISS_ROAD_SYS_Off( ISS_ROAD_SYS* system );
