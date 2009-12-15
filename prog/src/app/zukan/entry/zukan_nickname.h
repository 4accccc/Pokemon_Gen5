//============================================================================
/**
 *  @file   zukan_nickname.h
 *  @brief  ニックネームを付けるか否か
 *  @author Koji Kawada
 *  @data   2009.12.10
 *  @note   
 *
 *  モジュール名：ZUKAN_NICKNAME
 */
//============================================================================
#pragma once

// lib
#include <gflib.h>

// system
#include "gamesystem/gamesystem.h"

//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
typedef enum
{
  ZUKAN_NICKNAME_RESULT_NO,
  ZUKAN_NICKNAME_RESULT_YES,
  ZUKAN_NICKNAME_RESULT_SELECT,
}
ZUKAN_NICKNAME_RESULT;

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _ZUKAN_NICKNAME_WORK ZUKAN_NICKNAME_WORK;

//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
extern ZUKAN_NICKNAME_WORK* ZUKAN_NICKNAME_Init( HEAPID a_heap_id, GFL_CLUNIT* a_clunit, GFL_FONT* a_font, PRINT_QUE* a_print_que );
extern void ZUKAN_NICKNAME_Exit( ZUKAN_NICKNAME_WORK* work );
extern ZUKAN_NICKNAME_RESULT ZUKAN_NICKNAME_Main( ZUKAN_NICKNAME_WORK* work );

