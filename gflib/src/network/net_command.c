//=============================================================================
/**
 * @file	net_command.c
 * @brief	データを送るためのコマンドをテーブル化しています ここはシステム部
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#include "gflib.h"
#include "net.h"
#include "net_def.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "tool/net_tool.h"

//==============================================================================
//  static定義
//==============================================================================
static void _commCommandRecvThrowOut(const int netID, const int size, const void* pData, void* pWork,GFL_NETHANDLE* pNetHandle);
static void _commCommandRecvThrowOutReq(const int netID, const int size, const void* pData, void* pWork,GFL_NETHANDLE* pNetHandle);
static void _commCommandRecvThrowOutEnd(const int netID, const int size, const void* pData, void* pWork,GFL_NETHANDLE* pNetHandle);


//==============================================================================
//	テーブル宣言
//  comm_shar.h の enum と同じならびにしてください
//  CALLBACKを呼ばれたくない場合はNULLを書いてください
//  コマンドのサイズを返す関数を書いてもらえると通信が軽くなります
//  _getZeroはサイズなしを返します。_getVariableは可変データ使用時に使います
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
    {NULL,                        GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {CommRecvExit,                GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {GFL_NET_SystemRecvAutoExit,            GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {CommRecvNegotiation,         CommRecvGetNegotiationSize, NULL},
    {CommRecvNegotiationReturn,   CommRecvGetNegotiationSize, NULL},
    {GFL_NET_SystemRecvDSMPChange,          GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_SystemRecvDSMPChangeReq,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_SystemRecvDSMPChangeEnd,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {_commCommandRecvThrowOut,    GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {_commCommandRecvThrowOutReq, GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {_commCommandRecvThrowOutEnd, GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {GFL_NET_ToolRecvTimingSync,          GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_ToolRecvTimingSyncEnd,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
};

typedef struct{
    const NetRecvFuncTable* pCommPacket;  ///< fieldやbattleのコマンドのテーブル
    int listNum;                       ///< _pCommPacketのlist数
    void* pWork;                       ///< fieldやbattleのメインになるワーク
    u8 bThrowOutReq[GFL_NET_MACHINE_MAX];        ///< コマンドを交換したいフラグ
    u8 bThrowOuted;    ///< コマンド入れ替えが終了したらTRUE
} _COMM_COMMAND_WORK;

static _COMM_COMMAND_WORK* _pCommandWork = NULL;


//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの初期化
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系
 * @param   listNum          コマンド数
 * @param   pWork            呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------

void GFL_NET_CommandInitialize(const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork)
{
    int i;
    
    if(!_pCommandWork){
        _pCommandWork = GFL_HEAP_AllocMemory(GFL_HEAPID_SYSTEM, sizeof(_COMM_COMMAND_WORK));
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

void GFL_NET_CommandFinalize( void )
{
    if(_pCommandWork){
        GFL_HEAP_FreeMemory(_pCommandWork);
        _pCommandWork = NULL;
    }
}

//--------------------------------------------------------------
/**
 * @brief   テーブルに従い 受信コールバックを呼び出します
 * @param   command         受信コマンド
 * @param   netID           ネットワークID
 * @param   size            受信データサイズ
 * @param   pData           受信データ
 * @retval  none
 */
//--------------------------------------------------------------

void GFL_NET_CommandCallBack(int netID, int command, int size, void* pData)
{
    PTRCommRecvFunc func;

    if( command < GFL_NET_CMD_COMMAND_MAX ){
        func = _CommPacketTbl[command].callbackFunc;
    }
    else{
        if((_pCommandWork==NULL) || (command > (_pCommandWork->listNum + GFL_NET_CMD_COMMAND_MAX))){
            OHNO_PRINT("command %d \n", command);
            OS_Panic("no command");
            GFL_NET_SystemSetError();
            return;  // 本番ではコマンドなし扱い
        }
        func = _pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].callbackFunc;
    }
    if(func != NULL){
        if(_pCommandWork){
            func(netID, size, pData, _pCommandWork->pWork, GFL_NET_SystemGetHandle(netID));
        }
        else{
            func(netID, size, pData, NULL, GFL_NET_SystemGetHandle(netID));
        }
    }
}


//--------------------------------------------------------------
/**
 * @brief   定義があったコマンドのサイズを返します
 * @param   command         コマンド
 * @retval  データのサイズ   可変なら COMM_VARIABLE_SIZEを返します
 */
//--------------------------------------------------------------

int GFL_NET_CommandGetPacketSize(int command)
{
    int size = 0;
    PTRCommRecvSizeFunc func=NULL;

    if( command < GFL_NET_CMD_COMMAND_MAX ){
        func = _CommPacketTbl[command].getSizeFunc;
    }
    else{
        if((_pCommandWork==NULL) || (command > (_pCommandWork->listNum + GFL_NET_CMD_COMMAND_MAX))){
            OHNO_PRINT("command %d \n", command);
            OS_Panic("no command");
            GFL_NET_SystemSetError();
        }
        else{
            func = _pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].getSizeFunc;
        }
    }
    if(func != NULL){
        size = func();
    }
    return size;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファを持っているかどうかの検査
 * @param   command         コマンド
 * @retval  持ってるならTRUE
 */
//--------------------------------------------------------------

BOOL GFL_NET_CommandCreateBuffCheck(int command)
{
    if( command < GFL_NET_CMD_COMMAND_MAX ){
        return ( _CommPacketTbl[command].getAddrFunc != NULL);
    }
    return (_pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].getAddrFunc != NULL);
}

//--------------------------------------------------------------
/**
 * @brief   
 * @param   command         コマンド
 * @retval  持ってるならTRUE
 */
//--------------------------------------------------------------

void* GFL_NET_CommandCreateBuffStart(int command,int netID, int size)
{
    PTRCommRecvBuffAddr func;

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

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの廃棄
 * @param   none
 * @retval  受付完了したらTRUE
 */
//--------------------------------------------------------------

BOOL GFL_NET_CommandThrowOut(void)
{
    if(_pCommandWork==NULL){
        OS_TPanic("no init");
    }
    _pCommandWork->bThrowOuted = FALSE;
    return GFL_NET_SystemSendFixData(GFL_NET_CMD_THROWOUT);
}


//==============================================================================
/**
 * コマンド廃棄コールバック
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandRecvThrowOut(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
        return;
    }
//    OHNO_PRINT("CommRecvDSMPChange 受信\n");
    // 全員に切り替え信号を送る
    _pCommandWork->bThrowOutReq[netID] = TRUE;
    for(i = 0 ; i < GFL_NET_MACHINE_MAX; i++){
        if(!GFL_NET_SystemIsConnect(i)){
            continue;
        }
        if(!_pCommandWork->bThrowOutReq[i]){
            return;
        }
    }
    GFL_NET_SystemSendData_ServerSide(GFL_NET_CMD_THROWOUT_REQ, NULL, 0);
}

//==============================================================================
/**
 * コマンド廃棄実行コールバック
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandRecvThrowOutReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    _pCommandWork->pCommPacket = NULL;
    _pCommandWork->listNum = 0;
    _pCommandWork->pWork = NULL;
    _pCommandWork->bThrowOuted = TRUE;

    GFL_NET_SystemSendFixSizeData(GFL_NET_CMD_THROWOUT_END,pData);
}

//==============================================================================
/**
 * コマンド廃棄完了コールバック
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandRecvThrowOutEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
        return;
    }
    _pCommandWork->bThrowOutReq[netID] = FALSE;
}

//==============================================================================
/**
 * コマンド交換できたかどうかを確認する
 * @retval  TRUE 交換完了
 * @retval  FALSE 完了していない
 */
//==============================================================================

BOOL GFL_NET_CommandIsThrowOuted(void)
{
    if(_pCommandWork){
        return _pCommandWork->bThrowOuted;
    }
    return FALSE;
}

