//============================================================================================
/**
 * @file  pokewood_setup.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.07.04
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "app/pkwd_count.h"
#include "app/pkwd_book.h"

//=============================================================================
//    ŠO•”ŒöŠJ
//=============================================================================
extern PKWD_COUNT_PARAM * PKWD_COUNT_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param );

#ifdef  PM_DEBUG
extern void PKWD_COUNT_PARAM_Setup_Debug( GAMESYS_WORK * gsys, PKWD_COUNT_PARAM * pkwd_param );
#endif

extern PKWD_BOOK_PARAM * PKWD_BOOK_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work );
extern void PKWD_BOOK_PARAM_Delete( PKWD_BOOK_PARAM * pkwd_param );



