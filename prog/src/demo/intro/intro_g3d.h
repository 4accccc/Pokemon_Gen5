//============================================================================
/**
 *
 *	@file		intro_g3d.h
 *	@brief
 *	@author	hosaka genya
 *	@data		2009.12.20
 *
 */
//============================================================================
#pragma once

//=============================================================================
/**
 *								íËêîíËã`
 */
//=============================================================================

//=============================================================================
/**
 *								ç\ë¢ëÃíËã`
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERNêÈåæ
 */
//=============================================================================

typedef struct _INTRO_G3D_WORK INTRO_G3D_WORK;

extern INTRO_G3D_WORK* INTRO_G3D_Create( INTRO_GRAPHIC_WORK* graphic , HEAPID heap_id );
extern void INTRO_G3D_Exit( INTRO_G3D_WORK* wk );
extern void INTRO_G3D_Main( INTRO_G3D_WORK* wk );

extern BOOL INTRO_G3D_SelectStart( INTRO_G3D_WORK* wk );

extern void INTRO_G3D_SelectDecideStart( INTRO_G3D_WORK* wk, BOOL is_man );
extern BOOL INTRO_G3D_SelectDecideWait( INTRO_G3D_WORK* wk );

extern void INTRO_G3D_SelectVisible( INTRO_G3D_WORK* wk, BOOL is_visible );

extern void INTRO_G3D_SelectSet( INTRO_G3D_WORK* wk, u32 in_frame );
extern BOOL INTRO_G3D_SelectDecideReturn( INTRO_G3D_WORK* wk );

