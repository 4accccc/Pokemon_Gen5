//======================================================================
/**
 *
 * @file	dlplay_disp_func
 * @brief	ダウンロード親機の表示周り
 * @author	ariizumi
 * @data	08/10/31
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

//======================================================================
//	define
//======================================================================
typedef struct _DLPLAY_DISP_SYS DLPLAY_DISP_SYS;
struct DLPLAY_BOX_INDEX;
//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
extern DLPLAY_DISP_SYS*	DLPlayDispSys_InitSystem( int heapID );
extern void	DLPlayDispSys_TermSystem( DLPLAY_DISP_SYS *dispSys );
extern void	DLPlayDispSys_UpdateDraw( DLPLAY_DISP_SYS *dispSys );
extern void DLPlayDispSys_DispBoxIcon( DLPLAY_BOX_INDEX *boxData , u8 trayNo , DLPLAY_DISP_SYS *dispSys );
//======================================================================
//	proto
//======================================================================
