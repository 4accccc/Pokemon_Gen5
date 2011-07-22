//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_info.c
 *	@brief  メダル詳細画面アプリ
 *	@author	Masayuki Onoue
 *	@date		2011.05.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "app/medal_info.h"
#include "medal_info_viewer.h"  // アプリ描画モジュール
#include "system/main.h"        // HEAPID

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メダル情報画面メインワーク
//=====================================
typedef struct {

	HEAPID heapID;
  MEDALINFO_INIT_PARAM    *p_param;     // アプリ初期化用パラメータ
  MEDALVIEWER_INITPARAM   init_param;   // 描画モジュール初期化用パラメータ
  MEDALVIEWER_WK          *p_viewer_wk; // ビューワーク
  MEDAL_INFO              medal_info;   // 表示メダル情報
}MEDALINFO_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GFL_PROC_RESULT MEDALINFO_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALINFO_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALINFO_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );

// 外部参照用プロセスデータ定義
GFL_PROC_DATA medalInfo_ProcData =
{
  MEDALINFO_ProcInit,
  MEDALINFO_ProcMain,
  MEDALINFO_ProcTerm
};


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	ヒープサイズ
//=====================================
#define MEDALINFO_HEAP_SIZE (0x21000)

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムプロセス初期化処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALINFO_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  // ワーク定義
  MEDALINFO_WK          *p_wk;
  MEDALINFO_INIT_PARAM  *p_param;

  // プロセス用ヒープID
  HEAPID heapID = HEAPID_MEDALINFO;
	
  // 初期化用パラメータ
  p_param	= p_param_adrs;

	// ヒープ作成
	GFL_HEAP_CreateHeap( p_param->parent_heap_id, heapID , MEDALINFO_HEAP_SIZE );

	//ワーク作成
	p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(MEDALINFO_WK), heapID );
	GFL_STD_MemClear( p_wk, sizeof(MEDALINFO_WK) );

  // メダルデータの設定
  p_wk->medal_info.p_trainer_name = p_param->p_trainer_name;  // トレーナー名
  p_wk->medal_info.medal_id   = p_param->medal_id;            // メダルID
  p_wk->medal_info.medal_cnt  = p_param->medal_cnt;           // メダル数
  p_wk->medal_info.box_rank   = p_param->box_rank;            // ランク
  p_wk->medal_info.get_year   = p_param->get_year;
  p_wk->medal_info.get_month  = p_param->get_month;
  p_wk->medal_info.get_day    = p_param->get_day;

  // パラメータの保存
  p_wk->p_param = p_param;

  // パラメータの設定
  p_wk->init_param.heapID     = heapID;
  p_wk->init_param.list_size  = 1;
  p_wk->init_param.p_info_list= &p_wk->medal_info;
  p_wk->init_param.target     = 0;

  if(  p_param->call_mode == MEDALINFO_CALL_TOP )
  {
    p_wk->init_param.call_mode  = MEDALINFO_CALL_TOP;
  }
  else
  {
    p_wk->init_param.call_mode = MEDALINFO_CALL_SUB;
  }



  // viewerの生成
  p_wk->p_viewer_wk = MEDALVIEWER_Setup( &p_wk->init_param );

  // 初期化呼び出し
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面メイン
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALINFO_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALINFO_WK *p_wk = p_wk_adrs;
  GFL_PROC_RESULT  proc_ret = GFL_PROC_RES_FINISH;

  // メイン処理呼び出し
  proc_ret = MEDALVIEWER_Main( p_wk->p_viewer_wk );

  // メイン呼び出し
  return proc_ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステム終了処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALINFO_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALINFO_WK *p_wk = p_wk_adrs;

  // メイン処理呼び出し
  u32 result = MEDALVIEWER_Delete( p_wk->p_viewer_wk );

  // 終了方法の判定
  if( result == MEDALVIEWER_RESULT_RETURN )
  {
    p_wk->p_param->result = MEDALINFO_RESULT_RETURN;
  }
  else if( result == MEDALVIEWER_RESULT_EXIT )
  {
    p_wk->p_param->result = MEDALINFO_RESULT_EXIT;
  }

  // ワーク開放
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_MEDALINFO );

  // 終了処理呼び出し
  return GFL_PROC_RES_FINISH;
}

