//======================================================================
/**
 *
 * @file	field_debug.c
 * @brief	フィールドデバッグ
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================
#ifndef __FIELD_DEBUG_H__
#define __FIELD_DEBUG_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
typedef struct _DEBUG_FLDMENU DEBUG_FLDMENU;

//======================================================================
//	extern
//======================================================================
extern DEBUG_FLDMENU * FldDebugMenu_Init( u32 heapID );
extern void FldDebugMenu_Delete( DEBUG_FLDMENU *d_menu );
extern void FldDebugMenu_Create( DEBUG_FLDMENU *d_menu );
extern BOOL FldDebugMenu_Main( DEBUG_FLDMENU *d_menu );

#endif //__FIELD_DEBUG_H__
