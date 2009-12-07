//======================================================================
/**
 * @file	field_debug.c
 * @brief	フィールドデバッグ処理
 * @author	kagaya
 * @date	08.09.29
 */
//======================================================================
#ifndef FIELD_DEBUG_H_FILE
#define FIELD_DEBUG_H_FILE
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"

#include "field/fieldmap_proc.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
typedef struct _TAG_FIELD_DEBUG_WORK FIELD_DEBUG_WORK;

//======================================================================
//	extern
//======================================================================
extern FIELD_DEBUG_WORK * FIELD_DEBUG_Init(
	FIELDMAP_WORK *pFieldMainWork, u8 bg_frame, HEAPID heapID );
extern void FIELD_DEBUG_Delete( FIELD_DEBUG_WORK *work );
extern void FIELD_DEBUG_UpdateProc( FIELD_DEBUG_WORK *work );

extern void FIELD_DEBUG_SetPosPrint( FIELD_DEBUG_WORK *work );
extern void FIELD_DEBUG_RecoverBgCont( FIELD_DEBUG_WORK *work );

#endif //FIELD_DEBUG_H_FILE
