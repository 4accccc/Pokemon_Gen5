//=============================================================================
/**
 * @file	net_command.c
 * @brief	データを送るためのコマンドをテーブル化しています ここはシステム部
 * @author	Katsumi Ohno
 * @date	2005.07.26
 */
//=============================================================================

#include "net_def.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "net_handle.h"

#define RecvTestFunc(val) RecvTest ## val

//==============================================================================
//  static定義
//==============================================================================

static void RecvTestB(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestC(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestD(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestE(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestF(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestG(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void RecvTestH(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

static void RecvTestB(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc B\n" ); }
static void RecvTestC(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc C\n" ); }
static void RecvTestD(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc D\n" ); }
static void RecvTestE(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc E\n" ); }
static void RecvTestF(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc F\n" ); }
static void RecvTestG(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc G\n" ); }
static void RecvTestH(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{ NET_PRINT( "CallFunc H\n" ); }


//==============================================================================
//	テーブル宣言
//  enum と同じならびにしてください
//  CALLBACKを呼ばれたくない場合はNULLを書いてください
//  コマンドのサイズを返す関数を書いてもらうか
//  固定バイトならば GFL_NET_COMMAND_SIZE(size) マクロを書いてください
//  GFL_NET_COMMAND_VARIABLE は可変データ使用時に使います
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
	{NULL,									NULL},	//ここは元からNULL
//	{GFL_NET_StateRecvExit,					NULL},
	{RecvTestFunc(B)				,					NULL},
//	{GFL_NET_StateRecvExitStart,			NULL},
	{RecvTestFunc(C),									NULL},
	{GFL_NET_HANDLE_RecvNegotiation,		NULL},
	{GFL_NET_HANDLE_RecvNegotiationReturn,  NULL},
//	{GFL_NET_StateRecvDSMPChange,			NULL},
	{RecvTestFunc(F)						,			NULL},
//	{GFL_NET_HANDLE_RecvTimingSync,			NULL},
	{RecvTestFunc(G),									NULL},
//	{GFL_NET_HANDLE_RecvTimingSyncEnd,		NULL},
	{RecvTestFunc(H),									NULL},
	//メニュー用
	//汎用送信用
};

typedef struct{
	const NetRecvFuncTable* pCommPacket;  ///< fieldやbattleのコマンドのテーブル
	int listNum;					   ///< _pCommPacketのlist数
	void* pWork;					   ///< fieldやbattleのメインになるワーク
	u8 bThrowOutReq[GFL_NET_MACHINE_MAX];		///< コマンドを交換したいフラグ
	u8 bThrowOuted;	///< コマンド入れ替えが終了したらTRUE
} _COMM_COMMAND_WORK;

static _COMM_COMMAND_WORK* _pCommandWork = NULL;


//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの初期化
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系
 * @param   listNum		  コマンド数
 * @param   pWork			呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------

void GFL_NET_COMMAND_Init(const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork)
{
	int i;
	
	if(!_pCommandWork){
		_pCommandWork = (_COMM_COMMAND_WORK*)GFL_HEAP_AllocClearMemory(GFL_HEAPID_SYSTEM, sizeof(_COMM_COMMAND_WORK));
	}
	_pCommandWork->pCommPacket = pCommPacketLocal;
	_pCommandWork->listNum = listNum;
	_pCommandWork->pWork = pWork;


	for(i = 0 ; i < GFL_NET_MACHINE_MAX; i++){
		_pCommandWork->bThrowOutReq[i] = FALSE;
	}
	_pCommandWork->bThrowOuted = FALSE;


}

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの終了処理
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void GFL_NET_COMMAND_Exit( void )
{
	if(_pCommandWork){
		GFL_HEAP_FreeMemory(_pCommandWork);
		_pCommandWork = NULL;
	}
}

//--------------------------------------------------------------
/**
 * @brief   コマンドを受け取る人かどうか
 * @param   recvID  受け取り先ID
 * @retval  受け取る場合TRUE
 */
//--------------------------------------------------------------

BOOL GFI_NET_COMMAND_RecvCheck(int recvID)
{
	if(recvID == GFL_NET_SENDID_ALLUSER){
		return TRUE;
	}
	else if( GFL_NET_SystemGetCurrentID() == recvID ){
		return TRUE;
	}
	else if((GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE) && (recvID==GFL_NET_NETID_SERVER) ){  // 受信者が違う場合return
		return TRUE;
	}
	return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief   テーブルに従い 受信コールバックを呼び出します
 * @param   command		 受信コマンド
 * @param   netID		   ネットワークID
 * @param   size			受信データサイズ
 * @param   pData		   受信データ
 * @retval  none
 */
//--------------------------------------------------------------

void GFI_NET_COMMAND_CallBack(int netID, int recvID, int command, int size, void* pData,GFL_NETHANDLE* pNetHandle)
{
	int i;
	PTRCommRecvFunc func;
	BOOL bCheck;

	if( GFI_NET_COMMAND_RecvCheck(recvID) == FALSE){
		return;
	}
	bCheck=TRUE;  // ネゴシエーションコマンド以外解析しない
	if(!GFL_NET_HANDLE_IsNegotiation(pNetHandle)){  // ネゴシエーションがすんでない場合
		if( command >= GFL_NET_CMD_COMMAND_MAX ){
			bCheck=FALSE;
		}
	}
	if(bCheck){
		if( command < GFL_NET_CMD_COMMAND_MAX ){
			func = _CommPacketTbl[command].callbackFunc;
		}
		else{
			if((_pCommandWork==NULL) || (command > (_pCommandWork->listNum + GFL_NET_CMD_COMMAND_MAX))){
				NET_PRINT("command %d \n", command);
				OS_Panic("no command");
				GFL_NET_SystemSetError();
				return;  // 本番ではコマンドなし扱い
			}
			func = _pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].callbackFunc;
		}
		if(func != NULL){
			NET_PRINT("command %d recvID %d\n", command, recvID);
			if(_pCommandWork){
				func(netID, size, pData, _pCommandWork->pWork, pNetHandle);
			}
			else{
				func(netID, size, pData, NULL, pNetHandle);
			}
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファを持っているかどうかの検査
 * @param   command		 コマンド
 * @retval  持ってるならTRUE
 */
//--------------------------------------------------------------

BOOL GFI_NET_COMMAND_CreateBuffCheck(int command)
{
	if( command < GFL_NET_CMD_COMMAND_MAX ){
		return ( _CommPacketTbl[command].getAddrFunc != NULL);
	}
	return (_pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].getAddrFunc != NULL);
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファを得る
 * @param   command		 コマンド
 * @param   netID		   送信者
 * @param   size		 サイズ
 * @retval  受信バッファ 持っていないならばNULL
 */
//--------------------------------------------------------------

void* GFI_NET_COMMAND_CreateBuffStart(int command,int netID, int size)
{
	PTRCommRecvBuffAddr func;

	NET_PRINT("書き込みバッファ取得\n");

	if( command < GFL_NET_CMD_COMMAND_MAX ){
		func = _CommPacketTbl[command].getAddrFunc;
		return func(netID, NULL, size);
	}
	else{
		func = _pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].getAddrFunc;
		return func(netID, _pCommandWork->pWork, size);
	}
	return NULL;
}

