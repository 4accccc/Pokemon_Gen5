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

FS_EXTERN_OVERLAY( notwifi_event_egg_birth );

//-------------------------------------------------------------------------------------
/**
 * @brief タマゴ孵化イベント 生成
 *
 * @param gameSystem
 * @param egg        孵化させるタマゴのPOKEMON_PARAM
 */
//-------------------------------------------------------------------------------------
extern GMEVENT* EVENT_EggBirth( GAMESYS_WORK* gameSystem, void* parent );

