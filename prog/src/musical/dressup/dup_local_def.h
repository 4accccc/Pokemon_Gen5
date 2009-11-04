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

#if 0
#define DUP_TPrintf(...) (void)((ARI_TPrintf(__VA_ARGS__)))
#define DUP_Printf(...)  (void)((OS_Printf(__VA_ARGS__)))
#else
#define DUP_TPrintf(...) ((void)0)
#define DUP_Printf(...)  ((void)0)
#endif //DEB_ARI

struct _DRESSUP_INIT_WORK
{
  MUS_COMM_WORK *commWork;
	MUSICAL_POKE_PARAM *musPoke;
	MUSICAL_SAVE *mus_save;
};
