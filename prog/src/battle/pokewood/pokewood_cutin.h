//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	@file		pokewood_cutin.h
 *	@brief  カットインモジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.05.24
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "../btl_main_def.h"
#include "../btlv/btlv_core.h"

#include "pokewood_cutin_def.h"
#include "pokewood_senario.h"

//=============================================================================
/**
 *					カットイン動作モジュール
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _POKEWOOD_CUTIN POKEWOOD_CUTIN;

//-------------------------------------
///	外部公開関数
//=====================================
extern POKEWOOD_CUTIN * POKEWOOD_CUTIN_Init( HEAPID heapID );
extern void POKEWOOD_CUTIN_Exit( POKEWOOD_CUTIN *p_wk );
extern void POKEWOOD_CUTIN_Main( POKEWOOD_CUTIN *p_wk );
extern BOOL POKEWOOD_CUTIN_IsEnd( const POKEWOOD_CUTIN *cp_wk );
extern void POKEWOOD_CUTIN_Start( POKEWOOD_CUTIN *p_wk, const POKEWOOD_SENARIO * cp_senario, u16 event );
extern void POKEWOOD_CUTIN_AttachViewCore( POKEWOOD_CUTIN *p_wk, BTLV_CORE* p_core );

//Initしてなくても使えます
extern BOOL POKEWOOD_CUTIN_IsExits( const POKEWOOD_SENARIO * cp_senario, u16 event );

//本体はbtl_main.cにある
extern POKEWOOD_CUTIN * BTL_MAIN_GetPokewoodCutin( BTL_MAIN_MODULE *wk );

#ifdef PM_DEBUG
extern BOOL DEBUG_POKEWOOD_CUTIN_GetExitsEventNo( const POKEWOOD_SENARIO * cp_senario, u16 cnt, u16 *pw_event_no );
#endif

//=============================================================================
/**
 *					カットインスクリプトデータ読み込みモジュール
*/
//=============================================================================
#define PW_CUTIN_BINARY_SCR_PARAM_NUM (8)

//-------------------------------------
///	ヘッダデータ引数
//=====================================
typedef enum
{
  PW_CUTIN_BINARY_HEADER_PARAM_SENARIO,
  PW_CUTIN_BINARY_HEADER_PARAM_EVENT,
  PW_CUTIN_BINARY_HEADER_PARAM_WND_NUM,
  PW_CUTIN_BINARY_HEADER_PARAM_NCL,
  PW_CUTIN_BINARY_HEADER_PARAM_NCG,
  PW_CUTIN_BINARY_HEADER_PARAM_NCE,
  PW_CUTIN_BINARY_HEADER_PARAM_NAN,
  
  PW_CUTIN_BINARY_HEADER_PARAM_MAX
}PW_CUTIN_BINARY_HEADER_PARAM;

//-------------------------------------
///	スクリプトータ引数
//=====================================
typedef enum
{
  PW_CUTIN_BINARY_SCR_PARAM_CMD,
  PW_CUTIN_BINARY_SCR_PARAM_PARAM_START,
  PW_CUTIN_BINARY_SCR_PARAM_PARAM_END = PW_CUTIN_BINARY_SCR_PARAM_PARAM_START + PW_CUTIN_BINARY_SCR_PARAM_NUM-1,

  PW_CUTIN_BINARY_SCR_PARAM_MAX
}PW_CUTIN_BINARY_SCR_PARAM;

//-------------------------------------
///	ワーク
//=====================================
typedef struct _PW_CUTIN_BINARY PW_CUTIN_BINARY;

//-------------------------------------
///	PUBLIC
//=====================================
extern PW_CUTIN_BINARY * PW_CUTIN_BINARY_Init( ARCDATID datID, HEAPID heapID );
extern void PW_CUTIN_BINARY_Exit( PW_CUTIN_BINARY *p_wk );
extern u16 PW_CUTIN_BINARY_GetHeaderData( const PW_CUTIN_BINARY *cp_wk, PW_CUTIN_BINARY_HEADER_PARAM param );
extern u16 PW_CUTIN_BINARY_GetScriptData( const PW_CUTIN_BINARY *cp_wk, int num, PW_CUTIN_BINARY_SCR_PARAM param );
extern u32 PW_CUTIN_BINARY_GetScriotMax( const PW_CUTIN_BINARY *cp_wk );
