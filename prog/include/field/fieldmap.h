//======================================================================
/**
 * @file	fieldmap.h
 * @brief
 * @date	2008.11.18
 * @author	tamada GAME FREAK inc.
 */
//======================================================================
#pragma once

typedef struct _FIELD_MAIN_WORK FIELD_MAIN_WORK;

extern FIELD_MAIN_WORK * FIELDMAP_Create(GAMESYS_WORK * gsys, HEAPID heapID );
extern void	FIELDMAP_Delete( FIELD_MAIN_WORK * fieldWork );
extern BOOL	FIELDMAP_Main( GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldWork );

extern void FIELDMAP_Close( FIELD_MAIN_WORK * fieldWork );

