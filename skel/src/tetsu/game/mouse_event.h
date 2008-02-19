//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _MOUSE_EVENT_SYS MOUSE_EVENT_SYS;

typedef enum {
	MOUSE_EVENT_NONE = 0,

	MOUSE_EVENT_MOVESTART,
	MOUSE_EVENT_MOVE,
	MOUSE_EVENT_JUMP,

	MOUSE_EVENT_CAMERAMOVE_L,
	MOUSE_EVENT_CAMERAMOVE_R,
	MOUSE_EVENT_CAMERAMOVE_U,
	MOUSE_EVENT_CAMERAMOVE_D,

	MOUSE_EVENT_ACTION_1,
	MOUSE_EVENT_ACTION_2,
	MOUSE_EVENT_ACTION_3,
	MOUSE_EVENT_ACTION_4,
	MOUSE_EVENT_ACTION_5,
	MOUSE_EVENT_ACTION_6,
	MOUSE_EVENT_ACTION_7,
	MOUSE_EVENT_ACTION_8,
	MOUSE_EVENT_ACTION_9,
	MOUSE_EVENT_ACTION_10,
	MOUSE_EVENT_ACTION_11,
	MOUSE_EVENT_ACTION_12,
	MOUSE_EVENT_ACTION_13,

	MOUSE_EVENT_OPEN_MENU,

	MOUSE_EVENT_ATTACK_1,
	MOUSE_EVENT_ATTACK_2,
	MOUSE_EVENT_ATTACK_3,
	MOUSE_EVENT_ATTACK_4,
	MOUSE_EVENT_ATTACK_5,
	MOUSE_EVENT_ATTACK_6,
//åªèÛ32bitÇ‹Ç≈
}MOUSE_EVENT;

extern MOUSE_EVENT_SYS*	InitMouseEvent( GAME_SYSTEM* gs, HEAPID heapID );
extern void				ExitMouseEvent( MOUSE_EVENT_SYS* mes );
extern void				MainMouseEvent( MOUSE_EVENT_SYS* mes );

extern void				SetMouseLine( MOUSE_EVENT_SYS* mes, fx32 lineH );

extern void				StartMouseEvent( MOUSE_EVENT_SYS* mes );
extern BOOL				CheckMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID );
extern void				GetMousePos( MOUSE_EVENT_SYS* mes, VecFx32* pos );


