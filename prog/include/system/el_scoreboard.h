//============================================================================================
/**
 * @file	el_scoreboard.h
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#pragma once

typedef struct _EL_SCOREBOARD	EL_SCOREBOARD;

typedef enum {
	ELB_MODE_S = 0,
	ELB_MODE_T,
}ELB_MODE;

EL_SCOREBOARD*	ELBOARD_Add( const STRBUF* str, const ELB_MODE mode, HEAPID heapID );
void						ELBOARD_Delete( EL_SCOREBOARD* elb );
void						ELBOARD_Main( EL_SCOREBOARD* elb );
void						ELBOARD_Draw( EL_SCOREBOARD* elb, 
															VecFx32* trans, fx32 scale, u16 width, u16 height, 
															GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );

