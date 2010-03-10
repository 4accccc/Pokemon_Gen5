//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\test\nagihashi\debug_utilproc.c
 *	@brief
 *	@author		Toru=Nagihashi
 *	@data		2010.03.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "gamesystem/game_data.h"

//セーブデータ
#include "savedata/regulation.h"

//LIVEマッチのモジュール
#include "net/delivery_irc.h"

//外部公開
#include "debug_utilproc.h"

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
  DELIVERY_IRC_WORK         *p_delivery;
  GAMEDATA                  *p_gamedata;
  REGULATION_CARDDATA       carddata;
} DEBUG_NAGI_IRC_REGULATION_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	赤外線レギュレーション送信プロセス
//=====================================
const GFL_PROC_DATA	DEBUG_NAGI_IRC_REGULATION_ProcData =
{ 
  DEBUG_NAGI_IRC_REGULATION_PROC_Init,
  DEBUG_NAGI_IRC_REGULATION_PROC_Main,
  DEBUG_NAGI_IRC_REGULATION_PROC_Exit
};
//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線レギュレーション配信でばっぐ  初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  DEBUG_NAGI_IRC_REGULATION_WORK  *p_wk;

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x10000 );

  //プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_NAGI_IRC_REGULATION_WORK), HEAPID_NAGI_DEBUG_SUB );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_NAGI_IRC_REGULATION_WORK) );	
  p_wk->p_gamedata  = GAMEDATA_Create( HEAPID_NAGI_DEBUG_SUB );

  { 
    DELIVERY_IRC_INIT init;
    GFL_STD_MemClear( &init, sizeof(DELIVERY_IRC_INIT) );
    init.NetDevID = WB_NET_IRC_BATTLE;
    init.datasize = sizeof(REGULATION_CARDDATA);
    init.pData  = (u8*)&p_wk->carddata;
    init.ConfusionID  = 0;
    init.heapID = HEAPID_NAGI_DEBUG_SUB;
    p_wk->p_delivery  = DELIVERY_IRC_Init(&init);
  }

  Regulation_SetDebugData( &p_wk->carddata );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	赤外線レギュレーション配信でばっぐ  破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_NAGI_IRC_REGULATION_WORK  *p_wk     = p_wk_adrs;

  GAMEDATA_Delete( p_wk->p_gamedata );

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	赤外線レギュレーション配信でばっぐ  メイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_NAGI_IRC_REGULATION_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_START_RECVCARD,
    SEQ_WAIT_RECVCARD,
    SEQ_DELETE,
    SEQ_END,
  };

  DEBUG_NAGI_IRC_REGULATION_WORK  *p_wk     = p_wk_adrs;

  DELIVERY_IRC_Main( p_wk->p_delivery );

  switch( *p_seq )
  { 
  case SEQ_START_RECVCARD:
    if( DELIVERY_IRC_SendStart( p_wk->p_delivery) )
    { 
      *p_seq  = SEQ_WAIT_RECVCARD;
    }
    break;
  case SEQ_WAIT_RECVCARD:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    { 
      *p_seq  = SEQ_DELETE;
    }
    break;
  case SEQ_DELETE:
    DELIVERY_IRC_End( p_wk->p_delivery );
    *p_seq  = SEQ_END;
    break;
  case SEQ_END:

    if( GFL_NET_IsResetEnable() )
    {
      p_wk->p_delivery  = NULL;
      return GFL_PROC_RES_FINISH;
    }
  }



  return GFL_PROC_RES_CONTINUE;
}
