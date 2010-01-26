//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_core.c
 *	@brief	WIFIのバトルマッチコア画面
 *	@author	Toru=Nagihashi
 *	@data		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//WIFIバトルマッチの流れ
#include "wifibattlematch_rnd.h"
#include "wifibattlematch_wifi.h"

//外部公開
#include "wifibattlematch_core.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================


//=============================================================================
/**
 *        定義
*/
//=============================================================================


//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMatchCore_ProcData =
{	
	WIFIBATTLEMATCH_CORE_PROC_Init,
	WIFIBATTLEMATCH_CORE_PROC_Main,
	WIFIBATTLEMATCH_CORE_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;

  switch( p_param->p_param->mode )
  { 
  case WIFIBATTLEMATCH_MODE_RANDOM:  //ランダム対戦
    return WifiBattleMatchRnd_ProcData.init_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_WIFI:    //WIFI大会
    return WifiBattleMatchWifi_ProcData.init_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_LIVE:    //ライブ大会
    return WifiBattleMatchRnd_ProcData.init_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  default:
    GF_ASSERT( 0 );
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;

  switch( p_param->p_param->mode )
  { 
  case WIFIBATTLEMATCH_MODE_RANDOM:  //ランダム対戦
    return WifiBattleMatchRnd_ProcData.end_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_WIFI:    //WIFI大会
    return WifiBattleMatchWifi_ProcData.end_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_LIVE:    //ライブ大会
    return WifiBattleMatchRnd_ProcData.end_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  default:
    GF_ASSERT( 0 );
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;

  switch( p_param->p_param->mode )
  { 
  case WIFIBATTLEMATCH_MODE_RANDOM:  //ランダム対戦
    return WifiBattleMatchRnd_ProcData.main_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_WIFI:    //WIFI大会
    return WifiBattleMatchWifi_ProcData.main_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  case WIFIBATTLEMATCH_MODE_LIVE:    //ライブ大会
    return WifiBattleMatchRnd_ProcData.main_func( p_proc, p_seq, p_param_adrs, p_wk_adrs );

  default:
    GF_ASSERT( 0 );
    return 0;
  }
}
