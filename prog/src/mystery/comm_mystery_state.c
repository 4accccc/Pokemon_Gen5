//=============================================================================
/**
 * @file	comm_mystery_state.c
 * @brief	通信状態を管理するサービス  通信の上位にある
 *          	スレッドのひとつとして働き、自分の通信状態や他の機器の
 *          	開始や終了を管理する
 *		※comm_field_state.cの真似っ子
 * @author	Satoshi Mitsuhara
 * @date    	2006.05.17
 */
//=============================================================================
#include <gflib.h>
#include "system/main.h"

#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "print/global_font.h"
#include "print/wordset.h"

#include "mystery.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_mystery.h"
#include "msg/msg_wifi_system.h"

#include "savedata/save_tbl.h"
#include "savedata/mystery_data.h"
#include "savedata/mystatus.h"
#include "savedata/save_control.h"
#include "savedata/config.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"

#include "comm_mystery_state.h"
#include "comm_mystery_gift.h"

//==============================================================================
//	型宣言
//==============================================================================
// コールバック関数の書式
typedef void (*PTRStateFunc)(void);
//==============================================================================
// ワーク
//==============================================================================
typedef struct{
	MYSTERYGIFT_WORK *pMSys;
	MATHRandContext32 sRand; 			//< 親子機ネゴシエーション用乱数キー
	GFL_TCBL *pTcb;
	PTRStateFunc state;
	u16 timer;
	u8 bStateNoChange;
	u8 connectIndex;   				// 子機が接続する親機のindex番号
	MYSTATUS *status[SCAN_PARENT_COUNT_MAX];

	DOWNLOAD_GIFT_DATA recv_data;				// 受信したデータ
	u8 recv_flag;					// 受信したフラグ
	u8 result_flag[SCAN_PARENT_COUNT_MAX];	// ちゃんと受信しました返事

	//  u8 beacon_data[MYSTERY_BEACON_DATA_SIZE];	// ビーコンデータ
	
} _COMM_STATE_WORK;

static _COMM_STATE_WORK *_pCommStateM = NULL;  ///<　ワーク構造体のポインタ

//==============================================================================
// 定義
//==============================================================================
#define _START_TIME (50)     // 開始時間
#define _CHILD_P_SEARCH_TIME (12) ///子機として親を探す時間
#define _PARENT_WAIT_TIME (40) ///親としてのんびり待つ時間
#define _FINALIZE_TIME (2)
#define _EXIT_SENDING_TIME (5)
#define _PARENT_END_TIME (2)
#define _SEND_NAME_TIME (10)
#define _PARENTSCAN_PA (3)  // 親機として検索する確立は1/3

#define _TCB_COMMCHECK_PRT   (10)    ///< フィールドを歩く通信の監視ルーチンのPRI

//==============================================================================
// static宣言
//==============================================================================
static void _commCheckFunc(GFL_TCBL *tcb, void* work);  // ステートを実行しているタスク
static void _mysteryParentInit(void);
static void _mysteryParentWaiting(void);

static void _mysteryChildInit(void);
static void _mysteryChildBconScanning(void);
static void _mysteryChildConnecting(void);
static void _mysteryChildSendName(void);
static void _mysteryChildWaiting(void);

void CommMysteryGiftRecvPlace(const int netID,const  int size, const void* pBuff, void* pWork , GFL_NETHANDLE *pNetHandle);
void CommMysterySendRecvResult(const int netID,const  int size, const void* pBuff, void* pWork , GFL_NETHANDLE *pNetHandle);

//通信コマンドテーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
	/* ふしぎなおくりもの　データ送信 */
	{ CommMysteryGiftRecvPlace,	CommGetMysteryGiftRecvBuff },
	/* 受信成功しましたのフラグ　データ送信 */
	{ CommMysterySendRecvResult,	NULL },
};


#define   _CHANGE_STATE(state, time)  _changeState(state, time)


//NET_Init用構造体に渡す機能関数
BOOL CommMysteryBeaconCheck(GameServiceID GameServiceID1, GameServiceID GameServiceID2);
void* CommMysteryGetBeaconData(void* pWork);
int CommMysteryGetBeaconSize(void* pWork);

//	ビーコン比較関数
BOOL CommMysteryBeaconCheck(GameServiceID GameServiceID1, GameServiceID GameServiceID2)
{
	return TRUE;
}


//==============================================================================
/**
 * 通信管理ステートの初期化処理
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _commStateInitialize(MYSTERYGIFT_WORK *pMSys)
{
	void* pWork;

	if(_pCommStateM != NULL)		// すでに動作中の場合必要ない
		return;

	_pCommStateM = (_COMM_STATE_WORK*)GFL_HEAP_AllocMemory(HEAPID_MYSTERYGIFT, sizeof(_COMM_STATE_WORK));
	MI_CpuFill8(_pCommStateM, 0, sizeof(_COMM_STATE_WORK));
	_pCommStateM->timer = _START_TIME;
	_pCommStateM->pTcb = GFL_TCBL_Create(pMSys->tcblSys , _commCheckFunc, 0, _TCB_COMMCHECK_PRT);
	_pCommStateM->pMSys = pMSys;

	{
	CommMysteryInitNetLib( (void*)_pCommStateM );
	}


#if 0//OLD
	CommCommandMysteryInitialize((void *)pMSys);
	// 初期化
	_pCommStateM = (_COMM_STATE_WORK*)GFL_HEAP_AllocMemory(HEAPID_MYSTERYGIFT, sizeof(_COMM_STATE_WORK));
	MI_CpuFill8(_pCommStateM, 0, sizeof(_COMM_STATE_WORK));
	_pCommStateM->timer = _START_TIME;
	_pCommStateM->pTcb = GFL_TCB_AddTask(pMSys->tcblSys , _commCheckFunc, NULL, _TCB_COMMCHECK_PRT);
	_pCommStateM->pMSys = pMSys;
	CommRandSeedInitialize(&_pCommStateM->sRand);
#endif
}

//他の場所でライブラリだけ初期化する必要が出てきたので
void CommMysteryInitNetLib(void* pWork)
{
	GFLNetInitializeStruct mysteryGiftNetInit = {
		_CommPacketTbl,		//NetSamplePacketTbl,  // 受信関数テーブル
		CM_COMMAND_MAX,		// 受信テーブル要素数
    NULL,		///< ハードで接続した時に呼ばれる
    NULL,		///< ネゴシエーション完了時にコール
    NULL,	// ユーザー同士が交換するデータのポインタ取得関数
		NULL,	// ユーザー同士が交換するデータのサイズ取得関数
		NULL,	//FIELD_COMM_FUNC_GetBeaconData,			// ビーコンデータ取得関数  
		NULL,	//FIELD_COMM_FUNC_GetBeaconSize,			// ビーコンデータサイズ取得関数 
		CommMysteryBeaconCheck,//FIELD_COMM_FUNC_CheckConnectService,	// ビーコンのサービスを比較して繋いで良いかどうか判断する
		NULL,//FIELD_COMM_FUNC_ErrorCallBack,			// 通信不能なエラーが起こった場合呼ばれる
    NULL,  //FatalError
    NULL,//FIELD_COMM_FUNC_DisconnectCallBack,	// 通信切断時に呼ばれる関数(終了時
		NULL,	// オート接続で親になった場合
		NULL,		///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
		NULL,		///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
		NULL,		///< wifiフレンドリスト削除コールバック
		NULL,		///< DWC形式の友達リスト	
		NULL,		///< DWCのユーザデータ（自分のデータ）
		0,			///< DWCのヒープサイズ
		TRUE,        ///< デバック用サーバにつなぐかどうか
		0x333,	//ggid  DP=0x333,RANGER=0x178,WII=0x346
		GFL_HEAPID_APP,  //元になるheapid
		HEAPID_NETWORK,  //通信用にcreateされるHEAPID
		HEAPID_WIFI,  //wifi用にcreateされるHEAPID
		HEAPID_NETWORK,	//
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		4,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		4,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		GFL_NET_TYPE_WIRELESS,		//通信タイプの指定
		TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		WB_NET_FIELDMOVE_SERVICEID,	//GameServiceID
		IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
    0,//MP親最大サイズ 512まで
    0,//dummy
	};
	
	GFL_NET_Init( &mysteryGiftNetInit , NULL , pWork ); 

}


//==============================================================================
/**
 * 通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================
static void _changeState(PTRStateFunc state, int time)
{
	_pCommStateM->state = state;
	_pCommStateM->timer = time;
}


//==============================================================================
/**
 * 親機として初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryParentInit(void)
{
	int i;
	//  MYSTERYGIFT_WORK *wk;
	
	// まずは自分と接続をする
	//TODO:必要？
//	if(!CommIsConnect(CommGetCurrentID()))
//		return;

	for(i = 0; i < SCAN_PARENT_COUNT_MAX; i++)
		_pCommStateM->result_flag[i] = FALSE;
	// 親機としての準備が完了したので、ビーコン情報を添付する
	//  wk = PROC_GetWork(MyseryGiftGetProcp());
	// ↓このデータは送らねば
	//FIXME 適宜置き換え
//	CommInfoSendPokeData();
	_CHANGE_STATE(_mysteryParentWaiting, 0);
}


//==============================================================================
/**
 * 親機として待機中
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryParentWaiting(void)
{
	int i;
	for(i = 0; i < SCAN_PARENT_COUNT_MAX; i++){
//		const GFL_NETHANDLE *handle = GFL_NET_HANDLE_Get(i);
			//FIXME const対応後
//		if( _pCommStateM->status[i] == NULL && GFL_NET_HANDLE_IsNegotiation(handle) == TRUE)
			{
			//FIXME 自前で受信関数が要る
			//_pCommStateM->status[i] = CommInfoGetMyStatus(i);
			if(_pCommStateM->status[i])
			{
#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_Printf("子機 %d から返信がありました！\n", i);
#endif
			}
		}
	}
}


//==============================================================================
/**
 * 子機の初期化
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryChildInit(void)
{
	if(GFL_NET_IsInit() == FALSE){
		return;
	}
	GFL_NET_StartBeaconScan();
	_CHANGE_STATE(_mysteryChildBconScanning, 0);
}


//==============================================================================
/**
 * 子機待機状態  親機ビーコン収集中
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryChildBconScanning(void)
{
	//  CommParentBconCheck();
}


//==============================================================================
/**
 * 子機待機状態  親機に許可もらい中
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryChildConnecting(void)
{
//	CommStateConnectBattleChild(_pCommStateM->connectIndex);
	u8* macAdr = GFL_NET_GetBeaconMacAddress(_pCommStateM->connectIndex);
	GFL_NET_ConnectToParent( macAdr );
	_CHANGE_STATE(_mysteryChildSendName, 0);
}


//==============================================================================
/**
 * 子機待機状態  親機に情報を送信
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryChildSendName(void)
{
	GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
//	if(!CommIsBattleConnectingState())
	if( GFL_NET_HANDLE_IsNegotiation(selfHandle) )
		return;

	_pCommStateM->recv_flag = 0;
	//FIXME 適宜置き換え
//	CommInfoSendPokeData();
//	GFL_NET_TimingSyncStart(selfHandle,MYSTERYGIFT_SYNC_CODE);
	_CHANGE_STATE(_mysteryChildWaiting, 0);
}


//==============================================================================
/**
 * 子機待機状態
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _mysteryChildWaiting(void)
{
	//Exitを受け取ったら子機切断
}




//==============================================================================
/**
 * 通信管理ステートの処理
 * @param
 * @retval  none
 */
//==============================================================================
void _commCheckFunc(GFL_TCBL *tcb, void* work)
{
	if(_pCommStateM==NULL){
		GFL_TCBL_Delete(tcb);
	} else {
		if(_pCommStateM->state != NULL){
			PTRStateFunc state = _pCommStateM->state;
			if(!_pCommStateM->bStateNoChange){
	state();
			}
		}
	}
}


//==============================================================================
/**
 * 「ふしぎなおくりもの」の親としての通信処理開始
 * @param   serviceNo  通信サービス番号
 * @retval  none
 */
//==============================================================================
void CommMysteryStateEnterGiftParent(MYSTERYGIFT_WORK *pMSys, SAVE_CONTROL_WORK *sv, int serviceNo)
{
	//TODO 必要？
//	if(CommIsInitialize())
//		return;      // つながっている場合今は除外する

//	CommStateEnterMysteryParent(sv, serviceNo);
	_commStateInitialize(pMSys);
	GFL_NET_InitServer();
	_CHANGE_STATE(_mysteryParentInit, 0);
}

//==============================================================================
/**
 * 準備が完了した子の数を返す
 * @param   none
 * @retval  none
 */
//==============================================================================
int CommMysteryGetCommChild(void)
{
	int i, max;
	for(max = 0, i = 1; i < SCAN_PARENT_COUNT_MAX; i++)
		if(_pCommStateM->status[i])	max++;
	return max;
}

//==============================================================================
/**
 * 親機：繋がっている子機へデータを送る
 * @param   none
 * @retval  none
 */
//==============================================================================
void CommMysterySendGiftDataParent(const void *p, int size)
{
//	CommSendHugeData_ServerSide(CM_GIFT_DATA, p, size);
	GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
	GFL_NET_SendData( selfHandle , CM_GIFT_DATA , size , p );
}










//==============================================================================
/**
 * 「ふしぎなおくりもの」の子としての通信処理開始
 * @param   serviceNo  通信サービス番号
 * @retval  none
 */
//==============================================================================
void CommMysteryStateEnterGiftChild(MYSTERYGIFT_WORK *pMSys, int serviceNo)
{
	SAVE_CONTROL_WORK *sv;
	//TODO 必要？
//	if(CommIsInitialize())
//		return;      // つながっている場合今は除外する

	// 通信ヒープ作成
//	sv = ((MAINWORK *)PROC_GetParentWork(MyseryGiftGetProcp()))->savedata;
	sv = SaveControl_GetPointer();
//	CommStateEnterMysteryChild(sv, serviceNo);
	_commStateInitialize(pMSys);
	_CHANGE_STATE(_mysteryChildInit, 0);
}


//==============================================================================
/**
 * 「ふしぎなおくりもの」の子としての通信処理開始
 * @param   connectIndex 接続する親機のIndex
 * @retval  none
 */
//==============================================================================
void CommMysteryStateConnectGiftChild(int connectIndex)
{
	_pCommStateM->connectIndex = connectIndex;
	_CHANGE_STATE(_mysteryChildConnecting, 0);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	有効なビーコン情報を受け取ったか返す
 * @param	NONE
 * @return	0～15: 受け取った -1: 受け取らなかった
 *		※受け取った場合はbeacon_dataに情報がコピーされる
*/
//--------------------------------------------------------------------------------------------
int CommMysteryCheckParentBeacon(MYSTERYGIFT_WORK *wk)
{
	int i;
	GIFT_BEACON *p;

	for(i = 0; i < SCAN_PARENT_COUNT_MAX; i++)
	{
		p = (GIFT_BEACON *)GFL_NET_GetBeaconData(i);
		if(p)
		{
			
			if(p->event_id)
			{
#if 0
	GFL_STD_MemCopy(p, &wk->gift_data.gd2.b, sizeof(GIFT_DATA2_B));
#endif
	GFL_STD_MemCopy(p, &wk->gift_data.beacon, sizeof(GIFT_BEACON));
#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_Printf("ビーコンデータを %d に受け取りました！\n", i);
	OS_Printf("event_id = %d\n", p->event_id);
	OS_Printf("have_card = %d\n", p->have_card);
#endif
	return i;
			}
		}
	}
	return -1;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ちゃんとデータを受け取れた事を親機へ伝達
 * @param	NONE
 * @return	NONE
 */
//--------------------------------------------------------------------------------------------
void CommMysteryResultRecvData(void)
{
	//CommSendData(CM_RECV_RESULT, NULL, 0);
	GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
	GFL_NET_SendData( selfHandle , CM_RECV_RESULT , 0 , NULL );
}


//==============================================================================
/**
 * おくりもの　のデータを受け取った際に呼ばれるコールバック
 * @param   netID    送信してきたID
 * @param   size     送られてきたデータサイズ
 * @param   pData    おくりものへのデータポインタ
 * @retval  none
 */
//==============================================================================
void CommMysteryGiftRecvPlace(const int netID, const int size, const void* pData, void* pWork , GFL_NETHANDLE *pNetHandle)
{
	/* 知らない相手からデータが送られてきたら無視 */
	if(_pCommStateM->connectIndex != netID)
		return;
	_pCommStateM->recv_flag = 1;
#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_Printf("おくりもののデータを受け取りました！\n");
#endif
}

//==============================================================================
/**
 * @brief	おくりもののデータが送られてきたか返す
 * @param	NONE
 * @return	TRUE: 送られてきた : FALSE: 送られてきていない
 */
//==============================================================================
int CommMysteryCheckRecvData(void)
{
	return _pCommStateM->recv_flag;
}

//==============================================================================
/**
 * おくりもの　のデータサイズを返す
 * @param   none
 * @retval  データサイズ
 */
//==============================================================================
int CommMysteryGetRecvPlaceSize(void)
{
	return sizeof(GIFT_DATA);
}

//==============================================================================
/**
 * おくりもの　を格納するポインタを返す
 * @param   none
 * @retval  ポインタ
 */
//==============================================================================
u8* CommGetMysteryGiftRecvBuff( int netID, void* pWork, int size)
{
	return (u8 *)&_pCommStateM->recv_data;
}

//==============================================================================
/**
 * おくりもの　が送られてきたことを送る
 * @param   none
 * @retval  ポインタ
 */
//==============================================================================
void CommMysterySendRecvResult(const int netID,const  int size, const void* pBuff, void* pWork , GFL_NETHANDLE *pNetHandle)
{
#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_Printf("%d %d %08X %08X\n", netID, size, pBuff, pWork);
#endif

	_pCommStateM->result_flag[netID] = TRUE;
}

//==============================================================================
/**
 * 接続されているすべての子機から返事が返ってきたか返す
 * @param   none
 * @retval  TRUE: 返ってきた : FALSE: 返ってきていない
 */
//==============================================================================
int CommMysteryGiftGetRecvCheck(void)
{
	int i;
	for(i = 1; i < SCAN_PARENT_COUNT_MAX; i++){
		if(/*CommInfoGetMyStatus(i) && */_pCommStateM->status[i] && _pCommStateM->result_flag[i] != TRUE)
			return FALSE;
	}
	return TRUE;
}


//==============================================================================
/**
 * 通信を終了させる
 * @param   none
 * @retval  none
 */
//==============================================================================
void CommMysteryExitGift(void)
{
	GFL_HEAP_FreeMemory(_pCommStateM);
	_pCommStateM = NULL;
	GFL_NET_Exit(NULL);
//	CommStateExitBattle();
}

/*  */
