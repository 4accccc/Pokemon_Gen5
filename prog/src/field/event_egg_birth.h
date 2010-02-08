///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  タマゴ孵化イベント
 * @file   event_egg_birth.h
 * @author obata
 * @date   2010.02.05
 */
///////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"
#include "poke_tool/poke_tool.h"

//-------------------------------------------------------------------------------------
/**
 * @brief タマゴ孵化イベント 生成
 *
 * @param gameSystem
 * @param fieldmap
 * @param egg        孵化させるタマゴ
 */
//-------------------------------------------------------------------------------------
extern GMEVENT* EVENT_EggBirth( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, POKEMON_PARAM* egg ); 
