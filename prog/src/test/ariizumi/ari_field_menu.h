//======================================================================
/**
 *
 * @file	field_debug.c
 * @brief	フィールドデバッグ
 * @author	kagaya
 * @data	08.09.29
 */
//======================================================================
#ifndef ARI_FIELD_MENU_H__
#define ARI_FIELD_MENU_H__
//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
typedef struct _DEBUG_COMMMENU DEBUG_COMMMENU;

//======================================================================
//	extern
//======================================================================
extern DEBUG_COMMMENU * AriFldMenu_Init( u32 heapID );
extern void AriFldMenu_Delete( DEBUG_COMMMENU *d_menu );
extern void AriFldMenu_Create( DEBUG_COMMMENU *d_menu );
extern BOOL AriFldMenu_Main( DEBUG_COMMMENU *d_menu );

#endif //ARI_FIELD_MENU_H__
