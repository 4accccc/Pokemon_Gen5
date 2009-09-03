//======================================================================
/**
 * @file	dup_local_def.h
 * @brief	ミュージカル　ドレスアップ用定義
 * @author	ariizumi
 * @data	09/05/29
 */
//======================================================================

#pragma once

#include "musical/musical_system.h"
#include "savedata/musical_save.h"
#include "musical/comm/mus_comm_func.h"

struct _DRESSUP_INIT_WORK
{
  MUS_COMM_WORK *commWork;
	MUSICAL_POKE_PARAM *musPoke;
	MUSICAL_SAVE *mus_save;
};
