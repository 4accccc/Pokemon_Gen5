//==============================================================================
/**
 * @file	compati_comm.c
 * @brief	相性チェック：通信
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#include <gflib.h>
#include "system\main.h"
#include "net\network_define.h"

#include "compati_types.h"
#include "compati_tool.h"
#include "compati_comm.h"


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void _initCallBack(void* pWork);
static void _CommSystemEndCallback(void* pWork);
static void _connectCallBack(void* pWork, int netID);
static void _endCallBack(void* pWork);
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvResultData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

//==============================================================================
//	データ
//==============================================================================
///通信コマンドテーブル
static const NetRecvFuncTable _CompatiCommPacketTbl[] = {
    {_RecvFirstData,         NULL},    ///NET_CMD_FIRST
    {_RecvResultData,          NULL},  ///NET_CMD_RESULT
};

enum{
	NET_CMD_FIRST = GFL_NET_CMD_COMPATI_CHECK,
	NET_CMD_RESULT,
};

#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (32)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数


static const GFLNetInitializeStruct aGFLNetInit = {
    _CompatiCommPacketTbl,  // 受信関数テーブル
    NELEMS(_CompatiCommPacketTbl), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    _connectCallBack,    ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,            // 普通のエラーが起こった場合 通信終了
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    _endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x532,//0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_COMPATI_CHECK,  //GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};


//--------------------------------------------------------------
/**
 * @brief   通信初期化
 * @param   commsys		
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_Init(COMPATI_CONNECT_SYS *commsys, u32 irc_timeout)
{
	switch(commsys->seq){
	case 0:
		{
			GFLNetInitializeStruct net_ini = aGFLNetInit;
			
			net_ini.irc_timeout = irc_timeout;
			if(irc_timeout == IRC_TIMEOUT_STANDARD){
//				net_ini.bNetType = GFL_NET_TYPE_IRC;
			}
			GFL_NET_Init(&net_ini, _initCallBack, commsys);	//通信初期化
		}
		commsys->seq++;
		break;
	case 1:
		if(GFL_NET_IsInit() == TRUE){	//初期化終了待ち
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   通信接続
 * @param   commsys		
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_Connect(COMPATI_CONNECT_SYS *commsys)
{
	switch(commsys->seq){
	case 0:
		GFL_NET_ChangeoverConnect(NULL); // 自動接続
		commsys->seq++;
		break;
	case 1:
		//自動接続待ち
		if(commsys->connect_ok == TRUE){
			OS_TPrintf("接続した\n");
			commsys->seq++;
		}
		break;
	case 2:		//タイミングコマンド発行
		GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15, WB_NET_COMPATI_CHECK);
		commsys->seq++;
		break;
	case 3:
		if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),15, WB_NET_COMPATI_CHECK) == TRUE){
			OS_TPrintf("タイミング取り成功\n");
			OS_TPrintf("接続人数 = %d\n", GFL_NET_GetConnectNum());
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   通信切断
 * @param   commsys		
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_Shoutdown(COMPATI_CONNECT_SYS *commsys)
{
	switch(commsys->seq){
	case 0:	//通信終了
		if(GFL_NET_IsParentMachine() == TRUE){
			OS_TPrintf("親機：GFL_NET_CMD_EXIT_REQ送信\n");
			if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL)){
				commsys->seq++;
			}
		}
		else{
			commsys->seq++;
		}
		break;
	case 1:	//通信終了待ち
		if(commsys->connect_ok == FALSE){
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   通信システム終了
 *
 * @param   commsys		
 *
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_Exit(COMPATI_CONNECT_SYS *commsys)
{
#if 0
	return TRUE;
#else
	switch(commsys->seq){
	case 0:
		if(GFL_NET_Exit(_CommSystemEndCallback) == TRUE){
			commsys->seq++;
		}
		else{
			return TRUE;
		}
		break;
	case 1:
		if(commsys->lib_finish == TRUE){
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
#endif
}

//--------------------------------------------------------------
/**
 * @brief   通信エラーチェック
 *
 * @param   commsys		
 *
 * @retval  TRUE:通信エラー発生中。FALSEが戻ってくるまで他のCompatiComm_???を呼んではいけない
 * 			FALSE:エラーなし
 */
//--------------------------------------------------------------
BOOL CompatiComm_ErrorCheck(COMPATI_CONNECT_SYS *commsys)
{
	switch(commsys->err_seq){
	case 0:
		if(GFL_NET_IsInit() == TRUE && GFL_NET_SystemIsError() != 0){
			OS_TPrintf("通信エラーが発生\n");
			if(GFL_NET_Exit(_CommSystemEndCallback) == TRUE){
				commsys->err_seq++;
			}
			else{
				commsys->err_seq = 100;
			}
			return TRUE;
		}
		break;
	case 1:
		if(commsys->lib_finish == TRUE){
			commsys->err_seq = 100;
		}
		return TRUE;
	case 100:
		GFL_STD_MemClear(commsys, sizeof(COMPATI_CONNECT_SYS));
		return FALSE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   通信：最初のデータ送受信
 * @param   commsys		
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_FirstSend(COMPATI_CONNECT_SYS *commsys)
{
	switch(commsys->seq){
	case 0:
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
				NET_CMD_FIRST, sizeof(CCNET_FIRST_PARAM), commsys->send_first_param)){
			OS_TPrintf("最初のデータ送信成功\n");
			commsys->seq++;
		}
		break;
	case 1:	//相手のデータが送られてくるのを待つ
		if(commsys->receive_ok == TRUE){
			OS_TPrintf("最初のデータ受信完了\n");
			commsys->receive_ok = FALSE;
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   通信：最後のデータ送受信
 * @param   commsys		
 * @retval  TRUE:処理完了　FALSE:処理継続中
 */
//--------------------------------------------------------------
BOOL CompatiComm_ResultSend(COMPATI_CONNECT_SYS *commsys)
{
	switch(commsys->seq){
	case 0:
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
				NET_CMD_RESULT, sizeof(CCNET_RESULT_PARAM), commsys->send_result_param)){
			OS_TPrintf("最後のデータ送信成功\n");
			commsys->seq++;
		}
		break;
	case 1:	//相手のデータが送られてくるのを待つ
		if(commsys->receive_ok == TRUE){
			OS_TPrintf("最後のデータ受信完了\n");
			commsys->receive_ok = FALSE;
			commsys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}


//==============================================================================
//	
//==============================================================================
typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { WB_NET_COMPATI_CHECK };

//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void* _netBeaconGetFunc(void* pWork)
{
	return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void* pWork)
{
	return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
    if(myNo != beaconNo){
        return FALSE;
    }
    return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   初期化完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _initCallBack(void* pWork)
{
	OS_TPrintf("初期化完了コールバックが呼び出された\n");
	return;
}

//--------------------------------------------------------------
/**
 * @brief   通信システム終了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _CommSystemEndCallback(void* pWork)
{
	COMPATI_CONNECT_SYS *commsys = pWork;
	
	OS_TPrintf("通信システム終了コールバックが呼び出された\n");
	commsys->lib_finish = TRUE;
}

//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _connectCallBack(void* pWork, int netID)
{
	COMPATI_CONNECT_SYS *commsys = pWork;
	u32 temp;
	
    OS_TPrintf("ネゴシエーション完了 netID = %d\n", netID);
	commsys->connect_bit |= 1 << netID;
	temp = commsys->connect_bit;
	if(MATH_CountPopulation(temp) >= 2){
		OS_TPrintf("全員のネゴシエーション完了 人数bit=%d\n", commsys->connect_bit);
		commsys->connect_ok = TRUE;
	}
}

static void _endCallBack(void* pWork)
{
	COMPATI_CONNECT_SYS *commsys = pWork;

    OS_TPrintf("endCallBack終了\n");
    commsys->connect_ok = FALSE;
    commsys->connect_bit = 0;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   移動コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	COMPATI_CONNECT_SYS *commsys = pWork;
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if(netID == GFL_NET_SystemGetCurrentID()){
		return;	//自分のデータは無視
	}

	GFL_STD_MemCopy(pData, commsys->receive_first_param, size);
	commsys->receive_ok = TRUE;
	OS_TPrintf("最初のデータ受信コールバック netID = %d\n", netID);
}

//--------------------------------------------------------------
/**
 * @brief   キー情報コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _RecvResultData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	COMPATI_CONNECT_SYS *commsys = pWork;
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if(netID == GFL_NET_SystemGetCurrentID()){
		return;	//自分のデータは無視
	}

	GFL_STD_MemCopy(pData, commsys->receive_result_param, size);
	commsys->receive_ok = TRUE;
	OS_TPrintf("最後のデータ受信コールバック netID = %d\n", netID);
}


