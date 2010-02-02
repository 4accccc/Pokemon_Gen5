//=============================================================================================
/**
 * @file  btl_adapter.h
 * @brief ポケモンWB バトルシステム サーバ-クライアント間アダプター
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================
#ifndef __BTL_ADAPTER_H__
#define __BTL_ADAPTER_H__

#include "battle/battle.h"
#include "btl_common.h"


//--------------------------------------------------------------
/**
 *  ハンドラ用型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_ADAPTER   BTL_ADAPTER;


//--------------------------------------------------------------
/**
 *  アダプタータイプ
 */
//--------------------------------------------------------------
typedef enum {
  BTL_ADAPTER_LOCAL,      ///< 通信せず、サーバと同一マシン上にクライアントがある
  BTL_ADAPTER_COMM_PARENT,  ///< DS 無線通信を利用（親機側）
  BTL_ADAPTER_COMM_CHILD,   ///< DS 無線通信をを利用（子機側）
  BTL_ADAPTER_WIFI_PARENT,  ///< DS Wi-Fi 機能を利用（親機側）
  BTL_ADAPTER_WIFI_CHILD,   ///< DS Wi-Fi 機能を利用（子機側）
}BtlAdapterType;

//--------------------------------------------------------------
/**
 *  アダプターコマンド
 */
//--------------------------------------------------------------
typedef enum {
  BTL_ACMD_NONE = 0,
  BTL_ACMD_WAIT_SETUP,
  BTL_ACMD_SELECT_ROTATION,
  BTL_ACMD_SELECT_ACTION,
  BTL_ACMD_SELECT_POKEMON_FOR_COVER,
  BTL_ACMD_SELECT_POKEMON_FOR_CHANGE,
  BTL_ACMD_SELECT_CHANGE_OR_ESCAPE,
  BTL_ACMD_SERVER_CMD,
  BTL_ACMD_QUIT_BTL,
  BTL_ACMD_QUIT_ESCAPE,
  BTL_ACMD_RECORD_DATA,
  BTL_ACMD_EXIT_WIN_TRAINER,
  BTL_ACMD_NOTIFY_TIMEUP,

  BTL_ACMD_MAX,
}BtlAdapterCmd;


//--------------------------------------------------------------------------------------
/**
 * システム初期化，終了（バトルシステム起動直後，終了直前に１回だけ呼び出し）
 */
//--------------------------------------------------------------------------------------
extern void BTL_ADAPTERSYS_Init( BtlCommMode mode );
extern void BTL_ADAPTERSYS_Quit( void );

extern BTL_ADAPTER*  BTL_ADAPTER_Create( GFL_NETHANDLE* netHandle, HEAPID heapID, u16 adapterID );
extern void BTL_ADAPTER_Delete( BTL_ADAPTER* wk );


// for server
extern void BTL_ADAPTERSYS_BeginSetCmd( void );
extern void BTL_ADAPTERSYS_EndSetCmd( void );
extern void BTL_ADAPTER_SetCmd( BTL_ADAPTER* wk, BtlAdapterCmd cmd, const void* sendData, u32 sendDataSize );
extern BOOL BTL_ADAPTER_WaitCmd( BTL_ADAPTER* wk );
extern void BTL_ADAPTER_ResetCmd( BTL_ADAPTER *wk );
extern const void* BTL_ADAPTER_GetReturnData( BTL_ADAPTER* wk, u32* size );
extern u32 BTL_ADAPTER_GetReturnDataSize( BTL_ADAPTER* wk );

// for clients
extern BtlAdapterCmd BTL_ADAPTER_RecvCmd( BTL_ADAPTER* wk );
extern u32 BTL_ADAPTER_GetRecvData( BTL_ADAPTER* wk, const void** ppRecv );
extern BOOL BTL_ADAPTER_ReturnCmd( BTL_ADAPTER* wk, const void* data, u32 size );




#endif

