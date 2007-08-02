//============================================================================================
/**
 * @file	game_net.c
 * @brief	通信動作用関数
 */
//============================================================================================
#include "gflib.h"
#include "main.h"
#include "net_icondata.h"

#include "game_net.h"

#define _BCON_GET_NUM  (1)
//------------------------------------------------------------------
// 
//	初期化
//
//------------------------------------------------------------------
// ローカル通信コマンドの定義
enum _testCommand_e {
    _TEST_VARIABLE = GFL_NET_CMD_COMMAND_MAX,
    _TEST_GETSIZE,
    _TEST_HUGE,
    _TEST_VARIABLE_HUGE,
};

typedef enum {
	gNetSysParent = 0,
	gNetSysChild = 1,
}GAMENET_SYSMODE;

#define _TEST_HUGE_SIZE (5000)

typedef struct {
	u8				_dataPool[_TEST_HUGE_SIZE];  //大容量受信バッファ テスト用
	u8				_dataSend[_TEST_HUGE_SIZE];  //大容量送信バッファ テスト用
	int				_connectSeqNo;
	int				_exitSeqNo;
	GFL_NETHANDLE*	_pHandle;
	GFL_NETHANDLE*	_pHandleServer;
	GAMENET_SYSMODE	mode;

}GAMENET_SYS;

static GAMENET_SYS	gNetSys;

#define _TEST_TIMING (12)

typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 1 };

static void* _netBeaconGetFunc(void)    ///< ビーコンデータ取得関数
{
    return &_testBeacon;
}

static int _netBeaconGetSizeFunc(void)    ///< ビーコンデータサイズ取得関数
{
    return sizeof(_testBeacon);
}

static BOOL _netBeaconCompFunc(int myNo,int beaconNo)    ///< ビーコンデータ取得関数
{
    OS_TPrintf("比較してます%d\n",beaconNo);
    return TRUE;
}

#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}


// 可変サイズ受信
static void _testRecvVariable(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    OS_Printf("_testRecvVariable %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}


static void _testRecvGetSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    OS_Printf("_testRecvGetSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}

static int _getTestCommandSize(void)
{
    return 12;
}

static void _testRecvHugeSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    OS_Printf("_testRecvHugeSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[size-1]);
}

static void _testRecvVariableHugeSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    OS_Printf("_testRecvVariableHugeSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[size-1]);
}

static u8* _getHugeMemoryPoolAddress(int netID, void* pWork, int size)
{
    return gNetSys._dataPool;
}

static void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo)
{
	// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
}

// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    // 可変サイズテスト
    { _testRecvVariable, GFL_NET_COMMAND_VARIABLE, NULL },
    // サイズ取得関数テスト
    { _testRecvGetSize, _getTestCommandSize, NULL },
    // 大きなサイズテスト
    { _testRecvHugeSize, GFL_NET_COMMAND_SIZE( _TEST_HUGE_SIZE ), _getHugeMemoryPoolAddress },
    // 巨大な可変サイズのテスト
    { _testRecvVariableHugeSize, GFL_NET_COMMAND_VARIABLE, _getHugeMemoryPoolAddress },
};

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

// 通信初期化構造体  wifi用
static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    NET_ICONDATA_GetTableData,   // 通信アイコンのファイルARCテーブルを返す関数
    NET_ICONDATA_GetNoBuff,      // 通信アイコンのファイルARCの番号を返す関数
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

void InitGameNet(void)
{
    GFL_NET_Init(&aGFLNetInit);

	gNetSys._connectSeqNo = 0;
	gNetSys._exitSeqNo = 0;
	gNetSys._pHandle = NULL;
	gNetSys._pHandleServer = NULL;
}


//------------------------------------------------------------------
// 
//	接続
//
//------------------------------------------------------------------
enum{
    _CONNECT_START = 0,
    _CONNECT,
    _CONNECT_CHECK,
    _CONNECT_NEGO,
    _CONNECT_NEGOCHECK,
    _CONNECT_TIMINGSTART,
    _CONNECT_TIMINGCHECK,
};

BOOL ConnectGameNet(void)
{
	BOOL result = FALSE;

	switch( gNetSys._connectSeqNo ){

	case _CONNECT_START:
		if( GFL_NET_IsInit() == TRUE ){
			gNetSys._connectSeqNo = _CONNECT;
		}
		break;

	case _CONNECT:
		gNetSys._pHandle = GFL_NET_CreateHandle();
		GFL_NET_ChangeoverConnect( gNetSys._pHandle ); // 自動接続
		gNetSys._connectSeqNo = _CONNECT_NEGO;
		break;

	case _CONNECT_NEGO:
		if( GFL_NET_RequestNegotiation( gNetSys._pHandle ) == TRUE ){
			gNetSys._connectSeqNo = _CONNECT_NEGOCHECK;
		}
		break;

	case _CONNECT_NEGOCHECK:
		if( GFL_NET_IsNegotiation( gNetSys._pHandle ) == TRUE ){
			if( gNetSys.mode == gNetSysChild ){
				//モードセット
				//GFL_NET_ChangeMpMode(gNetSys._pHandle);
			}
			gNetSys._connectSeqNo = _CONNECT_TIMINGSTART;
		}
		break;

	case _CONNECT_TIMINGSTART:
		GFL_NET_TimingSyncStart(gNetSys._pHandle, _TEST_TIMING);
		gNetSys._connectSeqNo = _CONNECT_TIMINGCHECK;
		break;

	case _CONNECT_TIMINGCHECK:
		if(GFL_NET_IsTimingSync(gNetSys._pHandle,_TEST_TIMING)){
			OS_Printf("タイミング取れた\n");
			result = TRUE;
		}
		break;
	}
	return result;
}

#if 0
enum{
    _TEST_CONNECT,
    _TEST_CONNECT2,
    _TEST_1,
    _TEST_4,
    _TEST_2,
    _TEST_3,
    _TEST_END,
    _TEST_EXIT,

};

BOOL ConnectGameNet(void)
{
    // クライアント側のテスト
    if(PAD_BUTTON_B == GFL_UI_KEY_GetTrg()){
        switch( gNetSys._connectSeqNo){
          case _TEST_CONNECT:
            {
                gNetSys._pHandle = GFL_NET_CreateHandle();
                GFL_NET_StartBeaconScan(gNetSys._pHandle );    // ビーコンを待つ
//                GFL_NET_InitClientAndConnectToParent(gNetSys._pHandle, mac);  //macアドレスへ接続
//                GFL_NET_ChangeoverConnect(gNetSys._pHandle); // 自動接続
            }
             gNetSys._connectSeqNo++;
            break;
          case _TEST_CONNECT2:
            {
                u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
                if(pData){
                    GFL_NET_ConnectToParent(gNetSys._pHandle, pData);
                }
            }
             gNetSys._connectSeqNo++;
            break;
          case _TEST_1:
            GFL_NET_RequestNegotiation( gNetSys._pHandle );
             gNetSys._connectSeqNo++;
            break;

          case _TEST_4:
            {
                const u8 buff[10]={1,2,3,4,5,6,7,8,9,10};
                int i;
//                GFL_NET_SendDataEx(gNetSys._pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_VARIABLE, 10, buff, FALSE, FALSE);
//                GFL_NET_SendDataEx(gNetSys._pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_GETSIZE, 0, buff, FALSE, FALSE);
//                for(i=0;i<_TEST_HUGE_SIZE;i++){
//                    gNetSys._dataSend[i] = (u8)i;
//                }
//                GFL_NET_SendDataEx(gNetSys._pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_HUGE, 0, gNetSys._dataSend, FALSE, FALSE);
//                GFL_NET_SendDataEx(gNetSys._pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_VARIABLE_HUGE, 10, gNetSys._dataSend, FALSE, FALSE);


                GFL_NET_ChangeMpMode(gNetSys._pHandle);

            }
             gNetSys._connectSeqNo++;
            break;
            


          case _TEST_2:
            {
                u8 send = _TEST_TIMING;
//                BOOL ret = GFL_NET_SendData(gNetSys._pHandle, GFL_NET_CMD_TIMING_SYNC,&send);
//                OS_TPrintf("send %d\n",ret);
                GFL_NET_TimingSyncStart(gNetSys._pHandle, send);
            }
             gNetSys._connectSeqNo++;
            break;
          case _TEST_3:
            if(GFL_NET_IsTimingSync(gNetSys._pHandle,_TEST_TIMING)){
                OS_Printf("タイミング取れた\n");
				gNetSys.mode = gNetSysChild;	//子
	
                 gNetSys._connectSeqNo++;
            }
            else{
                OS_Printf("タイミングは取れてない\n");
            }
            break;

          case _TEST_END:
            // その場で切断
         //   GFL_NET_Disconnect();
            // 通信で全員を切断
            GFL_NET_SendData(gNetSys._pHandle, GFL_NET_CMD_EXIT_REQ, NULL);
             gNetSys._connectSeqNo++;
            break;
        }
        OS_TPrintf("c %d\n", gNetSys._connectSeqNo);
    }
    // サーバー側のテスト
    if(PAD_BUTTON_R == GFL_UI_KEY_GetTrg()){
        switch( gNetSys._connectSeqNo){
          case _TEST_CONNECT:
            {
                gNetSys._pHandleServer = GFL_NET_CreateHandle();
                GFL_NET_InitServer(gNetSys._pHandleServer);   // サーバ
                gNetSys._pHandle = GFL_NET_CreateHandle();  // クライアント
//                GFL_NET_ChangeoverConnect(gNetSys._pHandle); // 自動接続
            }
             gNetSys._connectSeqNo++;
            break;
          case _TEST_CONNECT2:
            GFL_NET_RequestNegotiation( gNetSys._pHandle );
             gNetSys._connectSeqNo++;
            break;
          case _TEST_1:
            GFL_NET_TimingSyncStart(gNetSys._pHandle, _TEST_TIMING);
			gNetSys.mode = gNetSysParent;	//親
	
             gNetSys._connectSeqNo = _TEST_END;
            break;
          case _TEST_END:
            GFL_NET_Disconnect();
             gNetSys._connectSeqNo++;
            break;
          case _TEST_EXIT:
            GFL_NET_Exit();
             gNetSys._connectSeqNo++;
            break;
        }
        OS_TPrintf("p %d\n", gNetSys._connectSeqNo);
    }
}
#endif

//------------------------------------------------------------------
// 
//	終了
//
//------------------------------------------------------------------
enum{
    _EXIT_START = 0,
    _EXIT_END,
};

BOOL ExitGameNet(void)
{
	BOOL result = FALSE;

	switch( gNetSys._exitSeqNo ){

	case _EXIT_START:
		if( gNetSys.mode == gNetSysParent ){
			GFL_NET_Disconnect();
		}else{
			GFL_NET_SendData(gNetSys._pHandle, GFL_NET_CMD_EXIT_REQ, NULL);
		}
		gNetSys._exitSeqNo = _EXIT_END;
		break;

	case _EXIT_END:
		GFL_NET_Exit();
		result = TRUE;
		break;
	}
	return result;
}



