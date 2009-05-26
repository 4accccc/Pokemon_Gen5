//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *  GAME FREAK inc.
 *
 *  @file   minigame_commcomand_func.c
 *  @brief    ミニゲームツール通信コマンド
 *  @author   tomoya takahashi
 *  @data   2007.11.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "minigame_commcomand.h"
#include "minigame_commcomand_func.h"

#include "net_app/net_bugfix.h"

//-----------------------------------------------------------------------------
/**
 *          コーディング規約
 *    ●関数名
 *        １文字目は大文字それ以降は小文字にする
 *    ●変数名
 *        ・変数共通
 *            constには c_ を付ける
 *            staticには s_ を付ける
 *            ポインタには p_ を付ける
 *            全て合わさると csp_ となる
 *        ・グローバル変数
 *            １文字目は大文字
 *        ・関数内変数
 *            小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *          定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *          構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *          プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// 通信初期化関数
void CommCommandMNGMInitialize( MNGM_RESULTWK* p_wk )
{
#if WB_FIX
  CommCommandInitialize( MNGM_CommCommandTclGet(), 
      MNGM_CommCommandTblNumGet(), p_wk );
#else
	GFL_NET_AddCommandTable(GFL_NET_CMD_MINIGAME_TOOL, 
	  MNGM_CommCommandTclGet(), MNGM_CommCommandTblNumGet(), p_wk);
#endif
}
void CommCommandMNGMEntryInitialize( MNGM_ENTRYWK* p_wk )
{
#if WB_FIX
  CommCommandInitialize( MNGM_CommCommandTclGet(), 
      MNGM_CommCommandTblNumGet(), p_wk );
#else
	GFL_NET_AddCommandTable(GFL_NET_CMD_MINIGAME_TOOL, 
	  MNGM_CommCommandTclGet(), MNGM_CommCommandTblNumGet(), p_wk);
#endif
}

// 親ー＞子　Rareゲームタイプ
void CommMNGMRareGame( const int netID, const int size, const void* pBuff, void* pWork, GFL_NETHANDLE *pNetHandle )
{
  MNGM_ENTRY_SetRareGame( pWork, *((u32*)pBuff) );
}


// 子ー>親  もういちど
void CommMNGMRetryYes( const int netID, const int size, const void* pBuff, void* pWork, GFL_NETHANDLE *pNetHandle )
{
  MNGM_RESULT_SetKoRetry( pWork, netID, TRUE );
}

// 子ー>親  もうやらない
void CommMNGMRetryNo( const int netID, const int size, const void* pBuff, void* pWork, GFL_NETHANDLE *pNetHandle )
{
  MNGM_RESULT_SetKoRetry( pWork, netID, FALSE );
}


// 親ー>子  もういちど
void CommMNGMRetryOk( const int netID, const int size, const void* pBuff, void* pWork, GFL_NETHANDLE *pNetHandle )
{
  MNGM_RESULT_SetOyaRetry( pWork, TRUE );
}

// 親ー>子  もうやらない
void CommMNGMRetryNg( const int netID, const int size, const void* pBuff, void* pWork, GFL_NETHANDLE *pNetHandle )
{
  MNGM_RESULT_SetOyaRetry( pWork, FALSE );
}

