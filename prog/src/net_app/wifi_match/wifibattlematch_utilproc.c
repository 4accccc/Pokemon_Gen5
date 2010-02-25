//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_utilproc.c
 *	@brief  wifibattlematch_sysで色々な処理をさせないために、PROC化したもの
 *	@author	Toru=Nagihashi
 *	@data		2010.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//	システム
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "net/dwc_error.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_net.h"

//外部公開
#include "wifibattlematch_utilproc.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					リスト後プロセス
 *					    タイミングをとってPokePartyを送るだけ
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	リスト後プロセスメインワーク
//=====================================
typedef struct 
{
  //ネット
  WIFIBATTLEMATCH_NET_WORK  *p_net;
} WIFIBATTLEMATCH_LISTAFTER_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMatch_ListAfter_ProcData =
{	
	WIFIBATTLEMATCH_LISTAFTER_PROC_Init,
	WIFIBATTLEMATCH_LISTAFTER_PROC_Main,
	WIFIBATTLEMATCH_LISTAFTER_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	リスト後プロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_LISTAFTER_WORK  *p_wk;
  WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param  = p_param_adrs;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SUB, 0x10000 );

  //プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_LISTAFTER_WORK), HEAPID_WIFIBATTLEMATCH_SUB );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_LISTAFTER_WORK) );	
  p_param->result  = WIFIBATTLEMATCH_LISTAFTER_RESULT_SUCCESS;

  //ネットモジュールの作成
  p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( p_param->p_param->p_game_data, NULL, HEAPID_WIFIBATTLEMATCH_SUB );
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	リスト後プロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_LISTAFTER_WORK  *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param  = p_param_adrs;

  //ネットモジュール破棄
  WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SUB );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	リスト後メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_LISTAFTER_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_INIT,
    SEQ_START_TIMING,
    SEQ_WAIT_TIMING,
    SEQ_SEND_PARTY,
    SEQ_RECV_PARTY,
    SEQ_END,
  };

  WIFIBATTLEMATCH_LISTAFTER_WORK  *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param  = p_param_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    (*p_seq)++;
    break;

  case SEQ_START_TIMING:
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),17, WB_NET_WIFIMATCH);
    (*p_seq)++;
    break;

  case SEQ_WAIT_TIMING:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),17, WB_NET_WIFIMATCH) )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_SEND_PARTY:
    if( WIFIBATTLEMATCH_NET_SendPokeParty( p_wk->p_net, p_param->p_player_btl_party )  )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_RECV_PARTY:
    if( WIFIBATTLEMATCH_NET_RecvPokeParty( p_wk->p_net, p_param->p_enemy_btl_party )  )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  }

  //エラー処理ここで起きたら復帰が難しいので切断
  if( GFL_NET_IsInit() )
  { 
    if( GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE) != GFL_NET_DWC_ERROR_RESULT_NONE )
    { 
      p_param->result = WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_NEXT_LOGIN;
      return GFL_PROC_RES_FINISH;
    }
  }

  return GFL_PROC_RES_CONTINUE;
}
