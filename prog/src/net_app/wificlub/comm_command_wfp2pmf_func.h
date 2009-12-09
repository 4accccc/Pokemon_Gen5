//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		comm_command_wfp2pmf_func.h
 *	@brief		２～４人専用待合室	通信コマンド
 *	@author		tomoya takahashi
 *	@data		2007.05.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once


#include "net_app/wificlub/wifi_p2pmatch.h"
#include "wifi_p2pmatch_local.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// 通信初期化関数
//extern void CommCommandWFP2PMFInitialize( WFP2P_WK* p_wk );
extern void CommCommandWFP2PMF_MatchStartInitialize( WIFIP2PMATCH_WORK *wk );
//extern void CommCommandWFP2PMF_MatchStartInitialize( void );	// wifi_p2pmatch.c内でコマンドを設定するときの関数

// 共通サイズ取得関数
extern int CommWFP2PMFGetZeroSize( void );
extern int CommWFP2PMFGetWFP2PMF_COMM_RESULTSize( void );
extern int CommWFP2PMFGetWFP2PMF_COMM_VCHATSize( void );

// 親からのリクエスト
extern void CommWFP2PMFGameResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
extern void CommWFP2PMFGameStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
extern void CommWFP2PMFGameVchat(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);



