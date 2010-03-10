//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_battle.c
 *	@brief
 *	@author		Toru=Nagihashi
 *	@data		2010.02.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//	システム
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "gamesystem/btl_setup.h"
#include "poke_tool/pokeparty.h"
#include "net/dwc_error.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_net.h"

//外部公開
#include "wifibattlematch_battle.h"


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトル接続プロセスメインワーク
//=====================================
typedef struct 
{
  GFL_PROCSYS *p_procsys;
} WIFIBATTLEMATCH_BATTLELINK_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMatch_BattleLink_ProcData =
{	
	WIFIBATTLEMATCH_BATTLELINK_PROC_Init,
	WIFIBATTLEMATCH_BATTLELINK_PROC_Main,
	WIFIBATTLEMATCH_BATTLELINK_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	バトル接続プロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_p_wk						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_BATTLELINK_WORK  *p_wk;
  WIFIBATTLEMATCH_BATTLELINK_PARAM *p_param  = p_param_adrs;

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SUB, 0x100 );

  //プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_BATTLELINK_WORK), HEAPID_WIFIBATTLEMATCH_SUB );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_BATTLELINK_WORK) );	
  p_param->result = WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS;

  //モジュール初期化
  p_wk->p_procsys = GFL_PROC_LOCAL_boot( HEAPID_WIFIBATTLEMATCH_SUB );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	バトル接続プロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_p_wk						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_BATTLELINK_WORK  *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_BATTLELINK_PARAM *p_param  = p_param_adrs;

  //モジュール破棄
  GFL_PROC_LOCAL_Exit( p_wk->p_procsys );

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SUB );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	バトル接続メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_p_wk						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_BATTLELINK_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_START_DEMO = 0,  ///< バトル前デモ呼び出し
    SEQ_WAIT_START_DEMO,
    SEQ_BATTLE_TIMING_INIT,
    SEQ_BATTLE_TIMING_WAIT,
    SEQ_BATTLE_INIT,
    SEQ_BATTLE_WAIT,
    SEQ_BATTLE_END,
    SEQ_CALL_END_DEMO,        ///< バトル後デモ呼び出し
    SEQ_WAIT_END_DEMO,
    SEQ_CALL_BTL_REC_SEL,     ///< 通信対戦後の録画選択画面
    SEQ_BGM_POP,
    SEQ_END
  };

  WIFIBATTLEMATCH_BATTLELINK_WORK  *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_BATTLELINK_PARAM *p_param  = p_param_adrs;
  GFL_PROC_MAIN_STATUS status;

  status = GFL_PROC_LOCAL_Main( p_wk->p_procsys );

  switch (*p_seq) {
  case SEQ_CALL_START_DEMO:
    // 通信バトル前デモ呼び出し
    {
      p_param->p_demo_param->type = COMM_BTL_DEMO_TYPE_NORMAL_START;
      GFL_PROC_LOCAL_CallProc( p_wk->p_procsys, FS_OVERLAY_ID( comm_btl_demo ), &CommBtlDemoProcData, p_param->p_demo_param);
    }
    (*p_seq) = SEQ_WAIT_START_DEMO;
    break;
  
  case SEQ_WAIT_START_DEMO:
    if ( status != GFL_PROC_MAIN_VALID ){
      (*p_seq) = SEQ_BATTLE_TIMING_INIT;
    }
    break;
  
  case SEQ_BATTLE_TIMING_INIT:
    {
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), 200, WB_NET_WIFIMATCH );
      OS_TPrintf("戦闘用通信コマンドテーブルをAddしたので同期取り\n");
      (*p_seq) = SEQ_BATTLE_TIMING_WAIT;
    }
    break;
  case SEQ_BATTLE_TIMING_WAIT:
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(), 200, WB_NET_WIFIMATCH ) ){
      OS_TPrintf("戦闘用通信コマンドテーブルをAdd後の同期取り完了\n");
      (*p_seq) = SEQ_BATTLE_INIT;
    }
    break;
  case SEQ_BATTLE_INIT:
    GFL_PROC_LOCAL_CallProc(p_wk->p_procsys, NO_OVERLAY_ID, &BtlProcData, p_param->p_btl_setup_param);
    (*p_seq) = SEQ_BATTLE_WAIT;
    break;
  case SEQ_BATTLE_WAIT:
    if ( status != GFL_PROC_MAIN_VALID ){
      (*p_seq) = SEQ_BATTLE_END;
    }
    break;
  case SEQ_BATTLE_END:
    OS_TPrintf("バトル完了\n");
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

    (*p_seq) = SEQ_CALL_END_DEMO;
    break;
  case SEQ_CALL_END_DEMO:
    // 通信バトル後デモ呼び出し
    {
      // マルチバトル判定
      if( p_param->p_btl_setup_param->multiMode == 0 )
      {
        p_param->p_demo_param->type = COMM_BTL_DEMO_TYPE_NORMAL_END;
      }
      else
      {
        p_param->p_demo_param->type = COMM_BTL_DEMO_TYPE_MULTI_END;
      }
      HOSAKA_Printf("comm battle demo type=%d\n",p_param->p_demo_param->type);

      // 勝敗設定
      switch( p_param->p_btl_setup_param->result )
      {
      case BTL_RESULT_RUN_ENEMY:
      case BTL_RESULT_WIN :
        p_param->p_demo_param->result = COMM_BTL_DEMO_RESULT_WIN;
        break;
      case BTL_RESULT_LOSE :
      case BTL_RESULT_RUN :
        p_param->p_demo_param->result = COMM_BTL_DEMO_RESULT_LOSE;
        break;
      case BTL_RESULT_DRAW :
        p_param->p_demo_param->result = COMM_BTL_DEMO_RESULT_DRAW;
        break;
      default : 
        p_param->p_demo_param->result = COMM_BTL_DEMO_RESULT_DRAW;
      }

      GFL_PROC_LOCAL_CallProc(p_wk->p_procsys, FS_OVERLAY_ID( comm_btl_demo ), &CommBtlDemoProcData, p_param->p_demo_param);
      (*p_seq) = SEQ_WAIT_END_DEMO;
    }
    break;
  case SEQ_WAIT_END_DEMO:
    if ( status != GFL_PROC_MAIN_VALID ){
      (*p_seq) = SEQ_END;
    }
    break;

  case SEQ_END:
    return GFL_PROC_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  //エラー処理ここで起きたらCOREに戻る
  if( GFL_NET_IsInit() )
  {
    if( GFL_NET_GetNETInitStruct()->bNetType == GFL_NET_TYPE_IRC )
    { 

    }
    else 
    { 
      if( GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE) != GFL_NET_DWC_ERROR_RESULT_NONE )
      { 
        p_param->result = WIFIBATTLEMATCH_BATTLELINK_RESULT_DISCONNECT;
        return GFL_PROC_RES_FINISH;
      }
    }
  }

  { 
    //不正チェック
    if( 0 )
    { 
      return WIFIBATTLEMATCH_BATTLELINK_RESULT_EVIL;
    }
  }

  return GFL_PROC_RES_CONTINUE;
}
