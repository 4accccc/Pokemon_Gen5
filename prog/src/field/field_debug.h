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

typedef enum {
  FIELD_DEBUG_PRINT_TYPE_POSITION = 0,
  FIELD_DEBUG_PRINT_TYPE_MEMORY,
}FIELD_DEBUG_PRINT_TYPE;

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
extern void FIELD_DEBUG_Draw( FIELD_DEBUG_WORK *work );

extern void FIELD_DEBUG_SetPrint( FIELD_DEBUG_WORK *work, FIELD_DEBUG_PRINT_TYPE type );
extern void FIELD_DEBUG_SetHitchCheckPrint( FIELD_DEBUG_WORK *work );
extern void FIELD_DEBUG_SetPosUpdateFlag( FIELD_DEBUG_WORK *work, BOOL flag );
extern void FIELD_DEBUG_RecoverBgCont( FIELD_DEBUG_WORK *work );

extern void FIELD_DEBUG_SetDrawCallBackFunc(const BOOL sw);
extern void FIELD_DEBUG_ClearDrawCallBackWork(void);

extern void FIELD_DEBUG_AddDrawLandNum(void);

#endif //FIELD_DEBUG_H_FILE
