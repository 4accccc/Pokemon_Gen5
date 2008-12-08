//==============================================================================
/**
 * @file	net_irc_wiress.c
 * @brief	赤外線通信でマッチング後、ワイヤレス通信へ移行
 * @author	matsuda
 * @date	2008.11.11(火)
 */
//==============================================================================
#include "gflib.h"

#include "ui/ui_def.h"
#include "net_def.h"
#include "net_local.h"
#include "device/net_whpipe.h"
#include "device/dwc_rapcommon.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "wm_icon.h"
#include "device/dwc_rap.h"


#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "net_handle.h"
#include "net_irc.h"
#include "net_irc_wireless.h"


//==============================================================================
//	構造体定義
//==============================================================================
///赤外線ワイヤレス管理ワーク
typedef struct{
	GFLNetInitializeStruct aNetInit;
	void *user_pWork;
	
	NetConnectEndFunc user_connectEndFunc;		///<通信切断時に呼ばれる関数(ユーザー退避)
	
	NetStepEndCallback connect_callback;///<全ての接続完了後に呼ばれるコールバック(ユーザー指定)
	
	BOOL wireless_shift;	///<TRUE:ワイヤレス通信へ移行中
	
	BOOL irc_matching;		///<TRUE:赤外線マッチング完了
	BOOL irc_shutdown;		///<TRUE:赤外線切断完了
	int net_system_id;		///<赤外線マッチング完了時の自分のマシンのNetID
	
	u8 parent_MacAddress[6];	///<親のMacアドレス
	u8 padding[2];
	
	int seq;
	int timer;
}NET_IW_SYS;

//==============================================================================
//	ローカル変数
//==============================================================================
static NET_IW_SYS *NetIwSys = NULL;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Local_IrcConnectCallback(void *pWork);
static void Local_IrcConnectEndFunc(void *pWork);



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   赤外線ワイヤレス移行システムを作成
 *
 * @param   pNetInit		
 * @param   callback		
 * @param   pWork			
 */
//--------------------------------------------------------------
const GFLNetInitializeStruct * GFL_NET_IW_SystemCreate(const GFLNetInitializeStruct* pNetInit, void* pWork)
{
	NET_IW_SYS *iw;
	
	GF_ASSERT(NetIwSys == NULL || NetIwSys->wireless_shift == TRUE);
	
	if(NetIwSys != NULL && NetIwSys->wireless_shift == TRUE){
		return pNetInit;	//赤外線からワイヤレス通信へ移行中。既にシステムは作成済み
	}
	
    iw = GFL_HEAP_AllocClearMemoryLo(pNetInit->baseHeapID, sizeof(NET_IW_SYS));
    iw->aNetInit = *pNetInit;
    iw->user_pWork = pWork;
    iw->user_connectEndFunc = pNetInit->connectEndFunc;
    
    //最初は赤外線通信で接続するようにGFLNetInitializeStructを書き換える
    iw->aNetInit.bNetType = GFL_NET_TYPE_IRC;
	iw->aNetInit.connectEndFunc = Local_IrcConnectEndFunc;
	
    NetIwSys = iw;
    return &NetIwSys->aNetInit;
}

//--------------------------------------------------------------
/**
 * @brief   赤外線ワイヤレス移行システムを破棄
 */
//--------------------------------------------------------------
void GFL_NET_IW_SystemExit(void)
{
	if(NetIwSys != NULL && NetIwSys->wireless_shift == FALSE){
		GFL_HEAP_FreeMemory(NetIwSys);
		NetIwSys = NULL;
	}
}

//--------------------------------------------------------------
/**
 * @brief   赤外線ワイヤレス移行システムが作成されているか確認
 * @retval  TRUE:作成されている
 */
//--------------------------------------------------------------
BOOL GFL_NET_IW_SystemCheck(void)
{
	if(NetIwSys != NULL){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   赤外線接続＞ワイヤレス通信移行完了、まで全てが終了したら呼び出すコールバック関数を登録
 *
 * @param   callback		コールバック関数へのポインタ
 *
 * @retval  最初の赤外線マッチング完了時に呼ばせる代わりのコールバック関数へのポインタ
 */
//--------------------------------------------------------------
NetStepEndCallback GFL_NET_IW_ConnectCallbackSetting(NetStepEndCallback callback)
{
	if(NetIwSys != NULL){
		NetIwSys->connect_callback = callback;
		return Local_IrcConnectCallback;
	}
	return callback;
}

//--------------------------------------------------------------
/**
 * @brief   親のMACアドレスを保存する
 *
 * @param   mac_address		親のMACアドレス
 */
//--------------------------------------------------------------
void GFL_NET_IW_ParentMacAddressSet(const u8 *mac_address)
{
	if(NetIwSys != NULL){
		GFL_STD_MemCopy(mac_address, NetIwSys->parent_MacAddress, 6);
	}
}

//--------------------------------------------------------------
/**
 * @brief   赤外線でマッチング完了後に呼ばれるコールバック関数
 * @param   pWork		ユーザーがGFL_NET_Initで指定しているワークへのポインタ
 * 						その為、この関数内では使用しない
 */
//--------------------------------------------------------------
static void Local_IrcConnectCallback(void *pWork)
{
	OS_TPrintf("IW:赤外線マッチング完了\n");
	NetIwSys->irc_matching = TRUE;
	NetIwSys->net_system_id = GFL_NET_SystemGetCurrentID();	//赤外線を切断する前にID取得
}

//--------------------------------------------------------------
/**
 * @brief   赤外線を切断したら呼ばれるコールバック関数
 * @param   pWork		ユーザーがGFL_NET_Initで指定しているワークへのポインタ
 * 						その為、この関数内では使用しない
 */
//--------------------------------------------------------------
static void Local_IrcConnectEndFunc(void *pWork)
{
	OS_TPrintf("IW:赤外線切断\n");
	NetIwSys->irc_shutdown = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   赤外線からワイヤレス通信への移行、メインプロセス
 */
//--------------------------------------------------------------
void GFL_NET_IW_Main(void)
{
	NET_IW_SYS *iw;
	
	if(NetIwSys == NULL){
		return;
	}
	
	iw = NetIwSys;
	
	switch(iw->seq){
	case 0:
		//赤外線マッチングを待って、その後切断リクエストを送信
		if(iw->irc_matching == TRUE){
			NetIwSys->wireless_shift = TRUE;	//ワークが解放されないように。
			if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL)){
				iw->seq++;
			}
		}
		break;
	case 1:
	#if 0	//※check　現状GFL_NET_CMD_EXIT_REQで切断コールバックは呼ばれないので、
			//暫定的にタイマーで切断した事にする
		//赤外線切断を待ち、ワイヤレス通信を始める
		if(iw->irc_shutdown == TRUE){
	#else
		iw->timer++;
		if(iw->timer > 60){
			iw->timer = 0;
	#endif
			iw->aNetInit.bNetType = GFL_NET_TYPE_WIRELESS;
			iw->aNetInit.connectEndFunc = iw->user_connectEndFunc;
			GFL_NET_Init(&iw->aNetInit, NULL, iw->user_pWork);
			NetIwSys->wireless_shift = FALSE;	//ワイヤレスの初期設定は通ったので。
			iw->seq++;
		}
		break;
	case 2:	//初期化待ち
		if(GFL_NET_IsInit() == TRUE){
			if(iw->net_system_id == GFL_NET_NO_PARENTMACHINE){
				GFL_NET_InitServer();
				OS_TPrintf("IW:親機になってワイヤレス開始\n");
			}
			else{
				GFL_NET_InitClientAndConnectToParent(iw->parent_MacAddress);
				OS_TPrintf("IW:子機になってワイヤレス開始\n");
			}
			iw->seq++;
		}
		break;
	case 3:	//ワイヤレス接続待ち
		//※check　現状接続したか判断する手段がないので、暫定的にタイマーで繋がった事にする
		iw->timer++;
		if(iw->timer > 60*3){
			iw->timer = 0;
			//ネゴシエーション開始
			OS_TPrintf("IW:ネゴシエーション送信\n");
			if(GFL_NET_HANDLE_RequestNegotiation() == TRUE){
				iw->seq++;
			}
		}
		break;
	case 4:	//ネゴシエーション完了待ち
		if(GFL_NET_HANDLE_GetNumNegotiation() >= 2){	//IRCからの移行なので2人固定
			OS_TPrintf("IW:ネゴシエーション揃った\n");
	        if(iw->connect_callback){
				iw->connect_callback(iw->user_pWork);
			}
			GFL_NET_IW_SystemExit();
			return;
		}
		break;
	}
}

