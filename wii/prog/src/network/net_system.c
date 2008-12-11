//=============================================================================
/**
 * @file	net_system.c
 * @brief	通信システム
 * @author	GAME FREAK Inc.
 * @date	2006.12.05
 */
//=============================================================================


#include "net_def.h"
#include "net_local.h"
#include "net_system.h"
#include "net_command.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"

#define _SENDRECV_LIMIT  (3)  // 送信と受信の数がずれた場合送信を抑制する

//==============================================================================
// extern宣言
//==============================================================================




//==============================================================================
// 定義
//==============================================================================

//データシェアリング通信のヘッダー  MP時の子機のヘッダー
const int _DS_CRCNO1 = 0;
const int _DS_CRCNO2 = 1;
const int _DS_INCNO = 2;
const int _DS_HEADERNO = 3;
const int _DS_HEADER_MAX = 4;
const int _DS_CRC_MAX = 2;

//MP時の親機のヘッダー


const int _MP_CONNECTBIT = 1;
const int _MP_SIZEO1 = 2;
const int _MP_SIZEO2 = 3;
const int _MP_CRCNO1 = 4;
const int _MP_CRCNO2 = 5;
const int _MP_INCNO = 6;
const int _MP_DATA_HEADER_POS = 7;



/// クライアント送信用キューの本数
#define _SENDQUEUE_NUM_MAX  (100)
/// サーバー送信用キューの本数
#define _SENDQUEUE_SERVER_NUM_MAX	  (180)

// 子機RING送信係数
#define _SEND_RINGBUFF_FACTOR_CHILD  (15)
// 親機RING送信係数
#define _SEND_RINGBUFF_FACTOR_PARENT  (2)

//#define _MIDDLE_BUFF_NUM  (4)  ///DS用ミドルバッファにどの程度ためられるのか

// 初期化されていないイテレーターの数
#define _NULL_ITERATE (-1)
// ありえないID
#define _INVALID_ID  (GFL_NET_NETID_INVALID)
// 送信データがまだもらえていなかった
//#define _NODATA_SEND  (0x0e)

//VBlank処理のタスクのプライオリティー
#define _PRIORITY_VBLANKFUNC (0)

/** @brief 通信先頭バイトのBITの意味 */
#define _SEND_NONE  (0x01)  ///< 一回で送れる場合
#define _SEND_NEXT  (0x02)  ///< 一回で送れない場合
#define _MP_DATA_HEADER (0x08)  ///< MPデータの場合BITON  DS時にはOFF
#define _INVALID_HEADER  (0xff)  ///< 初期化＆送信データがまだセットされていない場合


typedef enum{   // 送信状態
	_SEND_CB_NONE,		   // なにもしていない
	_SEND_CB_DSDATA_PACK,	// DS専用 データを蓄積
	_SEND_CB_FIRST_SEND,	 // 流れの中での最初の送信
	_SEND_CB_FIRST_SENDEND,  // 最初の送信のコールバックが来た
} _sendCallbackCondition_e;




//==============================================================================
// ワーク
//==============================================================================


static _COMM_WORK_SYSTEM* _pComm = NULL;  ///<　ワーク構造体のポインタ

// 送信したことを確認するためのフラグ
static volatile u8 _sendServerCallBack = _SEND_CB_FIRST_SENDEND;
// 同上
static volatile u8 _sendCallBack = _SEND_CB_FIRST_SENDEND;

static void _debugSetSendCallBack(u8 data,int line)
{
	const int test = line;	//ワーニング回避
//	NET_PRINT("scb: %d\n",line);
	_sendCallBack = data;
}

#define   _setSendCallBack(state)  _debugSetSendCallBack(state, __LINE__)


#pragma mark [>NewFuncList

//==============================================================================
// static宣言
//==============================================================================
static void _commCommandInit(void);
static void _dataDsStep( MPDSContext *DSContext );
static BOOL _sendDataSet(BOOL bDataShare);
static void _recvDataServerFunc(void);
static BOOL _setSendDataSystem(u8* pSendBuff);

//==============================================================================
/**
 * @brief   最大接続人数を得る
 * @return  最大接続人数
 */
//==============================================================================

static int _getUserMaxNum(void)
{
	return GFI_NET_GetConnectNumMax();
}

//==============================================================================
/**
 * @brief   子機送信byte数を得ます
 * @retval  子機送信サイズ
 */
//==============================================================================

static int _getUserMaxSendByte(void)
{
	return GFI_NET_GetSendSizeMax();
}

//==============================================================================
/**
 * @brief   親機送信byte数を得ます
 * @retval  親機送信サイズ
 */
//==============================================================================

static int _getUserMaxSendByteParent(void)
{
	return GFI_NET_GetSendSizeMax()*GFI_NET_GetConnectNumMax()+_MP_DATA_HEADER_POS;
}

//==============================================================================
/**
 * @brief   親子共通、通信の初期化をまとめた
 * @param   packetSizeMax   確保したいパケットサイズ
 * @param   heapID		  確保してもいいheapID
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================

BOOL _commSystemInit(int packetSizeMax, HEAPID heapID)
{
//	void* pWork;
//	int i;

	if(_pComm==NULL){
		int machineMax = _getUserMaxNum();
		int parentSize = _getUserMaxSendByteParent();

		NET_PRINT("_COMM_WORK_SYSTEM size %d \n", sizeof(_COMM_WORK_SYSTEM));
		_pComm = (_COMM_WORK_SYSTEM*)GFL_HEAP_AllocClearMemory(heapID, sizeof(_COMM_WORK_SYSTEM));

		_pComm->packetSizeMax = packetSizeMax + 64;

		_pComm->pRecvBufRing = (u8*)GFL_HEAP_AllocClearMemory(heapID, _pComm->packetSizeMax*2); ///< 子機が受け取るバッファ
		_pComm->pTmpBuff = (u8*)GFL_HEAP_AllocClearMemory(heapID, _pComm->packetSizeMax*2);  ///< 受信受け渡しのための一時バッファ
		_pComm->pServerRecvBufRing = (u8*)GFL_HEAP_AllocClearMemory(heapID, machineMax * _pComm->packetSizeMax);   ///< 受け取るバッファをバックアップする
		// キューの初期化

		_pComm->sSendBuf = (u8*)GFL_HEAP_AllocClearMemory(heapID, _getUserMaxSendByte());
		_pComm->sSendBufRing = (u8*)GFL_HEAP_AllocClearMemory(heapID, _getUserMaxSendByte()*_SEND_RINGBUFF_FACTOR_CHILD);
		_pComm->sSendServerBuf = (u8*)GFL_HEAP_AllocClearMemory(heapID, parentSize);
		_pComm->sSendServerBufRing = (u8*)GFL_HEAP_AllocClearMemory(heapID, parentSize * _SEND_RINGBUFF_FACTOR_PARENT);


		GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgr, _SENDQUEUE_NUM_MAX, &_pComm->sendRing, heapID);
		GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgrServer, _SENDQUEUE_SERVER_NUM_MAX, &_pComm->sendServerRing, heapID);

		_commCommandInit();
	}

	_pComm->bitmap = 0;
	_pComm->bWifiConnect = FALSE;
	return TRUE;
}


//==============================================================================
/**
 * @brief   親子共通、通信のコマンド管理の初期化をまとめた
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandInit(void)
{
	//void* pWork;
	int i;
	int machineMax = _getUserMaxNum();
	int parentSize = _getUserMaxSendByteParent();
	int childSize = _getUserMaxSendByte();

	// 親機のみの送受信
	{
		NET_PRINT("packet %d %d\n",_pComm->packetSizeMax,machineMax);
		MI_CpuFill8(_pComm->pServerRecvBufRing, 0, _pComm->packetSizeMax * machineMax);
		for(i = 0; i< machineMax;i++){
			GFL_NET_RingInitialize(&_pComm->recvServerRing[i],
								   &_pComm->pServerRecvBufRing[i*_pComm->packetSizeMax],
								   _pComm->packetSizeMax);
		}

	}
	MI_CpuFill8(_pComm->sSendServerBufRing, 0, parentSize * _SEND_RINGBUFF_FACTOR_PARENT);
	GFL_NET_RingInitialize(&_pComm->sendServerRing, _pComm->sSendServerBufRing,
						   parentSize * _SEND_RINGBUFF_FACTOR_PARENT);
	for(i = 0; i < parentSize; i++){
		_pComm->sSendServerBuf[i] = GFL_NET_CMD_NONE;
	}
	// 子機の送受信
	MI_CpuFill8(_pComm->sSendBufRing, 0, childSize*_SEND_RINGBUFF_FACTOR_CHILD);
	GFL_NET_RingInitialize(&_pComm->sendRing, _pComm->sSendBufRing, childSize*_SEND_RINGBUFF_FACTOR_CHILD);

	for(i = 0;i < childSize;i++){
		_pComm->sSendBuf[i] = GFL_NET_CMD_NONE;
	}
	_pComm->sSendBuf[_DS_HEADERNO] = _INVALID_HEADER;
	MI_CpuFill8(_pComm->pRecvBufRing, 0, _pComm->packetSizeMax*2);
	GFL_NET_RingInitialize(&_pComm->recvRing, _pComm->pRecvBufRing, _pComm->packetSizeMax*2);

	_pComm->bNextSendData = FALSE;
	_pComm->bNextSendDataServer = FALSE;
	for(i = 0; i< GFL_NET_MACHINE_MAX;i++){
		_pComm->bFirstCatch[i] = TRUE;
		_pComm->recvCommServer[i].valCommand = GFL_NET_CMD_NONE;
		_pComm->recvCommServer[i].realSize = 0xffff;
		_pComm->recvCommServer[i].tblSize = 0xffff;
		_pComm->recvCommServer[i].pRecvBuff = NULL;
		_pComm->recvCommServer[i].dataPoint = 0;
		_pComm->countSendRecvServer[i] = 0;
	}
	_pComm->countSendRecv = 0;
	_pComm->bWifiSendRecv = TRUE;
	_pComm->recvCommClient.valCommand = GFL_NET_CMD_NONE;
	_pComm->recvCommClient.realSize = 0xffff;
	_pComm->recvCommClient.tblSize = 0xffff;
	_pComm->recvCommClient.pRecvBuff = NULL;
	_pComm->recvCommClient.dataPoint = 0;

	_pComm->bFirstCatchP2C = TRUE;
	//_pComm->bSendNoneSend = TRUE;

	_sendServerCallBack = _SEND_CB_FIRST_SENDEND;
	_setSendCallBack( _SEND_CB_FIRST_SENDEND);

	// キューのリセット
	GFL_NET_QueueManagerReset(&_pComm->sendQueueMgr);
	GFL_NET_QueueManagerReset(&_pComm->sendQueueMgrServer);
	_pComm->bResetState = FALSE;
	_pComm->bDSSendOK = TRUE;

	NET_PRINT(" 通信データリセット\n");

}

void	GFL_NET_UpdateSystem( MPDSContext *DSContext )
{
	_dataDsStep( DSContext );
	_recvDataServerFunc();  // サーバー側の受信処理
	
}


//==============================================================================
/**
 * @brief   通信コールバックの最後に呼ぶ コマンドを消す
 * @param   netID   ネットID
 * @retval  none
 */
//==============================================================================

static void _endCallBack(int command,int size,void* pTemp, _RECV_COMMAND_PACK* pRecvComm)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	GFL_NETHANDLE* pNetHandle = GFL_NET_GetNetHandle(GFL_NET_SystemGetCurrentID());
	GFI_NET_COMMAND_CallBack(pRecvComm->sendID, pRecvComm->recvBit, command, size, pTemp, pNetHandle);
	if(GFL_NET_SystemGetCurrentID()==GFL_NET_NO_PARENTMACHINE){
		pNetHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
		if(command < GFL_NET_CMD_COMMAND_MAX){  // 親機に送るコマンドを制限した
			GFI_NET_COMMAND_CallBack(pRecvComm->sendID, pRecvComm->recvBit, command, size, pTemp, pNetHandle);
		}
	}
	pRecvComm->valCommand = GFL_NET_CMD_NONE;
	pRecvComm->realSize = 0xffff;
	pRecvComm->tblSize = 0xffff;
	pRecvComm->pRecvBuff = NULL;
	pRecvComm->dataPoint = 0;
}

//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   pRing  リングバッファのポインタ
 * @param   netID	 処理しているnetID
 * @param   pTemp	コマンドを結合するためのtempバッファ
 * @retval  none
 */
//==============================================================================

static void _recvDataFuncSingle(RingBuffWork* pRing, int netID, u8* pTemp, _RECV_COMMAND_PACK* pRecvComm)
{
	int mcSize = _getUserMaxSendByte();
	int size;
	u16 command;
	int bkPos;
	int realbyte;
	int sendID;
	int recvID;
	u8* pB;
//	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();


	while( GFL_NET_RingDataSize(pRing) != 0 ){
		bkPos = pRing->startPos;
		if(pRecvComm->valCommand != GFL_NET_CMD_NONE){
			command = pRecvComm->valCommand;
		}
		else{
            if( GFL_NET_RingDataSize(pRing) < 2 ){  // 残りデータが無い場合消す
                GFL_NET_RingGetByte(pRing);
                break;
            }
            command = GFL_NET_RingGetShort(pRing);
            if(command == GFL_NET_CMD_NONE){
				continue;
			}
		}
		bkPos = pRing->startPos;
		pRecvComm->valCommand = command;
		if(pRecvComm->tblSize != 0xffff){
			size = pRecvComm->tblSize;
		}
		else{
			if(_pComm->bError){
				return;
			}
            if( GFL_NET_RingDataSize(pRing) < _GFL_NET_QUEUE_HEADERBYTE ){  // 残りデータがヘッダーだけだったら次回
				pRing->startPos = bkPos;
				break;
			}
			size = GFL_NET_RingGetShort(pRing);
			pRecvComm->realSize = size;
			pRecvComm->tblSize = size;
			pRecvComm->sendID = netID;
			pRecvComm->recvBit = GFL_NET_RingGetByte(pRing);
		}
		bkPos = pRing->startPos; // 進める
		
		pB = NULL;
		if(GFI_NET_COMMAND_CreateBuffCheck(command) && GFI_NET_COMMAND_RecvCheck(pRecvComm->recvBit)){  // 受信バッファがある場合
			if(pRecvComm->pRecvBuff == NULL){
				pRecvComm->pRecvBuff = (u8*)GFI_NET_COMMAND_CreateBuffStart(command, pRecvComm->sendID, pRecvComm->tblSize);
			}
			pB = &pRecvComm->pRecvBuff[pRecvComm->dataPoint];
		}
		else{
			pRecvComm->pRecvBuff = pTemp;
			pB = pTemp;
		}
		if(GFI_NET_COMMAND_RecvCheck(pRecvComm->recvBit) == FALSE){
			pB = NULL;
		}

		if(GFI_NET_COMMAND_CreateBuffCheck(command)==FALSE){
			realbyte = GFL_NET_RingGets(pRing, pTemp, pRecvComm->realSize , pRecvComm->realSize );
		}
		else{
			realbyte = GFL_NET_RingGets(pRing, pTemp, pRecvComm->realSize, mcSize - _DS_HEADER_MAX);
		}
		pRecvComm->realSize -= realbyte;

		size = realbyte;
		if(pRecvComm->tblSize > pRecvComm->dataPoint){
			size = pRecvComm->tblSize - pRecvComm->dataPoint;
			if(size > realbyte){
				size = realbyte;
			}
			if(pB){
				MI_CpuCopy8(pTemp, pB, size);
			}
		}
		pRecvComm->dataPoint += size;
		if(pRecvComm->realSize == 0 ){
			_endCallBack(command, pRecvComm->tblSize, pRecvComm->pRecvBuff, pRecvComm);
		}
		else{   // まだ届いていない大きいデータの場合ぬける
			NET_PRINT("結合待ち command %d size %d %d\n",command,size,pRecvComm->realSize);
			if(GFI_NET_COMMAND_CreateBuffCheck(command)==FALSE){
				pRecvComm->realSize += realbyte;
				pRing->startPos = bkPos;
			}
			break;
		}
	}
}

//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _recvDataServerFunc(void)
{
	int id;
	int size;
	u8 command;
	int machineMax;

	if(!_pComm){
		return;
	}

	machineMax = _getUserMaxNum();

	for(id = 0; id < machineMax; id++){
		GFL_NET_RingEndChange(&_pComm->recvServerRing[id]);

		if(GFL_NET_RingDataSize(&_pComm->recvServerRing[id]) > 0){
#if 1
//			NET_PRINT("親機が子機%dを解析\n",id);
#endif
			_recvDataFuncSingle(&_pComm->recvServerRing[id], id, _pComm->pTmpBuff, &_pComm->recvCommServer[id]);
		}
	}
}

//==============================================================================
/**
 * @brief   DSデータ交換
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _dataDsStep( MPDSContext *DSContext )
{
	u8 i;
	s32 result;
	ShareData sendData;
	MPDSDataSet recvDataSet;
	
	(void)NETMemSet( &sendData, 0, sizeof(sendData) );

	// 他のスレッドと共有するグローバル変数に触る際には排他を行う
	EnterCriticalSection(); // 排他開始

	//if(_pComm->bDSSendOK)
	{
		if(_sendDataSet(TRUE))
		{
			// 処理が必要かも@@OO
		}
	}

	sendData.data[_DS_HEADERNO] = _INVALID_HEADER;
	result = MPDSTryStep( DSContext, _pComm->sSendBuf, &recvDataSet );

	if( result == MP_RESULT_OK )
	{
		_setSendCallBack( _SEND_CB_FIRST_SENDEND );
		_pComm->bDSSendOK=TRUE;
		// 共有データの内容を解釈する
		for ( i = 0; i < MY_MAX_NODES+1; i++ )
		{
			u8* adr = (u8*)MPDSGetData( DSContext, &recvDataSet, (u32)i );
			if ( adr != NULL )
			{
				// i 番目の共有相手からのデータを共有できた
				u8 *pHeader;
				pHeader = adr;
				adr += _DS_HEADER_MAX;
				if(pHeader[_DS_HEADERNO] == _INVALID_HEADER){
					continue;
				}
				if((_pComm->bFirstCatch[i]) && (pHeader[_DS_HEADERNO] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
					NET_PRINT("連続データ parent %d \n",i);
					continue;
				}
				{//_DS_CRC_MAX
					u16 crc = GFL_STD_CrcCalc(&pHeader[_DS_CRC_MAX], MY_DS_LENGTH-_DS_CRC_MAX);
					GF_ASSERT(pHeader[_DS_CRCNO1] == (crc & 0xff));
					GF_ASSERT(pHeader[_DS_CRCNO2] == ((crc & 0xff00) >> 8));
				}
				//NET_PRINT("RING %d %d\n",i,GFL_NET_RingDataSize(&_pComm->recvServerRing[i]));
				//DEBUG_DUMP((u8*)adr, MY_DS_LENGTH - _DS_HEADER_MAX ,"DB");
				GFL_NET_RingPuts(&_pComm->recvServerRing[i], adr, MY_DS_LENGTH - _DS_HEADER_MAX);

				_pComm->bFirstCatch[i] = FALSE;
				
			}
		}
	}
	LeaveCriticalSection(); // 排他終了
	
/*
	int index = 0;
	int mcSize;
	int machineMax;


	if (WH_GetSystemState() != WH_SYSSTATE_DATASHARING){
		return;
	}
	if(_pComm->bDSSendOK){
		if(_sendDataSet(TRUE)){
			// 処理が必要かも@@OO
		}
	}
	if (WH_StepDS(_pComm->sSendBuf) == TRUE) {  //同期取れた
		_setSendCallBack( _SEND_CB_FIRST_SENDEND );
		//	   _pComm->countSendRecv++;  //wifi client
		_pComm->bDSSendOK=TRUE;
		{
			mcSize = _getUserMaxSendByte();
			machineMax = _getUserMaxNum();
			for(index = 0; index < machineMax ; index++){ // 受信コールバック
				u8 *pHeader;
				u8* adr = (u8*)WH_GetSharedDataAdr(index);
				if(adr==NULL){
					continue;
				}
				pHeader = adr;
				adr += _DS_HEADER_MAX;
				if(pHeader[_DS_HEADERNO] == _INVALID_HEADER){
					continue;
				}
				if((_pComm->bFirstCatch[index]) && (pHeader[_DS_HEADERNO] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
					NET_PRINT("連続データ parent %d \n",index);
					continue;
				}
				{//_DS_CRC_MAX
					u16 crc = GFL_STD_CrcCalc(&pHeader[_DS_CRC_MAX], mcSize-_DS_CRC_MAX);
					GF_ASSERT(pHeader[_DS_CRCNO1] == (crc & 0xff));
					GF_ASSERT(pHeader[_DS_CRCNO2] == ((crc & 0xff00) >> 8));
				}
				NET_PRINT("RING %d %d\n",index,GFL_NET_RingDataSize(&_pComm->recvServerRing[index]));
				DEBUG_DUMP((u8*)adr, mcSize - _DS_HEADER_MAX ,"DB");
				GFL_NET_RingPuts(&_pComm->recvServerRing[index], adr, mcSize - _DS_HEADER_MAX);

				_pComm->bFirstCatch[index] = FALSE;
			}
		}
	}
	else{  //同期できなかった=受け取れてない
		_pComm->bDSSendOK=FALSE;
		//		NET_PRINT("DS同期FALSE\n");
	}
	*/
}


//==============================================================================
/**
 * @brief   送るデータのセットをフラグを見て行う
 * @param   BOOL   bDataShare データがない場合からのデータを送る同期通信の場合
 * @retval  TRUE   送信準備ができた
 * @retval  FALSE  送信準備ができなかった
 */
//==============================================================================

static BOOL _sendDataSet(BOOL bDataShare)
{
	BOOL bSend = FALSE;

	if(_sendCallBack != _SEND_CB_FIRST_SENDEND){  // 1個送ったが送信完了していない
		//NET_PRINT("うけとってない %d _sendCallBack\n",_sendCallBack);
		return FALSE;
	}
	if(bDataShare){
		if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
			NET_PRINT("親の同期待ち\n");
			return FALSE;
		}
		_setSendDataSystem(_pComm->sSendBuf);  // 送るデータをリングバッファから差し替える
		_setSendCallBack( _SEND_CB_NONE );
		bSend=TRUE;
	}
	else{
		if(_setSendDataSystem(_pComm->sSendBuf)){  //送るデータが存在するときだけフラグを立てる
			_setSendCallBack( _SEND_CB_NONE );
			bSend=TRUE;
		}
	}
	return bSend;
}

//==============================================================================
/**
 * @brief   送信キューにあったものを送信バッファに入れる
 * @param   none
 * @retval  none
 */
//==============================================================================

static BOOL _setSendDataSystem(u8* pSendBuff)
{
	int i;
	int mcSize = _getUserMaxSendByte();
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	
	if(_pComm->bNextSendData == FALSE){  // 一回で送れる場合
		pSendBuff[_DS_HEADERNO] = _SEND_NONE;
	}
	else{
		pSendBuff[_DS_HEADERNO] = _SEND_NEXT;  // 一回で送れない場合
	}
	_pComm->bNextSendData = FALSE;
	if(GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgr) 
		|| (pInit->bNetType==GFL_NET_TYPE_IRC || pInit->bNetType==GFL_NET_TYPE_IRC_WIRELESS) )
	{
		pSendBuff[_DS_HEADERNO] = _INVALID_HEADER;  // 空っぽなら何も送らない
		return FALSE;  // 送るものが何も無い
	}
	else
	{
		SEND_BUFF_DATA buffData;
		buffData.size = mcSize - _DS_HEADER_MAX;
		buffData.pData = &pSendBuff[_DS_HEADER_MAX];
		if(!GFL_NET_QueueGetData(&_pComm->sendQueueMgr, &buffData)){
			_pComm->bNextSendData = TRUE;
		}
		GFL_NET_QueueInc(&_pComm->sendQueueMgr, &pSendBuff[_DS_INCNO]);
		{
			u16 crc = GFL_STD_CrcCalc(&pSendBuff[_DS_CRC_MAX], mcSize - _DS_CRC_MAX);
			pSendBuff[_DS_CRCNO1] = crc & 0xff;
			pSendBuff[_DS_CRCNO2] = (crc & 0xff00) >> 8;
		}
		DEBUG_DUMP((u8*)pSendBuff, buffData.size ,"Pkt");
		NET_PRINT("送信\n");
	}
	return TRUE;
}

#ifdef GFL_NET_DEBUG

//==============================================================================
/**
 * @brief   デバッグ用にダンプを表示する
 * @param   adr		   表示したいアドレス
 * @param   length		長さ
 * @param   pInfoStr	  表示したいメッセージ
 * @retval  サービス番号
 */
//==============================================================================

void GFL_NET_SystemDump_Debug(u8* adr, int length, char* pInfoStr)
{
	int i,j = 0;

	NET_PRINT("%s \n",pInfoStr);
	while(length){
		NET_PRINT(">> ");
		for(i = 0 ; i < 8 ; i++){
			NET_PRINT("%2x ",adr[j]);
			j++;
			if(j == length){
				break;
			}
		}
		NET_PRINT("\n");
		if(j == length){
			break;
		}
	}
	NET_PRINT(" --end\n");
}

#endif


//==============================================================================
/**
 * @brief   自分の機のIDを返す
 * @param
 * @retval  自分の機のID  つながっていない場合GFL_NET_PARENT_NETID
 */
//==============================================================================

u16 GFL_NET_SystemGetCurrentID(void)
{
	//親機なので0でいい
	return 0;
}

//==============================================================================
/**
 * @brief   エラーにする場合TRUE
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetError(void)
{
	_pComm->bError=TRUE;
}


//==============================================================================
/**
 * @brief   送信メソッド
 * @param   command	comm_sharing.hに定義したラベル
 * @param   data	   送信したいデータ ない時はNULL
 * @param   size	   送信量	コマンドだけの場合0
 * @param   bFast	   送信量	コマンドだけの場合0
 * @param   myID	   送信する人のハンドルno
 * @param   sendBit	 送る相手のBIT   全員なら0xff
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendData(int command, const void* data, int size, BOOL bFast, int myID, int sendBit, BOOL bSendBuffLock)
{
	int bSave = !bSendBuffLock;
	int cSize;  // = GFI_NET_COMMAND_GetPacketSize(command);
	SEND_QUEUE_MANAGER* pMgr;
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();

	DEBUG_DUMP((u8*)data, size ,"pkt");

	if(_pComm==NULL){
		return FALSE;
	}
	cSize = size;
	if(cSize >= _pComm->packetSizeMax){  // 大きいコマンドは保存しない SendDataExを使う必要がある
#if PM_DEBUG
		GF_ASSERT(0);
#endif
		bSave = FALSE;
	}
	if((pInit->bMPMode) && (myID == GFL_NET_NETID_SERVER)){
		pMgr = &_pComm->sendQueueMgrServer;
	}
	else{
		pMgr = &_pComm->sendQueueMgr;
	}

#if GFL_IRC_DEBUG
	OS_TPrintf("aaa que put size = %d, fast = %d, myID = %d\n", cSize, bFast, myID);
	{
		int i;
		u8 *buf;
		buf = (u8*)data;
	//	OS_TPrintf("que buf print\n");
	//	for(i = 0; i < cSize; i++){
	//		OS_TPrintf("%d, ", buf[i]);
	//	}
	//	OS_TPrintf("\n");
	}
#endif
	if(GFL_NET_QueuePut(pMgr, command, (u8*)data, cSize, bFast, bSave, sendBit)){
		return TRUE;
	}
	NET_PRINT("-キュー無い-");
	GFL_NET_SystemSetError();
	return FALSE;
}


//==============================================================================
/**
 * @brief   特定のコマンドを送信し終えたかどうかを調べる
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//==============================================================================


BOOL GFL_NET_SystemIsSendCommand(int command,int myID)
{
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	if((pInit->bMPMode) && (myID == 0)){
		return GFL_NET_QueueIsCommand(&_pComm->sendQueueMgrServer, command);
	}
	return GFL_NET_QueueIsCommand(&_pComm->sendQueueMgr, command);

}

//==============================================================================
/**
 * @brief   送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================

int GFL_NET_SystemGetSendRestSize(void)
{
	return GFL_NET_RingDataRestSize(&_pComm->sendRing);
}

//==============================================================================
/**
 * @brief   サーバ側の送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================

int GFL_NET_SystemGetSendRestSize_ServerSide(void)
{
	return GFL_NET_RingDataRestSize(&_pComm->sendServerRing);
}


//==============================================================================
/**
 * @brief   通信可能状態なのかどうかを返す ただしコマンドによるネゴシエーションがまだの状態
 * @param   親子機のnetID
 * @retval  TRUE  通信可能	FALSE 通信切断
 */
//==============================================================================
BOOL GFL_NET_SystemIsConnect(u16 netID)
{
	//おそらく独自実装が必要・・・
	return TRUE;
/*
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if(!_pComm){
		return FALSE;
	}
	if( pNetIni->bNetType == GFL_NET_TYPE_WIFI ){
#if GFL_NET_WIFI
		//#if 0 //wifi
		if(_pComm->bWifiConnect){
			int id = mydwc_getaid();
			if(-1 != id){
				if(netID == 0){
					return TRUE;
				}
				else if(netID == 1){
					return TRUE;
				}
			}
		}
#endif
		return FALSE;
	}
	
	if(pNetIni->bNetType == GFL_NET_TYPE_IRC || pNetIni->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
#if GFL_NET_IRC
		if(GFL_NET_IRC_IsConnect() == TRUE){
			return TRUE;
		}
#endif
		return FALSE;
	}
	
	if(!GFL_NET_SystemIsInitialize()){
		return FALSE;
	}
	if (!GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) {
		return FALSE;
	}
	if(GFL_NET_SystemGetCurrentID()==netID){// 自分はONLINE
		return TRUE;
	}
	else if(GFL_NET_SystemGetCurrentID()==GFL_NET_NO_PARENTMACHINE){  // 親機のみ子機情報をLIBで得られる
		u16 bitmap = GFL_NET_WL_GetBitmap();
		if( bitmap & (1<<netID)){
			return TRUE;
		}
	}
	else if(_pComm->bitmap & (1<<netID)){
		return TRUE;
	}
	return FALSE;
	*/
}

#pragma mark [>OldFuncList
#if 0
//==============================================================================
// static宣言
//==============================================================================

static void _dataMpStep(void);
static void _dataDsStep(void);

static void _dataMpServerStep(void);
static void _sendCallbackFunc(BOOL result);
static void _sendServerCallback(BOOL result);
static void _commRecvCallback(u16 aid, u16 *data, u16 size);
static void _commRecvParentCallback(u16 aid, u16 *data, u16 size);

static void _recvDataFunc(void);
static void _setSendDataServer(u8* pSendBuff);

static BOOL _sendQueueSet(void);
static BOOL _sendServerQueueSet(void);
static void _queueSet(int restSize);
static void _queueSetServer(int restSize);
static void _spritDataSendFunc(void);

static void _commRecvIrcCallback(u16 aid, u16 *data, u16 size);
static void _dataIrcStep(void);
static void _update_irc_data(void *data, int size);



//==============================================================================
/**
 * @brief   指定された子機の領域をクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _clearChildBuffers(int netID)
{
	//	_pComm->recvDSCatchFlg[netID] = 0;  // 通信をもらったことを記憶 DS同期用
	GF_ASSERT(netID < GFL_NET_MACHINE_MAX);

	_pComm->bFirstCatch[netID] = TRUE;  // コマンドをはじめてもらった時用
	_pComm->countSendRecvServer[netID] = 0;  // SERVER受信

	GFL_NET_RingInitialize(&_pComm->recvServerRing[netID],
						   &_pComm->pServerRecvBufRing[netID * _pComm->packetSizeMax],
						   _pComm->packetSizeMax);

	_pComm->recvCommServer[netID].valCommand = GFL_NET_CMD_NONE;
	_pComm->recvCommServer[netID].realSize = 0xffff;
	_pComm->recvCommServer[netID].tblSize = 0xffff;
	_pComm->recvCommServer[netID].pRecvBuff = NULL;
	_pComm->recvCommServer[netID].dataPoint = 0;
}

//==============================================================================
/**
 * @brief   接続時の処理
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _connectFunc(void)
{
	int i;

	for(i = 0 ; i < GFL_NET_MACHINE_MAX ; i++){
		if((!GFL_NET_SystemIsConnect(i)) && !_pComm->bFirstCatch[i]){
			if(!GFL_NET_SystemGetAloneMode()){
				NET_PRINT("非接続になった時に %d\n",i);
				_clearChildBuffers(i);  // 非接続になった時に
			}
		}
	}

}

#if GFL_NET_WIFI

//==============================================================================
/**
 * @brief   WiFiGameの初期化を行う
 * @param   packetSizeMax パケットのサイズマックス
 * @param   regulationNo  ゲームの種類
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemWiFiModeInit(int packetSizeMax, HEAPID heapIDSys, HEAPID heapIDWifi)
{
	BOOL ret = TRUE;
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if(!GFL_NET_IsInitIchneumon()){
		return FALSE;
	}



	//sys_CreateHeapLo( HEAPID_BASE_APP, heapIDWifi, _HEAPSIZE_WIFI);
	_commSystemInit(packetSizeMax, heapIDSys);
	mydwc_startConnect( GFI_NET_GetMyDWCUserData() ,GFI_NET_GetMyDWCFriendData(),
						heapIDWifi, pNetIni->maxBeaconNum);
	//	mydwc_setFetalErrorCallback(CommFatalErrorFunc);   //@@OO エラー処理追加必要 07/02/22
	mydwc_setReceiver( _commRecvParentCallback, _commRecvCallback );
	//	GFL_NET_SystemSetTransmissonTypeDS();
	return TRUE;
}

//==============================================================================
/**
 * @brief   wifiのゲームを開始する
 * @param   target:   負なら親、０以上ならつなぎにいく親機の番号
 * @retval  TRUE	  成功
 * @retval  FALSE	 失敗
 */
//==============================================================================
int GFL_NET_SystemWifiApplicationStart( int target )
{
	if( mydwc_getFriendStatus(target) != DWC_STATUS_MATCH_SC_SV ){
		return FALSE;
	}
	//	mydwc_setReceiver( CommRecvParentCallback, CommRecvCallback );
	return TRUE;
}

#endif //GFL_NET_WIFI

#ifdef GFL_NET_IRC
static void _irc_commRecvCallback(u16 aid, u16 *data, u16 size)
{
	_commRecvIrcCallback(aid, data, size);
#if 0
	if(GFL_NET_IRC_IsSender() == TRUE){
		//親機としての受信処理
		_commRecvParentCallback(aid, data, size);
	}
	else{
		//子機としての受信処理
		_commRecvCallback(aid, data, size);
	}
#endif
}

//==============================================================================
/**
 * @brief   IRC通信システムの初期化を行う
 * @param   bTGIDChange	新規のゲームの初期化の場合TRUE 古いビーコンでの誤動作を防ぐため用
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemIrcModeInit(BOOL bChangeTGID)
{
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();

	pInit->bTGIDChange = bChangeTGID;
	GFL_NET_IRC_RecieveFuncSet(_irc_commRecvCallback);
	_commSystemInit(GFL_NET_IRC_SEND_MAX, pInit->netHeapID);
	_clearChildBuffers(0);	//※check　暫定で3つ分固定でクリア。
	_clearChildBuffers(1);	//			本来なら接続完了時に接続された子機だけクリアする
	_clearChildBuffers(2);

//	_setSendCallBack( _SEND_CB_NONE );
	return TRUE;
}
#endif	//GFL_NET_IRC


//==============================================================================
/**
 * @brief   親機の初期化を行う
 * @param   id
 * @retval  none
 */
//==============================================================================
static void _connectCallBackP(int id)
{
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

	_clearChildBuffers(id);
	if(pInit->connectHardFunc){  // コールバック呼び出し
		pInit->connectHardFunc(pNet->pWork, id);
	}
}

//==============================================================================
/**
 * @brief   親機の初期化を行う
 * @param   id
 * @retval  none
 */
//==============================================================================
static void _connectCallBackC(int id)
{
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

	if(pInit->connectHardFunc){  // コールバック呼び出し
		pInit->connectHardFunc(pNet->pWork, id);
	}
}

//==============================================================================
/**
 * @brief   親機の初期化を行う
 * @param   work_area 　システムで使うメモリー領域
 *					  NULLの場合すでに初期化済みとして動作
 * @param   bTGIDChange	新規のゲームの初期化の場合TRUE 古いビーコンでの誤動作を防ぐため用
 * @param   packetSizeMax  大きいパケットがいる場合、メモリーも大きくなる
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFI_NET_SystemParentModeInit(BOOL bChangeTGID, int packetSizeMax)
{
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	BOOL bRet = FALSE;

	pInit->bTGIDChange = bChangeTGID;
	// WH_ParentDataInit();
	WHSetConnectCallBack(_connectCallBackP);
	WH_SetReceiver(_commRecvParentCallback);
	_commSystemInit(packetSizeMax, _GFL_NET_GetNETInitStruct()->netHeapID);
	if(WH_GetSystemState() == WH_SYSSTATE_IDLE){
		if(WH_StartMeasureChannel()){
			bRet = TRUE;
		}
	}
	return bRet;
}

//==============================================================================
/**
 * @brief   親機の確立を行う
 * @param   channelChange  親機のチャンネルを変更する場合TRUE 基本は変更で
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFI_NET_SystemParentModeInitProcess(BOOL channelChange)
{
	return GFL_NET_WL_ParentConnect(channelChange);
}


//==============================================================================
/**
 * @brief   子機の初期化を行う
 * @param   work_area 　システムで使うメモリー領域
 *					  NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildModeInit(BOOL bBconInit, int packetSizeMax)
{
	BOOL ret = TRUE;

	WHSetConnectCallBack(_connectCallBackC);
	ret = GFL_NET_WLChildInit(bBconInit);
	if(ret==TRUE){
		GFL_NET_WLSetRecvCallback( _commRecvCallback );
		_commSystemInit(packetSizeMax, _GFL_NET_GetNETInitStruct()->netHeapID);
		_setSendCallBack( _SEND_CB_FIRST_SENDEND );
	}
	return ret;
}

//==============================================================================
/**
 * @brief   子機の初期化を行う+ 接続にすぐ行く
 * @param   work_area 　システムで使うメモリー領域
 *					  NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildModeInitAndConnect(BOOL bInit,u8* pMacAddr,int packetSizeMax,_PARENTFIND_CALLBACK pCallback)
{
	BOOL ret = TRUE;
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
	u16 mode[]={WH_CONNECTMODE_DS_CHILD,WH_CONNECTMODE_MP_CHILD};

	GFL_NET_WLSetRecvCallback( _commRecvCallback );
	if (WH_GetSystemState() == WH_SYSSTATE_SCANNING) {
		(void)WH_EndScan();
		return FALSE;
	}
	else if (WH_GetSystemState() == WH_SYSSTATE_IDLE) {
		if(bInit){
			_commSystemInit(packetSizeMax, _GFL_NET_GetNETInitStruct()->netHeapID);
			_setSendCallBack( _SEND_CB_FIRST_SENDEND );
		}
		WH_ChildConnectAuto(mode[pInit->bMPMode], pMacAddr, 0);
		GFI_NET_BeaconSetScanCallback(pCallback);
		return TRUE;
	}
	return FALSE;
}

//==============================================================================
/**
 * @brief   通信切断を行う
 * @param   none
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemFinalize(void)
{
	BOOL bEnd = FALSE;
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if(_pComm){
		if( pNetIni->bNetType == GFL_NET_TYPE_WIFI ){
#if GFL_NET_WIFI
			mydwc_Logout();  // 切断
#endif
		}
		else if(pNetIni->bNetType == GFL_NET_TYPE_IRC){
#if GFL_NET_IRC
			IRC_Shutdown();
#endif
		}
		else{
			WH_Finalize();
		}
		NET_PRINT("切断----開放処理--\n");
		GFL_HEAP_FreeMemory(_pComm->pRecvBufRing);
		GFL_HEAP_FreeMemory(_pComm->pTmpBuff);
		GFL_HEAP_FreeMemory(_pComm->pServerRecvBufRing);
		GFL_NET_QueueManagerFinalize(&_pComm->sendQueueMgrServer);
		GFL_NET_QueueManagerFinalize(&_pComm->sendQueueMgr);
		GFL_HEAP_FreeMemory(_pComm->sSendBuf);
		GFL_HEAP_FreeMemory(_pComm->sSendBufRing);
		GFL_HEAP_FreeMemory(_pComm->sSendServerBuf);
		GFL_HEAP_FreeMemory(_pComm->sSendServerBufRing);
		GFL_HEAP_FreeMemory(_pComm);
		_pComm = NULL;
	}
}

//==============================================================================
/**
 * @brief   子機 index接続
 * @param   index   親のリストのindex
 * @retval  子機接続を親機に送ったらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildIndexConnect(u16 index,_PARENTFIND_CALLBACK pCallback,GFL_NETHANDLE* pHandle)
{
	return GFL_NET_WLChildIndexConnect(index, pCallback, pHandle);
}


//==============================================================================
/**
 * @brief   終了コマンドを受信したらモードに応じて処理を行い自動切断する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _autoExitSystemFunc(void)
{
	if(!GFL_NET_WLIsAutoExit()){
		return;
	}
	if(GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE){   // 自分が親の場合みんなに逆返信する
		if(GFL_NET_WLIsChildsConnecting()){
			return;
		}
		GFL_NET_SystemFinalize();  // 終了処理に入る
	}
	else{   //子機の場合
		GFL_NET_SystemFinalize();  // 終了処理に入る
	}
}

//==============================================================================
/**
 * @brief   通信データの更新処理  データを収集
 *	main.c   から  vblank後にすぐに呼ばれる
 * @param   none
 * @retval  データシェアリング同期が取れなかった場合FALSE
 */
//==============================================================================


BOOL GFL_NET_SystemUpdateData(void)
{
	int j;

	if(_pComm != NULL){

		if( _pComm->bWifiConnect ){
#if GFL_NET_WIFI
			GFL_NET_WirelessIconEasy_SetLevel(WM_LINK_LEVEL_3 - DWC_GetLinkLevel());
#endif
		}
		else if(GFL_NET_WLIsInitialize()){
			GFL_NET_WirelessIconEasy_SetLevel(WM_LINK_LEVEL_3 - WM_GetLinkLevel());
		}

		if(!_pComm->bShutDown){
			GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
			if(!pInit->bMPMode && (pInit->bNetType == GFL_NET_TYPE_WIRELESS)){
				_dataDsStep();
				_recvDataServerFunc();  // サーバー側の受信処理
			}
			else if(pInit->bNetType == GFL_NET_TYPE_IRC || pInit->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
#if GFL_NET_IRC
				_dataIrcStep();
				_recvDataServerFunc();  // サーバー側の受信処理
				GFL_NET_IRC_CommandContinue();
#endif
			}
			else{
				_dataMpStep();
				_recvDataFunc();	// 子機としての受け取り処理
				if((GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE) && (GFL_NET_SystemIsConnect(GFL_NET_NO_PARENTMACHINE)) || GFL_NET_SystemGetAloneMode() ){
					// サーバーとしての処理
					_dataMpServerStep();
				}
				if((GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE) || GFL_NET_SystemGetAloneMode() ){
					_recvDataServerFunc();  // サーバー側の受信処理
				}
			}
		}
		CommMpProcess(_pComm->bitmap);
		if(GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE){
			_connectFunc();
		}
		_autoExitSystemFunc();  // 自動切断 _pComm=NULLになるので注意
	}
	else{
		CommMpProcess(0);
	}
	return TRUE;
}


//==============================================================================
/**
 * @brief   通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemReset(void)
{
	if(_pComm){
		_commCommandInit();
	}
}

//==============================================================================
/**
 * @brief   DSモードで通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetDS(void)
{
	if(_pComm){
		_commCommandInit();
	}
}

//==============================================================================
/**
 * @brief   通信バッファをクリアーする+ビーコンの初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetBattleChild(void)
{
	if(_pComm){
		_commCommandInit();
		GFL_NET_WLChildBconDataInit();
	}
}


//==============================================================================
/**
 * @brief   データの収集
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _updateMpData(void)
{
	u16 i;
	u8 *adr;
	int size;

	{
		int mcSize = _getUserMaxSendByte();
		if(GFL_NET_SystemGetAloneMode()){   // aloneモードの場合
			if(_sendCallBack == _SEND_CB_NONE){
				_setSendCallBack( _SEND_CB_FIRST_SEND );
				_sendCallbackFunc(TRUE);
				// 子機のふりをする部分		  // 親機は自分でコールバックを呼ぶ
				_commRecvParentCallback(GFL_NET_NO_PARENTMACHINE, (u16*)_pComm->sSendBuf,
										mcSize);
				//_pComm->sendSwitch = 1 - _pComm->sendSwitch;
				_pComm->countSendRecv++; // MP送信親
				return;
			}
		}
		if(GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())){
			if(!GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID())){
				return;
			}
			if(_sendCallBack == _SEND_CB_NONE){
				// 子機データ送信
				if(GFL_NET_SystemGetCurrentID() != GFL_NET_NO_PARENTMACHINE){
					if(!GFL_NET_WL_SendData(_pComm->sSendBuf,
											mcSize, _sendCallbackFunc)){
						NET_PRINT("failed WH_SendData\n");
					}
					else{
						_setSendCallBack( _SEND_CB_FIRST_SEND );
						_pComm->countSendRecv++; // MP送信
					}
				}
				else{		// サーバーとしての処理 自分以外の誰かにつながっている時
					_setSendCallBack( _SEND_CB_FIRST_SEND );
					_sendCallbackFunc(TRUE);
					// 子機のふりをする部分		  // 親機は自分でコールバックを呼ぶ
					_commRecvParentCallback(GFL_NET_NO_PARENTMACHINE, (u16*)_pComm->sSendBuf,
											mcSize);
					_pComm->countSendRecv++; // MP送信
				}
			}
		}
	}
}

//==============================================================================
/**
 * @brief   DSデータ交換
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _dataDsStep(void)
{
	int index = 0;
	int mcSize;
	int machineMax;


	if (WH_GetSystemState() != WH_SYSSTATE_DATASHARING){
		return;
	}
	if(_pComm->bDSSendOK){
		if(_sendDataSet(TRUE)){
			// 処理が必要かも@@OO
		}
	}
	if (WH_StepDS(_pComm->sSendBuf) == TRUE) {  //同期取れた
		_setSendCallBack( _SEND_CB_FIRST_SENDEND );
		//	   _pComm->countSendRecv++;  //wifi client
		_pComm->bDSSendOK=TRUE;
		{
			mcSize = _getUserMaxSendByte();
			machineMax = _getUserMaxNum();
			for(index = 0; index < machineMax ; index++){ // 受信コールバック
				u8 *pHeader;
				u8* adr = (u8*)WH_GetSharedDataAdr(index);
				if(adr==NULL){
					continue;
				}
				pHeader = adr;
				adr += _DS_HEADER_MAX;
				if(pHeader[_DS_HEADERNO] == _INVALID_HEADER){
					continue;
				}
				if((_pComm->bFirstCatch[index]) && (pHeader[_DS_HEADERNO] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
					NET_PRINT("連続データ parent %d \n",index);
					continue;
				}
				{//_DS_CRC_MAX
					u16 crc = GFL_STD_CrcCalc(&pHeader[_DS_CRC_MAX], mcSize-_DS_CRC_MAX);
					GF_ASSERT(pHeader[_DS_CRCNO1] == (crc & 0xff));
					GF_ASSERT(pHeader[_DS_CRCNO2] == ((crc & 0xff00) >> 8));
				}
				NET_PRINT("RING %d %d\n",index,GFL_NET_RingDataSize(&_pComm->recvServerRing[index]));
				DEBUG_DUMP((u8*)adr, mcSize - _DS_HEADER_MAX ,"DB");
				GFL_NET_RingPuts(&_pComm->recvServerRing[index], adr, mcSize - _DS_HEADER_MAX);

				_pComm->bFirstCatch[index] = FALSE;
			}
		}
	}
	else{  //同期できなかった=受け取れてない
		_pComm->bDSSendOK=FALSE;
		//		NET_PRINT("DS同期FALSE\n");
	}
}

//==============================================================================
/**
 * @brief   DSデータ交換(赤外線用)
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _dataIrcStep(void)
{
#if GFL_NET_IRC
	GFL_NET_IRC_Move();
	if(GFL_NET_IRC_IsConnectSystem() == TRUE){
		int mcSize = _getUserMaxSendByte();
		
		if(GFL_NET_IRC_SendCheck() == TRUE){
			if(_sendDataSet(FALSE)){
				_update_irc_data(_pComm->sSendBuf, mcSize);
				_setSendCallBack( _SEND_CB_FIRST_SENDEND );
			//?	_pComm->countSendRecv++;
			}
		}
	}
#endif
}

//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック IRC用
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================
static void _commRecvIrcCallback(u16 aid, u16 *data, u16 size)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
	u8 *pHeader = (u8*)data;
	u8* adr = ((u8*)data) + _DS_HEADER_MAX;
	int i,ringRestSize,ringRestSize2;
	int mcSize = _getUserMaxSendByte();
	RingBuffWork* pRing;
	int index = aid;// - 1;
	
//	GF_ASSERT(aid != 0);
	
#if GFL_IRC_DEBUG
	{
		int i;
		u8 *buf;
		buf = (u8*)data;
		OS_TPrintf("aaa irc recieve buf print : aid=%d,index=%d,size=%d\n", aid, index, size);
		for(i = 0; i < size; i++){
			OS_TPrintf("%d, ", buf[i]);
		}
		OS_TPrintf("\n");
	}
#endif

	if(pHeader[_DS_HEADERNO] == _INVALID_HEADER){
		return;
	}

//?	_pComm->countSendRecvServer[aid]--;  //SERVER受信

	if((_pComm->bFirstCatch[index]) && (pHeader[_DS_HEADERNO] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
		NET_PRINT("連続データ parent %d \n",index);
		return;
	}
	{//_DS_CRC_MAX
		u16 crc = GFL_STD_CrcCalc(&pHeader[_DS_CRC_MAX], mcSize-_DS_CRC_MAX);
		GF_ASSERT(pHeader[_DS_CRCNO1] == (crc & 0xff));
		GF_ASSERT(pHeader[_DS_CRCNO2] == ((crc & 0xff00) >> 8));
	}
	GFL_NET_RingPuts(&_pComm->recvServerRing[index], adr, mcSize - _DS_HEADER_MAX);


	_pComm->bFirstCatch[index] = FALSE;  //初回受信フラグをOFFする
}

//--------------------------------------------------------------
/**
 * @brief   IRC:送信バッファのデータを送信
 *
 * @param   none		
 *
 * @retval  
 *
 *
 */
//--------------------------------------------------------------
static void _update_irc_data(void *data, int size)
{
	int i;
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if(GFL_NET_IRC_SendTurnGet() == FALSE || GFL_NET_IRC_IsConnectSystem() == FALSE){
		GF_ASSERT(0);
		return;
	}
	
	if(_sendCallBack == _SEND_CB_NONE){
		// 子機データ送信
		IRC_PRINT("IRC Send : size=%d\n", size);
	#if 0
		if(GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE){
			//送信者が親機の場合は、子機へ直接送信せずに、親機受信コールバックへ回し、
			//親機の送信データとして子機へ流す
			_commRecvIrcCallback(GFL_NET_PARENT_NETID+1, data, size);
//			_commRecvParentCallback(GFL_NET_PARENT_NETID+1, data, size);
//			_commRecvCallback(GFL_NET_PARENT_NETID, data, size);
		}
		else{
			GFL_NET_IRC_Send(data, size, 0);
		}
	#else
		GFL_NET_IRC_Send(data, size, 0);
		//送信した自分も親機から再度配信された事を偽装する為、自分で受信を呼び出す
		_commRecvIrcCallback(GFL_NET_IRC_System_GetCurrentAid(), data, size);
	#endif
		_sendServerCallBack = _SEND_CB_FIRST_SENDEND;
//?		_pComm->countSendRecv++; // MP送信

		//_commRecvCallback(GFL_NET_IRC_GetCurrentAid(), data, size);
	}
}


//==============================================================================
/**
 * @brief   データ送信処理
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _dataMpStep(void)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if( pNetIni->bNetType == GFL_NET_TYPE_WIFI ){
#if GFL_NET_WIFI
		if( _pComm->bWifiConnect ){
			int mcSize = _getUserMaxSendByte();

			if(mydwc_canSendToServer()){
				if(_sendDataSet(_pComm->bWifiSendRecv)){
					if( mydwc_sendToServer( _pComm->sSendBuf, mcSize )){
						_setSendCallBack( _SEND_CB_FIRST_SENDEND );
						_pComm->countSendRecv++;  //wifi client
					}
				}
			}
		}
#endif
	}
	else if(((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) &&
			 (GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()))) || GFL_NET_SystemGetAloneMode()){
		_sendDataSet(TRUE);
		_updateMpData();	 // データ送受信
	}
}

//==============================================================================
/**
 * @brief   中間RINGBUFFから子機全員に逆送信するためbuffにコピー
 * @param   none
 * @retval  送ったらTRUE
 */
//==============================================================================
/*
static volatile int _debugCounter = 0;

static BOOL _copyDSData(void)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
	int mcSize;
	int machineMax;
	int i,num,bSend = FALSE;
	u8* pBuff;

	mcSize = _getUserMaxSendByte();
	machineMax = _getUserMaxNum();

	/// 中間RINGBUFFから子機全員に逆送信するためbuffにコピー
	for(i = 0; i < machineMax; i++){
//		GFL_NET_RingEndChange(&_pComm->recvMidRing[i]);
		pBuff = &_pComm->sSendServerBuf[_MP_DATA_HEADER_POS + (i * mcSize)];
		pBuff[0] = _INVALID_HEADER;
		num = GFL_NET_RingGets(&_pComm->recvMidRing[i] ,
							   pBuff,
							   mcSize);
		if(pBuff[0] != _INVALID_HEADER){
			bSend = TRUE;
		}
		if(i==0){
			_pComm->sSendServerBuf[2] = num;
		}
		if(i==1){
			_pComm->sSendServerBuf[3] = num;
		}
	}
	if(bSend == FALSE){
		_pComm->sSendServerBuf[0] = _INVALID_HEADER;
		return FALSE;
	}
	_debugCounter++;
	_pComm->sSendServerBuf[0] = _SEND_NONE + (_debugCounter * 0x10);
	if( pNetIni->bNetType){
#if GFL_NET_WIFI
		_pComm->sSendServerBuf[1] = mydwc_GetBitmap();
#endif
	}
	else{
		_pComm->sSendServerBuf[1] = WH_GetBitmap();
	}
//	OS_TPrintf("_copyDSDataTRU\n");
//	_pComm->sSendServerBuf[2] = mcSize;
	return TRUE;
}
 */

//==============================================================================
/**
 * @brief   データ送信処理  サーバー側
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _updateMpDataServer(void)
{
	int i;
	int debug=0;
	int mcSize ,machineMax;

	machineMax = _getUserMaxNum();

	if((_sendServerCallBack == _SEND_CB_DSDATA_PACK) && (GFL_NET_SystemGetConnectNum()>1)){
		_sendServerCallBack = _SEND_CB_FIRST_SEND;
		if( (GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()))  && !GFL_NET_SystemGetAloneMode()){
			//DEBUG_DUMP(_pComm->sSendServerBuf,32,"mi");
			if(!GFL_NET_WL_SendData(_pComm->sSendServerBuf,
									_getUserMaxSendByteParent(),
									_sendServerCallback)){
				_sendServerCallBack = _SEND_CB_DSDATA_PACK;
				NET_PRINT("送信失敗%d\n",__LINE__);
			}
		}
		else if((_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC 
				|| (_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
			if(GFL_NET_IRC_SendCheck() == FALSE){
				_sendServerCallBack = _SEND_CB_DSDATA_PACK;
				NET_PRINT("送信失敗%d\n",__LINE__);
			}
			else{
				int size;
				size = _pComm->sSendServerBuf[_MP_SIZEO1]*256 + _pComm->sSendServerBuf[_MP_SIZEO2];
				if(size == 0){
					return;
				}
				GFL_NET_IRC_Send(
					(u8*)(_pComm->sSendServerBuf), //_getUserMaxSendByteParent(), 
							GFI_NET_GetSendSizeMax(), 100);
			}
		}

		// 送信完了
		if((_sendServerCallBack == _SEND_CB_FIRST_SEND) ){

			for(i = 0; i < machineMax; i++){
				if(GFL_NET_SystemIsConnect(i)){
					_pComm->countSendRecvServer[i]++; // 親MP送信
				}
				else if(GFL_NET_SystemGetAloneMode() && (i == 0)){
					_pComm->countSendRecvServer[i]++; // 親MP送信
				}
			}
			// 親機自身に子機の動きをさせるためここでコールバックを呼ぶ
			_commRecvCallback(GFL_NET_NO_PARENTMACHINE,
							  (u16*)_pComm->sSendServerBuf,
							  _getUserMaxSendByteParent());
		}

		if( !(GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()))  || GFL_NET_SystemGetAloneMode() || (_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC || (_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
			// 割り込みが無い状況でも動かす為ここでカウント
			_sendServerCallBack = _SEND_CB_FIRST_SENDEND;
		}
	}
}

//==============================================================================
/**
 * @brief   データを送信しすぎていないかどうか検査する
 * @param   none
 * @retval  none
 */
//==============================================================================

static BOOL _checkSendRecvLimit(void)
{
	int i;

	for(i = 0; i < GFL_NET_MACHINE_MAX; i++){
		if(GFL_NET_SystemIsConnect(i)){
			if(_pComm->countSendRecvServer[i] > _SENDRECV_LIMIT){ // 送信しすぎの場合
				return FALSE;
			}
		}
	}
	return TRUE;
}

//==============================================================================
/**
 * @brief   データ送信処理  サーバー側
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _dataMpServerStep(void)
{
	int i;
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if( pNetIni->bNetType == GFL_NET_TYPE_WIFI){
#if GFL_NET_WIFI
		if( GFL_NET_SystemIsConnect(GFL_NET_NO_PARENTMACHINE) ){

			GF_ASSERT(!pNetIni->bMPMode);
			if((_sendServerCallBack == _SEND_CB_FIRST_SENDEND) && (mydwc_canSendToClient())){
				if( _pComm->bWifiSendRecv ){  // 同期を取っている場合
					if(_checkSendRecvLimit()){
						//	  _copyDSData();  //DS通信ならコピー
						_sendServerCallBack = _SEND_CB_DSDATA_PACK;
					}
				}
				else{
					//			  if(_copyDSData()){  //DS通信ならコピー
					//		   _sendServerCallBack = _SEND_CB_DSDATA_PACK;
					//	  }
				}
			}
			if(_sendServerCallBack==_SEND_CB_DSDATA_PACK){
				if( mydwc_sendToClient( _pComm->sSendServerBuf, _getUserMaxSendByteParent() )){
					OHNO_SP_PRINT("send %d \n",_pComm->sSendServerBuf[0]);
					_sendServerCallBack = _SEND_CB_FIRST_SENDEND;
					_pComm->countSendRecvServer[0]++; // wifi server
					_pComm->countSendRecvServer[1]++; // wifi server
				}
				else{
					NET_PRINT("mydwc_sendToClientに失敗\n");
				}
			}
		}
#endif  //wifi封印
	}
	else if(pNetIni->bNetType == GFL_NET_TYPE_IRC 
			|| pNetIni->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
#if GFL_NET_IRC
		if(_sendServerCallBack == _SEND_CB_FIRST_SENDEND){
			_setSendDataServer(_pComm->sSendServerBuf);  // 送るデータをリングバッファから差し替える
			_sendServerCallBack = _SEND_CB_DSDATA_PACK;
		}
		// 最初の送信処理
		_updateMpDataServer();
#endif
	}
	else if((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) || (GFL_NET_SystemGetAloneMode()) ){
		if(!_checkSendRecvLimit()){
			return;
		}
		if(_sendServerCallBack == _SEND_CB_FIRST_SENDEND){
			_setSendDataServer(_pComm->sSendServerBuf);  // 送るデータをリングバッファから差し替える
			_sendServerCallBack = _SEND_CB_DSDATA_PACK;
		}
		// 最初の送信処理
		_updateMpDataServer();
	}
}



//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック MP用
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================
static void _commRecvCallback(u16 aid, u16 *data, u16 size)
{
	u8* adr = (u8*)data;
	u8* adr2 = (u8*)data;
	int i;
	int recvSize = size;
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	OS_TPrintf("_commRecvCallback %d %d\n",aid, size);

#if GFL_IRC_DEBUG
	{
		int i;
		u8 *buf;
		buf = (u8*)data;
		OS_TPrintf("aaa child recieve buf print\n");
		for(i = 0; i < size; i++){
			OS_TPrintf("%d, ", buf[i]);
		}
		OS_TPrintf("\n");
	}
#endif

	_pComm->countSendRecv--;  //受信
#ifdef PM_DEBUG
	_pComm->countRecvNum++;
#endif
	if(adr==NULL){
		return;
	}
	{
		int mcSize = _getUserMaxSendByte()+4;
		DEBUG_DUMP(&adr[mcSize], 8,"cr1");
		DEBUG_DUMP(&adr[0], 8,"cr0");
	}

	if(adr[0] == _INVALID_HEADER){
		return;
	}

#ifdef WIFI_DUMP_TEST
	DEBUG_DUMP(&adr[0], _SEND_BUFF_SIZE_CHILD,"cr0");
	DEBUG_DUMP(&adr[_SEND_BUFF_SIZE_CHILD], _SEND_BUFF_SIZE_CHILD,"cr1");
#endif

	_pComm->bFirstCatchP2C = FALSE;

	_pComm->bitmap = adr[_MP_CONNECTBIT];


	recvSize = adr[_MP_SIZEO1]*256;  // MP受信サイズ
	recvSize += adr[_MP_SIZEO2];  // MP受信サイズ
	
	if((_pComm->bFirstCatchP2C) && (adr[0] & _SEND_NEXT)){
		// まだ一回もデータをもらったことがない状態なのに連続データだった
		NET_PRINT("連続データ child %d \n",aid);
		DEBUG_DUMP((u8*)data,24,"cr");
		return;
	}
	if(recvSize!=0){
		DEBUG_DUMP(&adr[0], _getUserMaxSendByte(),"MP");
	}
	adr += _MP_DATA_HEADER_POS;	  // ヘッダー１バイト読み飛ばす + Bitmapでーた + サイズデータ
	{
		u16 crc = GFL_STD_CrcCalc(adr, recvSize);
		OS_TPrintf("recieve crc = %x, size = %d\n", crc, recvSize);
		GF_ASSERT(adr2[_MP_CRCNO1] == (crc & 0xff));
		GF_ASSERT(adr2[_MP_CRCNO2] == ((crc & 0xff00) >> 8));
	}
	GFL_NET_RingPuts(&_pComm->recvRing , adr, recvSize);
}

//==============================================================================
/**
 * @brief   親機側で子機の通信を受信した時に呼ばれるコールバック MP用
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

static void _commRecvParentCallback(u16 aid, u16 *data, u16 size)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
	u8* adr = (u8*)data;
	int i,ringRestSize,ringRestSize2;
	int mcSize = _getUserMaxSendByte();
	RingBuffWork* pRing;

#if GFL_IRC_DEBUG
	{
		int i;
		u8 *buf;
		buf = (u8*)data;
		OS_TPrintf("aaa parent recieve buf print\n");
		for(i = 0; i < size; i++){
			OS_TPrintf("%d, ", buf[i]);
		}
		OS_TPrintf("\n");
	}
#endif

	_pComm->countSendRecvServer[aid]--;  //SERVER受信
	if(adr==NULL){
		//	GF_ASSERT_MSG(0,"callback");
		return;
	}
	if(adr[0] == _INVALID_HEADER){
		return;
	}

//#ifdef WIFI_DUMP_TEST
	if(aid == 1){
		DEBUG_DUMP(&adr[0], _getUserMaxSendByte(),"pr1");
	}
	else{
		DEBUG_DUMP(&adr[0], _getUserMaxSendByte(),"pr0");
	}
//#endif

	if((_pComm->bFirstCatch[aid]) && (adr[0] & _SEND_NEXT)){
		// まだ一回もデータをもらったことがない状態なのに連続データだった
		NET_PRINT("連続データ parent %d \n",aid);
		i = 0;
		DEBUG_DUMP(adr,12,"連続データ");
		return;
	}
	_pComm->bFirstCatch[aid] = FALSE;  //初回受信フラグをOFFする

	adr++;
	pRing = &_pComm->recvServerRing[aid];
	mcSize -= 1;
	GFL_NET_RingPuts(pRing , adr, mcSize);
}

//==============================================================================
/**
 * @brief   通信を送信したときに呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================




static void _sendCallbackFunc(BOOL result)
{
	if((result) && (_sendCallBack == _SEND_CB_FIRST_SEND)){
		_setSendCallBack( _SEND_CB_FIRST_SENDEND );
	}
	else{
		GF_ASSERT_MSG(0,"send failed");
	}
}

//==============================================================================
/**
 * @brief   通信を送信したときに呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

static void _sendServerCallback(BOOL result)
{
	int i;
	int mcSize = _getUserMaxSendByte();
	int machineMax = _getUserMaxNum();
	GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();

	if(result){
		_sendServerCallBack = _SEND_CB_FIRST_SENDEND;
		//		NET_PRINT("送信callback server側\n");
	}
	else{
		GF_ASSERT_MSG(0,"send failed");
	}

	for(i = 0; i < machineMax; i++){
		if(!pInit->bMPMode){			 // 初期化
			_pComm->sSendServerBuf[_MP_DATA_HEADER_POS+(i*mcSize)] = _INVALID_HEADER;
		}
	}
}

//==============================================================================
/**
 * @brief   子機側のアクションが行われたことを設定
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemActionCommandSet(void)
{
	//	_pComm->actionCount = _ACTION_COUNT_MOVE;
}

//==============================================================================
/**
 * @brief   送信キューにあったものを送信バッファに入れる サーバーMP通信用
 * @param   pSendBuff 入れる送信バッファ
 * @retval  none
 */
//==============================================================================

static void _setSendDataServer(u8* pSendBuff)
{
	int i;

	pSendBuff[0] = _MP_DATA_HEADER;

	if(_pComm->bNextSendDataServer != FALSE){
		pSendBuff[0] |= _SEND_NEXT;  // 一回で送れない場合
	}
	{
		u16 bitmap;
		int size;
		
		if((_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC
				|| (_GFL_NET_GetNETInitStruct())->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
			bitmap = 0x3;
		}
		else{
			bitmap = GFL_NET_WL_GetBitmap();
		}
		
		pSendBuff[_MP_CONNECTBIT] = bitmap;

		{
			SEND_BUFF_DATA buffData;
			buffData.size = _getUserMaxSendByteParent() - _MP_DATA_HEADER_POS;
			buffData.pData = &pSendBuff[_MP_DATA_HEADER_POS];
			if(GFL_NET_QueueGetData(&_pComm->sendQueueMgrServer, &buffData)){
				_pComm->bNextSendDataServer = FALSE;
				size = (_getUserMaxSendByteParent() - _MP_DATA_HEADER_POS) - buffData.size;
			}
			else{
				_pComm->bNextSendDataServer = TRUE;
				size = _getUserMaxSendByteParent() - _MP_DATA_HEADER_POS;
			}
			pSendBuff[_MP_SIZEO1] = (size >> 8) & 0xff;
			pSendBuff[_MP_SIZEO2] = size & 0xff;
			GFL_NET_QueueInc(&_pComm->sendQueueMgr, &pSendBuff[_MP_INCNO]);
			{
				u16 crc = GFL_STD_CrcCalc(&pSendBuff[_MP_DATA_HEADER_POS], size);
				pSendBuff[_MP_CRCNO1] = crc & 0xff;
				pSendBuff[_MP_CRCNO2] = (crc & 0xff00) >> 8;
			}
		}
	}
}


//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _recvDataFunc(void)
{
	int id = GFL_NET_NETID_SERVER;
	int size;
	u8 command;
	int bkPos;

	if(!_pComm){
		return;
	}

	GFL_NET_RingEndChange(&_pComm->recvRing);
	if(GFL_NET_RingDataSize(&_pComm->recvRing) > 0){
		_recvDataFuncSingle(&_pComm->recvRing, id, _pComm->pTmpBuff, &_pComm->recvCommClient);
	}
}

//==============================================================================
/**
 * @brief   通信可能状態の人数を返す
 * @param   none
 * @retval  接続人数
 */
//==============================================================================
int GFL_NET_SystemGetConnectNum(void)
{
	int num = 0,i;

	for(i = 0; i < GFL_NET_MACHINE_MAX; i++){
		if(GFL_NET_SystemIsConnect(i)){
			num++;
		}
	}
	return num;
}

//==============================================================================
/**
 * @brief   初期化しているかどうかを返す
 * @param   none
 * @retval  初期が終わっていたらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemIsInitialize(void)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

	if(_pComm){
		if( pNetIni->bNetType == GFL_NET_TYPE_WIFI || pNetIni->bNetType == GFL_NET_TYPE_IRC || pNetIni->bNetType == GFL_NET_TYPE_IRC_WIRELESS){
			return TRUE;
		}
	}
	return GFL_NET_WLIsInitialize();
}


//==============================================================================
/**
 * @brief   WHライブラリで　通信状態のBITを確認
 * @param   none
 * @retval  接続がわかるBIT配列
 */
//==============================================================================

BOOL GFL_NET_SystemIsChildsConnecting(void)
{
	return GFL_NET_WLIsChildsConnecting();
}

//==============================================================================
/**
 * @brief   エラー状態かどうか
 * @param   none
 * @retval  エラーの時TRUE
 */
//==============================================================================

BOOL GFL_NET_SystemIsError(void)
{
	if(GFL_NET_SystemGetAloneMode()){  // 一人モードの場合ここはエラーにしない
		return FALSE;
	}
	if(_pComm){
		if(_pComm->bError){
			//			CommStateSetErrorCheck(TRUE,TRUE);
			return TRUE;
		}
	}
	return GFL_NET_WLIsError();
}

//==============================================================================
/**
 * @brief   一人通信モードを設定
 * @param   bAlone	一人通信モード
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetAloneMode(BOOL bAlone)
{
	if(_pComm){
		_pComm->bAlone = bAlone;
	}
}

//==============================================================================
/**
 * @brief   一人通信モードかどうかを取得
 * @param   none
 * @retval  一人通信モードの場合TRUE
 */
//==============================================================================

BOOL GFL_NET_SystemGetAloneMode(void)
{
	if(_pComm){
		return _pComm->bAlone;
	}
	return FALSE;
}

//==============================================================================
/**
 * @brief   自動終了コマンド受信
 * @param   netID  通信ID
 * @param   size   サイズ
 * @param   pData  データ
 * @param   pWork  ワーク
 * @return  none
 */
//==============================================================================

void GFL_NET_SystemRecvAutoExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	NET_PRINT("CommRecvAutoExit 受信 \n");
	if(!GFL_NET_WLIsAutoExit()){
		if(GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE){   // 自分が親の場合みんなに逆返信する
			GFL_NET_SendData(pNetHandle,GFL_NET_CMD_EXIT_REQ, 0, NULL);
		}
	}
	GFL_NET_WLSetAutoExit();
}


//==============================================================================
/**
 * @brief   キューが空っぽかどうか サーバー側
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

BOOL GFL_NET_SystemIsEmptyQueue_ServerSize(void)
{
	return GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgrServer);

}

//==============================================================================
/**
 * @brief   キューが空っぽかどうか
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

BOOL GFL_NET_SystemIsEmptyQueue(void)
{
	return GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgr);

}

//==============================================================================
/**
 * @brief   wifi接続したかどうかを設定する
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

void GFL_NET_SystemSetWifiConnect(BOOL bConnect)
{
	// 通信待機状態にあるかどうかを伝える
	_pComm->bWifiConnect = bConnect;
}

//==============================================================================
/**
 * @brief   VCHAT状態かどうか
 * @param   none
 * @retval  VCHAT状態ならTRUE
 */
//==============================================================================

BOOL GFL_NET_SystemIsVChat(void)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

#if GFL_NET_WIFI
	if( pNetIni->bNetType == GFL_NET_TYPE_WIFI ){
		return mydwc_IsVChat();
	}
#endif
	return FALSE;
}

//==============================================================================
/**
 * @brief   WIFI通信を同期通信するか、非同期通信するかの切り替えを行う
 * @param   TRUE 同期 FALSE 非同期
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetWifiBothNet(BOOL bFlg)
{
	GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

#if GFL_NET_WIFI
	if( pNetIni->bNetType == GFL_NET_TYPE_WIFI ){
		_pComm->bWifiSendRecv = bFlg;
		if(bFlg){
			_pComm->countSendRecv = 0;
			_pComm->countSendRecvServer[0] = 0;
			_pComm->countSendRecvServer[1] = 0;
		}
		NET_PRINT("oo同期切り替え %d\n",bFlg);
	}
#endif
}


//==============================================================================
/**
 * @brief   通信を止める
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemShutdown(void)
{
	if(_pComm){
		_pComm->bShutDown = TRUE;
	}
}

//==============================================================================
/**
 * @brief   サーバー側の通信キューをリセットする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetQueue_Server(void)
{
	GFL_NET_QueueManagerReset(&_pComm->sendQueueMgrServer);
}

//==============================================================================
/**
 * @brief   データシェアリングが成功したかどうか
 * @retval  TRUE  データシェアリング成功
 * @retval  FALSE データシェアリング失敗
 */
//==============================================================================

BOOL GFL_NET_SystemCheckDataSharing(void)
{
	if(_pComm){
		GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();
		if(!pInit->bMPMode && (pInit->bNetType == GFL_NET_TYPE_WIRELESS) ){
			return _pComm->bDSSendOK;
		}
	}
	return TRUE;
}


#endif