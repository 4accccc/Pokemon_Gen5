//============================================================================
/**
 *
 *	@file		intro_mcss.h
 *	@brief
 *	@author		hosaka genya
 *	@data		2009.12.18
 *
 */
//============================================================================
#pragma once

#include <gflib.h>
#include "system/mcss.h"

typedef struct _INTRO_MCSS_WORK INTRO_MCSS_WORK;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================



extern INTRO_MCSS_WORK* INTRO_MCSS_Create( HEAPID heap_id );

extern void INTRO_MCSS_Exit( INTRO_MCSS_WORK* wk );

extern void INTRO_MCSS_Main( INTRO_MCSS_WORK* wk );

extern void INTRO_MCSS_Add( INTRO_MCSS_WORK* wk, fx32 px, fx32 py, fx32 pz, const MCSS_ADD_WORK* add, u8 id );

extern void INTRO_MCSS_SetVisible( INTRO_MCSS_WORK* wk, BOOL is_visible, u8 id );

//@TODO ロード
//@TODO 表示切替




