//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_callselect.h
 *	@brief  呼び出し選択画面
 *	@author	Toru=Nagihashi
 *	@date		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "local_tvt_message.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//リスト登録可能数
#define LOCAL_TVT_CALLSELECT_SELECT_MAX (6)

//終了状態
typedef enum
{
  LOCAL_TVT_CALLSELECT_RESULT_NONE,
  LOCAL_TVT_CALLSELECT_RESULT_DECIDE,
  LOCAL_TVT_CALLSELECT_RESULT_CANCEL,
} LOCAL_TVT_CALLSELECT_RESULT;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	呼び出し選択画面ワーク
//=====================================
typedef struct _LOCAL_TVT_CALLSELECT LOCAL_TVT_CALLSELECT;

//-------------------------------------
///	初期化設定
//=====================================
typedef struct
{
  LOCAL_TVT_MESSAGE *p_message;
  GFL_CLUNIT        *p_clunit;
  const GFL_MSGDATA *p_msgdata;
  int               msgID[LOCAL_TVT_CALLSELECT_SELECT_MAX];
  int               is_select[LOCAL_TVT_CALLSELECT_SELECT_MAX];
  int               max;
}LOCAL_TVT_CALLSELECT_PARAM;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern LOCAL_TVT_CALLSELECT * LOCAL_TVT_CALLSELECT_Init( const LOCAL_TVT_CALLSELECT_PARAM * cp_param, HEAPID heapID );
extern void LOCAL_TVT_CALLSELECT_Exit( LOCAL_TVT_CALLSELECT *p_wk );
extern void LOCAL_TVT_CALLSELECT_Main( LOCAL_TVT_CALLSELECT *p_wk );
extern LOCAL_TVT_CALLSELECT_RESULT LOCAL_TVT_CALLSELECT_GetResult( const LOCAL_TVT_CALLSELECT *cp_wk );
extern s32 LOCAL_TVT_CALLSELECT_GetSelect( const LOCAL_TVT_CALLSELECT *cp_wk );

