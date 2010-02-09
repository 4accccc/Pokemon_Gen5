//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifilogout_proc.c
 *	@brief  WIFI ログアウト画面
 *	@author	Toru=Nagihashi
 *	@data		2010.02.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/wipe.h"

//アーカイブ
#include "message.naix"
#include "msg/msg_wifi_system.h"

//自分のモジュール
#include "wifilogin_local.h"  //wifloginと同じものを使う

//外部公開
#include "net_app/wifi_logout.h"

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
///	SEQFUNCTION
//=====================================
typedef struct _WIFILOGOUT_WORK WIFILOGOUT_WORK;  //以下の関数のため前方宣言
typedef void (*SEQ_FUNCTION)( WIFILOGOUT_WORK* p_wk );

//-------------------------------------
///	メインワーク
//=====================================
struct _WIFILOGOUT_WORK
{
  WIFILOGIN_YESNO_WORK    *p_select;    //選択肢  現状未使用
  WIFILOGIN_DISP_WORK     *p_display;   //描画系
  WIFILOGIN_MESSAGE_WORK  *p_message;   //メッセージ系
  SEQ_FUNCTION            seq_funcion;  //シーケンス
  WIFILOGOUT_PARAM        *p_param;     //引数
  HEAPID                  heapID;       //ヒープID

  BOOL                    is_end_net;
} ;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFILOGOUT_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFILOGOUT_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFILOGOUT_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	SEQ wifiloginと同じ方式
//=====================================
static void SEQ_Change( WIFILOGOUT_WORK *p_wk, SEQ_FUNCTION seq_funcion );
static void SEQ_ChangeDebug( WIFILOGOUT_WORK *p_wk, SEQ_FUNCTION seq_funcion, int line );
static BOOL SEQ_Main( WIFILOGOUT_WORK *p_wk );

#if PM_DEBUG
#define   SEQ_CHANGE_STATE(p_wk, state)  SEQ_Change( p_wk, state )
#else  //PM_DEBUG
#define   SEQ_CHANGE_STATE(p_wk, state)  SEQ_ChangeDebug( p_wk, state, __LINE__ )
#endif //PM_DEBUG

//-------------------------------------
///	SEQ_FUNC
//    基本的に↓への流れ順です
//=====================================
static void SEQFUNCTION_StartFadeIn( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_WaitFadeIn( WIFILOGOUT_WORK *p_wk );

static void SEQFUNCTION_StartDisConnectMessage( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_WaitDisConnectMessage( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_StartDisConnect( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_WaitDisConnect( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_StartEndMessage( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_WaitEndMessage( WIFILOGOUT_WORK *p_wk );

static void SEQFUNCTION_StartFadeOut( WIFILOGOUT_WORK *p_wk );
static void SEQFUNCTION_WaitFadeOut( WIFILOGOUT_WORK *p_wk );

//-------------------------------------
///	通信コールバック
//=====================================
static void NETCALLBACK_End( void* p_wk_adrs );   ///< 通信終了時に呼ばれるコールバック

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
const GFL_PROC_DATA WiFiLogout_ProcData = 
{
  WIFILOGOUT_PROC_Init,
  WIFILOGOUT_PROC_Main,
  WIFILOGOUT_PROC_Exit,
};

//=============================================================================
/**
 *					プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFILOGOUT	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFILOGOUT_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFILOGOUT_WORK   *p_wk;
  WIFILOGOUT_PARAM  *p_param  = p_param_adrs;

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFILOGOUT, 0x10000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFILOGOUT_WORK), HEAPID_WIFILOGOUT );
	GFL_STD_MemClear( p_wk, sizeof(WIFILOGOUT_WORK) );	
  p_wk->p_param = p_param;
  p_wk->heapID  = HEAPID_WIFILOGOUT;

  //モジュール作成
  p_wk->p_display = WIFILOGIN_DISP_Init( p_wk->heapID, p_param->bg,p_param->display );
  p_wk->p_message = WIFILOGIN_MESSAGE_Init( p_wk->heapID, NARC_message_wifi_system_dat, p_param->display );

  //開始シーケンス
  SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartFadeIn );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WIFILOGOUT	メインプロセス終了
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFILOGOUT_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFILOGOUT_WORK   *p_wk     = p_wk_adrs;

  //モジュール破棄
  WIFILOGIN_MESSAGE_End( p_wk->p_message );
  WIFILOGIN_DISP_End( p_wk->p_display );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFILOGOUT );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WIFILOGOUT	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFILOGOUT_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFILOGOUT_WORK   *p_wk     = p_wk_adrs;

  //モジュール動作
  if( p_wk->p_select )
  {
    WIFILOGIN_MESSAGE_YesNoUpdate( p_wk->p_select );
  }
  WIFILOGIN_DISP_Main( p_wk->p_display );
  WIFILOGIN_MESSAGE_Main( p_wk->p_message );

  //シーケンス実行
  { 
    GFL_PROC_RESULT ret = GFL_PROC_RES_CONTINUE;

    if( SEQ_Main( p_wk ) )
    { 
      ret = GFL_PROC_RES_FINISH;
    }

    return ret;
  }
}

//=============================================================================
/**
 *    SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンス変更
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク  
 *	@param	*p_state              設定するシーケンス
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( WIFILOGOUT_WORK *p_wk, SEQ_FUNCTION seq_funcion )
{ 
  p_wk->seq_funcion = seq_funcion;
}
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンス変更  デバッグ版
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 *	@param	*p_state              設定するシーケンス
 *	@param	line                  デバッグ用行数
 */
//-----------------------------------------------------------------------------
static void SEQ_ChangeDebug( WIFILOGOUT_WORK *p_wk, SEQ_FUNCTION seq_funcion, int line )
{ 
  OS_TPrintf( "neg: %d\n", line );
  SEQ_Change( p_wk, seq_funcion );
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーケンス変更  実行メイン
 *
 *	@param	WIFILOGOUT_WORK *p_wk   ワーク
 *
 *	@return TRUEで終了  FALSEで実行中
 */
//-----------------------------------------------------------------------------
static BOOL SEQ_Main( WIFILOGOUT_WORK *p_wk )
{
  if( p_wk->seq_funcion )
  { 
    p_wk->seq_funcion( p_wk );
    return FALSE;
  }

  return TRUE;
}

//=============================================================================
/**
 *    SEQFUNCTION
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン開始
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_StartFadeIn( WIFILOGOUT_WORK *p_wk )
{ 
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , p_wk->heapID );
  SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_WaitFadeIn );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン待ち
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_WaitFadeIn( WIFILOGOUT_WORK *p_wk )
{ 
  if(WIPE_SYS_EndCheck())
  {
    SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartDisConnectMessage );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ開始
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_StartDisConnectMessage( WIFILOGOUT_WORK *p_wk )
{ 
  WIFILOGIN_MESSAGE_InfoMessageDisp( p_wk->p_message, dwc_message_0011 );
  SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_WaitDisConnectMessage );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ待ち
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_WaitDisConnectMessage( WIFILOGOUT_WORK *p_wk )
{ 
  if(WIFILOGIN_MESSAGE_InfoMessageEndCheck( p_wk->p_message ))
  {
    SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartDisConnect );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断開始
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_StartDisConnect( WIFILOGOUT_WORK *p_wk )
{ 
  if( GFL_NET_IsExit() )
  { 
    //解放されていたら何もしない
    SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartEndMessage );
    OS_TPrintf( "WIFILOGOUT 既にネットは解放されていた\n" );
  }
  else
  { 
    //初期化されていたら解放
    if( GFL_NET_Exit( NETCALLBACK_End ) )
    { 
      SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_WaitDisConnect );
      OS_TPrintf( "WIFILOGOUT ネット解放開始\n" );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断中
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_WaitDisConnect( WIFILOGOUT_WORK *p_wk )
{ 
  if( p_wk->is_end_net )
  {
    SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartEndMessage );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ開始
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_StartEndMessage( WIFILOGOUT_WORK *p_wk )
{ 
  WIFILOGIN_MESSAGE_InfoMessageDisp( p_wk->p_message, dwc_message_0012 );
  SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_WaitEndMessage );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ待ち
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_WaitEndMessage( WIFILOGOUT_WORK *p_wk )
{ 
  if(WIFILOGIN_MESSAGE_InfoMessageEndCheck( p_wk->p_message ))
  {
    SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_StartFadeOut );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト開始
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_StartFadeOut( WIFILOGOUT_WORK *p_wk )
{ 
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , p_wk->heapID );
  SEQ_CHANGE_STATE( p_wk, SEQFUNCTION_WaitFadeOut );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト待ち
 *
 *	@param	WIFILOGOUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNCTION_WaitFadeOut( WIFILOGOUT_WORK *p_wk )
{ 
  if(WIPE_SYS_EndCheck())
  {
    SEQ_CHANGE_STATE( p_wk, NULL ); //終了
  }
}

//=============================================================================
/**
 *    通信コールバック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  通信終了時に呼ばれるコールバック
 *
 *	@param	void* p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void NETCALLBACK_End( void* p_wk_adrs )
{ 
  WIFILOGOUT_WORK *p_wk = p_wk_adrs;
  p_wk->is_end_net  = TRUE;
}
