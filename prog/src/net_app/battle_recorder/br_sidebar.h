//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_sidebar.h
 *	@brief  サイドバー制御
 *	@author	Toru=Nagihashi
 *	@date		2009.12.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>
//バトルレコーダー内モジュール
#include "br_res.h"
#include "br_fade.h"
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
//-------------------------------------
///	サイドバー制御
//=====================================
typedef struct _BR_SIDEBAR_WORK BR_SIDEBAR_WORK;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//システム
extern BR_SIDEBAR_WORK * BR_SIDEBAR_Init( GFL_CLUNIT *p_clunit, BR_FADE_WORK *p_fade, BR_RES_WORK *p_res, HEAPID heapID );
extern void BR_SIDEBAR_Exit( BR_SIDEBAR_WORK *p_wk, BR_RES_WORK *p_res );
extern void BR_SIDEBAR_Main( BR_SIDEBAR_WORK *p_wk );

//操作
extern void BR_SIDEBAR_StartBoot( BR_SIDEBAR_WORK *p_wk );
extern void BR_SIDEBAR_StartShake( BR_SIDEBAR_WORK *p_wk );
extern void BR_SIDEBAR_StartClose( BR_SIDEBAR_WORK *p_wk );

extern void BR_SIDEBAR_SetShakePos(BR_SIDEBAR_WORK *p_wk  );

extern BOOL BR_SIDEBAR_IsMoveEnd( const BR_SIDEBAR_WORK *cp_wk );

//br_musicallook_proc.cに上画面破棄・読み込み用
extern void BR_SIDEBAR_UnLoadMain( BR_SIDEBAR_WORK *p_wk, BR_RES_WORK *p_res );
extern void BR_SIDEBAR_LoadMain( BR_SIDEBAR_WORK *p_wk, GFL_CLUNIT *p_clunit, BR_FADE_WORK *p_fade, BR_RES_WORK *p_res, HEAPID heapID );
