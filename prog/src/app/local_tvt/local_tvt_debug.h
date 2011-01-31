//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_debug.h
 *	@brief  ローカルライブキャスターデバッグ機能
 *	@author	Toru=Nagihashi
 *	@date		2011.01.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG
#include "local_tvt_chara.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグフラグ
//=====================================
typedef enum
{
  LOCAL_TVT_DEBUG_FLAG_CHECK_DORYOKU_ON,
  LOCAL_TVT_DEBUG_FLAG_MAX
}LOCAL_TVT_DEBUG_FLAG;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	初期化パラメータ
//=====================================
typedef struct 
{
  GFL_FONT            *p_font;
  LOCAL_TVT_CHARA_SYS *p_chara_sys;
  GAMEDATA            *p_gamedata;
} LOCAL_TVT_DEBUG_PARAM;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern void LOCAL_TVT_DEBUG_Init( const LOCAL_TVT_DEBUG_PARAM *cp_param, HEAPID heapID );
extern void LOCAL_TVT_DEBUG_Exit( void );
extern void LOCAL_TVT_DEBUG_Main( void );

extern BOOL LOCAL_TVT_DEBUG_IsFlagOn( LOCAL_TVT_DEBUG_FLAG flag );
#else
//製品版
#define LOCAL_TVT_DEBUG_IsFlagOn( flag )  (FALSE)
#endif
