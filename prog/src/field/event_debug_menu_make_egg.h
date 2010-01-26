#pragma once
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h" 

//--------------------------------------------------------------
/**
 * @brief タマゴ作成イベント生成
 *
 * @parma gameSystem
 * @param heapID
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_MakeEgg( GAMESYS_WORK* gameSystem, HEAPID heapID );
