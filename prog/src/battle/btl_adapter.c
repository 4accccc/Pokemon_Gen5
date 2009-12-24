//=============================================================================================
/**
 * @file  btl_adapter.c
 * @brief ポケモンWB バトルシステム サーバ-クライアント間アダプター
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================
#include <gflib.h>

#include "btl_common.h"

#include "btl_net.h"
#include "btl_server_cmd.h"
#include "btl_action.h"
#include "btl_adapter.h"

enum {
  PRINT_FLG = FALSE,
};


enum {
  DATA_BUFFER_SIZE = BTL_SERVER_CMD_QUE_SIZE + 4,
};

typedef enum {

  AS_FREE = 0,
  AS_CMD_RECEIVED,
  AS_WAIT_SEND_TO_CLIENT,
  AS_WAIT_RECV_FROM_CLIENT,
  AS_DONE,

}AdapterState;


typedef struct {

  struct {
    u16   cmd;
    u16   size;
  } header;

  u8    data[ DATA_BUFFER_SIZE ];

}SEND_DATA_BUFFER;

/**
 *
 */
struct _BTL_ADAPTER {
  GFL_NETHANDLE*  netHandle;

  const void* returnDataAdrs;
  u32     returnDataSize;
  u8      returnDataPreparedFlag;

  u8  myID;
  u8  myState;
  u8  processingCmd;

  SEND_DATA_BUFFER   sendDataBuffer;
};



//-------------------------------------------------------------------------
/**
 *  作成されたアダプタ管理用配列
 * （１マシンに同一IDのアダプタは１つのみという条件の保守を簡単にするため）
 */
//-------------------------------------------------------------------------
static BTL_ADAPTER* AdappterTable[ BTL_CLIENT_MAX ];
static u8 CommMode;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BOOL _StartToReception( BTL_ADAPTER* wk );
static BOOL _ReceptionClient( BTL_ADAPTER* wk );
static inline void sendBuf_store( SEND_DATA_BUFFER* buf, BtlAdapterCmd cmd, const void* data, u32 dataSize );
static inline u32 sendBuf_calcSize( const SEND_DATA_BUFFER* buf );
static inline BtlAdapterCmd sendBuf_getCmd( const SEND_DATA_BUFFER* buf );
static inline u32 sendBuf_getData( const SEND_DATA_BUFFER* buf, const void** pptr );



//=============================================================================================
/**
 * システム初期化（バトルシステム起動直後に１回だけ呼び出し）
 */
//=============================================================================================
void BTL_ADAPTERSYS_Init( BtlCommMode mode )
{
  int i;

  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    AdappterTable[i] = NULL;
  }

  CommMode = mode;
}
//=============================================================================================
/**
 *  システム終了（バトルシステム終了時に１回だけ呼び出し）
 */
//=============================================================================================
void BTL_ADAPTERSYS_Quit( void )
{
  // とくになにも
}


BTL_ADAPTER*  BTL_ADAPTER_Create( GFL_NETHANDLE* netHandle, HEAPID heapID, u16 adapterID )
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
  }

  // 非通信モード：同一IDは１つだけ作ってサーバ・クライアント双方から使い回し
  if( CommMode == BTL_COMM_NONE )
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
    GF_ASSERT( AdappterTable[adapterID] == NULL );
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
 * [SERVER用] BTL_ADAPTER_SetCmd の前に呼び出し
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTERSYS_BeginSetCmd( void )
{
  if( CommMode != BTL_COMM_NONE )
  {
    BTL_NET_ClearRecvData();
  }
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用] BTL_ADAPTER_SetCmd の後に呼び出し
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTERSYS_EndSetCmd( void )
{
  if( CommMode != BTL_COMM_NONE )
  {

  }
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用] クライアントに対するコマンド送信を開始する
 *
 * @param   wk        アダプタモジュール
 * @param   cmd       コマンドナンバー
 * @param   sendData    送信データアドレス
 * @param sendDataSize  送信データサイズ
 *
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTER_SetCmd( BTL_ADAPTER* wk, BtlAdapterCmd cmd, const void* sendData, u32 sendDataSize )
{
  GF_ASSERT(wk->myState == AS_FREE);

  wk->myState = AS_CMD_RECEIVED;
  wk->processingCmd = cmd;

  sendBuf_store( &wk->sendDataBuffer, cmd, sendData, sendDataSize );

  wk->returnDataAdrs = NULL;
  wk->returnDataSize = 0;
  wk->returnDataPreparedFlag = FALSE;

  BU_Printf(PRINT_FLG, " Adapter-%d, コマンド %d を送信開始します\n", wk->myID, wk->processingCmd );
}


//--------------------------------------------------------------------------------------
/**
 * [SERVER用] 送信したコマンドに対するクライアントからの返信を待つ
 *
 * @param   wk      アダプタモジュール
 *
 * @retval  BOOL    クライアントからの返信を受け取ったらTRUE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_ADAPTER_WaitCmd( BTL_ADAPTER* wk )
{
  switch( wk->myState ){
  case AS_CMD_RECEIVED:
    if( !_StartToReception(wk) ){
      break;
    }
    wk->myState = AS_WAIT_RECV_FROM_CLIENT;
    /* fallthru */
  case AS_WAIT_RECV_FROM_CLIENT:
    if( !_ReceptionClient(wk) )
    {
      break;
    }
    BU_Printf(PRINT_FLG, " [Adapter] Received from All Clients\n");
    wk->myState = AS_DONE;
    /*fallthru*/;
  case AS_DONE:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用] クライアントから帰ってきたデータへのポインタを取得
 *
 * @param   wk        アダプタモジュール
 * @param   size      [out] データサイズ
 *
 * @retval  const void*   データポインタ
 */
//--------------------------------------------------------------------------------------
const void* BTL_ADAPTER_GetReturnData( BTL_ADAPTER* wk, u32* size )
{
  GF_ASSERT(wk->returnDataPreparedFlag);

  if( size != NULL ){
    *size = wk->returnDataSize;
  }
  return wk->returnDataAdrs;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用] クライアントから帰ってきたデータのサイズを取得
 *
 * @param   wk    アダプタモジュール
 *
 * @retval  u32   データポインタ
 */
//--------------------------------------------------------------------------------------
u32 BTL_ADAPTER_GetReturnDataSize( BTL_ADAPTER* wk )
{
  GF_ASSERT(wk->returnDataPreparedFlag);
  return wk->returnDataSize;
}

//--------------------------------------------------------------------------------------
/**
 * [SERVER用] クライアントに送信するコマンドをリセットする
 *
 * @param   wk    アダプタモジュール
 *
 */
//--------------------------------------------------------------------------------------
void BTL_ADAPTER_ResetCmd( BTL_ADAPTER *wk )
{
  GF_ASSERT(wk->myState == AS_FREE || wk->myState == AS_DONE);
  wk->myState = AS_FREE;

  if( CommMode != BTL_COMM_NONE )
  {
    BTL_NET_ClearRecvData();
  }
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


static BOOL _StartToReception( BTL_ADAPTER* wk )
{
  // スタンドアロンなら処理不要
  if( CommMode != BTL_COMM_NONE )
  {
    SEND_DATA_BUFFER* sendBuf = &wk->sendDataBuffer;
    return BTL_NET_SendToClient( wk->myID, sendBuf, sendBuf_calcSize(sendBuf) );
  }
  else{
    return TRUE;
  }
}

static BOOL _ReceptionClient( BTL_ADAPTER* wk )
{
  if( CommMode != BTL_COMM_NONE )
  {
    if( BTL_NET_CheckReturnFromClient() )
    {
      wk->returnDataSize = BTL_NET_GetRecvClientData( wk->myID, &wk->returnDataAdrs );
      wk->returnDataPreparedFlag = TRUE;
      BU_Printf(PRINT_FLG, " [Adapter] Received from AdapterID=%d\n", wk->myID );
      return TRUE;
    }
    return FALSE;
  }
  else
  {
    return wk->returnDataPreparedFlag;
  }
}


//--------------------------------------------------------------------------------------
/**
 * [CLIENT用] アダプタコマンド受信チェック
 *
 * @param   wk
 *
 * @retval  BtlAdapterCmd
 */
//--------------------------------------------------------------------------------------
BtlAdapterCmd BTL_ADAPTER_RecvCmd( BTL_ADAPTER* wk )
{
  // 通信時
  if( CommMode != BTL_COMM_NONE )
  {
    if( BTL_NET_IsServerCmdReceived() )
    {
      SEND_DATA_BUFFER* sendBuf;
      BtlAdapterCmd cmd;

      BTL_NET_GetReceivedCmdData( (const void**)&sendBuf );
      cmd = sendBuf_getCmd( sendBuf );

      BTL_Printf("サーバコマンド(%d)受信しました\n", cmd);

      return cmd;
    }
    else
    {
      return BTL_ACMD_NONE;
    }
  }
  // 非通信時
  else
  {
    if( wk->returnDataPreparedFlag == FALSE )
    {
      return sendBuf_getCmd( &wk->sendDataBuffer );
    }
    else
    {
      return BTL_ACMD_NONE;
    }
  }
}

//--------------------------------------------------------------------------------------
/**
 * [CLIENT用] アダプタコマンド実データ部受信
 *
 * @param   wk
 * @param   ppRecv
 *
 * @retval  u32
 */
//--------------------------------------------------------------------------------------
u32 BTL_ADAPTER_GetRecvData( BTL_ADAPTER* wk, const void** ppRecv )
{
  // 通信時
  if( CommMode != BTL_COMM_NONE )
  {
    SEND_DATA_BUFFER* sendBuf;

    BTL_NET_GetReceivedCmdData( (const void**)&sendBuf );
    return sendBuf_getData( sendBuf, ppRecv );
  }
  // 非通信時
  else
  {
    return sendBuf_getData( &wk->sendDataBuffer, ppRecv );
  }
}

//=============================================================================================
/**
 * クライアント→サーバへコマンド返信
 *
 * @param   wk
 * @param   data    返信データアドレス
 * @param   size    返信データサイズ
 *
 * @retval  BOOL    成功時TRUE（成功するまで毎フレーム呼ぶこと）
 */
//=============================================================================================
BOOL BTL_ADAPTER_ReturnCmd( BTL_ADAPTER* wk, const void* data, u32 size )
{
  // 通信時
  if( CommMode != BTL_COMM_NONE )
  {
    return BTL_NET_ReturnToServer( data, size );
  }
  // 非通信時
  else
  {
    if( wk->returnDataPreparedFlag == FALSE )
    {
      wk->returnDataAdrs = data;
      wk->returnDataSize = size;
      wk->returnDataPreparedFlag = TRUE;
    }
    return TRUE;
  }
}




static inline void sendBuf_store( SEND_DATA_BUFFER* buf, BtlAdapterCmd cmd, const void* data, u32 dataSize )
{
  GF_ASSERT(dataSize < sizeof(buf->data));

  buf->header.cmd = cmd;
  buf->header.size = dataSize;
  GFL_STD_MemCopy( data, buf->data, dataSize );
}

static inline u32 sendBuf_calcSize( const SEND_DATA_BUFFER* buf )
{
  return sizeof(buf->header) + buf->header.size;
}

static inline BtlAdapterCmd sendBuf_getCmd( const SEND_DATA_BUFFER* buf )
{
  return buf->header.cmd;
}

static inline u32 sendBuf_getData( const SEND_DATA_BUFFER* buf, const void** pptr )
{
  *pptr = buf->data;
  return buf->header.size;
}

