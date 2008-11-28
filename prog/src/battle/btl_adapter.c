//=============================================================================================
/**
 * @file	btl_adapter.c
 * @brief	ポケモンWB バトルシステム	サーバ-クライアント間アダプター
 * @author	taya
 *
 * @date	2008.09.25	作成
 */
//=============================================================================================
#include <gflib.h>

#include "btl_common.h"
#include "btl_adapter.h"


typedef enum {

	AS_FREE = 0,
	AS_CMD_RECEIVED,
	AS_WAIT_SEND_TO_CLIENT,
	AS_RECEPTION_CLIENT,
	AS_DONE,

}AdapterState;


/**
 *
 */
struct _BTL_ADAPTER {
	GFL_NETHANDLE*	netHandle;

	const void*	returnDataAdrs;
	u32			returnDataSize;
	u8			returnDataPreparedFlag;

	const void*	sendDataAdrs;
	u32			sendDataSize;

	u8	myID;
	u8	myState;
	u8	processingCmd;

};

//-------------------------------------------------------------------------
/**
 *	作成されたアダプタ管理用配列
 * （１マシンに同一IDのアダプタは１つのみという条件の保守を簡単にするため）
 */
//-------------------------------------------------------------------------
static BTL_ADAPTER* AdappterTable[ BTL_CLIENT_MAX ];


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void _StartToReception( BTL_ADAPTER* wk );
static BOOL _WaitToReception(  BTL_ADAPTER* wk );
static BOOL _ReceptionClient( BTL_ADAPTER* wk );



//=============================================================================================
/**
 * システム初期化（バトルシステム起動直後に１回だけ呼び出し）
 */
//=============================================================================================
void BTL_ADAPTER_InitSystem( void )
{
	int i;
	for(i=0; i<BTL_CLIENT_MAX; i++)
	{
		AdappterTable[i] = NULL;
	}
}
//=============================================================================================
/**
 * 	システム終了（バトルシステム終了時に１回だけ呼び出し）
 */
//=============================================================================================
void BTL_ADAPTER_QuitSystem( void )
{
	// とくになにも
}


BTL_ADAPTER*  BTL_ADAPTER_Create( BtlCommMode commMode, GFL_NETHANDLE* netHandle, HEAPID heapID, u16 adapterID )
{
	BTL_ADAPTER* wk = NULL;

	if( AdappterTable[adapterID] == NULL )
	{
		wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_ADAPTER) );
		wk->netHandle = netHandle;
		wk->myID = adapterID;
		wk->myState = AS_FREE;
		wk->processingCmd = BTL_ACMD_NONE;
		wk->returnDataPreparedFlag = FALSE;

		wk->sendDataAdrs = NULL;
		wk->sendDataSize = 0;
	}

	// 非通信モード：同一IDは１つだけ作ってサーバ・クライアント双方から使い回し
	if( commMode == BTL_COMM_NONE )
	{
		if( AdappterTable[adapterID] == NULL )
		{
			AdappterTable[adapterID] = wk;
		}

		return AdappterTable[ adapterID ];
	}
	// 通信モード：同一IDの生成リクエストが来ることはあり得ない
	else
	{
		if( AdappterTable[adapterID] )
		{
			GF_ASSERT(0);
		}
		AdappterTable[adapterID] = wk;
		return AdappterTable[ adapterID ];
	}
}

void BTL_ADAPTER_Delete( BTL_ADAPTER* wk )
{
	GFL_HEAP_FreeMemory( wk );
}



//--------------------------------------------------------------------------------------
/**
 * [SERVER用]	クライアントに対するコマンド送信を開始する
 *
 * @param   wk				アダプタモジュール
 * @param   cmd				コマンドナンバー
 * @param   sendData		送信データアドレス
 * @param	sendDataSize	送信データサイズ
 *
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTER_SetCmd( BTL_ADAPTER* wk, BtlAdapterCmd cmd, const void* sendData, u32 sendDataSize )
{
	GF_ASSERT(wk->myState == AS_FREE);

	wk->myState = AS_CMD_RECEIVED;
	wk->processingCmd = cmd;
	wk->returnDataAdrs = NULL;
	wk->returnDataSize = 0;
	wk->returnDataPreparedFlag = FALSE;

	wk->sendDataAdrs = sendData;
	wk->sendDataSize = sendDataSize;

//	BTL_Printf(" [AD] %d, コマンド %d を送信開始します\n", wk->myID, wk->processingCmd);
}


//--------------------------------------------------------------------------------------
/**
 * [SERVER用]	送信したコマンドに対するクライアントからの返信を待つ
 *
 * @param   wk			アダプタモジュール
 *
 * @retval  BOOL		クライアントからの返信を受け取ったらTRUE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_ADAPTER_WaitCmd( BTL_ADAPTER* wk )
{
	switch( wk->myState ){
	case AS_CMD_RECEIVED:
		_StartToReception( wk );
		wk->myState = AS_WAIT_SEND_TO_CLIENT;
		/* fallthru */
	case AS_WAIT_SEND_TO_CLIENT:
		if( !_WaitToReception(wk) )
		{
			break;
		}
		wk->myState = AS_RECEPTION_CLIENT;
		/* fallthru */
	case AS_RECEPTION_CLIENT:
		if( !_ReceptionClient(wk) )
		{
			break;
		}
		wk->myState = AS_DONE;
		/*fallthru*/;
	case AS_DONE:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用]	クライアントから帰ってきたデータへのポインタを取得
 *
 * @param   wk				アダプタモジュール
 *
 * @retval  const void*		データポインタ
 */
//--------------------------------------------------------------------------------------
const void* BTL_ADAPTER_GetReturnData( BTL_ADAPTER* wk )
{
	GF_ASSERT(wk->returnDataPreparedFlag);
	return wk->returnDataAdrs;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用]	クライアントから帰ってきたデータのサイズを取得
 *
 * @param   wk		アダプタモジュール
 *
 * @retval  u32		データポインタ
 */
//--------------------------------------------------------------------------------------
u32 BTL_ADAPTER_GetReturnDataSize( BTL_ADAPTER* wk )
{
	GF_ASSERT(wk->returnDataPreparedFlag);
	return wk->returnDataSize;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用]	クライアントに送信するコマンドをリセットする
 *
 * @param   wk		アダプタモジュール
 *
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTER_ResetCmd( BTL_ADAPTER *wk )
{
	GF_ASSERT(wk->myState == AS_FREE || wk->myState == AS_DONE);
	wk->myState = AS_FREE;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


static void _StartToReception( BTL_ADAPTER* wk )
{
	// @@@ スタンドアロンなら処理不要／通信時処理は後で
}
static BOOL _WaitToReception(  BTL_ADAPTER* wk )
{
	// @@@ スタンドアロンなら処理不要／通信時処理は後で
	return TRUE;
}
static BOOL _ReceptionClient( BTL_ADAPTER* wk )
{
	return wk->returnDataPreparedFlag;
}



BtlAdapterCmd BTL_ADAPTER_RecvCmd( BTL_ADAPTER* wk )
{
	if( wk->returnDataPreparedFlag == FALSE )
	{
		return wk->processingCmd;
	}
	else
	{
		return BTL_ACMD_NONE;
	}
}
u32 BTL_ADAPTER_GetRecvData( BTL_ADAPTER* wk, const void** ppRecv )
{
	// @@@ 通信時・SERVERと同一以外のマシン上にあるクライアントなら独自バッファに格納してから
	*ppRecv = wk->sendDataAdrs;
	return wk->sendDataSize;
}

void BTL_ADAPTER_ReturnCmd( BTL_ADAPTER* wk, const void* data, u32 size )
{
	// @@@ 通信・非通信で処理ワケ必要
	if( wk->returnDataPreparedFlag == FALSE )
	{
		wk->returnDataAdrs = data;
		wk->returnDataSize = size;
		wk->returnDataPreparedFlag = TRUE;
//		BTL_Printf(" [AD] %d, コマンド %d に対する返信データを受け付け\n", wk->myID, wk->processingCmd );
	}
}
