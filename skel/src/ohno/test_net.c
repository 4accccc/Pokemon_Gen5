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
#include "net.h"
#include "ui.h"


//------------------------------------------------------------------
// NETのテスト
//------------------------------------------------------------------

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



//----------------------------------------------------------------
enum{
    _TEST_CONNECT,
    _TEST_1,
    _TEST_2,

};


static int _testNo = 0;
static GFL_NETHANDLE* _pHandle=NULL;

void TEST_NET_Main(void)
{
    u8 mac[6] = {0x00,0x09,0xbf,0x08,0x2e,0x6e};

    
    if(PAD_BUTTON_B == GFL_UI_KeyGetTrg()){
        switch(_testNo){
          case _TEST_CONNECT:
            {
                _pHandle = GFL_NET_CreateHandle();
                GFL_NET_ClientConnectTo(_pHandle,mac);
            }
            _testNo++;
            break;
          case _TEST_1:
            {
                BOOL ret = GFL_NET_SendData(_pHandle, GFL_NET_CMD_COMM_NEGOTIATION,NULL);
                OS_TPrintf("send %d\n",ret);
            }
            _testNo++;
            break;
        }
        OS_TPrintf("c %d\n",_testNo);
    }
    if(PAD_BUTTON_R == GFL_UI_KeyGetTrg()){
        switch(_testNo){
          case _TEST_CONNECT:
            {
                GFL_NETHANDLE* pHandle = GFL_NET_CreateHandle();
                GFL_NET_ServerConnect(pHandle);
            }
            _testNo++;
            break;
        }
        OS_TPrintf("p %d\n",_testNo);
    }
    GFL_NET_MainProc();


}

#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}


// 通信初期化構造体
GFLNetInitializeStruct aGFLNetInit = {
    NULL,  // 受信関数テーブル
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    NULL,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_SYSTEM,  //allocNo
    2,     // 最大接続人数
    16,    // 最大ビーコン収集数
    1,     // 通信を開始するかどうか
};


void TEST_NET_Init(void)
{

    GFL_NET_Initialize(&aGFLNetInit, GFL_HEAPID_SYSTEM);

}



