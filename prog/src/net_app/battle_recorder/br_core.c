//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_core.c
 *	@brief	バトルレコーダーコア部分（バトルレコードプレイヤー以外の部分）
 *	@author	Toru=Nagihashi
 *	@data		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//自分のモジュール
#include "br_graphic.h"
#include "br_res.h"

//外部参照
#include "br_core.h"

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
///	メインワーク
//=====================================
typedef struct 
{

	//グラフィック
	BR_GRAPHIC_WORK	*p_graphic;

	//リソース管理
	BR_RES_WORK			*p_res;

	//引数
	BR_CORE_PARAM		*p_param;
} BR_CORE_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BRコアプロセス
//=====================================
static GFL_PROC_RESULT BR_CORE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_CORE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_CORE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルレコーダーコアプロセス
//=====================================
const GFL_PROC_DATA BR_CORE_ProcData =
{	
	BR_CORE_PROC_Init,
	BR_CORE_PROC_Main,
	BR_CORE_PROC_Exit,
};


//=============================================================================
/**
 *					BRコアプロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_CORE_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_RECORDER_CORE, 0x10000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_CORE_WORK), HEAPID_BATTLE_RECORDER_CORE );
	GFL_STD_MemClear( p_wk, sizeof(BR_CORE_WORK) );	
	p_wk->p_param		= p_param_adrs;

	//グラフィック初期化
	p_wk->p_graphic	= BR_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_BATTLE_RECORDER_CORE );
	p_wk->p_res			= BR_RES_Init( HEAPID_BATTLE_RECORDER_CORE );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_CORE_WORK	*p_wk	= p_wk_adrs;

	//モジュール破棄


	//グラフィック破棄
	BR_RES_Exit( p_wk->p_res );
	BR_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_BATTLE_RECORDER_CORE );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_CORE_WORK	*p_wk	= p_wk_adrs;

	//描画
	BR_GRAPHIC_2D_Draw( p_wk->p_graphic );

	return GFL_PROC_RES_CONTINUE;
}
