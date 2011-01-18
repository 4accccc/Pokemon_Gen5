//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_callmode.h
 *	@brief  ローカルライブキャスター呼び出しモードでの動作
 *	@author	Toru=Nagihashi
 *	@date		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//ワーク
typedef struct _LOCAL_TVT_CALL LOCAL_TVT_CALL;

//初期化構造体
typedef struct
{
  GAMEDATA            *p_gamedata;
  LOCAL_TVT_CHARA_SYS *p_chara_sys;
  LOCAL_TVT_MESSAGE   *p_message;
  GFL_CLUNIT          *p_clunit;
}LOCAL_TVT_CALL_PARAM;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern LOCAL_TVT_CALL * LOCAL_TVT_CALL_Init( const LOCAL_TVT_CALL_PARAM *cp_param, HEAPID heapID );
extern void LOCAL_TVT_CALL_Exit( LOCAL_TVT_CALL *p_wk );
extern BOOL LOCAL_TVT_CALL_Main( LOCAL_TVT_CALL *p_wk );
