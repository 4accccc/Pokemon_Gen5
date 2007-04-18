//=============================================================================
/**
 * @file	net_command.c
 * @brief	データを送るためのコマンドをテーブル化しています ここはシステム部
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#include "gflib.h"

#include "net_def.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "tool/net_tool.h"

//==============================================================================
//  static定義
//==============================================================================


//==============================================================================
//	テーブル宣言
//  comm_shar.h の enum と同じならびにしてください
//  CALLBACKを呼ばれたくない場合はNULLを書いてください
//  コマンドのサイズを返す関数を書いてもらうか、固定バイトならば
//  GFL_NET_COMMAND_SIZE(size) マクロを書いてもらうと、
//  サイズを通信で送らないので少し軽くなります
//  GFL_NET_COMMAND_VARIABLE は可変データ使用時に使います
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
    {NULL,                        GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {GFL_NET_StateRecvExit,                GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {GFL_NET_StateRecvExitStart,            GFL_NET_COMMAND_SIZE( 0 ), NULL},
    {GFL_NET_StateRecvNegotiation,         GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_StateRecvNegotiationReturn,   GFL_NET_COMMAND_SIZE( 2 ), NULL},
    {GFL_NET_StateRecvDSMPChange,          GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_StateRecvDSMPChangeReq,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_StateRecvDSMPChangeEnd,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_TOOL_RecvTimingSync,          GFL_NET_COMMAND_SIZE( 1 ), NULL},
    {GFL_NET_TOOL_RecvTimingSyncEnd,       GFL_NET_COMMAND_SIZE( 1 ), NULL},
#if GFL_NET_WIFI
    {GFL_NET_StateRecvWifiExit,            GFL_NET_COMMAND_SIZE( 1 ), NULL},
#endif
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

void GFL_NET_COMMAND_Init(const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork)
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

void GFL_NET_COMMAND_Exit( void )
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

void GFI_NET_COMMAND_CallBack(int netID, int recvID, int command, int size, void* pData)
{
    int i;
    PTRCommRecvFunc func;
    BOOL bCheck;
    GFL_NETHANDLE* pNetHandle;

    NET_PRINT("--- %d \n", netID);

    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        pNetHandle = GFL_NET_GetNetHandle(i);
        if(!pNetHandle){
            continue;
        }
        if((recvID != GFL_NET_SENDID_ALLUSER) && (i != recvID)){  // 受信者が違う場合continue
            continue;
        }
        bCheck=TRUE;  // ネゴシエーションコマンド以外解析しない
        if(!GFL_NET_IsNegotiation(pNetHandle)){  // ネゴシエーションがすんでない場合
            if(!((command == GFL_NET_CMD_NEGOTIATION) || (command == GFL_NET_CMD_NEGOTIATION_RETURN))){
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
                NET_PRINT("command %d recvID %d\n", command,i);
                if(_pCommandWork){
                    func(netID, size, pData, _pCommandWork->pWork, pNetHandle);
                }
                else{
                    func(netID, size, pData, NULL, pNetHandle);
                }
            }
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

int GFI_NET_COMMAND_GetPacketSize(int command)
{
    int size = 0;
    PTRCommRecvSizeFunc func=NULL;
    u32 num;

    if( command < GFL_NET_CMD_COMMAND_MAX ){
        func = _CommPacketTbl[command].getSizeFunc;
    }
    else{
        if((_pCommandWork==NULL) || (command > (_pCommandWork->listNum + GFL_NET_CMD_COMMAND_MAX))){
            NET_PRINT("command %d \n", command);
            OS_Panic("no command");
            GFL_NET_SystemSetError();
        }
        else{
            func = _pCommandWork->pCommPacket[command - GFL_NET_CMD_COMMAND_MAX].getSizeFunc;
        }
    }
    if(func != NULL){
        num = (int)func;
        if((_GFL_NET_COMMAND_SIZE_H & num) == _GFL_NET_COMMAND_SIZE_H){
            size = num - _GFL_NET_COMMAND_SIZE_H;
        }
        else{
            size = func();
        }
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
 * @param   command         コマンド
 * @param   netID           送信者
 * @param   size         サイズ
 * @retval  受信バッファ 持っていないならばNULL
 */
//--------------------------------------------------------------

void* GFI_NET_COMMAND_CreateBuffStart(int command,int netID, int size)
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

