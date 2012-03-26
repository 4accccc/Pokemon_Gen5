//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		inf_dungeon_retire.h
 *	@brief  無限ダンジョンリタイア画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "field/field_subscreen.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//=============================================================================
//
//					構造体宣言
//
//=============================================================================
typedef struct _INFRETIRESUBSCRN_WORK  INFRETIRESUBSCRN_WORK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern INFRETIRESUBSCRN_WORK* INFRETIRESUBSCRN_Init( FIELD_SUBSCREEN_WORK* p_sub,GAMESYS_WORK* p_gamesys ,HEAPID heapID);
extern void INFRETIRESUBSCRN_Main( INFRETIRESUBSCRN_WORK* p_wk );
extern void INFRETIRESUBSCRN_Exit( INFRETIRESUBSCRN_WORK* p_wk );
