//============================================================================================
/**
 * @file	test_net.c
 * @brief	通信動作テスト用関数
 * @author	ohno
 * @date	2006.12.19
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "gf_standard.h"
#include "test_net.h"
#include "main.h"
#include "fatal_error.h"
#include "net_icondata.h"
#include "gfl_use.h"

#define _BCON_GET_NUM  (6)   //親機ビーコンを取得する台数 自由に決められる
#define _USERNAME_MAXLEN (8) //いわゆる主人公の名前の長さ ビーコン内に含める

//------------------------------------------------------------------
// NETのテスト
//------------------------------------------------------------------

// ローカル通信コマンドの定義
enum _testCommand_e {
    _TEST_VARIABLE = GFL_NET_CMD_COMMAND_MAX,
    _TEST_GETSIZE,
    _TEST_HUGE,
    _TEST_VARIABLE_HUGE,
};

#define _TEST_HUGE_SIZE (5000)
static u8 _dataPool[_TEST_HUGE_SIZE];  //大容量受信バッファ テスト用
static u8 _dataSend[_TEST_HUGE_SIZE];  //大容量送信バッファ テスト用

typedef struct{
    int gameNo;      ///< ゲーム種類
    u8 userName[_USERNAME_MAXLEN];  ///< ユーザーの名前
} _testBeaconStruct;

//ダミービーコンデータ
static _testBeaconStruct _testBeacon = { 1,{'G','F','L','I',0,0,0,0} };


static void* _netBeaconGetFunc(void)    ///< ビーコンデータ取得関数
{
    return &_testBeacon;
}

static int _netBeaconGetSizeFunc(void)    ///< ビーコンデータサイズ取得関数
{
    return sizeof(_testBeacon);
}

static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)    ///< ビーコンデータ取得関数
{
 //   OS_TPrintf("比較してます%d\n",beaconNo);
    return TRUE;
}



//----------------------------------------------------------------
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


static int _testNo = 0;
static GFL_NETHANDLE* _pHandle=NULL;
static GFL_NETHANDLE* _pHandleServer=NULL;
#define _TEST_TIMING (12)

#if GFL_NET_WIFI //GFL_NET_WIFI WIFI通信テスト

void TEST_NET_Main(void)
{
    int i;
    
    // クライアント側のテスト
    if(PAD_BUTTON_B == GFL_UI_KEY_GetTrg()){
        switch(_testNo){
          case _TEST_CONNECT:
            {
                _pHandle = GFL_NET_CreateHandle();
                GFL_NET_WifiLogin(_pHandle );    // wifiLogin マッチング待機へ移動
            }
            _testNo++;
            break;
          case _TEST_CONNECT2:
            _testNo++;
            break;
          case _TEST_1:
            _testNo++;
            break;
          case _TEST_4:
            _testNo++;
            break;
          case _TEST_2:
            _testNo++;
            break;
          case _TEST_3:
            break;
          case _TEST_END:
            break;
        }
        OS_TPrintf("c %d\n",_testNo);
    }

}

#else  //GFL_NET_WIFI 普通のワイヤレス通信

void TEST_NET_Main(void)
{
    u8 mac[6] = {0x00,0x09,0xbf,0x08,0x2e,0x6e};
    u8 beacon;
    int i;
    
//    if(PAD_BUTTON_X == GFL_UI_KEY_GetTrg()){
//        for(i = 0;i < _BCON_GET_NUM; i++){
//            u8* pData = GFL_NET_GetBeaconMacAddress(i);
//            if(pData){
//                OS_TPrintf("%d: mac %x%x%x%x%x%x\n",i,pData[0],pData[1],pData[2],pData[3],pData[4],pData[5]);
////            }
//        }
//    }
    if(PAD_BUTTON_X == GFL_UI_KEY_GetTrg()){
        _testBeaconStruct* pBS; // ビーコンに親機の情報があるので加工して使用する
        for(i = 0;i < _BCON_GET_NUM; i++){
            pBS = GFL_NET_GetBeaconData(i);
            if(pBS){  //発見したら名前を表示している
                OS_TPrintf("%d: Find %s\n",i, pBS->userName);
            }
        }
    }

    // クライアント側のテスト
    if(PAD_BUTTON_B == GFL_UI_KEY_GetTrg()){
        switch(_testNo){
          case _TEST_CONNECT:
            {
                _pHandle = GFL_NET_CreateHandle();
                GFL_NET_StartBeaconScan(_pHandle );    // ビーコンを待つ
//                GFL_NET_InitClientAndConnectToParent(_pHandle, mac);  //macアドレスへ接続
//                GFL_NET_ChangeoverConnect(_pHandle); // 自動接続
            }
            _testNo++;
            break;
          case _TEST_CONNECT2:
            {
                u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
                if(pData){
                    GFL_NET_ConnectToParent(_pHandle, pData);
                }
            }
            _testNo++;
            break;
          case _TEST_1:
            GFL_NET_RequestNegotiation( _pHandle );
            _testNo++;
            break;

          case _TEST_4:
            {
                const u8 buff[10]={1,2,3,4,5,6,7,8,9,10};
                int i;
//                GFL_NET_SendDataEx(_pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_VARIABLE, 10, buff, FALSE, FALSE);
//                GFL_NET_SendDataEx(_pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_GETSIZE, 0, buff, FALSE, FALSE);
//                for(i=0;i<_TEST_HUGE_SIZE;i++){
//                    _dataSend[i] = (u8)i;
//                }
//                GFL_NET_SendDataEx(_pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_HUGE, 0, _dataSend, FALSE, FALSE);
//                GFL_NET_SendDataEx(_pHandle,GFL_NET_SENDID_ALLUSER,
//                                   _TEST_VARIABLE_HUGE, 10, _dataSend, FALSE, FALSE);


                GFL_NET_ChangeMpMode(_pHandle);

            }
            _testNo++;
            break;
            


          case _TEST_2:
            {
                u8 send = _TEST_TIMING;
//                BOOL ret = GFL_NET_SendData(_pHandle, GFL_NET_CMD_TIMING_SYNC,&send);
//                OS_TPrintf("send %d\n",ret);
                GFL_NET_TimingSyncStart(_pHandle, send);
            }
            _testNo++;
            break;
          case _TEST_3:
            if(GFL_NET_IsTimingSync(_pHandle,_TEST_TIMING)){
                NET_PRINT("タイミング取れた\n");
                _testNo++;
            }
            else{
                NET_PRINT("タイミングは取れてない\n");
            }
            break;
          case _TEST_END:
            // その場で切断
         //   GFL_NET_Disconnect();
            // 通信で全員を切断
            GFL_NET_SendData(_pHandle, GFL_NET_CMD_EXIT_REQ, NULL);
            _testNo++;
            break;
        }
        OS_TPrintf("c %d\n",_testNo);
    }
    // サーバー側のテスト
    if(PAD_BUTTON_R == GFL_UI_KEY_GetTrg()){
        switch(_testNo){
          case _TEST_CONNECT:
            {
                _pHandleServer = GFL_NET_CreateHandle();
                GFL_NET_InitServer(_pHandleServer);   // サーバ
                _pHandle = GFL_NET_CreateHandle();  // クライアント
//                GFL_NET_ChangeoverConnect(_pHandle); // 自動接続
            }
            _testNo++;
            break;
          case _TEST_CONNECT2:
            GFL_NET_RequestNegotiation( _pHandle );
            _testNo++;
            break;
          case _TEST_1:
            GFL_NET_TimingSyncStart(_pHandle, _TEST_TIMING);
            _testNo = _TEST_END;
            break;
          case _TEST_END:
            GFL_NET_Disconnect();
            _testNo++;
            break;
          case _TEST_EXIT:
            GFL_NET_Exit();
            _testNo++;
            break;
        }
        OS_TPrintf("p %d\n",_testNo);
    }


}

#endif//GFL_NET_WIFI 


#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}


// 可変サイズ受信
static void _testRecvVariable(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvVariable %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}


static void _testRecvGetSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvGetSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[9]);
}

static int _getTestCommandSize(void)
{
    return 12;
}

static void _testRecvHugeSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvHugeSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[size-1]);
}

static void _testRecvVariableHugeSize(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff=pData;
    NET_PRINT("_testRecvVariableHugeSize %d:  %d : %d %d\n",__LINE__,size,pBuff[0],pBuff[size-1]);
}

static u8* _getHugeMemoryPoolAddress(int netID, void* pWork, int size)
{
    return _dataPool;
}


// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    // 可変サイズテスト
    {_testRecvVariable,          GFL_NET_COMMAND_VARIABLE, NULL},
    // サイズ取得関数テスト
    {_testRecvGetSize,           _getTestCommandSize, NULL},
    // 大きなサイズテスト
    {_testRecvHugeSize,          GFL_NET_COMMAND_SIZE( _TEST_HUGE_SIZE ), _getHugeMemoryPoolAddress},
    // 巨大な可変サイズのテスト
    {_testRecvVariableHugeSize,  GFL_NET_COMMAND_VARIABLE,      _getHugeMemoryPoolAddress},
};

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

#if GFL_NET_WIFI //GFL_NET_WIFI WIFI通信テスト

// この二つのデータが「ともだちこーど」になります。本来はセーブする必要があります
static DWCUserData _testUserData;
static DWCFriendData _testFriendData[_BCON_GET_NUM];

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
    NULL,  // wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL,  // wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    &_testFriendData[0],  // DWC形式の友達リスト	
    &_testUserData,  // DWCのユーザデータ（自分のデータ）
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid  通信ゲームによって変える数字
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    TRUE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

#else  //GFL_NET_WIFI 普通のワイヤレス通信

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
    GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};

#endif //GFL_NET_WIFI

void TEST_NET_Init(void)
{
    //イクニューモンを使用する前に VRAMDをdisableにする必要があるのだが
    //VRAMDが何に使われていたのかがわからないと、消すことができない

    if(GX_VRAM_LCDC_D == GX_GetBankForLCDC()){
        GX_DisableBankForLCDC(); // LCDCにVRAMDが割り当てられてるようなので打ち消す
    }
    else if(GX_VRAM_D & GX_GetBankForBG()){
        GX_DisableBankForBG();
    }
    else if(GX_VRAM_D & GX_GetBankForTex()){
        GX_DisableBankForTex();
    }
    else if(GX_VRAM_D & GX_GetBankForClearImage()){
        GX_DisableBankForClearImage();
    }

    GX_DisableBankForLCDC(); // LCDCにVRAMDが割り当てられてるようなので打ち消す
    GX_DisableBankForBG();
    GX_DisableBankForTex();
    GX_DisableBankForClearImage();

	GFLUser_VIntr();
	GFLUser_VIntr();
	GFLUser_VIntr();
	GFLUser_VIntr();
	GFLUser_VIntr();
	GFLUser_VIntr();
//    SkeltonVBlankFunc();
    
    GFL_NET_Init(&aGFLNetInit);

}



