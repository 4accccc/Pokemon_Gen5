//============================================================================================
/**
 * @file	yt_net.c
 * @brief	DS版ヨッシーのたまご ネットワーク関連のプログラム
 * @author	ohno
 * @date	2007.03.05
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"
#include "yt_net.h"
#include "fallchr.h"
#include "player.h"
#include "../ohno/fatal_error.h"


// static 関数定義
static void _recvCLACTPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTPlayerAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTDelete(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//通信用構造体
struct _NET_PARAM
{
    GFL_NETHANDLE* pNetHandle[2];  //通信用ハンドル 親と子のハンドルを管理するから２個必要
    CLWK* pCLACT[YT_CLACT_MAX];  // 描画してるclact
    PLAYER_PARAM* pp;
    u32 seq;
    BOOL bGameStart;
};


#define _BCON_GET_NUM  (1)

static void _netAutoParent(void* work)
{
//    NET_PARAM* pNet= (NET_PARAM*)work;
//    pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // クライアントハンドル作成
}

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

#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}

static BOOL _netBeaconCompFunc(int myNo,int beaconNo)    ///< ビーコンデータ取得関数
{
    OS_TPrintf("比較してます%d\n",beaconNo);
    return TRUE;
}

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

//普通のワイヤレス通信


/// バトル専用通信コマンドの定義。
enum CommCommandBattle_e {
    YT_NET_COMMAND_ANIM = GFL_NET_CMD_COMMAND_MAX,  ///< アニメコマンド送信
    YT_NET_COMMAND_POS,                             ///< 位置コマンド送信
    YT_NET_COMMAND_PLAYERANIM,            ///< プレーヤーのアニメーション情報の送信
    YT_NET_COMMAND_DELETE,            /// CLACTを消す命令
    //------------------------------------------------ここまで
    YT_NET_COMMAND_MAX   // 終端--------------これは移動させないでください
};

//----------------------------------------------------------------------------
// @brief	アニメーション設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
    u16 anmseq;
} COMM_ANIM_ST;

//----------------------------------------------------------------------------
// @brief	プレイヤーアニメーション設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u16 anm_no;
    u8 line_no;
    u8 rot;
    u8 actno;
} COMM_PLAYER_ANIM_ST;

//----------------------------------------------------------------------------
// @brief	位置設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
    s16 x;
    s16 y;
} COMM_POS_ST;

// @brief	消す命令
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
} COMM_DELETE_ST;



// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    {_recvCLACTAnim, GFL_NET_COMMAND_SIZE(sizeof(COMM_ANIM_ST)), NULL},
    {_recvCLACTPos, GFL_NET_COMMAND_SIZE(sizeof(COMM_POS_ST)), NULL},
    {_recvCLACTPlayerAnim, GFL_NET_COMMAND_SIZE(sizeof(COMM_PLAYER_ANIM_ST)), NULL},
    {_recvCLACTDelete, GFL_NET_COMMAND_SIZE(sizeof(COMM_DELETE_ST)), NULL},
};


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
    NULL, //_netAutoParent,    //自動接続の際に親になる場合に呼ばれる関数
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



enum{
    _INIT_WAIT_PARENT,
    _INIT_WAIT_CHILD,
    _SEARCH_CHILD,
    _CONNECT_WAIT,
    _NEGO_START,
    _TIM_START,
    _TIM_OK,
    _LOOP,
};


#define _TEST_TIMING (14)

static void _seqChange(NET_PARAM* pNet, int no)
{
    NET_PRINT("ytst change %d\n",no);
    pNet->seq = no;
}


#define _SEQCHANGE(no)   _seqChange(pNet,no)



//----------------------------------------------------------------------------
/**
 *	@brief	位置命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_POS_ST* pPos = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;
	CLSYS_POS	pos;

    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    
    //OS_TPrintf("testPos %d %d %d\n",pPos->clactNo, pPos->x, pPos->y);
    pos.x = pPos->x;
    pos.y = pPos->y;
    if(pNet->pCLACT[pPos->clactNo] != NULL){
        GFL_CLACT_WkSetWldPos(pNet->pCLACT[pPos->clactNo],&pos);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	消す命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTDelete(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_POS_ST* pPos = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;
	CLSYS_POS	pos;

    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    if(pNet->pCLACT[pPos->clactNo] != NULL){
        GFL_CLACT_WkDel(pNet->pCLACT[pPos->clactNo]);
        pNet->pCLACT[pPos->clactNo] = NULL;
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_ANIM_ST* pAnim = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;


    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }

    //OS_TPrintf("testAnim %d %d\n",pAnim->clactNo,pAnim->anmseq);

    GF_ASSERT(pAnim->clactNo < YT_CLACT_MAX);
    
    if(pNet->pCLACT[pAnim->clactNo]==NULL){
        pNet->pCLACT[pAnim->clactNo] = YT_InitNetworkFallChr(gp,netID,pAnim->anmseq,0);
    }
    else{
        GFL_CLACT_WkSetAnmSeq(pNet->pCLACT[pAnim->clactNo],pAnim->anmseq);
    }

}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーアニメーション命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTPlayerAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_PLAYER_ANIM_ST* pAnim = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }

    if(pNet->pp == NULL){
        pNet->pp = YT_InitPlayer(gp,netID,netID);
    }
    else{
        YT_PlayerAnimeNetSet(gp, pNet->pp, netID, pAnim->anm_no, pAnim->line_no, pAnim->rot,pAnim->actno);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	親機かどうかを判断する
 */
//-----------------------------------------------------------------------------

BOOL YT_NET_IsParent(NET_PARAM* pNet)
{
    return (pNet->pNetHandle[0] != NULL);
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信の場合はアニメーションＮＯを送信
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPlayerAnmReq(int anm_no,u16 line_no,int rot, int actno,NET_PARAM* pNet)
{
    COMM_PLAYER_ANIM_ST CommPlayerAnmSt;

    if(pNet){
        CommPlayerAnmSt.anm_no = anm_no;
        CommPlayerAnmSt.line_no = line_no;
        CommPlayerAnmSt.rot = rot;
        CommPlayerAnmSt.actno = actno;

        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_PLAYERANIM, &CommPlayerAnmSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、アニメーションナンバーを送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendAnmReq(int clactno,u16 anmseq,NET_PARAM* pNet)
{
    COMM_ANIM_ST CommAnmSt;
    if(pNet){

    CommAnmSt.clactNo = clactno;
    CommAnmSt.anmseq = anmseq;

    GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_ANIM, &CommAnmSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	CLACTを消す
 */
//-----------------------------------------------------------------------------

void YT_NET_DeleteReq(int clactno,NET_PARAM* pNet)
{
    COMM_DELETE_ST CommDelSt;

    if(pNet){

    CommDelSt.clactNo = clactno;
    GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_DELETE, &CommDelSt);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、位置を送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPosReq(int clactno,s16 x, s16 y,NET_PARAM* pNet)
{
    COMM_POS_ST CommPosSt;

    if(pNet){
    CommPosSt.clactNo = clactno;
    CommPosSt.x = x;
    CommPosSt.y = y;
    //OS_TPrintf("位置送信 %d %d\n",x,y);
    GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_POS, &CommPosSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信のメインシーケンス
 */
//-----------------------------------------------------------------------------

BOOL YT_NET_Main(NET_PARAM* pNet)
{

    switch(pNet->seq){
      case _INIT_WAIT_PARENT:
        if(GFL_NET_IsInit()){
            pNet->pNetHandle[0] = GFL_NET_CreateHandle();   // ハンドル作成
            GFL_NET_InitServer(pNet->pNetHandle[0]); // 自動接続
            pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // ハンドル作成
            _SEQCHANGE(_CONNECT_WAIT);
        }
        break;
      case _INIT_WAIT_CHILD:
        if(GFL_NET_IsInit()){
            pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // ハンドル作成
            GFL_NET_StateBeaconScan(pNet->pNetHandle[1]); // 自動接続
            _SEQCHANGE( _SEARCH_CHILD );
        }
        break;
      case _SEARCH_CHILD:
        {
            u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
            if(pData){
                GFL_NET_ConnectToParent(pNet->pNetHandle[1], pData);
                _SEQCHANGE( _CONNECT_WAIT );
            }
        }
        break;
      case _CONNECT_WAIT:
        _SEQCHANGE( _NEGO_START );
        break;
      case _NEGO_START:
        if(YT_NET_IsParent(pNet)){  //親機の場合
            if(GFL_NET_GetNegotiationConnectNum( pNet->pNetHandle[1]) != 0){
                if(GFL_NET_RequestNegotiation( pNet->pNetHandle[1] )){
                    _SEQCHANGE( _TIM_START );
                }
            }
        }
        else{
//            if(GFL_NET_GetNegotiationConnectNum( pNet->pNetHandle[1]) != 0){
                if(GFL_NET_RequestNegotiation( pNet->pNetHandle[1] )){
                    _SEQCHANGE( _TIM_START );
                }
  //          }
        }
        break;
      case _TIM_START:
        if(GFL_NET_GetNegotiationConnectNum(pNet->pNetHandle[1])==2){
            GFL_NET_TimingSyncStart(pNet->pNetHandle[1] , _TEST_TIMING);
            _SEQCHANGE( _TIM_OK );
        }
        break;
      case _TIM_OK:
        if(GFL_NET_IsTimingSync(pNet->pNetHandle[1] , _TEST_TIMING)){
            pNet->bGameStart = TRUE;
            _SEQCHANGE( _LOOP );
        }
        break;

    }
    return pNet->bGameStart;
}

//----------------------------------------------------------------------------
/**
 *	@brief	
 */
//-----------------------------------------------------------------------------

void YT_NET_Init(GAME_PARAM* gp, BOOL bParent)
{
    TCBSYS* tcbsys;
    NET_PARAM* pNet = GFL_HEAP_AllocMemory(gp->heapID, sizeof(NET_PARAM));

    GFL_STD_MemClear(pNet, sizeof(NET_PARAM));
    aGFLNetInit.pWork = gp;
    gp->pNetParam = pNet;

    GFL_NET_sysInit(&aGFLNetInit);

    if(bParent){
        pNet->seq = _INIT_WAIT_PARENT;
    }
    else{
        pNet->seq = _INIT_WAIT_CHILD;
    }

//	GFL_TCB_AddTask(gp->tcbsys,YT_NET_Main,pNet,1);
}



