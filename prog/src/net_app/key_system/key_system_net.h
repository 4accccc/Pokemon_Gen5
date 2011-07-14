//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_net.h
 *	@brief  キーシステムネット
 *	@author	Toru=Nagihashi
 *	@date		2011.06.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	通信タイプ
//=====================================
typedef enum
{
  KEYSYS_NET_TYPE_IRCWIRELESS,

  KEYSYS_NET_TYPE_MAX
} KEYSYS_NET_TYPE;

//-------------------------------------
///	エラータイプ
//=====================================
typedef enum
{
  KEYSYS_NET_ERROR_TYPE_NONE,        //エラーが起こっていません。
  KEYSYS_NET_ERROR_TYPE_RETURN,      //再度通信をすればつながります。
  KEYSYS_NET_ERROR_TYPE_DISCONNECT,  //アプリ終了orWi-Fiから切断してください。
} KEYSYS_NET_ERROR_TYPE;

//-------------------------------------
///	状態リクエスト
//=====================================
typedef enum
{
  KEYSYS_NET_REQ_CONNECT_IRCWIRELESS,     //引数KEYSYS_NET_SEQ_PARAM_IW_CONNECT
  KEYSYS_NET_REQ_DISCONNECT_IRCWIRELESS,  //引数NULL
  KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS,//引数KEYSYS_NET_SEQ_PARAM_IW_SEND
  KEYSYS_NET_REQ_TIMINGSYNC,              //引数NULL

  KEYSYS_NET_REQ_MAX
} KEYSYS_NET_REQ;

//-------------------------------------
///	内部状態
//=====================================
typedef enum
{
  KEYSYS_NET_STATE_WAIT, //何もしていない

  KEYSYS_NET_STATE_IRCWIRELESS_CONNECT,
  KEYSYS_NET_STATE_IRCWIRELESS_DISCONNECT,
  KEYSYS_NET_STATE_IRCWIRELESS_WAIT,
  KEYSYS_NET_STATE_IRCWIRELESS_SENDATA_BOTH,
  KEYSYS_NET_STATE_IRCWIRELESS_TIMINGSYNC,

  KEYSYS_NET_STATE_MAX
} KEYSYS_NET_STATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	キーシステムネット
//=====================================
typedef struct _KEYSYS_NET_WORK KEYSYS_NET_WORK;

//-------------------------------------
///	初期化パラメータ
//=====================================
typedef struct
{
  KEYSYS_NET_TYPE net_type;  //通信タイプ
  GAMEDATA *p_gamedata;               //ゲームデータ
} KEYSYS_NET_INIT_PARAM;

//=============================================================================
/**
 *					RequestStateの引数構造体
*/
//=============================================================================
//-------------------------------------
///	KEYSYS_NET_REQ_CONNECT_IRCWIRELESSの引数
//=====================================
typedef struct
{
  void *p_wk_adrs;
  void (*IrcEnd)( void *p_wk_adrs );
} KEYSYS_NET_SEQ_PARAM_IW_CONNECT;
//-------------------------------------
///	KEYSYS_NET_STATE_IRCWIRELESS_SENDATAの引数
//=====================================
typedef struct
{
  const void *cp_data;
  u32 size;
} KEYSYS_NET_SEQ_PARAM_IW_SEND;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//初期化。破棄、メイン処理
extern KEYSYS_NET_WORK * KEYSYS_NET_Init( const KEYSYS_NET_INIT_PARAM *cp_param, HEAPID heapID );
extern void KEYSYS_NET_Exit( KEYSYS_NET_WORK *p_wk );
extern void KEYSYS_NET_Main( KEYSYS_NET_WORK *p_wk );

//状態変移.状態取得
extern void KEYSYS_NET_RequestState( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_REQ req, const void *cp_param_adrs );
extern KEYSYS_NET_STATE KEYSYS_NET_GetState( const KEYSYS_NET_WORK *cp_wk );

extern BOOL KEYSYS_NET_GetRecvData( const KEYSYS_NET_WORK *cp_wk, void *p_data, u32 size );

//エラー
extern KEYSYS_NET_ERROR_TYPE KEYSYS_NET_HandleError( KEYSYS_NET_WORK *p_wk );
extern void KEYSYS_NET_ClearError( KEYSYS_NET_WORK *p_wk );
