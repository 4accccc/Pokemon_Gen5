//=============================================================================
/**
 * @file	net_command.c
 * @brief	データを送るためのコマンドをテーブル化しています ここはシステム部
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================


#include "net_def.h"
#include "net_system.h"
#include "net_command.h"
#include "net_handle.h"

//==============================================================================
//  static定義
//==============================================================================

#define _COMM_TBL_MAX (4)  // 積み重ねるテーブル

//==============================================================================
//	テーブル宣言
//  enum と同じならびにしてください
//  CALLBACKを呼ばれたくない場合はNULLを書いてください
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
    {NULL,                                   NULL},
    {NULL,NULL},
    {NULL,NULL},
    {GFL_NET_HANDLE_RecvNegotiation,         NULL},
    {GFL_NET_HANDLE_RecvNegotiationReturn,   NULL},
    {NULL,NULL},
    {GFL_NET_HANDLE_RecvTimingSync,          NULL},
    {GFL_NET_HANDLE_RecvTimingSyncEnd,       NULL},
    //メニュー用
    //汎用送信用
#if GFL_NET_WIFI
    {GFL_NET_StateRecvWifiExit,              NULL},
#endif

};
/*
static const NetRecvFuncTable _CommPacketTbl[] = {
    {NULL,                                   NULL},
    {GFL_NET_StateRecvExit,                  NULL},
    {GFL_NET_StateRecvExitStart,             NULL},
    {GFL_NET_HANDLE_RecvNegotiation,         NULL},
    {GFL_NET_HANDLE_RecvNegotiationReturn,   NULL},
    {GFL_NET_StateRecvDSMPChange,            NULL},
    {GFL_NET_HANDLE_RecvTimingSync,          NULL},
    {GFL_NET_HANDLE_RecvTimingSyncEnd,       NULL},
    //メニュー用
    //汎用送信用
#if GFL_NET_WIFI
    {GFL_NET_StateRecvWifiExit,              NULL},
#endif

};
*/
typedef struct{
    const NetRecvFuncTable* pCommPacket;  ///< fieldやbattleのコマンドのテーブル
    int listNum;                       ///< _pCommPacketのlist数
    void* pWork;                       ///< fieldやbattleのメインになるワーク
    int NetworkCommandHeaderNo;      ///< テーブル種別番号
} _NET_COMMAND_TABLE;


static _NET_COMMAND_TABLE _BaseTable = {
    _CommPacketTbl,
    GFL_NET_CMD_COMMAND_MAX,
    NULL,
    0,
};


typedef struct{
    _NET_COMMAND_TABLE commTbl[_COMM_TBL_MAX];
    u8 bThrowOutReq[GFL_NET_MACHINE_MAX];        ///< コマンドを交換したいフラグ
    u8 bThrowOuted;    ///< コマンド入れ替えが終了したらTRUE
} _COMM_COMMAND_WORK;

static _COMM_COMMAND_WORK* _pCommandWork = NULL;


//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの初期化
 * @param   cokind          コマンド体系番号 NetStartCommand_e
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系
 * @param   listNum          コマンド数
 * @param   pWork            呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------

void GFL_NET_COMMAND_Init(const int cokind,const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork)
{
    int i;
    
    if(!_pCommandWork){
        _pCommandWork = (_COMM_COMMAND_WORK*)GFL_HEAP_AllocClearMemory(GFL_HEAPID_SYSTEM, sizeof(_COMM_COMMAND_WORK));
    }

    GFL_NET_AddCommandTable(cokind, pCommPacketLocal, listNum, pWork);
    _BaseTable.pWork = pWork;

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
        GFL_STD_MemClear(_pCommandWork, sizeof(_COMM_COMMAND_WORK));
        GFL_HEAP_FreeMemory(_pCommandWork);
        _pCommandWork = NULL;
    }
}

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの追加
 * @param   cokind          コマンド体系番号 NetStartCommand_e
 * @param   pCommPacketLocal 呼び出しモジュール専用のコマンド体系テーブル
 * @param   listNum          コマンド数
 * @param   pWork            呼び出しモジュール専用のワークエリア
 * @retval  none
 */
//--------------------------------------------------------------
void GFL_NET_AddCommandTable(const int cokind,const NetRecvFuncTable* pCommPacketLocal,const int listNum,void* pWork)
{
    int i;

    for(i = 0 ; i < _COMM_TBL_MAX ; i++){
        if(_pCommandWork->commTbl[i].NetworkCommandHeaderNo == 0){
            _pCommandWork->commTbl[i].NetworkCommandHeaderNo = cokind;
            _pCommandWork->commTbl[i].pCommPacket = pCommPacketLocal;
            _pCommandWork->commTbl[i].listNum = listNum;
            _pCommandWork->commTbl[i].pWork = pWork;
            break;
        }
    }
}

//--------------------------------------------------------------
/**
 * @brief   コマンドテーブルの削除
 * @param   cokind          コマンド体系番号 NetStartCommand_e
 * @retval  none
 */
//--------------------------------------------------------------
void GFL_NET_DelCommandTable(const int cokind)
{
    int i;

    for(i = 0 ; i < _COMM_TBL_MAX ; i++){
        if(_pCommandWork->commTbl[i].NetworkCommandHeaderNo == cokind){
            GFL_STD_MemClear(&_pCommandWork->commTbl[i], sizeof(_NET_COMMAND_TABLE));
            break;
        }
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
 * @brief   コマンドテーブルをかえす
 * @param   command          コマンド
 * @retval  _NET_COMMAND_TABLE
 */
//--------------------------------------------------------------
static _NET_COMMAND_TABLE* _getCommandTable(int command)
{
    int i;
    int cokind = command&0xff00;
    int co = command&0xff;

    if(cokind==0){
        return &_BaseTable;
    }
    for(i = 0 ; i < _COMM_TBL_MAX ; i++){
        if(_pCommandWork->commTbl[i].NetworkCommandHeaderNo == cokind){
            NET_PRINT("_NET_COMMAND_TABLE %d %d\n",i,co);
            GF_ASSERT(_pCommandWork->commTbl[i].listNum > co);
            return &_pCommandWork->commTbl[i];
        }
    }
    GF_ASSERT(0);
    return NULL;
}

//--------------------------------------------------------------
/**
 * @brief   コマンド関数を返す
 * @param   command          コマンド
 * @retval  _NET_COMMAND_TABLE
 */
//--------------------------------------------------------------
static void* _getCommandFunc(int command)
{
    _NET_COMMAND_TABLE* pTbl = _getCommandTable(command);

    if(pTbl){
        return pTbl->pCommPacket[(command&0xff)].callbackFunc;
    }
    return NULL;
}

//--------------------------------------------------------------
/**
 * @brief   コマンドworkを返す
 * @param   command          コマンド
 * @retval  _NET_COMMAND_TABLE
 */
//--------------------------------------------------------------
static void* _getCommandWork(int command)
{
    _NET_COMMAND_TABLE* pTbl = _getCommandTable(command);

    if(pTbl){
        return pTbl->pWork;
    }
    return NULL;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファを得る関数を返す
 * @param   command          コマンド
 * @retval  _NET_COMMAND_TABLE
 */
//--------------------------------------------------------------
static void* _getCommandRecvAddrFunc(int command)
{
    _NET_COMMAND_TABLE* pTbl = _getCommandTable(command);

    if(pTbl){
        return pTbl->pCommPacket[(command&0xff)].getAddrFunc;
    }
    return NULL;
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
        func = (PTRCommRecvFunc)_getCommandFunc(command);
        if(func==NULL){
            GFL_NET_SystemSetError();
            return;  // 本番ではコマンドなし扱い
        }
        NET_PRINT("command %d recvID %d\n", command, recvID);
        func(netID, size, pData, _getCommandWork(command), pNetHandle);
    }
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
    return ( _getCommandRecvAddrFunc(command) != NULL);
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

    NET_PRINT("書き込みバッファ取得\n");
    func = (PTRCommRecvBuffAddr)_getCommandRecvAddrFunc(command);
    if( func ){
        return func(netID, _getCommandWork(command), size);
    }
    return NULL;
}

