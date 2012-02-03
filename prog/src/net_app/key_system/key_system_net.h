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

#include "multiboot/mb_parent_dataconv_sys.h"
#include "net/nhttp_rap.h"
#include "net/dreamworld_netdata.h"
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
  KEYSYS_NET_TYPE_NONE,
  KEYSYS_NET_TYPE_IRCWIRELESS,
  KEYSYS_NET_TYPE_MULTIBOOT,
  KEYSYS_NET_TYPE_WIFI_NTTP,

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
  KEYSYS_NET_REQ_CANCEL_IRCWIRELESS,      //引数NULL
  KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS,//引数KEYSYS_NET_SEQ_PARAM_IW_SEND
  KEYSYS_NET_REQ_TIMINGSYNC,              //引数NULL

  KEYSYS_NET_REQ_MULTIBOOT_SEND_ROM,      //引数KEYSYS_NET_SEQ_PARAM_MULTIBOOT_SEND_ROM
  KEYSYS_NET_REQ_MULTIBOOT_CONNECT_ROM,      //引数KEYSYS_NET_SEQ_PARAM_MULTIBOOT_CONNECT_ROM

  KEYSYS_NET_REQ_PGL_GETBW,//引数KEYSYS_NET_SEQ_PARAM_PGL_GETBW,
  KEYSYS_NET_REQ_PGL_PLAYSTATUS,//引数NULL 受信データKEYSYS_NET_SEQ_RESULT_PLAYSTATUS

  KEYSYS_NET_REQ_MAX
} KEYSYS_NET_REQ;

//-------------------------------------
///	内部状態
//=====================================
typedef enum
{
  KEYSYS_NET_STATE_WAIT, //何もしていない

  //赤外線ワイヤレス
  KEYSYS_NET_STATE_IRCWIRELESS_CONNECT,
  KEYSYS_NET_STATE_IRCWIRELESS_DISCONNECT,
  KEYSYS_NET_STATE_IRCWIRELESS_WAIT,
  KEYSYS_NET_STATE_IRCWIRELESS_SENDATA_BOTH,
  KEYSYS_NET_STATE_IRCWIRELESS_TIMINGSYNC,
  KEYSYS_NET_STATE_IRCWIRELESS_CANCEL,

  //マルチブート
  KEYSYS_NET_STATE_MULTIBOOT_SEND_ROM,
  KEYSYS_NET_STATE_MULTIBOOT_CONNECT_ROM,

  //PGL
  KEYSYS_NET_STATE_PGL_GETBW,
  KEYSYS_NET_STATE_PGL_PLAYSTATUS,

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


//-------------------------------------
///	KEYSYS_NET_REQ_MULTIBOOT_SEND_ROMの引数
//=====================================
typedef enum
{
  KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_CONTINUE,
  KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_FINISH,
  KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_CANCEL,
}KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM;
typedef struct
{
  const STRBUF *cp_title;
  const STRBUF *cp_info;
  KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM  result;
} KEYSYS_NET_SEQ_PARAM_MULTIBOOT_SEND_ROM;

//-------------------------------------
///	KEYSYS_NET_SEQ_MULTIBOOT_CONNECT_ROMの引数
//=====================================
typedef enum
{
  KEYSYS_NET_RESULT_MULTIBOOT_CONNECT_ROM_CONTINUE,
  KEYSYS_NET_RESULT_MULTIBOOT_CONNECT_ROM_FINISH,
}KEYSYS_NET_RESULT_MULTIBBOT_CONNECT_ROM;
typedef struct
{
  KEYSYS_NET_RESULT_MULTIBBOT_CONNECT_ROM result;
  MB_PARENT_DATACONV_RESULT data_result;
  void *p_recv_data;
} KEYSYS_NET_SEQ_PARAM_MULTIBOOT_CONNECT_ROM;

//-------------------------------------
///	KEYSYS_NET_REQ_PGL_GETBWの引数
//=====================================
typedef enum
{
  KEYSYS_NET_RESULT_PGL_GETBW_CONTINUE,
  KEYSYS_NET_RESULT_PGL_GETBW_NHTTP_ERROR,
  KEYSYS_NET_RESULT_PGL_GETBW_RESPONSE_ERROR,
  KEYSYS_NET_RESULT_PGL_GETBW_PDW_ERROR,
  KEYSYS_NET_RESULT_PGL_GETBW_FINISH,
  KEYSYS_NET_RESULT_PGL_GETBW_SWANDATA,
  KEYSYS_NET_RESULT_PGL_GETBW_NO_REGIST,
  KEYSYS_NET_RESULT_PGL_GETBW_NO_DATA,
}KEYSYS_NET_RESULT_PGL_GETBW;
typedef struct
{
  u32 gsid;
  KEYSYS_NET_RESULT_PGL_GETBW result;
  void *p_buffer_adrs; 
  u32   buffer_size;
} KEYSYS_NET_SEQ_PARAM_PGL_GETBW;

//-------------------------------------
///	KEYSYS_NET_REQ_PGL_PLAYSTATUSの引数
//=====================================
typedef enum
{
  KEYSYS_NET_RESULT_PLAYSTATUS_CONTINUE,
  KEYSYS_NET_RESULT_PLAYSTATUS_NHTTP_ERROR,
  KEYSYS_NET_RESULT_PLAYSTATUS_RESPONSE_ERROR,
  KEYSYS_NET_RESULT_PLAYSTATUS_PDW_ERROR,
  KEYSYS_NET_RESULT_PLAYSTATUS_FIHISH,
}KEYSYS_NET_RESULT_PLAYSTATUS;
typedef struct
{
  DREAM_WORLD_SERVER_PLAY_TYPE play_status;
  KEYSYS_NET_RESULT_PLAYSTATUS  result;
} KEYSYS_NET_SEQ_RESULT_PLAYSTATUS;




//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//初期化。破棄、メイン処理
extern KEYSYS_NET_WORK * KEYSYS_NET_Init( const KEYSYS_NET_INIT_PARAM *cp_param, HEAPID heapID );
extern void KEYSYS_NET_Exit( KEYSYS_NET_WORK *p_wk );
extern void KEYSYS_NET_Main( KEYSYS_NET_WORK *p_wk );
extern void KEYSYS_NET_Start( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_TYPE net_type );

//状態変移.状態取得
extern void KEYSYS_NET_RequestState( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_REQ req, const void *cp_param_adrs );
extern KEYSYS_NET_STATE KEYSYS_NET_GetState( const KEYSYS_NET_WORK *cp_wk );

extern BOOL KEYSYS_NET_GetRecvData( const KEYSYS_NET_WORK *cp_wk, void *p_data, u32 size );
extern void * KEYSYS_NET_GetParamAdrs( KEYSYS_NET_WORK *p_wk );

//エラー
extern KEYSYS_NET_ERROR_TYPE KEYSYS_NET_HandleError( KEYSYS_NET_WORK *p_wk );
extern void KEYSYS_NET_ClearError( KEYSYS_NET_WORK *p_wk );

extern void KEYSYS_NET_SetDWCSvlResult( KEYSYS_NET_WORK *p_wk, DWCSvlResult *p_svl );
