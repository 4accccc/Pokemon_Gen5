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
#include "livebattlematch_irc.h"

//外部公開
#include "wifibattlematch_utilproc.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);
FS_EXTERN_OVERLAY(wifibattlematch_core);
FS_EXTERN_OVERLAY(battle_championship);

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
  union
  { 
    WIFIBATTLEMATCH_NET_WORK  *p_net;
    LIVEBATTLEMATCH_IRC_WORK  *p_irc;
  };
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

//-------------------------------------
///	WIFIのネット
//=====================================
static void UTIL_WIFI_Init( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, HEAPID heapID );
static BOOL UTIL_WIFI_Main( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, int *p_seq );
static void UTIL_WIFI_Exit( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param );

//-------------------------------------
///	IRCのネット
//=====================================
static void UTIL_IRC_Init( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, HEAPID heapID );
static BOOL UTIL_IRC_Main( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, int *p_seq );
static void UTIL_IRC_Exit( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param );
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

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SUB, 0x10000 );

  //プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_LISTAFTER_WORK), HEAPID_WIFIBATTLEMATCH_SUB );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_LISTAFTER_WORK) );	
  p_param->result  = WIFIBATTLEMATCH_LISTAFTER_RESULT_SUCCESS;

  switch( p_param->type )
  { 
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_WIFI:
    UTIL_WIFI_Init( p_wk, p_param, HEAPID_WIFIBATTLEMATCH_SUB );
    break;
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_IRC:
    UTIL_IRC_Init( p_wk, p_param, HEAPID_WIFIBATTLEMATCH_SUB );
    break;
  }

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

  switch( p_param->type )
  { 
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_WIFI:
    UTIL_WIFI_Exit( p_wk, p_param );
    break;
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_IRC:
    UTIL_IRC_Exit( p_wk, p_param );
    break;
  }


 //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SUB );



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
  WIFIBATTLEMATCH_LISTAFTER_WORK  *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param  = p_param_adrs;
  BOOL ret;

  switch( p_param->type )
  { 
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_WIFI:
    ret = UTIL_WIFI_Main( p_wk, p_param, p_seq );
    break;
  case WIFIBATTLEMATCH_LISTAFTER_NETTYPE_IRC:
    ret = UTIL_IRC_Main( p_wk, p_param, p_seq );
    break;
  }
 
  if( ret )
  { 
    return GFL_PROC_RES_FINISH;
  }
  else
  { 
    return GFL_PROC_RES_CONTINUE;
  }
}
//=============================================================================
/**
 *  WIFIのネット
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIのネット初期化
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param  引数
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void UTIL_WIFI_Init( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, HEAPID heapID )
{ 
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(wifibattlematch_core));

  //ネットモジュールの作成
  p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( p_param->p_param->p_game_data, NULL, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIのネット処理
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param    引数
 *	@param	*p_seq      シーケンス
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_WIFI_Main( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, int *p_seq )
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
    return TRUE;
  }

   //エラー処理ここで起きたら復帰が難しいので切断
  if( GFL_NET_IsInit() )
  { 
    if( GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE) != GFL_NET_DWC_ERROR_RESULT_NONE )
    { 
      p_param->result = WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_NEXT_LOGIN;
      return TRUE;
    }
  } 


  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIのネット終了
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param                              引数
 */
//-----------------------------------------------------------------------------
static void UTIL_WIFI_Exit( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param )
{ 
  //ネットモジュール破棄
  WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifibattlematch_core));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
}

//=============================================================================
/**
 *  IRCのネット
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  IRCのネット初期化
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param  引数
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void UTIL_IRC_Init( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, HEAPID heapID )
{ 
  GFL_OVERLAY_Load( FS_OVERLAY_ID(battle_championship));

  p_wk->p_irc = LIVEBATTLEMATCH_IRC_Init( p_param->p_param->p_game_data, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCのネット処理
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param    引数
 *	@param	*p_seq      シーケンス
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_IRC_Main( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param, int *p_seq )
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

  switch( *p_seq )
  { 
  case SEQ_INIT:
    (*p_seq)++;
    break;

  case SEQ_START_TIMING:
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),17, WB_NET_IRC_BATTLE);
    (*p_seq)++;
    break;

  case SEQ_WAIT_TIMING:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),17, WB_NET_IRC_BATTLE) )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_SEND_PARTY:
    if( LIVEBATTLEMATCH_IRC_SendPokeParty( p_wk->p_irc, p_param->p_player_btl_party )  )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_RECV_PARTY:
    if( LIVEBATTLEMATCH_IRC_RecvPokeParty( p_wk->p_irc, p_param->p_enemy_btl_party )  )
    { 
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    return TRUE;
  }

  LIVEBATTLEMATCH_IRC_Main( p_wk->p_irc );

  if( NET_ERR_CHECK_NONE != NetErr_App_CheckError() )
  { 
    NetErr_App_ReqErrorDisp();
    p_param->result = WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_RETURN_LIVE;
    return TRUE;
  } 

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCのネット終了
 *
 *	@param	WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk  ワーク
 *	@param	*p_param                              引数
 */
//-----------------------------------------------------------------------------
static void UTIL_IRC_Exit( WIFIBATTLEMATCH_LISTAFTER_WORK *p_wk, WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param )
{ 
  LIVEBATTLEMATCH_IRC_Exit( p_wk->p_irc );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(battle_championship));
}
