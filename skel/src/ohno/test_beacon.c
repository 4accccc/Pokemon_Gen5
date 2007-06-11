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
#include "test_beacon.h"
#include "main.h"
#include "fatal_error.h"
#include "net_icondata.h"
#include "textprint.h"

#define _BCON_GET_NUM  (6)

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

typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 5 };

//-------------------------------------------------デバッグ表示

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

static const char	test_index1[] = {"beacon catch"};
static const char	test_index2[] = {"scan mode"};
static const char	test_index3[] = {"point mode"};


enum {
	NUM_TITLE = 0,
	NUM_URL,
};

static const TESTMODE_PRINTLIST _childIndexList[] = {
	{ test_index1, 2, 1, 30, 2 },
	{ test_index2, 2, 22, 30, 1 },
};

static const TESTMODE_PRINTLIST _parentIndexList[] = {
	{ test_index1, 2, 1, 30, 2 },
	{ test_index3, 2, 22, 30, 1 },
};

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static const char font_path[] = {"src/gfl_graphic/gfl_font.dat"};

#define G2D_BACKGROUND_COL	(0x0000)
#define G2D_FONT_COL		(0x7fff)
#define G2D_FONTSELECT_COL	(0x001f)

static const GFL_TEXT_PRINTPARAM default_param = 
{ NULL, 0, 0, 1, 1, 1, 0, GFL_TEXT_WRITE_16 };

#define TEXT_FRM			(GFL_BG_FRAME3_M)
#define TEXT_FRM_PRI		(0)

//------------------------------------------------------------------
/**
 * @brief		内部ワーク構造体
 */
//------------------------------------------------------------------

struct _SKEL_TEST_BEACON_WORK{
    HEAPID heapID;
    BOOL bParent;   ///< 親機モードかどうか
    GFL_TEXT_PRINTPARAM* textParam;
    GFL_BMPWIN*				bmpwin[32];
    int testNo;
};

//------------------------------------------------------------------
/**
 * @brief		メッセージビットマップウインドウコントロール
 */
//------------------------------------------------------------------
static void msg_bmpwin_make
			(SKEL_TEST_BEACON_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	//ビットマップ作成
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 
														GFL_BG_CHRAREA_GET_B );

	//テキストをビットマップに表示
	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	//ビットマップキャラクターをアップデート
	GFL_BMPWIN_TransVramCharacter( testmode->bmpwin[bmpwinNum] );
	//ビットマップスクリーン作成
	//GFL_BMPWIN_MakeScreen( testmode->bmpwin[bmpwinNum] );
}

//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( SKEL_TEST_BEACON_WORK* testmode )
{
	//フォント読み込み
	GFL_TEXT_CreateSystem( font_path );
	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( testmode->heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 16*2 );

		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocClearMemoryLo
										( testmode->heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
		testmode->textParam = param;
	}
	//文字表示ビットマップの作成
	{
		int i,num;
        const TESTMODE_PRINTLIST* pList;

        if(testmode->bParent){
            pList = &_parentIndexList[0];
            num = NELEMS(_parentIndexList);
        }
        else{
            pList = &_childIndexList[0];
            num = NELEMS(_childIndexList);
        }
        
		//表題ビットマップの作成
		for(i = 0 ; i < num ; i++,pList++){
			msg_bmpwin_make( testmode, i, pList->msg,
							pList->posx, pList->posy, 
							pList->sizx, pList->sizy );
		}
	}
	//ＢＧスクリーン転送リクエスト発行
	//GFL_BG_LoadScreenReq( TEXT_FRM );
}



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


static void _testChild(SKEL_TEST_BEACON_WORK* pWork)
{
    u8 mac[6] = {0x00,0x09,0xbf,0x08,0x2e,0x6e};
    u8 beacon;
    int i;

    if(PAD_BUTTON_X == GFL_UI_KEY_GetTrg()){
        for(i = 0;i < _BCON_GET_NUM; i++){
            u8* pData = GFL_NET_GetBeaconMacAddress(i);
            if(pData){
                OS_TPrintf("%d: mac %x%x%x%x%x%x\n",i,pData[0],pData[1],pData[2],pData[3],pData[4],pData[5]);
            }
        }
    }

    // クライアント側のテスト
    switch(pWork->testNo){
      case _TEST_CONNECT:
        {
            _pHandle = GFL_NET_CreateHandle();
            GFL_NET_StartBeaconScan(_pHandle );    // ビーコンを待つ
            //                GFL_NET_InitClientAndConnectToParent(_pHandle, mac);  //macアドレスへ接続
            //                GFL_NET_ChangeoverConnect(_pHandle); // 自動接続
        }
            pWork->testNo++;
            break;
          case _TEST_CONNECT2:
            {
                u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
                if(pData){
                    GFL_NET_ConnectToParent(_pHandle, pData);
                }
            }
            pWork->testNo++;
            break;
          case _TEST_1:
            GFL_NET_RequestNegotiation( _pHandle );
            pWork->testNo++;
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
            pWork->testNo++;
            break;
            


          case _TEST_2:
            {
                u8 send = _TEST_TIMING;
//                BOOL ret = GFL_NET_SendData(_pHandle, GFL_NET_CMD_TIMING_SYNC,&send);
//                OS_TPrintf("send %d\n",ret);
                GFL_NET_TimingSyncStart(_pHandle, send);
            }
            pWork->testNo++;
            break;
          case _TEST_3:
            if(GFL_NET_IsTimingSync(_pHandle,_TEST_TIMING)){
                NET_PRINT("タイミング取れた\n");
                pWork->testNo++;
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
            pWork->testNo++;
            break;
        }
}


static void _testParent(SKEL_TEST_BEACON_WORK* pWork)
{

    // サーバー側のテスト  ビーコン出しているだけ
    switch(_testNo){
      case _TEST_CONNECT:
        {
            _pHandleServer = GFL_NET_CreateHandle();
            GFL_NET_InitServer(_pHandleServer);   // サーバ
            _pHandle = GFL_NET_CreateHandle();  // クライアント
            //                GFL_NET_ChangeoverConnect(_pHandle); // 自動接続
        }
        pWork->testNo++;
        break;
      case _TEST_CONNECT2:
        GFL_NET_RequestNegotiation( _pHandle );
        pWork->testNo++;
        break;
      case _TEST_1:
        break;
    }
}

void TEST_BEACON_Main(SKEL_TEST_BEACON_WORK* pWork)
{
    if(pWork->bParent){
        _testParent(pWork);
    }
    else{
        _testChild(pWork);
    }
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


// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    // 可変サイズテスト
    {_testRecvVariable,          GFL_NET_COMMAND_VARIABLE, NULL},
    // サイズ取得関数テスト
    {_testRecvGetSize,           _getTestCommandSize, NULL},
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

//----------------------------------------------------------------------------
/**
 *	@brief	初期化処理
 */
//-----------------------------------------------------------------------------

SKEL_TEST_BEACON_WORK* TEST_BEACON_Init(HEAPID heapID)
{

    SKEL_TEST_BEACON_WORK* pWork = GFL_HEAP_AllocMemory(heapID, sizeof(SKEL_TEST_BEACON_WORK));
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

    GFL_NET_Init(&aGFLNetInit);

    if(PAD_BUTTON_R == GFL_UI_KEY_GetCont()){
        pWork->bParent = TRUE;
    }
    else if(PAD_BUTTON_L == GFL_UI_KEY_GetCont()){
        pWork->bParent = FALSE;
    }

    return pWork;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了処理
 */
//-----------------------------------------------------------------------------

void TEST_BEACON_End(SKEL_TEST_BEACON_WORK* pWork)
{
    GFL_HEAP_FreeMemory(pWork);
}
